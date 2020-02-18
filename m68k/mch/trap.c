/*
 *  Trap instruction handling module
 *
 **********************************************************************
 * HISTORY
 * 23-Jun-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Created.
 *
 **********************************************************************
 *
 *  This module is specially assembled such that the normal C register
 *  save/restore processing is not done by any of the routines in this
 *  file.  This permits the registers to be changed for the caller when
 *  needed.  Do not use any register declarations unless you intend
 *  this result.
 */


#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proc.h"

#include "../mch/mch.h"
#include "../mch/trap.h"
#include "../mch/procu.h"


struct resetvec {
    char *init_sp;
    int (*init_start)();
    int (*init_restart)();
};


struct resetvec *bootvec = (struct resetvec *)0x20000;

#ifdef PDP11
/* #include "../h/psw.h" */



/*
 *  trap - trap instruction handler
 *
 *  dummy = dummy parameter pushed as part of regular interrupt handling
 *  r1    = register 1 at time of trap (change relected back to caller)
 *  r0    = register 0 at time of trap (change relected back to caller)
 *  pc    = PC at time of trap (change restored by RTI)
 *  ps    = PS at time of trap (change restored by RTI)
 */

trap(dummy, r1, r0, pc, ps)
short *pc;
{
    switch(((ps&PS_PM)?(mfpi(pc-1):(pc[-1])))&077)
    {
	/*
	 *  Set processor priority (from R0)
	 */
	case 0:
	    r0 = ps;
	    ps = (ps&~PS_BR7)|(r1&PS_BR7);
	    break;

	/*
	 *  Reboot processor
	 */
	case 1:
	    boot();
	    break;

	/*
	 *  Oops.
	 */
	default:
	    panic("trap");
    }
}
#endif PDP11

/*
 *  panic - terminate on "impossible" condition
 *
 *  str = a string describing the condition
 *
 *  Save the current process context (for possible later examination in a
 *  dump).  Display the current uptime and error condition message on the
 *  console and branch to the boot ROM (if present) or halt the processor (if
 *  no ROM is available).
 */

struct context savecx = {0};	/* saved process context on crash */
char *rbaddr = {0};		/* boot ROM address (set at startup) */
char tbuf[20] = {0};		/* buffer for uptime (not local to keep */
				/*  the stack size down within panic() in */
				/*  case the problem was caused by a stack */
				/*  error */

panic(str)
{
    spl7();
    saveframe(&savecx);
    cprintf("\r\n%s panic: %s\r\n  ",  timeup(tbuf), str);
    reboot();
    /* no return */
}



/*
 *  utrap - unexpected interrupt handler
 *
 *  This routine is invoked directly from the unexpected trap handler once it
 *  has decided which vector was responsible for the interrupt.  The register,
 *  PC, and PSW parameters are made directly available from the interrupt stack
 *  and may be changed within this routine to take effect if/when returning
 *  from the interrupt.
 *
 *  If the global variable 'nxmok' is non-zero on a NXM interrupt, it is simply
 *  cleared (to catch potential recursive traps) and the interrupt is ignored.
 *  This permits controlled checks for non-existent devices/addresses without
 *  crashing the entire system.
 *
 *  If the global variable 'introk' is non-zero on any non-special (vectors 040
 *  and above) interrupt, it is interpreted as the address of a routine to call
 *  with the interrupting vector as its only parameter.  This routine will
 *  process the interrupt (and presumably clear this variable).  This feature
 *  allows the system to auto-configure its devices without neededing to know
 *  at which vectors they are going to interrupt.
 *
 *  If neither of these two cases hold, the interrupt is fatal.  The current
 *  process context is saved, the trap vector and registers are displayed on
 *  the console, and control is transferred to the boot ROM (if present) or the
 *  processor is halted (if no ROM is available).
 */

int nxmok = 0;		/* allow (one) NXM interrupt */
int (*introk)() = 0;	/* send unexpected device interrupts to this routine */

utrap(vec, arg)
char *arg;
{
    register struct except *e = (struct except *)&arg;
    register struct buserr *b = (struct buserr *)&arg;

    switch (vec >> 2) {
	case 2:
	case 3:
	    if (nxmok) {
	       nxmok = 0;
	       return;
	    }
	    cprintf ("\r\nbus trap: type=%d, func=%04x, addr=%08x\r\n",
	    	     vec, b->funct, b->acc);
	    cprintf ("IR=%04x, SR=%04x, PC=%08x\r\n",b->ir, b->sr, b->pc);
	    break;
	case 15:
	case 25: /* 1 */
	case 26: /* 2 */
	case 27: /* 3 */
	case 28: /* 4 */
	case 29: /* 5 */
	case 30: /* 6 */
	    if (introk) {
		(*introk)(vec);
		return;
	    }
	case 31: /* 7 */
	    cprintf ("\r\nSOFTWARE ABORT\r\n");
	    break;
	    
	default:
            cprintf("trap: type=%d, SR=%04x, PC=%08x\r\n", vec, e->sr, e->pc);
    }

    spl7();
    saveframe(&savecx);
    cprintf("A0=%08x A1=%08x A2=%08x A3=%08x\r\n",e->a0,e->a1,e->a2,e->a3);
    cprintf("A4=%08x A5=%08x A6=%08x A7=%08x\r\n",e->a4,e->a5,e->a6,e->a7);
    cprintf("D0=%08x D1=%08x D2=%08x D3=%08x\r\n",e->d0,e->d1,e->d2,e->d3);
    cprintf("D4=%08x D5=%08x D6=%08x D7=%08x\r\n",e->d4,e->d5,e->d6,e->d7);
    boot();
}

/*
 *  boot - reboot processor if possible (from kernel mode)
 *
 *  dummy = dummy argument declaration to obtain a handle on the call frame
 *
 *  Check for a reboot address (i.e. that a boot ROM was found during the
 *  auto-configuration process).  If no ROM was found, simply halt.  If a boot
 *  ROM  is present, prepare to transfer to it.  Before transferring control to
 *  the ROM, however, a RESET instruction must be executed to restore the
 *  processor to a known state (and especially to disable memory management).
 *  This will cause problems when the instruction completes since the PC will
 *  now address a physical rather than virtual address.  Therefore, the RESET
 *  and JMP instruction to transfer control to the boot ROM are copied down
 *  into physical and virtual locations 0,2 and 4 and executed from there.
 *
 *  N.B.  This routine may only be called from kernel mode since it needs
 *  to do a RESET instruction as part of the final transfer to the bootstrap
 *  ROM (or to simply halt the processor when no ROM is present).
 */

boot(dummy)
short dummy;
{
    spl7();
    cprintf ("[REBOOT]\r\n%c%c%c",0xff, 0xff, 0xff);
    exit();
}
