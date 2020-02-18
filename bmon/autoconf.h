/*	autoconf.h	*/

/* Device configuration information */
struct config {
    /* we are only going to use the following address when there is no
       conf-rom in the system */
/*djw: killed   caddr_t	cf_addr;*/	/* device bus address */
    long	*devconf;		/* device instance specific info */
    int		(*cf_probe)();		/* device probe routine */
    int		(*cf_init)();		/* initialize routine */
    int		(*cf_rcv)();		/* packet receive routine */
    int		(*cf_poll)();		/* receiver done poll routine */
    int		(*cf_xmt)();		/* packet transmit routine */
    struct device cf_dev;		/* network control */
    caddr_t	cf_netcb;		/* device dependent control */
    char	cf_name[4];		/* device name (ascii) MUST <=4 CHARS*/
};

#define AUTOSTRING	"IPconf00"
#define AUTOROM		(0xfe8000)
#define AUTOROMDEV 	(AUTOROM + (sizeof (struct autorom) * 2))

struct autoromdev {
    char	rom_ipaddr[4];		/* used in ip.c */
    int		rom_cable;		/* never used */
    int		rom_reserved[6];	/* never used */
};

/* very old:
 * struct autorom {
 *    char	rom_id[8];
 *    int	rom_num;
 *    char	rom_gway[4];
 *    unsigned	rom_boot;
 *};
 */

struct devid {
    char	devname[4];		/* device name of form il, padded
    					   with nulls to 4 chars if needed */
    long	devminor;		/* device minor unit number */
};

/*
 * The autoid structure holds information about a single device in the system
 */
struct autoid {
    struct devid id;		/* the name and number of the device */
    long   devconf;		/* where the device specific info will go */
};
    

/*
 * Autoconf argv structure taken from ~bob/router/?/mkrom.h on 15 Aug 86
 */
struct autorom {
    char	rom_id[8];		/* Magic number w/ rom version.*/
    	/* rom_id was type in bob's */
    long	rom_size;		/* Size of rom in bytes. */ 
    long	last_long;		/* Number of contiguous longs. */
    long	serial_number;		/* Unique rom serial number. */
    long	num_boot;		/* Number of boot strings. */
    char	**boot;			/* Boot strings. */
    long	num_dump;		/* Number of dump strings. */
    char	**dump;			/* Dump strings. */
    long	num_dev;		/* Number of device descriptors. */
    struct autoid **autoids;		/* Device descriptions. */
    long	num_nets;
    u_char	**net;			/* IP address and subnet lists. */
    long	num_glob;
    char	**glob;			/* global configure info
    					 * first entry must be wngw.
					 */
    long	num_text;
    char	**text;			/* Text. */
/*    long	rest[1];  start of arrays */
};



/* convert two chars into an int */
/*#define atoi2(x) ((10 * (*(char *)(x) - '0')) + (*(char *)((x) + 1) - '0'))*/

#ifndef NULL
#define NULL 0
#endif

/*
 * access the device instance specific info in the cf structure from the
 * conf-rom
 */
#define getdevs(type,struc) ((struct type *)(struc->devconf))

#define EXTRA 1000   /* the maximum amount of argv & data space in the rom */
