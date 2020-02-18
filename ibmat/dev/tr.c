/*
 * IBM  Token Ring Device driver, 1986 Matt Mathis.
 * Lifted from the RT driver for the same device.
 *	See IBM part 5799-CGZ and Copyright form G120-2083
 * Portions inspired by the AT Token Ring Driver, also by IBM.
 *
 */
#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/dma.h"
#include "../../h/devstats.h"



#include "../dev/trreg.h"
#include "../dev/tr.h"

#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/tr.h"
#include "debug/tr.h"

#if	C_TR > 0

#include "../../h/profile.h"
#include "../../h/globalsw.h"

u_int TRPOFF = 0;
/* stuff to identify who interupted on the partyline */
extern int numtr;
extern struct device *trdvtab[MAXTR];

struct tr_list * tr_buildlists();

/*
 * trreset(dv)
 *	dv - ptr to device info structure
 *
 * Called from autoconf code at boot time to do 'hardware initialization'
 * trreset  - initialize adapter, open, and issue receive
 *           to enable packet reception
 *             Initialization consists of several stages
 *              1) check of bring-up diagnostics
 *              2) transfer of initialization parameters
 *              3) dma interface check.
 *		4) get the device hardware (network) address
 *		5) build the transmit and receive lists
 *		6) open the device for reads
 *
 * The remaining initialization is completed at interupt level
 * 	when the open completes.
 *
 */
trreset(dv, flag)
register struct device *dv;
{

    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    int s = spl7();
    int retry;

    int success = 0;

#ifdef TRIDEBUG
    cprintf("TR begin init\n\r");
#endif

    /*  decide which dma to use */
    if (reg == FIRSTCARD) DMAPREP(dv->dv_tr.dma,5);
    if (reg == SECONDCARD) DMAPREP(dv->dv_tr.dma,6);


    /* assume initial attempt plus 3 more retries of full procedure */
    for (retry=0; (retry<LAN_RETRY) && !success; retry++)
    {

#ifdef TRIDEBUG
	cprintf("TR retry %d\r\n",retry);
#endif TRIDEBUG
	DMAOFF(dv->dv_tr.dma);	/* otherwise bringup can glitch the DMA */

	if(!tr_bringup(dv->dv_addr)) continue; /* Move to probe? */
	if(!tr_handshake(dv)) continue;
	if(!tr_dmatest(dv)) continue;
	if(!tr_getaddr(dv)) continue;
	if(!dv->dv_tr.tr_rlist)
	    dv->dv_tr.tr_rlist=tr_buildlists(RECEIVE_LISTS);
	if(!dv->dv_tr.tr_xlist)
	    dv->dv_tr.tr_xlist=tr_buildlists(TRANSMIT_LISTS);
	dv->dv_tr.tr_clist=dv->dv_tr.tr_xlist;
	if(!tr_open(dv)) continue;
	success++;
    }

    dvflush(dv, DVFL_X); 	/* flush queued traffic */

    				/* bring it up if requested and safe */
    if ((flag == DVR_ON) && success) {
/* BUG */
    };
    dv->dv_istatus &= ~(DV_ONLINE);	/* online after the open */
    dv->dv_istatus |= DV_ENABLED;	/* just enabled for the moment */
    spl(s);
}

/* tr_bringup runs the bringup diagnostic */
/* feature/BUG this is also called at probe time to test if a 
 * card is really present:  To eliminate some problems with false
 * affirmitive probes, the initalization test is more restrictive than
 * called for in the documentation.  The status word must have TR_RESET
 * set and no other bits.   This works with the current rev of the cards
 * but may fail on future revs.   --- All this because the AT does not
 * generate an interupt in response to references to non existant devices.
 */
tr_bringup(reg)
register struct trdevice far *reg;
{

    int j, success;
    u_short sifrbuf;

    DIOR(tr_reset);		/* reset the card - DMA MUST BE OFF */
    DIOR(tr_reset);

    DIOW(tr_cmdstat, TR_RESET);
    /* Wait for up to 3 seconds for the test */
    POLL(3, TR_INITIALIZE&~(sifrbuf=DIOR(tr_cmdstat)) );
#ifdef TRIDEBUG
    cprintf("tr status = %x\r\n",sifrbuf);
#endif
    if (sifrbuf == TR_INITIALIZE ){   /* CAUTION: see above */
	return(TRUE);
    }
    else {
	cprintf("TR (%lx) Not initialized, Stat=%x\r\n",
		    	reg, sifrbuf);
	return(FALSE);
    }
}

