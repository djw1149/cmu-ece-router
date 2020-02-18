#include "cond/ec.h"

/*
 * 3Com Multibus Ethernet interface driver.
 * HISTORY
 * 5-Dec-85 Matt Mathis at CMU
 *	Updated to use shared devstats structure
 * Written 1/26/85 by David Bohman
 */

#if	C_EC > 0


#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ecreg.h"
#include "../dev/ec.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/profile.h"
#include "../../h/globalsw.h"





/* #include "../h/psw.h" */

u_short ECPOFF = 0;	/* initial receive packet offset */

/*
 *  ecreset()
 *
 * The board has already been reset in the probe routine, so
 * just start a receive into buffer A.
 */

ecreset(dv, flag)
register struct device *dv;
int flag;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;
    register struct packet *rp;
    int map;

    map = mapIO();
    dvflush(dv, DVFL_X);
    if (flag&DVR_ON)
    {
	int br;

	br = spldv(dv);
	dv->dv_istatus |= DV_ONLINE;
	SET_BIT(reg->ec_csr.csr_absw);
	SET_BIT(reg->ec_csr.csr_ainten);
	SET_BIT(reg->ec_csr.csr_bbsw);
	SET_BIT(reg->ec_csr.csr_binten);
	spl(br);
    }
    remap(map);
}

/*
 * ecintr()
 *
 * Field an interrupt, and dispatch to the correct routine.
 */

ecintr(dv)
register struct device *dv;
{
    register ipl = spl6();
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;

    if (reg->ec_csr.csr_absw == 0 || reg->ec_csr.csr_bbsw == 0)
    	ecrintr(dv);
    if (reg->ec_csr.csr_jam)
    	ecjintr(dv);
    if (reg->ec_csr.csr_tinten && reg->ec_csr.csr_tbsw == 0)
    	ecxintr(dv);
    spl(ipl);
}
    
/*
 *  ecrintr()
 *
 *  Start a receive on the other receive buffer, and read the new
 *  data into a packet.
 */

ecrintr(dv)
register struct device *dv;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;
    register struct packet *rp;
    register u_short *sp;
    register u_long len;
    int bb;

again:
    rp = dv->dv_rp;
    if (reg->ec_csr.csr_absw == 0 && reg->ec_csr.csr_bbsw == 0) {
	if (reg->ec_csr.csr_rbba == 0) {
	    CLEAR_BIT(reg->ec_csr.csr_ainten);
	    sp = (u_short *)reg->ec_rbufa;
	    bb = 0;
	} else {
	    CLEAR_BIT(reg->ec_csr.csr_binten);
	    sp = (u_short *)reg->ec_rbufb;
	    bb = 1;
	}
    } else {
	if (reg->ec_csr.csr_absw == 0) {
	    CLEAR_BIT(reg->ec_csr.csr_ainten);
	    sp = (u_short *)reg->ec_rbufa;
	    bb = 0;
	} else if (reg->ec_csr.csr_bbsw == 0) {
	    CLEAR_BIT(reg->ec_csr.csr_binten);
	    sp = (u_short *)reg->ec_rbufb;
	    bb = 1;
	}
    }

    len = *sp++;
    len = (len&0x7ff) - 1;
    rp->p_off = ECPOFF;
    rp->p_len = len;
 if (debugflags & TRACE_IL) cprintf ("ec (%x) rintr %d", reg, len);
    bcopy(sp, rp->p_ba, len);
    dv_profile(dv,dr_rcnt);
    if (pfull(&dv->dv_rq) || (dv->dv_rp = palloc()) == 0) {
	dv->dv_rp = rp;
	dv_profile(dv,dr_rdrop);
 if (debugflags & TRACE_IL) cprintf ("[dropped]\r\n");
    } else {
 if (debugflags & TRACE_IL) cprintf ("[ok]\r\n");
	enqueue(&dv->dv_rq, rp);
	wakeup(&dv->dv_rq);
    }
    if (bb) {
    	SET_BIT(reg->ec_csr.csr_bbsw);
	SET_BIT(reg->ec_csr.csr_binten);
    } else {
    	SET_BIT(reg->ec_csr.csr_absw);
	SET_BIT(reg->ec_csr.csr_ainten);
    }
    if (reg->ec_csr.csr_absw == 0 || reg->ec_csr.csr_bbsw == 0)
    	goto again;
}

/*
 *  ecstart()
 *
 *  Start a transmit operation.
 */

ecstart(dv)
register struct device *dv;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;
    register struct packet *xp = dv->dv_xp;
    register len = xp->p_len;
    register u_short *sp = (u_short *)reg->ec_tbuf;

if (debugflags & TRACE_IL) cprintf ("ec (%x) xstart %d\r\n", reg, len);

    *sp = 2048 - len;
    bcopy(poff(u_char *, xp), &reg->ec_tbuf[2048 - len], len);
    SET_BIT(reg->ec_csr.csr_tbsw);
    SET_BIT(reg->ec_csr.csr_tinten);
    SET_BIT(reg->ec_csr.csr_jinten);
    dv->dv_ec.ec_jam = ~0;
}

/*
 *  ecxintr()
 *
 *  Process a packet transmit operation, and start the next transfer
 */

ecxintr(dv)
register struct device *dv;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;
    register struct packet *xp = dv->dv_xp;

    CLEAR_BIT(reg->ec_csr.csr_tinten);
    CLEAR_BIT(reg->ec_csr.csr_jinten);
    dv->dv_xp = (struct packet *)dequeue(&dv->dv_xq);
    (*(xp->p_done))(xp);
if (debugflags & TRACE_IL) cprintf ("ec (%x) xintr\r\n", reg);
    if (dv->dv_xp)
	ecstart(dv);
}

/*
 * ecjintr()
 *
 * A jam (collision) has occurred during a transmit.  Compute the backoff
 * time, and try to resend the packet.  If we have already tried again 15
 * times, then drop the packet.
 */

ecjintr(dv)
register struct device *dv;
{
    register struct ecdevice *reg = (struct ecdevice *)dv->dv_addr;

    dv_profile(dv,dr_jcnt);
    if ((dv->dv_ec.ec_jam <<= 1) == 0) {
	SET_BIT(reg->ec_csr.csr_reset);
	reg->ec_csr.csr_pa = EC_PA_MINE_BROAD_NERR;
	SET_BIT(reg->ec_csr.csr_amsw);
	SET_BIT(reg->ec_csr.csr_absw);
	SET_BIT(reg->ec_csr.csr_ainten);
	SET_BIT(reg->ec_csr.csr_bbsw);
	SET_BIT(reg->ec_csr.csr_binten);
	dv_profile(dv,dr_xnerr);
	ecxintr(dv);
    } else {
	cprintf("ec (%x) jam backoff %x\r\n", reg, (~dv->dv_ec.ec_jam)&0xffff);
	reg->ec_back = -(~dv->dv_ec.ec_jam);
	SET_BIT(reg->ec_csr.csr_jam);
    }
}
#endif	C_EC
