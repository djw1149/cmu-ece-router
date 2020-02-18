#ifndef lint
static char rcsid[] = "$Header: mkrom.c,v 1.13 86/10/26 00:50:47 djw Exp $";
#endif

/*
 *
 *  mkrom - make a boot rom in the new (IPconf00) format.
 *
 ***
 * HISTORY
 ***
 *
 ****
 * Sat Oct  4 1986 david waitzman (djw) at cmu
 * rewrote the input routines for much better robustness
 *
 ****
 * Tue Sep 30 1986 david waitzman (djw) at cmu
 *  added simple comment capability and pi structure keywords
 *
 ****
 * Tue Sep 23 20:08:18 EDT 1986- David Waitzman, created V1.0
 *  added global configure stuff argv.
 *
 ****
 * 31 July 86- Robert Barker, created V0.0:
 *  Batteries not included.
 *  Error checking in this program is minimal, and doesn't always work,
 *  especially the IP address entry.
 *  Note: this program was written to get the maximum functionality in the
 *  minimum time.  It does not represent the programming ideology of the
 *  author or the ECE department.
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include "mkrom.h"
#include "keyw.h"

extern char *malloc(), *calloc();

#define LP	sizeof(long int *)

/* Flags */
enum {f_bin, f_srec, f_ccode} outform = f_bin;	/* output file format */
int q_flag = 0;		/* quiet mode for input files */
int i_flag;		/* interactive flag (1 = interactive, 0 else) */
int d_flag = 0;		/* 1 = do a dump, 0 no */
int spr_flag = 1;	/* Supress echo */

/* Random statics */
int line_count = 0;		/* Number of lines read from input file */

char *inf, *outf;		/* File names */

FILE *fi, *fo;			/* file descriptors */
char line[MAX_LINE];

int devconflens[MAXARGV];	/* lengths of each devconf in each autoid */
int numdevconfs = 0;		/* index into above */

int netlens[MAXARGV];		/* lengths of net list in a net group */
int numnets = 0;		/* index into above */

int globlens[MAXARGV];		/* lengths of glob stuff */
int numglobs = 0;		/* index into above */

void read_dev();

void got_eof()
{
    fatal_error("Read EOF- aborting");
}


/*
 * main program
 */
main(argc,argv)
int argc;
char **argv;
{
    int getopt();
    extern char *optarg;
    extern int optind;
    FILE *fopen();
    char *str = "stdin";
    struct rom_image im;
    char c;
    
    /* Deal with the switches */
    while ((c = getopt(argc, argv, "cbsdq")) != EOF)
	switch(c) {
	    case 'd':
	        d_flag = 1; break;		    /* do a dump */
	    case 'b':
		outform = f_bin; break;		    /* output binary */
	    case 's':
	        outform = f_srec; break;	    /* output s-records */
	    case 'c':
	        outform = f_ccode; break;	    /* output C code */
	    case 'q':
		q_flag = 1; spr_flag = 0; break;    /* quiet mode */
	    default:
	        fprintf(stderr, "usage: mkrom [-scbqd] fout [fin]\n");
		exit(-1);
	}

    /* check for too many arguments */
    if ((argc - optind) > 2) {
    	fprintf(stderr, "usage: mkrom [-scbqd] fout [fin]\n");
	exit(-1);
    }

    printf("Mkrom version %d.%d\n\n",VERSION,RELEASE);
    outf = argv[optind];
    printf("Generating file %s\n",outf);
    if ((fo = fopen(outf,"w")) == NULL) {
	fprintf(stderr, "mkrom: can't open file %s for output.\n",outf);
	exit(-1);
    }

    if (optind == argc - 2) {
	inf = argv[optind + 1];
	if ((fi = fopen(inf,"r")) == NULL) {
	    fprintf(stderr, "mkrom: can't open file %s for input.\n",inf);
	    exit(-1);
	}
	i_flag = 0;
    }
    else {
	inf = str;
	fi = stdin;
	spr_flag = 0;		/* Supress echo */
	if (q_flag) {
	    fprintf(stderr, 
	       "Ignoring -q option. (Invalid when reading from stdin.)\n");
	    q_flag = 0;
	}
	i_flag = 1;
    }
    printf("Reading definitions from %s\n\n",inf);
    init_image(&im);
    make_rom(&im);
    printf("\nProcessed %d lines from %s\n", line_count, inf);
    if (d_flag) dump_image(&im);
    output_rom(&im);
}