/* initialization template: shaired by all cards. */
struct tr_init_params tip = DEF_INIT_PARAMS;

/* Transfer and verify (handshake) initialization parameter block */
tr_handshake(dv)
register struct device *dv;
{

    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    u_short *initptr;		/* Short pointer for copying init params */
    int success = 0;
    int iaddr;			/* init address */
    int k;
    u_short temp;		/* TEMP */

#ifdef TRIDEBUG
    cprintf("TR handshake\r\n");
#endif TRIDEBUG

    if (!dv->dv_tr.tr_scb)
    	dv->dv_tr.tr_scb = (struct tr_scb *)malloc(sizeof(struct tr_scb));
    if (!dv->dv_tr.tr_ssb)
    	dv->dv_tr.tr_ssb = (struct tr_ssb *)malloc(sizeof(struct tr_ssb));
    if (!dv->dv_tr.tr_scb || !dv->dv_tr.tr_ssb)
    	panic("TR handshake\r\n");

    /* Tip is filled in above in the declarations. THIS IS NOT RE-ENTERANT */
    tip.scb_addr = NEARTODIO(dv->dv_tr.tr_scb);
    tip.ssb_addr = NEARTODIO(dv->dv_tr.tr_ssb);

		    /* write adapter init parameters*/
    DIOW(tr_address, TR_INIT_DATAA);
    initptr = (u_short *) &tip;
    for (k=0; k<(sizeof tip/sizeof (short)); k++){
	DIOW(tr_datai,*initptr++);
    }

	    /* verify initialization parms    */
    DIOW(tr_address, TR_INIT_DATAA);
    initptr = (u_short *) &tip;
    for (k=0;k<(sizeof tip/sizeof (short)); k++){
    	temp=DIOR(tr_datai);
	if (temp != *initptr++) {
	    cprintf("TR %x; parameter initialization failed\r\n",reg);
	    return (FALSE);
	};
    }

#ifdef TRIDEBUG
    cprintf("tr init parm read successful\r\n");
#endif TRIDEBUG
    return(TRUE);
}


/* DMA test */
tr_dmatest(dv)
register struct device *dv;
{

    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    u_short sifrbuf;
    int k;

#ifdef TRIDEBUG
    cprintf("tr Begin DMA test\r\n");
#endif TRIDEBUG

    DMACONFIGURE(dv->dv_tr.dma);
    DMAON(dv->dv_tr.dma);

    DIOW(tr_cmdstat, TR_EXECUTE);

    /* WAIT AT LEAST 10 SECONDS BEFORE READING TO ALLOW FOR DMA TIMEOUT	*/
		/* Unless there is a successful completion first */
    NSEC(1);
    POLL(10, (sifrbuf=DIOR(tr_cmdstat)) );

#ifdef TRIDEBUG
    cprintf("TR dma interface test, final status=%x\r\n",sifrbuf);
#endif TRIDEBUG

    SSBCLEAR;
    if (sifrbuf == 0x0000) {
	return(TRUE);
    }
    else {
	printf ("TR dma interface error, code=%x\r\n",sifrbuf);
	return(FALSE);
    }
}

/* Get the burned in address for the router's use.   This is tricky because
 * the first set: getting the address within the adapter must be done
 * before the open and the second part, getting the actual address, must
 * be done after the open.
 *
 * Hack: dv_phys is used as a buffer to put the return value.
 */
tr_getaddr(dv)
register struct device *dv;
{
    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    u_short sifrbuf;

#ifdef TRIDEBUG
    cprintf("TR getaddr\r\n");
#endif

    tr_readadapt(dv, dv->dv_phys, TR_BIA, 2);	/* get the pointer */
    return(TRUE);
}

