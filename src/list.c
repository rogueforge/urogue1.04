/*
 * Last Modified: Dec 29, 1990
 */
/*
 * Functions for dealing with linked lists of goodies
 *
 */

#include "rogue.h"

/*
 * ur_alloc, ur_free
 *
 * These are just calls to the system alloc and free, and they also
 * adjust the totals. The buffer is cleared out because idents need to 
 * be zero before going into the pack, or they will be used as indices!
 */

char *
ur_alloc(size)

register unsigned int size;

{
register char *buf_p, *tmp_p;
total++;
buf_p = (char *) malloc(size);
if (buf_p == NULL)
  return(NULL);
for (tmp_p = buf_p; size--;)	/* clear out the buffer */
  *tmp_p++ = 0;
return(buf_p);
}

ur_free(buf_p)

char *buf_p;
{
cfree(buf_p);
total--;
}

/*
 * detach:
 *	Takes an item out of whatever linked list it might be in
 */

_detach(list, item)
register struct linked_list **list, *item;
{
    if (*list == item)
	*list = next(item);
    if (prev(item) != NULL) item->l_prev->l_next = next(item);
    if (next(item) != NULL) item->l_next->l_prev = prev(item);
    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
 * _attach:
 *	add an item to the head of a list
 */

_attach(list, item)
register struct linked_list **list, *item;
{
    if (*list != NULL)
    {
	item->l_next = *list;
	(*list)->l_prev = item;
	item->l_prev = NULL;
    }
    else
    {
	item->l_next = NULL;
	item->l_prev = NULL;
    }

    *list = item;
}

/*
 * _attach_after
 *
 * Attaches the given item after the supplied one in the list. If the 
 * listed item is NULL, the new item is attached at the head of the 
 * list.
 */

_attach_after(list_pp, list_p, new_p)

register linked_list **list_pp;	/* pointer to the head of the list */
register linked_list *list_p;	/* pointer to place to insert after */
register linked_list *new_p;	/* new item to insert */

{
if (list_p == NULL)
  {
    _attach(list_pp, new_p);	/* stuff it at the beginning */
    return;
  }

if (next(list_p) != NULL)	/* something after this one? */
  {
    next(new_p) = next(list_p);
    prev(next(list_p)) = new_p;
  }

else
  next(new_p) = NULL;

next(list_p) = new_p;
prev(new_p) = list_p;
}

/*
 * _free_list:
 *	Throw the whole blamed thing away
 */

_free_list(ptr)
register linked_list **ptr;
{
    register linked_list *item;

    while (*ptr != NULL)
    {
	item = *ptr;
	*ptr = next(item);
	discard(item);
    }
}

/*
 * discard:
 *	free up an item
 */

discard(item)
     register struct linked_list *item;
{
  throw_away(item->l_data);
  ur_free((char *)item);
}

/*
 * throw_away
 *
 * toss out something (like discard, but without the link_list)
 */

throw_away(ptr)

     char *ptr;		/* probably an object */
{
  free_ident(ptr);
  ur_free(ptr);
}
/*
 * new_item
 *	get a new item with a specified size
 */

struct linked_list *
new_item(size)
int size;
{
    register struct linked_list *item;

    if ((item = (struct linked_list *) new(sizeof *item)) == NULL)
	msg("Ran out of memory for header after %d items.", total);
    if ((item->l_data = new(size)) == NULL)
	msg("Ran out of memory for data after %d items.", total);
    item->l_next = item->l_prev = NULL;
    return item;
}

char *
new(size)
unsigned int size;
{
    register char *space = ur_alloc(size);
    static char errbuf[2*LINELEN];

    if (space == NULL) {
	sprintf(errbuf,"Rogue ran out of memory (used = %d, wanted = %d).",
		sbrk(0), size);
	fatal(errbuf);
    }
    return space;
}
