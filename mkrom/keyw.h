#define KWLEN	16		/* max length of keyword */
struct kwsT {
    int kwv;			/* keyword value */
    char kws[KWLEN];		/* keyword string */
};

#define KWLISTEND 0xabcdfade	/* hack */

/* from ../h/proto.h */
#define NPR 6


/*
 * keyword definitions
 */
struct kwsT kws[] = {
/*
 *  Protocol/identification option codes
 */
 {(NPR+0), "PI_HRD"},
 {(NPR+1), "PI_GWAY"},
 {(NPR+2), "PI_REDIR"},
 {(NPR+3), "PI_CABLE"},
 {(NPR+4), "PI_AUTH"},
 {(NPR+5), "PI_RESTR"},
 {(NPR+6), "PI_FLAG"},
 {(NPR+7), "PI_C_ISME"},
 {(NPR+8), "PI_C_ISPHYS"},
 {(NPR+9), "PI_C_END"},
 {(NPR+10), "PI_DONE"},
 {(NPR+11), "PI_ROM"},
 {(NPR+12), "PI_END"},
 {(NPR+13), "PI_SUB"},
 {KWLISTEND, NULL}
};
