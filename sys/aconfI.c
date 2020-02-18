/*
 *  Device autoconfiguration module (intialization only)
 *  $Header: aconfI.c,v 1.3 86/10/22 13:14:04 djw Exp $
 *
 **********************************************************************
 * HISTORY
 *
 * Wed Sep 24 10:42:03 1986 david waitzman (djw) at cmu
 *	added new configure rom support
 *
 ****
 *  2-July-86  Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University moved:
 *		probe struct definition 	to intr.h
 *		setvector			to intr.c
 *		Ibadaddr			to trapI.c
 *		Iprobe,Iintr			to intrI.c
 *
 ****
 *  12 Dec 85 Matt Mathis (mathis) at CMU
 * 	Removed the per protocol statistics initialization.  Replaced it
 *	with shared per device statistics, initialized in autoconf
 *
*/

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/aconf.h"

extern p_inter *Iprobe();
extern pu_char patchpi();
extern struct autoconf *autoconf[];	/* supported devices */

/*
 *  Iconfigure - scan the driver table and probe for devices
 *
 *  Scan the driver configuration table to locate, configure, and initialize
 *  all devices found on the processor.  The driver table is scanned in order
 *  from top to bottom, looking for all possible devices for the indicated
 *  driver before continuing with the next driver.  The auto-configuration
 *  identification list for each driver is scanned and the driver probe routine
 *  is called to test for a device supported by that driver at each of the
 *  listed CSR addresses.  When a possible device is found, the driver probe
 *  routine is called again to cause the device to interrupt and determine its
 *  interrupt vector and BR level.
 *  
 *  When a device has been successfully configured in this manner, a device
 *  structure is allocated and initialized for it.  The CSR address is recorded
 *  there and the interrupt vector(s) are initialized to branch to the device
 *  driver interrupt routine(s).  The device initialization routine is then
 *  called to perform any device specific initialization (such as allocating
 *  device dependent data structures, determining its physical network address,
 *  etc.).
 *  
 *  If the device is a network device (as indicated by the presence of a common
 *  device parameter structure), the common device parameters for this device
 *  and driver (linkage routines, supported protocol list, phsyical address
 *  length, etc.) are copied into the device structure.  The device structure
 *  is then added to the global list of configured network devices.
 *  
 *  The device protocol/identification list is then executed to perform
 *  initialization specific to the addressed device.  If the watchdog display
 *  device is configured into the system and the device is a network, an entry
 *  is made in the display queue to display its received packet count at
 *  regular intervals.
 *  
 *  Finally, the input process for the device is created and if the device is
 *  not a network it is turned on.  When all devices have been configured and
 *  intialized, the network device list is scanned and the once-only driver
 *  initialization routine for each network device is called after which the
 *  device is turned on.  This last step in the configuration procedure is
 *  delayed until all devices have been found for two reasons.  It prevents
 *  network devices configured early in the procedure from accumulating too
 *  many packets which cannot yet be routed and also provides the opportunity
 *  for a device to peform the last stage of its own initialization perhaps
 *  based on state provided by previous devices (e.g. whether a device of a
 *  particular type was also configured).
 *  
 *  If a device supports multiple units (such as the DZ-11), device structures
 *  are allocated and intialized for each existing unit (as specified in the
 *  autoconfiguration information table).  The device structures for each unit
 *  are allocated contiguously so that the structure for a particular unit can
 *  be easily determined by array indexing from the address of the first
 *  structure.
 *
 *  TODO: save BR level in device structure.
 */

struct conflist {
    p_autoid ai;
    p_autoconf ac;
    pu_char pi;
    pu_char ipaddr;	
};

/*
 * returns 1 if the minor number "minor" isn't being used in conflist by a
 * network device, else returns 0
 */
