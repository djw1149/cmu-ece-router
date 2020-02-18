/*
 *  Data type declarations
 *	Caution: This file is shared by both the router and router tools
 *		vax must be defined to use the 4.2 network library
 *
 **********************************************************************
 * HISTORY
 * 10-Oct-85  Matt Mathis (mathis) at CMU
 * 	Added caddr_t etc from sys/types.h so this can be used with the 4.2
 *	library in place of sys/types.h
 *
 * 12-Oct-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Moved p_alarm declaration from time.h [V2.0(402)].
 *
 * 12-Oct-84  Mike Accetta (mja) at Carnegie-Mellon University
 *	Started history.
 *
 **********************************************************************
 */

typedef	unsigned int		u_int;
typedef	unsigned short		u_short;
typedef	unsigned char		u_char;
typedef unsigned long		u_long;

#ifndef	VAXCOM

typedef	long *			p_long;
typedef	int *			p_int;
typedef	short *			p_short;
typedef	char *			p_char;

typedef	unsigned int *		pu_int;
typedef	unsigned short *	pu_short;
typedef	unsigned char *		pu_char;

typedef	int 		      (*pf_int)();

#define	SWAB(const)	 (((const)<<8)|(((const)>>8)&0377))

#else

typedef	VAXCOM			p_long;
typedef	VAXCOM			p_short;
typedef	VAXCOM			p_int;
typedef	VAXCOM			p_char;

typedef	VAXCOM			pu_int;
typedef	VAXCOM			pu_short;
typedef	VAXCOM			pu_char;

typedef	VAXCOM 			pf_int;

#define	p_struct(type)	\
    typedef VAXCOM 			p_/**/type

#endif	VAXCOM

typedef struct device *p_device;
typedef struct packet *p_packet;
typedef struct alarm  *p_alarm;

union longval
{
    long lv_long;
    struct
    {
	short lvs_hiword;
	short lvs_loword;
    }    lv_s;
};
#define	lv_hiword	lv_s.lvs_hiword
#define	lv_loword	lv_s.lvs_loword

#ifndef	VAXCOM

#define	TRUE	1
#define	FALSE	0

#endif	VAXCOM

#define MAXHWADDRL	6
