#include "types.h"
#include "mbuf.h"
#include "in.h"
#include "ip.h"
#include "tftp.h"
#include "boot.h"
#include "globals.h"
#include "led.h"

boot_handle (p, blk)
register char *p;
{
    register struct boothdr *b = (struct boothdr *)p;

    if (blk == 1) {
	if (b->type != BO_TYPE) {
	    printf ("boot: bad file type=%x\n", b->type);
#if C_LED > 0
	    lederr(LED_BADFILETYPE);
#endif C_LED
	    return 0;
	}
	boot_var->entry = (char *)b->entry;
	bcopy (&p[b->hlen], (char *)b->entry, (TFTP_SEGLEN - b->hlen));
	boot_var->adr = (char *)b->entry + (TFTP_SEGLEN - b->hlen);
    }
    else {
        putc ('#');
#if C_LED > 0
	ledgood();
#endif C_LED
	bcopy(p,boot_var->adr,TFTP_SEGLEN);
	boot_var->adr += TFTP_SEGLEN;
    }
    return 1;
}

boot(str)
char *str;
{
    char file[255];
    struct in_addr to;

    bzero (boot_var, sizeof *boot_var);
    putc (':');
    if (str == 0) {
	   str = mainstr;
	   gets (str);
    }
    else printf ("%s\n", str);

    if (scan_ip (&str, &to)) {
	scan_str (&str, file);
    }
    else {
	printf ("bad file name format: %s\n", mainstr);
#if C_LED > 0
	lederr(LED_BADFILENAME);
#endif C_LED
	return 0;
    }

    bpinit(&to, file);			/* fill in the bootp packet */

    if (tftp_get (&to, file, mainbuf, boot_handle) == 0) {
       printf ("\nFILE LOADED, PROGRAM STARTS AT 0x%x\n",boot_var->entry);
#if C_LED > 0
       ledok();
#endif C_LED
       return 1;
    }
    return 0;
}
