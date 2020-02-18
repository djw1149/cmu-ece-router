/*
 * file: il.c
 *
 * synopsis: Interlan NI3210 Ethernet interface driver.
 *
 *
 * modification history
 *
 *    Aug-86  Chriss Stephens (chriss) at Carnegie Mellon University removed
 *	support for version zero of rcp.
 *
 *    Jul-86  Chriss Stephens (chriss) at Carnegie Mellon University installed
 *	watchdog timer for transmitter queue. See functions ilxwedged and 
 *	ilcxintr. Added receiver congestion logging.
 *	
 *
 *    Jun-86  Chriss Stphens (chriss) at Carnegie Mellon University modified
 *	to use rc_append and version 1 of router client software for remote
 *	diagnoses.
 *
 * 21-Mar-86  David Bohman (bohman) at Carnegie-Mellon University
 *	Added check in ilfrintr() for new rfd/rbd already being at the end
 *	of the list.  Previously, the list would end up without a EL bit
 *	being set and next time ilrestart() was called, the system would
 *	hang.
 *
 * Written 10/15/85 by David Bohman
 *
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
#include "../../h/devstats.h"
#include "../mch/device.h"

#include "../../h/profile.h"
#include "../../h/globalsw.h"
#include "../../h/rcp.h"

u_int ILPOFF = 0;

extern char *rc_append(); 
extern int time;

/*
 * ilreset(dv,flag)
 *	dv - ptr to device info structure
 *	flag - indicates the new status of the interface.
 * ilreset sets up the shared memory, and resets the ethernet chip.  It also
 * sets the ethernet address and starts the receiver (RU).  ilreset is
 * called from autoconf code at boot time, and also from the transmit
 * watchdog if the transmitter wedges.
 */
ilreset(dv, flag)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct scp *scp;
    register struct iscp *iscp;
    register struct scb *scbp;
    register s = spl7();

    reg->csr_enb = 0;
    reg->csr_dmaie = 0;
    reg->csr_swp = 1;
    dv->dv_il.il_free = dv->dv_il.il_mem;
    dv->dv_il.il_flags = 0;
    iscp = (struct iscp *)(dv->dv_il.il_free+SCPOFFSET-sizeof (struct iscp));
    iscp->iscp_busy = 1;
    iscp->iscp_scb = 0;
    iscp->iscp_memb = (u_char *)swap(dv->dv_il.il_free);
    scp = (struct scp *)(dv->dv_il.il_free+SCPOFFSET);
    scp->scp_bus = 0;
    scp->scp_iscp = (struct iscp *)swap(iscp);
    ilalloc(dv, scbp, struct scb, sizeof (struct scb));
    dv->dv_il.il_scb = scbp;
    reg->csr_enb = 1;
    reg->csr_ca = 1;
    while (reg->csr_ca == 0)
    	;
    while (iscp->iscp_busy)
    	;
    while (scbp->scb_cx == 0)
    	;
    scbp->scb_ack = scbp->scb_stat;
    reg->csr_ca = 1;
    while (reg->csr_ca)
    	;
    scbp->scb_reset = 1;
    reg->csr_ca = 1;
    while (scbp->scb_cmd)
    	;
    ildoconf(dv);
    /*  ildump(dv); Only for debugging ildoconf() */
    ilsetaddr(dv);
    ilxinit(dv);
    ilrinit(dv);
    dvflush(dv, DVFL_X);
    if (flag == DVR_ON) {
	dv->dv_istatus |= (DV_ENABLED|DV_ONLINE);
	scbp->scb_ruc = XUC_START;
	reg->csr_ca = 1;
	while (scbp->scb_ruc)
	    ;
	reg->csr_onl = 1;
    } else {
	dv->dv_istatus &= ~(DV_ENABLED|DV_ONLINE);
	reg->csr_onl = 0;
    }
    dv->dv_pb->pb_br = PS_BR6;
    spl(s);
}

/*
 * ilrinit(dv)
 *	dv - ptr to device info structure
 *
 * ilrinit initializes the receive buffer ring in the shared memory.
 */
