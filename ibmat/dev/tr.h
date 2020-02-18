/*
 * IBM Token Ring Driver, 1986 Matt Mathis
 * Lifted from the RT driver for the same card.
 * This module gives router dependent (and TR independant) definitions 
 */

#define	TR_HLN		6
#define	TR_MTU		TRMAXDATA
#define	TR_HRD		0x1	/* ARP hardware address space */

#define	TRCRCLEN	0	/* bytes of transfered CRC */
#define	TRLEADER	0	/* bytes of transfered leader not in frame */
#define	TRMINDATA	10	/* BUG minimum data length */
#define	TRMAXDATA	1500	/* maximum data length (same as Enet) */
#define	TRMINLEN	(TRHEADLEN+TRMINDATA+TRCRCLEN+TRLEADER)
#define	TRMAXLEN	(TRHEADLEN+TRMAXDATA+TRCRCLEN+TRLEADER) /*Max buffer*/

/*
 * packet headers
 */
struct trpacket
{
    u_char  tr_pcf0;
    u_char  tr_pcf1;
    u_char  tr_dhost[TR_HLN];
    u_char  tr_shost[TR_HLN];
/*  u_char  tr_route[18]; */	/* deal with IBM routing info later */
    u_char  tr_dsap;
    u_char  tr_ssap;
    u_char  tr_llc_ctl;
};
/* COMPILER Work around: sizeof lies about odd sized objects */

#define TRHEADLEN 	17 /* should be (sizeof (struct trpacket)) */

/*
 *  Packet types
 */

#define	TRT_IP		0x06	/* internet protocol */
#define TRT_FAKEIP	0x0800	/* Just to be funky: use the Enet IP type */
#define	TRT_AR		0x99	/* address resolution protocol
					(BUG: Not yet a spec) */

/*
 * Token Ring device dependent fields.
 */

#define	TRDEVDEP
struct trdevdep {
	struct tr_scb *tr_scb;		/* the system control block */
        struct tr_ssb *tr_ssb;		/* the system status block */
	struct tr_list *tr_rlist;	/* receive list */
	struct tr_list *tr_xlist;	/* transmit list */
	struct tr_list *tr_clist;	/* transmit cleanup list */
	struct dma_chan dma;		/* the dma descriptor */
	u_short bia_addr;		/* addr of the bia within the adapt */
};

#define MAXTR 2		/* at most two adapters in one system */
