#define mtod(m,t) ((t)((int)(m) + (m)->offset))
#define dtom(m)   ((struct mbuf *)((int)(m) - ((int)(m) % MLEN) + MBASE))

#define M_FREE 1
#define MPOOL  10
#define MBASE  ((int)(m_pool) % MLEN)
#define MHEAD  (sizeof (struct mbuf_header))
#define MLEN   1600				/* djw: was 1024 */
#define MDATA  MLEN - MHEAD;

typedef unsigned short m_funct();

struct mbuf_header
{
    struct mbuf    *next;
    struct mbuf	   *last;
    unsigned short flags;
    unsigned short offset;
    unsigned short length;
};

struct mbuf
{
    struct mbuf    *next;
    struct mbuf	   *last;
    unsigned short flags;
    unsigned short offset;
    unsigned short length;
    unsigned char data [MLEN - sizeof(struct mbuf_header)];
};
