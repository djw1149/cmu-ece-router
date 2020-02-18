/*
 * pronet initialization module
 * $Header: vvI.c,v 1.4.1.3 86/10/22 13:20:07 djw Exp $
 */
#include "cond/vv.h"

#if	C_VV > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/vvreg.h"
#include "../dev/vv.h"
#include "../mch/device.h"
#include "../../h/aconf.h"
#include "../mch/dma.h"
#include "../../h/ip.h"

extern int vvreset();
extern int vv_output();
extern struct addmap *ar_map();

struct config vvconfig =
{
    vvreset,			/* reset routine */
    vv_output,			/* output routine */
    0,				/* diagnosis routine */
    ar_map,			/* address resolution routine */
    {				/* output packet types per protocol: */
	VVT_IP,			/* -Internet */
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	VVT_AR,			/* -Address Resolution */
	0,			/* -Echo */
	0,
    },
    VV_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    VVHEAD,			/* packet receive header length (bytes) */
    0,				/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    VV_MTU,			/* maximum transmission unit (data bytes) */
    VV_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	VV_BROADCAST,
    },
    DVT_PRONET,			/* generic device type */
    sizeof (struct vvdevdep),	/* device dependent fields */
};



extern Ivvprobe(), Ivv(), vv_input();
extern vvintr();

struct autoconf vvautoconf =
{
    Ivvprobe,			/* device probe routine */
    Ivv,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt vectors: */
	vvintr,			/* -general */
    },
    vv_input,			/* packet input process routine */
    sizeof (struct vvdevice),	/* device register area length */
    &vvconfig,			/* driver configuration parameters */
    {"vv"},			/* device mnemonic */
};


/*
 *  Ivvprobe - test/probe for Pronet device
 *
 *  vv   = candidate CSR address
 *  flag = probe flags
 */

Ivvprobe(ai, flag)
register struct vvdevice **ai;
{
    register struct vvdevice *reg = *ai;

    if(NXM_check(reg, sizeof(struct vvdevice)))
	return(0);
    switch (flag)
    {
	case PB_CHECK:	/* check, reset board */
		reg->vv_rcsr = VV_RESET|VV_HOST;
		reg->vv_xcsr = VV_RESET|VV_CLEAR;
		break;

	case PB_RESET:	/* reset interrupt */
		reg->vv_rcsr = VV_RESET;
		reg->vv_xcsr = VV_RESET|VV_CLEAR;
		break;

	case PB_PROBE:	/* setup interrupt */
		reg->vv_xba = -1;
		reg->vv_xwc = 2;
		reg->vv_xcsr = VV_DMA|VV_INT;
		break;
    }
    return(TRUE);
}

Ivv (dv, ai)
register struct device *dv;
{
    register struct vvdevice *reg = (struct vvdevice *)dv->dv_addr;
    register struct vvpacket *vp;
    register struct packet *p;
    register waitcount, attempts = 0;
    extern short VVRWC;

    VVRWC = VVPLEN >> 1;
    dv->dv_rp = palloc();
    if (dv->dv_rp == 0)
    	panic("Ivv rp");

    /*
     * Create the packet that we are going to transmit
     */
    p = dv->dv_rp;
    p->p_off = 0;
    p->p_len = VVHEAD + sizeof(short);
    vp = poff(vvpacket,p);
    vp->vv_dhost = VV_BROADCAST;
    vp->vv_shost = 0;
    vp->vv_version = VV_VERSION;
    vp->vv_type = VVT_IP;
    vp->vv_inf = 0;

    /*
     * Set up the input side of the operation
     */
retry:
    reg->vv_rcsr = VV_RESET;
    reg->vv_rba = (u_long) swal(&(p->p_ba)[p->p_off]);
    reg->vv_rwc = (u_short) swab(VVRWC);
    reg->vv_rcsr = VV_STEST | VV_DLOOP | VV_DMA | VV_COPY;

    DELAY(100000);

    reg->vv_xcsr = VV_RESET | VV_CLEAR;
    reg->vv_xba = (u_long) swal(&(p->p_ba)[p->p_off]);
    reg->vv_xwc = swab((p->p_len+1) >> 1);
    reg->vv_xcsr = VV_DMA | VV_INITR | VV_ORIG | VV_CLEAR;

    DELAY(10000);

    for(waitcount = 0; (reg->vv_rcsr&VV_RDY) == 0; waitcount++) {
	if (waitcount < 10) {
	    DELAY(2000);
	    continue;
	}
	if (attempts++ >= 10) {
	    cprintf("\r\nvv: can not initialize vv_rcsr=%o vv_xcsr=%o\r\n",
		reg->vv_rcsr,reg->vv_xcsr);
	    panic("Ivv: addr");
	    return;
	}
	goto retry;
    }

    dv->dv_phys[0] = (char) vp->vv_shost;
    if (dv->dv_phys[0] == 0)
	panic("Ivv: 0 addr");
}
#endif C_VV
