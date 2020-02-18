/* 
 * $Header: rcp.c,v 1.9 86/10/10 09:46:21 chriss Exp $
 *
 * file: rcp.c 	*** client side ***
 *
 * Router Control Protocol module
 *	Supports the basic remote interface to the routers
 *
 * Modification history
 *
 *    Oct-86 modified by Chriss Stephens (chriss) at Carnegie Mellon - added
 *	print_error which calls our version of perror.
 *
 *    Jun-86 Modified by Chriss Stephens (chriss) at Carnegie Mellon 
 *      University. This version is only compatible with version 1 of the
 *	server side router software. It uses the common diagfo header for
 *	both RC_INFO and RC_DIAG.
 *
 *	Additionally, sockets to router are opened one level above this
 *	so that all these routines require a socket pointer instead of a
 *	router name.
 *
 *	Added random packet sequence number generation. See function rand_id.
 *	Added customized perror for user error messages as well as UNIX
 *	system errors.
 *
 * 28-Mar-86 by David Waitzman: now can use names instead of addresses
 *
 * 10/3/85 by Matt Mathis
 *
 * Created 8/85 by Kevin Kirmse
 */

#include <stdio.h>
#include <sys/types.h>
#include "./h/rtypes.h"
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>			/* structures for gettimeofday      */

#include "../h/rcp.h"
#include "../h/globalsw.h"
#define RCPCOM				/* indicates we are compiling rcp.c */
#include "./h/perror.h"			/* define error constants */
#include "./debug/rcp.h"

#define RCP_SERV_PORT 0xff
#define RETRIES 10			/* Ten attempts if needed */
#define MBUFSIZ 1500

       int rcp_socket = -1;
static char main_rbuff[MBUFSIZ];	/* receive buffer */
static char main_xbuff[MBUFSIZ];	/* transmit buffer*/
extern int errno;
extern char * strcpy();

/* Null procedure for signals */
rcp_fail(){};

/*****************************************************************
 * set up the rcp socket for the given router
 *
 */
set_rcp_sin(rname,sin)
char *rname;			/* name or IP address of the router */
struct sockaddr_in *sin;	/* the resultant socket */
{				/* returns -1 for failure */
#ifndef NUMONLY
    struct hostent *hp;
#endif	    

    bzero((char *)sin, sizeof(struct sockaddr_in));
    if (rcp_socket < 0) {
       if ((rcp_socket = socket(AF_INET,SOCK_DGRAM,0)) < 0)
          return (ERROR_EXIT);
    }

    if ( (int) (sin->sin_addr.s_addr = inet_addr(rname)) == ERROR_EXIT)
    {	
#ifndef NUMONLY
        if (!(hp = gethostbyname(rname)))
        {
#endif	    
            errno = BAD_IPADDR;
            return(ERROR_EXIT);
#ifndef NUMONLY
        }
	else
    	    bcopy(hp->h_addr, &(sin->sin_addr.s_addr), hp->h_length);
#endif	    
    } 
    sin->sin_family = AF_INET;
    sin->sin_port = ntohs(RCP_SERV_PORT);

#ifdef SINDEBUG
    printf("\nRCP connection \n\n");
    printf("Communications socket : %d\n",rcp_socket);
    printf("rcp/udp port : %d\n",sin->sin_port);
    printf("sin_family : %d\n",sin->sin_family);
    printf("IP Address : [%8x] ",ntohl((long)sin->sin_addr.s_addr));
    printf("\n");
#endif SINDEBUG
    return(SUCESSFULL);
}

/*
 *  rand_id - generates a random number to be used as packet id, i.e.
 *	      sequence number, to uniquely identify every packet 
 *	      transmitted between the tools host and the router.
 *
 *	      The sequence number is chosen simply by using the low byte
 *	      of the process id number and the low byte of the value returned
 *	      by gettimeofday to initialize the random number generator built
 *	      into UNIX.
 *
 */

u_short rand_id()

{
    struct timeval  tv;
    struct timezone tzp;
	   char	    highbyte,		/* bytes rand number made from */
		    lowbyte;
    static int	    flag;

int temp;

  if (!flag)				/* first time function called? */
  {
    gettimeofday(&tv,&tzp);
    lowbyte = (char)tv.tv_usec;
    highbyte = (char)getpid();
    srandom((highbyte<<8)|lowbyte);
    flag = TRUE;
  }
  return(random());
}

/*
 * cleanup - resets the ITIMER_REAL timer and resets the vector originally
 *	     associated with the SIGALRM signal. The sole purpose of this
 *	     routine is to provide a convient method of restoring the 
 *	     state of the interrupt system to the way it was before rcptrans
 *	     was called. Since rcptrans can exit from any of four different
 *	     points, this routine makes it a litter easier to read.
 */

