/*
 * The game's own storage allocator.  Everything it hands out, and
 * all of its own bookkeeping, is in the game's data, so that a saved
 * game holds it.
 */

#ifdef __SANITIZE_ADDRESS__
#include <stdlib.h>
#include <sanitizer/asan_interface.h>
#define NOCHECK	__attribute__((no_sanitize_address))
#define TWICE(cp)	rg_twice(cp)
#else
#define ASAN_POISON_MEMORY_REGION(addr, size)
#define ASAN_UNPOISON_MEMORY_REGION(addr, size)
#define NOCHECK
#define TWICE(cp)
#endif

#define ARENA	(256 * 1024)		/* bytes of storage to hand out */

union header {
    struct {
	union header *h_next;		/* next block on the free list */
	unsigned long h_units;		/* size of block, in headers */
	unsigned long h_size;		/* bytes in use, 0 if free */
    } h;
    long h_align;
};
typedef union header HEADER;

#define NHEADERS	(ARENA / sizeof (HEADER))
#define UNITS(n)	(((n) + sizeof (HEADER) - 1) / sizeof (HEADER) + 1)

/*
 * This file is linked last and these are its only variables, so that
 * they are the last of the game's data: arena of what starts out
 * unset, and heap of what starts out set.
 */
static HEADER arena[NHEADERS];
static struct {
    HEADER *h_top;			/* storage never yet handed out */
    HEADER *h_free;			/* blocks handed back */
    HEADER *volatile h_base;		/* where the game is in memory */
    int h_guarded;
} heap = { arena, 0, arena, 0 };

char *const rg_edata = (char *) (&heap + 1);
char *const rg_ebss = (char *) &arena[NHEADERS];

/*
 * rg_alloc:
 *	Get some storage.  New storage is used while there is any, so
 *	that a block handed back is left alone for as long as possible.
 *	After that the first block handed back that is big enough is
 *	used, and what it has left over goes back on the free list.
 */
NOCHECK char *
rg_alloc(nbytes)
unsigned int nbytes;
{
    register HEADER *hp, **pp, *rest;
    register unsigned long units;

    if (!heap.h_guarded)
    {
	ASAN_POISON_MEMORY_REGION(arena, sizeof arena);
	heap.h_guarded = 1;
    }
    if (nbytes == 0)
	nbytes = 1;
    units = UNITS(nbytes);
    if (heap.h_top + units <= &arena[NHEADERS])
    {
	hp = heap.h_top;
	hp->h.h_units = units;
	heap.h_top += units;
    }
    else
    {
	for (pp = &heap.h_free; (hp = *pp) != 0; pp = &hp->h.h_next)
	    if (hp->h.h_units >= units)
		break;
	if (hp == 0)
	    return 0;
	if (hp->h.h_units - units >= 2)
	{
	    rest = hp + units;
	    rest->h.h_units = hp->h.h_units - units;
	    rest->h.h_size = 0;
	    rest->h.h_next = hp->h.h_next;
	    *pp = rest;
	    hp->h.h_units = units;
	}
	else
	    *pp = hp->h.h_next;
    }
    hp->h.h_size = nbytes;
    ASAN_UNPOISON_MEMORY_REGION(hp + 1, nbytes);
    return (char *) (hp + 1);
}

/*
 * rg_calloc:
 *	Get some storage and clear it
 */
char *
rg_calloc(n, size)
unsigned int n, size;
{
    register char *cp, *p;
    register unsigned int len = n * size;

    if (size != 0 && len / size != n)
	return 0;
    if ((cp = rg_alloc(len)) != 0)
	for (p = cp; len--; )
	    *p++ = 0;
    return cp;
}

/*
 * rg_free:
 *	Hand some storage back.  Handing the same storage back a second
 *	time does nothing.  The free list is kept in the order of the
 *	blocks in the array, and a block is joined to a free one on
 *	either side of it, so that what is handed back does not end up
 *	in pieces too small for anything.
 */