/* build generic transmit and receive lists */
struct tr_list *
tr_buildlists(cnt)
int cnt;
{
    struct tr_list *this,*last,*first;
    int i;

    for(i=0;i<cnt;i++){
	this = (struct tr_list *) malloc(sizeof (struct tr_list));
	if (!this) panic("tr_buildlist\r\n");
	if (i){
	    this->forward = last;
	    this->adapt.forward = NEARTO24(&last->adapt.forward);
	}
	else {
	    first=this;
	}

	this->pkt = 0;
	this->adapt.cstat = 0;		/* mark it not valid */
	last=this;
    }
    first->forward = last;
    first->adapt.forward = NEARTO24(&last->adapt.forward);
    return(first)
;}

/* Start the open, to be finished at interupt level */

/* Open structure: shaired by all cards */
struct tr_open_params opens = DEF_OPEN_PARAMS;
tr_open(dv)
register struct device *dv;
{
    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    tr_exec(dv, TR_OPEN, &opens);
    DIOR(tr_enable);			/* allow the card to interupt */
}

/*****************************************************************
 *  tr_exec:	Serialize and issue command requests to the adapter
 *		Interupts must be disabled before calling.
 *	dv:	The device structure
 *	cmd:	The command to be issued
 *	clist:	pointer to the command argument list
 */
tr_exec(dv,cmd,clist)
register struct device *dv;
u_short cmd;
u_short *clist;
{
    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
#ifdef TRDEBUG
    cprintf("tr exec, cmd=%x\r\n",cmd);
#endif

    while (dv->dv_tr.tr_scb->command) {	/* wait for the last command*/
#ifdef TRDEBUG
	cprintf("busy\r");	/* gets overprinted */
#endif TRDEBUG
#ifdef TRIDEBUG
	cprintf("busy\r");	/* gets overprinted */
#endif TRIDEBUG
    }
    dv->dv_tr.tr_scb->command = swas(cmd);
    dv->dv_tr.tr_scb->clist = NEARTO24(clist);
    DIOW(tr_cmdstat, TR_EXECUTE);
}

/*****************************************************************
 * read adapter buffer (synchronously)
 *	This executes the read adapter command and spin loops to get
 * the results.  It is unsafe when ever there are other commands in progress
 * since it assumes that the next command to complete will be the read
 * adapter.  This should only be used before the adapter is open, or after
 * some event such as an adapter check, which aborts all other commands.
 *	dv - device structure
 *	buff - buffer for the results
 *	addr - address within the adapter to be read
 *	cnt - number of bytes to be read
 *
 * interupts must be disabled
 */
tr_readadapt(dv,buff,addr,cnt)
register struct device *dv;
u_short *buff;
u_short addr;
u_short cnt;
{
    register struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    u_short sifrbuf;

#ifdef TRIDEBUG
    cprintf("TR readadpt\r\n");
#endif

    buff[0] = swas(cnt);
    buff[1] = swas(addr);

    tr_exec(dv, TR_RDADAPTR, buff);
    return(TRUE);
}

/*****************************************************************
 * trparty()	Token ring party line interupt: Shaired by both cards for 
 *		all functions.
 *			They are not so nice as to tell us who is responsible.
 *		So all possible cards must be polled.
 */
trparty()
{
    register struct device *dv;
    struct trdevice far *reg;
    int i;

    spl7();			/* disarm the 8259 */
    sti();			/* rearm processor interupts */
#ifdef TRDEBUG
    cprintf("TR partyline\r\n");
#endif

    for (i=0;i<numtr;i++){
	reg = (struct trdevice far *) trdvtab[i]->dv_addr;
	if (DIOR(tr_cmdstat) & TR_INT)
	    trintr(trdvtab[i]);
    }

    cli();			/* no processor interupts */
    spl0();			/* arm the 8259 */
    diow(TR_ILE,0);		/* and the party line */
    EOI(2);			/* tell both interupt controllers */
    EOI12;			/* that we are done */
}

/*
 * trintr(dv)
 *
 *	Process an interupt from the adapter
 */