cleanup (ovec,oldtimer)

  struct sigvec    *ovec;		/* old vector to SIGALRM handler */
  struct itimerval *oldtimer;		/* old timer to be rest		 */

{

  oldtimer->it_value.tv_sec = 0;	/* turn off interrupt timer      */
  oldtimer->it_value.tv_usec = 0;
  setitimer(ITIMER_REAL,oldtimer,NULL);
  sigvec (SIGALRM,ovec,(struct sigvec *)NULL);
}

/*****************************************************************
 * rcptrans - send a command to a router
 *
 *	This routine sends packet to router and waits for return packet.
 *	The return packet is passed back in the pointer rc. This routine
 *	tries to establish communication with the router the number of times
 *	specified by retry. If it fails to do so, it error exits with errno
 *	set appropriately.
 *
 *	Additionally, this routine checks the sequence number of the return
 *	packet against the one previously transmitted. If they do not match
 *	the return packet is discarded and the routine retransmits the 
 *	request and awaits a new reply.
 *
 */

rcptrans(sin,rcout,rcin,retry,slen,rlen)

  struct sockaddr_in *sin;		/* socket to the router */
  struct rcp         *rcout;		/* packet to be sent */
  struct rcp	     *rcin;		/* packet to be received */
         int          retry;		/* number of retrys before punting */
         int          slen;		/* length of packet to be sent */
         int          rlen;		/* length of returned packet buffer */

{
    struct sockaddr_in  tsin;
    struct sigvec       rcp_fail_vec,	/* vector to pass to sigvec */
			ovec;		/* stores old sigvec        */
    struct itimerval	alarm;
           int          n;
           int          ssin;

    errno = 0;

    /* init timer handler routine vec */
    rcp_fail_vec.sv_handler = rcp_fail;
    rcp_fail_vec.sv_mask    = 0;
    rcp_fail_vec.sv_onstack = 0;

    /* set up interrupt handler for when timer expires */
    sigvec (SIGALRM, &rcp_fail_vec, &ovec);

    /* init timer setup vector */
    alarm.it_interval.tv_sec  = 0;
    alarm.it_interval.tv_usec = 0;
    alarm.it_value.tv_usec    = 0;

    do {
	bcopy ((char *) sin, (char *) & tsin, sizeof (struct sockaddr_in));

        alarm.it_value.tv_sec = 1;		/* one second timer */
	setitimer(ITIMER_REAL,&alarm,NULL);

	if (sendto (rcp_socket, (char *) rcout, slen, 0,
		   (struct sockaddr *) &tsin, sizeof (struct sockaddr_in))<0)
        {
	  cleanup(&ovec,&alarm);
          return(ERROR_EXIT);
	}

	ssin = sizeof (struct sockaddr_in);
	n = recvfrom (rcp_socket, (char *) rcin, rlen, 0,
		      (struct sockaddr *) &tsin, &ssin);
	if (n < 0)
	{
	    switch (errno) {
		case EINTR: 	/* A few timeouts are ok */
#ifdef RCPDEBUG
		    printf ("Timeout\n");
#endif
		    break;
		default:
		   cleanup(&ovec,&alarm);
		   return (ERROR_EXIT);
	    }
	}
	else if (ntohs(rcout->rc_id) == ntohs(rcin->rc_id))
        {					/* check return sequence # */
	  if (ntohs(rcin->rc_type) == V1_RCT_ERROR)
	  {
	    errno = ntohs(rcin->rc_code)+USER_ERROR;
	    cleanup(&ovec,&alarm);
	    return (ERROR_EXIT);
	  }
	  else
	  {
	    cleanup(&ovec,&alarm);
	    return (n);
	  }
	}
    } while (retry-- > 0);
    errno = TIMEOUT;				    /* ran out of retrys */
    cleanup(&ovec,&alarm);
    return (ERROR_EXIT);
}

/*****************************************************************
 * Authorize restricted operations (Reboot, deposit memory etc)
 *
 */
rcpauth(sin,pswd,id)
struct sockaddr_in *sin;		/* Socket to the router */
char *pswd;				/* Passwd (Tested by the router) */
short id;				/* random number to identify me */
{
    struct rcp  rcpout,rcpin;

    rcpout.rc_type = htons (V1_RCT_AUTH);
    bcopy (pswd, rcpout.rc_u.rcu_pswd, RCMAXPSWD);
    rcpout.rc_id = htons (id);
    return (rcptrans (sin, &rcpout, &rcpin, RETRIES,
		     (RCHEAD + sizeof (rcpout.rc_u.rcu_pswd)),
		     sizeof (struct rcp)));
}

