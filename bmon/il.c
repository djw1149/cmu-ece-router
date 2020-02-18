#include "types.h"
#include "mbuf.h"
#include "device.h"
#include "in.h"
#include "ip.h"
#include "ilreg.h"
#include "il.h"
#include "autoconf.h"
#include "globals.h"

#define ilalloc(a,r)	(a = (struct r *)il->free, \
			 il->free += sizeof (struct r))

int ilprobe(), ilinit(), il_rcv(), il_poll(), il_xmt();

struct ilcb ildconfig = {
    0,
    0,
    0,
    0,
    0,
};

struct config ilconfig = {
    NULL, /* djw: replaced (caddr_t)0xFFAAA0, */
    ilprobe,
    ilinit,
    il_rcv,
    il_poll,
    il_xmt,
    {
	IL_HRD,
	ILT_IP,
	ILT_AR,
	IL_HLN,
	{
	    0, 0, 0,
	    0, 0, 0
	},
	{
	    0xff, 0xff, 0xff,
	    0xff, 0xff, 0xff
	},
    },
    &ildconfig,
    {'i','l',0,0},
};

extern u_char *swap();
extern u_short swab();
extern u_char *swal();

struct ilbuf *
ilbuf_alloc(il, n)
register struct ilcb *il;
register int n;
{
    register struct ilbuf *fb = (struct ilbuf *)il->free;
    register struct ilbuf *b;
    register int i;

    for (i = 0; i < n; i++) {
	ilalloc(b, ilbuf);
	b->fa.stat = b->fa.cmd = 0;
	b->fa.link = iloff(il->free);
	b->fa.bd   = iloff(&b->bd);
	b->bd.stat = 0;
	b->bd.next = b->fa.link + sizeof b->fa;
	b->bd.addr = (u_char *)swap(b->buf);
	b->bd.size = sizeof b->buf;
    }
    b->bd.next = iloff(&fb->bd);
    b->fa.link = iloff(&fb->fa);
    fb->fa.cmd = ILCMD_S;
    return (fb);
}

il_cmd(cf, cbl)
register struct config *cf;
register int cbl;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ilscb *scb = il->scb;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register int tempcbl;

    /* start command and wait for acknowledge*/
    tempcbl = scb->cbl;
    scb->cbl = iloff(cbl);
    scb->cuc = ILCUC_START;
    reg->il_ca = 1;
    while (((struct ilhead *)scb)->cmd) ;

    /* wait for command unit to go idle */
    while ((scb->stat & ILACK_CNR) == 0) ;
    scb->ack = scb->stat;
    reg->il_ca = 1;

    while (((struct ilhead *)scb)->cmd) ;
    scb->cbl = tempcbl;
}

ildoconf(cf)
register struct config *cf;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ilconfigure *cbl = il->free;

    bzero(cbl, sizeof *cbl);
    cbl->cmd = ILCMD_CONFIG | ILCMD_EL;
    cbl->fifolim = 8;					/* 7(1) */
    cbl->bytecnt = 10;					/* 6(2) */
    cbl->preamb = 2; cbl->addlen = 6; cbl->acloc = 1;   /* 9(3) */
							/* 8(4) */
    cbl->if_spacing = 96;				/* B(5) */
							/* A(6) */
    cbl->retry = 15;					/* D(7) */
    cbl->slottime = 512;				/* C(8) */
							/* F(9) */
    cbl->tono = 1;					/* E(10) */
    il_cmd(cf, iloff(cbl));
}

ilsetaddr(cf)
register struct config *cf;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register struct ilasetup *cbl = il->free;

    bzero(cbl, sizeof *cbl);
    cbl->cmd = ILCMD_IASETUP | ILCMD_EL;
    ilbcopy(reg->il_addr, cbl->addr, sizeof reg->il_addr);
    il_cmd(cf, iloff(cbl));
}

ilinit(cf)
register struct config *cf;
{
    register struct ilscb *scb;
    register struct ilscp *scp;
    register struct iliscp *iscp;
    register struct ilhead *h;
    register struct ilcb *il;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register char *mem;

    printf("il at csr %x", reg);
    il = (struct ilcb *)halloc(sizeof (struct ilcb));
    if (il) {
	bcopy(cf->cf_netcb, il, sizeof (struct ilcb));
	cf->cf_netcb = (caddr_t)il;
    } else
    	panic("ilinit halloc il");

    bcopy(reg->il_addr, cf->cf_dev.phys, IL_HLN);
    il->free = mem = (char *)getdevs(ildevt, cf)->smaddr;
    printf("   memory at %x",mem);

    reg->il_swp  = 1;
    reg->il_chpe = 0;

    ilalloc(il->scb, ilscb);
    h = (struct ilhead *)(scb = il->scb);
    bzero(scb, sizeof *scb);
    il->lrbuf = ilbuf_alloc(il, IL_NRBUFS);
    il->rbuf =  ilmem(il->lrbuf->fa.link);
    il->tbuf =  ilbuf_alloc(il, IL_NTBUFS);
    scb->rfa = iloff(il->rbuf);
    scb->cbl = iloff(il->tbuf);

    iscp = (struct iliscp *)il->free;
    bzero(iscp, sizeof *iscp);
    iscp->busy = 1;
    iscp->base = swap(mem);

    scp = (struct ilscp *)(mem + ILMEMSIZE - sizeof *scp);
    bzero(scp, sizeof *scp);
    scp->iscp = swap(iscp);

    reg->il_chpe = 1;
    reg->il_ca = 1;
    while (reg->il_ca == 0) ;

    if ((iscp->busy) || (h->stat != 0xa000))
       return (0);

    h->cmd = h->stat;
    reg->il_ca = 1;
    while (reg->il_ca) ;

    ildoconf(cf);
    ilsetaddr(cf);

    scb->ack = scb->stat;
    scb->ruc = ILRUC_START;
    reg->il_ca = 1;
    while (scb->ruc) ;

    reg->il_dmae = 1;
    reg->il_onl = 1;
}

