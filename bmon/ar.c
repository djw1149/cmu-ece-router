#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ar.h"
#include "ip.h"
#include "device.h"
#include "globals.h"

#define bsha(p)	(p->addr)
#define bspa(p)	(p->addr + p->hln)
#define btha(p) (bspa(p) + p->pln)
#define btpa(p) (btha(p) + p->hln)
u_char bcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

char *ar_resolved (ipaddr)
struct in_addr *ipaddr;
{
    register struct arcb *ar = ar_var;
    register int i;
    for (i = 0; i < AR_ARRSIZE; i++)
	if (ar->entry[i].ipaddr.s_addr == ipaddr->s_addr)
	    return ar->entry[i].physaddr;
    return 0;
}
    
arinit (ar) 
register struct arcb *ar;
{
    bzero (ar, (sizeof *ar) + AR_ARRSIZE);
}

ar_output(dev, to, m)
register struct device *dev;
register struct mbuf *m;
struct in_addr *to;
{
    register struct ar_header *p;

    /* build ar packet */

    m->length = sizeof *p + 2*(dev->size + 4);
    m->offset = MLEN - m->length;

    p = mtod(m, struct ar_header *);
    p->hrd = dev->hrd;
    p->pro = dev->pro;
    p->hln = dev->size;
    p->pln = 4;
    p->op = AR_REQUEST;
    bcopy (dev->phys, bsha(p), p->hln);
    bcopy (&ip_var->my_ipaddr, bspa(p), p->pln);
    bcopy (to, btpa(p), p->pln);
}

ar_input (dev, m)
register struct device *dev;
register struct mbuf *m;

{
    register struct ar_header *p;

    p = mtod(m, struct ar_header *);
    if ( p->op == AR_REPLY &&
         !bcmp(&ip_var->my_ipaddr, tpa(p), p->pln) &&
         !bcmp(dev->phys, tha(p), p->hln))
    {
       register struct arcb *ar = ar_var;
       register struct arentry *e;
       char *dst;

       if (dst = ar_resolved (spa(p)))
	   bcopy (sha(p), dst, p->hln);
       else {
	   putc('A');	/* djw: added */
	   e = &ar->entry[ar->current++];
	   ar->current %= AR_ARRSIZE;
	   bcopy (spa(p), &e->ipaddr, p->pln);
	   bcopy (sha(p), e->physaddr, p->hln);
       }
    }
}
