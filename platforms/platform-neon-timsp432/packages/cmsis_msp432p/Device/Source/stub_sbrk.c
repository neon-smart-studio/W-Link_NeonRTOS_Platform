#include <sys/types.h>

caddr_t _sbrk(int incr)
{
    (void)incr;
    return (caddr_t)-1;
}
