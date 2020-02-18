#include "types.h"
#include "varargs.h"
#include "print.h"

#define max(a,b) ((a) > (b)? (a): (b))
#define min(a,b) ((a) < (b)? (a): (b))

int doprnt (format, args, func)
char *format;
va_list args;
int (*func)();
{
	/* Current position in format */
	char *cp;

	/* Starting and ending points for value to be printed */
	char *bp, *p;

	/* Field width and precision */
	int width;

	/* Format code */
	char fcode;

	char prefix = 0;

	char buf[MAXDIGS + 1];
	/* The value being converted, if integer */
	long val;

	/* Set to point to a translate table for digits of whatever radix */
	char *tab = "0123456789abcdef";

	/* Work variables */
	int n, hradix, lowbit;

	cp = format;

	while (*cp)
		if (*cp != '%')
		   (*func)(*cp++);
		else
		{

		*cp++;
		scan:
		        switch (fcode = *cp++) {
			case 'd':
			case 'u':
				hradix = 5;
				goto fixed;

			case 'o':
				hradix = 4;
				goto fixed;

			case 'X':
			case 'x':
				hradix = 8;
			fixed:
				/* Fetch the argument to be printed */
				if (fcode == 'd')
					val = va_arg (args, int);
				else
					val = va_arg (args, unsigned);

				/* If signed conversion, establish sign */
				if (fcode == 'd') {
					if (val < 0) {
						prefix = '-';
						if (val != HIBIT)
						   val = -val;
					} 
				}

				/* Set translate table for digits */

				/* Develop the digits of the value */
				p = bp = buf + MAXDIGS + 1;
				do {
					lowbit = val & 1;
					val = (val >> 1) & ~HIBIT;
					*--bp = tab[val % hradix * 2 + lowbit];
					val /= hradix;
				} while (val);
				if (prefix) *--bp = prefix;
				while (bp < p)
				      (*func)(*bp++);
				break;
			case 'c':
				*buf = va_arg (args, int);
				(*func)(*buf);
				break;

			case 'A':
			{
			    register u_char val;
			    register int i;
			    register int rx = 10;

			    p = buf + MAXDIGS + 1;
			    for (i = 0;;) {
				    bp = p;
				    val = va_arg(args, u_char *)[i];
				    do {
					*--bp = tab[(val % rx)];
					val /= rx;
				    } while (val);
				    while (bp < p) (*func)(*bp++);
				    if (++i < 4) (*func)('.');
				    else break;
			    }
			}
			break;
				    
				
			case 's':
				bp = va_arg (args, char *);
				while (*bp)
				    (*func)(*bp++);
				break;

			case '\0':
				cp--;
				break;

		/*	case '%':	*/
			default:
				(*func)(fcode);
				break;

			}
		}

	return;
}

