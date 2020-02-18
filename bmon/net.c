#include "param.h"
#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ip.h"
#include "ec.h"
#include "udp.h"
#include "globals.h"
#include "device.h"
#include "autoconf.h"

struct upcb *net_open(addr, func)
struct in_addr *addr;
int (*func)();
{
    extern struct upcb *udp_create();
    register struct upcb *up = udp_create(func);

    if (up)
       up->his_ipaddr = *addr;
    return up;
}

int net_close(up)
register struct upcb *up;
{
    bzero (up, sizeof *up);
}

int net_read (up)
struct upcb *up;
{
    int i = up->count;
    up->count = 0;
    return i;
}

int net_write (up, m, len)
struct upcb *up;
struct mbuf *m;
int len;
{
    extern int udp_output();
    return udp_output (up, m, len);
}

int net_port (up, d, s)
struct upcb *up;
{
    extern int udp_port();
    return udp_port (up, d, s);
}

int net_poll(timeout, m)
register int *timeout;
register struct mbuf *m;
{
    if ((*net_var->cf_poll)(net_var, timeout)) {
       (*net_var->cf_rcv)(net_var, m);
       return 1;
    }
    return 0;
}    
