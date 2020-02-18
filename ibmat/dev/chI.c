#include "../../h/rtypes.h"
#include "../../h/proto.h"
#include "../../h/queue.h"
#include "../../h/packet.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/chreg.h"
#include "../dev/ch.h"
#include "../mch/cable.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/ch.h"

#if	C_CH > 0


#include "../../h/ip.h"

extern int chreset();
extern int ch_output();
extern struct addmap *ar_map();

struct config chconfig =
{
    chreset,			/* reset routine */
    ch_output,			/* output routine */
    0,				/* remote diagnostics */
    ar_map,			/* address resolution routine */
    {				/* output packet types per protocol: */
	CHT_IP,			/* -Internet */
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	CHT_AR,			/* -Address Resolution */
	0,			/* -Echo */
	CHT_CTP,		/* -Configuration Testing */
    },
    CH_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    CHHEADLEN,			/* packet receive header length (bytes) */
    CHCRCLEN,			/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    CH_MTU,			/* maximum transmission unit (data bytes) */
    CH_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	0xff,0xff,0xff,
	0xff,0xff,0xff
    },
    DVT_CH,			/* generic device type */
    sizeof (struct chdevdep),	/* device dependent fields */
};

/* No Pi foolishness */
char ch_pi[] =
{
    PI_END
};


struct autoid chautoid[] =
{
    {
	(p_pdev)0x80000000,  0,	/* Really far pointer 8000:0000 = 0x80000 */
	ch_pi,
	0, 2
    },

    {
	(p_pdev)0x88000000,  0,	/* Really far pointer 8800:0000 = 0x88000 */
	ch_pi,
	0, 3
    },

    {
	(p_pdev)0xC8000000,  0,	/* Really far pointer C800:0000 = 0xC8000 */
	ch_pi,
	0, 4
    },

    0
};

extern Ichprobe(), Ich(), ch_input();
extern chintr();

struct autoconf chautoconf =
{
    Ichprobe,			/* device probe routine */
    Ich,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt vectors: */
	chintr,			/* -general */
    },
    ch_input,			/* packet input process routine */
    sizeof(short),		/* device register area length, is too big*/
    chautoid,			/* autoconfiguration identification */
    &chconfig,			/* driver configuration parameters */
    {"ch"},			/* device mnemonic */
};


/*
 *  Ichprobe - test/probe for Interlan 10Mb ethernet device
 *
 *  ch   = candidate CSR address
 *  flag = probe flags
 *
 *  To check for device, verify that CSR is not all ones (there are
 *  configurations with uncompletely used ROM's overlapping the candidate
 *  address).
 *
 *  To reset the device, simply clear the command done bit which may have been
 *  set by the previous attempt to generate an interrupt.
 *
 */

Ichprobe(ai, flag)
register struct autoid *ai;
{
    register struct chdevice far *reg = (struct chdevice far *) ai->ai_csr;

    u_char i;	/* for testing */

    switch (flag)
    {
	case PB_CHECK:	/* Is the device really here? */
		/* This is a crock since the HW does not report bus errors */
		/* We poke around, and guess */
		i = reg->eprom[PADDR];		/*  constant physical addr */
		if ( i==0xFF || i != reg->eprom[PADDR]) return(FALSE);
		reg->eprom[PADDR]= ~i;
		if (i != reg->eprom[PADDR] ) {
		    reg->eprom[PADDR] = i;		/* Pray.... */
		    return(FALSE);
		}
		
	    break;

	case PB_RESET:	/* disarm the device */
		reg->csr = 0;		/* No interupts */
		reg->fpp &= ~ARM;
	    break;

	case PB_PROBE:	/* force an interupt */
		reg->fpp = 0;
		reg->csr = 0;		/* disable interupts */
		reg->fpp |= ARM;	/* read&write fpp/epp */
		reg->csr = SFTI;		/* make it interupt */
	    break;
    }
    return(TRUE);
}

/* Hardware initialization
 *	(Also sometimes the software)
 */

Ich(dv)	
register struct device *dv;
{
    register struct chdevice far *reg = (struct chdevice far *)dv->dv_addr;
    extern u_int CHPOFF;

    copin(dv->dv_phys, &reg->eprom[PADDR], CH_HLN);	/* set the physical */

    reg->csr = 0;		/* disable any old interupts */
    reg->rst = BEGINRESET;	/* reset EDLC */
    reg->xcsr = CLRXCSR;
    reg->ximask = 0;
    reg->rcsr = CLRRCSR;
    reg->rimask = 0;
    reg->xmode = XIGNP|LBC;	/* transmit is off line */
    reg->rmode= 0;		/* not on line yet */
    copout(reg->addr, dv->dv_phys, CH_HLN);		/* network address */
    reg->rst = ENDRESET;	/* should be all squeaky clean now */

    CHPOFF = POFF-CHHEADLEN;
    dv->dv_rp = palloc();
    if (dv->dv_rp == 0)
	panic("Ich rp");

}
#endif C_CH