/*****************************************************************
 * get (peek at) router memory
 *	To be completed
 */

rcpgetmem(sin,addr,buff,len)
  
  struct sockaddr_in *sin;		/* Socket to the router */
         int          addr;
         char        *buff;
         int          len;
{
	errno = UNSUPPORTED;
	return(ERROR_EXIT);
}

/*****************************************************************
 * put (poke) router memory
 *	To be completed
 */

rcpputmem(sin,addr,buff,len,pswd)

  struct sockaddr_in *sin;		/* Socket to the router */
         int          addr;
         char        *buff;
         int          len;
         char        *pswd;
{
	errno = UNSUPPORTED;
	return(ERROR_EXIT);
}

/*****************************************************************
 * Reboot the router
 *
 */

rcpboot(sin,bootstr,pswd)

  struct sockaddr_in *sin;		/* Socket to the router */
         char        *bootstr;		/* file to boot from (or NULL) */
         char        *pswd;		/* authorization passwd */

{
           short id = htons(rand_id());
           int   bootlen;
    extern char *srncpy();
    struct rcp *rcp_out = (struct rcp *) & (main_xbuff[0]);
    struct rcp *rcp_in  = (struct rcp *) & (main_rbuff[0]);

    if (rcpauth (sin, pswd, id++) == ERROR_EXIT)
	return (ERROR_EXIT);

    rcp_out->rc_type = htons (V1_RCT_REBOOT);
    rcp_out->rc_id = htons (id);

    if (rcptrans (sin, rcp_out, rcp_in, RETRIES,
		  RCHEAD, sizeof (struct rcp)) == ERROR_EXIT)
    {
	errno = BOOT_FAIL;
	return (ERROR_EXIT);
    };
    rcp_out->rc_type = htons (V1_RCT_REBOOT);
    rcp_out->rc_id = htons (++id);

    if (bootstr)
    {
	bootlen = strlen (bootstr);
	(void) (char *) strcpy (rcp_out->rc_b_addr, bootstr);
	bootlen++;
    }
    else
	bootlen = 0;

    if (rcptrans (sin, rcp_out, rcp_in, RETRIES,
		  RCHEAD + bootlen, sizeof (struct rcp)) == ERROR_EXIT)
    {
	errno = BOOT_FAIL;
	return (ERROR_EXIT);
    }
    return (SUCESSFULL);
}

/*****************************************************************
 *  Retreive an information or diagnostic structure.
 *
 *  The new version 1 implementation uses the common diagfo structure
 *  for requesting and returning data. It is NOT compatible with 
 *  the older version 0 rcpsnatch. In fact, it should probably have a
 *  different name.
 *
 */
V1_rcpsnatch(sin,sntype,data,device)

   struct sockaddr_in *sin;		/* Socket to the router */
   int    sntype;			/* information type */
   char  *data;				/* Buffer for the statistics */
   int    device;			/* which router interface */

{
           char       *dstart;
    struct rcp        *rcp_out = (struct rcp *) & (main_xbuff[0]);
    struct rcp        *rcp_in  = (struct rcp *) & (main_rbuff[0]);
    struct rcp_diagfo *rdi;

    rdi = (struct rcp_diagfo *) &(rcp_out->rc_diagfo);
    rcp_out->rc_id = htons(rand_id());
    rcp_out->rc_type = htons ((u_short) V1_RCT_INFO);
    rdi->subtype = htons ((u_short) sntype);
    rdi->u_di.card = htons (device);
    rdi->length = 0;
    if (rcptrans (sin, rcp_out, rcp_in, RETRIES,
		  sizeof (struct rcp), MBUFSIZ) == ERROR_EXIT)
      return(ERROR_EXIT);
    rdi = (struct rcp_diagfo *) &(rcp_in->rc_diagfo);
    dstart = &(rdi->data[0]);
    bcopy (dstart, data, ntohs(rdi->length));
    return (ntohs(rdi->length));
}

/*****************************************************************
 * Get and Put Switch registers/ control parameters
 *
 */

rcp_get_glsw(sin,glsw)

  struct socketaddr_in *sin;
  struct glsw_tran     *glsw;

{
    struct rcp       *rcp_out = (struct rcp *) &(main_xbuff[0]);
    struct rcp       *rcp_in  = (struct rcp *) &(main_rbuff[0]);
    struct glsw_tran *tglsw =
               (struct glsw_tran *) &(rcp_in->rc_u.rcu_rglsw[0]);

    rcp_out->rc_id = htons(rand_id());
    rcp_out->rc_type = htons(V1_RCT_GGLSW);

    if (rcptrans(sin,rcp_out,rcp_in,RETRIES,RCHEAD,MBUFSIZ) == ERROR_EXIT)
       return(ERROR_EXIT);

