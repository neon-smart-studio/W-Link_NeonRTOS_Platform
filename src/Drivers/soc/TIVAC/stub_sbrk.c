#include <sys/types.h>
#include <errno.h>

caddr_t _sbrk(int incr)
{
    (void)incr;
    errno = ENOMEM;
    return (caddr_t)-1;
}
