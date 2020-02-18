/*
 *  Interlan 10Mb ethernet protocol I/O module
 *
 **********************************************************************
 * HISTORY
 *  5 Dec 85 Matt Mathis (mathis) at CMU
 * 	Removed the per protocol statistics initialization.  Replaced it
 *	with shared per device statistics, initialized in autoconf
 *	Added CTP, CHAOS, and XPUP stubs.
 *
 * 10-Sep-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Revised to use new Interlan 10Mb ethernet specific device
 *	dependent fields.
 *
 * 09-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Conditionalized CHAOSnet protocol support.
 *
 **********************************************************************
 */

#include "cond/il.h"
#include "cond/chaos.h"

#if	C_IL > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ilreg.h"
#include "../dev/il.h"
#include "../../h/devstats.h"
#include "../mch/device.h"

#include "../../h/packet.h"
#include "../../h/errorlog.h"
#include "../../h/profile.h"

#include "debug/il.h"
/* #include "../../h/profile.h" 	* why twice	*/
/* #include "../h/psw.h" */
/* #include "../mch/proc.h" */


/*
 *  il_input - Interlan 10Mb ethernet input packet process
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

il_input(dv)
struct device *dv;
{
    for (;;)
    {
	register struct packet *p;
 	register struct ilpacket *ilr;
	int br;

	br = spldv(dv);
	while ((p=(struct packet *)dequeue(&dv->dv_rq)) == 0)
	    sleep(&dv->dv_rq);
	spl(br);

	dv->dv_cnts->dr_rq=lengthqueue(&dv->dv_rq);
	if (lengthqueue(&dv->dv_rq) > dv->dv_cnts->dr_rqmax)
		dv->dv_cnts->dr_rqmax=lengthqueue(&dv->dv_rq);

	ilr = poff(ilpacket, p);
	if (isbroadcast(dv, ilr->il_dhost))
	{
	    p->p_flag |= P_BCAST;
	    dv_profile(dv,dr_rbcast);
	}

	if (p->p_len < ILMINLEN || p->p_len > ILMAXLEN)
	{
	    char temp[20];

	    printf("[ %s il bad length %d (%x) ]\r\n",
	    	   timeup(temp), p->p_len, p);
#ifdef	ILDEBUG
	    il_prt(ilr, p->p_len);
#endif	ILDEBUG
	    profile(dv,dr_rlen);
	    (*(p->p_done))(p);
	    goto out;
	}
#ifdef	ILDEBUG
	if (p->p_flag&P_TRACE)
	{
	    printf("RIL (%d): ", p->p_len);
	    il_prt(ilr, p->p_len);
	}
#endif	ILDEBUG
	padjust(p, -ILHEADLEN);
	switch (ilr->il_type)
	{
	    case ILT_AR:
		ar_input(dv, p, ilr->il_shost);
		break;
	    case ILT_IP:
		ip_input(dv, p, ilr->il_shost);
		break;
	    case ILT_CHAOS:
		profile(dv,cn_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case ILT_CTP:
		profile(dv,ct_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case ILT_XPUP:
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
 *  il_output - send a packet on the Interlan 10Mb ethernet
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

il_output(dv, p, pr, dhost)
struct device *dv;
struct packet *p;
int pr;
char *dhost;
{
    register struct ilpacket *ilx;
    register char *from;
    register char *to;
    int i;
    int br;

    if ((dv->dv_istatus&DV_ONLINE) == 0)
    {
	profile(dv,dr_xierr);
	return(DVO_DEAD);
    }

    if (p->p_len > IL_MTU)
    {
	cprintf ("il (%x): oversize packet\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    if (pfull(&dv->dv_xq))
    {
	cprintf ("il (%x): xmit queue full\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	ilxwedged(dv);		/* Give it a goose */
	return(DVO_DROPPED);
    };

    padjust(p, ILHEADLEN);
    p->p_len = MAX(p->p_len, ILMINLEN);
    ilx = poff(ilpacket, p);
    bcopy(dhost, ilx->il_dhost, IL_HLN);
    bcopy(dv->dv_phys, ilx->il_shost, IL_HLN);
    ilx->il_type = dv->dv_pr[pr];
#ifdef	ILDEBUG
    if (p->p_flag&P_TRACE)
    {
	cprintf("XIL (%x): len=%d ", dv->dv_addr, p->p_len);
	il_prt(ilx, p->p_len);
    }
#endif	ILDEBUG
    profile(dv,dr_xcnt);
    br = spldv(dv);

    dv->dv_cnts->dr_xq=lengthqueue(&dv->dv_xq);
    if (lengthqueue(&dv->dv_xq) > dv->dv_cnts->dr_xqmax)
	dv->dv_cnts->dr_xqmax=lengthqueue(&dv->dv_xq);

    if (dv->dv_xp == 0)
    {
	dv->dv_xp = p;
	ilstart(dv);
    }
    else
    {
	enqueue(&dv->dv_xq, p);
    }
    spl(br);
    return(DVO_QUEUED);
}



#ifdef	ILDEBUG
/*
 *  il_prt - display an Interlan 10Mb ethernet packet header
 *
 *  ilp = packet header to display
 *  len = length of packet
 */
il_prt(ilp, len)
register struct ilpacket *ilp;
{
    printf("%02x%02x%02x%02x%02x%02x=>%02x%02x%02x%02x%02x%02x (%x)\r\n",
	    ((u_char *)ilp->il_shost)[0], ((u_char *)ilp->il_shost)[1],
	    ((u_char *)ilp->il_shost)[2], ((u_char *)ilp->il_shost)[3],
	    ((u_char *)ilp->il_shost)[4], ((u_char *)ilp->il_shost)[5],
	    ((u_char *)ilp->il_dhost)[0], ((u_char *)ilp->il_dhost)[1],
	    ((u_char *)ilp->il_dhost)[2], ((u_char *)ilp->il_dhost)[3],
	    ((u_char *)ilp->il_dhost)[4], ((u_char *)ilp->il_dhost)[5],
	    ilp->il_type);
}
#endif	ILDEBUG

#endif	C_IL
