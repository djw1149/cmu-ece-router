/*++*
 * module vv.c
 * abstract Proteon PROnet Driver for P1200 multibus adapter
 * created January 14, 1984
 * by Gregg Lebovitz at Carnegie Mellon University
 *
 *--*/
/*++*
 * History
 *  5 Dec 85 Matt Mathis (mathis) at CMU
 * 	Removed the per protocol statistics structure.  Replaced it
 *	with shared per device statistics
 *
 * Wed 20 Nov 1985 Matt Mathis CMU ECE dept
 *	Fixed some residual bugs in xintr.
 *
 * date Tur Sep 26 1985
 * by:  Matt Mathis
 *	Updated to use the device independant instrumentation, where it 
 *	makes sense.
 *
 * date Thu Jan 17 23:52:03 1985
 * by   Gregg Lebovitz
 * version 0E(a)
 *
 *	created experimental version from Eric Cranes Unibus driver.
 *
 *--*/

#include "cond/vv.h"

#if	C_VV > 0

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"
#include "../../h/time.h"

#include "../mch/mch.h"
#include "../mch/intr.h"

#include "../dev/vvreg.h"
#include "../dev/vv.h"
#include "../../h/devstats.h"
#include "../mch/device.h"

#include "../../h/profile.h"

#include "debug/vv.h"


short VVPOFF = 0;	/* initial receive packet offset */
short VVRWC  = 1023;	/* initial receive word count register value */

/*
 *  vvreset - proNET p1000 device reset 
 *
 *  dv   = device to reset
 *  flag = reset parameter
 *
 *  Reset the transmit and receive devices and flush 
 *  the transmit packet queue.
 *  If we are turning the device on, enable the receiver by starting a read
 *  operation.
 */

vvreset(dv,flag)
register struct device *dv;
int flag;
{
    register struct vvdevice *addr = (struct vvdevice *)dv->dv_addr;

#ifdef	VVDEBUG
	if (debugflags & TRACE_IL)
	    cprintf("DBG: vvreset xcsr = %x rcsr = %x\r\n",
		addr->vv_xcsr,addr->vv_rcsr);
#endif	VVDEBUG
    addr->vv_rcsr = VV_RESET | VV_HOST;
    addr->vv_xcsr = VV_RESET | VV_CLEAR;
    DELAY(5000);		/* Wait for the dust to clear */
				/* Is this needed? */
    dvflush(dv, DVFL_X);
    dv->dv_istatus &= ~(DV_ONLINE|DV_ENABLED);
    dv->dv_vv.vv_ricnt = 0;
    if (flag&DVR_ON)
    {
	int br;

	dv->dv_istatus |= (DV_ONLINE|DV_ENABLED);
	br = spldv(dv);		/* this is probably unnecessary */
	vvrstart(dv);
	spl(br);
    }
}


    
/*
 *  vvrstart - receive packet from proNET 
 *
 *  dv = device on which to start receive
 *
 *  Start a receive in the current input packet, beginning at the default
 *  packet offset for the maximum length.
 *
 */

vvrstart(dv)
register struct device *dv;
{
    register struct vvdevice *addr = (struct vvdevice *)dv->dv_addr;
    register struct packet *rp = dv->dv_rp;
    register u_short t;

#ifdef	VVDEBUG
	if (debugflags & TRACE_IL)
	    cprintf("DBG: vvrstart rcsr = %x\r\n",addr->vv_rcsr);
#endif	VVDEBUG
    addr->vv_rba = (u_long) swal(&(rp->p_ba)[VVPOFF]);
    addr->vv_rwc = (u_short) swab(VVRWC);
    t = (VV_HOST | VV_INT | VV_DMA | VV_COPY) & (~VV_RESET);
    addr->vv_rcsr = t;
}


vvintr(dv)
register struct device *dv;
{
    register struct vvdevice *reg = (struct vvdevice *)dv->dv_addr;

    if (reg->vv_rcsr&VV_IRESET)
    	vvrintr(dv);

    if (reg->vv_xcsr&VV_IRESET)
    	vvxintr(dv);
}

