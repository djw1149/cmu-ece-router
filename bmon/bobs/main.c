/****************************************************************************/
/*
    This version of main continuously boots, and uses protected ram to record
	statistics.  It must be used with the modified version of vmch.asm.
*/
/****************************************************************************/
#include "param.h"
#include "types.h"
#include "version.h"
#include "mbuf.h"
#include "device.h"
#include "in.h"
#include "ip.h"
#include "udp.h"
#include "tty.h"
#include "ar.h"
#include "ec.h"
#include "boot.h"
#include "break.h"
#include "autoconf.h"
#include "globals.h"
#include "led.h"
#include "s_ram.h"

struct globals *globals = (struct globals *)0x800;

init ()
{
    extern int  catchjsr[];
    extern int  strboot (), monstart (), reboot ();
    register int  **vec = 0;
    register int    i;

    for (i = 2; i < 48; i++)
	vec[i] = catchjsr;
    vec[32 + 15] = strboot;	/* assign trap #15 to strboot */
    vec[31] = monstart;		/* interuppt seven always starts monitor */
}


main (arg,s_ram_start)
{
    register struct globals *g = globals;

    struct ttcb ttcb[TT_ARRSIZE];
    struct ipcb ipcb[IP_ARRSIZE];	/* internet */
    struct arcb arcb[AR_ARRSIZE];
    struct upcb udpcb[UDP_ARRSIZE];
    struct boot bootcb[BO_ARRSIZE];	/* boot struct */

    char buf[1024];
    char input[256];
    char cmd[20];
    char *str;
    struct s_ram_element *sr;

    bzero(g, sizeof *g);
    g->tt = ttcb;
    g->ip = ipcb;
    g->udp = udpcb;
    g->ar = arcb;
    g->boot = bootcb;

    g->buf = buf;
    g->input = input;
    g->s_ram_pointer = s_ram_start;
    sr = (struct s_ram_element *) s_ram_start;

#if C_LED > 0
    ledinit();
    ledputw(0xCCCC);  /* indicate alive */
#endif C_LED
    ttinit(ttcb);
    hinit();
    
    printf("\nCMU Experimental 68000 Router boot system %d%c(%d)\n",
	    versmajor, versminor, versedit);
    printf("Protected RAM starts at 0x%x.\n",s_ram_start);
    g->net = configure();
    arinit(arcb);
    udpinit(udpcb);
    ipinit(ipcb);
    s_ram_init(s_ram_start);

  for(;;)
    if ((arg == 1) && rom_var) {
	romcopy((u_short *)(AUTOROM+rom_var->rom_boot),(u_char *)mainstr,256);
        if (boot(mainstr) == 0)
	    sr->fail++;
	else
	    sr->success++;
	sr->total++;
	sr->cksm = -(unsigned int)(sr->total + sr->success + sr->fail);
	printf("\nBoot statistics:");
	printf("\n    %d successful, %d failed, %d total.\n",
		sr->success,sr->fail,sr->total);
    }
    else
	if (arg == 0)
	  for (;;) {
	    str = input;
	    printf (">>> ");
	    gets(str);
	    scan_str (&str, cmd);
	    switch (*cmd) {
		case 't':
	            trap15("[128.2.251.4]/usr/router/boot/ombyte.beta");
		    break;
		case 'a':
	            restart();
		    break;
		case 'b':
		    boot (0);
		    break;
		case 's':
		    (* (int * ())(g->boot->entry))();
		    break;
		case 'r':
		    resolve (&net_var->cf_dev, &str, mainbuf);
		    break;
		case 'h':
	            printf (" choose one of the following: \n");
		    printf ("     autoboot    boot    help    resolve    start test_boot\n");
		    break;
		case 0:
	            break;
		case 'e':
	            if (strcmp (cmd, "exit") == 0)
		       putc ('\n'), putc(0xff), exit(0);
		default:
	    	    printf ("huh?\n");
		    break;
	    }
	  } else {
	    if (boot(arg))
	       (* (int * ())(g->boot->entry))();
	    printf("boot failed\n");
	    reboot();
	  }
}

trap()
{
    printf ("\n\n\007*TRAP ERROR*\n");
#if C_LED > 0
    lederr(LED_TRAP);
#endif C_LED
    restart ();
}

panic (str)
char *str;
{
    printf ("panic %s\n", str);
#if C_LED > 0
    lederr(LED_PANIC);
#endif C_LED
}

resolve (dev, str, m)
register struct device *dev;
register char **str;
register struct mbuf *m;
{
    register int i;
    struct in_addr to;
    char *dst;
    int timeout;

    if (scan_ip (str, &to))
	for (i = 0; i < 5; i++) {
	    timeout = TIMEOUT;
	    (*net_var->cf_xmt)(net_var, m, &to, 1);
	    while (net_poll (&timeout, m)) {
		if (dst = (char *)ar_resolved(&to)) {
		    printf ("\t");
		    ipprint (&to);
		    printf (" ==> ");
		    enprint (dst);
		    printf ("\n");
		    return;
		}
	    }
	}
    else printf ("resolve: bad ip address format\n");
}

enprint (b)
u_char *b;
{
    register int i;
    putc ('[');
    for (i = 0; i < 5; i++)
        printf ("%x.", b[i]);
    printf ("%x]", b[i]);
}
