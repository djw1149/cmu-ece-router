#include "param.h"
#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ip.h"
#include "udp.h"
#include "globals.h"

udpinit (up)
register struct upcb *up;
{
    bzero (up, (sizeof *up) * UDP_ARRSIZE);
}

struct upcb *udp_create (func)
int (*func)();
{
    register struct upcb *up = udp_var;
    register int i;

    for (i = 0; i < UDP_ARRSIZE; i++)
	if (up[i].active == 0) {
	    up[i].active = 1;
	    up[i].input = func;
	    return &up[i];
	}
    return 0;
}

udp_port (up, d, s)
register struct upcb *up;
u_short d;
{
    if (d)
        up->hisport = d;
    if (s)
        up->myport = s;
    else {
        register struct ipcb *ip = ip_var;

	if (ip->ports < 1024)
	    ip->ports = 1024;
        up->myport = ip->ports++;
    }
}

udp_output (up, m, len)
register struct upcb *up;
register struct mbuf *m;
{
    register struct udpiphdr *u;

    m->offset -= sizeof *u;
    m->length = sizeof *u + len;

    u = mtod(m, struct udpiphdr *);
    u->ui_next = u->ui_prev = 0;
    u->ui_x1 = 0;
    u->ui_pr = IPPROTO_UDP;
    u->ui_len = len + sizeof u->ui_u;
    bcopy (&(up->his_ipaddr), &(u->ui_dst), sizeof u->ui_dst);
    bcopy (&(ip_var->my_ipaddr), &(u->ui_src), sizeof u->ui_src);
    u->ui_dport = up->hisport;
    u->ui_sport = up->myport;
    u->ui_ulen = u->ui_len;
    u->ui_sum = 0;
    u->ui_sum = (u_short)cksum (m, sizeof *u + len);
    ip_output (ip_var, &up->his_ipaddr, m);
}

udp_input (m)
register struct mbuf *m;
{
    register struct upcb *up;
    register struct udpiphdr *u = mtod (m, struct udpiphdr *);
    register int i;


    for (i = 0, up = udp_var; i < UDP_ARRSIZE; i++, up++) {
        if ( up->active &&
	     ( (up->his_ipaddr.s_addr == 0) ||
	       (up->his_ipaddr.s_addr == u->ui_src.s_addr) ) &&
	     u->ui_dport == up->myport )
	{
	    if (up->peerport == 0)
	       up->peerport = u->ui_sport;
	    m->offset += sizeof *u;
	    m->length = u->ui_ulen - sizeof u->ui_u;
	    
	    up->count = m->length;
	    if (up->input)
	       (*up->input)(up, m);
	    return 1;
	}
    }
    return 0;
}
