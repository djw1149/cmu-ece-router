/* Version information */
#define	VERSION		1
#define RELEASE		2

#ifndef NULL
#define NULL 0
#endif

#ifndef u_char
#define u_char unsigned char
#endif

#define AUTOSTRING	"IPconf00"

#define ROMLENMAX	4096
#define MAXARGV		16	/* max argv pointers (- 1 for the NULL) */
#define DEVMAX		16	/* max longs in a devconf */
#define NETMAX		128	/* max bytes in a net entry */


/* Random defines */
#define MAX_LINE	132
#define	DOT		'.'
#define	CR		'\n'
#define SEMI		';'
#define COMC		'#'		/* the comment start character */

#define iskeystart(c)	(isalpha((c)) || ((c) == '_'))
#define iskeycont(c)	(isalnum((c)) || ((c) == '_'))
#define isignore(c)	(ispunct((c)) && ((c) != '_'))

#define	qpr(A)		if (!q_flag) { fflush(stdout); printf A; }
#define spr(A)		if (spr_flag) printf A
#define fgetl(A)	fgets(A, MAX_LINE, fi); if (!i_flag) printf(A); \
			line_count++

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
    long   devconf[DEVMAX];	/* where the device specific info will go */
};

#define NUMARGL 6			/* number of ** in rom_image */
struct rom_image {
    char	rom_id[8];		/* Magic number w/ rom version.*/
    	/* rom_id was type in bob's */
    long	rom_size;		/* Size of rom in bytes */ 
    long	last_long;		/* Number of contiguous longs */
    long	serial_number;		/* Unique rom serial number */
    long	num_boot;		/* Number of boot strings */
    char	**boot;			/* Boot strings */
    long	num_dump;		/* Number of dump strings */
    char	**dump;			/* Dump strings */
    long	num_dev;		/* Number of device descriptors */
    struct autoid **autoids;		/* Device descriptions */
    long	num_nets;
    u_char	**net;			/* IP address and subnet lists */
    long	num_glob;
    u_char	**glob;			/* global configure info
    					 * first entry must be wngw.
					 */
    long	num_text;
    char	**text;			/* Text */
};
