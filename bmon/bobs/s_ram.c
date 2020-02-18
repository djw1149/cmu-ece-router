/*
    File: s_ram.c.
    Contents: procedures that manipulate the contents of the protected
    	ram segment to provide buffers for information that survives
	through a crash.
*/
#include "s_ram.h"

s_ram_init(start_pointer)
int start_pointer;
{
    register struct s_ram_element *sr = (struct s_ram_element *)start_pointer;

    if ((sr->lower_guard != 0) | (sr->upper_guard != 0)) {
	printf("\nInitializing protected ram.");
	bzero (sr, sizeof (struct s_ram_element));
	return (1);
    }

    if (sr->total != (sr->success + sr->fail)) {
	printf("\nProtected ram data inconsistency.");
	reboot();
	return (0);
    }

    if (sr->cksm != -(unsigned int)(sr->total + sr->success + sr->fail)) {
	printf("\nProtected Ram checksum error.");
	reboot();
	return (0);
    }
}
