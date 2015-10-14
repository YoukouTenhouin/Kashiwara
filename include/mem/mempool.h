#ifndef __IZUMO_MEM_MEMPOOL_H__
#define __IZUMO_MEM_MEMPOOL_H__

#include <config.h>
#include <stdint.h>
#include <stdlib.h>
#include <misc/util_list.h>

struct em_large {
	list_ctl_t lctl;
	char blk[];
};

struct izm_mempool {
	uint32_t size, pos, failed, nfail, nlarge;
	struct izm_mempool *major, *current, *next;
	list_ctl_t lctl, large;
	char pool[];
};

enum izm_mempool_err {
	e_mpool_noerr,
	e_mpool_nocore,
};

#define     MAX_NFAILURE    IZM_MEMPOOL_NFAILURE

struct izm_mempool *izm_mempool_get(uint32_t nlarge, uint32_t nfail);
void izm_mempool_drain(struct izm_mempool *mp);
void izm_mempool_init(uint32_t cap, uint32_t psize);
enum izm_mempool_err izm_mempool_geterr(void);
void *izm_mempool_alloc(struct izm_mempool *mp, size_t size);

#endif	/* __IZUMO_MEM_POOL_H__ */
