
#include "../../h/rtypes.h" 
#include "../../h/queue.h"
#include "../../h/arp.h" 
#include "../../h/globalsw.h" 


unsigned long functionflags;
unsigned long debugflags;

/*
 * ttysetswitch:  set/clear/toggle switches in response to kbd input
 *	called from tty.c
 *	return 0 if character does not match an assigned switch
 */
ttysetswitch(ttc)
char ttc;
{
    static tty_sflag=0;		/* Shutdown requires two key strokes */

    switch (ttc) {
	case 'I': 
	    tty_sflag = 1;
	    printf("Shutdown armed\r\n");
	    break;
	case 'S': 		/* Shutdown */
	    if (tty_sflag){
		functionflags |= FCNF_ARPDOWN | FCNF_IPDOWN;
		printf("Shuting down: R to resume\r\n");
		systat();
	    }
	    break;
	case 'E':
	    if (tty_sflag){
		toggle("IP shutdown", FCNF_IPDOWN, &functionflags);
		systat();		/* Warning that we are really down */
	    }
	    break;
	case 'R': 		/* Both up */
	case 'r':
	    functionflags &= ~(FCNF_ARPDOWN | FCNF_IPDOWN);
	    systat();
	    break;
	case 'A': 
	    toggle ("AppleBus trace", TRACE_AB,&debugflags);
	    break;
	case 'e': 
	    toggle ("error trace", TRACE_ER,&debugflags);
	    break;
	case 'd': 
	    toggle ("device trace", TRACE_IL,&debugflags);
	    break;
	case 'a': 
	    toggle ("ARP trace", TRACE_AR,&debugflags);
	    break;
	case 'i': 
	    toggle ("IP trace", TRACE_IP,&debugflags);
	    break;
	case '?':
	    printf("Trace: A:Applebus, e:Errors, d:Device, a:ARP, i:IP\r\n");
	    printf("Function: I:arm, S:shutdown, E:Toggle IP down, r&R:Up\r\n");
	    break;
	default:
	    tty_sflag = 0;		/* nolonger prefixed */
	    return(0);
    }
    if (ttc != 'I') tty_sflag = 0;	/* nolonger prefixed */
    return (1);
}

/* Announce and toggle the selected bits */
toggle (s, flag, flags)
char *s;
int *flags;
{
    if (*flags & flag) {
       printf ("[%s OFF]\r\n", s);
       *flags &= ~flag;
    }
    else {
       printf ("[%s ON]\r\n", s);
       *flags |= flag;
    }
}


