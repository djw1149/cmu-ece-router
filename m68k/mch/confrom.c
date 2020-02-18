/*#define DEBUGCONF*/
/*
 * New configure rom handling code
 * $Header: confrom.c,v 1.3 86/10/22 13:34:24 djw Exp $
 *
 *****
 * HISTORY
 * Fri Sep 19 11:42:21 EDT 1986 David Waitzman (djw) at CMU
 *	Created, based on bmon/autoconf.c
 *
 */
 
#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/packet.h"
#include "../../h/proto.h"

#include "../mch/mch.h"

#include "../mch/intr.h"
#include "../../h/devstats.h"
#include "../mch/device.h"
#include "../../h/aconf.h"

#include "../mch/fakerom.h"

/*
 * returns 1 if there is a new configure rom present, else returns 0.
 * - mrom is the main rom (a fixed copy).
 * - srom is the secondary rom (a fixed copy).
 * if there is no real hardware configure rom present, then mrom is the
 * fakerom, and 0 is returned.  srom is just zeroed in this case.
 */
int find_confrom(mrom, srom)
register p_autorom mrom, srom;
{
    /*
     * zero out the rom variables, and zero out the extra space that
     * will be used by the argv and value data
     */
    bzero(mrom, sizeof (struct autorom) + EXTRA);
    bzero(srom, sizeof (struct autorom) + EXTRA);

    /*
     *  the rom is only in the even have of memory.  This will allow
     *  use to use only one rom.  We take the even bytes of rom
     *  and load them into a local structure so that they are now
     *  using both odd and even bytes.
     */
    romcopy((u_short *)AUTOROM, (u_char *)mrom, sizeof *mrom + EXTRA);

    if ((bcmp(AUTOSTRING, mrom->rom_id, sizeof (AUTOSTRING)-1) == 0)
          && (mrom->serial_number) > 0) {
	cprintf("Configure rom at %x\r\n", (char *)AUTOROM);
	fix_rom(mrom);
        bcopy((u_char *)&fakerom, (u_char *)srom, sizeof *srom + EXTRA);
	fix_rom(srom);
	return(1);
    } else {
	cprintf("No conf-rom found, using built in configure table at %d\r\n",
	       &fakerom);
        bcopy((u_char *)&fakerom, (u_char *)mrom, sizeof *mrom + EXTRA);

#ifdef DEBUGCONF
display_rom(mrom);
#endif
	fix_rom(mrom);
	return(0);
    }
}

/*
 * display some information about the rom
 */
void display_rom(rom)
register p_autorom rom;
{
    cprintf("Rom serial number %d\r\n", rom->serial_number);
    cprintf("# boot = %d | ", rom->num_boot);
    cprintf("# dump = %d | ", rom->num_dump);
    cprintf("# dev  = %d | ", rom->num_dev);
    cprintf("# nets = %d | ", rom->num_nets);
    cprintf("# glob = %d | ", rom->num_glob);
    cprintf("# text = %d\r\n", rom->num_text);
}


#define fixromadr(ele, typ) ele = (typ)((long)ntohl(ele) + (long)rom)

static void fix_rom(rom)
register p_autorom rom;
{
    p_autoid *aiargv = rom->autoids;
/* cprintf("fixing rom @%d\r\n", rom); */
    rom->serial_number	= ntohl(rom->serial_number);
    rom->rom_size	= ntohl(rom->rom_size);
    rom->last_long	= ntohl(rom->last_long);
    rom->num_boot	= ntohl(rom->num_boot);
    rom->num_dump	= ntohl(rom->num_dump);
    rom->num_dev	= ntohl(rom->num_dev);
    rom->num_glob	= ntohl(rom->num_glob);
    rom->num_text	= ntohl(rom->num_text);
    fixromadr(rom->boot, char **);
    fixptrs(rom,rom->boot);
    fixromadr(rom->dump, char **);
    fixptrs(rom,rom->dump);
    fixromadr(rom->autoids, p_autoid *);
    fixptrs(rom,rom->autoids);
    fixromadr(rom->net, long **);
    fixptrs(rom,rom->net);
    fixromadr(rom->glob, char **);
    fixptrs(rom,rom->glob);
    fixromadr(rom->text, char **);
    fixptrs(rom,rom->text);

    /* convert the devminor numbers */
    for (aiargv = rom->autoids; *aiargv; aiargv++)
	(**aiargv).id.devminor = ntohl((**aiargv).id.devminor);
#ifdef DEBUGCONF
cprintf("fixed rom\r\n");
#endif
}

/*
 * swaps and adjusts all of the argv like list of pointers given it.
 * adr must point to the first element in the array of pointers.  argv must
 * be zero terminated.
 */
fixptrs(rom,adr)
register p_autorom rom;
long *adr;
{
#ifdef DEBUGCONF
    cprintf("fixing @ %d: ", adr);
#endif
    while(*adr) {
#ifdef DEBUGCONF
  	cprintf("%d->", *adr); 
#endif
        fixromadr(*adr, long);
#ifdef DEBUGCONF
	cprintf("%d  ", *adr); 
#endif
        adr++;
    }
#ifdef DEBUGCONF
    cprintf("\r\n");
#endif
}




/*
 * This routine copies every-other-byte into continuous bytes, for instance,
 * the conf-roms into ram.  The different versions defined copy from
 * different bytes of the word.
 */
#ifdef OLDROM
static void romcopy(s, d, n)
register u_short *s;
register u_char *d;
register n;
{
    while (n--)
    	*d++ = *s++;	/* this ignores the high byte of the word */

}

#else
/* CURRENTLY MUST USE THIS SIDE OF THE #IF */

/* a special version of rom copy that uses the other half of the word */
static void romcopy(s, d, n)
register u_short *s;
register u_char *d;
register n;
{
    while (n--)
    	*d++ = (*s++ >> 8);	/* this punts the low byte of the word */
}

#endif
