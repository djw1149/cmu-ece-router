/*
 *  Address resolution module
 *
 **********************************************************************
 * HISTORY
 *  20-Sep-86 Matt Mathis at CMU
 *	Changed to keep it's own internal internal memory free pools to
 *	avoid fragmenting the malloc pool.
 *	Addeded code to discard arp with unreasonable protocol addresses.
 *	Added preliminary subnet hooks, and a hack to reduce CS arp traffic.
 *
 *  1-Aug-86 Matt Mathis at CMU
 *	errorlog lockerrors to identify bad ARP, extended the ageing
 *	timer to 20 minutes
 *
 *  9-Jan-86 Kevin Kirmse (kdk) at CMU 
 *      Added the ability to shut down arp handling for external
 *      hosts.
 *
 *  5-Dec-85 Matt Mathis (mathis) at CMU
 *	Updated to use shared device statistics 
 *
 * 10-Aug-85  Kevin Kirmse (kdk) at Carnegie-Mellon University
 *      Added LRU (least recently access) queue. Added functions  
 *      ar_addlru(), ar_remlru(), ar_xppr(), ar_xppring. Changed 
 *      ar_input to support and utilize pp requests and replys.
 *  
 * 12-Oct-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed ar_remap() and ar_map() to update the reference time in
 *	the translation cache entry whenever it is accessed;  changed
 *	ar_remap() to initialize new am_flag field; added new
 *	ar_unmap() routine which discards a cache entry and new
 *	ar_ring() routine which implements the cache timer [V2.0(402)].
 *
 * 28-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Changed ar_recast() to synchronize at BR7 since the system now
 *	allows devices at this level and to use new POFF variable
 *	as the initial offset for generated packets.
 *
 * 25-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed bug in neglecting to check if ARP is supported on device
 *	before sending attempting to forward reply messages.  This is
 *	a problem for CPU interfaces which have address resolution
 *	mapping entries in the table but do not support the protocol.
 *
 * 10-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Fixed bug in neglecting to protect against matching protocol
 *	type 0 in the device protocol list (where it indicates that
 *	the corresponding protocol is not supported).
 *
 * 03-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

/*
 *  Address resolution information (the mapping of arbitrary protocol addresses
 *  to hardware physical addresses) is maintained in a hash table indexed by
 *  protocol address.  The hash table is organized as an array of queues of
 *  address mapping entries.  Each queue in the table contains all entries
 *  whose protocol address hashes to that index in the array.
 *
 *  An address mapping entry consists of the following four fields:
 *
 *  - a protocol identifer
 *  - the protocol address
 *  - the corresponding hardware physical address
 *  - a pointer to the network device on which the hardware address exists
 *
 *  Both the protocol and hardware address fields are represented as arrays of
 *  bytes.  The length of the protocol address is implicitly specified by the
 *  protocol identifier and the length of the hardware address is implicitly
 *  specified by its associated network device.
 *
 *  In addition to maintaining the address resolution table, this module
 *  also processes incoming address resolution message and generates
 *  appropriate outgoing address resolution messages both on behalf of
 *  the current host as well as hosts on other cables.
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/time.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/arp.h"
#include "../../h/errorlog.h"
#include "../../h/globalsw.h"
#include "../../h/profile.h"
#include "../../h/malloc.h"
#include "debug/ar.h"
#include "cond/ar.h"

#define NULL 0

/*
 *  The address resolution hash table array.  The queue headers in this array
 *  are set up empty during the initialization procedures.
 */
struct qentry addmap[NADDMAP] = {0};
int ar_rescnt = 0;

/* 
 * LRU queue
 */

struct queue ar_lruq = INITQUEUE(&ar_lruq);

/*
 * The free pools.  We could be using malloc, but that subjects the malloc
 *	free pool to fragmentation.   Rather than using a complex malloc
 *	keep our own internal free pools here.
 */

struct queue arfree = INITQUEUE(&arfree);
struct queue lrufree = INITQUEUE(&lrufree);

/*
 *  The number of well-formed address resolution messages processed by the
 *  system.
 */
long ar_cnt = 0;

/*
 *  FLag to indicate that source has moved and a broadcast should
 *  be rebroadcast.
 */
short ar_src_moved = 0;

extern char *pa_fmt();
extern struct addmap *ar_ckmap();

/*
 *  ar_ring - ARP cache timer
 *
 *  Scan the translation cache and remove any entries which have expired
 *  because they have not been referenced recently.
 *
 *  This routine is called at AM_IRING intervals by the clock process.
 */