/*
 * return codes: 1- ok, 0- error
 */
int parse_num(str, base, num)
char *str;
int base;
long *num;
{
    int res;

    switch (base) {
	case 10: res = sscanf(str, "%ld", num); break;
	case 16: res = sscanf(str, "%lx", num); break;
	case  8: res = sscanf(str, "%lo", num); break;
	default: fatal_error("Unknown base to parse_num");
    }
/* printf("parsed %d\n", *num); */
    return(res);
}

/*
 * return codes: 1- ok, 0- end_of_input, 1- error in input
 */
int scan_num(num, kw)
long *num;				/* returned number */
int kw;					/* 1 to allow keyword entry, else 0 */
{
    char c = mygetc(fi);		/* input character */
    char oc = (char)0;			/* old c */
    char strarr[KWLEN];
    char *str = strarr;

loop:
    /* strip off leading white space */
    while (isspace(c)) {
	oc = c;
	c = mygetc(fi);
    }

    /* check for a newline . newline terminator */
    if ((oc == '\n') && (c == '.'))
	if ((c = mygetc(fi)) == '\n') 
	    return(0);				/* end of input */

    if (isignore(c)) {
	c = mygetc(fi);
	goto loop;				/* read more white space */
    }

    if (iskeystart(c))
        if (kw) {
	    do {
		*(str++) = c;
		c = mygetc(fi);
	    } while (iskeycont(c));
            ungetc(c, fi);			/* in case of a newline */
	    *str = NULL;			/* terminate the string */
	    *num = lookup_kw(strarr);
	    return( (*num == -1) ? -1 : 1 );
	} else
	    fatal_error("Illegal char in input");

    /* Get first character and convert the line */
    if (c == '0') {				/* hex */
	c = mygetc(fi);
        if (isspace(c) || isignore(c)) {
            ungetc(c, fi);			/* in case of a newline */
	    *num = 0;
	    return(1);
	}
	if ((c == 'x') || (c == 'X')) {
	    c = mygetc(fi);
	    do {
		*(str++) = c;
		c = mygetc(fi);
	    } while (isxdigit(c));
	    ungetc(c, fi);
	    *str = NULL;			/* terminate the string */
	    return((parse_num(strarr, 16, num) == 1)
	    	   ? (kw&&(*num > 255) ? fatal_error("Number not a byte") : 1)
		   : -1);
	} else
	    do {
		*(str++) = c;
		c = mygetc(fi);
	    } while (isdigit(c) && ((c - '0') < 8));
	    ungetc(c, fi);
	    *str = NULL;			/* terminate the string */
	    return((parse_num(strarr, 8, num) == 1)? 1 : -1);

    }

    if (!isdigit(c)) return(-1);
    do {
	*(str++) = c;
	c = mygetc(fi);
    } while (isdigit(c));
    ungetc(c, fi);
    *str = NULL;			/* terminate the string */
    return((parse_num(strarr, 10, num) == 1)? 1 : -1);
}

	


/*
 * init_image - initialize the rom image structure.
 */
init_image(image)
struct rom_image *image;
{
    bzero((char *)image, sizeof(*image));
}


/*
 * make_rom - get rom information and build rom in image.
 */
make_rom(image)
struct rom_image *image;
{
    char c;
    
    bcopy((char *)AUTOSTRING, (char *)image->rom_id, 8);

