/*
 * Last Modified: 05/25/86
 */
/*
 * RTU2 is defined if used on Masscomp Real Time Unix 2.x system
 *
 * BSD4 is defined if the system you are on is a 4.x BSD system
 *
 * USGV3 is defined if the system you are on is a System 3 system
 *
 * USGV5 is defined if the system you are on is a System 5 system
 * USGV5 *and* PC7300 is defined if the system you are on is a UNIX PC 7300
 * USGV5 *and* RTU3 is defined used on Masscomp Real Time Unix 3.x system
 *		using the sys5r2 universe
 * USGV5 *and* SCO5 is defined used on SCO Xenix System 5
 * 		for SCO5, use the -LARGE and -Ml2 flags as well
 *
 * define NRANDOM if your system does not have random() and srandom()
 */
#define BSD4
