/*
 * $Header: Dil.c,v 1.4 86/07/28 07:15:41 chriss Exp $
 *
 * file : Dil.c
 *
 * synopsis:
 *	 router interlan 10MB ethernet board diagnosis module
 *
 * modification history:
 *
 *   Jul-86 Chriss Stephens (chriss) at Carnegie Mellon University modified
 *	to use new router organization (i.e. modified includes). Modified
 *	to return error code ERROR_EXIT and set perror variable in the
 *	event that diagnosis type in returned packet is not defined.
 *
 * david waitzman (djw@ampere.ece.cmu.edu) march '86
 *	created
 *
 *
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include "./h/rtypes.h"
#include <netinet/in.h>
#include "../h/rcp.h"
#include "./h/rcpinfo.h"
#include "./h/perror.h"			/* define error constants */

extern int verbose; 	/* from rdiag.c */
extern int errno;	/* from system  */

/*
 * execute the appropriate diagnosis routine
 */

ildiag_main(diag)
  
  struct rcp_diagfo *diag;

{
    switch (ntohs(diag->subtype)) 
    {
	case ILDIAG_STATS: 
	    ildiag_stats((u_short *)diag->data);
	    break;
	case ILDIAG_TDR: 
	    ildiag_tdr((struct tdrcmd *)diag->data);
	    break;
	case ILDIAG_DIAG: 	/* could make use diagnosis failed */
	    ildiag_diag((int *)diag->data);
	    break;
	case ILDIAG_DUMP: 
	    ildiag_dump((u_char *)diag->data);
	    break;
	default: errno = BAD_DIAG;
	         return(ERROR_EXIT);
    };
}

/*
 * print out the four error counters the 82586 keeps. First entry in data
 * field is the devtype so skip it.
 */
ildiag_stats(data)
u_short data[5];
{
printf(verbose ?
  "Errors: cyclic redundancy check %d, alignment %d, resource %d, overrun %d\n"
  : "crc %d, aln %d, rsc %d, ovr %d\n",
  ntohs(data[1]), ntohs(data[2]), ntohs(data[3]), ntohs(data[4]));
}

/*
 * show results from a time domain reflectometry command
 */
struct tdrcmd {	/* from ../h/ilreg.h */
    u_short time:11,	/* distance in transmit clock cycles */
            dummy:1,
	    et_srt:1,	/* short on the link (valid only ...) */
	    et_opn:1,	/* open on the link id'ed */
    	    xcvr_prb:1,	/* cable problem id'ed (valid only if transceiver
	    		   returns carrier sense during transmission */
	    lnk_ok:1;	/* link ok */
};

#define OK(val) ((val) == 0? "false" : "true")

ildiag_tdr(tdr)
struct tdrcmd *tdr;
{

    u_short *temp = (u_short *)++tdr;		/* skip over devtype */

    *temp = ntohs(*temp);
    tdr = (struct tdrcmd *)temp;
    printf("TDR- lnk_ok: %s, xcvr-prb: %s, et-opn: %s, et-srt: %s, time: %d\n",
            OK(tdr->lnk_ok),OK(tdr->xcvr_prb),OK(tdr->et_opn),OK(tdr->et_srt),
	    tdr->time);
}

/*
 * show results from a self diagnose command
 */
ildiag_diag(data)
register int *data;
{
    data += sizeof(u_short);		/* skip over devtype field */

    if (*data & 0x2000) printf("passed self test\n");
    if (*data & 0x0400) printf("failed self test\n");
}

/*
 * Display 165 or so bytes of dump information- boring.  Really should
 * only print out useful information
 */
#define ILDUMPSIZE	170
#define COL 0x10		/* number of columns to show */

ildiag_dump(data)
u_char *data;
{
    int i, j;
    bntohs((u_short *)data, ILDUMPSIZE);

    if (verbose) printf("Dump returned:\n");

    /* print heading */
    printf("    ");
    for(j = 0; j < COL; j++)
    	printf("%2x ", j);
    putchar('\n');

    /* print data */
    for(i = 0; i < ILDUMPSIZE; i+= COL) {
	printf("%2x: ", i);
	for(j = i; (j < 0xa5) && (j < i + COL); j++)
	    printf("%2x ", data[j]);
	putchar('\n');
    }
    putchar('\n');
}

