/*
 *  Device interface definitions
 *
 **********************************************************************
 * HISTORY
 * 18-Nov-85 Matt Mathis (mathis) at CMU ECE dept
 *	Changed to use a single statistics structure
 *
 * 18-Jun-85  David Waitzman (djw) at Carnegie-Mellon University
 *	Added definitions for AppleBus.
 *
 * 27-Aug-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Reorganized device structure to eliminate all device dependent fields
 *	from the common declarations (moving them instead to a new device
 *	dependent area) and to avoid allocating the unneeded network device
 *	fields for support devices.
 *
 * 08-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added transmit/receive busy and interrupt in progress status
 *	bit definitions.
 *
 * 28-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added EGP statistics pointer to device structure.
 *
 * 25-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added definitions for DUP11, 3Com 10Mb ethernet and DEUNA
 *	10Mb ethernet interfaces.
 *
 * 02-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added dv_nunit field (to support multiple unit devices
 *	like the DZ-11).
 *
 * 02-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#ifndef	D_DEVICE
#define	D_DEVICE

#ifndef	VAXCOM
typedef	struct addmap
		 *(*pf_addmap)();/* the real thing */
#else	VAXCOM
typedef	VAXCOM	    pf_addmap;	/* external (e.g. VAX) inclusion */
#endif	VAXCOM

typedef struct config * p_config;		/* needed below */

struct config
{
    pf_int	     cf_reset;		/* driver reset routine */
    pf_int	     cf_output;		/* driver output routine */
    pf_int	     cf_diag;		/* driver diagnosis routine */
    pf_addmap	     cf_ar;		/* driver address resolution routine */
    short	     cf_pr[NPR];	/* protocol type words */
    short	     cf_hrd;		/* address resolution hardware space */
    short	     cf_flag;		/* driver dependent flags */
    short	     cf_head;		/* packet header length (bytes) */
    short	     cf_trail;		/* packet trailer length (bytes) */
    short	     cf_slop;		/* packet slop (bytes) */
    short	     cf_mtu;		/* maximum transmission unit (bytes) */
    char	     cf_hln;		/* hardware address length (bytes) */
    char	     cf_bcast[MAXHWADDRL];/* hardware broadcast address */
    char	     cf_type;		/* device type */
    short	     cf_ddlen;		/* device dependent fields length */
};


/*
 *  All of the following type definitions are needed below.  They are declared
 *  as separate types so that they may be redefined when included in host
 *  programs where the pointer size will not be the same (e.g. the VAX).
 */

typedef struct devstats * p_devstats;

/*
 *  General purpose device structure defintion.  This structure is shared by
 *  both support and network devices.  For support devices space for only the
 *  first common fields is allocated.  For network devices space for the
 *  remaining fields plus any device dependent state is also allocated.
 *
 *  N.B.  Avoid use of the "sizeof" construct with aruments of this structure
 *  type.  It is almost certainly not what you intend.
 */


struct device
{
    /*
     *  The following fields are common to all devices:
     */
    struct
    {
        p_device  dvc_next;		/* next device in chain */
        p_char	  dvc_addr;		/* device CSR address */
        int	  dvc_nunit;		/* number of units on this controller */
        int	  dvc_istatus;		/* device independent status */
        int	  dvc_dstate;		/* device dependent state */
	p_char	  dvc_mem;
	p_inter	  *dvc_pb;
    } dv_common;

    /*
     *  The following fields are defined only for network devices:
     */
    p_device	  dv_prnext[NPRLINK];	/* next protocol "spliced" device */
					/*   in chain */ 
    char	  dv_phys[MAXHWADDRL];	/* physical device hardware address */
    char	  dv_praddr[PRL_ALL];	/* logical device protocol addresses */
    char	  dv_restr[NPRLINK];	/* restricted device flags */
    struct config dv_cf;		/* device characteristics and */
					/*   configuration information copied
					/*   at autoconfiguration time  */
    int		  dv_cable;		/* cable number */
    p_packet	  dv_rp;		/* current receive packet */
    p_packet	  dv_xp;		/* current transmit packet */
    struct queue  dv_rq;		/* receive packet queue */
    struct queue  dv_xq;		/* transmit packet queue */

    p_devstats	  dv_cnts;		/* all driver and protocol stats */


    /*
     *  The remainder of this structure is device dependent and is provided by
     *  the relevant device include files when those modules are compiled.
     *  Each driver which maintains device dependent state information defines
     *  the appropriate conditional compilation symbol before including this
     *  structure definition.  The individual variants are defined in this way
     *  in order to avoid dependencies on the device driver modules when not
     *  including them.
     */
    union				/* device dependendent fields */
    {
#ifdef	DADEVDEP
	struct dadevdep dvu_da;		/* DA28-F interprocessor link */
#endif	DADEVDEP
#ifdef	DTDEVDEP
	struct dtdevdep dvu_dt;		/* DTE-20 */
#endif	DTDEVDEP
#ifdef	DUDEVDEP
	struct dudevdep dvu_du;		/* DUP11 */
#endif	DUDEVDEP
#ifdef	DZDEVDEP
	struct dzdevdep dvu_dz;		/* DZ11 */
#endif	DZDEVDEP
#ifdef	ECDEVDEP
	struct ecdevdep dvu_ec;		/* 3Com 10Mb ethernet */
#endif	ECDEVDEP
#ifdef	ENDEVDEP
	struct endevdep dvu_en;		/* 3Mb ethernet */
#endif	ENDEVDEP
#ifdef	ILDEVDEP
	struct ildevdep dvu_il;		/* Interlan 10Mb ethernet */
#endif	ILDEVDEP
#ifdef	ABDEVDEP
	struct abdevdep dvu_ab;		/* AppleBus */
#endif	ABDEVDEP
#ifdef	LDDEVDEP
	struct lddevdep dvu_ld;		/* ACC LH-DH/11 */
#endif	LDDEVDEP
#ifdef	VVDEVDEP
	struct vvdevdep dvu_vv;		/* proNet p1000 */
#endif	VVDEVDEP
	int             dvu_XX;		/* (place holder) */
    } dv_devdep;
};

