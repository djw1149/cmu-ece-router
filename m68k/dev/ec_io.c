/*
 * 3Com Multibus Ethernet Interface Protocol I/O Module
 *
 * HISTORY
 *  5 Dec 85 Matt Mathis (mathis) at CMU
 * 	Removed the per protocol statistics structures.  Replaced it
 *	with shared per device statistics.
 *	Added CTP, CHAOS, and XPUP stubs.
 *
 * Written by Gregg Lebovitz
 */
#include "cond/ec.h"
#include "cond/chaos.h"

#if	C_EC > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"
#include "../../h/packet.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ecreg.h"
#include "../dev/ec.h"
#include "../mch/device.h"

#include "../../h/errorlog.h"
#include "../../h/devstats.h"
#include "../../h/profile.h"

#include "debug/ec.h"

/* #include "../mch/proc.h" */
/* #include "../h/psw.h" */

/*
 *  ec_input - Interlan 10Mb ethernet input packet process
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

ec_input(dv)
struct device *dv;
{
    for (;;)
    {
	register struct packet *p;
 	register struct ecpacket *ecr;
	int br;

	br = spldv(dv);
	while ((p=(struct packet *)dequeue(&dv->dv_rq)) == 0)
	    sleep(&dv->dv_rq);
	spl(br);

	dv->dv_cnts->dr_rq=lengthqueue(&dv->dv_rq);
	if (lengthqueue(&dv->dv_rq) > dv->dv_cnts->dr_rqmax)
		dv->dv_cnts->dr_rqmax=lengthqueue(&dv->dv_rq);

	ecr = poff(ecpacket, p);
	if (isbroadcast(dv, ecr->ec_dhost))
	{
	    p->p_flag |= P_BCAST;
	    dv_profile(dv,dr_rbcast);
	}

	if (p->p_len < ECRMINLEN || p->p_len > ECRMAXLEN)
	{
	    char temp[20];

	    printf("[ %s ec bad length %d (%x) ]\r\n",
	    	   timeup(temp), p->p_len, p);
#ifdef	ECDEBUG
	    ec_prt(ecr, p->p_len);
#endif	ECDEBUG
	    profile(dv,dr_rlen);
	    (*(p->p_done))(p);
	    goto out;
	}
#ifdef	ECDEBUG
	if (p->p_flag&P_TRACE)
	{
	    printf("REC (%d): ", p->p_len);
	    ec_prt(ecr, p->p_len);
	}
#endif	ECDEBUG
	padjust(p, -ECRHEAD);
	switch (ecr->ec_type)
	{
	    case ECT_AR:
		ar_input(dv, p, ecr->ec_shost);
		break;
	    case ECT_IP:
		ip_input(dv, p, ecr->ec_shost);
		break;
	    case ECT_CHAOS:
		profile(dv,cn_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case ECT_CTP:
		profile(dv,ct_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    case ECT_XPUP:
		profile(dv,xp_rcnt);
		(*(p->p_done))(p);	/* discarded */
		break;
	    default:
                profile(dv,dr_rproto);
                errorlog(p,EL_DR_RPROTO);
		(*(p->p_done))(p);	/* discarded */
		break;
	}
    out:
	prelease();
    }
}



/*
 *  ec_output - send a packet on the Interlan 10Mb ethernet
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

ec_output(dv, p, pr, dhost)
struct device *dv;
struct packet *p;
int pr;
char *dhost;
{
    register struct ecpacket *ecx;
    register char *from;
    register char *to;
    int i;
    int br;

    if ((dv->dv_istatus&DV_ONLINE) == 0)
    {
	profile(dv,dr_xierr);
	return(DVO_DEAD);
    }

    if (p->p_len > EC_MTU)
    {
	printf ("ec (%x): oversize packet\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    dv->dv_cnts->dr_xq=lengthqueue(&dv->dv_xq);
    if (lengthqueue(&dv->dv_xq) > dv->dv_cnts->dr_xqmax)
	dv->dv_cnts->dr_xqmax=lengthqueue(&dv->dv_xq);

    if (pfull(&dv->dv_xq))
    {
	printf ("ec (%x): xmit queue full\r\n", dv->dv_addr);
	panic("Out of xmit packets\n");		/* Hung interface */
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    padjust(p, ECXHEAD);
    p->p_len = MAX (p->p_len, ECRMINLEN);
    ecx = poff(ecpacket, p);
    bcopy(dhost, ecx->ec_dhost, EC_HLN);
    bcopy(dv->dv_phys, ecx->ec_shost, EC_HLN);
    ecx->ec_type = dv->dv_pr[pr];
#ifdef	ECDEBUG
    if (p->p_flag&P_TRACE)
    {
	printf("XEC (%x): len=%d ", dv->dv_addr, p->p_len);
	ec_prt(ecx, p->p_len);
    }
#endif	ECDEBUG
    dv_profile(dv,dr_xcnt);
    br = spldv(dv);
    if (dv->dv_xp == 0)
    {
	dv->dv_xp = p;
	ecstart(dv);
    }
    else
    {
	enqueue(&dv->dv_xq, p);
    }
    spl(br);
    return(DVO_QUEUED);
}



#ifdef	ECDEBUG
/*
 *  ec_prt - display an Interlan 10Mb ethernet packet header
 *
 *  ecp = packet header to display
 *  len = length of packet
 */
ec_prt(ecp, len)
register struct ecpacket *ecp;
{
    printf("%02x%02x%02x%02x%02x%02x=>%02x%02x%02x%02x%02x%02x (%x)\r\n",
	    ((u_char *)ecp->ec_shost)[0], ((u_char *)ecp->ec_shost)[1],
	    ((u_char *)ecp->ec_shost)[2], ((u_char *)ecp->ec_shost)[3],
	    ((u_char *)ecp->ec_shost)[4], ((u_char *)ecp->ec_shost)[5],
	    ((u_char *)ecp->ec_dhost)[0], ((u_char *)ecp->ec_dhost)[1],
	    ((u_char *)ecp->ec_dhost)[2], ((u_char *)ecp->ec_dhost)[3],
	    ((u_char *)ecp->ec_dhost)[4], ((u_char *)ecp->ec_dhost)[5],
	    ecp->ec_type);
}
#endif	ECDEBUG

#endif	C_EC