    mntohl(glsw,tglsw,sizeof(struct glsw_tran)/sizeof(long));

}

rcp_put_glsw(sin,glsw,pswd)

  struct socketaddr_in *sin;
  struct glsw_tran     *glsw;
         char          *pswd;

{
           int        n;
	   short      id = rand_id();
    struct rcp       *rcp_out = (struct rcp *) &(main_xbuff[0]);
    struct rcp       *rcp_in  = (struct rcp *) &(main_rbuff[0]);
    struct glsw_tran *tglsw =
               (struct glsw_tran *) &(rcp_out->rc_u.rcu_rglsw[0]);
 
    if (rcpauth(sin,pswd,id++) == ERROR_EXIT)
        return(ERROR_EXIT);


    rcp_out->rc_id = htons(id);
    rcp_out->rc_type = htons(V1_RCT_PGLSW);

    mhtonl(tglsw,glsw,sizeof(struct glsw_tran)/sizeof(long));

    n = rcptrans(sin,rcp_out,rcp_in,RETRIES,
                 RCHEAD + sizeof(struct glsw_tran),MBUFSIZ);
                    
    if (n < 0)
    {
       errno = PGLSW_FAIL;
       return(ERROR_EXIT);
    }

    tglsw = (struct glsw_tran *) &(rcp_in->rc_u.rcu_rglsw[0]);
    mntohl(glsw,tglsw,sizeof(struct glsw_tran)/sizeof(long));

}

/*****************************************************************
 * Copy a list of longs, massaging the byte order
 *
 */
mntohl(new_arr,old_arr,num)
long *new_arr;
long *old_arr;
int  num;
{
    int cntr;
    for(cntr = 0; cntr < num; cntr++)
       *(new_arr + cntr) = ntohl(*(old_arr + cntr));
}


mhtonl(new_arr,old_arr,num)
long *new_arr;
long *old_arr;
int  num;
{
    int cntr;
    for(cntr = 0; cntr < num; cntr++) {
       *(new_arr + cntr) = htonl(*(old_arr + cntr));
    }
}


#ifdef RCPDEBUG
/*
 *  rc_prt - display an RCP message header
 *
 *  rc  = the header to display
 *  len = the physical length of the RCP message
 */

rc_prt(rc)
register struct rcp *rc;
{
    printf("ID: %d, Type: %d (",
	    rc->rc_id, rc->rc_type);
    switch (rc->rc_type)
    {
	case V1_RCT_GET:
	    printf("GET)\r\nFrom=%o, Length=%d\r\n", rc->rc_from, rc->rc_len);
	    break;
	case V1_RCT_PUT:
	    printf("PUT)\r\nTo=%o\r\n", rc->rc_to, rc->rc_len);
	    break;
	case V1_RCT_DATA:
	    printf("DATA)\r\n");
	    break;
	case V1_RCT_AUTH:
	    printf("AUTH)\r\n");
	    break;
	case V1_RCT_REBOOT:
	    printf("REBOOT)\r\n");
	    break;
	case V1_RCT_INFO:
	    printf("INFO)\r\n");
	    break;
	case V1_RCT_DIAG:
	    printf("DIAG)\r\n");
	    break;
	case V1_RCT_ERROR:
	    printf("ERROR)\r\n");
	    break;
	default:
	    printf("UNKNOWN)\r\n");
	    break;
    }
}
#endif

/*  print_error - interface to perror routine. If type of error is RECOVER,
 *	this routine calls perror with the appropriate string and returns.
 *	If the type is FATAL, it calls perror with string and then exits to
 *	shell with error.
 */

print_error(s,type)

  char *s;
  int   type;

{
  perror(s);
  if (type == RECOVER)
    return;
  else if (type == FATAL)
  {
    errno = 0;
    exit (ERROR_EXIT);
  }
}

/*
 *  perror - "customized" version of UNIX system call which prints
 *	      standard system errors and which allows for user defined
 *	      errors as well. It uses the standard UNIX error table
 *	      for errors below USER_ERROR, and the user's table all
 *	      errors above this number.
 *
 *	      The user's error constants and data structures are
 *	      defined in perror.h
 *
 */

perror (s)

 char *s;

{
    extern int   sys_nerr;
    extern char *sys_errlist[];

    fprintf (stderr,"%s: ",s);			/* print user's message */
    if (errno < sys_nerr)
	fprintf(stderr,"%s\n",sys_errlist[errno]);
    else if (errno > USER_ERROR)
	fprintf	(stderr,"%s\n",usr_errlist[errno-USER_ERROR]);
    else
	fprintf (stderr,"Unknown error\n");
}
