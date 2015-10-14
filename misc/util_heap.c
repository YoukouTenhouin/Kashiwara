#include    <stdio.h>
#include    <misc/util_heap.h>

#define GCC_SHUT_UP(exp) ((void)(exp))

// note that we use entries[1] as the root

struct izm_minheap *heap_create(compare_func_t cmp_func) {
    struct izm_minheap *h = (struct izm_minheap *) malloc(sizeof(struct izm_minheap));
    if (h == NULL)
        return NULL;
    h->heap_entries = (void **) malloc(sizeof(void*) * HEAP_INIT_SIZE);
    if (h->heap_entries == NULL) {
        free(h);
        return NULL;
    }
    h->heap_size = HEAP_INIT_SIZE;
    h->heap_curr = 0;
    h->heap_compare = cmp_func;
    memset(h->heap_entries, sizeof(void *) * HEAP_INIT_SIZE, 0);
    return h;
}


static
void min_heapify(unsigned i, struct izm_minheap *h) {
    unsigned l = 2 * i, r = 2 * i + 1, small;
    void *tmp_entry;
    if ((l <= h->heap_curr) && (h->heap_compare(h->heap_entries[l], h->heap_entries[i]) == -1))
        small = l;
    else
        small = i;
    if ((r <= h->heap_curr) && (h->heap_compare(h->heap_entries[r], h->heap_entries[small]) == -1))
        small = r;
    if (small != i) {
        tmp_entry = h->heap_entries[i];
        h->heap_entries[i] = h->heap_entries[small];
        h->heap_entries[small] = tmp_entry;
        min_heapify(small, h);
    }
}

void *heap_peek_min(struct izm_minheap *h) {
    return (h->heap_curr > 0) ? h->heap_entries[1] : NULL;
}

void *heap_extract_min(struct izm_minheap *h) {
    void *min_elt = h->heap_entries[1];
    if (h->heap_curr == 0)
        return NULL;
    h->heap_entries[1] = h->heap_entries[(h->heap_curr)--];
    min_heapify(1, h);
    return min_elt;
}

void heap_insert(void *e, struct izm_minheap *h) {
    unsigned j;
    void *tmp_entry;
    if (h->heap_curr >= h->heap_size) {
        __auto_type basesize = sizeof(void *) * h->heap_size;
        void **tmp_entries = (void **) realloc(h->heap_entries, basesize * 2);
        if (tmp_entries != NULL) {
            h->heap_entries = tmp_entries;
            h->heap_size *= 2;
        } else {
            return;
        }
    }
    h->heap_entries[++(h->heap_curr)] = e;
    for (j = h->heap_curr; j > 1 && h->heap_compare(h->heap_entries[j/2], h->heap_entries[j]) == 1; j /= 2) {
        tmp_entry = h->heap_entries[j];
        h->heap_entries[j] = h->heap_entries[j/2];
        h->heap_entries[j/2] = tmp_entry;
    }
}

void heap_delete(void *e, struct izm_minheap *h) {
    if (h->heap_curr == 0)
        return;
    unsigned j;
    void *tmp_entry;
    int retry = 1;
    /* SHUT UP */
    GCC_SHUT_UP(j);
    GCC_SHUT_UP(tmp_entry);
    GCC_SHUT_UP(retry);
    // XXX no error check - use with care
    unsigned idx;
    for (idx = 1; idx <= h->heap_curr; idx++)
        if (e == h->heap_entries[idx])
            break;
    if (idx > h->heap_curr)
        return;
    h->heap_entries[idx] = h->heap_entries[h->heap_curr];
    (h->heap_curr)--;
    min_heapify(idx, h);
}
