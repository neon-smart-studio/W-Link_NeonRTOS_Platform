#include <stdint.h>
#include <stddef.h>

extern void *__real_memset(void *s, int c, size_t n);

void *__wrap_memset(void *s, int c, size_t n)
{
    uintptr_t a = (uintptr_t)s;

    if (a >= 0x60000000UL)
    {
        volatile uint8_t *p = (volatile uint8_t *)s;
        uint8_t v = (uint8_t)c;

        while (n--)
        {
            *p++ = v;
        }

        return s;
    }

    return __real_memset(s, c, n);
}