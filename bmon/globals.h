#define HEAPSIZE 4096

#define ar_var		(globals->ar)
#define net_var		(globals->net)
#define ip_var	 	(globals->ip)
#define udp_var		(globals->udp)
#define boot_var	(globals->boot)
#define	rom_var		(globals->rom)
#define mainstr		(globals->input)
#define mainbuf		((struct mbuf *)globals->buf)
#define ledgoodcnt      (globals->gl_ledgoodcnt)
#define ledbadcnt       (globals->gl_ledbadcnt)
#define confdev		(globals->gl_device)
#define bootppkt	(globals->bp)

struct globals {
    struct bootpb	*bp;
    struct ttcb		*tt;
    struct config	*net;
    struct ipcb		*ip;
    struct udpcb	*udp;
    struct arcb		*ar;
    struct brcb		*br;
    struct boot		*boot;
    struct autorom	*rom;
    caddr_t		input;
    caddr_t		buf;
    caddr_t		heapp;
    caddr_t		heap_end;
    char                gl_ledgoodcnt;
    char                gl_ledbadcnt;
    struct autoid	*gl_device;	/* actual device in use */
};

extern struct globals *globals;
