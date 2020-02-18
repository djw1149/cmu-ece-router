/*
 *
 *	Memory Initialization.
 *		Responsible for sizing memory and allocating apropriate sized
 * chunks to various functions.   Also initalize the malloc data structures,
 * since they tend to have memory map dependencies.
 *
 * HISTORY:
 *	Matt Mathis Sep 1986, tuned up, moved function from mallocI
 */

#include "../../h/rtypes.h"
#include "../../h/malloc.h"
#include "../mch/mch.h"
#include "../mch/memory.h"
/*
 *  Isize - size memory, and set up globals for others.
 *		maxaddr: the top of the 'main' data segment.
 *		pkpool:  the policy size of the packet pool.
 *		maxfree: the number of bytes in the malloc pool.
 *		mhead:   the malloc free pool head pointer.
 * Uses		end:     the end of the text, start of the malloc pool
 *		tempcode:the begenning of the temporary code which is
 *				freed at the end of main.
 */

long pkpool;

Isize()
{
    register u_char temp;
    register u_char *mem;
    extern int nxmok;
    extern struct malloc end;
    extern struct malloc tempcode;
    extern int tempsize;
 
    /*
     *  Size memory.
     */
    
    mem = (char *)(((int)&end + 1023) & (~1023));
    for(;;mem+=1024)
    {
	nxmok++;
	temp = *mem;
	if (nxmok == 0)
	    break;
	*mem = 0xff;
	if (nxmok == 0 || *mem != 0xff)
	   break;
	*mem = temp;
	nxmok = 0;
    }
    maxaddr = mem;
    if (maxaddr > (char *) MAXMEMSIZE) maxaddr = (char *) MAXMEMSIZE;
    maxaddr = (char *) ( (int)maxaddr - HEADSPACE);

    end.ma_next = 0;			/*  Set up the malloc pool */
    end.ma_len = maxaddr-(char *)&end;
    MFSET(&end);
    mhead = &end;

				/* set up the temp code for later freeing */
    tempcode.ma_len = (char *)&end-(char *)&tempcode;
    tempsize = tempcode.ma_len-MEXTRA;
    MASET(&tempcode, tempsize);
    maxfree = end.ma_len+tempcode.ma_len;

    pkpool = maxfree/2;			/* set packet pool policy */

    cprintf("Total physical mem avail: %d bytes\r\n", maxaddr);
    cprintf("Malloc memory = %d+%d=%d bytes\r\n",
	    end.ma_len, tempcode.ma_len, maxfree);

}

/* special malloc to allocate packets: since the 68k routers alloacte
	packetes within the malloc pool, pkmalloc is just a malloc.
*/
pkmalloc(size) 
int size;
{
    malloc(size);
}