/*
 *  vvrintr - proNET p1000 receive interrupt handler
 *
 *  dv = interrupting device
 *
 *  If no receive is currently pending, record an unsolicited receive
 *  interrupt.  Otherwise, check for any receive errors and record the
 *  appropriate type.  If a packet has been received without error, set the
 *  length from the residual word count and add it to the device input queue for
 *  further protocol processing.  If the packet is too small, too many input
 *  packets are already queued, or a new packet cannot be allocated for the
 *  next receive, drop the current input packet and re-use it for the next
 *  receive.  In any case, start another receive using the newly allocated or
 *  re-user packet.
 *
 *  This routine is called in kernel mode at device interrupt level.
 *
 *  TODO: optimize to start next receive as soon as posisble.
 */

vvrintr(dv)
register struct device *dv;
{
    register struct vvdevice *addr = (struct vvdevice *)dv->dv_addr;
    register struct packet *rp = dv->dv_rp;
    extern long time;

#ifdef	VVDEBUG
	if (debugflags & TRACE_IL)
	    cprintf("DBG: vvrintr rcsr = %x\r\n",addr->vv_rcsr);
#endif	VVDEBUG
    /*
     *  If we have received too many interrupts in the same second (e.g. the
     *  device is continuously interrupting), silence it for a while so that it
     *  doesn't continue to adversely affect the rest of the system.  The
     *  device will be reenabled for input by the input handler after a decent
     *  interval has elapsed.
     */
    if ((int)time == dv->dv_vv.vv_ritime)
    {
	if (++(dv->dv_vv.vv_ricnt) >= VVRIMAX)
	{
	    addr->vv_rcsr = VV_RESET | VV_HOST;
	    dv->dv_vv.vv_ricnt = 0;
	    dv->dv_istatus |= DV_SILENCED;	/* notify input handler */
	    wakeup(&dv->dv_rq);
	    return;
	}
    }
    else
    {
	dv->dv_vv.vv_ricnt = 0;
	dv->dv_vv.vv_ritime = (int)time;
    }

    if (rp == 0)
    {
	dv_profile(dv,dr_unsol);
	return;
    }
    if (addr->vv_rcsr&VV_IERR)
    {
	if (addr->vv_rcsr&VV_PARITY)	/* Parity error */
	{
	    dv_profile(dv,dr_rnerr);
	    addr->vv_rcsr &= ~(VV_PARITY | VV_RESET);
	}
	if (addr->vv_rcsr&VV_NXM)	/* Non-existant memory */
	    dv_profile(dv,dr_rierr);
	if (addr->vv_rcsr&VV_OVR)	/* DMA overun */
	    dv_profile(dv,dr_rierr);
	if (addr->vv_rcsr&VV_BAF)	/* Bad message format */
	    dv_profile(dv,dr_rnerr);
	if (addr->vv_rcsr&VV_RNOK)
	    dv_profile(dv,dr_rnerr);	/* Ring not OK */
	if (addr->vv_rcsr&VV_NIR)
	    dv_profile(dv,dr_rnerr);	/* Not in ring */
	cprintf("DBG: vvrintr error = %x\r\n",addr->vv_rcsr);
	addr->vv_rcsr = VV_RESET | VV_HOST;
    }
    else
    {
	rp->p_off = VVPOFF;
	rp->p_len = (-swab(addr->vv_rwc)&0xffff) << 1;
	if (rp->p_len < sizeof(struct vvpacket))
	{
	    dv_profile(dv,dr_rlen);
	}
	else {
	dv_profile(dv,dr_rcnt);
	if (pfull(&dv->dv_rq) || (dv->dv_rp = palloc()) == 0)
	  {
	    dv_profile(dv,dr_rdrop);
	    dv->dv_rp = rp;
	 }
	 else
	  {
	    enqueue(&dv->dv_rq, rp);
	    wakeup(&dv->dv_rq);
	  }
	}
    }
    vvrstart(dv);
}