ilrinit(dv)
register struct device *dv;
{
    register struct scb *scbp = dv->dv_il.il_scb;
    register struct rfd *rf, *orf = 0;
    register struct rbd *rb, *orb = 0;
    register u_char *b;
    struct rfd *lrf;
    struct rbd *lrb;
    register i;

    for (i = 0; i < NILRBUF; i++) {
	ilalloc(dv, rf, struct rfd, sizeof (struct rfd));
	ilalloc(dv, rb, struct rbd, sizeof (struct rbd));
	ilalloc(dv, b, u_char, ILMAXLEN);
	if (i == 0) {
	    lrf = rf;
	    lrb = rb;
	}
	rf->rf_next = iloff(orf);
	rf->rf_rbd = 0xffff;
	rb->rb_next = iloff(orb);
	rb->rb_buf = swap(b);
	rb->rb_size = ILMAXLEN;
	orf = rf; orb = rb;
    }
    rf->rf_rbd = iloff(rb);
    lrf->rf_next = iloff(rf);
    lrf->rf_el = 1;
    lrb->rb_next = iloff(rb);
    lrb->rb_el = 1;
    scbp->scb_rfa = iloff(rf);
    dv->dv_il.il_lrf = lrf;
    dv->dv_il.il_lrb = lrb;
}

/*
 * ilxinit(dv)
 *	dv - ptr to device info structure
 *
 * ilxinit initializes the transmit buffer and command structure in shared
 * memory.
 */
ilxinit(dv)
register struct device *dv;
{
    register struct scb *scbp = dv->dv_il.il_scb;
    register struct tfd *tf;
    register struct tbd *tb;
    register u_char *b;

    ilalloc(dv, tf, struct tfd, sizeof (struct tfd));
    ilalloc(dv, tb, struct tbd, sizeof (struct tbd));
    ilalloc(dv, b, u_char, ILMAXLEN);
    dv->dv_il.il_tf = tf;
    scbp->scb_cbl = iloff(tf);
    tf->tf_tbd = iloff(tb);
    tb->tb_eof = 1;
    tb->tb_buf = swap(b);
}

/*
 * ildoconf(dv)
 *	dv - ptr to device info structure
 *
 * ildoconf configures the ethernet chip.  Most of the defaults are adequate
 * for the 10mb ethernet, but we want the chip to put the ethernet headers
 * in the data buffers, not the frame descriptors.  We also want to set
 * tonocrs.  This is so the chip transmits even if it does not see carrier
 * sense.  This means that we must initialize 10 bytes.
 */
ildoconf(dv)
register struct device *dv;
{
    register struct cmdb *cbl;
    register struct ilconfig *cf;

    cbl = (struct cmdb *)dv->dv_il.il_free;
    cf = (struct ilconfig *)(cbl+1);
    bzero(cbl, sizeof (struct cmdb) + sizeof (struct ilconfig));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_CONFIG;
    cf->cf_fifolim = 8;

    cf->cf_bytecnt = 10;

    cf->cf_preamb = 2; cf->cf_addrlen = 6; cf->cf_atloc = 1;

    cf->cf_ifspace = 96;

    cf->cf_maxrtry = 15; cf->cf_sltime = 512;

    cf->cf_tonocrs = 1;
    ilcmd(dv, cbl);
}

ildump(dv)
register struct device *dv;
{
    register struct cmdb *cbl;
    register struct ildump *du;
    register struct ilconfig *cf;
    int i;
    u_char *cp;

    cbl = (struct cmdb *)dv->dv_il.il_free;
    du = (struct ildump *)(cbl+1);
    cf = (struct ilconfig *)(du+1);
    bzero(cbl, sizeof (struct cmdb) + sizeof (struct ildump));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_DUMP;
    du->du_buf = iloff(cf);
    ilcmd(dv, cbl);
    cp = (u_char *)cf;
    cprintf("il (%x) config dump:\r\n", dv->dv_addr);
    for (i = 0; i < /*6*/20; i++) {
	cprintf("%02x: %02x %02x\r\n", i<<1, *cp, *(cp+1));
	cp+=2;
    }
}

/*
 * ilsetaddr(dv)
 *	dv - ptr to device info structure
 *
 * ilsetaddr sets the ethernet address of the station.  The correct address
 * is gotten from the info structure which was setup in ilinit.
 */
ilsetaddr(dv)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct cmdb *cbl;
    register u_char *adr;

    cbl = (struct cmdb *)dv->dv_il.il_free;
    adr = (u_char *)(cbl+1);
    bzero(cbl, sizeof (struct cmdb));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_SETADDR;
    reg->csr_swp = 0;
    bcopy(reg->csr_haddr, adr, IL_HLN);
    reg->csr_swp = 1;
    ilcmd(dv, cbl);
}

