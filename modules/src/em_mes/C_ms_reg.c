#include <em.h>
#include <em_mes.h>

C_ms_reg(offs, siz, class, prior)
	arith offs, siz;
	int class, prior;
{
	C_mes_begin(ms_reg);
	C_cst(offs);
	C_cst(siz);
	C_cst((arith)class);
	C_cst((arith)prior);
	C_mes_end();
}