/*
 *  vvxstart - transmit packet on proNET p1000
 *
 *  dv = device on which to transmit
 *
 *  Transmit the current output packet from the appropriate offset and for the
 *  indicated length.
 *
 */

vvxstart(dv)
register struct device *dv;
{
    register struct vvdevice *addr = (struct vvdevice *)dv->dv_addr;
    register struct packet *xp = dv->dv_xp;
    register u_short t;

#ifdef	VVDEBUG
	if (debugflags & TRACE_IL)
	    cprintf("DBG: vvxstart xcsr = %x\r\n",addr->vv_xcsr);
#endif	VVDEBUG
    addr->vv_xba = (u_long) swal(&(xp->p_ba)[xp->p_off]);
    addr->vv_xwc = swab((xp->p_len+1) >> 1);
    t =  VV_INT | VV_CLEAR | VV_DMA | VV_INITR | VV_ORIG;
    addr->vv_xcsr = t;
}



/*
 *  vvxintr - proNET p1000 transmit interrupt handler
 *
 *  dv = interrupting device
 *
 *  If no transmit is currently pending, record an unsolicited transmit
 *  interrupt.  Otherwise, check for any transmit errors and record the
 *  appropriate type or a successful transmission.  In any case, move the next
 *  packet in the transmit queue into the current transmit packet (if any),
 *  start transmission and then free the previously completed packet.
 *
 *  This routine is called in kernel mode at device interrupt level.
 */

vvxintr(dv)
struct device *dv;
{
    register struct vvdevice *addr = (struct vvdevice *)dv->dv_addr;
    register struct packet *xp = dv->dv_xp;
    register t;

#ifdef	VVDEBUG
	if (debugflags & TRACE_IL)
	    cprintf("DBG: vvxintr xcsr = %x\r\n",addr->vv_xcsr);
#endif	VVDEBUG
    if (xp == 0)
    {
	dv_profile(dv,dr_unsol);
	addr->vv_xcsr = VV_RESET;
	return;
    }
    if (addr->vv_xcsr&VV_OERR)
    {
	if (addr->vv_xcsr&VV_REFS){	/* destination refused */
	    dv_profile(dv,dr_jcnt);
	    if (dv->dv_vv.vv_retry-- > 0) {
		vvxstart(dv);
		return;
	    }
	}
	if (addr->vv_xcsr&VV_NXM)	/* Non existant memory */
	    dv_profile(dv,dr_xierr);
	if (addr->vv_xcsr&VV_OVR)	/* DMA over run */
	    dv_profile(dv,dr_xierr);
	if (addr->vv_xcsr&VV_TMOUT)	/* Timeout */
	    dv_profile(dv,dr_xnerr);
	if (addr->vv_xcsr&VV_RNOK)	/* Ring not ok (loss of token) */
	    dv_profile(dv,dr_xnerr);
	if (addr->vv_xcsr&VV_NIR)	/* not connected to ring */
	    dv_profile(dv,dr_xnerr);
	if (addr->vv_xcsr&VV_BAF)	/* Bad format of drained pkt */
	    dv_profile(dv,dr_xierr);	/* May be our problem */
	cprintf("DBG: vvxintr error = %x\r\n",addr->vv_xcsr);
	/*
	 *  Perhaps this will prevent the net from going into infinite
	 *  broadcast mode.
	 */
	addr->vv_xcsr = VV_RESET;
    }

    if ((dv->dv_xp=(struct packet *)dequeue(&dv->dv_xq)) != 0){
	dv->dv_vv.vv_retry=VVRETRY;
	vvxstart(dv);					/* start the next */
    }
    else {
	addr->vv_xcsr = VV_IRESET | VV_RESET;		/* else idle */
    }
    (*(xp->p_done))(xp);				/* free prior packet*/
}
#endif	C_VV
