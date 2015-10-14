#ifndef     _UTIL_HEAP_H_
#define     _UTIL_HEAP_H_

#include    <stdlib.h>
#include    <string.h>
#include    <config.h>

#define     HEAP_INIT_SIZE      IZM_HEAP_INIT_SIZE
#define     HEAP_MAX_SIZE       IZM_HEAP_MAX_SIZE

typedef int (*compare_func_t)(const void *, const void *);

struct izm_minheap {
    void **heap_entries;
    unsigned heap_size, heap_curr;
    compare_func_t heap_compare;
};

struct izm_minheap *heap_create(compare_func_t cmp_func);
void *heap_peek_min(struct izm_minheap *h);
void *heap_extract_min(struct izm_minheap *h);
void heap_insert(void *e, struct izm_minheap *h);
void heap_delete(void *e, struct izm_minheap *h);

#endif      // _UTIL_HEAP_H_
