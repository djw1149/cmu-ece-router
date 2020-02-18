/*
 *   file: errorlog.c
 *
 *   synopsis: server side routines to add entries to errorlog onboard
 *	     router
 *
 *   modification history:
 *
 *	Aug-86 Chriss Stephens (chriss) at Carngie Mellon University removed
 *	  version zero rcp support.
 *
 *	Jun-86 modified by Chriss Stephens (chriss) at Carnegie Mellon 
 *	  University to use rc_append, common diagfo header, and version 1
 *	  of software. The function call to add entries to the log is now
 *	  known as V1_rc_errorlog.
 *
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"

#include "../../h/ip.h"
#include "../../h/udp.h"
#include "../../h/errorlog.h"
#include "../../h/rcp.h"
#include "../../h/globalsw.h"

#define NULL 0

static struct errorlist errorlist={0};

Ierrorlog()
{}		/* future, to make the errorlog ride through reboots */

errorlog(p,etype)
struct packet *p;
int etype;
{
    int br=spl7();		/* prevent chaos */

    extern long time;
    if ((debugflags&TRACE_ER) || (p->p_flag & P_TRACE))
    	cprintf("Error:%o len=%d\r\n",etype,p->p_len);

    errorlist.errors[errorlist.el_current].e_type = htons(etype);
    errorlist.errors[errorlist.el_current].e_time = htonl((long) time);

#ifdef PCHECK
    errorlist.errors[errorlist.el_current].p_maxlen = htons(p -> p_maxlen);
#else
    errorlist.errors[errorlist.el_current].p_maxlen = 0;
#endif PCHECK

    errorlist.errors[errorlist.el_current].p_len = htons(p -> p_len);
    errorlist.errors[errorlist.el_current].p_off = htons(p -> p_off);
    errorlist.errors[errorlist.el_current].p_flag = htons(p -> p_flag);
    errorlist.errors[errorlist.el_current].e_seq = htonl(errorlist.el_seq++);
    bcopy (p -> p_data, errorlist.errors[errorlist.el_current].e_header, ERRORCOPY);
    errorlist.el_current = (errorlist.el_current + 1) % ELISTLEN;
    spl(br);
}

/* V1_rc_errorlog

	This function is compatible with the diagfo header and uses the
	rc_append function to add elements to the end of a packet. I.e.
	to add stuff to the packet's data section.
 */

V1_rc_errorlog(dv,p,rdi)

  struct device     *dv;
  struct packet     *p;
  struct rcp_diagfo *rdi;

{
    struct errorlist *el;

    rdi->length = 0;

    el = (struct errorlist *) 
         rc_append(rdi,sizeof(struct errorlist),&(p->p_len));
    bcopy(&errorlist,el,sizeof(struct errorlist));
    el->el_current = htonl(el->el_current);
    el->el_seq = htonl(el->el_seq);

}
