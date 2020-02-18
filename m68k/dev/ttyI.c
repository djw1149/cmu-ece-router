/*
 *  tty init module
 * $Header: ttyI.c,v 1.2.1.3 86/10/22 13:20:02 djw Exp $
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
#include "../../h/aconf.h"



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
	ttintr,		/* -transmit character done */
    },
    tt_input,
    sizeof (struct ttdevice),
/*    ttautoid,		 autoconfiguration identification */
    0,			/* driver configuration parameters */
    {"tt"},		/* device mnemonic */
};

/*
 * nprobe keeps us from reseting the acia if the we are
 * probing the console. Otherwise we loose the last char or so
 */
static int nprobe = 0;
Ittprobe(ai, flag)
register long *ai;
{
    register struct ttdevice *tt = (struct ttdevice *)*ai;
    register int i;

    if(NXM_check(tt, sizeof(struct ttdevice)))
	return(0);
    switch (flag)
    {
	case PB_CHECK:
	    break;

	case PB_RESET:
	    if (tt != TTADDR) {
		tt->tt_csr = TTS_RESET;
		for (i = 0; i < 10000; i++);
	    }
	    tt->tt_csr = TTS_STD;
	    break;

	case PB_PROBE:
	    tt->tt_csr = TTS_STD|TTS_XIE;
	    break;
    }
    return(TRUE);
}

Itt(dv, ai)
register struct device *dv;
{
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
}
