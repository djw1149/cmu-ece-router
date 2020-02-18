/*
    This version of tftp.c was modified by Robert Barker to correct bugs that
    caused the tftp boot program to fail under conditions of high load on the
    net.
*/

#include "param.h"
#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ip.h"
#include "tftp.h"
#include "udp.h"
#include "globals.h"
#include "led.h"

tftp_get(to,file,m,handle)
struct in_addr *to;
char *file;
struct mbuf *m;
int (*handle)();
{
	register struct tftphdr *tp;
        register struct upcb *up;
	register int block = 0, size;
	register int err = TFTP_GOOD;
	int retries = 0;
	int result = FAILURE;

        if ((up = (struct upcb *)net_open(to, 0)) == 0)
	   panic ("tftp open");
        net_port (up, IPPORT_TFTP, 0);
	for(;;) { /* send request/ACK */
	        m->offset = MLEN - TFTP_SEGLEN - 4;
	        tp = mtod (m, struct tftphdr *);
		if (block == 0)
		    size = tftp_request (RRQ,m,file);
		else
		    size = tftp_ack(m,block);
		net_write (up, m, size);

		if (retries++ >= MAXRETRIES) {
		    printf ("\ntftp: timeout, host not responding\n");
		    result = FAILURE;
		    goto exit;
		}
		err = tftp_receive(m, up, block, handle);
		switch(err) {
		    case TFTP_EOF:
		    	result = SUCCESS;
			goto exit;
		    case TFTP_ERROR:
		        tftp_error(m);
		    	result = FAILURE;
			goto exit;
		    case TFTP_GOOD:
			retries = 0;
			block++;
			break;
		    case TFTP_OK:
			retries--;	/* Don't count this time around. */
			break;
		    case TFTP_TIMEOUT:
			putc('.');
			break;
		    case TFTP_HANDLE_ERROR:
			printf("\ntftp: internal error.");
			result = FAILURE;
			goto exit;
		    default:
			panic("\ntftp: tftp_receive returned illegal value.");
		}
	}
exit:
    m->offset = MLEN - TFTP_SEGLEN - 4;
    tp = mtod (m, struct tftphdr *);
    tp->th_opcode = ERROR;
    net_write(up, m, 4);
    net_close(up);
    return(result);
}

tftp_receive(m, up, block, handle)
struct mbuf *m;
struct upcb *up;
int block;
int (*handle)();
{
    register struct tftphdr *tp;
    int timeout, n;
    
    timeout = TIMEOUT;
    while (net_poll(&timeout, m)) {
	if ((n = net_read(up, m) - 4) > 0) {
	    tp = mtod (m, struct tftphdr *);

	    if ((tp->th_opcode == DATA) && (tp->th_block == (block + 1)))
	    {
		if ((*handle)(tp->th_data, tp->th_block) == 0)
		    return(TFTP_HANDLE_ERROR);
		else if (n != TFTP_SEGLEN)
		    return(TFTP_EOF);

		if (block == 0)
		    net_port(up, up->peerport, up->myport);
		break;	/* Break out of while. */
	    }
	    else
	    	if (tp->th_opcode == ERROR)
		    return(TFTP_ERROR);
		else
		    return(TFTP_OK);
	}
    }
    if (timeout <= 0)
        return(TFTP_TIMEOUT);
    return(TFTP_GOOD);
}

tftp_request(request, m, file)
int request;
char *file;
struct mbuf *m;
{
	extern char *strcpy();
	register struct tftphdr *tp;
	register char *cp;
	register int n;

	tp = mtod(m, struct tftphdr *);
	tp->th_opcode = (u_short)request;
	cp = strcpy (tp->th_stuff, file);
	n = (int)(strcpy (cp, "octet") - mtod(m, char *));
	return (n);
}

tftp_ack(m, block)
register struct mbuf *m;
int block;
{
    register struct tftphdr *tp;
    
    tp = mtod(m, struct tftphdr *);
    tp->th_opcode = (u_short)ACK;
    tp->th_block = (u_short)block;
    return(4);
}

tftp_error(m)
struct mbuf *m;
{
    struct tftphdr *tp;

    tp = mtod(m, struct tftphdr *);
    printf("\ntftp: error from foreign host: %s",tp->th_data);
}