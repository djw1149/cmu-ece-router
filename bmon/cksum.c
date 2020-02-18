#include "param.h"
#include "types.h"
#include "mbuf.h"

/*++
 * 1s complement of [1s complement sum of header and data (16 bit words)] 
 *
 *--*/

cksum(m, length)
register struct mbuf *m;
register int length;
{
	register u_short *s;
	register long sum;
	u_short i;

	s = mtod(m, u_short *);
	sum = 0L;

	while (length > 0)
	{
	    if (length > 1)
		sum += *(s++);
	    else
		sum += *((u_char *)s);
	    length -= 2;
	}

	sum = (sum & 0xffffL) + (sum >> 16);
	sum = (sum + (sum >> 16));
	return((u_short)~sum);
}

