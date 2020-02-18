
/*  Copyright 1986 by Carnegie Mellon  */
/*  See permission and disclaimer notice in file "cmu-note.h"  */

/*
 * Bootstrap Protocol (BOOTP).  RFC 951.
 */

#define BOOTREPLY	(u_char)2
#define BOOTREQUEST	(u_char)1
struct bootp {
	u_char	bp_op;		/* packet opcode type */
	u_char	bp_htype;	/* hardware addr type */
	u_char	bp_hlen;	/* hardware addr length */
	u_char	bp_hops;	/* gateway hops */
	u_long	bp_xid;		/* transaction ID */
	u_short	bp_secs;	/* seconds since boot began */
	u_short	bp_unused;
	u_long	bp_ciaddr; /* client IP address */
	u_long	bp_yiaddr; /* 'your' IP address */
	u_long	bp_siaddr; /* server IP address */
	u_long	bp_giaddr; /* gateway IP address */
	u_char	bp_chaddr[16];	/* client hardware address */
	u_char	bp_sname[64];	/* server host name */
	u_char	bp_file[128];	/* boot file name */
	u_char	bp_vend[64];	/* vendor-specific area */
};

/*
 * UDP port numbers, server and client.
 */
#define	IPPORT_BOOTPS		67
#define	IPPORT_BOOTPC		68

/*
 * "vendor" data permitted for CMU boot clients.
 */
struct vend {
	u_char	v_magic[4];	/* magic number */
	u_long	v_flags;	/* flags/opcodes, etc. */
	u_long 	v_smask;	/* Subnet mask */
	u_long 	v_dgate;	/* Default gateway */
	u_long	v_dns1, v_dns2; /* Domain name servers */
	u_long	v_ins1, v_ins2; /* IEN-116 name servers */
	u_long	v_ts1, v_ts2;	/* Time servers */
	u_char	v_unused[25];	/* currently unused */
};

#define	VM_CMU		"CMU"	/* v_magic for CMU */

/* v_flags values */
#define VF_SMASK	1	/* Subnet mask field contains valid data */

#define MAXREQ		2	/* Maximum number of times to try a
request */

#define BACKINT		3	/* How long to back off, default */



