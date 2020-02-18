/*
 *  Router Control Protocol module
 *
 **********************************************************************
 * HISTORY
 *
 *     Aug-86 Chriss Stephens (chriss) at Carnegie Mellon University removed
 *	support for version 0 tools, and fixed packet size length checking
 *	for authorization packets.
 *
 *     Jun-86 Chriss Stephens (chriss) at Canegie Mellon University
 *	 Added ability to support multiple versions of client
 *	 software. Original software assigned version number 0.
 *	 Added rc_append, and modified rc_info and rc_diag to use
 *	 common data structure.
 *
 *  12-Mar-86 David Waitzman (djw) at Carnegie-Mellon University
 *	Added remote diagnosis (DIAG) functions.
 *
 *  6-Jan-86  Kevin Kirmse (kdk) at Carnegie-Mellon University
 *      Added Global Switch control.
 *
 * 20-Jun-85  Kevin Kirmse (kdk) at Carnegie-Mellon University
 *      Added RCP requests for router information.
 *
 * 27-Aug-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Revised to access RCP statistics in UDP stat structure.
 *	
 * 23-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Revised to understand new UPAR7 mapping.
 *
 * 25-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added new RCP requests to initiate a reboot and change memory
 *	locations (when properly authorized);  changed error reply to
 *	include a code field to distinguish different kinds of failures.
 *
 * 07-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#include "cond/rcppswd.h"

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/profile.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"
#include "../../h/aconf.h"
#include "../../h/devstats.h"
#include "../../h/ip.h"
#include "../../h/udp.h"
#include "../../h/rcp.h"
#include "../mch/dma.h"

#include "../../h/globalsw.h"
#include "../../h/errorlog.h"

#include "debug/rcp.h"

#define NULL 0


struct socket rcpsaddr = {0};	/* authorized source address */
short rcpsport = {0};		/* authorized source port */
short rcpid = {0};		/* authorized request ID */
char rcpvalid = {0};		/* authorization valid flag */

char rcppswd[RCMAXPSWD+1] = RCPPSWD;	/* authorization code */

char rcpreboot = {0};		/* reboot request pending flag */

/*
 *  rc_input - process an incoming RCP packet.
 *
 *  dv    = the device supplying the packet
 *  p     = the supplied ICMP packet (with offset and length adjusted to
 *	    remove any encapsulating headers/trailers)
 *  sport = the UDP source port of the sender of the datagram
 *  saddr = the datagram's IP source address
 *  daddr = the datagram's IP destination address
 *	    (These will typically be pointers into the encapsulating IP
 *	    header preceding the RCP packet - beware!)
 *
 *  The following consistency checks are performed on the RCP packet:
 *  - the physical length of the packet must be large enough to contain a
 *    minimal RCP header
 *  If the packet checks out, the message is counted and processed
 *  according to the protocol.
 */

rc_input(dv, p, sport, saddr, daddr)
struct device *dv;
struct packet *p;
short sport;
struct socket *saddr;
struct socket *daddr;
{
    register struct rcp *rc;
    unsigned short i;
    char temp[20];
    u_short code = RCE_LENGTH;		/* default error code */

    if (p->p_len < RCHEAD)
    {
	profile(dv,rc_rmin);
	goto out;
    }

    rc = poff(rcp, p);
#ifdef	RCDEBUG
    if(p->p_flag&P_TRACE)
    {
	printf("RRCP (%d):\r\n", p->p_len);
	rc_prt(rc, p->p_len);
    }
#endif	RCDEBUG

    profile(dv,rc_rcnt);
    rc->rc_type = ntohs(rc->rc_type);