/*
 * ilcmd(dv, cbl)
 *	dv - ptr to device info structure
 *	cbl - ptr to command block
 *
 * ilcmd executes the passed command block spin-loop.  The command block
 * must be setup elsewhere.  Note that this routine writes over the cbl
 * field in the scb without saving it first.
 */
ilcmd(dv, cbl)
register struct device *dv;
register struct cmdb *cbl;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct scb *scbp = dv->dv_il.il_scb;

    cbl->cmd_i = 1;
    scbp->scb_cbl = iloff(cbl);
    scbp->scb_cuc = XUC_START;
    reg->csr_ca = 1;
    while (scbp->scb_cmd)
    	;
    while (scbp->scb_cx == 0)
    	;
    scbp->scb_ack = scbp->scb_stat;
    reg->csr_ca = 1;
    while (scbp->scb_cmd)
    	;
}

/*
 * ilrestart(dv)
 *	dv - ptr to device info structure
 *
 * ilrestart is used to restart the receiver after a no-resources condition.
 * It finds the end of the rfd and rbd lists and resets the end pointers to
 * point there.  It also resets the rfa pointer in the scb and restarts the
 * RU.
 */
ilrestart(dv)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct scb *scbp = dv->dv_il.il_scb;
    register struct rfd *rf;
    register struct rbd *rb;

    rf = dv->dv_il.il_lrf;
    while (rf->rf_el == 0)
    	rf = (struct rfd *)ilabs(rf->rf_next);
    dv->dv_il.il_lrf = rf;

    rb = dv->dv_il.il_lrb;
    while (rb->rb_el == 0)
    	rb = (struct rbd *)ilabs(rb->rb_next);
    dv->dv_il.il_lrb = rb;
    rf = (struct rfd *)ilabs(rf->rf_next);
    rb = (struct rbd *)ilabs(rb->rb_next);
    rf->rf_rbd = iloff(rb);
    while (scbp->scb_cmd)
    	;
    scbp->scb_rfa = iloff(rf);
    scbp->scb_ruc = XUC_START;
    reg->csr_ca = 1;
}

/*
 * ilresume(dv)
 *	dv - ptr to device info structure
 *
 * NOT CURRENTLY USED
 * ilresume is used to resume the RU after it is supended.
 */
ilresume(dv)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct scb *scbp = dv->dv_il.il_scb;

    while (scbp->scb_cmd)
    	;
    scbp->scb_ruc = XUC_RESUME;
    reg->csr_ca;
}

/*
 * ilintr(dv)
 *	dv - ptr to device info structure
 *
 * ilintr is called when a chip interrupt occurs.  It saves the interrupt
 * status and acknowleges the interrupt(s).  It then processes each
 * interrupt in turn.  It is important to service the receive frame
 * interrupt before the rnr interrupt because we don't want to lose any
 * frames we might have received before the receiver went "not ready".
 */
ilintr(dv)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct scb *scbp = dv->dv_il.il_scb;
    register s = spl6();
    union scb_statun intr;

    while (scbp->scb_cmd)
    	;
    if (reg->csr_ca) {
	intr.scb_st2.scbst_stat = scbp->scb_status;
	scbp->scb_ack = intr.scb_st1.scbst_stat;
	reg->csr_ca = 1;

	if (intr.scb_st0.scbst_fr)
	    ilfrintr(dv);

	if (intr.scb_st0.scbst_rnr)
	    ilrnrintr(dv);

	if (intr.scb_st0.scbst_cx)
	    ilcxintr(dv);

	if (intr.scb_st0.scbst_cna)
	    /* do nothing */;
    }
    spl(s);
}

/*
 * ilfrintr(dv)
 *	dv - ptr to device info structure
 *
 * ilfrintr processes a receive frame interrupt.  It fetches through the rfd
 * list, processing frames until an unused rfd or the end of list is
 * reached.  The rfd and rbd list end pointers are reset correctly.
 */
