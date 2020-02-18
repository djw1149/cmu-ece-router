/*
 *  Token Ring protocol interface
 **********************************************************************
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/dma.h"
#include "../../h/proc.h"

#include "../dev/trreg.h"
#include "../dev/tr.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/tr.h"
#include "debug/tr.h"

#if	C_TR > 0

#include "../../h/errorlog.h"
#include "../../h/profile.h"
#include "../../h/globalsw.h"


/*
 *  tr_input - Generic input packet process
 *
 *  dv = device which received packet
 *
 *  Block waiting for a packet to be received and added to the packet input
 *  queue (by the receive interrupt handler).  Remove the packet from the queue
 *  and make it the currently mapped packet.  Increment the broadcast count for
 *  the device if the packet is a broadcast. Verify the packet length and
 *  discard if abnormal, otherwise set the packet length from the header.
 *  Strip off the hardware encapsulation and pass the the packet up to the
 *  appropriate higher level protocol handler.  Release the processor at the
 *  completion  of processing to allow any other processes to run before we
 *  look for the next packet.
 */

tr_input(dv)
struct device *dv;
{
    for (;;)
    {
	register struct packet *p;
 	register struct trpacket *trr;
	int br;

	br = spldv(dv);
	while ((p=(struct packet *)dequeue(&dv->dv_rq)) == 0)
	    sleep(&dv->dv_rq);
	spl(br);

#ifdef TRPDEBUG
	if (debugflags&TRACE_DV)
	    p->p_flag |= P_TRACE;
#endif TRPDEBUG

	dv->dv_cnts->dr_rq=lengthqueue(&dv->dv_rq);
	if (lengthqueue(&dv->dv_rq) > dv->dv_cnts->dr_rqmax)
		dv->dv_cnts->dr_rqmax=lengthqueue(&dv->dv_rq);

	trr = poff(trpacket, p);
	if (isbroadcast(dv, trr->tr_dhost))
	{
	    p->p_flag |= P_BCAST;
	    dv_profile(dv,dr_rbcast);
	}

	if (p->p_len < TRMINLEN || p->p_len > TRMAXLEN)
	{
	    char temp[20];

#ifdef	TRPDEBUG
	    printf("[ tr bad length %d (%x) ]\r\n",p->p_len, p);
	    tr_prt(trr, p->p_len);
#endif	TRPDEBUG
	    profile(dv,dr_rlen);
	    (*(p->p_done))(p);
	    goto out;
	}
#ifdef	TRPDEBUG
	if (p->p_flag&P_TRACE)
	{
	    printf("TRR (%lx): (%d)", dv->dv_addr, p->p_len);
	    tr_prt(trr, p->p_len);
	}
#endif	TRPDEBUG
	padjust(p, -TRHEADLEN);
/* BUG tr_dsap or tr_ssap? */
	switch (trr->tr_dsap)
	{
	    case TRT_AR:
		ar_input(dv, p, trr->tr_shost);
		break;
	    case TRT_IP:
	    case TRT_FAKEIP:
		ip_input(dv, p, trr->tr_shost);
		break;
	    default:
		profile(dv,dr_rproto);
                errorlog(p,EL_DR_RPROTO);
		(*(p->p_done))(p);
		break;
	}
    out:
	prelease();
    }
}



/*
 *  tr_output - send a packet on the Interlan 10Mb ethernet
 *
 *  dv    = device on which to transmit packet
 *  p     = packet to transmit
 *  pr    = protocol to use
 *  dhost = physical destination address for packet
 *
 *  Encapsulate the packet in the device header with the appropriate
 *  destination address and packet type (as per protocol) and my hardware
 *  address as the source address.  Finally, transmit the packet if the
 *  transmitter is currently idle, otherwise add it to the transmit queue.
 *
 *  Return: DVO_DOWN immediately without changing the packet if the device is
 *  currently unavailable, otherwise DVO_QUEUED.
 */

tr_output(dv, p, pr, dhost)
struct device *dv;
struct packet *p;
int pr;
char *dhost;
{
    register struct trpacket *trx;
    register char *from;
    register char *to;
    int i;
    int br;

    if ((dv->dv_istatus&DV_ONLINE) == 0)
    {
	profile(dv,dr_xierr);
	return(DVO_DEAD);
    }

    if (p->p_len > TR_MTU)
    {
	cprintf ("tr (%lx): oversize packet\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    if (pfull(&dv->dv_xq))
    {
/*	cprintf ("tr (%lx): xmit queue full\r\n", dv->dv_addr);*/
	profile(dv,dr_xnerr);
/*	panic("tr Q full");*/
	return(DVO_DROPPED);
    };

    padjust(p, TRHEADLEN);
    if (p->p_len < TRMINLEN) p->p_len=TRMINLEN;

    trx = (struct trpacket *) poff(trpacket, p);
    trx->tr_pcf0 = 0;
    trx->tr_pcf1 = 0x40;	/* BUG */
    bcopy(dhost, trx->tr_dhost, TR_HLN);
    bcopy(dv->dv_phys, trx->tr_shost, TR_HLN);
    trx->tr_dsap = trx->tr_ssap =		/* BUG: Fix ip types */
	dv->dv_pr[pr]==TRT_FAKEIP?TRT_IP:dv->dv_pr[pr];
    trx->tr_llc_ctl = 0xC0;	/* BUG */

#ifdef	TRPDEBUG
	if (p->p_flag&P_TRACE)
	{
	    printf("TRX (%lx): (%d)", dv->dv_addr, p->p_len);
	    tr_prt(trx, p->p_len);
	}
#endif	TRPDEBUG
    profile(dv,dr_xcnt);
    br = spldv(dv);

    dv->dv_cnts->dr_xq=lengthqueue(&dv->dv_xq);
    if (lengthqueue(&dv->dv_xq) > dv->dv_cnts->dr_xqmax)
	dv->dv_cnts->dr_xqmax=lengthqueue(&dv->dv_xq);

    if (dv->dv_xp == 0)
    {
	dv->dv_xp = p;
	trstart(dv);
    }
    else
    {
	enqueue(&dv->dv_xq, p);
    }
    spl(br);
    return(DVO_QUEUED);
}



#ifdef	TRPDEBUG
/*
 *  tr_prt - display an Interlan 10Mb ethernet packet header
 *
 *  trp = packet header to display
 *  len = length of packet
 */
tr_prt(trp, len)
register struct trpacket *trp;
{
    printf("(%x,%x) %02x%02x%02x%02x%02x%02x=>%02x%02x%02x%02x%02x%02x (%x,%x,%x)\r\n",
	    trp->tr_pcf0,trp->tr_pcf1,
	    ((u_char *)trp->tr_shost)[0], ((u_char *)trp->tr_shost)[1],
	    ((u_char *)trp->tr_shost)[2], ((u_char *)trp->tr_shost)[3],
	    ((u_char *)trp->tr_shost)[4], ((u_char *)trp->tr_shost)[5],
	    ((u_char *)trp->tr_dhost)[0], ((u_char *)trp->tr_dhost)[1],
	    ((u_char *)trp->tr_dhost)[2], ((u_char *)trp->tr_dhost)[3],
	    ((u_char *)trp->tr_dhost)[4], ((u_char *)trp->tr_dhost)[5],
	    trp->tr_dsap,trp->tr_ssap,trp->tr_llc_ctl);
}
#endif	TRPDEBUG

#endif	C_TR
