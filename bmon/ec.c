#include "types.h"
#include "mbuf.h"
#include "device.h"
#include "in.h"
#include "ip.h"
#include "ecreg.h"
#include "ec.h"
#include "autoconf.h"
#include "globals.h"

int ecprobe(), ecinit(), ec_rcv(), ec_poll(), ec_xmt();

struct config ecconfig = {
    NULL, /* djw: replaced (caddr_t)0xc0000, */
    ecprobe,
    ecinit,
    ec_rcv,
    ec_poll,
    ec_xmt,
    {
	EC_HRD,
	ECT_IP,
	ECT_AR,
	EC_HLN,
	{
	    0, 0, 0,
	    0, 0, 0
	},
	{
	    0xff, 0xff, 0xff,
	    0xff, 0xff, 0xff
	},
    },
    0,
    {'e','c',0,0},
};

ecprobe(cf)
register struct config *cf;
{
    return(probew(getdevs(ecdevt, cf)->addr));
}

ecinit(cf)
register struct config *cf;
{
    register struct eccb *ec;
    register struct ecdevice *reg = (getdevs(ecdevt, cf)->addr);

    printf("ec at csr %x", reg);
    ec = (struct eccb *)halloc(sizeof (struct eccb));
    if (ec) {
	bzero(ec, sizeof (struct eccb));
    	cf->cf_netcb = (caddr_t)ec;
    } else
    	panic("ecinit halloc ec");

    *((u_short *)reg) = 0;
    SET_BIT(reg->ec_csr.csr_reset);
    bcopy(reg->ec_arom, reg->ec_aram, EC_HLN);
    reg->ec_csr.csr_pa = EC_PA_MINE_BROAD_NERR;
    SET_BIT(reg->ec_csr.csr_amsw);
    bcopy(reg->ec_arom, cf->cf_dev.phys, EC_HLN);
    SET_BIT(reg->ec_csr.csr_absw);
    return (1);
}

ec_rcv(cf, m)
register struct config *cf;
register struct mbuf *m;
{
    register struct eccb *ec = cf->cf_netcb;
    register struct ec_header *h;
    register struct ecdevice *reg = (getdevs(ecdevt, cf)->addr);
    u_short *sp;
    u_long len;

    m->offset = MHEAD;
    sp = (u_short *)reg->ec_rbufa;
    
    h = mtod(m, struct ec_header *);
    len = *sp++;
    len = (len&0x7ff) - 1;
    m->length = len;

    if (m->length > MLEN)
    	panic ("ec_rcv len");

    bcopy(sp, mtod(m, char *), len);
    m->length -= sizeof *h;
    m->offset += sizeof *h;
/*    printf("ec_rcv: len %d type %x\n", len, h->ec_type);*/

    SET_BIT(reg->ec_csr.csr_absw);
    if (h->ec_type == ECT_IP)
    	return ip_input(ip_var, m);
    if (h->ec_type == ECT_AR) {
	ar_input(&cf->cf_dev, m);
	return (0);
    }
}

ec_poll(cf, timeout)
register struct config *cf;
register int *timeout;
{
    register struct ecdevice *reg = (getdevs(ecdevt, cf)->addr);

    while ((*timeout)-- > 0)
    	if (reg->ec_csr.csr_absw == 0)
		return (1);
    return (0);
}

ec_xmt(cf, m, to, flag)
register struct config *cf;
register struct mbuf *m;
register struct in_addr *to;
{
    register struct eccb *ec = (struct eccb *)cf->cf_netcb;
    register struct ec_header *h;
    register struct ecdevice *reg = (getdevs(ecdevt, cf)->addr);

    int type = ECT_IP;
    u_char *dst;

    if (flag || (dst = (char *)ar_resolved(to)) == 0) {
	ar_output(&cf->cf_dev, to, m);
	type = ECT_AR;
	dst = cf->cf_dev.bcast;
    }
    m->offset -= sizeof (*h);
    m->length += sizeof (*h);

    h = mtod(m, struct ec_header *);
    bcopy(dst, h->ec_dhost, sizeof (h->ec_dhost));
    bcopy(cf->cf_dev.phys, h->ec_shost, sizeof (h->ec_shost));
    h->ec_type = type;

    if (m->length < ECRMINLEN)
    	m->length = ECRMINLEN;

    *((u_short *)reg->ec_tbuf) = 2048 - m->length;
    bcopy(mtod(m, char *), &reg->ec_tbuf[2048 - m->length], m->length);
    SET_BIT(reg->ec_csr.csr_tbsw);
    ec->jam = ~0;
/*    printf("ec_xmt: len %d type %x\n", m->length, h->ec_type);*/
    while (reg->ec_csr.csr_tbsw)
	if (reg->ec_csr.csr_jam) {
	    if ((ec->jam <<= 1) == 0)
	    	CLEAR_BIT(reg->ec_csr.csr_tbsw);
	    else {
		printf("ec (%x) jam backoff %x\r\n", reg, (~ec->jam)&0xffff);
		reg->ec_back = -(~ec->jam);
		SET_BIT(reg->ec_csr.csr_jam);
	    }
	}
}
