/*****************************************************************
 *
 * $Header: showdevcnt.c,v 1.3 86/08/31 17:20:54 chriss Exp $
 *
 * file : showdevcnt.c
 *
 *	Show the device statistics (event counts) in various different
 *		formats
 *
 *	All argument usages are the same:
 *		router: is the routers name, to be used for titles only
 *		card: Which interface, 0 means total
 *		rdev[]: The array of device statistics structutures
 *			rdev[0] is the total, rdev[1] the 1st card etc.
 *		verbose: level of verbosity (0,1,2...)
 *		first: flag to force a title banner
 * HISTORY
 *	started 10/14/85 by Matt Mathis
 *		Combined several files into one, standardized formats etc.
 *	Written by Kevin Kirmse summer of '85
 *****************************************************************
 */
#include "./h/rcpinfo.h"
#include "../h/proto.h"
#include "./h/rtypes.h"
#include "../h/devstats.h"
/*****************************************************************
 *	Show driver (Link level) level statistics
 *
 */
rcp_drinfo(router, card, rdev, verbose, first)
char *router;
int card, verbose, first;
struct devinfo *rdev;
{
    int     cnt;

    if (first && verbose > 0) {
	printf ("  card|  ");
	printf ("cable|  ");
	printf (" IP address ");
	printf ("     EN address ");
	printf ("         card type  ");
	printf ("\n");
	for (cnt = 1; cnt <= numdev; cnt++) {
	    printf ("%6d| ", cnt);
	    printf ("%6x| ", rdev[cnt].dvc_cable);
	    printf ("  ");
	    print_ip (rdev[cnt].dvc_praddr);
	    printf (" ");
	    print_en (rdev[cnt].dvc_phys);
	    printf ("   ");
	    devswt (rdev[cnt].dvc_devtype);
	    printf ("\n");
	}
	printf ("\n");
    }
    if (first) {
	printf ("C| rQm| xQm|rnerr|     rcnt| rdrop| rbcast|rproto|     xcnt|xnerr|  jcnt\n");
    }

    if (card) {
	printf ("%1d|", card);
    }
    else {
	printf ("T|");;
    }
    printf ("%1d%3d|", rdev[card].ds.dr_rq,rdev[card].ds.dr_rqmax);
    printf ("%1d%3d|", rdev[card].ds.dr_xq,rdev[card].ds.dr_xqmax);
    printf ("%5d|", rdev[card].ds.dr_rnerr);
    printf ("%9d|", rdev[card].ds.dr_rcnt);
    printf ("%6d|", rdev[card].ds.dr_rdrop);
    printf ("%7d|", rdev[card].ds.dr_rbcast);
    printf ("%6d|", rdev[card].ds.dr_rproto);
    printf ("%9d|", rdev[card].ds.dr_xcnt);
    printf ("%5d|", rdev[card].ds.dr_xnerr);
    printf ("%6d|", rdev[card].ds.dr_jcnt);
    printf ("\n");

    if (rdev[card].ds.dr_unsol | rdev[card].ds.dr_rierr |
	rdev[card].ds.dr_rlen  | rdev[card].ds.dr_xierr)
	    printf("unsol=%d, rierr=%d, rlen=%d, xierr=%d\n",
		rdev[card].ds.dr_unsol, rdev[card].ds.dr_rierr,
		rdev[card].ds.dr_rlen, rdev[card].ds.dr_xierr);
}

/*****************************************************************
 *	Show the device type
 *
 */
devswt(tmp)
int tmp;
{   
    switch(tmp) {
       case DVT_3ETHER : printf("3MB ethernet    ");
                         break;
       case DVT_IL10   : printf("10MB Interlan   ");
                         break;
       case DVT_DZ11   : printf("DZ-11 serial    "); 
                         break;
       case DVT_LHDH11 : printf("ACC LH/DH-11 IMP"); 
                         break;
       case DVT_DTE    : printf("DTE-20          "); 
                         break;
       case DVT_DA28   : printf("DA28-F          "); 
                         break;
       case DVT_PRONET : printf("ProNET 10MB ring"); 
                         break;
       case DVT_DUP11  : printf("DUP11 sync line "); 
                         break;
       case DVT_3COM10 : printf("10MB 3Com       "); 
                         break;
       case DVT_DEUNA  : printf("10MB DEUNA      "); 
                         break;
       case DVT_APBUS  : printf("AppleBus net    "); 
                         break;
       default:          printf("Unknown device  ");
    }
}