    /* Type and serial number */
    qpr(("Rom type: %s.\n", image->rom_id));
top:
    qpr(("Serial Number: "));
    if ((c = fgetc(fi)) == '#') {	/* check for a comment */
        fgetl(line);
	goto top;
    } else
        ungetc(c,fi);
    if (feof(fi)) got_eof();
    if (fscanf(fi, "%ld", &image->serial_number) != 1)
	if (feof(fi))
	    got_eof();
	else
            fatal_error("Unable to read serial #\n");
    spr(("Serial #%ld\n", image->serial_number));
    fgetc(fi);					/* eat the terminator */

    /* Read the various sections into image */
    read_boot(image);
    read_dump(image);
    read_dev(image);
    read_nets(image);
    read_glob(image);
    read_text(image);
}


/*
 * output_rom - output the rom rom_image to the file out_p.
 */
output_rom(image)
struct rom_image *image;
{
    u_char rom[ROMLENMAX];	/* space to put the rom */
    struct rom_image *newim = (struct rom_image *)rom;
    long *argcb;		/* current arg place */
    u_char *valcb;		/* current value place */
    int argused = 0;		/* global count of used argv stuff */
    int i, i2;

#define romadr(offset) ((long *)((long)rom + (long)(offset)))

#define argscopy(ELE)							\
  newim->ELE = (char **)htonl(argcb);					\
  while (*(image->ELE)) {						\
      *romadr(argcb++) = (long)htonl(valcb);				\
      strcpy((char *)romadr(valcb), (char *)*(image->ELE));		\
      valcb += strlen(*(image->ELE++)) + 1;				\
  }									\
  *romadr(argcb++) = NULL;

#define argbcopy(ELE,AR)						\
   i = 0;								\
   while (*(image->ELE)) {						\
      while ((long)valcb & 0x3L) valcb++;	/* force alignment */	\
      *romadr(argcb++) = (long)htonl(valcb);				\
      bcopy(*(image->ELE++), romadr(valcb), AR[i]);			\
      valcb += AR[i++];							\
   }									\
   *romadr(argcb++) = NULL;						

    if (outform == f_ccode) pre_c_out(image);

    bzero((char *)rom, ROMLENMAX);
    /* in the following, NUMARGL is for the NULLS terminating the lists */
    argused = image->num_boot + image->num_dump + image->num_dev
	      + image->num_nets + image->num_glob + image->num_text + NUMARGL;
    argcb = (long *)sizeof(struct rom_image);
    valcb = (u_char *)((long)argcb + LP * argused);
    bcopy((char *)image, (char *)newim, 8);		/* copy the rom_id */
    newim->last_long	 = htonl((long)argcb);
    newim->serial_number = htonl((long)image->serial_number);
    newim->num_boot	 = htonl((long)image->num_boot);
    newim->num_dump	 = htonl((long)image->num_dump);
    newim->num_dev	 = htonl((long)image->num_dev);
    newim->num_nets	 = htonl((long)image->num_nets);
    newim->num_glob	 = htonl((long)image->num_glob);	    
    newim->num_text	 = htonl((long)image->num_text);
    argscopy(boot);
    argscopy(dump);
    newim->autoids = (struct autoid **)htonl(argcb);
    i = 0;
    while (*(image->autoids)) {
        while ((long)valcb & 0x3L) valcb++;	/* force alignment */
        *romadr(argcb++) = (long)htonl(valcb);
	bcopy(*(image->autoids++), romadr(valcb), devconflens[i]);
	for (i2 = LP; i2 < devconflens[i]; i2 += LP)
	    *romadr(valcb + i2) = htonl(*romadr(valcb + i2));
        valcb += devconflens[i++];
    }
    *romadr(argcb++) = NULL;						

    newim->net = (u_char **)htonl(argcb);
    argbcopy(net, netlens);
    newim->glob = (u_char **)htonl(argcb);
    argbcopy(glob, globlens);
    argscopy(text);

    newim->rom_size = htonl((long)valcb);
    while ((long)valcb & 0x3L) valcb++;	/* force alignment */

#if 0
printf("argcb = %ld, valcb = %ld\n", argcb, valcb);

