/*++*
 * Submodule: ip.c
 * Abstract:  This module contains the general ip_input, ip_output
 *	      and ip_routing routines.
 *
 * Created:	Sat May 21 15:00:51 1983
 * By:		Gregg Lebovitz at CMU-EE-AMPERE
 *
 *--*/

/*++*
 * History
 *
 **++
 ** Version:	1A(1)
 ** By:		Gregg Lebovitz at CMU-EE-AMPERE
 ** Date:	Sat May 21 15:00:52 1983
 **
 **	File Created.
 **
 **--
 *
 *--*/
 
#include "param.h"
#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ip.h"
#include "ec.h"
#include "icmp.h"
#include "globals.h"
#include "device.h"
#include "autoconf.h"

ipinit (ip)
register struct ipcb *ip;
{
    unsigned ipaddr;
    u_char *ipa = &ip->my_ipaddr.s_addr;

    bzero (ip, sizeof *ip);
    bcopy(getipaddr(), &ipaddr, sizeof (ipaddr));
    ip->my_ipaddr.s_addr = ipaddr;
    ip->ports = 1024;
    printf("ip address is %d.%d.%d.%d\n", ipa[0], ipa[1], ipa[2], ipa[3]);
};

ip_input (ip, m)
register struct ipcb *ip;
register struct mbuf *m;
{
    register struct ip *inp = mtod (m, struct ip *);
    register u_short sum, n;

    if (inp->ip_v == 4)
       if (inp->ip_ttl) {
          sum = inp->ip_sum;
	  inp->ip_sum = 0;
	  if ((n = cksum(m, 20)) == sum) {
             if ( (inp->ip_dst.s_addr == ip->my_ipaddr.s_addr) &&
	          (inp->ip_p == IPPROTO_UDP) )
	        return udp_input (m);
	  }
	  else printf ("ip cksum error rcv=%x cksum=%x\n", sum, n);
       }
     return 0;
}



ip_output (ip, to, m)
register struct ipcb *ip;
register struct in_addr *to;
struct mbuf *m;
{
    register struct ip *inp;

    inp = mtod (m, struct ip *);
    inp->ip_v = 4;
    inp->ip_hl = 5;
    inp->ip_tos = 0;
    inp->ip_len = m->length;
    inp->ip_id = 0;
    inp->ip_off = 0;
    inp->ip_ttl = 103;
    inp->ip_sum = 0;
    inp->ip_sum = cksum(m, 20);
    (*net_var->cf_xmt)(net_var, m, to, 0);
    return 1;
}
 
ipprint (b)
u_char *b;
{
    register int i;
    putc ('[');
    for (i = 0; i < 3; i++)
        printf ("%d.", b[i]);
    printf ("%d]", b[i]);
}
