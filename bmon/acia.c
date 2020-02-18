#include "types.h";
#include "acreg.h";
#include "acia.h";
#include "globals.h";

acinit (reg)
register struct acreg *reg;
{
    register int i;
    reg->data = 0;
    reg->csr = 0x03;
    reg->csr = 0x95;
    for (i = 0; i < 10000; i++);
}

acwrite (reg, c)
register struct acreg *reg;
char c;
{
    while ((reg->csr & 0x2) == 0);
    reg->data = c;
}


acread (reg, c)
register struct acreg *reg;
char *c;
{
    int timeout;

    while ((reg->csr & 0x1) == 0) {
        timeout = 5;
	net_poll(&timeout, mainbuf);
    }
    *c = reg->data;
}

