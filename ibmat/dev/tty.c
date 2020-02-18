
static char ttc;

/*
 *  tt_input - process terminal input
 *
 *  dv = terminal device 
 *
 *  This routine currently doesn'y pay any attention to the typed character and
 *  simply displays a system status message on the console whenever any
 *  character is typed.
 */

tt_input(dv)
register struct device *dv;
{
    printf("Begin polling console\r\n");
    for (;;) {
    	if (kbhit()){
	    ttc=getch();
	    if (!ttysetswitch(ttc))
	    	systat();
	}
	prelease();
    }
}

ttreset() {return 1;}

extern cputc();
printf(format, args)
{
    int s=spl7();
    char far *a=&args;
    doprintf(cputc, format, a);
    spl(s);
};

cprintf(format, args)
{
    int s=spl7();
    char far *a=&args;
    doprintf(cputc, format, a);
    spl(s);
};