/*
 *  Common support/network device fields
 */
#define dv_next		dv_common.dvc_next
#define dv_addr		dv_common.dvc_addr
/*#define dv_br		dv_common.dvc_br	*/
#define dv_nunit	dv_common.dvc_nunit
#define dv_istatus	dv_common.dvc_istatus
#define dv_dstate	dv_common.dvc_dstate
#define dv_mem		dv_common.dvc_mem
/* #define dv_intr		dv_common.dvc_br	*/
#define dv_pb		dv_common.dvc_pb
/*
 *  Device configuration fields
 */
#define	dv_reset  dv_cf.cf_reset
#define	dv_output dv_cf.cf_output
#define	dv_diag   dv_cf.cf_diag
#define dv_ar	  dv_cf.cf_ar
#define dv_pr	  dv_cf.cf_pr
#define dv_hrd	  dv_cf.cf_hrd
#define dv_flag	  dv_cf.cf_flag
#define dv_bcast  dv_cf.cf_bcast
#define dv_hln    dv_cf.cf_hln
#define	dv_type	  dv_cf.cf_type
#define	dv_head	  dv_cf.cf_head
#define	dv_trail  dv_cf.cf_trail
#define	dv_slop   dv_cf.cf_slop
#define	dv_mtu	  dv_cf.cf_mtu
#define	dv_ddlen  dv_cf.cf_ddlen

/*
 *  Device dependent state structures
 */
#define	dv_da	dv_devdep.dvu_da
#define	dv_dt	dv_devdep.dvu_dt
#define	dv_du	dv_devdep.dvu_du
#define	dv_dz	dv_devdep.dvu_dz
#define	dv_ec	dv_devdep.dvu_ec
#define	dv_en	dv_devdep.dvu_en
#define	dv_il	dv_devdep.dvu_il
#define	dv_ld	dv_devdep.dvu_ld
#define	dv_vv	dv_devdep.dvu_vv
#define	dv_ab	dv_devdep.dvu_ab


/*
 *  Device independent status bits
 */

#define	DV_XBUSY	0100000	/* transmit in progress on device  */
#define	DV_RBUSY	0040000	/* receive in progress on device */
#define	DV_XINTR	0020000	/* transmit interrupt already in progress */
#define	DV_RINTR	0010000	/* receive interrupt already in progress  */

#define	DV_SILENCED	0000004	/* device is temporarily disabled */
#define	DV_ONLINE	0000002	/* device is logically on-line */
#define	DV_ENABLED	0000001	/* device is physically enabled */


/*
 *  Device independent flag bits.
 */

#define	DV_SWAB	01		/* interface swaps byte order */
#define	DV_CPU	02		/* interface is a single CPU */


/*
 *  Device output routine return codes.
 */

#define	DVO_QUEUED	1	/* queued for transmission */
#define	DVO_DROPPED	2	/* could not be queued */
#define	DVO_DEAD	3	/* destination host is dead */
#define	DVO_DOWN	4	/* interface is down */


/*
 *  Device flush routine parameter flags.
 */

#define	DVFL_XQ	01			/* flush output queue */
#define	DVFL_XP	02			/* flush current output packet */

#define	DVFL_X	(DVFL_XQ|DVFL_XP)	/* flush all output */


/*
 *  Device type definitions.
 */
				/* support devices: */
#define	DVT_KW11	01	/* KW-11L line time clock */
#define	DVT_TTY		02	/* console terminal */

				/* network devices: */
#define	DVT_3ETHER	0100	/* 3MB ethernet */
#define	DVT_IL10	0101	/* 10MB (Interlan) ethernet */
#define	DVT_DZ11	0102	/* DZ-11 serial line(s) */
#define	DVT_LHDH11	0103	/* ACC LH/DH-11 IMP interface */
#define	DVT_DTE		0104	/* DTE-20 interface */
#define	DVT_DA28	0105	/* DA28-F interface */
#define	DVT_PRONET	0106	/* proNET 10MB ring interface */
#define	DVT_DUP11	0107	/* DUP11 synchronous line interface */
#define	DVT_3COM10	0110	/* 10MB (3Com) ethernet */
#define	DVT_DEUNA	0111	/* 10MB (DEUNA) ethernet */
#define DVT_APBUS	0112	/* Bob's AppleBus net */
#define DVT_TR		0113
#define DVT_CH		0114

/*
 *  Device reset routine parameters.
 */
#define	DVR_OFF		0	/* disable device */
#define	DVR_ON		1	/* disable and enable device */


#ifndef	VAXCOM

/*
 *  Macro to delay for "a while"
 */

#define	DELAY(n)	{int cnt=n; while (cnt--);}


/*
 *  Macro to change the processor priority level to device interrupt
 *  level.
 */

#define	spldv(dv)	\
    spl((dv)->dv_pb -> pb_br)



extern p_device dvhead;		/* head of network device chain */


#endif	VAXCOM

#endif	D_DEVICE
