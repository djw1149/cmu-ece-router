/*
 *  System initialization module
 *
 * 30-JUNE-86  Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University put machine dependent code into
 * 	macros to make this module machine indepent.
 *
 * Long Ago    Prior versions writen by Matt Mathis, Gregg Leboitz
 *	and/or Mike Accetta.
 *
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/malloc.h"
#include "../../h/packet.h"
#include "../../h/proc.h"
#include "../../h/proto.h"
#include "../../h/time.h"

#include "../mch/mch.h"
#include "../mch/memory.h"
#include "../mch/procu.h"

#include "../../h/globalsw.h"

char *timeup();
/*
 *  main - first routine
 *
 *  This routine is called from the startup sequence at location 0 which
 *  jumps here.  It assumes at least a minimal stack to permit the initial
 *  csav and call on Imain() below.
 *
 *  TODO:  change to call Imain() directly from startup.
 */

main()
{

    extern versmajor, versminor, versedit;
    extern char end[];
    extern char versdate[];
    extern char *splim;
    extern struct malloc tempcode;
    extern int tempsize;

    /*
     *  Set up a temporary stack - used only by main until the first
     *  context switch.   This is safe since malloc allocates memory
     *	from the far end of the chunk.
     */
    ITEMPSTK();
    Ivector();
    cprintf("\r\nCMU %s Router system V%d.%d(%d) of %s\r\n\n",
	    MACHINE, versmajor, versminor, versedit, versdate);

    Isize();
    Iproc();
    Ipalloc();
    curproc = &nullproc;
    Iconfigure();
    systat();
    alarm(HOUR, systat, 0, AL_AUTO);
    /*
     *  This call actually frees the segment which contains the code we are
     *  currently running.   This is safe because no mallocs can happen
     *  untill after the pswitch, then we never come back.
     */
    Ifree();
    pswitch();
    /* no return */
}



/*
 *  systat - print system statistics
 *
 *  This routine is called near the end of the initialization procedure and at
 *  regular intervals (currently once an hour) during system operation.  It
 *  displays the uptime, free packet count, and dynamic memory use.
 */

systat()
{
   char str[20];
   extern struct queue pfreeq;
   extern u_long maxfree;
   u_int n = mavail();

   if (functionflags & (FCNF_IPDOWN|FCNF_ARPDOWN))
      printf("[ down ");
   else
      printf("[ up ");

   printf("%s Free packets=%d, %d/%ld bytes free=%d% ]\r\n",
	  timeup(str), lengthqueue(&pfreeq), n, maxfree,
          (int)((long)n*100/(long)maxfree));
}


extern long time;
/*
 *  timeup - format a string with the current uptime
 *
 *  Fill in the supplied string with the ASCII representation of the current
 *  uptime.  The format is:  DD+HH:MM:SS  where:  DD is the number of days the
 *  system has been up, HH is the number of hours it has been up beyond this
 *  many days, MM is the number of minutes beyond this many hours and SS is the
 *  number of seconds beyond this many minutes.
 */

char *timeup(str)
char *str;
{
    int hours, mins, secs, days;
    long now;

    now = time;
    hours = now/3600;
    mins = (now/60)%60;
    secs = (now%60);
    days = hours/24;
    hours = hours%24;
    sprintf(str, "%2d+%02d:%02d:%02d", days, hours, mins, secs);
    return(str);
}


