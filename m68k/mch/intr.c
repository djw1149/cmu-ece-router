/*	intr.c
 * $Header: intr.c,v 1.3.2.1 86/10/22 13:32:25 djw Exp $
 *
 * HISTORY:
 * 2-July-86 	Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University moved contents from autoconfI.c
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"
#include "../../h/aconf.h"


/*
 *  setvector - initialize an interrupt vector
 *
 *  vector = vector address to initialize
 *  func   = address of interrupt routine
 *  psw	   = procesor status word to load on interrupt
 *  arg    = argument to pass to interrupt routine
 *
 *  An interrupt code block is allocated and intialized from the the template.
 *  The routine address and argument are stored in the block.  The interrupt
 *  vector PC is set to point to the block and the vector PS is set to the
 *  specified argument.
 */

setvector(probe, func, arg)
p_inter *probe;
{
    extern int intrf[];			/* first address of template */
    extern int intre[];			/* last address of template */
    register int *ip = intrf;		/* current word of template */
    register int *code;			/* allocated code block */

    code = (int *)malloc(((intre-intrf)+2)*sizeof(int));
    if (code == 0)
	panic("setvector");
    probe->pb_vector->vc_pc = code;
    while (ip < intre)
	*code++ = *ip++;
    *code++ = arg;
    *code = func;
}

