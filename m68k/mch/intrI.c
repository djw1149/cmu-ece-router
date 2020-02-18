/*	intrI.c
 * $Header: intrI.c,v 1.2.1.1 86/10/22 13:30:13 djw Exp $
 *
 * HISTORY:
 * 2-July-86  Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University moved contents from autoconfI.c
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"
#include "../../h/aconf.h"


/*
 *  Iprobe - test a device address for type and interrupt information
 *
 *  ac = autoconfiguration parameters for device
 *  ai = autoconfiguration CSR identification information for device
 *
 *  Iterate through BR levels 3-7 calling the device probe routine to cause an
 *  interrupt on the device at each level.  The device interrupt is processed
 *  by the unexpected interrupt handler which is enabled to call the Iintr()
 *  routine to reset the device state and record the interrupt vector address
 *  for future reference.  When the BR level reaches the BR level of the
 *  device, no interrupt will occur.  This will be detected by timing out on
 *  the wait for an interrupt (which is currently just a non-trivial loop) and
 *  the device interrupt level is then determined.  If the device never
 *  interrupts or never stops interrupting an error is indicated.
 *
 *  Returns a pointer to a probe structure containing the BR level and
 *  interrupt vector address of the device or 0 on error.  The returned
 *  interrupt vector address is "normalized" to point to the first vector in
 *  the block of vectors for the device based on the number of interrupt
 *  routines defined in the autoconfiguration structure (so that the probe
 *  routine may generate an interrupt though whichever of the device vectors is
 *  most convenient).
 */

static struct autoconf *curac = 0;	/* current device for Iintr() */
static p_autoid curai = 0;	/* current device for Iintr() */
static p_inter inter = {0};	/* return structure for Iintr() */

struct p_inter *
Iprobe(ac, ai)
register p_autoconf ac;
register p_autoid ai;
{
    int br;				/* current BR level */
    int tout = 1;			/* timeout count (0=>timed out) */
    register p_inter *pb = 0;		/* return value */
    extern Iintr(), (*introk)();

    curai = ai;
    introk = Iintr;

    spl0();
    curac = ac;
    (*(ac->ac_probe))(&(ai->devconf), PB_PROBE);
    for (tout=30000; --tout;)
    {
	/*
	 *  The current device pointer is cleared by the Iintr()
	 *  handler to indicate that an interrupt has occurred.
	 */
	if (curac == 0)
	{
	    pb = &inter;		/* remember the interrupt */
	    break;
	}
    }
    curac = 0;
    (*(ac->ac_probe))(&(ai->devconf), PB_RESET);
    introk = 0;

    if (pb == 0)
    {
	cprintf("didn't interrupt - not configured\r\n");
    }
    else
    {
	int mask = ~(sizeof (struct vector)-1);
	int nvec;

	for (nvec=0; nvec<ACMAXVEC; nvec++)
	    if (ac->ac_intr[nvec] == 0)
		break;
	if (nvec-- == 0)
	    panic("Iprobe");
	for (; nvec; nvec >>= 1)
	    mask <<= 1;
	pb->pb_vector = (struct vector *)((int)(pb->pb_vector)&mask);

    }
    return(pb);
}



/*
 *  Iintr - process a device probe interrupt
 *
 *  vector = interrupt vector address at which the interrupt occurred
 *
 *  If a device interrupt is expected (i.e. the current device pointer is non-
 *  null), reset the interrupting device, store the interrupt vector in the
 *  Iprobe() return structure and indicate that an interrupt has occurred by
 *  clearing the current device pointer.
 *
 *  Note: Some devices such as the KW11-L may still interrupt again after they
 *  have been reset once the interrupt handler returns and the processor
 *  priority is restored.  Presumably this occurs when an interrupt is pending
 *  while the interrupt enable bit is cleared (I'm not sure I really believe
 *  this is what is going on).  The routine thus checks to make sure an
 *  interrupt is expected before handling it.
 */

Iintr(vector)
struct vector *vector;
{
    register int v = (int)vector;
    extern (*introk)();

    if (curac)
    {
	(*(curac->ac_probe))(&(curai->devconf), PB_RESET);
	inter.pb_vector = vector;

	/* srval returns the current ipl */
	inter.pb_br = srval();
	curac = 0;
    }
}


