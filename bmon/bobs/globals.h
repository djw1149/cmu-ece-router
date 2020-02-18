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

struct globals {
    struct ttcb		*tt;
    struct config	*net;
    struct ipcb		*ip;
    struct udpcb	*udp;
    struct arcb		*ar;
    struct brcb		*br;
    struct boot		*boot;
    caddr_t		input;
    caddr_t		buf;
    caddr_t		heapp;
    struct autorom	*rom;
    char                gl_ledgoodcnt;
    char                gl_ledbadcnt;
    int			s_ram_pointer;		/* Start of protected ram. */
};

extern struct globals *globals;
