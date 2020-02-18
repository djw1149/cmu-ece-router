#define	ECRMINLEN	64	/* minimum frame length */
#define	ECRMAXLEN	1518	/* maximum frame length */
#define	ECRCRCLEN	4	/* length of CRC */
#define	ECRLEADER	0	/* bytes of leader not in frame length */

/*
 * 3Com Multibus Ethernet interface registers.
 */

#define MIN(a,b) (a < b ? a : b)
#define MAX(a,b) (a > b ? a : b)

struct ec_csr {
    u_short	csr_bbsw:1,		/* receive packet in b */
		csr_absw:1,		/* receive packet in a */
		csr_tbsw:1,		/* transmit packet */
		csr_jam:1,		/* jam: status and clear */
		csr_amsw:1,		/* set station address */
    		csr_rbba:1,		/* receive status */
    		csr_xxx:1,		/* not used */
		csr_reset:1,		/* reset interface */
		csr_binten:1,		/* enable receive b interrupts */
		csr_ainten:1,		/* enable receive a interrupts */
		csr_tinten:1,		/* enable transmit interrupts */
    		csr_jinten:1,		/* enable jam interrupts */
    		csr_pa:4;		/* packet address options */
};

struct ecdevice {
    struct ec_csr	ec_csr;		/* control & status */
    u_short		ec_back;	/* retransmit backoff */
    u_char		ex_xxx[1020];	/* not used */
    u_char		ec_arom[512];	/* station address rom */
    u_char		ec_aram[512];	/* station address ram */
    u_char		ec_tbuf[2048];	/* transmit buffer */
    u_char		ec_rbufa[2048];	/* receive buffer A */
    u_char		ec_rbufb[2048];	/* receive buffer B */
};

#define	SET_BIT(b)	b = 1
#define	CLEAR_BIT(b)	b = 0

/* packet reception classes */
#define	EC_PA_ALL			0
#define	EC_PA_ALL_NERR			1
#define	EC_PA_ALL_NFCS_NFRM		2
#define	EC_PA_MINE_MULT			3
#define	EC_PA_MINE_MULT_NERR		4
#define	EC_PA_MINE_MULT_NFCS_NFRM	5
#define	EC_PA_MINE_BROAD		6
#define	EC_PA_MINE_BROAD_NERR		7
#define	EC_PA_MINE_BROAD_NFCS_NFRM	8

/*
 * packet headers
 */
struct ecpacket
{
    u_char  ec_dhost[6];
    u_char  ec_shost[6];
    u_short ec_type;
};

#define	ec_header ecpacket
#define ECRHEAD (sizeof (struct ecpacket))
#define ECXHEAD (sizeof (struct ecpacket))

/*
 *  Packet types
 */

#define	ECT_CTP		0x009	/* configuration testing protocol */
#define	ECT_IP		0x800	/* internet protocol */
#define	ECT_CHAOS	0x804	/* CHASOnet protocol */
#define	ECT_AR		0x806	/* address resolution protocol */
