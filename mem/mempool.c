#include <mem/mempool.h>

static enum izm_mempool_err local_err = e_mpool_noerr;
static __thread
struct {
	uint32_t cap, psize, size;
	list_ctl_t list;
} local_pool;

static inline
struct em_large *em_large_new(uint32_t size) {
	struct em_large *lb = 
		(struct em_large *) malloc(sizeof(struct em_large) + sizeof(char) * size);
	local_err = (lb != NULL) ? e_mpool_noerr : e_mpool_nocore;
	return lb;
}

static inline
void em_large_del(struct em_large *lb) {
	free(lb);
}

static
struct izm_mempool *izm_mempool_new(uint32_t size, uint32_t nlarge, uint32_t nfail) {
	struct izm_mempool *mp = 
		(struct izm_mempool *) malloc(sizeof(struct izm_mempool) + sizeof(char) * size);
	if (mp != NULL) {
		(mp->size = size), (mp->nlarge = nlarge), (mp->nfail = nfail);
		(mp->pos = 0), (mp->failed = 0);
		(mp->major = mp), (mp->current = mp), (mp->next = NULL);
		init_list_head(&(mp->large));
		local_err = e_mpool_noerr;
	} else {
		local_err = e_mpool_nocore;
	}
	return mp;
}

static
void izm_mempool_del(struct izm_mempool *mp) {
	// note: large blocks are attached on the major pool only
	list_ctl_t *p, *h = &(mp->major->large), *t;
	list_foreach_remove(p, h, t) {
		list_del(t);
		struct em_large *l = container_of(t, struct em_large, lctl);
		em_large_del(l);
	}

	struct izm_mempool *major, *minor, *tmp;
	(major = mp->major), (minor = major->next);
	while (minor != NULL) {
		(tmp = minor), (minor = minor->next);
		free(tmp);
	}
	free(major);
}

void *izm_mempool_alloc(struct izm_mempool *mp, size_t size) {
	char *blk = NULL;
	(void)((mp->failed > mp->nfail) && (mp = mp->major->current));
	if (size < mp->nlarge) {
		// small block: use the pool (if there's enough space available)
		// TODO align
		uint32_t asize = size;
		// try to align, but unaligned case is also acceptable
		uint32_t finsize = (mp->size - mp->pos >= asize) ?
			(asize) :
			((mp->size - mp->pos >= size) ? (size) : (0));
		if (finsize) {
			// okay, we have the block
			blk = &(mp->pool[mp->pos]);
			mp->pos += finsize;
		} else {
			// failed - 
			//  0. mp->failed += 1
			//  1. if mp->next is not NULL, look for it
			//  2. if mp->next is NULL, create one and look for it
			//  3. check mp->failed and set mp->major->current if necessary
			mp->failed++;
			if (mp->next == NULL) {
				struct izm_mempool *p_minor = 
					izm_mempool_get(mp->major->nlarge, mp->major->nfail);
				if (p_minor != NULL) {
					p_minor->major = mp->major;
					mp->next = p_minor;
				} else {
					local_err = e_mpool_nocore;
					return NULL;
				}
			}
			blk = izm_mempool_alloc(mp->next, size);
			if (mp->failed > mp->nfail)
				mp->major->current = mp->next;
		}
	} else {
		// large block: note that we should use mp->major
		struct em_large *lb = em_large_new(size);
		blk = (lb != NULL) ? 
			({ mp = mp->major; list_add_tail(&(lb->lctl), &(mp->large)); lb->blk; }) :
			(NULL);
	}
	local_err = (blk != NULL) ? e_mpool_noerr : e_mpool_nocore;
	return (void *) blk;
}

struct izm_mempool *izm_mempool_get(uint32_t nlarge, uint32_t nfail) {
	struct izm_mempool *mp = NULL;
	if (local_pool.size) {
		local_pool.size--;
		list_ctl_t *p = local_pool.list.next;
		list_del(p);
		mp = container_of(p, struct izm_mempool, lctl);
		(mp->current = mp->major = mp), (mp->next = NULL);
		(mp->nfail = 0), (mp->pos = 0);
	} else {
		mp = izm_mempool_new(local_pool.psize, nlarge, nfail);
	}
	return mp;
}

void izm_mempool_drain(struct izm_mempool *mp) {
	if (local_pool.size < local_pool.cap) {
		mp = mp->major;
		list_ctl_t *p, *t, *h = &(mp->large);
		list_foreach_remove(p, h, t) {
			list_del(t);
			struct em_large *l = container_of(t, struct em_large, lctl);
			em_large_del(l);
		}
		struct izm_mempool *mpitr = mp;
		while (mpitr != NULL) {
			list_add_tail(&(mpitr->lctl), &(local_pool.list));
			local_pool.size++;
			mpitr = mpitr->next;
		}
	} else {
		izm_mempool_del(mp);
	}
}

void izm_mempool_init(uint32_t cap, uint32_t psize) {
	(local_pool.cap = cap), (local_pool.psize = psize);
	init_list_head(&(local_pool.list));
}

enum izm_mempool_err izm_mempool_geterr(void) {
	enum izm_mempool_err errsv = local_err;
	return errsv;
}