    { int i, i2; putchar('\n');
    for (i = 0; i <= (long)argcb; i+= 8) {
        for (i2 = i; i2 <= i + 7; i2++)
	    printf("%3d 0x%02x ", rom[i2],rom[i2]);
        putchar('\n');
    }
    putchar('\n');
    for (i = (long)argcb; i <= (long)valcb; i+= 8) {
        for (i2 = i; i2 <= i + 7; i2++)
	    printf("%3d 0x%02x ", rom[i2],rom[i2]);
        putchar('\n');
    }
    putchar('\n');
    for (i = (long)argcb; i <= (long)valcb; i+= 16) {
        for (i2 = i; i2 <= i + 15; i2++)
	    printf("_%c", rom[i2]);
        putchar('\n');
    }
    }
#endif 0
    switch (outform) {
        case f_bin:   bin_out(rom, (int)valcb); break;
	case f_srec:  s_out(rom, (int)valcb); break;
	case f_ccode: c_out(rom, (int)valcb); break;
    }
}

bin_out(rom, len)
u_char *rom;
int len;
{	/* a FILE to fd int conversion and using write() would be faster, but
	   not as portable */
    int i;
    for (i = 0; i < len; i++)
        fputc(rom[i], fo);
    fclose(fo);
}
/*
 * up cases the contents of its argument, a string
 */
void upcase(str)
char *str;
{
    while (*str) {
	if (islower(*str)) *str -= ('a' - 'A');
	str++;
    }
}

s_out(rom, len)
u_char *rom;
int len;
{
#define VPL	10

    long address = 0L;
    u_char cksum;
    int i1, i2;
    for (i1 = 0; i1 < len; i1 += VPL) {
	sprintf(line, "S1%02X%04X", VPL + 3, address);
	upcase(line);
	fputs(line, fo);
	cksum = VPL + 3 + (address & 0xFF) + ((address >> 8) & 0xFF);
/* printf("addr %x->%x ", address, (address & 0xFF) + ((address >> 8) & 0xFF)); */
	for (i2 = i1; i2 < (i1 + VPL); i2++, address++) {
	    sprintf(line, "%02X", rom[i2]);
	    upcase(line);
	    fputs(line, fo);
	    cksum += rom[i2];
	}
/* printf("cksum = %x, ", cksum); */
/* printf("high %x, low %x, ", (cksum >> 8) &0xff, cksum & 0xff); */
	cksum = ~cksum & 0xff;
/* printf("new cksum = %x\n", cksum); */
	sprintf(line, "%02X\n", cksum);
	upcase(line);
	fputs(line, fo);
    }
    fprintf(fo, "S9030000FC\n");
    fclose(fo);
}


pre_c_out(image)
struct rom_image *image;
{
    fprintf(fo, "/* Fakerom generated by Mkrom version %d.%d */\n\n",
    	    VERSION,RELEASE);
    fprintf(fo, "#if 0\n");
    dump_core(fo, image);
    fprintf(fo, "\n#endif 0\n");

}

c_out(rom, len)
u_char *rom;
int len;
{
    int l1;
    char syscmd[1024];

    fprintf(fo, "struct fakerom { u_char rom[%d]; } fakerom = {{\n", len + 1);
    for (l1 = 0; l1 < len; l1++) {
	fprintf(fo, "0x%02x, ", rom[l1]);
	if (!((l1 + 1) % 11)) fputc('\n', fo);
    }
    fprintf(fo, "0 }};\n\n\n");
    fclose(fo);
}

/*
 *   read_boot - read boot string information into image
 */
read_boot(image)
struct rom_image *image;
{
    char **boots = (char **)calloc(MAXARGV, LP);
    image->boot = boots;

    qpr(("Enter boot strings (terminate with a .):\n"));
top:
    fgetl(line);
    if (feof(fi)) got_eof();
    if (line[0] == '#') goto top;			/* ignore comments */
    while (line[0] != DOT) {
        if (line[0] == '#') goto readln;		/* ignore comments */
	line[strlen(line) - 1] = 0;			/* erase newline */
	spr(("%s\n", line));
	enter_string(line, &boots);
	image->num_boot++;
readln:	fgetl(line);
        if (feof(fi)) got_eof();
    }
    *boots = NULL;

