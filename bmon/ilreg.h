#define ILMEMSIZE 8192
#define IL_NRBUFS  4
#define IL_NTBUFS  1

#define	ILRMINLEN	60	/* minimum frame length */
#define	ILRMAXLEN	1518	/* maximum frame length */
#define	ILRCRCLEN	4	/* length of CRC */
#define	ILRLEADER	0	/* bytes of leader not in frame length */

#define iloff(f)	((u_short)(f))
#define ilmem(o)	((u_char *)(o + mem))

#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

struct ildevice {
    u_char il_do : 1,
    	   il_done : 1,
	   il_cmd : 3,
	   il_inta : 3;
    u_char il_ca : 1,
    	   il_onl : 1,
	   il_swp : 1,
	   il_dmae : 1,
	   il_chpe : 1,
	   il_intb : 3;
    u_char skip[6];
#define il_addr un.addr
#define il_buf  un.dma.buf
#define il_mem  un.dma.mem
#define il_cnt  un.dma.cnt
    union {
	u_char addr[6];
	struct {
	    u_short buf;
	    u_char *mem;
	    u_short cnt;
	} dma;
    } un;
};

/*
 * here are the onboard memory data structures for the intel
 * 82586 chip.
 * these structures are in multibus byte swapped
 * format
 */
struct ilhead {
    u_short stat;
    u_short cmd;
    u_short link;
};

struct ilscp {
    u_char s0;
    u_char bus;
    u_char s1[4];
    u_char *iscp;
};

struct iliscp {
    u_char s0;
    u_char s1 : 7,
    	   busy : 1;
    u_short offset;
    u_char *base;
};

struct ilscb {
    u_char stat:4, cus:4;
    u_char rus:4, skip0:4;
    u_char ack:4, cuc:4;
    u_char ruc:4, skip1:4;
    u_short cbl;
    u_short rfa;
    u_short crcerr;
    u_short alnerr;
    u_short rscerr;
    u_short ovrerr;
};

struct ilcbl {
    u_short stat;
    u_short cmd;
    u_short link;
    u_char data[16];
};

struct ilfa {
    u_short stat;
    u_short cmd;
    u_short link;
    u_short bd;
};

struct ilbd {
    u_short stat;
    u_short next;
    u_char *addr;
    u_short size;
};

struct ilbuf {
    struct ilfa fa;
    struct ilbd bd;
    u_char buf[ILRMAXLEN];
};

struct ilconfigure {
    u_short stat;
    u_short cmd;
    u_short link;
    u_char  fifolim;
    u_char  bytecnt;
    u_char  loopbk:2,preamb:2,acloc:1,addlen:3;
    u_char  sync;
    u_char  if_spacing;
    u_char  prior;
    u_short retry:4, slottime:12;
    u_char  cdtsrc:1,cdtf:3,crssrc:1,crsf:3;
    u_char  pad:1,btstf:1,crc:1,ncrc:1,tono:1,mannrz:1,bcdis:1,prm:1;
    u_short minlen;
};


struct ilasetup {
    u_short stat;
    u_short cmd;
    u_short link;
    u_char addr[6];
};

#define ILCMD_IASETUP	0x1
#define ILCMD_CONFIG	0x2
#define ILCMD_XMIT	0x4
#define ILCMD_I		0x2000
#define ILCMD_S		0x4000
#define ILCMD_EL	0x8000

#define ILSTAT_A	0x1000
#define ILSTAT_OK	0x2000
#define ILSTAT_B	0x4000
#define ILSTAT_F	0x4000
#define ILSTAT_C	0x8000
#define ILSTAT_EOF	0x8000
#define ILSTAT_CNT	0x3fff

#define ILRUC_NOP     0x0
#define ILRUC_START   0x1
#define ILRUC_RESUME  0x2
#define ILRUC_SUSP    0x3
#define ILRUC_ABORT   0x4

#define ILRUS_IDLE  0x0
#define ILRUS_SUSP  0x1
#define ILRUS_NORES 0x2
#define ILRUS_RDY   0x4

#define ILCUC_START	0x1
#define ILCUC_RESUME	0x2
#define ILCUC_SUSP	0x3
#define ILCUC_ABORT	0x4

#define ILCUS_IDLE	0x0
#define ILCUS_SUSP	0x1
#define ILCUS_RDY	0x2

#define ILACK_CX  0x8
#define ILACK_FR  0x4
#define ILACK_CNR 0x2
#define ILACK_RNR 0x1

#define ILDMA_BTOMEM	0
#define ILDMA_BTODEV	1
#define ILDMA_BSWPMEM	2
#define ILDMA_BSWPDEV	3
#define ILDMA_WTOMEM	4
#define ILDMA_WTODEV	5
#define ILDMA_WSWPMEM	6
#define ILDMA_WSWPDEV	7

#define ILS_DMA		0xf
#define ILS_DMARCV	0x1
#define ILS_RCVERR	0x2
#define ILS_DMAXMT	0x4
#define ILS_XMTERR	0x8

/*
 * packet headers
 */
#define il_header ilpacket
struct ilpacket
{
    u_char  il_dhost[6];
    u_char  il_shost[6];
    u_short il_type;
};

#define ILRHEAD (sizeof (struct ilpacket))
#define ILXHEAD (sizeof (struct ilpacket))

/*
 *  Packet types
 */

#define	ILT_CTP		0x009	/* configuration testing protocol */
#define	ILT_IP		0x800	/* internet protocol */
#define	ILT_CHAOS	0x804	/* CHASOnet protocol */
#define	ILT_AR		0x806	/* address resolution protocol */
#define ilbcopy(from, to, n)	\
{				\
    reg->il_swp = 0;		\
    bcopy(from,to,n);		\
    reg->il_swp = 1;		\
}
