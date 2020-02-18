
/*
 *  programmable timer module
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
#include "../h/time.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ptmreg.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"


/*
 *  Autoconfiguration identification parameters
 */


struct autoid ptmautoid[] =
{
    {
	(char *)0xFFFF61,  0,
	0, 
    },
    0
};



/*
 *  Autoconfiguration driver paramaters
 */

extern Iptmprobe(), Iptm();
extern ptmintr();
extern tm_ring();
extern ptmreset();

struct autoconf ptmautoconf =
{
    Iptmprobe,		/* device probe routine */
    Iptm,		/* device (hardware) initialization routine */
    0,			/* driver (software) initialization routine */
    ptmreset,		/* device reset routine */
    {			/* interrupt vectors: */
	ptmintr,		/* -transmit character done */
    },
    tm_ring,
    sizeof (struct ptmdevice),
    ptmautoid,		/* autoconfiguration identification */
    0,			/* driver configuration parameters */
    {"ptm"},		/* device mnemonic */
};



Iptmprobe(ai, flag)
register struct autoid *ai;
{
    register struct ptmdevice *ptm = (struct ptmdevice *)ai->ai_csr;
    register int i;

    switch (flag)
    {
	case PB_CHECK:
	    break;

	case PB_RESET:
	    break;

	case PB_PROBE:
	    Iintr(1);	/* fakeit */
	    break;
    }
    return(TRUE);
}

Iptm(dv)
register struct device *dv;
{
}
