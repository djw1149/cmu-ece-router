/*
 * Interlan init module
 * $Header: ilI.c,v 1.3.1.3 86/10/22 13:19:26 djw Exp $ 
 */
#include "cond/il.h"

#if	C_IL > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ilreg.h"
#include "../dev/il.h"
#include "../mch/cable.h" 
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/aconf.h"
#include "../mch/dma.h"
#include "../../h/ip.h"



/* #include "../h/psw.h" */

extern int ilreset();
extern int il_output();
extern int ildiag();
extern struct addmap *ar_map();

struct config ilconfig =
{
    ilreset,			/* reset routine */
    il_output,			/* output routine */
    ildiag,			/* diagnosis routine */
    ar_map,			/* address resolution routine */
    {				/* output packet types per protocol: */
	ILT_IP,			/* -Internet */
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	ILT_AR,			/* -Address Resolution */
	0,			/* -Echo */
	ILT_CTP,		/* -Configuration Testing */
    },
    IL_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    ILHEADLEN,			/* packet receive header length (bytes) */
    ILCRCLEN,			/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    IL_MTU,			/* maximum transmission unit (data bytes) */
    IL_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	0xff,0xff,0xff,
	0xff,0xff,0xff
    },
    DVT_IL10,			/* generic device type */
    sizeof (struct ildevdep),	/* device dependent fields */
};


extern Iilprobe(), Iil(), il_input();
extern ilintr();

struct autoconf ilautoconf =
{
    Iilprobe,			/* device probe routine */
    Iil,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt viltors: */
	ilintr,			/* -general */
    },
    il_input,			/* packet input process routine */
    sizeof (struct ildevice),	/* device register area length */
    &ilconfig,			/* driver configuration parameters */
    {"il"},			/* device mnemonic */
};


/*
 *  Iilprobe - test/probe for Interlan 10Mb ethernet device
 *
 *  il   = candidate CSR address
 *  flag = probe flags
 *
 *  To check for device, verify that CSR is not all ones (there are
 *  configurations with uncompletely used ROM's overlapping the candidate
 *  address).
 *
 *  To reset the device, simply clear the command done bit which may have been
 *  set by the previous attempt to generate an interrupt.
 *
 *  To probe for the interrupt vector and br level, issue an OFFLINE command.
 */

Iilprobe(ai, flag)
register struct ildevt *ai;
{
    register struct ildevice *reg = ai->baseaddr;
    int dum[2];

    if(NXM_check(reg, sizeof(struct ildevice)))
	return(0);
    switch (flag)
    {
	case PB_CHECK:
            cprintf("il @%lx shared memory @%lx ", reg, ai->smaddr);
	    if(NXM_check(ai->smaddr, ILMEMSZ)) {
		cprintf("not configured properly\r\n",
			reg, ai->smaddr);
		return(0);
	    }
	    cprintf("ok\r\n");
	    break;

	case PB_RESET:
	    reg->csr_dmadone = 0;
	    break;

	case PB_PROBE:
	    reg->csr_dmaiv = ai->priority;
	    reg->csr_iv = ai->priority;
	    reg->csr_swp = 1;
	    reg->csr_dmaie = 1;
	    reg->csr_enb = 1;
	    reg->csr_dmamem = swal(dum);
	    reg->csr_dmacnt = swab(4);
	    reg->csr_dmabuf = 0;
	    reg->csr_dmacmd = DMA_TO_M|DMA_BYTE;
	    reg->csr_dma = 1;
	    break;
    }
    return(TRUE);
}

Iil(dv, ai)
register struct device *dv;
register p_autoid ai;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    extern u_int ILPOFF;
    ILPOFF = POFF-ILHEADLEN;
    dv->dv_rp = palloc();
    if (dv->dv_rp == 0)
	panic("Iil rp");
    dv->dv_il.il_mem = dv->dv_mem = (p_char)*((long *)&(ai->devconf) + 1);
    reg->csr_swp = 1;
    bcopy(reg->csr_haddr, dv->dv_phys, IL_HLN);
}
#endif C_IL