ilfrintr(dv)
register struct device *dv;
{
    register struct rfd *lrf = dv->dv_il.il_lrf, *rf;
    register struct rbd *lrb = dv->dv_il.il_lrb, *rb;

    rf = (struct rfd *)ilabs(lrf->rf_next);
    while (rf->rf_el == 0)
	if (rf->rf_c) {
	    if (rf->rf_ok)
	    	ilrecv(dv, rf);
	    /* get first (only, hopefully) buffer for this frame */
	    rb = (struct rbd *)ilabs(rf->rf_rbd);
	    /* seek to last used buffer in this frame */
	    while (rb->rb_eof == 0) 
	    {
	    	rb = (struct rbd *)ilabs(rb->rb_next);
		if (rb == lrb) cprintf(" rb => lrb\r\n");
	    }
	    rf->rf_el = 1;
	    if (rf != lrf)
	    	lrf->rf_el = 0;
	    rf->rf_ok = rf->rf_c = 0;
	    rb->rb_el = 1;
	    if (rb != lrb)
	    	lrb->rb_el = 0;
	    rb->rb_eof = rb->rb_f = 0;
	    lrf = rf;
	    rf = (struct rfd *)ilabs(rf->rf_next);
	    lrb = rb;
	} else
	    break;
    dv->dv_il.il_lrf = lrf;
    dv->dv_il.il_lrb = lrb;
}

/*
 * ilrnrintr(dv)
 *	dv - ptr to device info structure
 *
 * ilrnrintr handles receiver not ready interrupts.  The only case that
 * should occur now is no-resources.
 */
ilrnrintr(dv)
register struct device *dv;
{
    register struct scb *scbp = dv->dv_il.il_scb;

/*  cprintf("il rnr: scb_rus %x\n", scbp->scb_rus); */
    switch (scbp->scb_rus) {
	case RUS_READY:
		break;

	case RUS_IDLE:
		break;

	case RUS_SUSP:
		ilresume(dv);
		break;

	case RUS_NORESOURCE:
		ilrestart(dv);
		break;
    }
}

/*
 * ilrecv(dv, rf)
 *	dv - ptr to device info structure
 *	rf - ptr to rfd with next packet
 *
 * ilrecv is called from ilfrintr to process a new receive frame.  The frame
 * is copied using dma into a packet.  The routine waits for the dma to
 * complete since we are using burst mode dma and the bus could not be used
 * for anything else anyway.  The received packet is then queued on the
 * receive queue.
 */
ilrecv(dv, rf)
register struct device *dv;
register struct rfd *rf;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct rbd *rb;
    register struct packet *p;
    u_long len;
    u_char *b;

    rb = (struct rbd *)ilabs(rf->rf_rbd);
    if (rb->rb_eof == 0 || rb->rb_f == 0) 
    {
	dv_profile(dv, dr_rlen);
    	return;
    }
    len = rb->rb_len;
    b = (u_char *)swap(rb->rb_buf);
    p = dv->dv_rp;
    dv_profile(dv, dr_rcnt);
    p->p_off = ILPOFF;
    p->p_len = len;

    reg->csr_dmacmd = DMA_TO_M|DMA_BYTE;
    reg->csr_dmacnt = swab(len);
    reg->csr_dmabuf = swab(b);
    reg->csr_dmamem = swal(p->p_ba);
    reg->csr_dma = 1;
    while (reg->csr_dma)
    	;
    reg->csr_dmadone = 0;

    if (pfull(&dv->dv_rq) || (dv->dv_rp = palloc()) == 0) {
	dv->dv_rp = p;
	dv_profile(dv, dr_rdrop);
    } else {
	enqueue(&dv->dv_rq, p);
	wakeup(&dv->dv_rq);
    }
}

/*
 * ilcxintr(dv)
 *	dv - ptr to device info structure
 *
 * ilcxintr is called to handle a command (transmit) interrupt.
 * The next transmit is started.
 */
ilcxintr(dv)
register struct device *dv;
{
    register struct tfd *tf = dv->dv_il.il_tf;
    register struct packet *p = dv->dv_xp;

    dv->dv_cnts->dr_jcnt += tf->tf_coll;
    if (tf->tf_ok == 0) {
        dv_profile(dv, dr_xnerr);
	if (tf->tf_s5)
	    dv->dv_cnts->dr_jcnt += 16;
    }
    dv->dv_il.il_flags &= ~ILF_OACTIVE;
    dv->dv_xp = (struct packet *)dequeue(&dv->dv_xq);
    (*(p->p_done))(p);
    if (dv->dv_xp)
        ilstart(dv);
    else				/* queue is empty so reset flags */
    {
      dv->dv_il.il_flags &= ~(ILF_XQFULL|ILF_XRESET);
    }
}

/*
 * ilstart(dv)
 *	dv - ptr to device info structure
 *
 * ilstart is called to start the next transmit.  The packet is gotten from
 * the send queue, and dma'd into shared memory.  Finally the transmit command
 * is given.
 */