int minor_free(minor, conflist, clind)
u_long minor;
struct conflist conflist[];
int clind;
{
    int i;

    for (i = 0; i < clind; i++) {
/* 	cprintf("[%d of %d %5s] ", i, clind, conflist[i].ai->id.devname); */
        if (conflist[i].ac->ac_cf && conflist[i].ai->id.devminor == minor) {
/* 	    cprintf("Duplicate minor number %d ignored\r\n", minor); */
	    return(0);
	}
    }
    return(1);
}

Iconfigure()
{
    register p_autorom mrom    /* the main conf rom */
        = (p_autorom)malloc(sizeof(struct autorom) + EXTRA);
    register p_autorom srom    /* the secondary conf rom */
        = (p_autorom)malloc(sizeof(struct autorom) + EXTRA);

#define MAXCONFLIST	20
    struct conflist conflist[MAXCONFLIST]; /* devices that passed csr probe */
    int clind = 0;			/* conf list index */
    int i;				/* temporary loop index for conflist */

    p_autoid *p_ai;			/* pointer to rom autoid list  */
    p_autoconf *acp;			/* current driver table slot */
    struct device *dv;			/* current device */

    int haverom;			/* 1 if have a confrom, else 0 */
    u_char /*FAR*/ *b;			/* to the first glob entry */

    if (!(haverom = find_confrom(mrom, srom))) {
/* 	OLDIconfigure(); */	/* use old version */
	panic("No configure rom found\r\n");
    }
/* haverom = 0;	/* debugging only */
    display_rom(mrom);			 /* debugging only */ 

    b = *(mrom->glob);			/* look at the first argv value */
    cprintf("Default gateway %d.%d.%d.%d\r\n", b[0],b[1],b[2],b[3]);
    Iipkngw(b);
	
    /*
     * for each autoconf
     *     for each autoid
     *         if (the autoid is of the correct device type and
     *             the device is present)
     *            then remember the device for later
     * the srom isn't looked at for a device if we found any pb_... for that
     * device in the mrom
     */
    for (acp=autoconf; *acp; acp++) {
	int found = 0;
	for (p_ai = mrom->autoids; *p_ai; p_ai++) {
	    if (bcmp((*p_ai)->id.devname, (*acp)->ac_name, 4)
		|| !minor_free((*p_ai)->id.devminor, conflist, clind) ||
	        !(((*acp)->ac_probe)(&((*p_ai)->devconf), PB_CHECK)))
		continue;
/* cprintf("added %s @0x%x (%d)\r\n", (*acp)->ac_name, (*p_ai)->devconf,
	(*p_ai)->id.devminor); */
	    conflist[clind].ac = *acp;		/* remember this combination */
	    conflist[clind].ipaddr = *(mrom->net + (*p_ai)->id.devminor);
	    conflist[clind].pi = conflist[clind].ipaddr + PRL_IP;
	    conflist[clind++].ai = *p_ai;
	    found = 1;
	    continue;
	}
	if (!found && haverom) {
	    for (p_ai = srom->autoids; *p_ai; p_ai++) {
	        if (bcmp((*p_ai)->id.devname, (*acp)->ac_name, 4)
		    || !minor_free((*p_ai)->id.devminor, conflist, clind) ||
	            !(((*acp)->ac_probe)(&((*p_ai)->devconf), PB_CHECK)))
		    continue;
/* cprintf("added %s @0x%x (%d) from srom\r\n", (*acp)->ac_name, 
	(*p_ai)->devconf, (*p_ai)->id.devminor); */
	        conflist[clind].ac = *acp;     /* remember this combination */
		conflist[clind].ipaddr = *(mrom->net + (*p_ai)->id.devminor);
		conflist[clind].pi = conflist[clind].ipaddr + PRL_IP;
	        conflist[clind++].ai = *p_ai;
		found = 1;
	        continue;
	    }
	} 
    }	


    for (i = 0; i < clind; i++) {
	p_autoid ai = conflist[i].ai;		/* current id list	    */
	p_autoconf ac = conflist[i].ac;		/* current driver parameters*/
        p_inter *pb;				/* Iprobe() return structure*/
	int nvec;				/* # of device intr vectors */
	int ddlen;				/* device dep. fields length*/

        cprintf("%s(%d) at csr 0x%x ",
	        ac->ac_name, ai->id.devminor, ai->devconf);

	if (ac->ac_intr[0] != 0) {
	    pb = Iprobe(ac, ai);
	    if (pb == 0) {
	        cprintf(" failed probe\r\n");
	        continue;
	    }
	    cprintf("vector %d, br%d ", pb->pb_vector,(pb->pb_br/PS_BR1)&0xf);
	} else
	    pb = 0;
#ifdef DEBUGA
cprintf("probe ok\r\n");
#endif /* DEBUGA */
	if (ac->ac_cf)
	    ddlen = (sizeof(*dv)-sizeof(dv->dv_devdep)) + ac->ac_cf->cf_ddlen;
	else
	    ddlen = sizeof(dv->dv_common);

	dv = (struct device *)zalloc(ddlen);
	if (dv == 0)
	    panic("Iconfigure dv");
#ifdef DEBUGA
cprintf("vector setup:");
#endif /* DEBUGA */
	/* set the vectors up */
	for (nvec = 0; nvec < ACMAXVEC && ac->ac_intr[nvec]; nvec++)
	    setvector(pb, ac->ac_intr[nvec], dv);
#ifdef DEBUGA
cprintf(" ok\r\n");
#endif /* DEBUGA */
	dv->dv_pb = pb;
	dv->dv_addr = (pu_char)ai->devconf;

	if (ac->ac_cf != 0) {
 	    initdevice(dv);
	    /* copy the ipaddress */
            bcopy(conflist[i].ipaddr, &dv->dv_praddr[proto[PR_IP].pr_aofs]
	    	  ,PRL_IP);
    	    dv->dv_phys[0] = 1;		/* was the current nunit WHY?? */
	    dv->dv_cf = *(ac->ac_cf);
	    dv->dv_next = dvhead;	/* link this device into the list */
	    dvhead = dv;
	    dv->dv_ddlen = ddlen;
	}
#ifdef DEBUGA
cprintf("hw init:");
#endif /* DEBUGA */
	/* do device (hardware) initialization */
	if (ac->ac_init)
	    (*(ac->ac_init))(dv, ai);
#ifdef DEBUGA
cprintf(" ok\r\n");
#endif /* DEBUGA */
	/*
	 * if a net device then process the pi list.
	 * optionally use patch code- by <romserial#,minor#> giving a new
	 * pi structure
	 */
	if (ac->ac_cf != 0)
	    Idvproto(dv, ai->id.devminor, needspatched(mrom, ai->id.devminor)
	    			           ? patchpi(mrom, ai->id.devminor)
					   : conflist[i].pi);

#if	C_WT > 0
	if (ac->ac_cf != 0)
	    wtmake(&((union longval *)(dv->dv_cnt.dvu_cnt))->lv_loword,
		   2, (dv->dv_cable<<4)|0xf, (dv->dv_cable<<4)|0xf);
#endif	C_WT
#ifdef DEBUGA
cprintf("makeproc:");
#endif /* DEBUGA */
	if (ac->ac_proc && (makeproc(ac->ac_proc, dv) == 0))
	    panic("Iconfigure proc");
#ifdef DEBUGA
cprintf(" ok\r\nreset:");
#endif /* DEBUGA */
	if (ac->ac_reset)
	    (*(ac->ac_reset))(dv, DVR_ON);
#ifdef DEBUGA
cprintf(" ok\r\n");
#endif /* DEBUGA */
 	((pu_char)dv) += ddlen;

	cprintf("\r\n");
    }

    /*
     * Go through all the network devices, doing optional software init,
     * and then reset the device
     */
    for (dv = dvhead; dv; dv = dv->dv_next)
    {
	/*
 	 *  Perform any driver specific software initialization
 	 */
	for (acp=autoconf; *acp; acp++)	{
	    p_autoconf ac = *acp;
	    if (ac->ac_cf &&
		ac->ac_cf->cf_type == dv->dv_type &&
		ac->ac_once)
    		(*(ac->ac_once))(dv);
	}

	if (dv->dv_reset)
	    (*(dv->dv_reset))(dv, DVR_ON);
    }
#undef MAXCONFLIST
}

