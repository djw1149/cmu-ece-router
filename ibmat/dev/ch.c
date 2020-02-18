/*
 * Chicken Hawk device driver, Matt Mathis Apr 15, 1986
 *
 *
 */
#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/chreg.h"
#include "../dev/ch.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/ch.h"

#if	C_CH > 0

#include "../../h/profile.h"
#include "../../h/globalsw.h"

#include "debug/ch.h"

u_int CHPOFF = 0;

/*
 * chreset(dv)
 *	dv - ptr to device info structure
 *
 * chreset sets up and resets the device memory.  It is called
 * from autoconf code at boot time to do 'hardware initialization'
 * It can also be called from the transmit watchdog if the transmitter wedges.
 */
chreset(dv, flag)
register struct device *dv;
{
    int z;
    register struct chdevice far *reg = (struct chdevice far *)dv->dv_addr;
    int s = spl7();

    reg->csr = 0;		/* disable any old interupts */
    reg->rst = BEGINRESET;	/* reset EDLC */
    reg->xcsr = CLRXCSR;
    reg->ximask = 0;
    reg->rcsr = CLRRCSR;
    reg->rimask = 0;		/* no interupts on raw events */
    reg->xmode= XIGNP|LBC;	/* ignore memory parity, not loop back */
    reg->rmode= RNORM;
    copout(reg->addr, dv->dv_phys, CH_HLN);	/* reassert our address */

/* Note:epp/fpp is read/written to prevent reprocessing past traffic */
    dv->dv_ch.nextpage = reg->epp & ~ ARM; /* agree on which page is next */
    z=reg->clrpav;
    reg->epp = dv->dv_ch.nextpage;	/* No ch interupts for the moment */
    reg->epp = ARM | dv->dv_ch.nextpage; /* fire when ready */
    reg->rst = ENDRESET;	/* should be all squeaky clean now */
    reg->csr = PAVI;		/* interupt only on pkt avail */

    dvflush(dv, DVFL_X); 	/* flush queued traffic */
    if (flag == DVR_ON) {
	dv->dv_istatus |= (DV_ENABLED|DV_ONLINE);
    } else {
	dv->dv_istatus &= ~(DV_ENABLED|DV_ONLINE);
    }
    spl(s);
}


/*
 * chintr(dv)  combined input/output interupt handeler 
 *	dv - ptr to info structure
 */
chintr(dv)
struct device *dv;
{
    struct chdevice far * reg = (struct chdevice far *) dv->dv_addr;
    int z;

    spl7();			/* disarm the 8259 */
    sti();			/* rearm processor interupts */

    dv->dv_ch.rearm = PAVI;	/* default, rearm receive packet avail only */

    while (~reg->csr & PAVI)
    	chrintr(dv);		/* receive as much as possible */

    if (~reg->csr &  (XRI|TINT))
    	chxintr(dv);		/* did a transmit */

    reg->csr = dv->dv_ch.rearm; /* And some stupid tricks to rearm */
    reg->fpp = dv->dv_ch.nextpage;
    reg->fpp = dv->dv_ch.nextpage|ARM;

    cli();			/* no processor interupts */
    spl0();			/* arm the 8259 */
    EOI(dv->dv_pb->pb_br);		/* rearm this device */
}

/*
 * chrintr(dv)
 *	dv - ptr to device info structure
 *
 * chrintr processes a receive  interrupt.  
 */