ilstart(dv)
register struct device *dv;
{
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct packet *p;
    register struct scb *scbp = dv->dv_il.il_scb;
    register struct tfd *tf = dv->dv_il.il_tf;
    register struct tbd *tb;
    u_long len;
    u_char *b;

    if (dv->dv_il.il_flags & ILF_OACTIVE)
    	return;

    p = dv->dv_xp;
    len = p->p_len;
    if (len < ILMINLEN)
    	len = ILMINLEN;
    tf->tf_c = tf->tf_b = tf->tf_ok = 0;
    tf->tf_el = tf->tf_i = 1;
    tf->tf_cmd = CMD_XMIT;
    tb = (struct tbd *)ilabs(tf->tf_tbd);
    b = (u_char *)swap(tb->tb_buf);
    tb->tb_eof = 1;
    tb->tb_len = len;
    reg->csr_dmacmd = DMA_BYTE|DMA_FROM_M;
    reg->csr_dmacnt = swab(len);
    reg->csr_dmabuf = swab(b);
    reg->csr_dmamem = swal(poff(u_char *, p));
    reg->csr_dma = 1;
    while (reg->csr_dma)
    	;
    reg->csr_dmadone = 0;

    scbp->scb_cuc = XUC_START;
    reg->csr_ca = 1;
    dv->dv_il.il_flags |= ILF_OACTIVE;
}

/*
 * ilxwedged(dv)
 *	dv - ptr to device info structure
 *
 * ilxwedged is called when the output queue fills. The following occurs :
 *
 *	If transmit queue has just become full then watchdog timer started.
 *
 *	If transmit queue has been full for more than IL_MAXSEC then the
 *	transmitter chip is reset and the watchdog timer reset.
 *
 *	If transmit queue is full and more than a second has past since the
 *      transmitter was reset, the assumption is made that the transmitter
 *	is wedged, and we panic, i.e. reboot the router.
 */

ilxwedged(dv)

  register struct device *dv;

{
    switch (dv->dv_il.il_flags & (ILF_XQFULL|ILF_XRESET)) {
	case 0:	   /* The queue just filled*/
	    dv->dv_il.il_flags |= ILF_XQFULL;
	    dv->dv_il.il_wt = time;		   /* start the clock	    */
	    break;
	case ILF_XQFULL:	
	    if ((time - dv->dv_il.il_wt) > IL_MAXSEC){  /* then check wdog */
		dv->dv_il.il_flags |= ILF_XRESET;
		dv->dv_il.il_wt = time;		/* start new watchdog    */
		ilreset(dv, DVR_ON);		/* nudge transmitter	    */
	    }
	    break;
	default:
	    if ((time - dv->dv_il.il_wt) > IL_MAXSEC)  /* must be dead */
		panic ("il: transmitter permanently wedged");
    }
}

/* ildiag

	This function is referenced by the diagnostic function field
	in the device description structure. It looks at the version
	number of the requesting packet and branches to the appropriate
	routine to handle the diagnosis.

 */
	
ildiag
(dv, diag_type, p, rdg, rc, sport, saddr)

  register struct device   *dv;
                  int       diag_type;
  register struct packet   *p;
           struct rcp_info *rdg;
  register struct rcp      *rc;
                  short     sport;
           struct socket   *saddr;

{
    switch (ntohs(rc->rc_type))
    {
	case V1_RCT_DIAG :
	{
          switch (ntohs(diag_type))
	  {
  	    case ILDIAG_STATS:	return(V1_ildiag_stats(dv, p, rdg));

#ifdef ILDIAG	/* WARNING: the following options can crash the router */
	    case ILDIAG_DUMP:
		      if (rc_valid(dv, rc, sport, saddr))
			 return(V1_ildiag_dump(dv, p, rdg));
  		      return(RCED_NOAUTH);
  	    case ILDIAG_TDR:
		      if (rc_valid(dv, rc, sport, saddr))
			  return(V1_ildiag_tdr(dv, p, rdg));
		      return(RCED_NOAUTH);
	    case ILDIAG_DIAG:	return(V1_ildiag_diag(dv, p, rdg));
#endif ILDIAG

	    default:		return(RCED_UNSUP);
	  }
	}
    }
}

/* have to suspend the chip first??? */

V1_ildiag_tdr(dv, p, rdg)

  struct device     *dv;
  struct packet     *p;
  struct rcp_diagfo *rdg;

