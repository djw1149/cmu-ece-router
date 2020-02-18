/*
 * patch - provides special load time patches to configure rom information
 * $Header: patch.c,v 1.2 86/10/18 01:22:24 djw Exp $
 *****
 * HISTORY
 * Fri Sep 19 15:53:59 EDT 1986 David Waitzman (djw) at CMU
 *	Created, based on bmon/autoconf.c
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
#include "../mch/patch.h"

/*
 * Returns a 1 for the given rom and minor number if patching is needed,
 * else returns 0
 */
int needspatched(rom, minor_num)
register p_autorom rom;
u_long minor_num;
{
    register struct patch_table_t *pt = patch_table;

/*  cprintf("patch? s%d, m%d\r\n", rom->serial_number, minor_num);  */
    for ( ; pt->serial_num != -1; pt++)
        if ((pt->serial_num == rom->serial_number) &&
  	    (pt->minor_num  == minor_num)) {
/*  cprintf("should patch s%d, m%d\r\n", pt->serial_num, minor_num);  */
	    return(1);
	    }
/*  else cprintf("NO p.s%d == s%d, p.m%d == m%d\r\n",
		pt->serial_num, rom->serial_number, pt->minor_num, minor_num);
 */

    return 0;
}

/*
 * Returns a new pi structure to use for the given rom and minor number
 */
p_char patchpi(rom, minor_num)
register p_autorom rom;
u_long minor_num;
{
    register struct patch_table_t *pt = patch_table;

    for ( ; pt->serial_num != -1; pt++)
        if ((pt->serial_num == rom->serial_number) &&
  	    (pt->minor_num  == minor_num)) {
/* cprintf("patching s%d, m%d\r\n", pt->serial_num, minor_num);  */
/* cprintf("First PI = %d\n", *(u_char *)pt->new_pi); */
	   return(pt->new_pi);
	    }
/*     panic("Patch requested, but non-available\r\n");     */
}
