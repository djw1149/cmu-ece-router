#ifdef OMBYTE
#define ACREG 0xffff01
#endif OMBYTE

#ifdef GPIB
#define ACREG 0x10040
#endif GPIB

struct acreg {
    u_char csr,
           res_a,
	   data;
};
