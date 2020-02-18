/*
 *  Device autoconfiguration module (intialization only)
 *
 **********************************************************************
 * HISTORY
 * 04-Sep-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Created (from autoconfI.c).
 *
 **********************************************************************
 */

#include "../../h/rtypes.h"
#include "../../h/queue.h"
#include "../../h/proto.h"

#include "../mch/mch.h"
#include "../mch/intr.h"
#include "../mch/device.h"
#include "../mch/autoconf.h"

#include "cond/xx.h"
#include "cond/ch.h"
#include "cond/tr.h"
#include "cond/ptm.h"

/*
 *  Supported device driver table
 *
 *  Each entry in this table is a pointer to the auto-configuration parameter
 *  block for its device driver.  There is exactly one entry in this table for
 *  each supported device interface.
 */

#ifdef PDP11
extern struct autoconf kwautoconf;	/* KW11-L line time clock */
#else PDP11
extern struct autoconf ptmautoconf;
#endif PDP11
extern struct autoconf ttautoconf;	/* console terminal line */

struct autoconf *autoconf[] =
{
#if	C_ROM1 > 0
    &r1autoconf,
#endif	C_ROM1

#if	C_ROM2 > 0
    &r2autoconf,
#endif	C_ROM2

#if	C_ROM3 > 0
    &r3autoconf,
#endif	C_ROM3

#ifdef PDP11
    &kwautoconf,
#endif PDP11

#if	C_AB > 0	/* It should be early, to turn off interupts */
    &abautoconf,
#endif	C_AB

#ifdef C_PTM > 0
    &ptmautoconf,
#endif C_PTM
    &ttautoconf,

#if	C_WT > 0
    &wtautoconf,
#endif	C_WT

#if	C_CH > 0
    &chautoconf,
#endif

#if	C_TR > 0
    &trautoconf,
#endif

#if	C_EN > 0
    &enautoconf,
#endif

#if	C_IL > 0
    &ilautoconf,
#endif	C_IL

#if	C_DZ > 0
    &dzautoconf,
#endif	C_DZ

#if	C_LD > 0
    &ldautoconf,
#endif	C_LD

#if	C_VV > 0
    &vvautoconf,
#endif	C_VV

#if	C_EC > 0
    &ecautoconf,
#endif	C_EC

#if	C_DU > 0
    &duautoconf,
#endif	C_DU

#if	C_DA > 0
    &daautoconf,	/* must follow en */
#endif	C_DA

#if	C_DT > 0
    &dtautoconf,	/* must follow en and vv */
#endif	C_DT

#if	C_XX > 0
    &xxautoconf,
#endif	C_XX

    0			/* end of the list */
};