    switch(rc->rc_type)
    {
	/*
	 *  Examine system memory.
	 *
	 *  the area to be examined must be no larger than 512 bytes, and must
	 *  not extend beyond the virtual address space of the system.
	 */
	case V1_RCT_GET:
	{
	    char *first, *last;
	    char *to;

	    rc->rc_from = (p_char)ntohl (rc->rc_from);
	    rc->rc_len  = ntohs (rc->rc_len);

	    rcpvalid = FALSE;
	    first = rc->rc_from;
	    last  = first+rc->rc_len;

	    if (p->p_len != (RCHEAD+sizeof(rc->rc_from)+sizeof(rc->rc_len)) ||
	        rc->rc_len > 512)
	 	goto error;

	    if (/*last > MMPGTOVA(7) || */first > last)
	    {
		code = RCE_ARG;
	 	goto error;
	    }
	    profile(dv,rc_get);
	    p->p_len = RCHEAD+rc->rc_len;
	    for (to= &(rc->rc_getdata)[0]; first < last; )
		*to++ = *first++;
	    rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_DATA, 
		      sport, saddr, daddr);
	    return;
	}

	/*
	 *  Deposit into system memory.
	 *
	 *  The area to be examined must be no larger than 512 bytes, and must
	 *  not extend beyond the virtual address space of the system.  The
	 *  request must be authorized.
	 */
	case V1_RCT_PUT:
	{
	    char *first, *last;
	    char *from;

	    if (!rc_valid(dv, rc, sport, saddr))
	    {
		code = RCE_AUTH;
		goto error;
	    }
	    if (p->p_len < (RCHEAD+sizeof(rc->rc_to)) ||
	        p->p_len > (RCHEAD+sizeof(rc->rc_to)+512))
	 	goto error;
	    first = rc->rc_to;
	    last  = first+(p->p_len-RCHEAD-sizeof(rc->rc_to));
	    if (last > MMPGTOVA(7) || first > last)
	    {
		code = RCE_ARG;
	 	goto error;
	    }
	    profile(dv,rc_put);
	    printf("[ %s Host ", timeup(temp));
	    printf("%s port %d PUT %d bytes at %o ]\r\n",
		   ip_fmt(saddr, temp), sport, last-first, first);
	    for (from= &(rc->rc_putdata)[0]; first < last; )
		*first++ = *from++;
	    p->p_len = RCHEAD;
	    rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_OK,
		      sport, saddr, daddr);
	    return;
	}

      case V1_RCT_INFO:
      {

#ifdef RCDEBUG
   printf ("RCP: received V1_RCT_INFO packet\n\r");
#endif

	switch(ntohs(rc->rc_u.rcu_info.info_type))
        {
	    case RINFO_DGET :
                 V1_ar_rcparplist(&rc->rc_diagfo,&p->p_len);
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                 break;
            case RINFO_UPTM :
                 V1_rc_uptime(dv,p,&rc->rc_diagfo);
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                 break;                  
            case RINFO_DEVH :
                 V1_rc_device(p,&rc->rc_diagfo);
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                 break;                  
            case RINFO_XPING :
                 rc->rc_u.rcu_info.info_type = RINFO_APING;
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                 break;                  
	    case RINFO_ELOG  :
                 V1_rc_errorlog(dv,p,&rc->rc_u.rcu_info);
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                 break;                  
	   default:
                 rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_INFO,
			   sport, saddr, daddr);
                  break;
	}
      	    return;
      }

      case V1_RCT_DIAG:
      {
	V1_do_rc_diag (rc, p, sport, saddr);
	rc_output (dv, p, ntohs(rc->rc_id), V1_RCT_DIAG, sport, saddr, daddr);
	return;
      }

	/*
	 *  Authorize special function.
	 *
	 *  Supply an authorization code to validate the following request.
         *  If the appropriate authorization code is supplied, the source
         *  address, port, and ID of this request are recorded so that they
         *  will be available to validate the immediately following request.
         *  This is a temporary authorization mechanism and will eventually
         *  be replaced (since it is not secure against eavesdropping and/or
         *  fraud).
	 */
	case V1_RCT_AUTH:
	{
	    register int i;

	    if (p->p_len < (RCHEAD+sizeof(rc->rc_pswd)))
	 	goto error;
	    for (i=0; i<sizeof(rc->rc_pswd); i++)
		if (rc->rc_pswd[i] != rcppswd[i])
		{
		    profile(dv,rc_unauth);
		    code = RCE_AUTH;
		    printf("\7\7\7[ %s Host ", timeup(temp));
		    printf("%s port %d authorization failure ]\7\7\7\r\n",
			   ip_fmt(saddr, temp), sport);
		    goto error;
		}
	    profile(dv,rc_auth);
	    rcpsaddr.s_addr = saddr->s_addr;
	    rcpsport = sport;
	    rcpid = ntohs(rc->rc_id)+1;
	    rcpvalid = TRUE;
            rcpreboot = 0;
	    p->p_len = RCHEAD;
	    rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_OK,
		      sport, saddr, daddr);
	    return;
	}

	/*
	 *  Reboot router.
	 *
	 *  Verify that the request has been authorized.  Use a three-way
	 *  handshake to eliminate accidental reboot requests.  The first
	 *  REBOOT request causes a flag to be set and is acknowledged.  The
	 *  next request (with the immediately following ID) actually causes
	 *  the system to reload itself.
	 */
	case V1_RCT_REBOOT:
	{
	    register int i;

	    if (!rc_valid(dv, rc, sport, saddr))
	    {
		code = RCE_AUTH;
		goto error;
	    }
	    if (rcpreboot++ == 0)
	    {
  	        if (p->p_len != RCHEAD)
	 	   goto error;
	        profile(dv,rc_reboot);	/*dumb*/
		p->p_len = RCHEAD;
		rc_output(dv,p,ntohs(rc->rc_id), V1_RCT_OK,
		          sport, saddr, daddr);
	    }
	    else
	    {
                char bootstr[100];
                int  flag,bootlen;
                if (p->p_len > RCHEAD)
                   flag = 1;
                else if (p->p_len == RCHEAD)
                   flag = 0;
                else
	 	   goto error;
		printf("[ %s Host ", timeup(temp));
		printf("%s port %d requested REBOOT ]\r\n\r\r",
		       ip_fmt(saddr, temp), sport);
                if (flag) {
                    register c = 0;
                    do {
                       bootstr[c] = rc->rc_b_addr[c];
                    } while(rc->rc_b_addr[c++]);
                    bootlen = c;
                }
		p->p_len = RCHEAD;
   	        rc_output(dv,p,ntohs(rc->rc_id), V1_RCT_OK,
			  sport, saddr, daddr);
		flush();
                if (flag) {
                   if (bootlen > 1) {
		      trap15(bootstr);
                      printf("[Boot at %s]\n",bootstr);
		   }
		   else
                      trap15(1);
		}
                else
                   trap15(NULL);
	    }
	    return;
	}

	case V1_RCT_PGLSW:
	{
            struct glsw_tran *glsw = &(rc->rc_u.rcu_rglsw[0]);

	    if (!rc_valid(dv, rc, sport, saddr))
	    {
		code = RCE_AUTH;
		goto error;
	    }

            functionflags = ntohl(glsw->fcnflags);
            debugflags     = ntohl(glsw->dbgflags);

            p->p_len = RCHEAD + sizeof(struct glsw_tran);
	    rc_output(dv, p, ntohs(rc->rc_id), V1_RCT_RGLSW,
		      sport, saddr, daddr);
	    return;
           
        }

	case V1_RCT_GGLSW:
	{
            struct glsw_tran *glsw = &(rc->rc_u.rcu_rglsw[0]);

            glsw->fcnflags   = htonl(functionflags);
            glsw->dbgflags = htonl(debugflags);

            p->p_len = RCHEAD + sizeof(struct glsw_tran);
	    rc_output(dv,p,ntohs(rc->rc_id), V1_RCT_RGLSW,
		      sport, saddr, daddr);
	    return;
        }

	default:
	    profile(dv,rc_rtype);
	    code = RCE_TYPE;

	/*
	 *  Common error processing.
	 *
	 *  Send an RCP error reply message to the sender.
	 */
	error:
	    rcpvalid = FALSE;
	    p->p_len = RCHEAD+sizeof(rc->rc_code);
	    rc->rc_code = htons(code);
	    profile(dv,rc_xerr);
	    rc_output(dv,p,ntohs(rc->rc_id), V1_RCT_ERROR,
		      sport, saddr, daddr);
	    return;
    }
