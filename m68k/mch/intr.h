/*	intr.h
 *
 * HISTORY:
 * 2-July-86  Harold Weisenbloom(htw) and Matt Mathis(mathis)
 * 	at Carnegie-Mellon University moved contents from autoconfI.c
 */

#define		PS_BR1	(0x0100)
#define		PS_BR6	(0x0600)


/*
 *  Structure returned by Iprobe().
 */

typedef struct
{
    int   	   pb_br;		/* BR level of device */
    struct vector *pb_vector;		/* interrupt vector address */
}p_inter;
