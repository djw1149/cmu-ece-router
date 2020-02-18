/*
 *  Packet allocation module (initialization)
 *
 **********************************************************************
 * HISTORY
 * 19-15-86  Matt Mathis at CMU
 *	Changed to use pkpool for the policy packet pool size (initialized
 *	by Isize() ), and PKMALLOC to actuallu get the space.
 *	in the 68k router (where packets reside in the main data segment)
 *	pkmalloc just does a malloc.   In other routers where packets
 *	reside outside of the main memory pool, pkmalloc does something else.
 *
 * 17-Jul-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Modified to dynamically calculate the maximum packet size, offset and
 *	lengths at configuration time based on the device drivers which
 *	are configured into the system.  This provides the device drivers
 *	with new PSIZE, PLEN and POFF variables for use in setting up
 *	I/O operations.
 *
 * 23-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed to use new extended memory referencing through UPAR7.
 *
 * 08-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed to always correctly initialize the map field in the packet
 *	structure even if memory management is not available.
 *
 * 07-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/dma.h"
#include "../mch/device.h"
#include "../../h/aconf.h"

#ifdef	PCHECK
extern struct packet *epacket, *fpacket;
#else	PCHECK
static struct packet *epacket, *fpacket;
#endif	PCHECK
extern struct queue pfreeq;
extern int Immflag;



/*
 *  Ipalloc - initialize free packet queue
 *
 *  Scan the device autoconfiguration table to determine the maximum packet
 *  buffer size and default offset and length parameters.  The maximum buffer
 *  size is calculated from the sum of the largest packet header length,
 *  largest MTU, largest packet trailer length, and the largest packet overrun
 *  area size (space allocated to the end of the buffer which will never be
 *  used unless the device goofs).  The default receive offset is the position
 *  of the data area for the device with the largest packet header (so that a
 *  packet received on any device can be turned around and reencapsulated for
 *  output on any other device and the header will always "fit").  The
 *  default receive packet length is the difference between the maximum buffer
 *  size, the default offset and the largest overrun size (since this area is
 *  not included in the buffer size supplied to devices).  A device may use
 *  this size or a smaller size more suitable to its MTU when beginning a
 *  receive operation.
 *
 */

Ipalloc()
{
    extern struct packet *Ipinit();
    extern struct autoconf *autoconf[];
    extern u_long pkpool;
    char *base = 0;		/* base address of packet data area */
    register int i;		/* number of packets to allocate */
    struct autoconf **acp;	/* current autoconfiguration structure */
    short maxhead = 0;		/* largest packet header size found */
    short maxmtu = 0;		/* largest packet MTU size found */
    short maxtrail = 0;		/* largest packet trailer size found */
    short maxslop = 0;		/* largest packet overrun size found */

    for (acp=autoconf; *acp; acp++)
    {
	struct autoconf *ac = *acp;	/* current configuration structure */
	struct config *cf = ac->ac_cf;	/* current configuration paramters */

	if (cf == 0)
	    continue;

	if (cf->cf_head > maxhead)
	    maxhead = cf->cf_head;
	if (cf->cf_trail > maxtrail)
	    maxtrail = cf->cf_trail;
	if (cf->cf_slop > maxslop)
	    maxslop = cf->cf_slop;
	if (cf->cf_mtu > maxmtu)
	    maxmtu = cf->cf_mtu;
    }

    PSIZE = maxhead+maxmtu+maxtrail+maxslop;
    POFF  = maxhead;
    PLEN  = PSIZE-POFF-maxslop;

    if (PSIZE)
    {
        i = (pkpool)/PSIZE;
	base = (char *)pkmalloc(PSIZE*i);
	if (base == 0)
	   panic("Ipalloc");

        cprintf("Available %d+%d+%d+%d=%d byte packets=%d at %x \r\n",
	         maxhead, maxmtu, maxtrail, maxslop, PSIZE, i, base);
        fpacket = Ipinit(&pfreeq, i, PSIZE, base, pfree, 0);
	epacket = fpacket+i;
    }
    else cprintf ("Ipalloc: warning, no packet devices available\r\n");
}


/*
 *  Ipinit - initialize a packet queue
 *
 *  q       = the free packet queue (assumed initialized)
 *  npacket = the number of packets to set up
 *  psize   = the size of the packet data area (bytes)
 *  mapbase = the base of extended memory at which the packets will be
 *	      mapped (in 64-byte blocks)
 *  done    = the packet done processing routine for this packet queue
 *  dv      = the associated device which owns this queue (0=>standard
 *	      system-wide queue
 *
 *  Allocate sufficient space for the specified number of packet headers and
 *  initialize the fields appropriately. 
 *  Initialize the offset and length fields to
 *  0 and the maximum packet size respectively.  Set the done processing
 *  routine and associated device fields as indicated.  Clear the flags field
 *  and initialize the maximum length and check fields if packet
 *  consistency checks are nabled.  Finally, queue the packet onto the free
 *  queue.
 *
 *  This routine is provided for device drivers which must maintain their own
 *  special packet queue (such as for the 3Com ethernet interface) because they
 *  require that the packet data area be in fixed physical memory.  Strictly
 *  speaking, the Xerox 3Mb ethernet interface falls in this category, but
 *  since it is the most common device the standard packet queue is optimized
 *  to support its restrictions.
 */

struct packet *
Ipinit(q, npacket, psize, mapbase, done, dv)
register struct queue *q;
register int npacket;
int (*done)();
struct device *dv;
{
    register char  *ba;
    register struct packet *p;
    struct packet  *fp;

    fp = p = (struct packet *) malloc (npacket * (sizeof (struct packet)));
    if (p == 0)
	panic ("Ipinit");
    ba = (char *) mapbase;
    for (; npacket--; p++, ba += psize) {
	p -> p_ba = ba;
	p -> p_data = ba;
	p -> p_map =  (int)ba;
	p -> p_off = 0;
	p -> p_len = psize;
	p -> p_done = done;
	p -> p_dv = dv;
	p -> p_flag = 0;
#ifdef	PCHECK
	p -> p_maxlen = psize;
	p -> p_check = 0;
#endif	PCHECK
	enqueue (q, p);
    }
    return (fp);
}