ar_ring()
{
    register struct qentry *qe_end;
    register struct addmap *am;
    static ringcnt = 0;
    static struct addmap *ringam = NULL;

    if (!ringam) {
	if (++ringcnt == NADDMAP)
	   ringcnt = 0;
	ringam = (struct addmap *) addmap[ringcnt].qe_next;
    }
    qe_end = &addmap[ringcnt];
    am     = ringam;

    while(am != qe_end) {
      if (am->am_flag&AM_XMITPP) {
	  ar_xppring(am);
	  ringam = am;
	  return;
      }
      else if ((am->am_flag&AM_PERM)==0 && (time-am->am_when)>=AM_UNREF) {
	   if (am->am_flag&AM_REQBCAST)
	  {
	      struct addmap *am_prev = (struct addmap *) am->am_link.qe_prev;
	      ar_unmap(am);
	      ar_profile(am->am_dv,ar_rng_unm);
	      am = am_prev;
	  }
	   else {
#ifdef RINGDEBUG
	      cprintf("ar_ring [%d] [%d] : pp xmit \n\r",ar_rescnt,ringcnt);
#endif RINGDEBUG
	      if (!(am->am_flag&AM_INLRUQ)) {
		  ar_profile(am->am_dv,ar_rng_alru);
	         ar_addlru(am);
	      }
	      am->am_when = time;
              ar_xppring(am);
              ringam = am;
              return;
	  }
       }
       am = (struct addmap *) am->am_link.qe_next;
    }
    ringam = NULL;

#ifdef RINGDEBUG
    cprintf("ar_ring [%d] [%d] : memory check \n\r",ar_rescnt,ringcnt);
#endif RINGDEBUG

    while(ar_rescnt > MAXADDRES && !emptyqueue(&ar_lruq)) {
       ar_unmap(((struct ar_lru *) ar_lruq.q_tail)->lru_addmap);
    }
}

/*
 *  ar_input - process an incoming address resolution message
 *
 *  dv    = the network device which received the message
 *  p     = the packet containing the message
 *  shost = a pointer (presumably into the network header of the packet)
 *	    indicating the hardware source address of the message. 
 *
 *  Restructure the 16-bit (hardware address space, protocol address space and
 *  opcode) or paired 8-bit (hardware and protocol address lengths) fields of
 *  the message as appropriate for the input device.  This corrects byte-
 *  swapped short word fields on byte-oriented devices (assuming the usual
 *  high-byte,low-byte transmission order) and exchanges the values of
 *  sequential bytes on word-oriented devices.
 *  
 *  Check the message for consistency.  The physical length of the packet must
 *  be at least as great as the size of the message calculated from the fixed
 *  header length and the protocol and hardware address lengths.  The hardware
 *  address space must match the hardware address space of the receiving
 *  network device.  The protocol address space must be found in the table of
 *  protocols.  The protocol table entry must indicate support for address
 *  resolution and the protocol address length must match the length recorded
 *  for that protocol in the table.  The hardware address length must match
 *  the hardware address length of the receiving network device.
 *
 * If the protocol is IP, then the protocol address is checked to be sure that
 * it is reasonable.  In particular both the source and destination addresses
 * must be from the same network as the router, and within this net they
 * must not be either the "broadcast" or "any" address.
 *  
 *  Once the message contents has been validated, examine the address
 *  information (which must first be byte-swapped on word-oriented devices).
 *  The source hardware address of the message must match the specifed source
 *  hardware address of the packet (to forestall fraud).  If the source
 *  hardware address of the packet matches the hardware address of the
 *  receiving network device, ignore the message since it was sent from the
 *  current host (e.g. by broadcast) and hence the source hardware address has
 *  already been "adjusted".
 *  
 *  Provided that the message passes all of these checks, enter the source
 *  protocol to hardware address mapping into the hash table along with the
 *  protocol identifier and the receiving network device.  If the message is a
 *  request addressed to the current host, simply generate and send the
 *  appropriate reply .  If it is not such a request or is instead a reply
 *  message, then pass the message on to arforw() for further processing.
 *  
 *  TODO: change to rebroadcast in seperate process.
 *  TODO: use general comparison routine to compare addresses.
 *  TODO: subnet hooks 
 */

