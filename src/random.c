/*
 * Last Modified: Dec 30, 1990
 */

/*
 * random and associated routines lifted from 4.2BSD
 */

#include <stdio.h>

#define		TYPE_0		0		/* linear congruential */
#define		BREAK_0		8
#define		DEG_0		0
#define		SEP_0		0

#define		TYPE_1		1		/* x**7 + x**3 + 1 */
#define		BREAK_1		32
#define		DEG_1		7
#define		SEP_1		3

#define		TYPE_2		2		/* x**15 + x + 1 */
#define		BREAK_2		64
#define		DEG_2		15
#define		SEP_2		1

#define		TYPE_3		3		/* x**31 + x**3 + 1 */
#define		BREAK_3		128
#define		DEG_3		31
#define		SEP_3		3

#define		TYPE_4		4		/* x**63 + x + 1 */
#define		BREAK_4		256
#define		DEG_4		63
#define		SEP_4		1

#define		MAX_TYPES	5		/* max number of types above */

static  int		degrees[ MAX_TYPES ]	= { DEG_0, DEG_1, DEG_2,
								DEG_3, DEG_4 };

static  int		seps[ MAX_TYPES ]	= { SEP_0, SEP_1, SEP_2,
								SEP_3, SEP_4 };

static  long		randtbl[ DEG_3 + 1 ]	= { TYPE_3,
			    0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342, 
			    0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb, 
			    0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd, 
			    0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86, 
			    0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7, 
			    0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc, 
			    0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b, 
					0xf5ad9d0e, 0x8999220b, 0x27fb47b9 };

static  long		*fptr			= &randtbl[ SEP_3 + 1 ];
static  long		*rptr			= &randtbl[ 1 ];

static  long		*state			= &randtbl[ -1 ];

static  int		rand_type		= TYPE_3;
static  int		rand_deg		= DEG_3;
static  int		rand_sep		= SEP_3;

static  long		*end_ptr		= &randtbl[ DEG_3 + 1 ];

srandom( x )

    unsigned		x;
{
	long random();
    	register  int		i, j;

	if(  rand_type  ==  TYPE_0  )  {
	    state[ 0 ] = x;
	}
	else  {
	    j = 1;
	    state[ 0 ] = x;
	    for( i = 1; i < rand_deg; i++ )  {
		state[i] = 1103515245*state[i - 1] + 12345;
	    }
	    fptr = &state[ rand_sep ];
	    rptr = &state[ 0 ];
	    for( i = 0; i < 10*rand_deg; i++ )  random();
	}
}

char  *
initstate( seed, arg_state, n )

    unsigned		seed;			/* seed for R. N. G. */
    char		*arg_state;		/* pointer to state array */
    int			n;			/* # bytes of state info */
{
	register  char		*ostate		= (char *)( &state[ -1 ] );

	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	if(  n  <  BREAK_1  )  {
	    if(  n  <  BREAK_0  )  {
		fprintf( stderr, "initstate: not enough state (%d bytes) with which to do jack; ignored.\n" );
		return;
	    }
	    rand_type = TYPE_0;
	    rand_deg = DEG_0;
	    rand_sep = SEP_0;
	}
	else  {
	    if(  n  <  BREAK_2  )  {
		rand_type = TYPE_1;
		rand_deg = DEG_1;
		rand_sep = SEP_1;
	    }
	    else  {
		if(  n  <  BREAK_3  )  {
		    rand_type = TYPE_2;
		    rand_deg = DEG_2;
		    rand_sep = SEP_2;
		}
		else  {
		    if(  n  <  BREAK_4  )  {
			rand_type = TYPE_3;
			rand_deg = DEG_3;
			rand_sep = SEP_3;
		    }
		    else  {
			rand_type = TYPE_4;
			rand_deg = DEG_4;
			rand_sep = SEP_4;
		    }
		}
	    }
	}
	state = &(  ( (long *)arg_state )[1]  );	/* first location */
	end_ptr = &state[ rand_deg ];	/* must set end_ptr before srandom */
	srandom( seed );
	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	return( ostate );
}

char  *
setstate( arg_state )

    char		*arg_state;
{
	register  long		*new_state	= (long *)arg_state;
	register  int		type		= new_state[0]%MAX_TYPES;
	register  int		rear		= new_state[0]/MAX_TYPES;
	char			*ostate		= (char *)( &state[ -1 ] );

	if(  rand_type  ==  TYPE_0  )  state[ -1 ] = rand_type;
	else  state[ -1 ] = MAX_TYPES*(rptr - state) + rand_type;
	switch(  type  )  {
	    case  TYPE_0:
	    case  TYPE_1:
	    case  TYPE_2:
	    case  TYPE_3:
	    case  TYPE_4:
		rand_type = type;
		rand_deg = degrees[ type ];
		rand_sep = seps[ type ];
		break;

	    default:
		fprintf( stderr, "setstate: state info has been munged; not changed.\n" );
	}
	state = &new_state[ 1 ];
	if(  rand_type  !=  TYPE_0  )  {
	    rptr = &state[ rear ];
	    fptr = &state[ (rear + rand_sep)%rand_deg ];
	}
	end_ptr = &state[ rand_deg ];		/* set end_ptr too */
	return( ostate );
}

long
random()
{
	long		i;
	
	if(  rand_type  ==  TYPE_0  )  {
	    i = state[0] = ( state[0]*1103515245 + 12345 )&0x7fffffff;
	}
	else  {
	    *fptr += *rptr;
	    i = (*fptr >> 1)&0x7fffffff;	/* chucking least random bit */
	    if(  ++fptr  >=  end_ptr  )  {
		fptr = state;
		++rptr;
	    }
	    else  {
		if(  ++rptr  >=  end_ptr  )  rptr = state;
	    }
	}
	return( i );
}


get_rand(x, y)
int x, y;
{
	long r, random();
	int s;

	two_sort(x, y);

	r = random();
	r = (r % ((y-x)+1)) + x;
	s = (int) r;
	return(s);
}

rand_percent(percentage)
{
	return(get_rand(1, 100) <= percentage);
}
