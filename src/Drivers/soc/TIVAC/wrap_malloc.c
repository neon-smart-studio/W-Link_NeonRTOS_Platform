#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct {
    size_t sz;
    unsigned char payload[];
} blk_hdr_t;

static void *hdr_alloc(size_t n) {
    blk_hdr_t *h = (blk_hdr_t *) pvPortMalloc(sizeof(blk_hdr_t)+n);
    if (!h) return NULL;
    h->sz = n;
    return h->payload;
}
static void hdr_free(void *p) {
    if (!p) return;
    blk_hdr_t *h = (blk_hdr_t*)((unsigned char*)p - offsetof(blk_hdr_t, payload));
    vPortFree(h);
}
static size_t hdr_size(void *p) {
    blk_hdr_t *h = (blk_hdr_t*)((unsigned char*)p - offsetof(blk_hdr_t, payload));
    return h->sz;
}

/* Non-reentrant */
void *__wrap_malloc(size_t size) { return hdr_alloc(size); }
void __wrap_free(void *ptr) { hdr_free(ptr); }

void *__wrap_calloc(size_t n, size_t s)
{
    if (n == 0 || s == 0) {
        // C 標準允許 calloc(0, ?) 回傳可釋放或 NULL，這裡直接回最小 0→1 bytes
        n = s = 1;
    }

    if (s != 0 && n > (SIZE_MAX / s)) {
        // overflow
        return NULL;
    }

    size_t total = n * s;
    void *p = hdr_alloc(total);
    if (p) memset(p, 0, total);
    return p;
}


void *__wrap_realloc(void *ptr, size_t ns) {
    if (!ptr) return hdr_alloc(ns);
    if (ns == 0) { hdr_free(ptr); return NULL; }
    size_t os = hdr_size(ptr);
    size_t copy = (os < ns) ? os : ns;
    void *n = hdr_alloc(ns);
    if (!n) return NULL;
    memcpy(n, ptr, copy);
    hdr_free(ptr);
    return n;
}

/* Reentrant */
struct _reent;
void *__wrap__malloc_r(struct _reent *r, size_t s)      { (void)r; return __wrap_malloc(s); }
void __wrap__free_r(struct _reent *r, void *p)          { (void)r; __wrap_free(p); }
void *__wrap__calloc_r(struct _reent *r, size_t n, size_t s){ (void)r; return __wrap_calloc(n,s); }
void *__wrap__realloc_r(struct _reent *r, void *p, size_t s){ (void)r; return __wrap_realloc(p,s); }