ar_input(dv, p, shost)
register struct device *dv;
register struct packet *p;
char *shost;
{
    register struct addres *ar = poff(addres, p);
    register struct addmap *am;
    int pr;
    int i;
    struct device *dest_me;
    int aofs;

    ar->ar_hrd=ntohs(ar->ar_hrd);
    ar->ar_pro=ntohs(ar->ar_pro);
    ar->ar_op=ntohs(ar->ar_op);

#ifdef	ARDEBUG
    if(p->p_flag&P_TRACE)
    {
	printf("RAR (%d): ", p->p_len);
	arprth(ar);
    }
#endif	ARDEBUG
/*							Check packet length */
    if (p->p_len < (ARHEAD + ((ar->ar_hln+ar->ar_pln)<<1)))
    {
#ifdef	ARDEBUG
	if(p->p_flag&P_TRACE)
	    printf("ar rmin %d\r\n", p->p_len);
#endif	ARDEBUG
	ar_profile(dv,ar_rmin);
        errorlog(p,EL_AR_RMIN);
	goto out;
    }
/*							Check hardware type */
    if (ar->ar_hrd != dv->dv_hrd)
    {
#ifdef	ARDEBUG
	if(p->p_flag&P_TRACE)
	    printf("ar rhrd != %o\r\n", dv->dv_hrd);
#endif	ARDEBUG
	ar_profile(dv,ar_rhrd);
        errorlog(p,EL_AR_RHRD);
	goto out;
    }
/*					Determine and check protocol type */
    for (pr=0; pr<NPR; pr++)
	if (dv->dv_pr[pr] == ar->ar_pro)
        {
	    break;
	}
    if (ar->ar_pro == 0 || pr == NPR || (proto[pr].pr_flag&PRF_AR) == 0)
    {
#ifdef	ARDEBUG
	if(p->p_flag&P_TRACE)
	    printf("ar rpro\r\n");
#endif	ARDEBUG
	ar_profile(dv,ar_rpro);
        errorlog(p,EL_AR_RPRO);
	goto out;
    }
/*						Check HW address length */
    if (ar->ar_hln != dv->dv_hln)
    {
#ifdef	ARDEBUG
	if(p->p_flag&P_TRACE)
	    printf("ar rhln != %d\r\n", dv->dv_hln);
#endif	ARDEBUG
	ar_profile(dv,ar_rhln);
        errorlog(p,EL_AR_RHLN);
	goto out;
    }
#ifdef	ARDEBUG
    if(p->p_flag&P_TRACE)
    {
	arprta(ar);
    }
#endif	ARDEBUG
/*					Check the from network address */
    for (i=dv->dv_hln; i--; )
	if (ar_sha(ar)[i] != shost[i])
	{
	    ar_profile(dv,ar_rfraud);
#ifdef	ARDEBUG
	    if(p->p_flag&P_TRACE)
	    {
		printf("ar: fraud %s \r\n", pa_fmt(shost, dv->dv_hln));
	    }
#endif	ARDEBUG
            errorlog(p,EL_AR_RFRAUD);
	    goto out;
	}
/*				Discard my own looped back broadcasts */
    for (i=dv->dv_hln; i--; )
	if (dv->dv_phys[i] != shost[i])
	    goto notmine;
	ar_profile(dv,ar_rmine);
#ifdef	ARDEBUG
	if(p->p_flag&P_TRACE)
	{
	    printf("ar: mine %s\r\n", pa_fmt(shost, dv->dv_hln));
	}
#endif	ARDEBUG
	goto out;
notmine:
/*				Discard unreasonable IP addresses */
    if (pr == PR_IP && (ar_badIP(ar_spa(ar)) || ar_badIP(ar_tpa(ar))))
	{
	    ar_profile(dv,ar_rfraud);
            errorlog(p,EL_AR_RFRAUD);
#ifdef	ARDEBUG
	    if(p->p_flag&P_TRACE)
	    {
		printf("ar: fraud %s \r\n", pa_fmt(shost, dv->dv_hln));
	    }
#endif	ARDEBUG
	    goto out;
	}

/*				End of the consistancy checking */
    profile(dv,ar_rcnt);
    ar_cnt++;

/* BUG HACK			Discard intra CS/RI/SEI traffic */
#ifdef ARNOCS
    if (pr == PR_IP && ar_csIP(ar_spa(ar)) && ar_csIP(ar_tpa(ar)))
	{
	    goto out;	/* not an error */
	}
#endif ARNOCS

    if (!(am = ar_remap(pr, ar_spa(ar), ar_sha(ar), dv))){
	errorlog(p,EL_AR_LOCKER);	 /* BUG could also be out of memory */
	goto out;
    }

    if (functionflags & FCNF_ARPCPY)
	errorlog(p,EL_AR_PACKET);

    /*
     *  Look through all my addresses in this protocol.
    */

    aofs = proto[pr].pr_aofs;
    for (dest_me=dvhead; dest_me; dest_me=dest_me->dv_next)
    {
       int len = proto[pr].pr_pln;
       if (dest_me->dv_pr[pr] == 0)
	  continue;
	  while (len--)
	     if (dest_me->dv_praddr[aofs+len] != ar_tpa(ar)[len])
		goto next;
	  break;
       next:;
    }

    if (ar->ar_op == AR_REQUEST){
	if (dest_me)
	{
	    ar_profile(dv,ar_req_me);
	    ar_output(dv, p, pr, AR_REPLY,ar_sha(ar), ar_spa(ar),
		      dv->dv_phys, &dest_me->dv_praddr[aofs]);
	    return;
	}
        else if (functionflags & FCNF_ARPDOWN)	/* don't forward thru arp */
            goto out;
	else if (p->p_flag&P_BCAST)       /* broadcast request */
	{
            struct addmap *dest_am;
            if (!(dest_am = ar_ckmap(pr,ar_tpa(ar)))) {
               ar_profile(dv,ar_req_bc_dnc);
 	       ar_recast(dv, p, 1, pr, ar_tpa(ar), ar_spa(ar));
               return;
	    }
            else if (ar_src_moved) {
		ar_profile(dv,ar_req_bc_smv);
		errorlog(p,EL_AR_SMV);
		ar_recast(dv, p, 1, pr, ar_tpa(ar), ar_spa(ar));
		return;
	    }
            else if (dest_am->am_flag&AM_REQBCAST)
	    {
               ar_profile(dv,ar_req_bc_rbc);
 	       ar_recast(dv, p, 1, pr, ar_tpa(ar), ar_spa(ar));
               return;
            }
            else if (dest_am->am_dv->dv_cable != am->am_dv->dv_cable)
	    {
               ar_profile(dv,ar_req_bc_sndn);
               ar_xppr(p,AR_REQUEST,dest_am,am->am_pa);
               return;
            }
            else
            {
               ar_profile(dv,ar_req_bc_rem);
	       am->am_flag |= AM_XMITPP;
               goto out;
	    }
	}
        else                /* Point to Point request */
        {
            struct addmap *dest_am;
            if (!(dest_am = ar_ckmap(pr,ar_tpa(ar))))
            {
               ar_profile(dv,ar_req_pp_dnc);
               goto out;
	    }
            else if (dest_am->am_dv->dv_cable != am->am_dv->dv_cable)
	    {
               ar_profile(dv,ar_req_pp_sndn);
               ar_xppr(p,AR_REQUEST,dest_am,am->am_pa);
               return;
            }
            else 
            {
               ar_profile(dv,ar_req_pp_rem);
	       am->am_flag |= AM_XMITPP;
               goto out;
            }           
        }
    }
    else if (ar->ar_op == AR_REPLY)
    {
        if (dest_me) 
        {
           ar_profile(dv,ar_rep_me);
           goto out;
	}
        else if (functionflags & FCNF_ARPDOWN)	/* don't forward thru arp */
            goto out;
        else if (p->p_flag&P_BCAST)
	{
           ar_profile(dv,ar_rep_bc);
           goto out;
	}
        else 
	{
	    struct addmap *dest_am;

	    if (!(dest_am = ar_ckmap(pr, ar_tpa(ar))) ||
                     !(dest_am->am_dv->dv_pr[PR_AR]) )
            {
               ar_profile(dv,ar_rep_pp_dnc);
               goto out;
	    }
            else
	    {
                ar_profile(dv,ar_rep_pp_for);
                ar_xppr(p,AR_REPLY,dest_am,ar_spa(ar));
		return;
	    }
	}
    }

out:
    (*(p->p_done))(p);
}