trintr(dv)
register struct device *dv;
{
    struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    u_short sifrbuf;
    struct tr_ssb ssbbuf;		/* stupid */
    struct packet *p;
    struct tr_list *list;

    sifrbuf=DIOR(tr_cmdstat);
    swab(dv->dv_tr.tr_ssb, &ssbbuf, sizeof (struct tr_ssb));

#ifdef TRDEBUG
cprintf("TRINTR %lx, stat=%x, ssb=%x,%x,%x,%x\r\n",reg,sifrbuf, 
	ssbbuf.command, ssbbuf.status0,
	ssbbuf.status1, ssbbuf.status2);
#endif

    switch (sifrbuf&TR_ADAP_INT) {
    case TR_RECVSTAT:		/* receive status */
	SSBCLEAR;
	if (ssbbuf.status0 != TR_COMPLETE){
	    cprintf("TR (%lx) Unexpected recvstat=%x\r\n",reg,ssbbuf.status0);
	};
	list=dv->dv_tr.tr_rlist;
	while( !(list->adapt.cstat & SWAS(TR_FVALID)) ){
#ifdef TRDEBUG
	    cprintf("cstat=%x\r\n",swas(list->adapt.cstat));
#endif TRDEBUG
	    if ((list->adapt.cstat&SWAS(TR_FCOMPLETE|TR_FSOFEOF))
				!= SWAS(TR_FCOMPLETE|TR_FSOFEOF)){
		cprintf("TR (%lx) Unexpected R Cstat=%x\r\n",
			reg,swas(list->adapt.cstat));
		profile(dv, dr_rierr);		/* assume driver problem */
	    };
	    if( pfull(&dv->dv_rq) || !(p=palloc()) ){
			 /* discard this packet by recycling this buffer */
#ifdef TRDEBUG
	cprintf("Pfull\r\n");
#endif TRDEBUG
		dv_profile(dv, dr_rdrop);
	    }
	    else {
		list->pkt->p_off = TRPOFF;
		list->pkt->p_len = swas(list->adapt.fsize);
		enqueue(&dv->dv_rq, list->pkt);
		wakeup(&dv->dv_rq);
		dv_profile(dv, dr_rcnt);
	    	list->pkt = p;
		list->adapt.daddr = NEARTO24(p->p_ba + TRPOFF);
	    }
	    list->adapt.cstat = SWAS(TR_FVALID|TR_FINT);
	    DIOW(tr_cmdstat,TR_RECVALID);
	    list=list->forward;
	}
	dv->dv_tr.tr_rlist=list;
	break;			/* receive status */

    case TR_XMITSTAT:		/* transmit status */
#ifdef TRDEBUG
	cprintf("TR transmit\r\n");
#endif TRDEBUG
	SSBCLEAR;
	if (ssbbuf.status0 != TR_FCOMPLETE){
	    cprintf("TR (%lx) Unexpected xmitstat=%x\r\n",reg,ssbbuf.status0);
	};

	list=dv->dv_tr.tr_clist;
	while( !(list->adapt.cstat & SWAS(TR_FVALID)) &&
		(list != dv->dv_tr.tr_xlist) ){
	    if ((list->adapt.cstat & SWAS(TR_FFS)) != SWAS(TR_FFSV)){
		profile(dv, dr_xnerr);
	    }
	    (*(list->pkt->p_done))(list->pkt);
	    list=list->forward;
	}
	dv->dv_tr.tr_clist = list;

	if (dv->dv_xp)
	    trstart(dv);
	break;			/* transmit status */

    case TR_CMDSTAT:		/* Command completion */
#ifdef TRDEBUG
    cprintf("TR cmdstat, stat=%x, ssb=%x,%x\r\n",
	sifrbuf, ssbbuf.command, ssbbuf.status0);
#endif TRDEBUG

	SSBCLEAR;
	switch (ssbbuf.command & TR_CMDMASK) {
	    case TR_OPEN:	/* open completion */
#ifdef TRIDEBUG
		cprintf("Open completed, allocate packets\r\n");
#endif TRIDEBUG
		cprintf("TR (%lx) OPEN STATUS=%4x\r\n",reg,ssbbuf.status0);

		if (ssbbuf.status0 != TR_COMPLETE) {
		    cprintf("TR (%lx) OPEN ERROR\r\n",reg);
		} /* BUG: plow on? */

		list=dv->dv_tr.tr_rlist;
		do{
		    if(list->pkt) {	/* don't re-allocate packets */
			list=list->forward;
			continue;
		    }
		    if (!(list->pkt = p = palloc())) panic("TR palloc");
		    list->adapt.dcount = SWAS(TRMAXLEN);
		    list->adapt.daddr = NEARTO24(p->p_ba + TRPOFF);
		    list->adapt.cstat = SWAS(TR_FVALID|TR_FINT);
		    list=list->forward;
		} while (list != dv->dv_tr.tr_rlist);
		tr_exec(dv,TR_RECEIVE,&list->adapt);
		tr_exec(dv,TR_TRANSMIT,&(dv->dv_tr.tr_xlist->adapt));
		tr_readadapt(dv, dv->dv_phys, dv->dv_tr.bia_addr, TR_HLN);
			/* online when the readadapt finally finishes */
		break;		/* open completion */

	    case TR_RDADAPTR:
		if (dv->dv_tr.bia_addr) {
		    dv->dv_istatus |= DV_ONLINE; /* BUG Race: First xmit ?? */
#ifdef TRIDEBUG
		    cprintf("TR net addr=%x,%x,%x,%x,%x,%x\r\n",
			dv->dv_phys[0],dv->dv_phys[1],dv->dv_phys[2],dv->dv_phys[3],dv->dv_phys[4],dv->dv_phys[5]);
#endif TRIDEBUG
		}
		else {
		    dv->dv_tr.bia_addr = swas( ((u_short *) dv->dv_phys)[0]);
#ifdef TRIDEBUG
		    cprintf("TR bia addr=%x\r\n",dv->dv_tr.bia_addr);
#endif TRIDEBUG
		}
		break;		/* read adapter completion */

	    default:
		cprintf("TR (%lx) Unexpected command completed %x\r\n",
			reg,ssbbuf.command);

	} /* switch( ssb.command ) */
	break;			/* Command completion */

    case TR_RINGSTAT:		/* Ring status */
	cprintf("TR (%lx) Ring Stat=%x\r\n",reg,ssbbuf.status0);
	SSBCLEAR;
	break;			/* Ring status */

    case TR_ACHECK:		/* adapter check */
	cprintf("TR %lx, ACHECK stat=%x, ssb=%x,%x,%x,%x\r\n",reg,sifrbuf, 
		ssbbuf.command, ssbbuf.status0,
		ssbbuf.status1, ssbbuf.status2);
	SSBCLEAR;
	break;			/* adapter check */

    case TR_SCBCLEAR:		/* The scb is clear (we don't request) */
    default:			/* and other chaos */
	cprintf("TR %xlx Unknown interupt=%x\r\n",reg,sifrbuf);
	profile(dv,dr_unsol);
	SSBCLEAR;
    } /* switch( tr_cmdstat ) */
}

