/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 *
 * Author: Ceriel J.H. Jacobs
 */

/* C O N S T A N T   E X P R E S S I O N   H A N D L I N G */

/* $Header$ */

#include	"debug.h"
#include	"target_sizes.h"

#include	<em_arith.h>
#include	<em_label.h>
#include	<assert.h>

#include	"idf.h"
#include	"type.h"
#include	"LLlex.h"
#include	"node.h"
#include	"Lpars.h"
#include	"standards.h"
#include	"warning.h"

long mach_long_sign;	/* sign bit of the machine long */
int mach_long_size;	/* size of long on this machine == sizeof(long) */
long full_mask[MAXSIZE];/* full_mask[1] == 0xFF, full_mask[2] == 0xFFFF, .. */
long max_int[MAXSIZE];	/* max_int[1] == 0x7F, max_int[2] == 0x7FFF, .. */
long min_int[MAXSIZE];	/* min_int[1] == 0xFFFFFF80, min_int[2] = 0xFFFF8000,
			   ...
			*/
unsigned int wrd_bits;	/* number of bits in a word */

extern char options[];

overflow(expp)
	t_node *expp;
{
	if (expp->nd_type != address_type) {
	    node_warning(expp, W_ORDINARY, "overflow in constant expression");
	}
}

underflow(expp)
	t_node *expp;
{
	if (expp->nd_type != address_type) {
	    node_warning(expp, W_ORDINARY, "underflow in constant expression");
	}
}

STATIC
commonbin(expp)
	register t_node *expp;
{
	expp->nd_class = Value;
	expp->nd_token = expp->nd_right->nd_token;
	CutSize(expp);
	FreeLR(expp);
}

cstunary(expp)
	register t_node *expp;
{
	/*	The unary operation in "expp" is performed on the constant
		expression below it, and the result restored in expp.
	*/
	register t_node *right = expp->nd_right;
	arith o1;

	switch(expp->nd_symb) {
	/* Should not get here
	case '+':
		break;
	*/

	case '-':
		if (right->nd_INT == min_int[(int)(right->nd_type->tp_size)])
			overflow(expp);
		
		o1 = -right->nd_INT;
		break;

	case NOT:
	case '~':
		o1 = !right->nd_INT;
		break;

	default:
		crash("(cstunary)");
	}

	commonbin(expp);
	expp->nd_INT = o1;
}

STATIC
divide(pdiv, prem)
	arith *pdiv, *prem;
{
	/*	Unsigned divide *pdiv by *prem, and store result in *pdiv,
		remainder in *prem
	*/
	register arith o1 = *pdiv;
	register arith o2 = *prem;

	/*	this is more of a problem than you might
		think on C compilers which do not have
		unsigned long.
	*/
	if (o2 & mach_long_sign)	{/* o2 > max_long */
		if (! (o1 >= 0 || o1 < o2)) {
			/*	this is the unsigned test
				o1 < o2 for o2 > max_long
			*/
			*prem = o2 - o1;
			*pdiv = 1;
		}
		else {
			*pdiv = 0;
		}
	}
	else	{		/* o2 <= max_long */
		long half, bit, hdiv, hrem, rem;

		half = (o1 >> 1) & ~mach_long_sign;
		bit = o1 & 01;
		/*	now o1 == 2 * half + bit
			and half <= max_long
			and bit <= max_long
		*/
		hdiv = half / o2;
		hrem = half % o2;
		rem = 2 * hrem + bit;
		*pdiv = 2*hdiv;
		*prem = rem;
		if (rem < 0 || rem >= o2) {
			/*	that is the unsigned compare
				rem >= o2 for o2 <= max_long
			*/
			*pdiv += 1;
			*prem -= o2;
		}
	}
}

