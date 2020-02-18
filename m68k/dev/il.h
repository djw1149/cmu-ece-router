/*
 *  Interlan 10Mb ethernet data structure definitions
 */

#define	IL_HLN		6
#define	IL_MTU		ILMAXDATA
#define	IL_HRD		0x1	/* ARP hardware address space */

#define	ILCRCLEN	0	/* length of CRC */
#define	ILLEADER	0	/* bytes of leader not in frame length */
#define	ILMINDATA	46	/* minimum data length */
#define	ILMAXDATA	1500	/* maximum data length */
#define	ILMINLEN	(ILHEADLEN+ILMINDATA+ILCRCLEN+ILLEADER)
#define	ILMAXLEN	(ILHEADLEN+ILMAXDATA+ILCRCLEN+ILLEADER)

#define IL_MAXSEC	2	/* length of transmitter watchdog timer */

/*
 * packet headers
 */
struct ilpacket
{
    u_char  il_dhost[6];
    u_char  il_shost[6];
    u_short il_type;
};

#define ILHEADLEN	(sizeof (struct ilpacket))

/*
 *  Packet types
 */

#define	ILT_CTP		0x009	/* configuration testing protocol */
#define	ILT_IP		0x800	/* internet protocol */
#define	ILT_CHAOS	0x804	/* CHASOnet protocol */
#define	ILT_AR		0x806	/* address resolution protocol */
#define ILT_XPUP	0xdb00	/* Rodeheffer pup encapsolation */

/*
 *  Interlan 10mb ethernet device dependent field
 */

#define	ILDEVDEP
struct ildevdep {
    u_char 	*il_mem;		/* base pointer to shared memory */
    u_char 	*il_free;	/* free pointer to shared memory */
    int		il_flags;
    long	il_wt;		/* 1 second xmit watchdog timer */
    struct scb	*il_scb;	/* pointer to system control block */
    struct tfd	*il_tf;		/* pointer to transmit desc */
    struct rfd	*il_lrf;	/* ptr to tail of recv desc chain */
    struct rbd	*il_lrb;	/* ptr to tail of receive buf chain */
};

/* Bits in il_flags */
#define	ILF_OACTIVE	0x01	/* transmitter active */
#define ILF_XQFULL	0x02	/* transmitter queue full - waiting */
#define ILF_XRESET	0x04	/* watchdog timer expired - reset device */
#define ILF_RCONGEST	0x08	/* receiver is congested  - transcribing */

#define	ilalloc(dv, p, t, sz)		\
{					\
    (p) = (t *)(dv)->dv_il.il_free;	\
    (dv)->dv_il.il_free += (sz);	\
    bzero((p), (sz));			\
}

#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

#define	NILRBUF	4


struct ildevt {
    struct ildevice *baseaddr;
    long smaddr;
    long priority;
};
