/*
 * Mashed into the AT router 1986, Matt Mathis
 *
 * For Prior life see:
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*****************************************************************
 *  Mystical data types and conversion routines
 *	These help to hide some of the chaos in the network world
 *		by making adapter data look like AT data.
 */
/* Byte order problems:  The TR stores far pointers
 * in the reverse order from the AT (The same as the network order).
 * However to make life difficult the direct IO interface does only
 * word (=16 bit) operations, and corrects the byte order.   The
 * DMA interface preserves byte order.   Thus to DMA longs (and far pointers)
 * they must be swal'd as with the network, but to dio them, they must only be
 * word swaped.   Since DMA is used heavily, the conversion for AT pointers
 * to adapter pointers done in assembly.
 */
typedef u_long long24;		/* An adapter style pointer, in net order */

/* This converts the segment:offset into a true address, and swaps bytes */
extern long24	caddrto24();

/* NEARTO24, converts near pointers to net order adapter pointers */
#define NEARTO24(thing) caddrto24( (char far *) thing)

/* NEARTODIO, converts near pointers to DIO order adapter pointers */
#define NEARTODIO(thing)	\
	( ( ((long)caddrto24((char far *)thing)) >>8) &0xFF00FF   | \
	( ( ((long)caddrto24((char far *)thing)) &0xFF00FF)  <<8) )
/* Yes it's ugly, but it only happens 4 times in the life of a router */

/* byte swap for use in structure initalizations and constants only */
#define SWAS(val) ( ((val)>>8)&0xFF | (((val)&0xFF)<<8) )
/* The compiler is smart enough to make SWAS(constant) into a new constant */

/*****************************************************************
 *	The definitions of the raw resources:
 *		Access to the adapter registers
 */
/* primitives to do direct io.  These correct the byte order */
extern		diow();		/* write: diow(far i/o address, u_short ) */
extern short	dior();		/* read: (u_short) dior(far i/o address ) */
/* macros to make the above easy to use.  */
/* reg is a far pointer to the device registers. */
#define DIOW(offset,value)	diow(&(reg->offset), value)
#define DIOR(offset)		dior(&(reg->offset))

/* Standard addresses for the cards. */
#define FIRSTCARD	(struct trdevice *) 0x1c0
#define SECONDCARD	(struct trdevice *) 0x140

/*	The device registers.  They reside in I/O space and are best
 *	referenced via the above macros: reg=0x1C0;DIOW(tr_data,value); */
struct trdevice {
    u_short  tr_data;			/* data to the card */
    u_short  tr_datai;			/* Auto increment data to the card */
    u_short  tr_address;		/* address to the card */
    u_short  tr_cmdstat;		/* command/status */
    u_short  tr_enable;			/* enable adapter interrupts	*/
    u_short  tr_padd;			/* A hole */
    u_short  tr_disable;		/* disable adapter interrupts	*/
    u_short  tr_reset;			/* reset the adapter */
};
#define      TR_ILE ((u_short far *)0x6F4)	/* Shaired interupt enable
							(absolute addr) */

/* Register functions/bits:
	These are all used directly with the above registers */

/*	System to Adapter Interrupts					*/
/* Written to the tr_cmdstat register */
#define  TR_RESET	0xFF80	/* reset adapter			*/
#define  TR_SSBCLEAR	0xA000	/* notify that status block available	*/
#define  TR_EXECUTE	0x9080	/* initiate command in command block	*/
#define  TR_RECVCONT	0x8480	/* request recv operation to continue	*/
#define  TR_RECVALID	0x8280	/* signal recv list suspension cleared	*/
#define  TR_XMTVALID	0x8180	/* signal xmit list suspension cleared	*/

#define SSBCLEAR {DIOW(tr_cmdstat, TR_SSBCLEAR);}

/*	Adapter Initialization Status (BringUp phase)	*/
/*	These values are read from the tr_cmdstat register		*/
#define  TR_INITIALIZE	0x0040	/* bring-up diagnostics complete	*/
#define  TR_TEST	0x0020	/* initialization test			*/
#define  TR_ERROR	0x0010	/* initialization error			*/

/*	Adapter to System Response					*/
/*	These values are read from the tr_cmdstat register		*/
#define  TR_INT		0x0080  /* valid interrupt			*/
#define  TR_ADAP_INT	0x000E	/* adapter -> system interrupt code	*/
#define  TR_ACHECK	0x0000	/* unrecoverable adapter error		*/
#define  TR_RINGSTAT	0x0004	/* ring status update			*/
#define  TR_SCBCLEAR	0x0006	/* SCB clear				*/
#define  TR_CMDSTAT	0x0008	/* command status update		*/
#define  TR_RECVSTAT	0x000A	/* receive status update		*/
#define  TR_XMITSTAT	0x000C	/* transmit status update		*/

