/*
 *  programmable timer module
 *  $Header: ptmI.c,v 1.2.1.3 86/10/22 13:19:51 djw Exp $
 **********************************************************************
 * HISTORY
 * 10-Dec-84  Gregg Lebovitz (gregg) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/time.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ptmreg.h"
#include "../mch/device.h"
#include "../../h/aconf.h"



/*
 *  Autoconfiguration driver paramaters
 */

extern Iptmprobe();
extern ptmintr();
extern tm_ring();
extern ptmreset();

struct autoconf ptmautoconf =
{
    Iptmprobe,		/* device probe routine */
    0,			/* device (hardware) initialization routine */
    0,			/* driver (software) initialization routine */
    ptmreset,		/* device reset routine */
    {			/* interrupt vectors: */
	ptmintr,		/* -transmit character done */
    },
    tm_ring,
    sizeof (struct ptmdevice),
    0,			/* driver configuration parameters */
    {"ptm"},		/* device mnemonic */
};

/*
 * nprobe keeps us from reseting the acia if the we are
 * probing the console. Otherwise we loose the last char or so
 */
static int nprobe = 0;
Iptmprobe(ai, flag)
register long **ai;
{
    register struct ptmdevice *ptm = (struct ptmdevice *)*ai;
    register int i;

    if(NXM_check(ptm ,sizeof(struct ptmdevice)))
	return(0);
    switch (flag)
    {
	case PB_CHECK:
	    break;

	case PB_RESET:
	    ptm->ptm_csr2 = PTM_REG1;
	    ptm->ptm_csr1 = PTM_DISABLE;
	    i = ptm->ptm_csr2;
	    i = ptm->ptm_msb1;
	    break;

	case PB_PROBE:
	    ptm->ptm_csr2 = PTM_REG1;
	    ptm->ptm_msb1 = 0;
	    ptm->ptm_cnt1 = 1;
	    ptm->ptm_csr1 = PTM_CONT|PTM_NORESET|PTM_IRQENA|PTM_INTERN;
	    break;
    }
    return(TRUE);
}
