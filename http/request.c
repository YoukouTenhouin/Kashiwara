#include <http/request.h>
#include <http/header.h>
#include <misc/constants.h>
#include <misc/macro_strequ.h>
#include <mem/mempool.h>
#include <stdint.h>

int _CTL[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 
};

int _SEPR[] = {	
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0
};

#define CRLF(s) IZM_STREQU2(s, "\r\n")
#define CRLF2(s) IZM_STREQU4(s, "\r\n\r\n")

#define WHITESPACE(c) ((c) == '\t' || (c) == ' ')
#define LWS(c) (WHITESPACE(c) || (c) == '\r' || (c) == '\n')
#define DIGIT(c) ((c) >= '0' && (c) <= '9')
#define CTL(c) (_CTL[(int)(c)])
#define SEPR(c) (_SEPR[(int)(c)])
#define TOKEN(c) (!CTL(c) && !SEPR(c))
#define UPALPHA(c) ((c) >= 'A' && (c) <= 'Z')
#define LOALPHA(c) ((c) >= 'a' && (c) <= 'z')
#define ALPHA(c) (UPALPHA(c) || LOALPHA(c))

#define ITER_FORWARD do {				\
		if (++i >= end) goto _EXIT_CONTINUE;	\
	} while(0)

#define ENSURE_INPUT_LONG_ENOUGH(n) do {		\
		if (i + n > end) goto _EXIT_CONTINUE;	\
	} while(0)
		

void izm_http_request_init(struct izm_http_request*);
void izm_http_req_parser_init(struct izm_http_req_parser*);

int
izm_http_req_parser_run(
	struct izm_http_req_parser *parser,
	char *input_data,
	size_t size
	)
{
	char *i = parser->i, *end = input_data + size;
	enum STATES {
		S_INIT = 0,
		S_METHOD,
		S_URL,
		S_VERSION,
		S_VERSION_MAJOR,
		S_VERSION_MINOR,
		S_REQUEST_LINE_END,
		S_HEADER_FIELD,
		S_HEADER_VALUE,
		S_HEADER_END,
		S_FINISH,
		S_FAILED
	};

	switch(parser->state) {
	case S_INIT:
		i = input_data;
		parser->state = S_METHOD;
		/* Fall down */
	case S_METHOD:
		goto _METHOD;
	case S_URL:
		goto _URL;
	case S_VERSION:
		goto _VERSION;
	case S_VERSION_MAJOR:
		goto _VERSION_MAJOR;
	case S_VERSION_MINOR:
		goto _VERSION_MINOR;
	case S_REQUEST_LINE_END:
		goto _REQUEST_LINE_END;
	case S_HEADER_FIELD:
		goto _HEADER_FIELD;
	case S_HEADER_VALUE:
		goto _HEADER_VALUE;
	case S_HEADER_END:
		goto _HEADER_END;
	case S_FINISH:
		return IZM_OK;
	case S_FAILED:
		return IZM_ERROR;
	}

_METHOD:
	parser->mark = i;
	while((i - parser->mark) <= 7 && ALPHA(*i)) {
		if(LOALPHA(*i)) *i += 32;
		ITER_FORWARD;
	}

	if(*i != ' ') goto _EXIT_BAD_INPUT;

	if(IZM_STREQU3(parser->mark, "GET"))
		parser->request.method = M_GET;
	else if(IZM_STREQU3(parser->mark, "PUT"))
		parser->request.method = M_PUT;
	else if(IZM_STREQU4(parser->mark, "POST"))
		parser->request.method = M_POST;
	else if(IZM_STREQU4(parser->mark, "HEAD"))
		parser->request.method = M_HEAD;
	else if(IZM_STREQU5(parser->mark, "TRACE"))
		parser->request.method = M_TRACE;
	else if(IZM_STREQU6(parser->mark, "DELETE"))
		parser->request.method = M_DELETE;
	else if(IZM_STREQU7(parser->mark, "CONNECT"))
		parser->request.method = M_CONNECT;
	else if(IZM_STREQU7(parser->mark, "OPTIONS"))
		parser->request.method = M_OPTIONS;
	else
		goto _EXIT_BAD_INPUT;

	ITER_FORWARD;
	parser->mark = i;
	parser->state = S_URL;	
	/* Fall down */
_URL:
	while(!CTL(*i) && *i != ' ') ITER_FORWARD;
	izm_string_set(
		&(parser->request.url),
		parser->mark,
		i - parser->mark
		);

	ITER_FORWARD;
	if (*i != 'H') goto _EXIT_BAD_INPUT;
	parser->state = S_VERSION;
	/* Fall down */
	
_VERSION:
	ENSURE_INPUT_LONG_ENOUGH(6);
	if(!IZM_STREQU5(i, "HTTP/"))
		goto _EXIT_BAD_INPUT;	
	i += 5;
	parser->state = S_VERSION_MAJOR;
	/* Fall down */
	
_VERSION_MAJOR:
	while(DIGIT(*i)) {
		parser->request.major_ver *= 10;
		parser->request.major_ver += *i - '0';
		ITER_FORWARD;
	}
	ITER_FORWARD;
	parser->state = S_VERSION_MINOR;
	/* Fall down */

_VERSION_MINOR:
	while(DIGIT(*i)) {
		parser->request.minor_ver *= 10;
		parser->request.minor_ver = *i - '0';
		ITER_FORWARD;
	}
	if(*i != '\r') goto _EXIT_BAD_INPUT;
	parser->state = S_REQUEST_LINE_END;
	/* Fall down */

_REQUEST_LINE_END:
	ENSURE_INPUT_LONG_ENOUGH(3);
	if (!CRLF(i)) goto _EXIT_BAD_INPUT;
	i += 2;
	parser->state = S_HEADER_FIELD;
	parser->mark = i;
	/* Fall down */
	
_HEADER_FIELD:
	while(TOKEN(*i)) ITER_FORWARD;
	if (*i != ':') goto _EXIT_BAD_INPUT;
	
	struct izm_http_header *header = izm_mempool_alloc(
		parser->pool, sizeof(struct izm_http_header)
		);
	izm_http_header_init(header);
	izm_string_set(&(header->field),
		       parser->mark, i - parser->mark);
	if(parser->request.headers)
		izm_http_header_insert(header, parser->request.headers);
	else
		parser->request.headers = header;

	ITER_FORWARD;
	while(WHITESPACE(*i)) ITER_FORWARD;
	parser->mark = i;
	parser->state = S_HEADER_VALUE;
	/* Fall down */

_HEADER_VALUE:
	while(!CTL(*i) || WHITESPACE(*i)) ITER_FORWARD;
	if(*i != '\r') goto _EXIT_BAD_INPUT;
	
	char *val_end = i;
	while(LWS(*(val_end-1))) --val_end;

	izm_string *val = &(parser->request.headers->prev->value);
	izm_string_set(val, parser->mark, val_end - parser->mark);
	
	parser->state = S_HEADER_END;
	/* Fall down */

_HEADER_END:
	ENSURE_INPUT_LONG_ENOUGH(4);
	if (CRLF2(i))
		goto _EXIT_OK;
	else if (CRLF(i)) {
		i += 2;
		parser->mark = i;
		parser->state = S_HEADER_FIELD;
		goto _HEADER_FIELD;
	} else
		goto _EXIT_BAD_INPUT;
	
_EXIT_OK:
	parser->state = S_FINISH;
	return IZM_OK;

_EXIT_CONTINUE:
	parser->i = i;
	return IZM_CONTINUE;

_EXIT_BAD_INPUT:
	parser->state = S_FAILED;
	return IZM_ERROR;

}
