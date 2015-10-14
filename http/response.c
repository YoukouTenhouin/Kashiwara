#include <http/response.h>
#include <misc/constants.h>
#include <misc/string.h>
#include <string.h>

static char *izm_res_status_1xx[] = {
	"Continue",
	"Switching Protocols"
};

static char *izm_res_status_2xx[] = {
	"Established",
	"Created",
	"Accepted",
	"Non-Authoritative Information",
	"No Content",
	"Reset Content",
	"Partial Content"
};

static char *izm_res_status_3xx[] = {
	"Multiple Choices",
	"Moved Permanently",
	"Found",
	"See Other",
	"Not Modified",
	"Use Proxy",
	"Switch Proxy",
};

static char *izm_res_status_4xx[] = {
	"Bad Request",
	"Unauthorized",
	"Payment Required",
	"Forbidden",
	"Not Found",
	"Method Not Allowed"
	"", "", "", "", "", "", "",
	"Header Too Large"
};

static char *izm_res_status_5xx[] = {
	"Internal Server Error",
	"Not Implemented",
	"Bad Gateway",
	"Service Unavaliable",
	"Gateway Timeout",
	"HTTP Version Not Supported"
};

static char **izm_res_status[] = {
	NULL,
	izm_res_status_1xx,
	izm_res_status_2xx,
	izm_res_status_3xx,
	izm_res_status_4xx,
	izm_res_status_5xx
};

void izm_http_response_writer_init(struct izm_http_response_writer*);
void izm_http_response_writer_set_response(
	struct izm_http_response_writer*,
	struct izm_http_response*);

const char*
izm_get_response_status_str(int code)
{
	int chead = code / 100;
	int ctail = code - 100*chead;
	return izm_res_status[chead][ctail];
}

int
izm_http_response_write(struct izm_http_response_writer *w,
			char *buffer, size_t buffer_size,
			size_t *writed)
{
	enum STATES {
		S_INIT,
		S_HTTP_VER,
		S_STATUS_CODE,
		S_STATUS_STR,
		S_HEADER_FIELD,
		S_HEADER_VALUE,
		S_FINISH
	};
	
	char *i = buffer, *end = buffer + buffer_size;

	if(buffer_size < 128)
		return IZM_ERROR;

	switch(w->state) {
	case S_INIT:
		w->state = S_HTTP_VER;
		/* Fall down */

	case S_HTTP_VER:
		strcpy(i, "HTTP/");		
		i += 5;
		int mav = w->response->major_ver,
			miv = w->response->minor_ver;

		*i++ = mav + '0';
		*i++ = '.';
		*i++ = miv + '0';
		*i++ = ' ';

		w->state = S_STATUS_CODE;
		/* Fall down */
		
	case S_STATUS_CODE:;
		int code = w->response->code;
		i += 3;
		char *j = i - 1;
		while(code) {
			*j-- = code - 10*(code/10) + '0';
			code /= 10;
		}

		*i++ = ' ';
		
		w->state = S_STATUS_STR;
		/* Fall down */
		
	case S_STATUS_STR:;
		code = w->response->code;
		const char *status_str = izm_get_response_status_str(code);
		strcpy(i, status_str);
		i += strlen(status_str);
		strcpy(i, "\r\n");
		i += 2;

		w->remaining = w->header->field.len;		
		w->state = S_HEADER_FIELD;
		/* Fall down */

	_HEADER_FIELD:
	case S_HEADER_FIELD:;
		izm_string *field = &(w->header->field);
		size_t bytes_writed = end - i;
		bytes_writed = bytes_writed > w->remaining ?
			w->remaining : bytes_writed;
		char *begin = field->data +
			(field->len - w->remaining);
		strncpy(i, begin, bytes_writed);
		i += bytes_writed;
		w->remaining -= bytes_writed;
		
		if(w->remaining)
			goto _EXIT_CONTINUE;

		if(end - i <= 2)
			goto _EXIT_CONTINUE;

		strcpy(i, ": ");
		i += 2;

		w->remaining = w->header->value.len;
		w->state = S_HEADER_VALUE;

	case S_HEADER_VALUE:;
		izm_string *value = &(w->header->value);
		bytes_writed = end - i;
		bytes_writed = bytes_writed > w->remaining ?
			w->remaining : bytes_writed;
		begin = value->data +
			(value->len - w->remaining);
		strncpy(i, begin, bytes_writed);
		i += bytes_writed;
		w->remaining -= bytes_writed;
		
		if(w->remaining)
			goto _EXIT_CONTINUE;

		if(end - i <= 4)
			goto _EXIT_CONTINUE;

		strcpy(i, "\r\n");
		i += 2;

		w->header = w->header->next;
		if (w->header == w->response->headers) {
			strcpy(i, "\r\n");
			i += 2;
			goto _EXIT_OK;
		} else {
			w->remaining = w->header->field.len;
			w->state = S_HEADER_FIELD;
			goto _HEADER_FIELD;
		}
	case S_FINISH:
		return IZM_OK;
	}

_EXIT_OK:
	*writed = (i - buffer);
	w->state = S_FINISH;
	return IZM_OK;

_EXIT_CONTINUE:
	*writed = (i - buffer);
	return IZM_CONTINUE;
}
