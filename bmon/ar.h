#define AR_ARRSIZE 4

struct arentry {
    struct in_addr ipaddr;
    char physaddr[6];
};

struct arcb {
    int current;
    struct arentry entry[AR_ARRSIZE];
};

/*
	nbytes: (ar$sha) Hardware address of sender of this
			 packet, n from the ar$hln field.
	mbytes: (ar$spa) Protocol address of sender of this
			 packet, m from the ar$pln field.
	nbytes: (ar$tha) Hardware address of target of this
			 packet (if known).
	mbytes: (ar$tpa) Protocol address of target.
*/

#define AR_SIZE		8
#define AR_REQUEST	0x1
#define AR_REPLY	0x2

#define	sha(p)	(p->addr)
#define	spa(p)	(p->addr+p->hln)
#define	tha(p)	(spa(p)+p->pln)
#define	tpa(p)	(tha(p)+p->hln)

struct ar_header {
	u_short	hrd;		/* hardware address space */
	u_short	pro;		/* Protocol to do addr resolution */
	u_char	hln;		/* length of hardware address */
	u_char	pln;		/* length of protocol address */
	u_short	op;		/* Request/reply opcode */
	u_char	addr[1];	/* Start of addresses (var len) */
};

#define AR_TRACE 0x1
