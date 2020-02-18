#define IL_ARRSIZE 1

#define	IL_MTU		(ILRMAXLEN-ILRHEAD-ILRCRCLEN-ILRLEADER)

#define	IL_HRD		0x1	/* ARP hardware address space */

#define	IL_HLN		6	/* hardware address length (bytes) */

#define	ILPLEN	(ILRHEAD+IL_MTU)/* maximum receive packet length */

struct ilcb {
    struct ilscb *scb;
    struct ilbuf *rbuf;
    struct ilbuf *lrbuf;
    struct ilbuf *tbuf;
    char *free;
    /* djw: killed- char *mem; */
};

extern struct config ilconfig;

struct ildevt {
    struct ildevice *baseaddr;
    long smaddr;
    long priority;	/* not used in bmon??? */
};
