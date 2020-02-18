/* Chicken Hawk register definitions */

#define MAXPAGE 95			/* the last page in the pool */

/* device structure, including all control registers, ROM, buffers etc. */
struct chdevice {
    u_char	eprom[0x2000];		/* on board eprom */
    u_char	pad1[0x2080-0x2000];	/* undefined */
    u_char	tsah;			/* Write only: T. S. A. high */
    u_char	tsal;			/* WO: Transmit starting addr low */
    u_char	csr;			/* control/status register */
    u_char	fpp;			/* WO/ full Page pointer  */
    u_char	pad2[0x2100-0x2084];	/* undefined */
    u_char	psize[0x215f-0x2100];	/* 96 byte page size ram */
    u_char	pad3[0x2180-0x215f];	/* undefined */
    u_char	xcsr;			/* EDLC transmit csr */
    u_char	ximask;			/* EDLC transmit interupt mask */
    u_char	rcsr;			/* EDLC receive csr */
    u_char	rimask;			/* EDLC recieve interupt mask */
    u_char	xmode:4,		/* EDLC transmitter mode */
    		colla:4;		/* EDLC Collision attempts */
    u_char	rmode;			/* EDLC recieve mode */
    u_char	rst;			/* EDLC reset */
    u_char	tdrlow;			/* EDLC TDR time (low byte) */
    u_char	addr[6];		/* EDLC enet address */
    u_char	pad4;			/* EDLC reserved */
    u_char	tdrhigh;		/* EDLC TDR, high byte */
    u_char	pad5[0x4000-0x2190];	/* undefined */
    u_char	rbuff[96][128];		/* recieve buffer pool */
    u_char	xbuff[2][0x800];	/* Two transmit buffers */
					/* That's it - all 32k */
};
/* Register aliases */
#define epp	fpp	/* read empty page pointer, write full page pointer */
#define startx	tsah	/* read only start transmit (write: tsah) */
#define clrpav	tsal	/* read only clr pkt avail (write: tasl) */

/* Transmit start address reg */
#define XAML	0x00FF		/* Low  TSAR, (address rel to CHSA) */
#define XAMH	0x0F00		/* High */

/* The physical address is in the eprom at... */
#define PADDR	0x10	/* DOES NOT AGREE WITH THE DOC */

/* Chicken Hawk CSR bits: Enables (writes) */
#define XRIE	0x80	/* Transmit Ready Interupt Enable */
#define PAVIE	0x40	/* Packet Availible Interupt enable */
#define SFTI	0x20	/* Generate an interupt request */
#define TIE	0x10	/* Timer interupt enable */

/* Chicken Hawk CSR bits: Status (reads) */
#define XRI	0x80	/* Transmit Ready Interupt */
#define PAVI	0x40	/* Packet Availible Interupt */
#define SFTI	0x20	/* Generate an interupt request */
#define TI	0x10	/* Timer interupt */
#define RINT	0x08	/* Raw RINT from EDLC */
#define TINT	0x04	/* Raw TINT from EDLC */
#define TPOK	0x02	/* ???? */
#define XDONE	0x01	/* Transmit done */

/* chicken hawk EPP/FPP bits */
#define PAV	0x80	/* read packet availible */
#define ARM	0x80	/* write arm interupts NOT IN DOC */
#define PPM	0x7F	/* read/write first/next packet page */

/* psize ram fields.  The psize ram is used to indicate if this is the last
	page of a packet, and if so, how many bytes */
#define PEND	0x80	/* last page of a packet */
#define PL	0x7f	/* byte count within the last page */

/* EDLC transmit csr bits.  The same bits are status (xcsr read) */
/* event clears (xcsr writes) and transmit interupt masks (ORed into ximask)*/
#define XRDY	0x80	/* ready for a packet */
#define NETBUS	0x40	/* Net is busy */
#define XREC	0x20	/* This transmit was recieved */
#define XSHORT	0x10	/* Cable may be shorted */
#define XUVR	0x08	/* Xmit under run */
#define XCOLL	0x04	/* Collision */
#define XMC	0x02	/* MAX collisions occorred */
#define XPAR	0x01	/* memory parity error */
#define CLRXCSR (XUVR|XCOLL|XMC|XPAR)	/* clear all errors */

/* EDLC receive csr bits.   These are status (rcsr read) */
/* event clears (rcsr writes) and receive masks (ORed into rimask) */
#define	PKTOK	0x80	/* Recieved a good packet */
#define	RSTP	0x10	/* Received a kiss of death STUPID BUG */
#define	SHRTP	0x08	/* Undersized */
#define	ALIGN	0x04	/* Alignment error */
#define	CRC	0x02	/* CRC error */
#define	OVF	0x01	/* DMA over run */
#define CLRRCSR (PKTOK|SHRTP|ALIGN|CRC|OVF)	/* Clear all errors, and ok */

/* ECLC xmode, ORed into xmode */
#define XIGNP	0x08	/* Ignore memory Parity */
#define TM	0x04	/* output signal TM */
#define LBC	0x02	/* not loop back: INVERTED FROM THE DOC */
#define XDC	0x01	/* disable contention: DANGEROUS */

/* EDLC rmode bits */
#define RNORM	0x2	/* All other bits =0, normal receive */
#define LOOSE	0x3	/* Promiscuous mode */

/* EDLC RESET function */
#define BEGINRESET	0x80	/* reset is a level */
#define ENDRESET	0x00
