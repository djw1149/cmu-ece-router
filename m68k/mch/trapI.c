/*	trapI.c
 *
 * HISTORY:
 *
 * 7-aug-86	Harold Weisenbloom(htw) at Carnegie-Mellon University
 *		made NXM_check
 *
 * 2-July-86  Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University moved contents from autoconfI.c
 */


#include "../../h/rtypes.h"
#include "../mch/mch.h"
/*
 * EXCEPTVECTORS are the exception vectors for the 68K processor. They
 * start at location 0 and extend to location 1024. vector 0 is at location
 * 0. exception 1 is at location 4. exception 2 is at location 8 and so on.
 *
 * CATCHVECTORS contains all the jump points for the exception vectors.
 * each exception vector contains a pointer to its corresponding catch
 * vector. exception vector 0 points to location CATCHVECTORS. exception
 * vector 1 points to location CATCHVECTORS+4. exception vector 2 points
 * to location CATCHVECTORS+8 and so on. when an exception is processed
 * the value its corresponding exception vector is loaded into the pc.
 * this value is a pointer to the corresponding catch vector.
 * since the catch vector contains a BSR to the catch routine the
 * value of the catch vector is pushed onto the stack as the return pc.
 * the catch routine can use this value to determine which exception
 * has occured
 */

#define EXCEPTVECTORS	((int *)0)
#define CATCHVECTORS	((int *)0x800)
#define VECTSIZE 1024

Ivector()
{
    extern int catchjsr[];
    register int **vector;
    register int *pc;
    register int i;
    extern (*catch)();
    extern utrap();

    /*
     *  Initialize interrupt vector area in low 1024 bytes
     *  of memory, and initialize the catch vector array.
     *
     */

    vector = EXCEPTVECTORS;
    pc = CATCHVECTORS;

    pc[2] = catchjsr[0];
    vector[2] = &pc[2];

    for (i = 25; i < 31; i++) 
    {
	pc[i] = catchjsr[0];
	vector[i] = &pc[i];
    }
    catch = utrap;

}
/*
 *	NXM_check - check if device memory exists
 *
 *	dev	- device CSR address
 *
 *	size	- size of memory to check
*/

NXM_check(dev,size)
	p_pdev *dev;
	int size;
{
int	i;
short	*wp;

    for (wp=(short *)dev, i= size; i; i-=sizeof(short))
	if (Ibadaddr(wp++))
		return(1);
}

/*
 *  Ibadaddr - check if a memory address exists
 *
 *  addr = address to check
 *
 *  Return: TRUE if an attempt to read the word at the specified address
 *  generates a NXM trap, otherwise FALSE.
 */

Ibadaddr(addr)
register u_char *addr;
{
    extern int nxmok;
    register int val;

    nxmok++;		/* not "nxmok = 1" because of the optimizer */
    val = *addr;
    if (nxmok == 0)
	val = TRUE;
    else
	val = FALSE;
    nxmok = 0;
    return(val);
}

