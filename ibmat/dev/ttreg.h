
#define	TTADDR		((struct ttdevice *)(0xFFFF01))

struct ttdevice
{
    u_char tt_csr;
    u_char skip;
    u_char tt_db;
};

#define	NTTCHAR	256
#define	TTLOWAT	64

#define TTS_DIV1	0x0
#define TTS_DIV16	0x1
#define TTS_DIV64	0x2
#define TTS_RESET	0x3

#define TTS_7EVEN2	0x00
#define TTS_7ODD2	0x04
#define TTS_7EVEN1	0x08
#define TTS_7ODD1	0x0C
#define TTS_8NOPAR2	0x10
#define TTS_8NOPAR1	0x14
#define TTS_8EVEN1	0x18
#define TTS_8ODD1	0x1C

#define TTS_RTS		0x40
#define TTS_BREAK	0x60

#define TTS_XIE		0x20
#define TTS_RIE		0x80

#define TTS_STD		(TTS_8NOPAR1|TTS_DIV16)

#define TTS_RCV		0x1
#define TTS_XMT		0x2
