#define	LOWER	0x1
#define	UPPER	0x2
#define	SPACE	0x4
#define DIGIT	0x8
#define	HEX	0x10
#define PUNC	0x20
#define CTRL	0x40

extern	char	charmap[];

#define	is_letter(c)	((charmap)[c]&(UPPER|LOWER))
#define	is_upper(c)	((charmap)[c]&UPPER)
#define	is_lower(c)	((charmap)[c]&LOWER)
#define	is_digit(c)	((charmap)[c]&DIGIT)
#define	is_hex(c)	((charmap)[c]&(DIGIT|HEX))
#define	is_space(c)	((charmap)[c]&SPACE)
#define is_punct(c)	((charmap)[c]&PUNC)
#define is_char(c)	((charmap)[c]&(UPPER|LOWER|DIGIT))
#define is_ctrl(c)	((charmap)[c]&CTRL)
#define is_ascii(c)	((unsigned)(c)<=0177)

#define c_upper(c)	((c)-'a'+'A')
#define c_lower(c)	((c)-'A'+'a')
#define c_mask(c)	((c)&0177)
