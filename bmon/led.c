#include "types.h"
#include "globals.h"
#include "led.h"

#if C_LED > 0

ledinit()
{
    register short *leddata = (u_short *)LEDDATA;
    register short *ledctrl = (u_short *)LEDCTRL;

    *leddata = LEDINIT;
    *ledctrl = LEDINIT;
    *leddata = LEDBLANK;
};

void lederr(num)
register u_char num;
{
    int i;
    
    ledputw((0xA00B | (num * 16)));  /* insert error code into [XX] */
    for(i=LEDWAIT; i; i--);
    ledgoodcnt = 0;
    ledbadcnt  = 0;
}

void ledok()
{
    int i;
    
    ledputw(0xABAB);		     /* display [][] (all ok) message */
    for(i=LEDWAIT/10; i; i--);
}

#define ledupdate ledputw((((ledbadcnt << 8)& 0xff00) | ledgoodcnt))

/* increments by one a two digit bcd number */
#define bcdplus(bcd) { 							     \
    bcd += 1;								     \
    if ((bcd & 0x0f) > 9) {		 /* if roll-over on low digit */     \
	bcd &= 0xf0;			 /* clear low digit */  	     \
	if ((bcd += 0x10) > 0x99)	 /* if roll-over on high digit */    \
	    bcd = 0;							     \
    }									     \
}

void ledgood()
{
    bcdplus(ledgoodcnt);
    ledupdate;
}

void ledbad()
{
    bcdplus(ledbadcnt);
    ledupdate;
}

#endif