cstibin(expp)
	register t_node *expp;
{
	/*	The binary operation in "expp" is performed on the constant
		expressions below it, and the result restored in expp.
		This version is for INTEGER expressions.
	*/
	arith o1 = expp->nd_left->nd_INT;
	arith o2 = expp->nd_right->nd_INT;
	register int sz = expp->nd_type->tp_size;

	assert(expp->nd_class == Oper);
	assert(expp->nd_left->nd_class == Value);
	assert(expp->nd_right->nd_class == Value);

	switch (expp->nd_symb)	{
	case '*':
		if (o1 > 0 && o2 > 0) {
			if (max_int[sz] / o1 < o2) overflow(expp);
		}
		else if (o1 < 0 && o2 < 0) {
			if (o1 == min_int[sz] || o2 == min_int[sz] ||
			    max_int[sz] / (-o1) < (-o2)) overflow(expp);
		}
		else if (o1 > 0) {
			if (min_int[sz] / o1 > o2) overflow(expp);
		}
		else if (o2 > 0) {
			if (min_int[sz] / o2 > o1) overflow(expp);
		}
		o1 *= o2;
		break;

	case DIV:
		if (o2 == 0)	{
			node_error(expp, "division by 0");
			return;
		}
		o1 /= o2;	/* ??? */
		break;

	case MOD:
		if (o2 == 0)	{
			node_error(expp, "modulo by 0");
			return;
		}
		o1 %= o2;	/* ??? */
		break;

	case '+':
		if (o1 > 0 && o2 > 0) {
			if (max_int[sz] - o1 < o2) overflow(expp);
		}
		else if (o1 < 0 && o2 < 0) {
			if (min_int[sz] - o1 > o2) overflow(expp);
		}
		o1 += o2;
		break;

	case '-':
		if (o1 >= 0 && o2 < 0) {
			if (max_int[sz] + o2 < o1) overflow(expp);
		}
		else if (o1 < 0 && o2 >= 0) {
			if (min_int[sz] + o2 > o1) overflow(expp);
		}
		o1 -= o2;
		break;

	case '<':
		o1 = (o1 < o2);
		break;

	case '>':
		o1 = (o1 > o2);
		break;

	case LESSEQUAL:
		o1 = (o2 <= o1);
		break;

	case GREATEREQUAL:
		o1 = (o2 >= o1);
		break;

	case '=':
		o1 = (o1 == o2);
		break;

	case '#':
		o1 = (o1 != o2);
		break;

	default:
		crash("(cstibin)");
	}

	commonbin(expp);
	expp->nd_INT = o1;
}

cstubin(expp)
	register t_node *expp;
{
	/*	The binary operation in "expp" is performed on the constant
		expressions below it, and the result restored in
		expp.
	*/
	arith o1 = expp->nd_left->nd_INT;
	arith o2 = expp->nd_right->nd_INT;
	register int sz = expp->nd_type->tp_size;
	arith tmp1, tmp2;

	assert(expp->nd_class == Oper);
	assert(expp->nd_left->nd_class == Value);
	assert(expp->nd_right->nd_class == Value);

	switch (expp->nd_symb)	{
	case '*':
		if (o1 == 0 || o2 == 0) {
			o1 = 0;
			break;
		}
		tmp1 = full_mask[sz];
		tmp2 = o2;
		divide(&tmp1, &tmp2);
		if (! chk_bounds(o1, tmp1, T_CARDINAL)) overflow(expp);
		o1 *= o2;
		break;

	case DIV:
		if (o2 == 0)	{
			node_error(expp, "division by 0");
			return;
		}
		divide(&o1, &o2);
		break;

	case MOD:
		if (o2 == 0)	{
			node_error(expp, "modulo by 0");
			return;
		}
		divide(&o1, &o2);
		o1 = o2;
		break;

	case '+':
		if (! chk_bounds(o2, full_mask[sz] - o1, T_CARDINAL)) {
			overflow(expp);
		}
		o1 += o2;
		break;

	case '-':
		if (! chk_bounds(o2, o1, T_CARDINAL)) {
			if (expp->nd_type->tp_fund == T_INTORCARD) {
				expp->nd_type = int_type;
				if (! chk_bounds(min_int[sz], o1 - o2, T_CARDINAL)) {
					underflow(expp);
				}
			}
			else	underflow(expp);
		}
		o1 -= o2;
		break;

	case '<':
		o1 = ! chk_bounds(o2, o1, T_CARDINAL);
		break;

	case '>':
		o1 = ! chk_bounds(o1, o2, T_CARDINAL);
		break;

	case LESSEQUAL:
		o1 = chk_bounds(o1, o2, T_CARDINAL);
		break;

	case GREATEREQUAL:
		o1 = chk_bounds(o2, o1, T_CARDINAL);
		break;

	case '=':
		o1 = (o1 == o2);
		break;

	case '#':
		o1 = (o1 != o2);
		break;

	case AND:
	case '&':
		o1 = (o1 && o2);
		break;

	case OR:
		o1 = (o1 || o2);
		break;

	default:
		crash("(cstubin)");
	}

	commonbin(expp);
	expp->nd_INT = o1;
	if (expp->nd_type == bool_type) expp->nd_symb = INTEGER;
}

