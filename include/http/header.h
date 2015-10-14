#ifndef __IZUMO_HTTP_HEADERS_H__
#define __IZUMO_HTTP_HEADERS_H__

#include <misc/string.h>

struct izm_http_header {
	izm_string field;
	izm_string value;
	struct izm_http_header *prev;
	struct izm_http_header *next;
};

inline void
izm_http_header_init(struct izm_http_header *header)
{
	header->prev = header->next = header;
}

inline void
izm_http_header_insert(struct izm_http_header *header,
		  struct izm_http_header *headers)
{
	header->prev = headers->prev;
	header->next = headers;
	headers->prev = header;
	header->prev->next = header;
}

inline struct izm_http_header*
izm_http_header_detach_next(struct izm_http_header *prev)
{
	struct izm_http_header *ret = prev->next;
	prev->next = prev->next->next;
	prev->next->prev = prev;
	ret->prev = ret->next = ret;
	return ret;
}

#endif	/* __IZUMO_HTTP_HEADERS_H__ */