ilrstart(cf, p)
register struct config *cf;
register struct mbuf *p;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register struct ilbuf *b = il->rbuf;
    register char *mem = (char *)getdevs(ildevt, cf)->smaddr;

    p->length = MIN((b->bd.stat & ILSTAT_CNT), MLEN);

    reg->il_cmd = ILDMA_BTOMEM;
    reg->il_cnt = swab(p->length);
    reg->il_buf = swab(b->buf);
    reg->il_mem = swal(mtod(p, struct ilpacket *));
    reg->il_do = 1;
    while (reg->il_done == 0) ;
    b->fa.cmd  = ILCMD_S;
    b->fa.stat = b->bd.stat = 0;
    il->lrbuf->fa.cmd = 0;
    il->lrbuf = b;
    il->rbuf = ilmem(b->fa.link);
}

ilxstart(cf, p)
register struct config *cf;
register struct mbuf *p;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register struct ilbuf *b = il->tbuf;

    reg->il_cmd = ILDMA_BTODEV;
    reg->il_mem = swal(mtod(p, struct ilpacket *));
    reg->il_cnt = swab(p->length);
    reg->il_buf = swab(il->tbuf->buf);
    reg->il_do = 1;
    while (reg->il_done == 0) ;
    b->fa.stat = 15;
    b->fa.cmd = ILCMD_XMIT | ILCMD_EL;
    b->bd.stat = p->length | ILSTAT_EOF;
}

il_rcmd(cf)
register struct config *cf;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    register struct ilscb *scb = il->scb;

    switch (scb->rus) {
	case ILRUS_SUSP:
	    scb->ack = 0;
	    scb->ruc = ILRUC_RESUME;
	    reg->il_ca = 1;
	    while (scb->ruc) ;
	    break;

        case ILRUS_IDLE:
	case ILRUS_NORES:
/*	    printf ("ilrcv: rus=%x\n", scb->rus); */
	    break;

	default:
	    break;
    }
}

il_rcv(cf, m)
register struct config *cf;
register struct mbuf *m;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ilscb *scb = il->scb;
    register struct il_header *l;

    m->offset = MHEAD;

    l = mtod(m, struct il_header *);
    ilrstart (cf, m);
    if (il->scb->rus != ILRUS_RDY)
       il_rcmd (cf);

    if (m->length > MLEN)
       panic ("il_rcv: illegal packet length.");

    m->length -= sizeof *l;
    m->offset += sizeof *l;

    if (l->il_type == ILT_IP)
       return ip_input (ip_var, m);
    if (l->il_type == ILT_AR) {
       ar_input (&cf->cf_dev, m);
       return 0;
    }
}    

il_xmt(cf, m, to, flag)
register struct config *cf;
register struct mbuf *m;
register struct in_addr *to;
{
    register struct ilcb *il = cf->cf_netcb;
    register struct ilscb *scb = il->scb;
    register struct il_header *l;
    register struct ildevice *reg = (getdevs(ildevt, cf)->baseaddr);
    int type = ILT_IP;
    u_char *dst;

    if (flag || (dst = (char *)ar_resolved (to)) == 0) {
	ar_output (&cf->cf_dev, to, m);
	type = ILT_AR;
	dst = cf->cf_dev.bcast;
    }

    m->offset -= sizeof *l;
    m->length += sizeof *l;

    l = mtod (m, struct il_header *);
    bcopy (dst, l->il_dhost, sizeof l->il_dhost);
    bcopy (cf->cf_dev.phys, l->il_shost, sizeof l->il_shost);
    l->il_type = type;

    if (m->length < ILRMINLEN)
       m->length = ILRMINLEN;

    ilxstart (cf, m);
    scb->ack = 0;
    scb->cuc = ILCUC_START;
    reg->il_ca = 1;
    while (scb->cuc) ;
}

il_poll(cf, timeout)
register struct config *cf;
register int *timeout;
{
    register struct ilbuf *b = ((struct ilcb *)cf->cf_netcb)->rbuf;

    while ((*timeout)-- > 0)
        if (b->fa.stat & ILSTAT_C) {
	   return 1;
	}
    return 0;
}

ilprobe(cf)
register struct config *cf;
{
    /* check for the device and for the buffer memory */
    return(probeb(getdevs(ildevt, cf)->baseaddr)
	   || probeb(getdevs(ildevt, cf)->smaddr));
}

ilprint (b)
u_char *b;
{
    register int i;
    putc ('[');
    for (i = 0; i < 5; i++)
        printf ("%x.", b[i]);
    printf ("%x]", b[i]);
}
