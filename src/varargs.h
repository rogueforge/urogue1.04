/*
 * Last Modified: 05/25/86
 */
/*
 *	This header should be used (and modified when needed) for systems
 *	with no "varargs.h" system header.  To use this file for compilation,
 *	name this file ``varargs.h'' and put it in a directory that is in
 *	the directory ``include list'' specified at compile time.  This file
 *	may be kept in the source directory.
 *
 *	If you do NOT want to use this file for compilation (e.g. a system
 *	with its own ``varargs.h'' header (probably in /usr/include)), RENAME
 *	this file so it is NOT ``varargs.h''.  I prefer ``varargs.hdr''.
 *
 *	The intent of ``varargs'' is to make it possible for C programs to
 *	access arguments in a general fashion as opposed to a name for each
 *	argument.  This is useful for implementing "printf()" like argument
 *	handling, for example.
 */

#if !defined(va_dcl)

typedef char	*va_list;

/*
 *	Various defines for the variable argument accessing macros
 */
#define		va_dcl			int va_alist;
#define		va_start(list)		list = (char *) &va_alist
#define		va_end(list)
#define		va_arg(list, mode)	((mode *)(list += sizeof(mode)))[-1]

#endif NOT va_dcl
