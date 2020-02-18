/*
 * $Header: rd_utils.c,v 1.3 86/10/10 09:46:49 chriss Exp $
 *
 * file: rd_utils.c
 *
 * synopsis: router diagnosis utility module
 *
 * modification history:
 *
 *   Jun-86 modified by Chriss Stephens (chriss) at Carnegie Mellon University
 *	to use common rdiagfo header. This version compatible with ver 1 of
 *	router side software. Additionally, now compatible with customized
 *	perror and standardized error handling.
 *
 * david waitzman (djw@ampere.ece.cmu.edu) march '86
 *	created from rcp code by Matt Mathis, Kevin Krimse and Mike Acceta
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

struct rd_devtypesT rd_devtypes[MAXDEVTYPES] =
    {
     {"il",  DVT_IL10, {{"dump", ILDIAG_DUMP, AY}, {"stats", ILDIAG_STATS, AN},
     			{"tdr",  ILDIAG_TDR, AY},  {"diag",  ILDIAG_DIAG, AN},
			{"DUMP", ILDIAG_DUMP, AY}, {"STATS", ILDIAG_STATS, AN},
			{"TDR",  ILDIAG_TDR, AY},  {"DIAG", ILDIAG_DIAG, AN},
			{"regs", ILDIAG_STATS, AN}}},
     {"ec",  72, {{"thump", 1, AN}}},	/* debug only */
     {"bug", 73, {{"bump", 5, AY}}},	/* debug only */
     {0,0,0}	/* end case */
    };

#define MBUFSIZ 1500
static char main_xbuff[MBUFSIZ];
static char main_rbuff[MBUFSIZ];
extern int errno;
extern char *strcpy();

#define RCP_SERV_PORT 0xff
#define RETRIES 10			/* Ten attempts if needed */


/* rcp_diag
 * This function retrieves an information (statistics) structure from given
 * socket and returns a pointer to the returned information. The information
 * is in a static area and must be copied between calls to this if the 
 * information is to be preserved.  if pswd is non-null, the diagnosis
 * is authorized using pswd as the password.
 *
 * This version is compatible with the diagfo rcp header and places all
 * diagnosis request information in this header with the exception of the 
 * dev_type. This parameter is placed at the head of the packet data area
 * where it is overwritten by the returning data.
 *
 */

struct rcp_diagfo *rdiag(sin, device, diagtype, devtype, pswd)

  struct sockaddr_in *sin;		/* Socket to the router */
                 int  device;		/* which router interface */
                 int  diagtype;		/* diagnosis type */
                 int  devtype;		/* device type */
                char *pswd;		/* the diagnosis needs authorization */

{
    struct        rcp *rcp_out = (struct rcp *) &(main_xbuff[0]);
    struct        rcp *rcp_in  = (struct rcp *) &(main_rbuff[0]);
    struct rcp_diagfo *rd        = (struct rcp_diagfo *)
				     &(rcp_out->rc_diagfo);
           u_short    *rc_devtype= (u_short *)rd->data;

    rcp_out->rc_type	= htons(V1_RCT_DIAG);
    if (pswd)
       rcp_out->rc_id   = htons(auth_diag(sin, pswd));
    rd->subtype 	= htons((u_short)diagtype);
    rd->u_di.card	= htons((u_short)device);
    *rc_devtype		= htons((u_short)devtype);
    rd->length		= htons(sizeof(u_short));

    if (rcptrans(sin, rcp_out, rcp_in, RETRIES, 
		 sizeof(struct rcp)+sizeof(u_short), MBUFSIZ) == ERROR_EXIT)
	return(NULL);

    return((struct rcp_diagfo *)&(rcp_in->rc_diagfo));
}

/*
 * takes a diagnosis name and returns the number of the diagnosis
 * *devtype is set to the device type indicated by the diagnosis name
 * *devtype is not read- i.e. the first name found is used
 * *needsauth is set to AY if the diagnosis requires authorization, else
 * it is set to AN.
 */
int diagname_to_num(input, devtype, needsauth)
char *input;				/* the diagnosis name */
int *devtype;				/* the device type (only returned) */
short *needsauth;			/* the diagnosis needs authorization */
{
    struct rd_devtypesT *rddv;
    struct rd_diagtypesT *rddt;
    int temp;

    *devtype = 0;	/* the default case */
    *needsauth = AN;	/* the default case */
    if ((temp = atoi(input)) > 0)   /* negative diagnosis #'s aren't allowed */
	return(temp);

    for (rddv = rd_devtypes; rddv->devtype; rddv++)
      for (rddt = rddv->diagtypes; rddt->diagnum; rddt++)
	if (!strcmp(rddt->diagname,input)) {
	  *devtype = rddv->devtype;
	  *needsauth = rddt->needsauth;
	  return(rddt->diagnum);
	};

    return(SUCESSFULL);
}

/*
 printf("devname %s, rddt %x, diag %s, num %d\n", rddv->devname, rddt,
	rddt->diagname, rddt->diagnum);
*/

char ntemp[5];
/*
 * takes a diagnosis number and returns the name of the diagnosis.
 * if devtype is non zero, it must point to a device name, and this looks
 * up and returned what input means to that device, else the first device whose
 * has a matching diagnosis type is used.
 * if no device uses that number, or the given device don't use that number,
 * the string "#<the number>" is returned.
 * the string returned is in a static area-> don't deallocate it.
 */
char *diagnum_to_name(input, devtype)
int input;
int devtype;
{
    struct rd_devtypesT *rddv;
    struct rd_diagtypesT *rddt;
    for (rddv = rd_devtypes; rddv->devtype; rddv++)
      if (!devtype || (rddv->devtype == devtype))
        for (rddt = rddv->diagtypes; rddt->diagnum; rddt++)
	  if (rddt->diagnum == input)
	    return(rddt->diagname);

    sprintf(ntemp, "#%d", input);
    return(ntemp);	   
}


/*
 * takes a device type name or a card number in input.  devtype a pointer to
 * the device type.  if input is represents a number, the number is returned.
 * else, tries to lookup the input as a device type name.  If it is found, then
 * *devtype is set to the device number (not the card number, instead,
 * something like DVT_???), and 0 is returned.  If the device name isn't found
 * -1 is returned as an error condition.
 */
int devname_to_num(input, devtype)
char *input;
int *devtype;
{
    struct rd_devtypesT *rddv;
    int temp;
    
    if ((temp = atoi(input)) != 0) 
	return(temp);
    
    for (rddv = rd_devtypes; rddv->devtype; rddv++)
      if (!strcmp(rddv->devname,input)) {
	*devtype = rddv->devtype;
	return(0);
      }
    return(ERROR_EXIT);
}


/*
 * does a net to host short coversion over the words in the given data block
 */
bntohs(data, len)
register u_short *data;
register int len;
{
    while(len--) {
	*data = ntohs(*data);
	*data++;
    }
}

/*
 * take the absolute value of a short
 */
short sabs(num)
short num;
{
    return(num < 0? -num : num);
}


/*
 * authorize the diagnosis.  exit if we can't authorize it.
 */
int auth_diag(sin, pswd)
struct sockaddr_in *sin;		/* Socket to the router */
char *pswd;				/* the diagnosis needs authorization */
{
    u_short     id = rand_id();

    if (rcpauth (sin, pswd, id++) == ERROR_EXIT)
	print_error("rd_utils",FATAL);
    return(id);
}
    
