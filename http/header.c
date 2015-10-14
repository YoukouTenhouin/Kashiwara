#include <http/header.h>

void izm_http_header_init(struct izm_http_header *header);
void izm_http_header_insert(struct izm_http_header*,
		       struct izm_http_header*);
struct izm_http_header* izm_http_header_detach_next(struct izm_http_header*);