/*
 *  ar_remap - add a mapping entry to the hash table
 *
 *  pr = the protocol identifier
 *  pa = the protocol address to be added
 *  ha = its corresponding hardware address
 *  dv = the network device for the hardware address
 *
 *  Hash the protocol address a table index and search for the protocol
 *  (identifier,address) pair in the resulting queue.  If the pair is not
 *  found, allocate a new address mapping entry filled in with the protocol
 *  identifier and address and insert it at the head of that queue, otherwise
 *  reuse the existing entry.  In either case, store the supplied hardware
 *  address and network device information in the entry and mark it referenced.
 *  
 *  Return: a pointer to the new or reused address mapping entry, or 0 if the
 *  protocol (identifier,address) pair is not currently in the table and a new
 *  mapping entry can not be allocated.
 *  
 *  TODO: change to only copy protocol information for new entries.
 *  TODO: change to move existing entry to head of queue?
 */

struct addmap *
ar_remap(pr, pa, ha, dv)
int pr;
char *pa;
char *ha;
struct device *dv;
{
    register struct qentry *qe;
    register struct addmap *am;
    register int len = proto[pr].pr_pln;

#ifdef	ARMDEBUG
    cprintf("ar_remap: %d [ %s ", pr, pa_fmt(pa, len));
    cprintf("%s ", pa_fmt(ha, dv->dv_hln));
#endif	ARMDEBUG

    ar_src_moved = 0;