out:
    (*(p->p_done))(p);
}



/*
 *  rc_valid - validate special request function as authorized
 *
 *  dv    = device on which request was received
 *  rc    = RCP header
 *  saddr = IP source address of request
 *  sport = UDP source port of request
 *
 *  Check that the source address, port and request ID of the indicated
 *  request matches those which were recorded by the preceding authorization
 *  request.  If they match, increment the recorded request ID so that a
 *  subseqent request may be done without reauthorization.
 *
 *  Return: TRUE if the request is valid otherwise FALSE.
 */

rc_valid(dv, rc, sport, saddr)
struct device *dv;
struct rcp *rc;
struct socket *saddr;
short sport;
{
    if (rcpvalid && ntohs(rc->rc_id) == rcpid &&
        saddr->s_addr == rcpsaddr.s_addr && sport == rcpsport)
    {
	rcpid++;
	return(TRUE);
    }
    profile(dv,rc_invalid);
    return(FALSE);
}

/*
 *  rc_output - send an RCP packet to the specified address
 *
 *  dv    = device on which packet should appear to be transmitted from
 *  p     = packet containing the data of the datagram (the offset and length
 *	    fields are assumed set to define the RCP message area, including
 *	    the header)
 *  id    = the RCP ID for the packet
 *  type  = the RCP message type for the packet
 *  dport = the UDP destination port for the message
 *  daddr = destination IP address for the datagram
 *  saddr = source IP address for the datagram (NULL means use addres of
 *	    the transmitting interface)
 *
 *  Encapsulate the data in the appropriate RCP header and pass it up to the
 *  UDP protocol module for further encapsulation before trasnmission.
 */