    if (image->num_boot == 0)
	fatal_error("No boot strings entered.");
    else
	qpr(("%d boot strings entered.\n", image->num_boot));
}

read_dump(image)
struct rom_image *image;
{
    char **dumps = (char **)calloc(MAXARGV, LP);
    image->dump = dumps;

    qpr(("Enter dump strings:\n"));
top:
    fgetl(line);
    if (feof(fi)) got_eof();
    if (line[0] == '#') goto top;			/* ignore comments */

    while (line[0] != DOT) {
        if (line[0] == '#') goto readln;		/* ignore comments */
	line[strlen(line) - 1] = 0;			/* erase newline */
	spr(("%s\n", line));
	enter_string(line, &dumps);
	image->num_dump++;
readln:	fgetl(line);
        if (feof(fi)) got_eof();
    }
    *dumps = NULL;
    
    qpr(("%d dump strings entered.\n", image->num_dump));
}

/*
 *   read_glob - read glob string information into image
 */
read_glob(image)
struct rom_image *image;
{
    int res;

    image->glob = (u_char **)calloc(MAXARGV, LP);

    qpr(("Enter glob info, any byte sequence.\n"));
    qpr(("Enter a . alone on a line to end the current glob\n"));
    qpr(("The first glob must be a well known gateway ip address\n"));
    while (1) {
	image->glob[numglobs] = (u_char *)malloc(NETMAX);
	qpr(("bytes: "));
	while (1) {
	    if (!(res=scan_num(&image->glob[numglobs][globlens[numglobs]],1)))
	        break;
	    if (res == -1) fatal_error("Illegal input");
	    globlens[numglobs]++;
	}

	numglobs++;
	image->num_glob++;
	
reask:	qpr(("Enter another glob group? (y/n):"));
	switch (mygetc(fi)) {
	    case 'n':
	    case 'N': mygetc(fi);
	    	      goto out;
	    case 'y':
	    case 'Y': mygetc(fi);
	    	      continue;
	    default : if (!i_flag) exit(-1);   /* unable to get a new input */
		      mygetc(fi);
		      goto reask;
	}
    }

out:
    if (image->num_glob < 1)
	fatal_error("No globs entered\n");
    else
	spr(("%d globs entered.\n", image->num_glob));
}


/*
 * read_dev - read in device autoids.
 */
void read_dev(image)
struct rom_image *image;
{
    long *dvcfs;
    struct autoid *ai;
    int res;
    char c;
    
    image->autoids = (struct autoid **)calloc(MAXARGV, LP);

    while (1) {
        while (1) {
	    qpr(("Enter device name, 4 chars max (e.x. il): "));
	    bzero(line, 4);		/* make sure of enough nulls */
	    fgetl(line);
	    if (feof(fi)) got_eof();
	    if (line[0] == '#') continue;		/* ignore comments */
	    line[strlen(line) - 1] = 0;			/* erase newline */
	    if (!strcmp(line, "quit")) {
		if (numdevconfs >= 0) return;
		fatal_error("No autoids entered\n");
	    }
	    if (strlen(line) <= 4)
	        break;
	    printf("Device name %s is too long\n", line);
	    if (!i_flag) exit(-1);	/* unable to get a new input */
	}

	ai = (struct autoid *)malloc(sizeof(struct autoid));
	image->autoids[image->num_dev] = ai;
	dvcfs = ai->devconf;
	numdevconfs++;
	devconflens[numdevconfs - 1] = 8;	/* account for devid  */
	image->num_dev++;
	bcopy(line, ai->id.devname, 4);

	while (1) {
	    qpr(("Enter device minor number (0 to 100) : "));
	    if ((c = fgetc(fi)) == '#') {	/* check for a comment */
        	fgetl(line);
		continue;
	    } else
	        ungetc(c,fi);
	    if (feof(fi)) got_eof();
	    if ( (fscanf(fi,"%ld",&(ai->id.devminor)) == 1) && 
	         (ai->id.devminor >= 0) && (ai->id.devminor < 101) )
	    {
		if (!i_flag) printf("%ld\n", ai->id.devminor);
	        break;
	    }
	    if (feof(fi)) got_eof();
	    printf("Illegal device number\n");
	    (void)getc(fi);	/* eat the return */
	    if (!i_flag) exit(-1);	/* unable to get a new input */
	}	

	qpr(("Enter longs, terminate by a '.'\n"));
	qpr(("longs: "));
	while (1) {	
	    if ((res = scan_num(dvcfs++, 0)) == 0) break;
	    if (res == -1) fatal_error("Illegal input");
	    devconflens[numdevconfs - 1] += LP;
	}
	qpr(("Enter 'quit' to quit entering autoids, or\n"));
    }
}