NOCHECK
rg_free(cp)
char *cp;
{
    register HEADER *hp = (HEADER *) cp - 1;
    register HEADER *np, **pp, *prev = 0;

    if (hp->h.h_size == 0)
	TWICE(cp);
    else
    {
	ASAN_POISON_MEMORY_REGION(cp, hp->h.h_size);
	hp->h.h_size = 0;
	for (pp = &heap.h_free; (np = *pp) != 0 && np < hp; pp = &np->h.h_next)
	    prev = np;
	if (np != 0 && hp + hp->h.h_units == np)
	{
	    hp->h.h_units += np->h.h_units;
	    hp->h.h_next = np->h.h_next;
	}
	else
	    hp->h.h_next = np;
	if (prev != 0 && prev + prev->h.h_units == hp)
	{
	    prev->h.h_units += hp->h.h_units;
	    prev->h.h_next = hp->h.h_next;
	}
	else
	    *pp = hp;
    }
}

/*
 * rg_rebase:
 *	The game has been loaded somewhere other than where it was when
 *	it was saved, so move everything that pointed into it by the
 *	same amount.  The game runs from text to end, and what was
 *	saved from data to edata and from bss to ebss.  end itself
 *	counts: the storage is the last thing in the game, and h_top
 *	is there once all of it has been handed out.
 */
NOCHECK
rg_rebase(text, end, data, edata, bss, ebss)
char *text, *end, *data, *edata, *bss, *ebss;
{
    register long *lp;
    register int i;
    register long delta = (char *) arena - (char *) heap.h_base;
    register long lo = (long) text - delta;
    register long hi = (long) end - delta;
    register long unused = (long) heap.h_top;
    register long limit = (long) &arena[NHEADERS] - delta;

    for (i = 0; i < 2; i++, data = bss, edata = ebss)
    {
	lp = (long *) (((long) data + sizeof (long) - 1) & ~(sizeof (long) - 1));
	for (; (char *) (lp + 1) <= edata; lp++)
	    if (*lp >= lo && *lp <= hi && !(*lp > unused && *lp < limit))
		*lp += delta;
    }
}

#ifdef __SANITIZE_ADDRESS__
/*
 * rg_read:
 *	Stands in for read() when a saved game is read back.  The
 *	sanitizer checks what read() writes, and there are guards
 *	between the variables being read over, so read into a buffer
 *	and copy from there without being checked.  The copy goes
 *	through a volatile pointer to keep the compiler from making
 *	a memcpy() of it, which is checked as well.
 */
NOCHECK
rg_read(fd, buf, n)
int fd;
char *buf;
unsigned int n;
{
    register char *tmp = malloc(n);
    register volatile char *to = buf;
    register int i, got;

    got = read(fd, tmp, n);
    for (i = 0; i < got; i++)
	to[i] = tmp[i];
    free(tmp);
    return got;
}

/*
 * rg_twice:
 *	Storage has been handed back a second time.  Read it where the
 *	sanitizer is looking, so that it is reported as a use of storage
 *	already handed back.
 */
rg_twice(cp)
volatile char *cp;
{
    return *cp;
}
#endif

/*
 * rg_restored:
 *	The storage has just been read back from a saved game.  Go
 *	through it, put the guards back around the blocks in use and
 *	make the free list again from the blocks that are not, in order
 *	and joined where they are side by side, so that the list in the
 *	file is never followed.  Returns zero if the
 *	blocks do not add up, so that a damaged file cannot keep the
 *	walk from ending.
 */
NOCHECK
rg_restored()
{
    register HEADER *hp, **pp, *last = 0;
    register unsigned long top;

    top = (char *) heap.h_top - (char *) arena;
    if (top > sizeof arena || top % sizeof (HEADER) != 0)
	return 0;
    ASAN_POISON_MEMORY_REGION(arena, sizeof arena);
    pp = &heap.h_free;
    for (hp = arena; hp < heap.h_top; hp += hp->h.h_units)
    {
	if (hp->h.h_units < 2 || hp->h.h_units > heap.h_top - hp
	    || hp->h.h_size > (hp->h.h_units - 1) * sizeof (HEADER))
		return 0;
	if (hp->h.h_size == 0)
	{
	    if (last != 0 && last + last->h.h_units == hp)
		last->h.h_units += hp->h.h_units;
	    else
	    {
		*pp = last = hp;
		pp = &hp->h.h_next;
	    }
	}
	else
	{
	    last = 0;
	    ASAN_UNPOISON_MEMORY_REGION(hp + 1, hp->h.h_size);
	}
    }
    *pp = 0;
    heap.h_guarded = 1;
    return 1;
}
