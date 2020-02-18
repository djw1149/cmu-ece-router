/*
 * $Header: main.c,v 1.109 86/10/17 17:23:21 djw Exp $
 */ 
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
#include "il.h"
#include "ec.h"
#include "boot.h"
#include "break.h"
#include "autoconf.h"
#include "bootp.h"
#include "globals.h"
#include "led.h"

/* possible versabug conflicts with the following structure: */
struct globals *globals = (struct globals *)0x800;

init()
{
    extern int catchjsr[];
    extern int strboot(), monstart(), reboot();
    register int **vec = 0;
    register int i;

    for (i = 2; i < 48; i++)
        vec[i] = catchjsr;
    vec[32 + 15] = strboot;  /* assign trap #15 to strboot */
    vec[31] = monstart;      /* interrupt seven always starts monitor */
}


main(arg, s_ram_start)
{
    register struct globals *g = globals;

    /* allocate stack based memory that the globals structure can use */
    struct bootp bpcb;			/* bootp reply packet */
    struct ttcb ttcb[TT_ARRSIZE];
    struct ipcb ipcb[IP_ARRSIZE];	/* internet */
    struct arcb arcb[AR_ARRSIZE];
    struct upcb udpcb[UDP_ARRSIZE];
    struct boot bootcb[BO_ARRSIZE];	/* boot struct */
    u_char the_heap[HEAPSIZE];		/* heap space */
    struct config configcb;		/* to store the config struct */

    char buf[MLEN];	/* an mbuf for holding packets    djw: was 1024 */
    char input[256];
    char cmd[20];
    char *str;
    int canauto;	/* flag as to whether configure says to autoboot */

    bzero(g, sizeof *g);
    g->bp = &bpcb;
    g->tt = ttcb;
    g->ip = ipcb;
    g->udp = udpcb;
    g->ar = arcb;
    g->boot = bootcb;
    g->net = &configcb;
    g->buf = buf;
    g->input = input;

#if C_LED > 0
    ledinit();					/* init the leds, if any */
    ledputw(0xCCCC);				/* indicate alive */
#endif C_LED
    ttinit(ttcb);				/* init the terminal */
    hinit(the_heap);				/* init the heap */

    printf("\nCMU 68000 Router boot system %d%c(%d)\n",
	    versmajor, versminor, versedit);
    printf("Protected RAM starts at 0x%x.\n",s_ram_start);
    canauto = configure();			/* find the conf-rom */
    showinterfaces();				/* print out the interfaces */
    if (!devconfigure(0))			/* configure some device */
	/*
	 * we could not find a working interface, so we will query the user
	 * for one to try (maybe he can fix something while we are on)
	 */
        select_interface();
    arinit(arcb);
    udpinit(udpcb);
    ipinit(ipcb);

    /*
     * if there is a conf-rom then we can auto boot.  We used to check rom_var
     * here but it is now always set in configure.
     */
    if ((arg == 1))
	if (canauto) {
	    autoboot(NULL);	/* loop through the possible ways to boot */
	    (* (int * ())(g->boot->entry))();	 /* start the booted file */
	}
	else 
	    panic ("autoboot: Must have a configure rom");
    else 
	if (arg == 0) {
	  print_text();
	  for (;;) {
	    str = input;
	    printf(">bmon> ");
	    gets(str);
	    scan_str(&str, cmd);
	    switch (*cmd) {
		case 'a':
	            reboot();	/* djw 091386: was restart() */
		    break;
		case 'b':
		    boot(0);
		    break;
		case 's':
		    (* (int * ())(g->boot->entry))();
		    break;
		case 'r':
		    resolve(&str, mainbuf);
		    break;
		case 'h':
		case '?':
	            printf(" choose one of the following: \n");
		    printf("     autoboot   boot   help   interface   resolve   start   ?\n");
		    break;
		case 0:
	            break;
		case 'i':
		    select_interface();
		    break;
		case 'e':
	            if (strcmp(cmd, "exit") == 0)
		       putc('\n'), putc(0xff), exit(0);
		    /* fall into: */
		default:
	    	    printf("huh?\n");
		    break;
	    } /* switch */
	  } /* for ever */
	} else { /* if !arg */
	    autoboot(arg);
	    (* (int * ())(g->boot->entry))();
	}
}

trap()
{
    printf("\n\n\007*TRAP ERROR*\n");
#if C_LED > 0
    lederr(LED_TRAP);
#endif C_LED
    restart();
}

panic(str)
char *str;
{
    printf("panic %s\n", str);
#if C_LED > 0
    lederr(LED_PANIC);
#endif C_LED
    restart();	/* djw 091386: added */
}

resolve(str, m)
 register char **str;
 register struct mbuf *m;
{
    register int i;
    struct in_addr to;
    char *dst;
    int timeout;

    if (scan_ip(str, &to))
	for (i = 0; i < 5; i++) {
	    timeout = TIMEOUT;
	    (*net_var->cf_xmt)(net_var, m, &to, 1);
	    while (net_poll(&timeout, m)) {
		if (dst = (char *)ar_resolved(&to)) {
		    printf("\t");
		    ipprint(&to);
		    printf(" ==> ");
		    enprint(dst);
		    printf("\n");
		    return;
		}
	    }
	}
    else {
	printf("resolve: bad ip address format\n");
	return;
    }
    printf("Not resolved\n");
}

enprint(b)
u_char *b;
{
    register int i;
    putc('[');
    for (i = 0; i < 5; i++)
        printf("%x.", b[i]);
    printf("%x]", b[i]);
}

/*
 * autoboot() has to rotate through the possible ways to boot
 */
autoboot(bs)
char *bs;	/* boot string or NULL to try all in rom */
{
    register struct autoid **aiargv;
    register int i;			/* current boot string index */
    int	realauto = (int)bs;		/* 1 if using an explicit boot str */

retry:
    for (aiargv = rom_var->autoids; *aiargv; aiargv++) /* for each interface*/
    	if (devconfigure(*aiargv))		  /* configure the interface*/
	    if (realauto) {
		if (boot(bs)) return;
	    } else
		for (i = 0; rom_var->boot[i] ; i++) {   /* for each boot str*/
	            strcpy(mainstr, rom_var->boot[i]);  /* make a safe copy */
	        if (boot(mainstr)) return;	  /* try to load a boot file*/
	    }

    /* retry with the rom boot strings if the passed boot string failed */
    if (realauto) {
	realauto = NULL;	/* set up trying the rom boot strings */
	goto retry;
    }

    panic("autoboot: couldn't ever boot"); 
    /* never makes it past the panic */
    reboot();
}

/*
 * Show all of the interfaces in the system
 */
showinterfaces()
{
    register struct autoid **aiargv;
    register int i = 1;
    
    printf("Available devices: ");
    for (aiargv = rom_var->autoids; *aiargv; aiargv++) {
        printf("[%x]-", i++);	     /* the numbers are for select_interface */
	printdev(&((*aiargv)->id));
    }
    putc('\n');
}

/*
 * loop until the user selects an interface that properly configures
 */
select_interface()
{
    char *str;
    int   inter;

    while (1) {
	str = mainstr;
	printf("Enter # of desired interface to use: ");
	gets(str);
	scan_sp(&str);
	inter = scan_int(&str, 16);
	if ((inter < 1) || (inter > rom_var->num_dev))
	    printf("bad interface\n");
	else if (devconfigure(rom_var->autoids[inter - 1])) {
	    printf("Configured ok\n");
	    return;
	} else
	    printf("Didn't configure\n");
        showinterfaces();
    }
}
