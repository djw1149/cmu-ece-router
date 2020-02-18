#include "../../h/rtypes.h"
#include "../../h/proto.h"
#include "../mch/mch.h"



#include "../../h/queue.h"
#include "../../h/packet.h"

#include "../mch/dma.h"
#include "../mch/intr.h"
#include "../dev/trreg.h"
#include "../dev/tr.h"
#include "../mch/cable.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/tr.h"

#if	C_TR > 0


#include "../../h/ip.h"



extern int trreset();
extern int tr_output();
extern struct addmap *ar_map();
extern struct probe probes[];

struct config trconfig =
{
    trreset,			/* reset routine */
    tr_output,			/* output routine */
    0,				/* remote diagnostics */
    ar_map,			/* address resolution routine */
    {				/* output packet types per protocol: */
	TRT_FAKEIP,		/* -Internet */
		/* BUG This is the IP type within ARP */
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	TRT_AR,			/* -Address Resolution */
	0,			/* -Echo */
	0,			/* -Configuration Testing */
    },
    TR_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    TRHEADLEN,			/* packet receive header length (bytes) */
    TRCRCLEN,			/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    TR_MTU,			/* maximum transmission unit (data bytes) */
    TR_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	0xff,0xff,0xff,
	0xff,0xff,0xff
    },
    DVT_TR,			/* generic device type */
    sizeof (struct trdevdep),	/* device dependent fields */
};

/* No Pi foolishness */
char tr_pi[] =
{
    PI_END
};


struct autoid trautoid[] =
{
    {
	(p_pdev)0x1C0,  0,  /* Really far pointer, but into the I/O space*/
	tr_pi,
	0, 2
    },

    {
	(p_pdev)0x140,  0,	/* Ditto */
	tr_pi,
	0, 3
    },

    0
};

extern Itrprobe(), Itr(), tr_input();
extern trparty();
struct autoconf trautoconf =
{
    Itrprobe,			/* device probe routine */
    Itr,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt vectors: */
	0,			/* No auto configured addresses */
    },
    tr_input,			/* packet input process routine */
    sizeof(short),		/* device register area length, is too big*/
    trautoid,			/* autoconfiguration identification */
    &trconfig,			/* driver configuration parameters */
    {"tr"},			/* device mnemonic */
};


/*
 *  Itrprobe - test/probe for an IBM Token Ring adapter
 *
 *  tr   = candidate CSR address
 *  flag = probe flags
 *
 * Since multiple token ring cards shair an interupt party line, 
 *	interupts are not probed here.   Rather, they are explicitly
 *	connected in trI();
 *
 *
 */
/* numtr keeps track of the number of tr adapters configured and trdvtab
 * their device structures.   This is used by the partyline interupt handler
 * to poll the adapters to determine which caused the interupt.
 *
 * They are set in trI as the adapters are configured.
 */
int numtr = 0;
struct device *trdvtab[MAXTR];

Itrprobe(ai, flag)
register struct autoid *ai;
{
    register struct trdevice far *reg = (struct trdevice far *) ai->ai_csr;

    switch (flag)
    {
	case PB_CHECK:	/* Is the device really here? */
		/* Try to run the bringup diagnostic. If it succeeds...*/
		if (!tr_bringup(reg)) return(0); /* Play with this later */

/* if ( (int) reg != 0x1C0 ) return(0);		/* lazy.... */

	    break;

	case PB_RESET:	/* disarm the device */
		/* nothing to do */
	    break;

	case PB_PROBE:	/* force an interupt */
		/* nothing to do */
	    break;
    }
    return(TRUE);
}

/* Interrupt and software queue initalization.
 *  The Token ring can take tens of seconds to initalize.  Rather than doing
 *  here, do it in trreset.
 */

Itr(dv)	
register struct device *dv;
{
    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    extern u_int TRPOFF;

    if (!numtr){	/* connect the partyline to the handler once */
    	setvector(&probes[2], trparty, 0, 0);
/* note that we are spl7() here: Interupts are still off at the first ctrlr */
	EOI12;	/* Set up the second interupt controller */
	ARM12;

    }
    DIOR(tr_disable);	/* leave them off at the adapter */

    trdvtab[numtr++]=dv;

    TRPOFF = POFF-TRHEADLEN;

}
#endif C_TR