/*****************************************************************
 * Adapter data structures and related constants
 */
/*	System Command Block						*/
struct tr_scb {
    unsigned short command;	/* what it is that we want done */
    long24 clist;		/* and the data for it */
}; 

/*	Adapter Commands						*/
/*	These values are written to the tr_scb control block		*/
#define  TR_OPEN	0x0003	/* open adapter				*/
#define  TR_TRANSMIT	0x0004	/* transmit frame			*/
#define  TR_TRANSHLT	0x0005	/* interrupt tranmsmit list chain	*/
#define  TR_RECEIVE	0x0006	/* receive frames			*/
#define  TR_CLOSE	0x0007	/* close adapter			*/
#define  TR_SETGADDR	0x0008	/* set group address			*/
#define  TR_SETFADDR	0x0009	/* set functional address		*/
#define  TR_RDERRORLOG	0x000A	/* read error log			*/
#define  TR_RDADAPTR	0x000B	/* read adapter storage			*/
#define  TR_CMDMASK	0x000F	/* get the command */

/*	System Status Block						*/
struct tr_ssb {
    unsigned short command;
    unsigned short status0;
    unsigned short status1;
    unsigned short status2;
}; 

/*	Generic command completion (in status0 for most commands) */
#define TR_COMPLETE	0x8000
#define TR_FCOMPLETE	0x4000		/* Frame complete */

/*	Adapter Status							*/
/*	These values are read from the tr_ssb control block		*/
#define  TR_SSB_RING	0x0001	/* ring status update			*/
#define  TR_SSB_REJECT	0x0002	/* command reject			*/

/*	Ring Status							*/
/*      These values are read from the lan_ssb on ring status update	*/
#define  TR_SIGNAL_LOSS	0x8000	/* signal loss			*/
#define  TR_HARD_ERROR	0x4000	/* xmit/recv beacon frames	*/
#define  TR_SOFT_ERROR	0x2000	/* xmit report error mac frame	*/
#define  TR_XMIT_BEACON	0x1000	/* xmit beacon frames		*/
#define  TR_WIRE_FAULT	0x0800	/* short circuit in data path	*/
#define  TR_AUTOER1	0x0400	/* auto-removal process		*/
#define  TR_AUTOER2	0x0200	/* reserved			*/
#define  TR_REMOVE_RECV	0x0100	/* remove received		*/
#define  TR_CTR_OVER	0x0080	/* counter overflow		*/
#define  TR_SINGLE	0x0040	/* single station		*/

/*	Adapter locations for initialization and adapter check		*/
/*	These values are written to the lan_address register		*/
#define  TR_INIT_DATAA  0x0200
#define  TR_ACHECK_DATA 0x05E0

/* Adapter Initialization parameters */
/* This gets copied into the adapter at initialization time (handshake) */
struct tr_init_params {
	unsigned short options;		/* Init options */
	char command;			/* Interrupt vector for SSB update */
	char transmit;			/* ... on tx status */
	char receive;			/* ... on rx status */
	char ring;			/* ... on ring status */
	char scb_clear;			/* ... on scb clear */
	char adpt_check;		/* ... on adapter check */
	unsigned short rx_burst;	/* Rx burst size */
	unsigned short tx_burst;	/* Tx burst size */
	unsigned short dma_abort;	/* Number of DMA fails before abort */
	long24 scb_addr;
	long24 ssb_addr;
};
/* Good value for above structure! */
/* Burst options, interrupt level 12..., burst size 16, 0x303 DMA aborts */
#define DEF_INIT_PARAMS	{ 0x9F00, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, \
	0x0010, 0x0010, 0x0303, 0L, 0L }

/* Adapter open parameters */
/* This is passed to the adapter to initiate an open */
struct tr_open_params {
	u_short options;		/* open options */
	u_char nodea[6];		/* node address */
	u_char groupa[4];		/* group address */
	u_char funa[4];			/* functional address */
	u_short rlist;			/* receive list size */
	u_short xlist;			/* transmit list size */
	u_short bsize;			/* buffer size */
	u_short ramstart;		/* expansion ram start address */
	u_short ramend;			/* expansion ram end address */
	u_short xbminmax;		/* transmit buffer min & max count */
	long24 prodid;			/* product ID buffer */
};
/* good value for the above structure! */
#define DEF_OPEN_PARAMS {\
		SWAS(0x0000),		/*  */ \
		{0}, {0}, {0},		/* use the BIA */ \
		SWAS(14), SWAS(14),	/* 14 byte (single buffer) lists */ \
		SWAS(112),		/* 112 byte internal buffers*/ \
		SWAS(0x4006), SWAS(0x7FFE), /* Expansion ram */ \
		SWAS(0x16),		/* 1540 byte max frame size */ \
		0}			/* no produce ID */