rc_output(dv, p, id, type, dport, daddr, saddr)
struct device *dv;
struct packet *p;
short id;
short type;
register struct socket *daddr;
register struct socket *saddr;
short dport;
{
    static struct socket dhost;
    static struct socket shost;
    register struct rcp *rc;

    rc = poff(rcp, p);
    rc->rc_id = htons(id);
    rc->rc_type = htons(type);
    dhost.s_addr = daddr->s_addr;
    shost.s_addr = saddr->s_addr;
#ifdef	RCDEBUG
    if(p->p_flag&P_TRACE)
    {
	printf("XRCP (%d):\r\n", p->p_len);
	rc_prt(rc, p->p_len);
    }
#endif	RCDEBUG
    profile(dv,rc_xcnt);
    ud_output(dv, p, UD_RCP, dport, &dhost, &shost);
}

/*
 *  rc_prt - display an RCP message header
 *
 *  rc  = the header to display
 *  len = the physical length of the RCP message
 */

rc_prt(rc, len)
register struct rcp *rc;
{
    printf("ID: %d, Type: %d (",
	    ntohs(rc->rc_id), rc->rc_type);
    switch (rc->rc_type)
    {
	case V1_RCT_GET:
	    printf("GET)\r\nFrom=%o, Length=%d\r\n", rc->rc_from, rc->rc_len);
	    break;
	case V1_RCT_PUT:
	    printf("PUT)\r\nTo=%o\r\n", rc->rc_to, rc->rc_len);
	    break;
	case V1_RCT_DATA:
	    printf("DATA)\r\n");
	    break;
	case V1_RCT_AUTH:
	    printf("AUTH)\r\n");
	    break;
	case V1_RCT_REBOOT:

#ifdef RCDEBUG
  cprintf("RCP: received Vx_RCT_REBOOT request\r\n");
#endif RCDEBUG

	    printf("REBOOT)\r\n");
	    break;
	case V1_RCT_INFO:
	    printf("INFO)\r\n");
	    break;
	case V1_RCT_DIAG:
	    printf("DIAG)\r\n");
	    break;
	case V1_RCT_ERROR:
	    printf("ERROR)\r\n");
	    break;
	default:
	    printf("UNKNOWN)\r\n");
	    break;
    }
}

