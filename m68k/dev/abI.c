/*
 *
 * David Waitzman (djw@maxwell.ee.cmu.edu) Jun-Jul '85 for cmu-ece department
 * from sumex seagate code and they got it from apple computer corp.
 * they have copyrights , and forbid commercial use.
 *
 */


#define kprintf	cprintf
#include "cond/ab.h"

#if	C_AB > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../dev/abreg.h"
#include "../dev/ab.h"
#include "../mch/device.h"
/* #include "../h/psw.h" */
#include "../mch/dma.h"
#include "../../h/ip.h"
#include "../../h/aconf.h"
/* #include "../h/cable.h" 	*unknown	*/

extern int abreset();
extern int ddpip_output();
extern struct addmap *ar_map();

int ab_foo[4]; /* for abreg.h not to fail */

struct scc_init_tableT {
    u_char val, reg;
} scc_init_table[] =
{
	{RESETA,9}, 	/* (0x80) reset port a  */
	{SDLCMODE,4}, 	/* (0x20) sdlc mode  */
	{SETFM0,10}, 	/* (0xe0) set fm0, preset crc  */
	{0x00,ADDRREG}  /* (0x00) address (set to 0 initially) */
	{SDLC,7}, 	/* (0x7e) sdlc flag byte  */
	{CLOCKS,11}, 	/* (0x70) dpll receive, baud rate gen xmit  */
	{BAUDRATE,BDRATELO}, 	/* (0x06) baud rate generator value  */
	{0,BDRATEHI}, 	/* (0x00)  */
	{FMMODE,14}, 	/* (0xc0) set fm mode  */
	{SRCHMODE,14}, 	/* (0x23) start pll, set brg src (brg enabled)  */
	{ENBRXSLV,3}, 	/* (0xdd) enable receiver, address match mode  */
	{DISTXRTS,5}, 	/* (0x60) disable transmitter  */
	{0x00,2}, 	/* (0x00) set zeros in interrupt vector  */
	{0,15}, 	/* (0x00) no external interrupts  */
	{RXIE,1}, 	/* (0x08) interrupt on first char  */
	{0,0}	 	/* *** end of table ***  */
};

IabinitSCC(reg)
register struct abdevice *reg;
{
    register struct scc_init_tableT *tptr = scc_init_table;
    u_char i;
    
    i = reg->cntlreg;
    i++;
    while (tptr->reg) {
	WRITE_REG (reg, tptr->reg, tptr->val);
	tptr++;
    }
}

extern struct addmap *ar_map();

struct config abconfig =
{
    abreset,			/* reset routine */
    ddpip_output,		/* output routine */
    0,				/* diagnosis routine */
    ar_map,  			/* address resolution routine */
    {				/* output packet types per protocol: */
	DDPT_IP,
	0,			/* -CHAOSnet */
    	0,			/* -PUP */
	DDPT_AR, 		/* -Address Resolution */
	0,			/* -Echo */
	0,			/* -Configuration Testing */
    },
    AB_HRD,			/* address resolution hardware space */
    (				/* driver dependent flags: */
	0			/* (none) */
    ),
    ABHEAD,			/* packet receive header length (bytes) */
    0,  			/* packet trailer length (bytes) */
    0,				/* packet buffer slop (bytes) */
    AB_MTU,			/* maximum transmission unit (data bytes) */
    AB_HLN,			/* hardware address length (bytes) */
    {				/* hardware broadcast address */
	0,0,0xff,0
    },
    DVT_APBUS,			/* generic device type */
    sizeof (struct abdevdep),	/* device dependent fields */
};


extern Iabprobe(), Iab(), ab_input(), abrintr();

struct autoconf abautoconf =
{
    Iabprobe,			/* device probe routine */
    Iab,			/* device (hardware) initialization routine */
    0,				/* driver (software) initialization routine */
    0,				/* device reset (non-net only) */
    {				/* interrupt vectors: */
	abrintr,		/* -general */
    },
    ab_input,			/* packet input process routine */
    sizeof (struct abdevice),	/* device register area length */
    &abconfig,			/* driver configuration parameters */
    {"ab"},			/* device mnemonic */
};


/*
 *  Iabprobe - test/probe for AppleBus device
 *
 *  ai   = candidate CSR address
 *  flag = probe flags
 *
 * ?djw? this whole routine is in question
 *
 *  To probe for the interrupt vector and br level, issue an OFFLINE command.
 */

Iabprobe(ai, flag)
register struct abdevice **ai;
{
    register struct abdevice *reg = *ai;
    int delay;

    if(NXM_check(ai->ai_csr, sizeof(struct abdevice))
	return(0);
    switch (flag)
    {
	case PB_CHECK:			/* check for right address */
		SYNC_REG(reg);
		WRITE_REG(reg,9,0300);	/* this does a hardware reset */
		/* we have to wait a long while */
		for(delay=0; delay < 20; delay++);
		return( READ_REG(reg, 15) == 248);

	case PB_RESET:			/* reset device from interuppting */
		/* no interuppt when done transmitting */
		WRITE_REG(reg, 1, RXIE);
		WRITE_REG(reg, 5, DISTXRTS);
		WRITE_REG(reg, 15, 0);
	    	break;

	case PB_PROBE:			/* generate an interupt */
		IabinitSCC(reg);
		SCC_ENABRINT(reg);
		/* interuppt when done transmitting */
		WRITE_REG(reg, 1, RXIE+0x02);
 		WRITE_REG(reg, 14, RESETCLKS);
		WRITE_REG(reg, 15, 0xff);	
		WRITE_REG(reg, 5, ENBRTS);
		WRITE_REG(reg, 5, DISTXRTS);
		WRITE_REG(reg, 5, ENBTXRTS);    /* enable transmitters */
		WRITE_REG(reg, 3, DISRX);
		WRITE_REG(reg, 0, RESTXCRC);
		WRITE_REG(reg, 8, '!');		/* write out a byte */
		break;
    }
    return(TRUE);
}

Iab(dv, ai)
register struct device *dv;
{
    addrblock meblk;
    dv->dv_rp = palloc();
    if (dv->dv_rp == 0)
    	panic("Iab rp");
    /* reinitialize the SCC after the junk that PB_PROBE pulled. */
    IabinitSCC((struct abdevice *)dv->dv_addr);
    abgetnaddr(dv);
    meblk.net = MyABNet;
    meblk.soc = ddpIPSkt;
    meblk.node = dv->dv_ab.mynaddr;
    bcopy(&meblk, dv->dv_phys, sizeof(addrblock));
}

#endif C_AB

