#ifdef	PDP11
#define	LSBFIRST
#define INTISSHORT
#endif	PDP11

#ifdef	IBMAT
#define	LSBFIRST
#define INTISSHORT
#endif	IBMAT

#ifdef	M68K
#define	MSBFIRST
#define	INTISLONG
#endif	M68K

#ifndef D_MCH
#define D_MCH

#ifdef MSBFIRST
#define adcr2()		    0
#define xbcopy(f,fm,t,tm,l) bcopy(f,t,l)
#define mapIO()		    0
#define remap(x)	    0
#define reboot		    boot
#endif MSBFIRST

#ifdef M68K
#define HZ	  100
#define TINTERVAL (HZ/10)
typedef char *p_pdev;
typedef struct autoid *p_autoid;	/* djw: added from sys/aconf.h */
typedef struct autoconf *p_autoconf;
typedef struct autorom *p_autorom;
#endif M68K


#define ntohs(x) (x)
#define htons(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)


#endif D_MCH

#ifdef MSBFIRST
#define MACHINE "M68000"
#endif MSBFIRST
