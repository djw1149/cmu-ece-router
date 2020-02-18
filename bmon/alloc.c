#include "types.h"
#include "globals.h"

caddr_t heap_end = (caddr_t)0x7ff;
caddr_t heap_begin = (caddr_t)0x400;

hinit()
{
    struct globals *g = globals;

    g->heapp = heap_begin;
}

caddr_t
halloc(n)
register n;
{
    register struct globals *g = globals;

    if (g->heapp + n > heap_end)
    	return (0);
    return (g->heapp += n);
}
