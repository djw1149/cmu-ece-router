/*
 *  Autoconfigure definitions
 * $Header: aconf.h,v 1.2 86/10/18 01:03:20 djw Exp $
 *
 * $Source: /ua1/djw/router/h/RCS/aconf.h,v $
 *
 **********************************************************************
 * HISTORY
 * 18-Sep-86  David Waitzman (djw) at @tiltedbox() Carnegie Mellon
 *	Added new configure rom information.
 *
 * 14-Dec-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added PI_FLAG definition [V2.0(407)].
 *
 * 06-Aug-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added new device dependent structure length field to driver
 *	configuration parameter structure.
 *
 * 23-May-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Added PI_DONE defintion.
 *
 * 02-Mar-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Turned ai_flag field into ai_nunit field since it really wasn't
 *	being used; added PI_C_ISPHYS definition.
 *
 * 20-Feb-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

#ifndef	ACMAXVEC

#define	ACMAXVEC	4	/* maximum interrupt vectors per device */
#define	ACMAXNAM	4	/* maximum device name length (bytes) */


/*
 *  hardware interrupt vector layout
 */

#ifdef PDP11
struct vector
{
    p_int vc_pc;		/* PC to transfer to */
    int   vc_ps;		/* PS to pick up before transfer */
};
#endif PDP11
#ifdef M68K
struct vector
{
    p_int vc_pc;		/* PC to transfer to */
};
#endif M68K


/*
 *  Auto-configuration driver parameter structure
 *
 *  Each device driver provides one of these structures to the auto-
 *  configuration module.  This structure defines all of the common device
 *  features for the type of device accessed through that driver.  Drivers
 *  which do not support a particular feature or for which the feature makes
 *  no sense will typically initialize the field to zero unless otherwise
 *  indicated.
 */



#define AUTOSTRING	"IPconf00"
#define AUTOROM		(0xfe8000)


/*
 * A device id.
 * devname is a device name of form 'il', padded with nulls to 4 chars
 * if needed.  devminor is used to index into the net structures, therefore,
 * it typically is a number from 0 to about 3.  For devices that do not use
 * a net structure, like a terminal, we currently use a devminor of 0.
 * There may not be duplicated devminor numbers of network devices in a
 * running router.  Duplicates will be ignored (i.e. the first netdevice of
 * a given minor number is configured, any others are ignored).
 */
struct devid {
    char	devname[4];		/* possibly padded device name */ 
    long	devminor;		/* device minor unit number */
};

/*
 * The autoid structure holds information about a single device in the system.
 * It is found in the confrom.  Only the internal fakerom will be holding
 * the autoids for things like terminals and the ptm.  The devconf field
 * always holds the csr address for the device.  This is really not just a two
 * element structure, since some devices may store information after the
 * devconf field (like the shared memory address and priority in interlan
 * drivers).
 */
struct autoid {
    struct devid id;		/* the name and number of the device */
    long   devconf;		/* where the device specific info will go */
    /* stuff will be here for some devices */
};
    

/*
 * Autoconf argv structure.
 * This is found either in physical rom, in the compiled in fakerom,
 * or on disk in the ibmat router.
 */
struct autorom {
    char	rom_id[8];		/* Magic number w/ rom version */
    long	rom_size;		/* Size of rom in bytes */ 
    long	last_long;		/* Number of contiguous longs */
    long	serial_number;		/* Unique rom serial number */
    long	num_boot;		/* Number of boot strings */
    p_char	*boot;			/* Boot strings */
    long	num_dump;		/* Number of dump strings */
    p_char	*dump;			/* Dump strings */
    long	num_dev;		/* Number of device descriptors */
    p_autoid	*autoids;		/* Device descriptions */
    long	num_nets;
    pu_char	*net;			/* IP address and subnet lists */
    long	num_glob;
    pu_char	*glob;			/* global configure info */
    /* first entry must be WNGW.  This was a char type in the bmon */
    long	num_text;
    p_char	*text;			/* Text */
};


#define EXTRA	4096   /* space after the main rom struct for argvs & data */


/*
 * Protocol/identification option codes
 * Values below NPR are individual protocol types (found in ../h/proto/h).
 * To change something like the ipaddr for the device, put PR_IP and then the
 * ipaddr in the net structure.
 */
#define	PI_HRD		(NPR+0)	 /* set hardware address of device */
#define	PI_GWAY		(NPR+1)	 /* supply known IP gateway address on this */
				 /*  directly connected network */
#define	PI_REDIR	(NPR+2)	 /* add IP redirect mapping for an address */
				 /*  on this directly connected network to */
				 /*  initial routing table */
#define	PI_CABLE	(NPR+3)	 /* set cable identification number */
#define	PI_AUTH		(NPR+4)	 /* authorize traffic to a protocol address */
				 /*  on a restricted cable */
#define	PI_RESTR	(NPR+5)	 /* restrict traffic between the device and */
				 /*  a given cable */
#define	PI_FLAG		(NPR+6)	 /* specify device flags */
#define	PI_C_ISME	(NPR+7)	 /* begin conditional on configured device */
#define	PI_C_ISPHYS	(NPR+8)	 /* begin conditional on physical address */
#define	PI_C_END	(NPR+9)	 /* end conditional interpretation */
#define	PI_DONE		(NPR+10) /* terminate option processing */
#define PI_ROM		(NPR+11) /* use information in rom */
#define	PI_END		(NPR+12) /* end of option code list */
#define PI_SUB		(NPR+13) /* subnet specification follows */


/*
 *  Auto-configuration intialization parameter structure
 *  
 *  Each driver provides one of these structures to the auto-configuration
 *  module.  The structure provides the linkages to all of the once-only
 *  initialization routines called by the auto-configure process to probe for,
 *  configure, and initialize the device.
 *
 */

struct autoconf
{
    pf_int ac_probe;		/* device probe routine */
    pf_int ac_init;		/* device (hardware) initialization routine */
    pf_int ac_once;		/* driver (software) initialization routine */
    pf_int ac_reset;		/* device reset (non-network device only) */
    pf_int ac_intr[ACMAXVEC];	/* interrupt vector routines */
    pf_int ac_proc;		/* input process routine */
    int    ac_size;		/* size of device register area in I/O page */
/*    p_autoid ac_ai;*/		/* autoconfiguration identification info */
    p_config ac_cf;		/* driver configuration information */
    char   ac_name[ACMAXNAM+1];	/* device mnemonic */
};


/*
 *  Flags for device probe routines
 */

#define	PB_CHECK	00	/* check that CSR is the correct device type*/
#define	PB_RESET	01	/* reset device */
#define	PB_PROBE	02	/* generate an interrupt on device */

#endif ACMAXVEC
