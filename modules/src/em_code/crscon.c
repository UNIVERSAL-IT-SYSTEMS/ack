#include "em_private.h"

/* $Id$ */

void
CC_crscon(op, v, s)
	char *v;
	arith s;
{
	/*	CON or ROM with argument SCON(v,z)
	*/
	PS(op);
	SCON(v, s);
	CEND();
	NL();
}
