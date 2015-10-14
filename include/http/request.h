#ifndef __IZUMO_HTTP_REQUEST_H__
#define __IZUMO_HTTP_REQUEST_H__

#include <misc/string.h>
#include <http/header.h>
#include <stdint.h>

enum METHODS {
	M_GET = 1,
	M_POST,
	M_HEAD,
	M_PUT,
	M_DELETE,
	M_TRACE,
	M_CONNECT,
	M_OPTIONS
};

enum REQ_PARSER_ERRORS {
	REQ_PARSER_EUNKNOWNMETHOD,
	REQ_PARSER_EBADINPUT
};

struct izm_http_request {
	int		method;
	izm_string	url;
	int		major_ver;
	int		minor_ver;
	struct izm_http_header *headers;
};

struct izm_http_req_parser {
	int	state;
	char	*mark, *i;
	
	struct izm_mempool *pool;
	struct izm_http_request request;
};

static inline void
izm_http_request_init(
	struct izm_http_request *req
	)
{
	req->method = 0;
	izm_string_init(&(req->url));
	req->major_ver = req->minor_ver = 0;
	req->headers = NULL;
}

static inline void
izm_http_req_parser_init(
	struct izm_http_req_parser *parser
	)
{
	parser->state = 0; 	/* S_INIT */
	parser->i = NULL;
	parser->mark = NULL;
	parser->pool = NULL;
	izm_http_request_init(&(parser->request));
}

int izm_http_req_parser_run(struct izm_http_req_parser*,
			    char*, size_t);

#endif	/* __IZUMO_HTTP_REQUEST_H__ */