/*  rc_append

	This function is designed to replace rc_snatch,rc_dsnatch,
	rc_prepsnatch, and rc_finsnatch. When these functions are no
	longer supported, they and this reference to them should be
	deleted.

	This function appends a block of data of the requested size 
	to the end of an rcp packet. It is used by the rc_info routines
	and by the rc_diag routines, and expects that in addition to the
	standard rcp header the standard header for these two functions
	is contained in the packet as well.

	This function returns a pointer to the head of this newly
	accquired data block.

	rdi	: the diagfo packet which the data is to be appended to
	delta	: the length of the data block to add in BYTES
	plen	: pointer to length of packet; used to update overall
		  packet length
 */

char *rc_append (rdi,delta,plen)

  struct rcp_diagfo *rdi;
                int  delta;
		int *plen;
{
   rdi->length = ntohs(rdi->length);
   rdi->length += delta;
   *plen += delta;

#ifdef RCDEBUG
   if (((int)&(rdi->data[0])) & 0x1)
     cprintf("warning: odd addr rc_append\r\n");
#endif

   if ((sizeof(struct rcp) + rdi->length) > RCMAXDATA)
     return(NULL);				   /* too big for one packet */

#ifdef RCDEBUG
   cprintf("rc_append: returning length = %d\n\r",rdi->length);
#endif RCDEBUG

   rdi->length = htons(rdi->length);
   return (&(rdi->data[ntohs(rdi->length)-delta]));
}

/* V1_getdiagdv

	This function, compatiable with version 1 software, searches the
	device list for the specified card number. If it is found it
	returns a pointer to this device structure. If the specified card
	cannot be found it returns NULL and sets the status field of the
	given packet to reflect the fact that this card number could not
	be found.

	It looks for the card number in the diagfo header of given
	rcp packet.

 */

struct device *V1_getdiagdv(rc)

  register struct rcp *rc;

{
 /* always start from the first device */
    register struct device  *dv = dvhead;
    register        u_short  c  = ntohs (rc->rc_diagfo.u_di.card);
				  /* from the command */

/***
 *  cprintf ("DIAG card: %d diagtype: %d devtype: %d: ", c,
 *	    ntohs (rc->rc_diag.diag_type), ntohs (rc->rc_diag.diag_devtype));
 ***/

    if (c > 0)
	while (dv && --c > 0)
	    dv = dv->dv_next;
    else
	dv = NULL;		/* out of range device numbers become 0 */
/*BUG*/    if (!dv)
	rc->rc_diagfo.status = htons(RCED_NOCARD);

    return (dv);
}

/*  V1_do_rc_diag

	This function is compatible with the rc_diagfo packet header and
	uses rc_append to add data to the end of the outgoing packet.
	Therefore it is compatiable with version 1 of the router software.

	It performs the requested diagnosis on the requested card and
	returns the information, which is device specific, in the 
	given packet's data section.

 */

V1_do_rc_diag(rc, p, sport, saddr)

  register struct rcp    *rc;
           struct packet *p;
                  short   sport;
           struct socket *saddr;

{
    struct device  *dv      = V1_getdiagdv(rc);
	   u_short *devtype = (u_short *)rc->rc_diagfo.data;


    if (!dv)	/* if we didn't find a card, return */
	return;

    /* check that device types match if specified in input */
    if (*devtype && *devtype != htons(dv->dv_type))
	rc->rc_diagfo.status = htons(RCED_BADTYPE);
    else
    {
	*devtype = htons(dv->dv_type);

	if (dv->dv_diag)    /* check if diagnosis routine exists */
          /* now call diagnosis routine */
	  rc->rc_diagfo.status = htons((*dv->dv_diag)(dv,rc->rc_diagfo.subtype,
						     p, &rc->rc_diagfo,
						     rc, sport, saddr));
	else
	    /* no diagnosis routine found */
	    rc->rc_diagfo.status = htons(RCED_BADCARD);
    }
}

