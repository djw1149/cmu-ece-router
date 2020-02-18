/*****************************************************************
 * Shaired network statistics structure
 *
 * devstats holds all of the counters and continous statistics (Queue lengths)
 * for a given device.
 */
#define XIC_MAX	 17	/* redefined here to avoid circular dependencys */
#define XIPOP_MAX 10	/* from icmp.h and ip.h */
struct devstats
{
		/* Driver statistics */
    long    dr_unsol;		/* unsolicited interupts */
    long    dr_rnerr;		/* network receive error (checksum etc) */
    long    dr_rierr;		/* internal driver problem (dma overun etc) */
    long    dr_rcnt;		/* sucessfully received packets */
    long    dr_rdrop;		/* discarded: no queue space **RESOURCE** */
    long    dr_rbcast;		/* received broadcasts */
    long    dr_rlen;		/* discarded: bad length */
    long    dr_rproto;		/* discarded: unknown protocol type */
    long    dr_xcnt;		/* queued transmitted packets */
    long    dr_xnerr;		/* transmit errors attributed to the net*/
    long    dr_xierr;		/* transmit errors attr. to the router (us) */
    long    dr_jcnt;		/* retrys due to jams */
    long    dr_rq;		/* receive queue */
    long    dr_rqmax;		/* receive queue max */
    long    dr_xq;		/* transmit queue */
    long    dr_xqmax;		/* transmit queue max */

    long    dr_spare1;
    long    dr_spare2;

		/* CTP statistics */
    long    ct_rcnt;		/* currently discarded */
    long    ct_xcnt;		/* future */
    long    ct_spare1;
    long    ct_spare2;

		/* ARP statistics */
    long    ar_rmin;		/* illegal length */
    long    ar_rhrd;		/* illegal hardware space */
    long    ar_rpro;		/* illegal (usupported) target protocol */
    long    ar_rhln;		/* illegal hardware address length */
    long    ar_rfraud;		/* illegal source HW address (!= actual) */
    long    ar_rmine;		/* discard my own broadcasts */
    long    ar_rcnt;		/* processed ARP */
    long    ar_runchanged;	/* source is unchanged */
    long    ar_runknown;	/* source is new */
    long    ar_rchanged;	/* source moved */
    long    ar_rlockerr;	/* source moved too quickly (a loop ?) */
    long ar_rng_xpp;            /* pp request sent from ring */
    long ar_rng_unm;            /* entry unmapped because of pp failure */
    long ar_rng_alru;           /* entry added to lru queue from ring */
    long ar_lru_used;           /* addmap entry in lru queue used */
    long ar_xcnt;		/* transmitted ARP packets */
    long ar_xcast;		/* transmitted ARP broadcast packets */
    long ar_xdrop;		/* transmitted ARP packets dropped */
    long ar_xppreq;             /* transmitted point to point requests */
    long ar_req_me;             /* recieved ARP request destination me */
    long ar_req_bc_dnc;         /* recieved bcast req. dest addr not known */
    long ar_req_bc_smv;         /* recieved bcast req. source moved */
    long ar_req_bc_rbc;         /* recieved bcast req. dest REQBCAST */
    long ar_req_bc_sndn;        /* recieved bcast req. src net != dest net*/
    long ar_req_bc_rem;         /* recieved bcast req. misc. */
    long ar_req_pp_dnc;         /* recieved pp req. dest not known */
    long ar_req_pp_sndn;        /* recieved pp req. src net != dest net */
    long ar_req_pp_rem;         /* recieved pp req. misc. */
    long ar_rep_me;             /* recieved ARP request destination me */
    long ar_rep_bc;             /* recieved bcast reply */
    long ar_rep_pp_dnc;         /* recieved pp reply dest addr not known */
    long ar_rep_pp_for;         /* recieved pp reply forwarded */
    long    ar_spare1;
    long    ar_spare2;

		/* chaos net statistics */
    long    cn_rcnt;		/* all discarded */
    long    cn_rrut;
    long    cn_xcnt;
    long    cn_xdrop;
    long    cn_tome;
    long    cn_spare1;

		/* XPUP statisitcs */
    long    xp_rcnt;		/* all discarded */
    long    xp_spare1;
    long    xp_spare2;

		/* IP statistics */
    long    ip_bcast;	/* received IP broadcast packets */
    long    ip_rvers;	/* received packets with wrong version number */
    long    ip_rhovfl;	/* received packets with an IP header larger than */
			/*  the physical length of the packet */
    long    ip_rlen;	/* received packets with an IP length larger than */
			/*  the physical length of the packet */
    long    ip_rcksm;	/* received packets with an incorrect IP checksum */
    long    ip_rttl;	/* received packets whos time-to-live has expired */
    long    ip_rcnt;	/* received valid IP packets */
    long    ip_popt;	/* packets with options */
    long    ip_nopt;	/* total number of options */
    long    ip_ocnt[XIPOP_MAX+1];
			/* parsed option counts (index by option number) */
    long    ip_oerr;	/* received packets with bad option lists */
    long    ip_osr;	/* received source routes through us */
    long    ip_ofrroute;	/* full record route options ignored */
    long    ip_oftstamp;	/* full time stamp options ignored */
    long    ip_onet;	/* Added routing entries to other nets */
    long    ip_unreach;	/* unreachable destination */
    long    ip_tome;	/* Addressed to me, may be source routed */
    long    ip_rproto;	/* packets addressed to me of unsupported protocol */
    long    ip_togway;	/* routed to a gateway */
    long    ip_xcast;	/* unknown address converted packet to ar bcast */
    long    ip_troll;	/* Traffic that didn't pass the troll */
    long    ip_redir;	/* converted to redirects */
    long    ip_frag;	/* messages sucessfully fragmented on output */
    long    ip_dfrag;	/* received messages which wouldve been fragmented */
			/*  if "dont fragment" had not been specfied */
    long    ip_cfrag;	/* received messages which could not be fragmented */
			/*  due to resource limitations (out of packets) */
    long    ip_fdrop;	/* trailing fragments dropped on output because of */
			/*  problems delivering them to the local network */
    long    ip_xcnt;	/* transmitted valid IP packets */
    long    ip_spare1;
    long    ip_spare2;

		/* ICMP statistics */
    long    ic_rmin;
    long    ic_rcksm;
    long    ic_rcnt[XIC_MAX + 1];
    long    ic_xcnt[XIC_MAX + 1];
    long    ic_spare;

		/* UDP statistics */
    long    ud_rmin;
    long    ud_rcksm;
    long    ud_rcnt;
    long    ud_rproto;
    long    ud_xcnt;
    long    ud_spare;

		/* RCP statistics */
    long    rc_rmin;
    long    rc_rcnt;
    long    rc_rtype;
    long    rc_xcnt;
    long    rc_xerr;
    long    rc_get;
    long    rc_put;
    long    rc_auth;
    long    rc_unauth;
    long    rc_reboot;
    long    rc_invalid;
    long    rc_spare1;
};

/*****************************************************************
 * devinfo is used to by rcp info service to get everything known about a
 * 	given device
 */
struct devinfo
{
		/* configuration information */
    long    dvc_br;
    long    dvc_nunit;
    long    dvc_istatus;
    long    dvc_dstate;
    long    dvc_cable;
    long    dvc_devtype;
    long    dvc_spare;

    char    dvc_phys[MAXHWADDRL];
    char    dvc_praddr[PRL_ALL];

    struct devstats ds;		/* all the statistics */
};