/*
 * read_nets - read ipaddress and other stuff.
 */
read_nets(image)
struct rom_image *image;
{
    int res;

    image->net = (u_char **)calloc(MAXARGV, LP);

    while (1) {
	image->net[numnets] = (u_char *)malloc(NETMAX);
	qpr(("Enter net info, any byte sequence, including keywords.\n"));
	qpr(("Enter a . alone on a line to end the current net\n"));
	qpr(("The first four bytes entered comprise the IP address\n"));
	qpr(("bytes: "));
	while (1) {
	    if (!(res = scan_num(&(image->net[numnets][netlens[numnets]]),1)))
	        break;
	    if (res == -1) fatal_error("Illegal input");
	    netlens[numnets]++;
	}
	if (netlens[numnets] < 4)
	    fatal_error("No IP address entered.");

	numnets++;
	image->num_nets++;
	
reask:	qpr(("Enter another net group? (y/n):"));
	switch (mygetc(fi)) {
	    case 'n':
	    case 'N': mygetc(fi);
	    	      goto out;
	    case 'y':
	    case 'Y': mygetc(fi);
	    	      continue;
	    default : if (!i_flag) exit(-1);	/* unable to get a new input */
		      mygetc(fi);
		      goto reask;
	}
    }

out:
    if (image->num_nets < 1)
	fatal_error("No nets entered\n");
    else
	spr(("%d subnet addresses entered.\n", image->num_nets));
}


/*
 * warning - report a non-fatal error and continue.
 */
warning(reason)
char *reason;
{
    fprintf(stderr, "Warning on line %d of %s: %s\n",
        line_count, inf, reason);
}


/*
 * read_text - read text of site description into string space.
 */
read_text(image)
struct rom_image *image;
{
    char **texts = (image->text = (char **)calloc(MAXARGV, LP));

    qpr(("Enter text:\n"));
top:
    fgetl(line);
    if (feof(fi)) got_eof();
    if (line[0] == '#') goto top;			/* ignore comments */
    spr(("%s", line));
    while (line[0] != DOT) {
        if (line[0] == '#') goto readln;		/* ignore comments */
	line[strlen(line) - 1] = 0;			/* erase newline */
	image->num_text++;
        enter_string(line, &texts);
readln:	fgetl(line);
        if (feof(fi)) got_eof();
	spr(("%s", line));
    }
    *texts = NULL;
}


/*
 * enter_string
 */
enter_string(string, argv)
char *string;
char ***argv;
{
    int len = strlen(string);

    **argv = malloc(len + 1);
    strcpy(**argv, string);
    (*argv)++;
}

dump_core(dfp, image)
FILE *dfp;	/* opened file to dump into */
struct rom_image *image;
{
    int count1, count2;
    char **c;
    struct autoid **ai;

    fprintf(dfp, "Dump by types.\n");

    fprintf(dfp, "type: %.8s.\n", image->rom_id);
    fprintf(dfp, "size: 0x%x.\n", image->rom_size);
/*  fprintf(dfp, "lastlong: %d.\n", image->last_long);*/
    fprintf(dfp, "serial number: %ld.\n", image->serial_number);

