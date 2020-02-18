/*
 * Proteon PRONet Interface Protocol I/O Module
 */
/* HISTORY
 * Sep 26, 1985 and 9-Dec-1985
 * by Matt Mathis
 *	Updated device independant instrumentation
 *
 * Aug 1985
 * written by Dave Bohman
 */

#include "cond/vv.h"
#include "cond/chaos.h"

#if	C_VV > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/vvreg.h"
#include "../dev/vv.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/packet.h"

#include "../../h/proc.h"
#include "../../h/errorlog.h"
#include "../../h/profile.h"

#include "debug/vv.h"


/*
 *  vv_input - input packet process
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

vv_input(dv)
struct device *dv;
{
    for (;;)
    {
	register struct packet *p;
 	register struct vvpacket *vvr;
	int br;

	br = spldv(dv);
	while ((p=(struct packet *)dequeue(&dv->dv_rq)) == 0)
	    sleep(&dv->dv_rq);
	spl(br);

	dv->dv_cnts->dr_rq=lengthqueue(&dv->dv_rq);
	if (lengthqueue(&dv->dv_rq) > dv->dv_cnts->dr_rqmax)
		dv->dv_cnts->dr_rqmax=lengthqueue(&dv->dv_rq);

	vvr = poff(vvpacket, p);
/*	cprintf("vv_input: s %d d %d t %d\r\n", vvr->vv_shost, vvr->vv_dhost, vvr->vv_type);*/
	if (isbroadcast(dv, &vvr->vv_dhost))
	{
	    p->p_flag |= P_BCAST;
	    dv_profile(dv,dr_rbcast);
	}

	if (p->p_len > VVPLEN)
	{
	    char temp[20];

	    printf("[ %s vv bad length %d (%x) ]\r\n",
	    	   timeup(temp), p->p_len, p);
	    profile(dv,dr_rlen);
	    (*(p->p_done))(p);
	    goto out;
	}
	padjust(p, -VVHEAD);
	switch (vvr->vv_type)
	{
	    case VVT_AR:
		ar_input(dv, p, &vvr->vv_shost);
		break;
	    case VVT_IP:
		ip_input(dv, p, &vvr->vv_shost);
		break;
	    case VVT_RWAY:
	        profile(dv,dr_rproto);	/* Ringway: Count it as unknown, but*/
		(*(p->p_done))(p);	/* Don't bother logging it */
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
 *  vv_output - send a packet on the Interlan 10Mb ethernet
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

vv_output(dv, p, pr, dhost)
struct device *dv;
struct packet *p;
int pr;
char *dhost;
{
    register struct vvpacket *vvx;
    register char *from;
    register char *to;
    int i;
    int br;

    if ((dv->dv_istatus&DV_ONLINE) == 0)
    {
	profile(dv,dr_xierr);
	return(DVO_DEAD);
    }

    if (p->p_len > VV_MTU)
    {
	cprintf ("vv (%x): oversize packet\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    dv->dv_cnts->dr_xq=lengthqueue(&dv->dv_xq);
    if (lengthqueue(&dv->dv_xq) > dv->dv_cnts->dr_xqmax)
	dv->dv_cnts->dr_xqmax=lengthqueue(&dv->dv_xq);

    if (pfull(&dv->dv_xq))
    {
	printf ("vv (%x): xmit queue full\r\n", dv->dv_addr);
	profile(dv,dr_xierr);
	return(DVO_DROPPED);
    };

    padjust(p, VVHEAD);
    vvx = poff(vvpacket, p);
    bcopy(dhost, &vvx->vv_dhost, VV_HLN);
    bcopy(dv->dv_phys, &vvx->vv_shost, VV_HLN);
    vvx->vv_version = VV_VERSION;
    vvx->vv_inf = 0;
    vvx->vv_type = dv->dv_pr[pr];
/*    cprintf("vv_output: s %d d %d t %d\r\n", vvx->vv_shost, vvx->vv_dhost, vvx->vv_type);*/

    dv_profile(dv,dr_xcnt);
    br = spldv(dv);
    if (dv->dv_xp == 0)
    {
	dv->dv_xp = p;
	dv->dv_vv.vv_retry=VVRETRY;
	vvxstart(dv);
    }
    else
    {
	enqueue(&dv->dv_xq, p);
    }
    spl(br);
    return(DVO_QUEUED);
}
#endif	C_VV
