/*
 * Configuration code for the 68000 boot monitor
 *
 ****
 * HISTORY:
 ****
 * 16 Aug 86 - David Waitzman (djw@ampere):
 *	split the configure command to configure devices separately
 ****
 * Dawn-of-router - gregg, mathis, bohman, bob, djw, krimse, etc...
 *	created, hacked, burned, reaped, and hacked some more.
 ****
 */

#include "types.h"
#include "in.h"			/* djw: added */
#include "ec.h"
#include "il.h"
#include "device.h"
#include "globals.h"
#include "autoconf.h"
#include "bootp.h"
#include "fakerom.h"

/*
 * search the following for addresses of the various routines in each driver
 * THESE GO INTO ROM, YOU CAN'T WRITE INTO THEM!
 */
struct config *autoconf[] = {
    &ilconfig,
    &ecconfig,
    0
};

/*
 * returns 1 if ok to autoboot, else returns 0. (i.e. if using the builtin,
 * fake, rom, don't autoboot)
 */
configure()
{
    register struct autorom *ap;
    int canauto = 0;

    ap = (struct autorom *)halloc(sizeof(struct autorom) + EXTRA);
    if (!ap)
    	panic("configure halloc rom");
    bzero(ap, sizeof (struct autorom) + EXTRA);
    romcopy((u_short *)AUTOROM, (u_char *)ap, sizeof (struct autorom) + EXTRA);

    if ((bcmp(AUTOSTRING, ap->rom_id, sizeof (AUTOSTRING)-1) == 0)
          && (ap->serial_number) > 0) {
	printf("Configure rom at %x\n", (char *)AUTOROM);
	canauto = 1;
    } else {
	printf("No conf-rom found, using built in configure table at %d(10)\n",
	       &fakerom);
        bcopy((u_char *)&fakerom, (u_char *)ap, sizeof(struct fakerom));
	canauto = 0;
    }

    rom_var = ap;
    printf("Serial Number %d\n", rom_var->serial_number);

#if 0
/* COMMENTED OUT--- */
    printf("num_boot = %d | ", rom_var->num_boot);
    printf("num_dump = %d | ", rom_var->num_dump);
    printf("num_dev  = %d | ", rom_var->num_dev);
    printf("num_nets = %d | ", rom_var->num_nets);
    printf("num_glob = %d | ", rom_var->num_glob);
    printf("num_text = %d\n",  rom_var->num_text);
#endif 

    fix_rom();			/* adjust offsets and such */

    /* allocate space for the global (reused) config structure */
    net_var = (struct config *)halloc(sizeof (struct config));
    if (!net_var)
    	panic("configure halloc ncf");
    bzero(net_var, sizeof (struct config));
    return(canauto);
}

/*
 * Configures the device with the devid pointed at by dev.  If dev = 0, then
 * it configures whatever devices it chooses.  Returns 0 on an error.
 *
 * using the device type, we search the autoconf structure to find the
 * driver handlers for the given device.  next, we configure the device.
 */
devconfigure(ai)
register struct autoid *ai;
{
    register struct config **acp;

    if (!ai) {
	/* find a device by trying them all until one works */
	register struct autoid **aiargv;

	for (aiargv = rom_var->autoids; *aiargv; aiargv++)
	    if (devconfigure(*aiargv)) return(1);

	panic("devconfigure: no working device");
	return(0);
    }

    confdev = ai;			/* set the global device */
	
    /* find the right config struct for the given device name */
    for (acp = autoconf; *acp; acp++) {
	register struct config *cf = *acp;

        if (bcmp(cf->cf_name, ai->id.devname, 4))    /* correct device? */
	    continue;				     /* try another device */

	if (rom_var->num_nets < ai->id.devminor) {
	    printf("No net for given minor number\n");
	    return(0);
	}
	
	*net_var = *cf;				     /* a structure copy */

	/* try to configure the device, if it really is there */
        net_var->devconf = &(ai->devconf);
        if (net_var->cf_probe)
	    if ((*net_var->cf_probe)(net_var))
	        return(0);		             /* it isn't there */

	if (net_var->cf_init)
	    if (!(*net_var->cf_init)(net_var)) {
	        printf(" didn't initialize\n");
	        return(0);
	    }
	putc('\n');
	return(1);
    }
    panic("devconfigure: no driver");
    return(0);
}