cstset(expp)
	register t_node *expp;
{
	extern arith *MkSet();
	register arith *set1, *set2;
	register arith *resultset;
	register unsigned int setsize;
	register int j;

	assert(expp->nd_right->nd_class == Set);
	assert(expp->nd_symb == IN || expp->nd_left->nd_class == Set);

	set2 = expp->nd_right->nd_set;
	setsize = (unsigned) (expp->nd_right->nd_type->tp_size) / (unsigned) word_size;

	if (expp->nd_symb == IN) {
		unsigned i;

		assert(expp->nd_left->nd_class == Value);

		expp->nd_left->nd_INT -= expp->nd_right->nd_type->set_low;
		i = expp->nd_left->nd_INT;
		expp->nd_class = Value;
		expp->nd_INT = (expp->nd_left->nd_INT >= 0 &&
				expp->nd_left->nd_INT < setsize * wrd_bits &&
		    (set2[i / wrd_bits] & (1 << (i % wrd_bits))));
		FreeSet(set2);
		expp->nd_symb = INTEGER;
		FreeLR(expp);
		return;
	}

	set1 = expp->nd_left->nd_set;
	switch(expp->nd_symb) {
	case '+': /* Set union */
	case '-': /* Set difference */
	case '*': /* Set intersection */
	case '/': /* Symmetric set difference */
		expp->nd_set = resultset = MkSet(setsize * (unsigned) word_size);
		for (j = 0; j < setsize; j++) {
			switch(expp->nd_symb) {
			case '+':
				*resultset = *set1++ | *set2++;
				break;
			case '-':
				*resultset = *set1++ & ~*set2++;
				break;
			case '*':
				*resultset = *set1++ & *set2++;
				break;
			case '/':
				*resultset = *set1++ ^ *set2++;
				break;
			}
			resultset++;
		}
		expp->nd_class = Set;
		break;

	case GREATEREQUAL:
	case LESSEQUAL:
	case '=':
	case '#':
		/* Constant set comparisons
		*/
		for (j = 0; j < setsize; j++) {
			switch(expp->nd_symb) {
			case GREATEREQUAL:
				if ((*set1 | *set2++) != *set1) break;
				set1++;
				continue;
			case LESSEQUAL:
				if ((*set2 | *set1++) != *set2) break;
				set2++;
				continue;
			case '=':
			case '#':
				if (*set1++ != *set2++) break;
				continue;
			}
			break;
		}
		if (j < setsize) {
			expp->nd_INT = expp->nd_symb == '#';
		}
		else {
			expp->nd_INT = expp->nd_symb != '#';
		}
		expp->nd_class = Value;
		expp->nd_symb = INTEGER;
		break;
	default:
		crash("(cstset)");
	}
	FreeSet(expp->nd_left->nd_set);
	FreeSet(expp->nd_right->nd_set);
	FreeLR(expp);
}

cstcall(expp, call)
	register t_node *expp;
{
	/*	a standard procedure call is found that can be evaluated
		compile time, so do so.
	*/
	register t_node *expr;
	register t_type *tp;

	assert(expp->nd_class == Call);

	expr = expp->nd_right->nd_left;
	tp = expr->nd_type;

	expp->nd_class = Value;
	expp->nd_symb = INTEGER;
	expp->nd_INT = expr->nd_INT;
	switch(call) {
	case S_ABS:
		if (expp->nd_INT < 0) {
			if (expp->nd_INT <= min_int[(int)(tp->tp_size)]) {
				overflow(expr);
			}
			expp->nd_INT = - expp->nd_INT;
		}
		CutSize(expp);
		break;

	case S_CAP:
		if (expp->nd_INT >= 'a' && expp->nd_INT <= 'z') {
			expp->nd_INT += ('A' - 'a');
		}
		break;

	case S_MAX:
		if (tp->tp_fund == T_INTEGER) {
			expp->nd_INT = max_int[(int)(tp->tp_size)];
		}
		else if (tp == card_type) {
			expp->nd_INT = full_mask[(int)(int_size)];
		}
		else if (tp->tp_fund == T_SUBRANGE) {
			expp->nd_INT = tp->sub_ub;
		}
		else	expp->nd_INT = tp->enm_ncst - 1;
		break;

	case S_MIN:
		if (tp->tp_fund == T_INTEGER) {
			expp->nd_INT = min_int[(int)(tp->tp_size)];
		}
		else if (tp->tp_fund == T_SUBRANGE) {
			expp->nd_INT = tp->sub_lb;
		}
		else	expp->nd_INT = 0;
		break;

	case S_ODD:
		expp->nd_INT &= 1;
		break;

	case S_SIZE:
		expp->nd_INT = tp->tp_size;
		break;

	default:
		crash("(cstcall)");
	}
	FreeLR(expp);
}

CutSize(expr)
	register t_node *expr;
{
	/*	The constant value of the expression expr is made to
		conform to the size of the type of the expression.
	*/
	register t_type *tp = BaseType(expr->nd_type);

	assert(expr->nd_class == Value);
	if (tp->tp_fund != T_INTEGER) {
		expr->nd_INT &= full_mask[(int)(tp->tp_size)];
	}
	else {
		int nbits = (int) (mach_long_size - tp->tp_size) * 8;

		expr->nd_INT = (expr->nd_INT << nbits) >> nbits;
	}
}

InitCst()
{
	register int i = 0;
	register arith bt = (arith)0;

	while (!(bt < 0))	{
		i++;
		bt = (bt << 8) + 0377;
		if (i == MAXSIZE)
			fatal("array full_mask too small for this machine");
		full_mask[i] = bt;
		max_int[i] = bt & ~(1L << ((i << 3) - 1));
		min_int[i] = - max_int[i];
		if (! options['s']) min_int[i]--;
	}
	mach_long_size = i;
	mach_long_sign = 1L << (mach_long_size * 8 - 1);
	if (long_size > mach_long_size) {
		fatal("sizeof (long) insufficient on this machine");
	}

	wrd_bits = 8 * (int) word_size;
}