/*****************************************************************
 *	trstart(dv) - (re)start the transmitter if there is
 *		something to be done.
 *	Called from both tr_output and transmit interupt
 *
 * Transmit sequence (queue) from first to last:
 *	1) Transmit packets already in the adapter
 *	2) Up to TR_LIST packets in the transmit list pool
 *	3) One packet in dv->dv_xp
 *	4) Everything backed up in dv->dv_xq
 *		All packets start at 3 or 4 and propagate up the list.
 * BUG the queue length instrumentation only knows about packets in 4)
 */
trstart(dv)
register struct device *dv;
{
    struct trdevice far *reg = (struct trdevice far *)dv->dv_addr;
    struct packet *p = dv->dv_xp;
    struct tr_list *list = dv->dv_tr.tr_xlist;

#ifdef TRDEBUG
cprintf("trstart\r\n");
#endif TRDEBUG

    while(p && (list->forward != dv->dv_tr.tr_clist)){
	list->pkt = p;
	list->adapt.dcount = list->adapt.fsize = swas(p->p_len);
	list->adapt.daddr = NEARTO24(p->p_ba + p->p_off);
	list->adapt.cstat = SWAS(TR_FVALID|TR_FSOFEOF|TR_FINT);
	DIOW(tr_cmdstat,TR_XMTVALID);
	list = list->forward;
	p = (struct packet *)dequeue(&dv->dv_xq);
    }
    dv->dv_tr.tr_xlist = list;
    dv->dv_xp = p;

}

#endif TR_C