    qe = &addmap[arhash(pa, len)];
    for (am=(struct addmap *)qe->qe_next;
         am != (struct addmap *)qe;
	 am=(struct addmap *)am->am_link.qe_next)
    {
	if (am->am_pr != pr)
	    continue;
	for (len=proto[pr].pr_pln; len--; )
	    if (am->am_pa[len] != pa[len])
		goto next;
	break;
next:
	;
    }
    if (am == (struct addmap *)qe)
    {
	if (am = lmalloc(addmap,arfree))
	{
            ar_profile(dv,ar_runknown);
            ar_rescnt++;
	    insqueue(am, qe);
   	    bcopy(pa, am->am_pa, proto[pr].pr_pln);
	    bcopy(ha, am->am_ha, dv->dv_hln);
	    am->am_pr = pr;
	    am->am_dv = dv;
	    am->am_when = time;
            am->am_lockout = time;
	    am->am_flag = 0;
#ifdef	ARMDEBUG
  	    cprintf(" ]\r\n");
#endif	ARMDEBUG
            return(am);
	}
        else {
#ifdef	ARMDEBUG
  	    cprintf(" ? ]\r\n");
#endif	ARMDEBUG
            return(NULL);
	}
    }
    else
    {
        for(len = dv->dv_hln;len--;)
	    if (am->am_ha[len] != ha[len])
               break;
  
        if (len < 0 && dv == am->am_dv)
        {    
           ar_profile(dv,ar_runchanged);
           am->am_flag &= ~AM_REQBCAST;
           am->am_flag &= ~AM_XMITPP;
	   am->am_lockout = time;
#ifdef	ARMDEBUG
 	   cprintf(" unchanged ]\r\n");
#endif	ARMDEBUG
           return(am);
        }           
        else if (time - am->am_lockout < AM_LOCKOUT) {
           ar_profile(dv,ar_rlockerr);
#ifdef	ARMDEBUG
  	   cprintf("[ lock error : %s ", pa_fmt(am->am_pa, proto[pr].pr_pln));
	   cprintf("%s ] ]\n\r", pa_fmt(am->am_ha, am->am_dv->dv_hln)); 
#endif	ARMDEBUG
           return(NULL);
	 }
         else {
           ar_profile(dv,ar_rchanged);
           ar_src_moved = 1;
           bcopy(ha, am->am_ha, dv->dv_hln);
	   am->am_pr = pr;
	   am->am_dv = dv;
	   am->am_when = time;
           am->am_lockout = time;
           am->am_flag &= ~AM_REQBCAST;
           am->am_flag &= ~AM_XMITPP;
#ifdef	ARMDEBUG
	   cprintf("[ was %s ", pa_fmt(am->am_pa, proto[pr].pr_pln));
	   cprintf("%s ] ]\n\r", pa_fmt(am->am_ha, am->am_dv->dv_hln)); 
#endif	ARMDEBUG
           return(am);
	 }
    }
}


/*
 *  ar_map - lookup a protocol address in the address resolution hash table
 *
 *  pr = the protocol identifier
 *  pa = the protocol address to be added
 *
 *  Hash the protocol address into a table index and search for the protocol
 *  (identifier,address) pair in the resulting queue.  If found, the
 *  appropriate cache entry is shown as referenced recently.
 *  
 *  Return: a pointer to the located address mapping entry, or 0 if
 *  the protocol (identifier,address) pair is not found in the table.
 *
 *  TODO: change to move existing entry to head of queue?
 */

struct addmap *
ar_map(pr, pa)
int pr;
char *pa;
{
    register struct qentry *qe;
    register struct addmap *am;
    register int len = proto[pr].pr_pln;

#ifdef	AMDEBUG
    cprintf("ar_map: %d [ %s ", pr, pa_fmt(pa, len));
#endif	AMDEBUG
    qe = &addmap[arhash(pa, len)];
    for (am=(struct addmap *)qe->qe_next;
         am != (struct addmap *)qe;
	 am=(struct addmap *)am->am_link.qe_next)
    {
	if (am->am_pr != pr)
	    continue;
	for (len=proto[pr].pr_pln; len--; )
	    if (am->am_pa[len] != pa[len])
		goto next;
	break;
next:
	;
    }
    if (am != (struct addmap *)qe)
    {
#ifdef	AMDEBUG
	cprintf("%s ]\r\n", pa_fmt(am->am_ha, am->am_dv->dv_hln));
#endif	AMDEBUG
	am->am_when = time;

        if (ar_remlru(am))
           ar_profile(am->am_dv,ar_lru_used);
    }
    else
    {
	am = 0;
#ifdef	AMDEBUG
	cprintf("? ]\r\n");
#endif	AMDEBUG
    }
    return(am);
}

/*
 *  ar_ckmap - lookup a protocol address in the address resolution hash table
 *
 *  pr = the protocol identifier
 *  pa = the protocol address to be added
 *
 *  Hash the protocol address into a table index and search for the protocol
 *  (identifier,address) pair in the resulting queue. Unlike ar_map()
 *  this procedure does not update the addmap entry as being referenced
 *  recenty.
 *  
 *  Return: a pointer to the located address mapping entry, or 0 if
 *  the protocol (identifier,address) pair is not found in the table.
 *
 */