/*	Open Status							*/
#define  LAN_OPEN_COMPLETE	0x8000	/* open complete		*/
#define  LAN_OPEN_NODE_ERROR	0x4000	/* node address error		*/
#define  LAN_OPEN_LIST_ERROR	0x2000	/* recv/xmit list size error	*/
#define  LAN_OPEN_BUF_ERROR	0x1000	/* buffer size error		*/
#define  LAN_OPEN_RAM_ERROR	0x0800	/* RAM address error		*/
#define  LAN_OPEN_XMIT_ERROR	0x0400	/* xmit buffer count error	*/
#define  LAN_OPEN_ERROR		0x0200	/* error detected during open	*/

/*	Open Command Phases						*/
#define  LAN_OPEN_LOBE_TEST	0x0010	/* lobe media test		*/
#define  LAN_OPEN_INSERTION	0x0020	/* physical insertion		*/
#define  LAN_OPEN_ADDR_VER	0x0030	/* address verification		*/
#define  LAN_OPEN_ROLL_CALL	0x0040	/* roll call poll		*/
#define  LAN_OPEN_REQ_PARM	0x0050	/* request parameters		*/

/*	Open Error Codes						*/
#define  LAN_OPEN_FUNC_FAILURE	0x0201	/* function failure		*/
#define  LAN_OPEN_OSIGNAL_LOSS	0x0202	/* signal loss			*/
#define  LAN_OPEN_OWIRE_FAULT	0x0203	/* wire fault			*/
#define  LAN_OPEN_FREQ_ERROR	0x0204	/* unused			*/
#define  LAN_OPEN_TIMEOUT	0x0205	/* timeout			*/
#define  LAN_OPEN_RING_FAILURE	0x0206	/* ring failure			*/
#define  LAN_OPEN_RING_BEACON	0x0207	/* ring beaconing		*/
#define  LAN_OPEN_DUP_NODE	0x0208	/* duplicate node address	*/
#define  LAN_OPEN_OREQ_PARM	0x0209	/* request parameters		*/
#define  LAN_OPEN_OREM_RECV	0x020A	/* remove received		*/

/* Adapter list structure, used for transmit and receive lists */
/* Trickery: This is a structure within a structure: The outer structure
 * has AT format fields.  The inner structure has adapter format fields,
 * some of which duplicate the AT fields.
 */
/* assumption: packets are to be manipulated as single pieces.
 *	The list size is always 14 bytes.
 */
typedef struct tr_list * p_tr_list;

struct tr_list {
    p_tr_list	forward;		/* forward pointer */
    p_packet	pkt;			/* packet containing this buffer */
    struct {
	long24      forward;		/* adapter style forward pointer */
	u_short     cstat;		/* command/status */
	u_short     fsize;		/* frame size */
	u_short     dcount;		/* data count (first part) */
	long24      daddr;		/* data address (first part) */
    } adapt;
};

/* Values for the list cstats (above) */
#define TR_FVALID	0x8000		/* frame valid */
#define TR_FCOMPETE	0x4000		/* frame complete */
#define TR_FSOFEOF	0x3000		/* Start of frame/end of frame */
#define TR_FINT		0x0800		/* frame interupt when done */
#define TR_FERROR	0x0400		/* strip error */
#define TR_FFS		0x00FF		/* copy of the striped FS field */
#define TR_FFSV		0x00CC		/* Expected value of the FS field */


#define TRANSMIT_LISTS 6		/* Number of transmit lists (silly) */
#define RECEIVE_LISTS 3			/* receive lists (tune this)*/

#define TR_BIA 0x0A04		/* locaition of the Burned in address */

/*	Miscellaneous							*/
#define  LAN_RETRY	4	/* Retries during initialization	*/
#define  LAN_PCF0	0x00	/* Physical Control Field 0		*/
#define  LAN_PCF1	0x40	/* Physical Control Field 1 (not mac)	*/
#define  SIXTY		60	/* ring recovery time in seconds	*/

#define  TENMS		1200	/* argument to DELAY 10 miliseconds	*/
/* macro to delay nseconds */
#define  NSEC(time)	{int cnt=time*100; while (cnt--) DELAY(TENMS);}
/* macro to poll for up to nseconds */
#define  POLL(time,cond) {int cnt=time*100; while ((cond)&&cnt--) DELAY(TENMS);}
