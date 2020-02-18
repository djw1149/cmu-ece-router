/*
 * $Header: rdiag.c,v 1.4 86/11/05 10:42:48 djw Exp $
 *
 * file: diag.c
 *
 * synopsis: router diagnosis simple user interface module
 *	     this module uses rd_utils.c, D*.c (only Dil.c now), showdevcnt.c,
 *	     and rcp.c to implement a simple user interface for remote router
 *	     diagnosis.  See the man page for details on usage.
 *
 * modification history:
 *
 * Wed Nov  5 1986 david waitzman (djw) at cmu
 *	fixed printing format problem- now prints card numbers on a nonverbose
 *	diagnosis.
 *
 * Jul-86 Chriss Stephens (chriss) at Carnegie Mellon University modified to
 *	use customized perror (see rcp.c) routine for consistent handling of
 *	error conditions.
 *
 * david waitzman (djw@ampere.ece.cmu.edu) march '86
 *	created.  much help from rcp code by Matt Mathis, Kevin Krimse
 *	and Mike Acceta.
 *
 */

#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include "./h/rtypes.h"
#include <netinet/in.h>

#include "../h/queue.h"
#include "./h/mch.h"		/* define byte order for target machine */
#include "../h/ip.h"
#include "../h/rcp.h"
#include "../h/errorlog.h"
#include "./h/rcpinfo.h"
#include "./h/perror.h"			/* define error constants */
#include "./h/rdiag.h"
	

extern int errno;
extern int perror();
extern char *strcpy();
extern struct rcp_diagfo *rdiag();

int verbose;		/* be verbose flag */
char xpswd[20];
char *pswd = xpswd;	/* the password for authorizing diagnoses */

/*
 * flagp returns 1 if the given flag character occurs in the options list
 * at the end of the input to rdiag, else it returns 0.
 */
int flagp(argc, argv, flag)
int argc;
char *argv[];
char flag;
{
    char *flaglist;

    if (argc != 5) return 0;
    flaglist = argv[4];
    if (*flaglist++ != '-') return 0;
    while (*flaglist)
        if (*flaglist++ == flag) return 1;
    return(SUCESSFULL);
}

main(argc, argv)
int argc;
char *argv[];
{
    struct sockaddr_in sin;
    int card, diagtype;
    int devtype1 = 0, devtype2 = 0;
    short needsauth;

    if ((argc < 4) || (argc > 5)) {
       fprintf(stderr, "usage %s ?.?.?.? card diag_type [-vp]\n",argv[0]);
       return(0);
    }

    if (set_rcp_sin(argv[1], &sin))
    {
	errno = NO_SOCKET;
	print_error("rdiag",FATAL);
    }

    if ((card = devname_to_num(argv[2], &devtype1)) < 0)
    {
	fprintf (stderr,"rdiag: unable to parse card number\n");
	return(0);
    }

    if ((diagtype = diagname_to_num(argv[3], &devtype2, &needsauth)) == 0)
    {
	fprintf (stderr,"rdiag: unable to parse diagnosis type\n");
	return(0);
    }

    /* check if should get a password, and do so if needed, else make the
       password null so we don't try to authorize with the null password */
    if ((needsauth == AY) || flagp(argc, argv, 'p')) {
	char *temppswd = (char *)getpass("Enter password: ");
	strcpy(pswd, temppswd);
    } else pswd = NULL;

    /* make sure the devtypes are consistent, if two device types are given */
    if (devtype1 && devtype2 && (devtype1 != devtype2))
    {
	fprintf (stderr,"rdiag: device types mismatch\n");
	return(0);
    }

    /* if one of the devtypes is set, make sure devtype2 is set, we already
       know already that if both are set, they are equal */
    if (devtype1)
        devtype2 = devtype1;

    /* set verbose true if have -v at end of input */
    verbose = flagp(argc, argv, 'v');

    
    return(urdiag(&sin, argv[1], card, diagtype, devtype2));
}

/*
 * process the results from a diagnosis. Now compatible with version 1
 * of software.
 */

