/*
 *  Generic network Driver
 **********************************************************************
 */

#include "cond/chaos.h"

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../../h/proc.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"
#include "cond/ch.h"
#if	C_CH > 0

#include "../dev/chreg.h"
#include "../dev/ch.h"

#include "../mch/device.h"
#include "../../h/errorlog.h"
#include "../../h/profile.h"

#include "debug/ch.h"

#ifdef CHPDEBUG
int chdebug = CHPDEBUG;
#endif

/*
 *  ch_input - Generic input packet process
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

ch_input(dv)
struct device *dv;
{
    for (;;)
    {
	register struct packet *p;
 	register struct chpacket *chr;
	int br;

	br = spldv(dv);
	while ((p=(struct packet *)dequeue(&dv->dv_rq)) == 0)
	    sleep(&dv->dv_rq);
	spl(br);
#ifdef CHDEBUG
	printf("Got a packet, off %d, len %d\r\n",p->p_off, p->p_len);
#endif
	dv->dv_cnts->dr_rq=lengthqueue(&dv->dv_rq);
	if (lengthqueue(&dv->dv_rq) > dv->dv_cnts->dr_rqmax)
		dv->dv_cnts->dr_rqmax=lengthqueue(&dv->dv_rq);

	chr = poff(chpacket, p);
#ifdef CHPDEBUG
	if (chdebug)
	    p->p_flag |= P_TRACE;
#endif CHPDEBUG
	if (isbroadcast(dv, chr->ch_dhost))
	{
	    p->p_flag |= P_BCAST;
	    dv_profile(dv,dr_rbcast);
	}

	if (p->p_len < CHMINLEN || p->p_len > CHMAXLEN)
	{
	    char temp[20];

	    printf("[ ch bad length %d (%x) ]\r\n",p->p_len, p);

#ifdef	CHDEBUG
	    ch_prt(chr, p->p_len);
#endif	CHDEBUG
	    profile(dv,dr_rlen);
	    (*(p->p_done))(p);
	    goto out;
	}
#ifdef	CHDEBUG
	if (p->p_flag&P_TRACE)
	{
	    printf("RCH (%d): ", p->p_len);
	    ch_prt(chr, p->p_len);
	}
#endif	CHDEBUG
	padjust(p, -CHHEADLEN);
	switch (ntohs(chr->ch_type))
	{
	    case CHT_AR:
		ar_input(dv, p, chr->ch_shost);
		break;
	    case CHT_IP:
		ip_input(dv, p, chr->ch_shost);
		break;
	    case CHT_CHAOS:
		profile(dv,cn_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case CHT_CTP:
		profile(dv,ct_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case CHT_XPUP:
		profile(dv,xp_rcnt);
		(*(p->p_done))(p);	/* discarded */
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
 *  ch_output - send a packet on the Interlan 10Mb ethernet
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

ch_output(dv, p, pr, dhost)
struct device *dv;
struct packet *p;
int pr;
char *dhost;
{
    register struct chpacket *chx;
    register char *from;
    register char *to;
    int i;
    int br;

    if ((dv->dv_istatus&DV_ONLINE) == 0)
    {
	profile(dv,dr_xierr);
	return(DVO_DEAD);
    }

    if (p->p_len > CH_MTU)
    {
	cprintf ("ch (%lx): oversize packet\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    if (pfull(&dv->dv_xq))
    {
/*	cprintf ("ch (%lx): xmit queue full\r\n", dv->dv_addr);*/
	profile(dv,dr_xnerr);
/*	panic("ch Q full");*/
	return(DVO_DROPPED);
    };

    padjust(p, CHHEADLEN);
    if (p->p_len < CHMINLEN) p->p_len=CHMINLEN;

    chx = poff(chpacket, p);
    bcopy(dhost, chx->ch_dhost, CH_HLN);
    bcopy(dv->dv_phys, chx->ch_shost, CH_HLN);
    chx->ch_type = htons(dv->dv_pr[pr]);
#ifdef	CHDEBUG
    if (p->p_flag&P_TRACE)
    {
	cprintf("CHX (%4x:%4x): len=%d ", dv->dv_addr, p->p_len);
	ch_prt(chx, p->p_len);
    }
#endif	CHDEBUG
    profile(dv,dr_xcnt);
    br = spldv(dv);

    dv->dv_cnts->dr_xq=lengthqueue(&dv->dv_xq);
    if (lengthqueue(&dv->dv_xq) > dv->dv_cnts->dr_xqmax)
	dv->dv_cnts->dr_xqmax=lengthqueue(&dv->dv_xq);

    if (dv->dv_xp == 0)
    {
	dv->dv_xp = p;
	chstart(dv);
    }
    else
    {
	enqueue(&dv->dv_xq, p);
    }
    spl(br);
    return(DVO_QUEUED);
}



#ifdef	CHDEBUG
/*
 *  ch_prt - display an Interlan 10Mb ethernet packet header
 *
 *  chp = packet header to display
 *  len = length of packet
 */
ch_prt(chp, len)
register struct chpacket *chp;
{
    printf("%02x%02x%02x%02x%02x%02x=>%02x%02x%02x%02x%02x%02x (%x)\r\n",
	    ((u_char *)chp->ch_shost)[0], ((u_char *)chp->ch_shost)[1],
	    ((u_char *)chp->ch_shost)[2], ((u_char *)chp->ch_shost)[3],
	    ((u_char *)chp->ch_shost)[4], ((u_char *)chp->ch_shost)[5],
	    ((u_char *)chp->ch_dhost)[0], ((u_char *)chp->ch_dhost)[1],
	    ((u_char *)chp->ch_dhost)[2], ((u_char *)chp->ch_dhost)[3],
	    ((u_char *)chp->ch_dhost)[4], ((u_char *)chp->ch_dhost)[5],
	    chp->ch_type);
}
#endif	CHDEBUG

#endif	C_CH
