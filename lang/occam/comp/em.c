/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include "sizes.h"
#include "Lpars.h"
#include <em.h>
#include "em.h"

/* This file is used to shield code.c as much as possible from em dependant
 * details.  It introduces some call overhead but not enough for a coffee
 * break. (Sorry)
 * Note that functions with a leading upper case letter normally decide between
 * word or double word arith.
 */

int wz = 4, pz = 4, vz = 4;
int Lflag;
static Lab=0;
char *malloc();

void init()
{
	C_init((arith) wz, (arith) pz);
}

void openfile(file) char *file;
{
	if (C_open(file) < 0) {
		fatal("Could not open output file");
	}
}

void meswp()
{
	C_ms_emx((arith) wz, (arith) pz);
}

void maxdes()
{
	C_df_dnam("maxcdes");
	rom(wz, 0L); rom(wz, -1L); rom(wz, (long) (wz+pz));
	C_df_dnam("maxwdes");
	rom(wz, 0L); rom(wz, -1L); rom(wz, (long) vz);
	C_df_dnam("maxbdes");
	rom(wz, 0L); rom(wz, -1L); rom(wz, 1L);
}

int new_label(L) register *L;
{
	if (*L==0) *L= ++Lab;
	return *L;
}

void Label(L) register L;
{
	if (L!=0) C_df_ilb((label) L);
}

static Dot_label=0;

int new_dot_label(L) int *L;
{
	return *L= ++Dot_label;
}

void dot_label(L) int L;
{
	C_df_dlb((label) L);
}

void branch(L) int *L;
{
	C_bra((label) new_label(L));
}

char *proc_label(L, name) register L; register char *name;
{
	static char *lab=nil;
	register char *n;

	if (lab!=nil) free(lab);

	lab=malloc(strlen(name)+(1+sizeof(int)*3+1));
		/* That is: P<L><name>\0 */

	sprint(lab, "P%d", L);

	n=lab+strlen(lab);

	while (*name!=0) {
		*n++ = *name=='.' ? '_' : *name;
		name++;
	}
	*n=0;
	return lab;
}

void magic()	/* magic? should be called invisible */
{
	C_magic();
}

void cwv()
{
	if (vz>wz) {
		C_loc((arith) wz);
		C_loc((arith) vz);
		C_cii();
	}
}

void cvw()
{
	if (vz>wz) {
		C_loc((arith) vz);
		C_loc((arith) wz);
		C_cii();
	}
}

void Loc(const) long const;
{
	if (vz>wz) C_ldc((arith) const); else C_loc((arith) const);
}

void Lol(offset) int offset;
{
	if (vz>wz) C_ldl((arith) offset); else C_lol((arith) offset);
}

void Lolp(offset) int offset;
{
	if (pz>wz) C_ldl((arith) offset); else C_lol((arith) offset);
}

void Lil(offset) register offset;
{
	if (vz>wz) {
		Lolp(offset);
		C_loi((arith) vz);
	} else
		C_lil((arith) offset);
}

void Lof(offset) int offset;
{
	if (vz>wz) C_ldf((arith) offset); else C_lof((arith) offset);
}

void Lofp(offset) int offset;
{
	if (pz>wz) C_ldf((arith) offset); else C_lof((arith) offset);
}

void Lif(offset) register offset;
{
	Lofp(offset);
	C_loi((arith) vz);
}

void Stl(offset) int offset;
{
	if (vz>wz) C_sdl((arith) offset); else C_stl((arith) offset);
}

void Inl(offset) register offset;
{
	if (vz>wz) {
		C_ldl((arith) offset);
		C_ldc((arith) 1);
		C_adi((arith) vz);
		C_sdl((arith) offset);
	} else
		C_inl((arith) offset);
}

void Del(offset) register offset;
{
	if (vz>wz) {
		C_ldl((arith) offset);
		C_ldc((arith) 1);
		C_sbi((arith) vz);
		C_sdl((arith) offset);
	} else
		C_del((arith) offset);
}

void Loe(name, offset) char *name; int offset;
{
	if (vz>wz)
		C_lde_dnam(name, (arith) offset);
	else
		C_loe_dnam(name, (arith) offset);
}

typedef int (*pfi)();

static int operators[]=	{ '<', '>', '=',  GE,  LE,  NE };

extern C_blt(), C_bgt(), C_beq(), C_bge(), C_ble(), C_bne();
extern C_tlt(), C_tgt(), C_teq(), C_tge(), C_tle(), C_tne();
extern C_zlt(), C_zgt(), C_zeq(), C_zge(), C_zle(), C_zne();

static pfi C_bxx[]= { C_blt, C_bgt, C_beq, C_bge, C_ble, C_bne };
static pfi C_txx[]= { C_tlt, C_tgt, C_teq, C_tge, C_tle, C_tne };
static pfi C_zxx[]= { C_zlt, C_zgt, C_zeq, C_zge, C_zle, C_zne };

void bxx(pos, op, L) register pos, op, L;
{
	register i;

	if (op==AFTER) {
		C_sbi((arith) vz);
		if (vz>wz) {
			C_ldc((arith) 0);
			C_cmi((arith) vz);
		}
		if (pos) C_zle((label) L); else C_zgt((label) L);
	} else {
		for (i=0; operators[i]!=op; i++) ;
		if (pos && (i+=3)>=6) i-=6;
		if (vz>wz) {
			C_cmi((arith) vz);
			(C_zxx[i])((label) L);
		} else {
			(C_bxx[i])((label) L);
		}
	}
}

