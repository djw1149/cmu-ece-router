/*
 *  tty init module
 *
 **********************************************************************
 * HISTORY
 * 10-Dec-84  Gregg Lebovitz (gregg) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ttreg.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"


/*
 *  Autoconfiguration identification parameters
 */


struct autoid ttautoid[] =
{
    {
	(char *)0xFFFF01,  0,
	0, 
    },
    0
};



/*
 *  Autoconfiguration driver paramaters
 */

extern Ittprobe(), Itt();
extern ttintr();
extern ttreset();
extern tt_input();

struct autoconf ttautoconf =
{
    Ittprobe,		/* device probe routine */
    Itt,		/* device (hardware) initialization routine */
    0,			/* driver (software) initialization routine */
    ttreset,		/* device reset routine */
    {			/* interrupt vectors: */
	0 /*ttintr*/,	/* -transmit character done */
    },
    tt_input,
    sizeof (struct ttdevice),
    ttautoid,		/* autoconfiguration identification */
    0,			/* driver configuration parameters */
    {"tt"},		/* device mnemonic */
};

/*
 * nprobe keeps us from reseting the acia if the we are
 * probing the console. Otherwise we loose the last char or so
 */
static int nprobe = 0;
Ittprobe(ai, flag)
register struct autoid *ai;
{
    register struct ttdevice *tt = (struct ttdevice *)ai->ai_csr;
    register int i;
    switch (flag)
    {
	case PB_CHECK:
	    break;

	case PB_RESET:
	    break;

	case PB_PROBE:
	    break;
    }
    return(TRUE);
}


Itt(dv)
register struct device *dv;
{
#ifndef IBMAT
    extern char *ttbuff;
    extern char *ttend;
    extern char *tthead;
    extern char *tttail;
    extern int ttchar;
    extern struct device *ttdv;

    ttbuff = (char *)malloc(NTTCHAR);
    if (ttbuff == 0)
	panic("Itt");
    ttend = ttbuff + NTTCHAR;
    tthead = ttbuff;
    tttail = ttbuff;
    ttchar = 0;
    ttdv = dv;
#endif IBMAT
}
