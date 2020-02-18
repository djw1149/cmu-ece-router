#include "charmap.h"
#include <stdio.h>

scan_str (s, d)
register char **s, *d;
{
    scan_sp(s);
    while (**s && !is_space(**s))
	*d++ = *(*s)++;
    *d = 0;
}

scan_int (s, r)
register char  **s;
register int r;
{
    	register int n, i;
	register char c;

	n = 0;
        while ((c = **s) && is_hex(c)) {
	    if (is_digit(c))
	       i = c - '0';
	    else if (is_upper(c))
	         i = c - 'A' + 0xa;
	    else i = c - 'a' + 0xa;

	    if (i < r)
		(n = n*r + i, (*s)++);	
	}
	return(n);
}

scan_sp(s)
register char **s;
{
    while (**s && is_space(**s)) {
          (*s)++;
    }
}

scan_ip (s, b)
register char **s;
register char *b;
{
    register int i;
    scan_sp(s);
    if (*(*s)++ ==  '[') {
	for (i = 0; i < 3; i++) {
	    b[i] = scan_int (s, 10);
	    if (*(*s)++ != '.')
	       return 0;
	}
        b[i] = scan_int (s, 10);
	if (*(*s)++ == ']')
           return 1;
    }
    return 0;
}


scan_assign (p, i, size)
char *p;
int i, size;
{
    switch (size) {
	case 2:
	    *(short *)p = i;
	    break;
	case 4:
	    *(long *)p = i;
	    break;
	default:
	    *(char *)p = i;
	    break;
    }
}