    fprintf(dfp, "num boot: %d.\n", image->num_boot);
    for (c = image->boot;  *c; c++)
        fprintf(dfp, "\t<%s>\n", *c);

    fprintf(dfp, "num_dump: %d.\n", image->num_dump);
    for (c = image->dump;  *c; c++)
        fprintf(dfp, "\t<%s>\n", *c);

    fprintf(dfp, "num_dev: %d.\n", image->num_dev);
    for (ai = image->autoids, count1 = 0;  *ai; ai++, count1++) {
        fprintf(dfp, "\tdevice <%.4s> minor <%ld> longs: <", (*ai)->id.devname,
		(*ai)->id.devminor);
	for (count2 = 0; count2 < ((devconflens[count1] >> 2) - 2); count2++)
	    fprintf(dfp, "[0x%x %ld]", (*ai)->devconf[count2],
	    			       (*ai)->devconf[count2]);
	fprintf(dfp, ">\n");
    }

    fprintf(dfp, "num_nets: %d.\n", image->num_nets);
    for (count1 = 0; image->net[count1]; count1++) {
	fprintf(dfp, "\tcnt: %d <", netlens[count1]);
	for (count2 = 0; count2 < netlens[count1]; count2++)
	    fprintf(dfp, "[0x%x %d]", image->net[count1][count2],
	    			      image->net[count1][count2]);
	fprintf(dfp, ">\n");
    }

    fprintf(dfp, "num_glob: %d.\n", image->num_glob);
    for (count1 = 0; image->glob[count1]; count1++) {
	fprintf(dfp, "\tcnt: %d <", globlens[count1]);
	for (count2 = 0; count2 < globlens[count1]; count2++)
	    fprintf(dfp, "[0x%x %d]", image->glob[count1][count2],
	    			      image->glob[count1][count2]);
	fprintf(dfp, ">\n");
    }
    
    fprintf(dfp, "num_text: %d.\n", image->num_text);
    for (c = image->text;  *c; c++)
        fprintf(dfp, "\t<%s>\n", *c);
	

}
/*
 * write out what was entered in human readable format
 */
dump_image(image)
struct rom_image *image;
{
    FILE *dfp;
    char dfname[MAX_LINE];		/* dump file name */

    printf("Dump file name: ");
    fgetl(dfname);
    if (feof(fi)) got_eof();
    dfname[strlen(dfname) - 1] = 0;			/* erase newline */
    if ((dfp = fopen(dfname, "w")) == NULL) {
	fprintf(stderr, "Cannot open file %s for write.\n", dfname);
	dfp = stdout;
    }
    dump_core(dfp, image);
    fclose(dfp);
}


/*
 * lookup_kw - find a keyword and return its corresponding integer value
 * returns -1 on an error.
 * assumes that at least one keyword is defined
 */
int lookup_kw(kw)
char *kw;
{
    struct kwsT *kwsp = kws;

    do {
        if (!strcmp(kw, kwsp->kws)) return(kwsp->kwv);
    } while ((++kwsp)->kwv != KWLISTEND);	/* hack */

    return(-1);		/* No matching keyword error */
}

/*
 * fatal_error - report a fatal error, the line it occurred on, and the
 *  reason.
 * doesn't return
 */
int fatal_error(reason)
char *reason;
{
    fprintf(stderr, "\nFatal error on line %d of %s: %s\n",
        line_count, inf, reason);
    fclose(fi);
    fclose(fo);
    unlink(outf);		/* erase the output file for make */
    exit(-1);
}

int mygetc(inf)
FILE *inf;
{
    int i;
    
    while (1) {
        if ((i = getc(inf)) == EOF) got_eof();

        if (i != '#') {
	    if (!i_flag) putchar(i);
            return(i);
	}
    
        while ((i = getc(inf)) != EOF && (i != CR))
	    ;

        if (feof(inf)) got_eof();
    }
}
