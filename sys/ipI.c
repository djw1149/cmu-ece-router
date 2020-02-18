/*
 *  Internet protocol module (initialization)
 *
 **********************************************************************
 * HISTORY
 *  12-Sep-86 Matt Mathis at CMU
 *	Set up ar_IPnet and ar_IPmask so that the ARP module can quickly
 *	check and discard ARP with bogus protocol addresses.
 *
 *  5 Dec 85 Matt Mathis (mathis) at CMU
 * 	Removed the per protocol statistics initialization.  Replaced it
 *	with shared per device statistics, initialized in autoconf
 *	
 * 27-Aug-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Chnaged to initialize ICMP statistics counts in IP statistics
 *	count structure.
 *
 * 21-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Removed support for obsolete "probe" function and changed to
 *	record a single well-known gateway (for systems without EGP);
 *	adjusted to provide the new reporting gateway parameter to
 *	ip_remap() and to initialize the autonomous system number field
 *	of routing entries created for our directly connected networks.
 *
 * 28-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Removed initialization of UDP and RCP statistics structures
 *	and added Iuddv() call to allow UDP module to perform its own
 *	initialization.  Added Iegdv() to initialize EGP module.
 *
 * 25-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed to create network mapping entries for CPU interfaces
 *	(so that the CPU can talk to itself through the interface
 *	even when it is the only device on the system and has no linked
 *	devices which will create the entry for its network).
 *
 * 20-Feb-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */


#include "cond/egp.h"

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"
#include "../../h/time.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"
#include "../../h/ip.h"
#include "../../h/icmp.h"
#include "../../h/udp.h"
#include "../../h/rcp.h"



/* #include "cond/wt.h" */


int Iipinit = 0;		/* IP module initilization flag */
				/* (0=need initialization) */

extern struct socket ar_IPnet;	/* Tell the arp what addresses are ok */
extern struct socket ar_IPmask;

/*
 *  Iipdv - initialize IP for a device interface
 *
 *  dv = device on which to intialize IP 
 *
 *  If no IP protocol intialization has yet been done, initial the the IP
 *  module.  Allocate and zero the IP and ICMP statistics count structure for
 *  this device.  Link this device for IP addressing to any other devices which
 *  have already been initialized and that are recorded as being part of the
 *  same IP network (this linkage is used for address resolution and IP
 *  rebroadcasting).  If the device is not a single CPU, add a permanent
 *  specific host routing table entry for the IP address of this device so that
 *  it will henceforth be known as one of our addresses, and also add a
 *  permanent routing table entry for its IP network number to record it as one
 *  of our directly connected networks (unless the device is a CPU and such an
 *  entry already exists).  Finally, initialize EGP and UDP for the device.
 */

Iipdv(dv)
register struct device *dv;
{
    register struct ipmap *im;		/* routing table entry */
    struct socket net;			/* IP network number in our address */
    struct socket addr;			/* temporary for our IP address */
    char temp[20];			/* buffer for printing IP address */

    if (Iipinit == 0)
	Iip();

    bcopy(&dv->dv_praddr[PRA_IP], &addr, PRL_IP);
    cprintf("\r\n   IP %s (device %o", ip_fmt(&addr, temp), dv);
    im = ip_map(&addr, IM_LOCAL);
    if (im)
    {
	dv->dv_prnext[PR_IP] = im->im_dv->dv_prnext[PR_IP];
	im->im_dv->dv_prnext[PR_IP] = dv;
cprintf(" linked with %o", im->im_dv);
    }
    else
	dv->dv_prnext[PR_IP] = dv;
cprintf(")");

    if ((dv->dv_flag&DV_CPU) == 0)
    {
	im = ip_remap(&addr, &addr, dv, IM_ME|IM_PERM, 0);
	if (im == 0)
	    panic("Iipdv");
    }
    net.s_addr = addr.s_addr&ip_mask(&addr);
    ar_IPmask.s_addr = ip_mask(&addr);
    ar_IPnet.s_addr = net.s_addr;

    if ((dv->dv_flag&DV_CPU) == 0 ||
        ip_map(&net, IM_NET) == 0)
    {
	im = ip_remap(&net, &addr, dv, IM_NET|IM_LOCAL|IM_PERM|IM_INTNET, 0);
	if (im == 0)
	    panic("Iipdv");
#if	C_EGP > 0
	im->im_asn = EGPASN;
#endif	C_EGP
    }
#if	C_EGP > 0
    Iegdv(dv);
#endif	C_EGP
    Iuddv(dv);
}



/*
 *  Iip - IP protocol module initialization
 *
 *  Initialize all the queue headers in the routing table.  Create a watchdog
 *  timer entry to display the received IP packet count every 5 seconds.
 */

Iip()
{
    register int i;
    extern union longval ip_cnt;

    for (i=NIPMAP; i--; )
	initqentry(&ipmap[i]);
#if	C_WT > 0
    wtmake(&ip_cnt.lv_loword, 5, 0xcf, 0xdf);
#endif	C_WT
    Iipinit++;
}



/*
 *  Iipkngw - declare a well-known gateway to the list
 *
 *  addr = the address of the well-known gateway on the directly connected
 *	    network
 *  
 *  Set the well-known gateway as specified.
 */

Iipkngw(addr)
char *addr;
{
    bcopy(addr, &ipknowngw, PRL_IP);
}



/*
 *  Iipredir - insert an initial routing table redirect entry for an address
 *
 *  flag = flags to enter into the routing table entry
 *  ap   = IP address to be added
 *  gp   = address of gateway to which address should be routed
 *  dv   = device requesting this redirection mapping
 *
 *  Initialize the IP module if this has not already been done.  Enter the
 *  specified redirection as a permanent entry in the routing table.  The
 *  address of the gateway must be on one of our directly connected (and
 *  already configured) networks.  If it isn't, it is assumed to be connected
 *  to the interface currently being initialized.
 */

Iipredir(flag, ap, gp, dv)
char *ap;
char *gp;
struct device *dv;
{
    register struct ipmap *im;		/* routing table entry */
    struct socket addr;			/* space for aligned IP address */
    struct socket gway;			/* space for aligned gateway address */

    if (Iipinit == 0)
	Iip();

    bcopy(ap, &addr, PRL_IP);
    bcopy(gp, &gway, PRL_IP);
    im = ip_map(&gway, IM_LOCAL);
    if (im != 0)
	dv = im->im_dv;
    im = ip_remap(&addr, &gway, dv, flag|IM_PERM, 0);
    if (im == 0)
	panic("Iipredir remap");
}