struct addmap *
ar_ckmap (pr, pa)
int pr;
char *pa;
{
    register struct qentry *qe;
    register struct addmap *am;
    register int len = proto[pr].pr_pln;

    qe = &addmap[arhash(pa, len)];
    for (am=(struct addmap *)qe->qe_next;
         am != (struct addmap *)qe;
	 am=(struct addmap *)am->am_link.qe_next)
    {
	if (am->am_pr != pr)
	    continue;
	for (len=proto[pr].pr_pln; len--; )
	    if (am->am_pa[len] != pa[len])
		goto next;
	break;
next:
	;
    }
    if (am != (struct addmap *)qe) {
       return(am);
    }
    else {
       return(NULL);
    }
}


/*
 *  arhash - hash a protocol address into a table index
 *
 *  pa = protocol address
 *  pl = protocol address length (bytes)
 *
 *  Add together all of the bytes of the protocol address, shifting
 *  the sum left 1 bit position each iteration.
 *
 *  Return: the resulting sum modulo the size of the address resolution
 *  table (NADDMAP).
 *
 *  TODO: is this algorithm fast enough?
 *  TODO: does it yield reasonable distributions?
 */

arhash(pa, pl)
register u_char *pa;
register int pl;
{
   register u_int hash = 0;

   while (pl--)
   {
	hash <<= 1;
	hash += *pa++;
   }
   return(hash%NADDMAP);
}

/*
 *  ar_unmap - remove an ARP translation entry from the cache
 *
 *  am = translation entry to be removed
 *
 *  Remove the specified translation table entry from the table (by unlinking it
 *  from its queue) and free its space.
 *
 *  It is incorrect to subsequently refer to any fields of the removed entry
 *  upon return from this routine.  The entry no longer exists.
 */

ar_unmap(am)
register struct addmap *am;
{
#ifdef LRUDEBUG
    cprintf("ar_unmap :\n\r");
#endif LRUDEBUG

    ar_rescnt--;
    ar_remlru(am);
    remqueue(&am->am_link);
    lfree(am,arfree);
}

/*
 *  ar_remlru() - remove an entry from the lru queue
 *
 *  am = translation entry that is to be removed from the lru queue
 *
 */

ar_remlru(am)
register struct addmap *am;
{

    if (!(am->am_flag&AM_INLRUQ))
       return(0);
    am->am_flag &= ~AM_INLRUQ;
    decrq(&ar_lruq);
    remqueue(&(am->am_lru->lru_link));
    lfree(am->am_lru,lrufree);
#ifdef LRUDEBUG
    cprintf("ar_remlru: ");
    cprintf("[lru queue length %d]\n\r",lengthqueue(&ar_lruq));
#endif LRUDEBUG
    return(1);
}

/*
 *  ar_addlru() - add an entry to the lru queue
 *
 *  am = translation entry that is to be added to the lru queue
 *
 */

ar_addlru(am)
register struct addmap *am;
{
    struct ar_lru *qe;
    qe = lmalloc(ar_lru,lrufree);

    if (qe) {
	incrq(&ar_lruq);
	insqueue(qe,&ar_lruq);
	qe->lru_addmap = am;
	am->am_lru = qe;
	am->am_flag |= AM_INLRUQ;
    }
#ifdef LRUDEBUG
    cprintf("ar_addlru: ");
    cprintf("[lru queue length %d]\n\r",lengthqueue(&ar_lruq));
#endif LRUDEBUG
}

/*
 *  ar_recast - retransmit an adjusted address resolution message on all cables
 *
 *  dv   = the network device receiving the original message
 *  p    = the packet containing the message
 *  flag = non-zero to omit the input device
 *  pr   = the protocol identifier to use
 *  tpa  = the target protocol address to use in the message
 *  spa  = the source protocol address to use in the message
 *
 *  Make local copies of the target and source protocol addresses since they
 *  may be pointers into the message which is about to be adjusted and then
 *  free the packet.  Iterate through the list of all network devices and send
 *  an address resolution message of the specified type on all cables matching
 *  the network protocol address of the receiving cable.
 *
 *  TODO: change to handle both requests and replies
 *  TODO: change to handle directed messages as well as broadcasts.
 */

