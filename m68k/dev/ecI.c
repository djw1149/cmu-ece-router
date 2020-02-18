/*
 * 3com init module
 * $Header: ecI.c,v 1.3.1.3 86/10/22 13:19:08 djw Exp $ 
 */

#include "cond/ec.h"

#if	C_EC > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ecreg.h"
#include "../dev/ec.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/aconf.h"
#include "../mch/cable.h" 
#include "../mch/dma.h"
#include "../../h/ip.h"

/* #include "../h/psw.h" */

extern int ecreset();
extern int ec_output();
extern int ecdiag();
extern struct addmap *ar_map();

struct config ecconfig =
{
    ecreset,			/* reset routine */
    ec_output,			/* output routine */
    0,				/* diagnosis routine */
    ar_map,			/* address resolution routine */
    {				/* output packet types per protocol: */
	ECT_IP,			/* -Internet */
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	ECT_AR,			/* -Address Resolution */
	0,			/* -Echo */
	ECT_CTP,		/* -Configuration Testing */
    },
    EC_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    ECRHEAD,			/* packet receive header length (bytes) */
    ECRCRCLEN,			/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    EC_MTU,			/* maximum transmission unit (data bytes) */
    EC_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	0xff,0xff,0xff,
	0xff,0xff,0xff
    },
    DVT_3COM10,			/* generic device type */
    sizeof (struct ecdevdep),	/* device dependent fields */
};



extern Iecprobe(), Iec(), ec_input();
extern ecintr();

struct autoconf ecautoconf =
{
    Iecprobe,			/* device probe routine */
    Iec,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt vectors: */
	ecintr,			/* -general */
    },
    ec_input,			/* packet input process routine */
    sizeof (struct ecdevice),	/* device register area length */
    &ecconfig,			/* driver configuration parameters */
    {"ec"},			/* device mnemonic */
};


/*
 *  Iecprobe - test/probe for 3com ethernet device
 *
 *  ec   = candidate CSR address
 *  flag = probe flags
 */

Iecprobe(p_reg, flag)
register struct ecdevice **p_reg;
{
    register struct ecdevice *reg = *p_reg;

    if(NXM_check(reg, sizeof(struct ecdevice)))
	return(0);
    switch (flag)
    {
	case PB_CHECK:
	    *((u_short *)reg) = 0;
	    SET_BIT(reg->ec_csr.csr_reset);	/* reset board */
	    bcopy(reg->ec_arom, reg->ec_aram, EC_HLN);
	    reg->ec_csr.csr_pa = EC_PA_MINE_BROAD_NERR;
	    					/* set packet recep. class */
	    SET_BIT(reg->ec_csr.csr_amsw);	/* set ether address */
	    break;

	case PB_RESET:
	    CLEAR_BIT(reg->ec_csr.csr_tinten);
	    break;

	case PB_PROBE:
	    SET_BIT(reg->ec_csr.csr_tinten);
	    break;
    }
    return(TRUE);
}

Iec(dv, ai)
register struct device *dv;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;

    dv->dv_rp = palloc();
    if (dv->dv_rp == 0)
    	panic("Iec rp");
    bcopy(reg->ec_arom, dv->dv_phys, EC_HLN);
}
#endif C_EC