print_ip(addr)
unsigned char addr[]; 
{
     printf("[%d.%d.%d.%d]",addr[0],addr[1],addr[2],addr[3]);

}

print_en(addr)
unsigned char addr[]; 
{
    printf ("[%x.%x.%x.%x.%x.%x]",
	    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}
/*****************************************************************
 *	Show ARP statistics
 *
 */
rcp_arcnt(router,card,rdev,v,first)
char *router;
int card,v,first;
struct devinfo *rdev;

{
    int tpp;			/* temp to fix compiler table space problem */

    if (!v) {			/* short form */
	if (first) {
	    printf ("    Rtot   Rbc   Rme  Thru   Xpp   Xbc  Xtot\n");
	}
	if (card) {
	    printf("%1d",card);
	}
	else
	    printf("T");
	tpp = rdev[card].ds.ar_xppreq + rdev[card].ds.ar_req_me;
	printf ("  %5d %5d %5d %5d %5d %5d %5d\n",
		rdev[card].ds.ar_rcnt,
		rdev[card].ds.ar_req_bc_dnc + rdev[card].ds.ar_req_bc_smv +
		rdev[card].ds.ar_req_bc_rbc + rdev[card].ds.ar_req_bc_sndn +
		rdev[card].ds.ar_req_bc_rem,
		rdev[card].ds.ar_req_me + rdev[card].ds.ar_rep_me,
		rdev[card].ds.ar_req_pp_dnc + rdev[card].ds.ar_req_pp_sndn +
		rdev[card].ds.ar_req_pp_rem + rdev[card].ds.ar_rep_pp_dnc +
		rdev[card].ds.ar_rep_pp_for,
		tpp,
		rdev[card].ds.ar_xcnt - tpp,
		rdev[card].ds.ar_xcnt);
    }
    else {			/* verbose form */
	printf ("\n");
	if (card != 0) {
	    printf ("---- arcnt ----[%s] card %d ----\n", router, card);
	}
	else {
	    printf ("---- arcnt ----[%s] totals  ----\n", router);
	}
	if (rdev[card].ds.ar_rmin || v > 1) {
	    printf ("received sub-minimum packets: ");
	    printf ("\t\t\t\t%d\n", rdev[card].ds.ar_rmin);
	}
	if (rdev[card].ds.ar_rhrd || v > 1) {
	    printf ("recieved unknown hardware space packets: ");
	    printf ("\t\t%d\n", rdev[card].ds.ar_rhrd);
	}
	if (rdev[card].ds.ar_rhln || v > 1) {
	    printf ("recieved bad hardware length packets: ");
	    printf ("\t\t\t%d\n", rdev[card].ds.ar_rhln);
	}
	if (rdev[card].ds.ar_rpro || v > 1) {
	    printf ("recieved unknown protocol packets: ");
	    printf ("\t\t\t%d\n", rdev[card].ds.ar_rpro);
	}
	if (rdev[card].ds.ar_rfraud || v > 1) {
	    printf ("recieved packets with incorrect hardware address: ");
	    printf ("\t%d\n", rdev[card].ds.ar_rfraud);
	}
	if (rdev[card].ds.ar_rmine || v > 1) {
	    printf ("recieved our own broadcast packets: ");
	    printf ("\t\t\t%d\n", rdev[card].ds.ar_rmine);
	}
	if (rdev[card].ds.ar_xdrop || v > 1) {
	    printf ("transmitted ARP packets dropped: ");
	    printf ("\t\t\t%d\n", rdev[card].ds.ar_xdrop);
	}

	printf ("Source stats");
	printf (":   total|  ");
	printf ("      new|  ");
	printf ("  changed|  ");
	printf ("unchanged|  ");
	printf ("  lockerr|  ");
	printf ("\n");
	printf ("            ");
	printf ("%9d|  ", rdev[card].ds.ar_rcnt);
	printf ("%9d|  ", rdev[card].ds.ar_runknown);
	printf ("%9d|  ", rdev[card].ds.ar_rchanged);
	printf ("%9d|  ", rdev[card].ds.ar_runchanged);
	printf ("%9d|  ", rdev[card].ds.ar_rlockerr);
	printf ("\n");

	printf ("Response: ");
	printf ("  total |");
	printf ("  drop |");
	printf ("   pps |");
	printf ("   ppo |");
	printf ("   dnk |");
	printf ("   smv |");
	printf ("   rbc |");
	printf ("\n");
	printf ("request_me");
	printf ("%7d |", rdev[card].ds.ar_req_me);
	printf ("   --- |");
	printf ("%6d |", rdev[card].ds.ar_req_me);
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("\n");
	printf ("request_bc");
	printf ("%7d |", rdev[card].ds.ar_req_bc_dnc +
		rdev[card].ds.ar_req_bc_smv +
		rdev[card].ds.ar_req_bc_rbc +
		rdev[card].ds.ar_req_bc_sndn +
		rdev[card].ds.ar_req_bc_rem);
	printf ("   --- |");
	printf ("%6d |", rdev[card].ds.ar_req_bc_rem);
	printf ("%6d |", rdev[card].ds.ar_req_bc_sndn);
	printf ("%6d |", rdev[card].ds.ar_req_bc_dnc);
	printf ("%6d |", rdev[card].ds.ar_req_bc_smv);
	printf ("%6d |", rdev[card].ds.ar_req_bc_rbc);
	printf ("\n");
	printf ("request_pp");
	printf ("%7d |",
		rdev[card].ds.ar_req_pp_dnc +
		rdev[card].ds.ar_req_pp_sndn +
		rdev[card].ds.ar_req_pp_rem);
	printf ("%6d |", rdev[card].ds.ar_req_pp_dnc);
	printf ("%6d |", rdev[card].ds.ar_req_pp_rem);
	printf ("%6d |", rdev[card].ds.ar_req_pp_sndn);
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("\n");
	printf ("reply_me  ");
	printf ("%7d |", rdev[card].ds.ar_rep_me);
	printf ("%6d |", rdev[card].ds.ar_rep_me);
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("\n");
	printf ("reply_bc  ");
	printf ("%7d |", rdev[card].ds.ar_rep_bc);
	printf ("%6d |", rdev[card].ds.ar_rep_bc);
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("\n");
	printf ("reply_pp  ");
	printf ("%7d |", rdev[card].ds.ar_rep_pp_dnc +
		rdev[card].ds.ar_rep_pp_for);
	printf ("%6d |", rdev[card].ds.ar_rep_pp_dnc);
	printf ("   --- |");
	printf ("%6d |", rdev[card].ds.ar_rep_pp_for);
	printf ("   --- |");
	printf ("   --- |");
	printf ("   --- |");
	printf ("\n");

	printf ("ring stats ");
	printf (" xmit pp | ");
	printf ("   unmap | ");
	printf ("added lru| ");
	printf ("lru_used | ");
	printf ("\n");
	printf ("           ");
	printf ("%8d | ", rdev[card].ds.ar_rng_xpp);
	printf ("%8d | ", rdev[card].ds.ar_rng_unm);
	printf ("%8d | ", rdev[card].ds.ar_rng_alru);
	printf ("%8d | ", rdev[card].ds.ar_lru_used);
	printf ("\n");
	printf ("ip -> arp bcasts : %d\n",rdev[card].ds.ip_xcast);
	printf ("ARP transmit statistics, (from all sources)\n");
	printf ("transmitted PP        %d\n", rdev[card].ds.ar_xppreq +
			rdev[card].ds.ar_req_me);
	printf ("Total transmitted ARP %d\n", rdev[card].ds.ar_xcnt);
    }
}

/*****************************************************************
 *	Show IP statistics
 *
 * TODO: Add ICMP information
 * TODO: Add counters to document IP protocol events (options, fragmentation)
 */
rcp_ipcnt(router, card, rdev, verbose, first)
char *router;
int card, verbose, first;
struct devinfo *rdev;
{
    int icrt,icxt,cnt;		/* to sum ICMP traffic */

    if (first) {

	printf ("card|");
	printf ("  ip recv|");
	printf (" to me|");
	printf (" rUDP |");
	printf (" xUDP |");
	printf (" rICMP|");
	printf (" xICMP|");
	printf ("n rte|");
	printf (" to GW|");
	printf ("  ip xmit|");
	printf (" xARP");
	printf ("\n");
    }
    for (icrt=icxt=cnt=0;cnt<=XIC_MAX+1;cnt++){
	icrt+=rdev[card].ds.ic_rcnt[cnt];
	icxt+=rdev[card].ds.ic_xcnt[cnt];
    }

    if (card) {
	printf ("%4d|", card);
    }
    else {
	printf ("   T|");;
    }
    printf ("%8d |", rdev[card].ds.ip_rcnt);
    printf ("%5d |", rdev[card].ds.ip_tome);
    printf ("%5d |", rdev[card].ds.ud_rcnt);
    printf ("%5d |", rdev[card].ds.ud_xcnt);
    printf ("%5d |", icrt);
    printf ("%5d |", icxt);
    printf ("%4d |", rdev[card].ds.ip_onet);
    printf ("%5d |", rdev[card].ds.ip_togway);
    printf ("%8d |", rdev[card].ds.ip_xcnt);
    printf ("%5d", rdev[card].ds.ip_xcast);
    printf ("\n");

	if (rdev[card].ds.ip_bcast || verbose>0) {
	    printf ("ip broadcasts: ");
	    printf ("%d\n", rdev[card].ds.ip_bcast);
	}
	if (rdev[card].ds.ip_rvers || verbose>0) {
	    printf ("wrong version ip: ");
	    printf ("%d\n", rdev[card].ds.ip_rvers);
	}
	if (rdev[card].ds.ip_rhovfl || verbose>0) {
	    printf ("ip header longer than physical packet length: ");
	    printf ("%d\n", rdev[card].ds.ip_rhovfl);
	}
	if (rdev[card].ds.ip_rlen || verbose>0) {
	    printf ("ip length longer than physical packet length: ");
	    printf ("%d\n", rdev[card].ds.ip_rlen);
	}
	if (rdev[card].ds.ip_rcksm || verbose>0) {
	    printf ("recieved packets with incorrect checksums: ");
	    printf ("%d\n", rdev[card].ds.ip_rcksm);
	}
	if (rdev[card].ds.ip_rttl || verbose>0) {
	    printf ("recieved packets who's time-to-live has expired: ");
	    printf ("%d\n", rdev[card].ds.ip_rttl);
	}
	if (rdev[card].ds.ip_popt || verbose>0) {
	    printf ("packets with options: ");
	    printf ("%d\n", rdev[card].ds.ip_popt);
	}
	if (rdev[card].ds.ip_nopt || verbose>0) {
	    printf ("total options: ");
	    printf ("%d\n", rdev[card].ds.ip_nopt);
	}
	if (rdev[card].ds.ip_oerr || verbose>0) {
	    printf ("recieved packets with bad option errors: ");
	    printf ("%d\n", rdev[card].ds.ip_oerr);
	}
	if (rdev[card].ds.ip_osr || verbose>0) {
	    printf ("recieved source routes through us: ");
	    printf ("%d\n", rdev[card].ds.ip_osr);
	}
	if (rdev[card].ds.ip_ofrroute || verbose>0) {
	    printf ("full record route options ignored: ");
	    printf ("%d\n", rdev[card].ds.ip_ofrroute);
	}
	if (rdev[card].ds.ip_oftstamp || verbose>0) {
	    printf ("full time stamp options ignored: ");
	    printf ("%d\n", rdev[card].ds.ip_oftstamp);
	}
	if (rdev[card].ds.ip_unreach || verbose>0) {
	    printf ("unreachable destination: ");
	    printf ("%d\n", rdev[card].ds.ip_unreach);
	}
	if (rdev[card].ds.ip_rproto || verbose>0) {
	    printf ("recieved packets for incorrect protocol: ");
	    printf ("%d\n", rdev[card].ds.ip_rproto);
	}
	if (rdev[card].ds.ip_troll || verbose>0) {
	    printf ("rejected by the troll: ");
	    printf ("%d\n", rdev[card].ds.ip_troll);
	}
	if (rdev[card].ds.ip_redir || verbose>0) {
	    printf ("converted to redirects: ");
	    printf ("%d\n", rdev[card].ds.ip_redir);
	}
	if (rdev[card].ds.ip_frag || verbose>0) {
	    printf ("successfully fragmented on output: ");
	    printf ("%d\n", rdev[card].ds.ip_frag);
	}
	if (rdev[card].ds.ip_dfrag || verbose>0) {
	    printf ("recieved messages which would have been fragmented 'flag': ");
	    printf ("%d\n", rdev[card].ds.ip_dfrag);
	}
	if (rdev[card].ds.ip_cfrag || verbose>0) {
	    printf ("recieved messages which would have been fragmented 'res': ");
	    printf ("%d\n", rdev[card].ds.ip_cfrag);
	}
	if (rdev[card].ds.ip_fdrop || verbose>0) {
	    printf ("trailing fragments drop due to net problems: ");
	    printf ("%d\n", rdev[card].ds.ip_fdrop);
	}
	if (rdev[card].ds.ic_rmin || verbose>0) {
	    printf ("icmp packets that are too small: ");
	    printf ("%d\n", rdev[card].ds.ic_rmin);
	}
	if (rdev[card].ds.ic_rcksm || verbose>0) {
	    printf ("icmp checksum errors: ");
	    printf ("%d\n", rdev[card].ds.ic_rcksm);
	}
	if (rdev[card].ds.ud_rmin || verbose>0) {
	    printf ("recieved UDP packets with below legal length: ");
	    printf ("%d\n", rdev[card].ds.ud_rmin);
	}
	if (rdev[card].ds.ud_rcksm || verbose>0) {
	    printf ("recieved UDP packets with checksum errors:");
	    printf ("%d\n", rdev[card].ds.ud_rcksm);
	}
	if (rdev[card].ds.ud_rproto || verbose>0) {
	    printf ("recieved UDP packets with protocol violations: ");
	    printf ("%d\n", rdev[card].ds.ud_rproto);
	}

}
/*****************************************************************
 *	Show a general traffic summary
 */
rcp_trcnt(router, card, rdev, verbose, first)
char *router;
int card, verbose, first;
struct devinfo *rdev;
{
    int     cnt;

    if (first && verbose > 0) {

    }
    if (first) {
	printf ("card|");
	printf (" raw recv|");
	printf (" raw xmit|");
	printf ("   ARP cnt   |");
	printf ("      IP cnt     |");
	printf ("   loss    |");
	printf ("\n");
    }
    if (card) {
	printf ("%4d|", card);
    }
    else {
	printf ("   T|");;
    }
    printf ("%8d |", rdev[card].ds.dr_rcnt);
    printf ("%8d |", rdev[card].ds.dr_xcnt);
    printf ("%6d/%-6d|", rdev[card].ds.ar_rcnt, rdev[card].ds.ar_xcnt);
    printf ("%8d/%-8d|", rdev[card].ds.ip_rcnt, rdev[card].ds.ip_xcnt);
    printf ("%5d/%-5d|",
	    (rdev[card].ds.dr_rcnt - (rdev[card].ds.ip_rcnt + rdev[card].ds.ar_rcnt)),
	    ((rdev[card].ds.ip_xcnt + rdev[card].ds.ar_xcnt) - rdev[card].ds.dr_xcnt));
    printf ("\n");
}