void Txx(op) register int op;
{
	register i;

	for (i=0; operators[i]!=op; i++) ;

	(C_txx[i])();
	cwv();
	C_ngi((arith) vz);
}

extern C_adi(), C_sbi(), C_mli(), C_dvi(), C_rmi();

void xxi(op) register op;
{
	static int operators[]=	{ '+',   '-',   '*',   '/',   BS };
	static pfi C_xxi[]=	{ C_adi, C_sbi, C_mli, C_dvi, C_rmi };
	register i;

	for (i=0; operators[i]!=op; i++) ;

	(C_xxi[i])((arith) vz);
}

void aar()			{	C_aar((arith) wz); }
void adp(offset) int offset;	{	C_adp((arith) offset); }
void and()			{	C_and((arith) vz); }
void asp(size) int size;	{	C_asp((arith) size); }
void blm(size) int size;	{	C_blm((arith) size); }
void blt(lab) int lab;		{	C_blt((label) lab); }
void cal(lab) char *lab;	{	C_cal(lab); }
void cmi()			{	C_cmi((arith) vz); }
void com()			{	C_com((arith) vz); }
void del(offset) int offset;	{	C_del((arith) offset); }
void _end(size) int size;	{	C_end((arith) size); }
void exp(lab) char *lab;	{	C_exp(lab); }
void ior()			{	C_ior((arith) vz); }
void lae(lab, offset) char *lab; int offset;
				{	C_lae_dnam(lab, (arith) offset); }
void laedot(lab) int lab;	{	C_lae_dlb((label) lab, (arith) 0); }
void lal(offset) int offset;	{	C_lal((arith) offset); }
void lar()			{	C_lar((arith) wz); }
void ldc0()			{	C_ldc((arith) 0); }
void ldl(offset) int offset;	{	C_ldl((arith) offset); }
void lfr(size) int size;	{	C_lfr((arith) size); }
void loc(cst) int cst;		{	C_loc((arith) cst); }
void loi(size) int size;	{	C_loi((arith) size); }
void lol(offset) int offset;	{	C_lol((arith) offset); }
void lor0()			{	C_lor((arith) 0); }
void lxa(offset) int offset;	{	C_lxa((arith) offset); }
void lxl(offset) int offset;	{	C_lxl((arith) offset); }
void meserr()			{	C_ms_err(); }
void ngi()			{	C_ngi((arith) vz); }
void pro(lab) char *lab;	{	C_pro_narg(lab); }
void ret(size) int size;	{	C_ret((arith) size); }
void init_rt()			{	C_cal("init");	}
void sli()			{	C_sli((arith) vz); }
void sri()			{	C_sri((arith) vz); }
void ste(lab, offset) char *lab; int offset;
				{	C_ste_dnam(lab, (arith) offset); }
void sti(size) int size;	{	C_sti((arith) size); }
void stl(offset) int offset;	{	C_stl((arith) offset); }
void trp()			{	C_trp(); }
void tst()			{	/* No flags in EM */ }
void xor()			{	C_xor((arith) vz); }
void zeq(lab) int lab;		{	C_zeq((label) lab); }
void zgt(lab) int lab;		{	C_zgt((label) lab); }
void zlt(lab) int lab;		{	C_zlt((label) lab); }
void zne(lab) int lab;		{	C_zne((label) lab); }

char *itoa(i) long i;
{
	static char a[sizeof(long)*3];
	sprint(a, "%ld", i);
	return a;
}

void rom(size, c) int size; long c;
{
	C_rom_icon(itoa(c), (arith) size);
}

void lin()
{
	static oldline=0;
	extern yylineno;

	if (Lflag) return;
	if (yylineno!=oldline)
		C_lin((arith) (oldline=yylineno));
}

static struct ftree {
	char *file;
	int lab;
	struct ftree *left, *right;
} std_f = { "stdin", 0, nil, nil }, *curr_f= &std_f, *main_f=nil;

char *curr_file="stdin";

static void do_fil(f) struct ftree *f;
{
	if (Lflag) return;
	if (f->lab==0) {
		dot_label(new_dot_label(&f->lab));
		C_rom_scon(f->file, (arith) (strlen(f->file)+1));
	}
	C_fil_dlb((label) f->lab);
}

void fil()
{
	do_fil(curr_f);
}

void main_fil()
{
	do_fil(main_f==nil ? &std_f : main_f);
}

int set_file(f) char *f;
{
	char *strcpy();
	static struct ftree *ftop=nil;
	register struct ftree *pf, **apf= &ftop;
	register cmp;

	while ((pf= *apf)!=nil && (cmp=strcmp(f, pf->file))!=0)
		apf= cmp<0 ? &pf->left : &pf->right;

	if (pf==nil) {
		*apf= pf= (struct ftree *) malloc(sizeof *pf);
		pf->file=strcpy(malloc(strlen(f)+1), f);
		pf->lab=0;
		pf->left=pf->right=nil;
	}
	curr_f=pf;
	curr_file=pf->file;
	if (main_f==nil) {
		main_f=curr_f;
		return 0;
	} else
		return curr_f!=main_f;
}

void par_begin()
{
	C_lal((arith) curr_offset);
	C_cal("pc_begin");
	C_asp((arith) pz);
}

void par_fork(NONZERO) int *NONZERO;
{
	C_cal("pc_fork");
	C_lfr((arith) wz);
	C_zne((label) new_label(NONZERO));
}

void resumenext()
{
	C_cal("resumene");
}

void no_deadlock()
{
	C_zre_dnam("deadlock", (arith) 0);
}

void par_end()
{
	C_cal("parend");
}

void closefile()
{
	C_close();
}