ar_recast(dv, p, flag, pr, tpa, spa)
struct device *dv;
register struct packet *p;
int pr;
char *tpa;
char *spa;
{
    register struct device *dvl;
    char stpa[PRL_MAX];
    char sspa[PRL_MAX];
    int tracef = (p->p_flag&P_TRACE);

    ar_profile(dv,ar_xcast);

    bcopy(tpa, stpa, proto[pr].pr_pln);
    bcopy(spa, sspa, proto[pr].pr_pln);
    (*(p->p_done))(p);

    for (dvl=dv->dv_prnext[pr]; flag == 0 || dvl != dv; dvl=dvl->dv_prnext[pr])
    {
	if (dvl->dv_pr[PR_AR])
	{
	    int br;

	    br = spl7();
	    while((p=palloc()) == 0)
		sleep(&pfreeq);
	    spl(br);
	    mapp(p);
	    p->p_off = POFF;
	    p->p_len = PLEN;
	    p->p_flag |= tracef;
	    ar_output(dvl, p, pr, AR_REQUEST,
		      dvl->dv_bcast, stpa, dvl->dv_phys, sspa);
	}
	if (dvl == dv)
	    break;
    }
}

/*
 *  ar_xppr() - send a point to point message
 *
 *  p    = the packet to contain the message
 *  op   = AR_REQUEST or AR_REPLY
 *  am   = the addmap entry for the target of the pp message
 *  spa  = the source protocol address to use in the message
 *  
 *
 *  
 */

ar_xppr(p,op,am,spa)
struct packet *p;
int op;
register struct addmap *am;
char *spa;
{
#ifdef PPDEBUG
    cprintf("ar_xppr: ");
    if (op == AR_REQUEST)
       cprintf("AR_REQUEST ");
    else
       cprintf("AR_REPLY ");
    cprintf("[ %s ] -> ",
                    pa_fmt(am->am_dv->dv_phys, am->am_dv->dv_hln));
    cprintf("[ %s ] \n\r",pa_fmt(am->am_ha, am->am_dv->dv_hln));
#endif PPDEBUG
    if (op == AR_REQUEST) {
       am->am_when = time;
       am->am_flag |= AM_REQBCAST;
       am->am_flag &= ~AM_XMITPP;
       ar_profile(am->am_dv,ar_xppreq);
    }
    return(ar_output(am->am_dv, p, am->am_pr, op,
	       am->am_ha, am->am_pa, am->am_dv->dv_phys, spa));
}

/*
 *  ar_xppring() - send a point to point request source me.
 *
 *  am   = the addmap entry for the target of the pp request
 *  
 */

  
ar_xppring(am)
register struct addmap *am;
{
   register struct packet *p;
   register int br;
   br = spl7();
   while((p=palloc()) == 0)
      sleep(&pfreeq);
   spl(br);
   mapp(p);
   p->p_off = POFF;
   p->p_len = PLEN;
   ar_profile(am->am_dv,ar_rng_xpp);
   ar_xppr(p, AR_REQUEST, am, am->am_dv->dv_praddr);
}


/*
 *  ar_output - format an address resolution message and transmit
 *
 *  dv  = device on which to transmit message
 *  p   = packet to use for message
 *  pr  = protocol identifier for message
 *  op  = message opcode to send (request/reply)
 *  tha = the target hardware address
 *  tpa = the target protocol address
 *  sha = the source hardware address
 *  spa = the source protocol address
 *
 *  Make a local copy of the source protocol address since it may be the old
 *  target protocol address in the packet to be formatted.  Fill in the
 *  hardware and protocol address space, and hardware addess length fields of
 *  the message with the values appropriate to the specified device.  Fill in
 *  the protocol address length field as appropriate for the specified
 *  protocol.  Set the target and source hardware and protocol addresses as
 *  specified.  On byte-oriented devices, byte-swap the word fields (hardware
 *  address space, protocol address space and opcode) otherwise byte-swap the
 *  paired byte fields (hardware and protocol address lengths) and the actual
 *  source and target addresses.  Finally, send the packet to the output
 *  routine of the specified device for transmission as an Address Resolution
 *  protocol packet to the specified target hardware address (which may be the
 *  device broadcast address).
 */

ar_output(dv, p, pr, op, tha, tpa, sha, spa)
struct device *dv;
struct packet *p;
int pr;
int op;
char *tha;
char *tpa;
char *sha;
char *spa;
{
    register struct addres *ar = poff(addres, p);
    register int i;
    char dhost[6];
    char sspa[4];

 
    bcopy(spa, sspa, proto[pr].pr_pln);
    ar->ar_hrd = dv->dv_hrd;
    ar->ar_pro = dv->dv_pr[pr];
    ar->ar_hln = dv->dv_hln;
    ar->ar_pln = proto[pr].pr_pln;
    ar->ar_op = op;
    bcopy(tha, dhost, ar->ar_hln);
    bcopy(tha, ar_tha(ar), ar->ar_hln);
    bcopy(tpa, ar_tpa(ar), ar->ar_pln);
    bcopy(sha, ar_sha(ar), ar->ar_hln);
    bcopy(sspa, ar_spa(ar), ar->ar_pln);
    p->p_len = ARHEAD+((ar->ar_hln+ar->ar_pln)<<1);
#ifdef	ARDEBUG
    if(p->p_flag&P_TRACE)
    {
	printf("XAR (%d): ", p->p_len);
	arprth(ar);
	arprta(ar);
    }
#endif	ARDEBUG

    ar->ar_hrd=htons(ar->ar_hrd);
    ar->ar_pro=htons(ar->ar_pro);
    ar->ar_op=htons(ar->ar_op);

    switch ((*(dv->dv_output))(dv, p, PR_AR, dhost))
    {
	case DVO_DEAD:
	case DVO_DOWN:
	case DVO_DROPPED:
	    ar_profile(dv,ar_xdrop);
	    (*(p->p_done))(p);
	    break;
	case DVO_QUEUED:
	    ar_profile(dv,ar_xcnt);
	    break;
    }
}

