#define EC_ARRSIZE 1

#define	EC_MTU		(ECRMAXLEN-ECRHEAD-ECRCRCLEN-ECRLEADER)

#define	EC_HRD		0x1	/* ARP hardware address space */

#define	EC_HLN		6	/* hardware address length (bytes) */

#define	ECPLEN	(ECRHEAD+EC_MTU)	/* maximum receive packet length */

struct eccb {
    u_short jam;
};

extern struct config ecconfig;

struct ecdevt {
     struct ecdevice *addr;
};