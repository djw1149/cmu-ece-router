#include "types.h"
#include "globals.h"

/* OLD BUGGY STUFF:
 *  caddr_t heap_end = (caddr_t)0x7ff;
 *  caddr_t heap_begin = (caddr_t)0x404;
 */

hinit(heap_begin)
caddr_t heap_begin;
{
    struct globals *g = globals;

    g->heapp = heap_begin;
    g->heap_end = heap_begin + HEAPSIZE;
}

caddr_t
halloc(n)
register n;
{
    register struct globals *g = globals;
    register caddr_t addr = g->heapp;

    if (n <= 0)
    	return (0);
    if (g->heapp + n > g->heap_end)
    	return (0);

    g->heapp += n;
    return (addr);
}
