#define BO_ARRSIZE 1

#define BO_M68K		0x01

#define BO_TYPE		0x100
#define BO_HDRLEN	16
struct boothdr {
    u_short	type;
    u_short	hlen;
    u_short	mch;
    u_short	cksum;
    int		entry;
};

struct boot {
    char *adr;
    char *entry;
};



