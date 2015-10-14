#ifndef __IZUMO_HTTP_RESPONSE_H__
#define __IZUMO_HTTP_RESPONSE_H__

#include <http/header.h>

struct izm_http_response {
	int code;
	int major_ver;
	int minor_ver;
	struct izm_http_header *headers;
};

struct izm_http_response_writer {
	int state;
	size_t remaining;
	struct izm_http_header *header;
	struct izm_http_response *response;
};

struct izm_http_response_parser {
};

static inline void
izm_http_response_writer_init(
	struct izm_http_response_writer *w)
{
	w->state = 0;		/* S_INIT */
	w->header = NULL;
	w->response = NULL;
	w->remaining = 0;
}

static inline void
izm_http_response_writer_set_response(
	struct izm_http_response_writer *w,
	struct izm_http_response *r)
{
	w->response = r;
	w->header = r->headers;	
}

const char* izm_get_response_status_str(int);
int izm_http_response_write(struct izm_http_response_writer*, char*, size_t, size_t*);

#endif /* __IZUMO_HTTP_RESPONSE_H__ */