chrintr(dv)
register struct device *dv;
{
    register struct chdevice far *reg = (struct chdevice far *)dv->dv_addr;
    struct packet *p;
    int			npage,		/* next page to read from */
    			fpage;		/* first page this packet */
    int			pslen=0,	/* length of this part of the packet*/
    			plen=0;		/* total packet length */
    char		*buf,		/* where the data is to go */
    			z;		/* place to pitch bytes */
    u_char		stat;		/* error flags */

#ifdef CHDEBUG
cprintf("CHRI %lx, csr=%x, rcsr=%x\r\n",reg,reg->csr,reg->rcsr);
#endif

    p = dv->dv_rp;			/* prepare a buffer to take a packet*/
    buf = p->p_ba + CHPOFF;

    stat = reg->rcsr;			/* read and clear past error latches*/
    reg->rcsr = stat;
    if (stat & (RSTP|SHRTP|ALIGN|CRC)) profile(dv,dr_rnerr);
    if (stat & OVF) profile(dv,dr_rierr);

    fpage = npage = dv->dv_ch.nextpage;
    while (!(reg->psize[npage]&PEND)){	/* must check for infinite packets */
    	pslen += 128;
	if(++npage>MAXPAGE){
	    if(pslen<=CHMAXLEN)
		copin(buf,reg->rbuff[fpage],pslen);
	    buf += pslen;
	    plen = pslen;
	    pslen = 0;
	    fpage = npage = 0;
	}
    }
    pslen += (reg->psize[npage]&PL) + 1;
    plen += pslen;
    if (plen<=CHMAXLEN){
	copin(buf,reg->rbuff[fpage],pslen);
	p->p_off = CHPOFF;
	p->p_len = plen;

	if (pfull(&dv->dv_rq) || (dv->dv_rp = palloc()) == 0) {
	    dv->dv_rp = p;
	    dv_profile(dv, dr_rdrop);
	} else {
	    enqueue(&dv->dv_rq, p);
	    wakeup(&dv->dv_rq);
	    dv_profile(dv, dr_rcnt);
	}

    }

    if(++npage>MAXPAGE) npage=0;
    if(npage == (reg->epp&PPM)) z=reg->clrpav;
    reg->fpp = npage;

    dv->dv_ch.nextpage = npage;		/* save it for next time */
}

/*
 * chxintr(dv)
 *	dv - ptr to device info structure
 *
 * chxintr is called to handle a command (transmit) interrupt.
 * The next transmit is started.
 */
chxintr(dv)
register struct device *dv;
{
    struct chdevice far *reg = (struct chdevice far *)dv->dv_addr;
    struct packet *p = dv->dv_xp;
    u_char		stat;		/* error flags */

#ifdef CHDEBUG
cprintf("CHXI %lx, csr=%x, xcsr=%x\r\n",reg,reg->csr,reg->xcsr);
#endif
    stat = reg->xcsr;		/* Clear prior latched events */
    reg->xcsr=stat;

    if ( stat & (XUVR|XPAR)) profile(dv, dr_xierr);
    if ( stat & XCOLL) dv->dv_cnts->dr_jcnt += reg->colla;	/* collision*/
    if ( stat & XMC) profile(dv, dr_xnerr);	/* 16 colls== net error */

    reg->ximask = 0;		/* clear enables */

    dv->dv_xp = (struct packet *)dequeue(&dv->dv_xq);

    if (p) (*(p->p_done))(p);
    if (dv->dv_xp){
        chstart(dv);
	dv->dv_ch.rearm |= XRI|TINT;	/* arm transmit interupts later */
    }
}

/*
 * chstart(dv)
 *	dv - ptr to device info structure
 *
 * chstart is called to start the next transmit.  Get the packet from
 * the send queue.
 * This is a primitive driver - It only uses one transmit buffer.
 */
#define THISB 0
chstart(dv)
register struct device *dv;
{
    struct chdevice far *reg = (struct chdevice far *)dv->dv_addr;
    register char far *off;
    register struct packet *p;
    char z;				/* trash */
    short len;

    p = dv->dv_xp;

    len = p->p_len;
    if (len < CHMINLEN)
    	len = CHMINLEN;

    off = (char far *)&(reg->xbuff[THISB+1][0]) - len;
    copout(off, poff(u_char *, p), len);
    reg->tsal = XAML & (u_long) off;	/* Wrong byte order, do it in parts */
    reg->tsah = (XAMH & (u_long) off) >> 8;
    reg->ximask = XRDY|XMC;
    z = reg->startx;
    reg->csr = XRI|PAVI;		/* arm interupts for ch_io */

}
#endif	C_CH
