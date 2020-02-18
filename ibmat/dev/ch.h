/*
 * Chichen Hawk 10 Mbit ethernet data structure definitions
 */

#define	CH_HLN		6
#define	CH_MTU		CHMAXDATA
#define	CH_HRD		0x1	/* ARP hardware address space */

#define	CHCRCLEN	0	/* bytes of transfered CRC */
#define	CHLEADER	0	/* bytes of transfered leader not in frame */
#define	CHMINDATA	46	/* minimum data length */
#define	CHMAXDATA	1500	/* maximum data length */
#define	CHMINLEN	(CHHEADLEN+CHMINDATA+CHCRCLEN+CHLEADER)
#define	CHMAXLEN	(CHHEADLEN+CHMAXDATA+CHCRCLEN+CHLEADER) /*Max buffer*/

/*
 * packet headers
 */
struct chpacket
{
    u_char  ch_dhost[6];
    u_char  ch_shost[6];
    u_short ch_type;
};

#define CHHEADLEN	(sizeof (struct chpacket))

/*
 *  Packet types
 */

#define	CHT_CTP		0x009	/* configuration testing protocol */
#define	CHT_IP		0x800	/* internet protocol */
#define	CHT_CHAOS	0x804	/* CHASOnet protocol */
#define	CHT_AR		0x806	/* address resolution protocol */
#define CHT_XPUP	0xdb00	/* Rodeheffer pup encapsolation */

/*
 *  Chicken Hawk 10mb ethernet device dependent fields
 */

#define	CHDEVDEP
struct chdevdep {
    int nextpage;		/* page where we will read the next packet */
    char rearm;			/* command to issue on interupt exit */
};