/*  V1_rc_uptime

    This function is an implementation of the remote info uptime request
    that is compatible with the rc_append function and version 1 of the
    client software.

    This function appends a block of data to the end of the standard
    rcp diagfo header and then initializes this data block to contain
    various router statistics.
 */

V1_rc_uptime(dv,p,rdi)

   struct device     *dv;
   struct packet     *p;
   struct rcp_diagfo *rdi;
{
    extern        long  time;
    extern struct queue pfreeq;
    extern        u_long maxfree;
    extern        int   versmajor,versminor,versedit;
    extern        char  versdate[];

    struct rcp_rstat *rt;
           u_short    vmajor = versmajor;
           u_short    vminor = versminor;
           u_short    vedit  = versedit;
           u_short    fpack  = lengthqueue(&pfreeq);
           long       frmem  = mavail();
    struct device    *dvl    = dv;
           int        pr     = 0;
           u_short    vlen;
    {
      register c = 0;
      while(versdate[c++]);
      vlen = c;
    }

    rt = (struct rcp_rstat *) 
         rc_append(rdi,sizeof(struct rcp_rstat) + vlen,&(p->p_len));

    rt->stat_vrecv = 0;
    rt->stat_vtran = 0;
    do {
       rt->stat_vrecv += dvl->dv_cnts->ip_rcnt;
       rt->stat_vtran += dvl->dv_cnts->ip_xcnt;
       dvl = dvl->dv_prnext[pr];
    } while(dvl != dv);

    rt->stat_vrecv  = htonl(rt->stat_vrecv);
    rt->stat_vtran  = htonl(rt->stat_vtran);
    rt->stat_timeup = htonl(time);
    rt->stat_frpac  = htons(fpack);
    rt->stat_frmem  = htonl(frmem);
    rt->stat_mxmem  = htonl(maxfree);
    rt->versmajor   = htons(vmajor); 
    rt->versminor   = htons(vminor); 
    rt->versedit    = htons(vedit);
    rt->versdlen    = htons(vlen);
    bcopy(versdate,&(rt->versdate[0]),vlen);
}

/*
   V1_rc_device 
 	
	Return all statistics associated with a given interface.

	This function is compatible with the version 1 client software,
	and in particular uses the new diagfo data structure.

	p	UDP packet which contains original info request
	rdi	diagfo data portion of UDP packet
 
 */

V1_rc_device (p,rdi)

   struct packet     *p;	/* packet which is being used */
   struct rcp_diagfo *rdi;	/* router info structure within p, used both
				   for the command and the returned results */

{
    extern struct   device   *dvhead;
           struct   devinfo   *rdev;
           struct   device    *dv;
                    u_short    c = ntohs(rdi->u_di.card);  /* from command */
           register int        i;


    dv = dvhead;		/* always start from the first device */
    while (dv && --c > 0)
	dv = dv -> dv_next;
    if (!dv) dv=dvhead;		/* out of range device numbers become 0 */

    rdi->length = 0;		/* clear all old data out of packet     */

    if ((rdev = (struct devinfo   *)
                rc_append (rdi, sizeof (struct devinfo), &(p->p_len))) == NULL)
	return(ERROR_EXIT);

    rdev -> dvc_br = 0;		/*	BUG obsolete field	*/
/*    rdev -> dvc_nunit = 1; obsolete field was: htonl((long)dv->dv_nunit)*/
    rdev -> dvc_istatus = htonl ((long)dv -> dv_istatus);
    rdev -> dvc_dstate = htonl ((long)dv -> dv_dstate);
    rdev -> dvc_cable = htonl ((long)dv -> dv_cable);
    rdev -> dvc_devtype = htonl ((long) dv -> dv_type);
    rdev -> dvc_spare = 0;

    bcopy (dv -> dv_phys, rdev -> dvc_phys, MAXHWADDRL);
    bcopy (dv -> dv_praddr, rdev -> dvc_praddr, PRL_ALL);

    for (i=0;i<((sizeof (struct devstats))/(sizeof (long)));i++) {
	((long *) &rdev->ds)[i] = htonl( ((long *) dv->dv_cnts)[i] );
    }

}