/*
 * This routine copies every-other-byte into continuous bytes, for instance,
 * the conf-roms into ram
 */
#ifndef ALTROM
romcopy(s, d, n)
register u_short *s;
register u_char *d;
register n;
{
    while (n--)
    	*d++ = *s++;	/* this ignores the high byte of the word */

}

#else

/* a special version of rom copy that uses the other half of the word */
romcopy(s, d, n)
register u_short *s;
register u_char *d;
register n;
{
    while (n--)
    	*d++ = (*s++ >> 8);	/* this punts the low byte of the word */
}
#endif


/*
 * returns the ip address from the configure structure
 */
long
*getipaddr()
{
    return(*((long **)rom_var->net + confdev->id.devminor));
}


#define fixromadr(ele, typ) ele = (typ)((long)ntohl(ele) + (long)rom_var)

fix_rom()
{
    struct autoid **aiargv = rom_var->autoids;

    fixromadr(rom_var->boot, char **);
    fixptrs(rom_var->boot);
    fixromadr(rom_var->dump, char **);
    fixptrs(rom_var->dump);
    fixromadr(rom_var->autoids, struct autoid **);
    fixptrs(rom_var->autoids);
    fixromadr(rom_var->net, long **);
    fixptrs(rom_var->net);
    fixromadr(rom_var->glob, char **);
    fixptrs(rom_var->glob);
    fixromadr(rom_var->text, char **);
    fixptrs(rom_var->text);

    rom_var->serial_number = ntohl(rom_var->serial_number);
    rom_var->rom_size	= ntohl(rom_var->rom_size);
    rom_var->last_long	= ntohl(rom_var->last_long);
    rom_var->num_boot	= ntohl(rom_var->num_boot);
    rom_var->num_dump	= ntohl(rom_var->num_dump);
    rom_var->num_dev	= ntohl(rom_var->num_dev);
    rom_var->num_nets	= ntohl(rom_var->num_nets);
    rom_var->num_glob	= ntohl(rom_var->num_glob);
    rom_var->num_text	= ntohl(rom_var->num_text);

    /* convert the devminor numbers */
    for (aiargv = rom_var->autoids; *aiargv; aiargv++)
	(**aiargv).id.devminor = ntohl((**aiargv).id.devminor);
}

/*
 * swaps and adjusts all of the argv like list of pointers given it.
 * adr must point to the first element in the array of pointers.  argv must
 * be zero terminated.
 */
fixptrs(adr)
long *adr;
{
/*  printf("fixing @ %d: ", adr); */
    while(*adr) {
/*  	printf("%d-->", *adr); */
        fixromadr(*adr, long);
/*      printf("%d  ", *(adr)); */
        adr++;
    }
}

printdev(id)
struct devid *id;
{
    putc(id->devname[0]);
    if (id->devname[1]) {
	putc(id->devname[1]);
	if (id->devname[2]) {
	    putc(id->devname[2]);
	    if (id->devname[3])
		putc(id->devname[3]);
	}

    }
    printf("%d ", id->devminor);
}

bpinit(addr, file)
struct in_addr *addr;
char *file;
{
    register struct bootp *bp = bootppkt;
    register struct vend *v;

    bzero(bp, sizeof *bp);
    bp->bp_op = 0;		/* Neither a reply or request */
    bp->bp_htype = (u_char)1;			/* 10mb ethernet */
    bp->bp_hlen = (u_char)6;			/* 10mb ethernet  */
    bp->bp_hops = (u_char)0;			/* not used */
    bp->bp_xid = htonl((u_long)0);		/* random */
    bp->bp_secs = htons((u_short)0);		/* elapsed time */
    bcopy(getipaddr(), &(bp->bp_ciaddr), 4);
    bcopy(&bp->bp_ciaddr, &(bp->bp_yiaddr), 4);
    bcopy(addr, &bp->bp_siaddr, 4);		/* fill in the server addr */
    strcpy(bp->bp_file, file);			/* copy file name */
    v = bp->bp_vend;

    strcpy(v->v_magic, VM_CMU);			/* copy magic number */
    bcopy(*rom_var->glob, &v->v_dgate, 4); 	/* copy gateway */
}

print_text()
{
    char **texts = rom_var->text;
    int	i;

    printf("Rom text contents:\n");
    for (i = 0; i < rom_var->num_text; i++, texts++)
        printf("%s\n", *texts);
    if (i == 0) printf("    --NOTHING--\n");
}