/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 *
 * Author: Ceriel J.H. Jacobs
 */

/* P A R S E   T R E E   W A L K E R */

/* $Header$ */

/*	Definition of WalkNode macro
*/

extern int (*WalkTable[])();

#define	WalkNode(xnd, xlab)	if (! xnd) ; else (*WalkTable[(xnd)->nd_class])((xnd), (xlab))

extern label	text_label;
extern label	data_label;