/*
 *
 * ar_badIP, check a given IP address, to be sure that it is reasonable
 *	for this network.   It must be from the same network (ie:128.2.x.x)
 *	and must not be for either the "broadcast" or "any" address (ie: not
 *	128.2.255.255 or 128.2.0.0).
 *	For efficiency and to reduce the inter-dependencys between the IP
 *	and ARP code, type punning is used.   The globals ar_IPmask, 
 *	and ar_IPnet are set up elsewhere (in ipI.c) and
 *	treated as longs here.   They should be struct socket.
 */
long ar_IPmask, ar_IPnet;

ar_badIP(addr)
long * addr;
{
#ifdef ARIPCHK
    printf ("addr=%x, mask=%x, net=%x\r\n",*addr,ar_IPmask,ar_IPnet);
#endif ARIPCHK
    return (	((*addr & ar_IPmask) != ar_IPnet) ||
		((*addr & ~ar_IPmask) == 0) ||
		((*addr | ar_IPmask) == ~0)
	   );
}
/*
 * ar_csIP, A hack to reduce background ARP traffic at CMU by stomping on all
 *	CS/RI/SEI to CS/RI/SEI traffic.   CRITICAL: this only works because 
 *	none of the systems in the above departments are seperated by 68k
 *	routers.  To be removed when we use real subneting.
 */
#ifdef ARNOCS
ar_csIP(addr)
u_char *addr;	/* now pun as chars (again this should be socket) */
{
    return (	(addr[2]==254) ||	/* CS priv 3 Mbit */
		(addr[2]==250) ||	/* CS public 10 */
		(addr[2]==237) ||	/* SEI */
		(addr[2]==222) );	/* additional hosts on CS pub 10 */
}
#endif ARNOCS

#ifdef	ARDEBUG
/*
 *  arprth - print the header fields of an address resolution message
 *
 *  ar = the address resolution message
 *
 *  Print the values of the hardware and protocol address space, opcode, and
 *  hardware and protocol length fields of the specified address resolution
 *  message.
 */

arprth(ar)
register struct addres *ar;
{
    printf("hrd=%o, pro=%o, op=%d, hln=%d, pln=%d\r\n",
	    ar->ar_hrd, ar->ar_pro, ar->ar_op, ar->ar_hln, ar->ar_pln);
}



/*
 *  arprta - print the address fields of an address resolution message
 *
 *  ar = the address resolution message
 *
 *  Print the values of the source and target hardware and protocol address
 *  fields of the specified address resolution message using two hexadecimal
 *  digits for each byte in an address.
 */

arprta(ar)
register struct addres *ar;
{
    arprint(" sha=", "%x", ar_sha(ar), ar->ar_hln);
    arprint(" spa=", "%d", ar_spa(ar), ar->ar_pln);
    arprint(" tha=", "%x", ar_tha(ar), ar->ar_hln);
    arprint(" tpa=", "%d", ar_tpa(ar), ar->ar_pln);
    printf("\r\n");
}

arprint (h, f, s, len)
register char *h, *f;
register u_char *s;
register int len;
{

    printf ("%s", h);
    while (len--) {
	printf (f, (s++)[0]);
	if (len) printf (".");
    }
}

#endif	ARDEBUG

/*
 *  pa_fmt - format a protocol address
 *
 *  a = address to print
 *  l = length of address in bytes
 *
 *  Return: a static buffer filled with the hexadecimal equivalent of the
 *  specified number of bytes of the indicated address using two hexadecimal
 *  digits to represent each byte.
 *
 *  TODO: fix to accept argument buffer rather than static (3/3/84)
 *  TODO: move this elsewhere (3/3/84)
 */

char *pa_fmt(a, l)
register u_char *a;
register int l;
{
    static char buffer[20] = {0};
    register char *bp = buffer;

    for (;l--; bp+=2)
	sprintf(bp, "%02x", (a++)[0]);
    return(buffer);
}
