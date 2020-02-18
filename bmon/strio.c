#include "types.h"
#include "acreg.h"

ttinit (tt)
register struct ttcb *tt;
{
    acinit (ACREG);
}

putc (c) char c;
{
    if (c == '\r')
       c = '\n';
    if (c == '\n')
       acwrite (ACREG, '\r');
    acwrite (ACREG, c);
}

getc ()
{
    char c;
 
    acread (ACREG, &c);
    c &= 0x7f;		/* djw: clear out the high bit of each character */
    putc (c);
    return c;
};

gets (s)
register char *s;
{
    register char *st = s;
    for (;;)
    {
	*s = getc();
	if (*s == 0177)
           if (s != st)
	   {
	      printf ("\010 \010");
	      s--;
	   }
	   else putc (7);
	else if (*s == '\r')
	     break;
	else if (*s) s++;
    }
    *s = 0;
}

char tab[] = "0123456789ABCDEF";
putn (n)
{
    char arr[8];
    register char *p = arr;
    register int i;

    for (i = 0; i < 8; i++, n >>= 4)
        *p++ = tab[n & 0xf];
    for (i = 0; i < 8; i++)
        putc (*--p);
}

printf (format, args)
char *format;
int args;
{
    doprnt (format, &args, putc);
}

printb (b, n)
register unsigned char *b;
register int n;
{
    register int i;
    putc('[');
    for (i = 0; i < n; i++)
        printf ("%x ",b[i]);
    printf ("]\n");
}


int strcmp (s1, s2)
register char *s1, *s2;
{
    while (*s1 == *s2++)
	if (*s1++ == 0)
	   return 0;
    return -1;
}

char *strcpy (s, t)
char *s, *t;
{    
    while (*s++ = *t++);
    return s;
}

strlen (s)
char *s;
{
    register char *b = s;

    while (*s) s++;
    return (int)(s - b);
}