/*
 * routine Idvproto
 *	If a pi structure exists for this device, then it
 *	is a network device. otherwise return immediately.
 *	Otherwise we have a pi structure so call Idvexec to
 *	execute the pi commands.
 */
Idvproto (dv, minor, pi)
register struct device *dv;
u_long minor;
pu_char pi;
{
    if (pi == 0)
       return;

    Idvexec (dv, minor, pi);
    Idvattach(dv);
}


/*
 *  Idvexec - initialize device protocol and/or identification information
 *
 *  dv = device structure to initialize
 *  pi = protocol identification description
 *
 *  Execute the device protocol identification list (see description for each
 *  op code below).  This list is provided as an array of bytes since this is
 *  the only way it may be conveniently initialized by the various device
 *  drivers.  
 *  
 *  TODO: fix protocol initialization to consistently test for protocol in the
 *	    protocol specific initialization routine.
 */
Idvexec(dv, minor, pi)
register struct device *dv;
u_long minor;
register pu_char pi;
{
    int cond = 0;	/* conditional execution mask (0=>okay, else=> */
			/*  execution currently suppressed), nesting */
			/*  is accomplished by shifting the bit mask */
			/*  left one bit for each new level and turning */
			/*  on the low order bit */

    if (pi == 0)
       return;

/*    dv->dv_cable = -1;*/
dv->dv_cable = minor + 1;	/* djw: just for now */
    while (*pi != PI_END)
    {
 cprintf("PI = %d ", *pi); 

	switch (*pi)
	{
	    /*
	     *  Conditionally execute following operations only if we are on
	     *  the specified cable (and optionally) have the indicated
	     *  physical address (if the address length is not zero).
	     *
	     *  Format: ISME, <cable-number>, <address-length>,
	     *                <address-byte>, ...
	     */
	    case PI_C_ISME:
	    {
		struct device *dvt;

		pi++;
		for (dvt=dvhead; dvt; dvt=dvt->dv_next)
		    if (dvt->dv_cable == *pi)
		    {
			if (*(pi+1) == 0 || isme(dvt, pi+2))
			{
			    cond <<= 1;
			    break;
			}
		     }
		if (dvt == 0)
		    cond = (cond<<1)|1;
		pi += *++pi + 1;
		break;
	    }

	    /*
	     *  Conditionally execute following operations only if we have the
	     *  specified physical address.
	     *
	     *  Format: ISPHYS, <address-byte>, ...
	     */
	    case PI_C_ISPHYS:
	    {
		if (isme(dv, ++pi))
		    cond <<= 1;
		else
		    cond = (cond<<1)|1;
		pi += dv->dv_hln;
		break;
	    }

	    /*
	     *  End the most recent conditional execution (nesting is limited
	     *  by the number of bits in a word).
	     *
	     *  Format: END
	     */
	    case PI_C_END:
		pi++;
		cond >>= 1;
		break;

	    /*
	     *  Set the physical hardware address of the device.
	     *
	     *  Format: HRD, <address-byte>, ...
	     */
	    case PI_HRD:
		pi++;
		if (!cond)
		{
		    bcopy(pi, dv->dv_phys, dv->dv_hln);
		}
		pi += dv->dv_hln;
		break;

	    /*
	     *  Supply the IP address of a well-known gateway on this network.
	     *
	     *  Format: GWAY, <address-byte>, ...
	     */
	    case PI_GWAY:
		pi++;
		if (!cond)
		    Iipkngw(pi);       /* this is also done globally */
		pi += PRL_IP;
		break;

	    /*
	     *  Supply a permanent redirection for an IP address to a
	     *  particular gateway address.
	     *
	     *  Format: REDIR, <routing-entry-flags>,
	     *		       <address-byte>, ..., <address-byte>
	     *		       <gateway-byte>, ..., <gateway-byte>
	     */
	    case PI_REDIR:
		pi++;
		if (!cond)
		    Iipredir(pi[0], pi+1, pi+1+PRL_IP, dv);
		pi += 1+PRL_IP+PRL_IP;
		break;

	    /*
	     *  Set the cable number of this device.
	     *
	     *  Format: CABLE, <number>
	     */
	    case PI_CABLE:
		pi++;
		if (!cond)
		{
		    dv->dv_cable = *pi;
		}
		pi++;
		break;

	    /*
	     *  Set this cable restricted for indicated protocol.
	     *
	     *  Format: RESTR, <protocol>
	     */
	    case PI_RESTR:
	    {
		int pr = *++pi;

		if (!cond)
		{
		    dv->dv_restr[pr] = TRUE;
		}
		pi++;
		break;
	    }

	    /*
	     *  Authorize an address for unrestricted communication.
	     *
	     *  Format: AUTH, <protocol-number>,
	     *		      <address-byte>, ..., <address-byte>
	     */
	    case PI_AUTH:
	    {
		int pr = *++pi;

		pi++;
		if (!cond)
		{
		    Irsdv(dv, pr, pi);
		}
		pi += proto[pr].pr_pln;
		break;
	    }

	    /*
	     *  Specify device independent flag(s).
	     *
	     *  Format: FLAG, <flag(s)>
	     */
	    case PI_FLAG:
	    {
		pi++;
		if (!cond)
		{
		    dv->dv_flag |= *((pu_char)pi);
		}
		pi++;
		break;
	    }

	    /*
	     *  Prematurely complete initialization processing (e.g.  within a
	     *  conditional).
	     *
	     *  Format: DONE
	     */
	    case PI_DONE:
		if (!cond)
		{
		    cond = 0;
		    goto done;
		}
		pi++;
		break;

	    /*
	     * handle subnet definitions
	     */
	    case PI_SUB:
	    	cprintf("PI_SUB is undefined\r\n");
		pi += 6;		/* PI + {8,...} + ipaddr */
		break;

	    /*
	     *  Set protocol address of this device.
	     *  Note that this can be used to give a new ipaddr for this device
	     *
	     *  Format: <protocol-number>, <address-byte>, ..., <address-byte>
	     */
	    default:
	    {
		int pr = *pi++;

		if (pr < 0 || pr >= NPR)
		    panic("pi bad praddr");
		if (!cond)
		    bcopy(pi, &dv->dv_praddr[proto[pr].pr_aofs], proto[pr].pr_pln);
		pi += proto[pr].pr_pln;
		break;
	    }
	}
    }
done:
    if (cond)
	panic("pi cond");
}

/*
 * routine Idvattach
 *	attach configured device to protocol drivers for IP and
 *	ARP (note: this use to be part of Idvproto).
 */
Idvattach (dv)
register struct device *dv;
{
    switch(dv->dv_hln)
    {
	case 1:
	    cprintf(dv->dv_type == DVT_3ETHER?"0%o":"%d",
		    (u_char)dv->dv_phys[0]);
	    break;
	default:
	    cprintf("%s", pa_fmt(dv->dv_phys, dv->dv_hln));
    }

    if (dv->dv_cable < 0)
	panic("no cable");
    cprintf(" cable %d", dv->dv_cable);

    dv->dv_cnts = (struct devstats *)zalloc(sizeof(struct devstats));
    if (dv->dv_cnts == 0)
	panic("Idvattach devstats");

    Iardv(dv);
    if (dv->dv_pr[PR_IP])
    {
	Iipdv(dv);
	if (dv->dv_restr[PR_IP])
	    cprintf(" (restricted IP)");
    }
#if	C_CHAOS > 0
    if (dv->dv_pr[PR_CHAOS])
    {
	Icndv(dv);
    }
#endif	C_CHAOS
}
