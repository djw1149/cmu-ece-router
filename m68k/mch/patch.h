#include "../../h/aconf.h"
#define MAX_PATCH	30 	/* maximum bytes in a patch */

static struct patch_table_t {
    u_long serial_num;		/* serial number -1 is the end of the table */
    u_long minor_num;
    u_char new_pi[MAX_PATCH];	/* new pi structure */
} patch_table[] =
{
/*	102,	0, {PR_IP,128,2,251,236,(u_long)PI_END},   test case */
	-1,	0, {0}			/* end of table */
};
