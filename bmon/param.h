#undef vax
#define	NULL	0

#ifdef OMBYTE
#define TIMEOUT 100000
#define BOOTFILE "[128.2.251.3]/usr/router/boot/ombyte"
#endif OMBYTE

#ifdef GPIB
#define TIMEOUT 100000
#define BOOTFILE "[128.2.251.3]/usr/router/boot/gpib"
#endif GPIB