{
    register struct cmdb *cbl;
    register u_short *tdr;
    register u_short *area = (u_short *)
			 rc_append(rdg, sizeof(struct tdrcmd *),&(p->p_len));
    int pl = spl7();

    cbl = (struct cmdb *)dv->dv_il.il_free;
    tdr = (u_short *)(cbl + 1);
    bzero(cbl, sizeof (struct cmdb) + sizeof (u_short));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_TDR;
    ilcmd(dv, cbl);
    if (!(cbl->cmd_spec & 0x2000)) return(RCED_DFAIL); /* check ok bit */
    *area = *tdr;
    spl(pl);
    return(RCED_NOERR);
}

/* have to suspend the chip first??? */

V1_ildiag_dump(dv, p, rdg)

  register struct device   *dv;
  register struct packet   *p;
           struct rcp_info *rdg;

{
    register struct cmdb *cbl;
    register struct ildumpbuf *dumpb;
    register struct ildevice *reg = (struct ildevice *)dv->dv_addr;
    register struct ildump *du;
    int pl = spl7();

    cbl = (struct cmdb *)dv->dv_il.il_free;
    du = (struct ildump *)(cbl + 1);
    dumpb = (struct ildumpbuf *)(du + 1);
    bzero(cbl, sizeof (struct cmdb) + sizeof (struct ildump));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_DUMP;
    du->du_buf = iloff(dumpb);
    ilcmd(dv, cbl);
if (!(cbl->cmd_spec & 0x2000)) return(RCED_DFAIL); /* check ok bit */

#ifdef DMAVERSION
    {
    int temp;

    cprintf("about to do dma from %x, len %x\n\r", swab(swal(du->du_buf)),
					       swab(ILDUMPSIZE));
    ildiag_dumpprt(dumpb);
    reg->csr_dmacmd = DMA_TO_M|DMA_BYTE|DMA_SWAP;
    reg->csr_dmacnt = swab(ILDUMPSIZE);
    reg->csr_dmabuf = swab(swap(du->du_buf));
    reg->csr_dmamem = swal(temp = rc_append(rdg, ILDUMPSIZE, &(p->p_len)));
    reg->csr_dma = 1;
    while (reg->csr_dma)
    	;
    reg->csr_dmadone = 0;
    ildiag_dumpprt(temp);
    }

#else

    {
	register int i;
	register u_short *temp = rc_append(rdg, ILDUMPSIZE, &(p->p_len));
	for(i=0; i < ILDUMPSIZE; i+= 2)
	    *temp++ = htons(*((u_short *)dumpb)++);
    }

#endif DMAVERSION    

    spl(pl);
    return(RCED_NOERR);
}


V1_ildiag_stats(dv, p, rdg)

  register struct device     *dv;
  register struct packet     *p;
           struct rcp_diagfo *rdg;

{
    register struct scb *scbp = dv->dv_il.il_scb;
    u_short *area = (u_short *)
		    rc_append(rdg, 4 * sizeof(u_short), &(p->p_len));
    *area++ = htons(scbp->scb_crcerrs);
    *area++ = htons(scbp->scb_alnerrs);
    *area++ = htons(scbp->scb_rscerrs);
    *area   = htons(scbp->scb_ovrnerrs);
    return(RCED_NOERR);

}

V1_ildiag_dumpprt(data)

  struct ildumpbuf *data;

{
#define COL 20
    int i, j;
    printf("Dump returned:\n");
    for(i = 0; i < 170; i+= COL) {
	cprintf("%3d: ", i);
	for(j = i; j < i + COL; j++)
	    cprintf("%2x ", data->buf[j]);
	cputchar('\n');	cputchar('\r');
    }
    cputchar('\n'); cputchar('\r');
    cputchar('\n'); cputchar('\r');    
}


V1_ildiag_diag(dv, p, rdg)

  struct device     *dv;
  struct packet     *p;
  struct rcp_diagfo *rdg;

{
    register struct cmdb *cbl;
    register u_short *area = (u_short *)
			 rc_append(rdg, sizeof(struct tdrcmd *),&(p->p_len));
    int pl = spl7();

    cbl = (struct cmdb *)dv->dv_il.il_free;
    bzero(cbl, sizeof (struct cmdb));
    cbl->cmd_el = 1;
    cbl->cmd_cmd = CMD_DIAG;
    ilcmd(dv, cbl);

    *area = htons(*(u_short *)cbl->cmd_spec);

    spl(pl);
    return(RCED_NOERR);
}

#endif C_IL
