/*	udp.h	6.1	83/07/29	*/

#define UDP_ARRSIZE 4

struct upcb {
    struct in_addr his_ipaddr;
    u_short myport;
    u_short hisport;    
    u_short peerport;
    u_short active;
    int count;
    struct ipcb *ip;
    int (*input)();
};

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
struct ipovly {
	char *ih_next, *ih_prev;	/* for protocol sequence q's */
	u_char	ih_x1;			/* (unused) */
	u_char	ih_pr;			/* protocol */
	short	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

struct udphdr {
	u_short	uh_sport;		/* source port */
	u_short	uh_dport;		/* destination port */
	short	uh_ulen;		/* udp length */
	u_short	uh_sum;			/* udp checksum */
};

struct	udpiphdr {
	struct 	ipovly ui_i;		/* overlaid ip structure */
	struct	udphdr ui_u;		/* udp header */
};
#define	ui_next		ui_i.ih_next
#define	ui_prev		ui_i.ih_prev
#define	ui_x1		ui_i.ih_x1
#define	ui_pr		ui_i.ih_pr
#define	ui_len		ui_i.ih_len
#define	ui_src		ui_i.ih_src
#define	ui_dst		ui_i.ih_dst
#define	ui_sport	ui_u.uh_sport
#define	ui_dport	ui_u.uh_dport
#define	ui_ulen		ui_u.uh_ulen
#define	ui_sum		ui_u.uh_sum
