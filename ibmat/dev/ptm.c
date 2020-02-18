#include "cond/wt.h"

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"
#include "../../h/time.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../dev/ptmreg.h"
#include "../mch/device.h"


long time     = 0;
int ticks     = 0;
int tinterval = 0;

struct queue timerq = INITQUEUE(&timerq);
struct queue alarmq = INITQUEUE(&alarmq);


ptmreset (dv, flag)
register struct device *dv;
register int flag;
{
    register struct ptmdevice *reg = (struct ptmdevice *)dv->dv_addr;
    short scale = PTM_SCALE;
    register u_char *b = (u_char *)&scale;

#ifndef IBMAT
/* Nothing to do for the IBMAT */
    if (flag &DVR_ON) {
	reg->ptm_msb1 = b[0];
	reg->ptm_cnt1 = b[1];
	reg->ptm_csr2 = PTM_REG1;
	reg->ptm_csr1 = (PTM_CONT|PTM_NORESET|PTM_IRQENA|PTM_INTERN);
    }

    else if (flag & DVR_OFF) {
	reg->ptm_csr2 = PTM_REG1;
	reg->ptm_csr1 = PTM_DISABLE;
    }
#endif IBMAT
}

ptmintr(dv)
struct device *dv;
{
    register struct ptmdevice *reg = (struct ptmdevice *)dv->dv_addr;
    register int i;

    if (++ticks >= TINTERVAL) {
	ticks -= TINTERVAL;
	    for (i = --(((struct alarm *)(timerq.q_head))->al_incr);
	         i == 0;
		 i = ((struct alarm *)(timerq.q_head))->al_incr)
	    {
		register struct alarm *al;

		al = (struct alarm *)dequeue(&timerq);
		enqueue(&alarmq, al);
		wakeup(&alarmq);
		if (lengthqueue(&timerq) <= 0)
		    break;
	    }
	    if (++tinterval >= SECOND) {
		time++;
		tinterval -= SECOND;
	    }
    }
#ifndef IBMAT
    i = reg->ptm_csr2;
    i = reg->ptm_msb1;
#endif IBMAT
}