process_diag(diag, rname, diagtype)

  struct rcp_diagfo *diag;		/* the returned diagnosis packet */
         char       *rname;		/* router name */
         int         diagtype;		/* diagnosis type */

{
    u_short  devtype = *(u_short *)diag->data;

    if (verbose) {
        printf("%s:: Card: %d, Diagnosis: %s, Device type: ",
		rname, ntohs(diag->u_di.card),
		diagnum_to_name(diagtype, ntohs(devtype)));
        devswt(ntohs(devtype));	/* show the device type */
        putchar('\n');
    } else
        printf("%d: ", ntohs(diag->u_di.card));	/* djw: Wed Nov  5 1986 */

    if (ntohs(diag->status) != RCED_NOERR) {
	fprintf(stderr,"rdiag: diagnosis error ");
	put_diag_status(diag->status);
	putchar('\n');
	return(0);
    }

    /* add cases here to diagnose more device types */
    switch (ntohs(devtype)) {
	case DVT_IL10: ildiag_main(diag);
		       break;
	default: fprintf (stderr,
			  "rdiag: panic! unhandled returned device type\n");
		 exit(0);
    }
    return(1);
}

/*
 * The main diagnosis control code.  loops for multiple device diagnosis.
 * calls process_diag to show results of diagnosis.
 */
urdiag(sin, rname, card, diagtype, devtype)
struct sockaddr_in *sin;		/* Socket to the router */
char *rname;				/* router name */
int  card;				/* which router interface */
int  diagtype;				/* diagnosis type */
int  devtype;				/* device type */
{
    struct rcp_diagfo *diag;
	   u_short    *rc_devtype = (u_short *)diag->data;
	   
    /* card is non-zero if not a multiple diagnosis */
    if (card)
    {
      if ((diag = rdiag(sin, card, diagtype, devtype, pswd)) == NULL) 
      {
        if (verbose)
	  printf("%s:: Card: %d, Diagnosis: %s\n", rname,
     	          card,
		  diagnum_to_name(diagtype, ntohs(*rc_devtype)));
        print_error ("rdiag",FATAL);
      }
      return (process_diag(diag, rname, diagtype));
    }

    /* a multiple diagnosis: */
    for (card = 1; card < MAXPOSSIBLECARD; card++)
    {
      if ((diag = rdiag (sin, card, diagtype, devtype, pswd)) == NULL)
      {
        if (verbose)
	{
	  printf("%s:: Card: %d, Diagnosis: %s\n", rname,
		  card,
		  diagnum_to_name(diagtype, ntohs(*rc_devtype)));
        }
	print_error("rdiag",FATAL);
      }
      else
      {
        /* check if done with the multiple diagnosis, must do this before
	   process_diag or the user is bothered with an error message */
        if (ntohs(diag->status) == RCED_NOCARD)
            return(1);

        if (ntohs(diag->status) == RCED_BADTYPE)
	    continue;

	process_diag(diag, rname, diagtype);
      }

	/* determine next state */
	switch(ntohs(diag->status))
	{
	    case RCED_NOERR:
	    case RCED_BADTYPE:
	    case RCED_DFAIL: break;
	    default:
		if (verbose)
		  fprintf (stderr,
			   "rdiag: can't continue multiple diagnosis\n");
		return(0);
	}
    }
}

/* prints the decoded diagnosis status field.  status must be in net format */

put_diag_status(status)
  
  short status;

{
    switch(ntohs(status)) {
	case RCED_NOERR: printf("no error");
			 break;
	case RCED_UNSUP: printf("unsupported diagnosis for given device");
			 break;
	case RCED_DFAIL: printf("diagnosis failed");
			 break;
	case RCED_BADCARD: printf("that card doesn't have any diagnostics");
			 break;
	case RCED_NOCARD: printf("no card of that number exists");
			 break;
	case RCED_BADTYPE: printf("given device type != actual device type");
			 break;
	case RCED_NOAUTH: printf("no authorization given for diagnosis");
			 break;
	default:	 printf("unknown status:%d", ntohs(status));
    }
}
