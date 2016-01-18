#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h> 

#include <mem/mempool.h>
#include <http/request.h>
#include <http/response.h>
#include <http/header.h>
#include <misc/string.h>
#include <misc/constants.h>

#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../servcraft/p7/libp7.h"
#include "../servcraft/p7/p7_root_alloc.h"
#include "../servcraft/include/model_alloc.h"

#define PATH = "./";

char preset_headers[][2][64] = {
	{ "Server", "Kashiwara" },
	{ "Content-Type", "text/html" },
	{ "Connection", "Close" }
};

char method_names[][128] = {
	"UNDEF", "GET", "POST", "HEAD", "PUT", "DELETE",
	"TRACE", "CONNECT", "OPTIONS"
};

char r400_body[] =
	"<html><head><title>Kashiwara Error</title><head>"
	"<body style=\"text-align:center;\"><h1>400 Bad Request</h1>"
	"<hr><span>Kashiwara</span>"
	"</body></html>";

struct client {
	int sock;
	struct sockaddr_in addr;
	struct izm_mempool *pool;
	char in_buff[4096];
	char head_buff[4096];
	char out_buff[65536];
	size_t body_size;
	struct izm_http_req_parser parser;
	struct izm_http_response_writer writer;
};

struct izm_http_header*
create_headers(size_t content_len, struct izm_mempool *pool)
{
	struct izm_http_header *ret;

	static char _contlen_field[] = "Content-Length";	
	// izm_mempool_alloc(pool, 1024);
	
	char *contlen_field =
		izm_mempool_alloc(pool, sizeof(_contlen_field)),
		*contlen_val = izm_mempool_alloc(pool, 64);

	strcpy(contlen_field, _contlen_field);

	sprintf(contlen_val, "%zu", content_len);
	
	struct izm_http_header *contlen_header =
		izm_mempool_alloc(pool, sizeof(struct izm_http_header));

	ret = contlen_header;
	
	izm_http_header_init(contlen_header);

	izm_string_from_cstr(&contlen_header->field, contlen_field);
	izm_string_from_cstr(&contlen_header->value, contlen_val);

	for(int i = 0; i < sizeof(preset_headers) / 128; ++i) {
		struct izm_http_header *h =
			izm_mempool_alloc(pool, sizeof(struct izm_http_header));
		izm_http_header_init(h);
		izm_string_from_cstr(&h->field, preset_headers[i][0]);
		izm_string_from_cstr(&h->value, preset_headers[i][1]);
		izm_http_header_insert(h, ret);
	}

	return ret->prev;
}

size_t
create_body(char *out, struct izm_http_request *req) {
	const char *html_head = "<html>"
		"<head><title>Kashiwara Test Page</title></head>"
		"<body style=\"margin:0;font-family:inconsolata;monospace;atril;\">"
		"<h1 style=\"color:white; background-color:black; padding:10px;\">"
		"Kashiwara Test Page"
		"</h1>"
		"<div style=\"max-width:960px;margin:auto;\">";
	
	const char *html_tail = "</div></body></html>";

	char *input = out;
	strcpy(out, html_head);
	input += strlen(html_head);
	
	char method_names[][128] = {
		"UNDEF", "GET", "POST", "HEAD", "PUT", "DELETE",
		"TRACE", "CONNECT", "OPTIONS"
	};
	
	sprintf(input, "<h1>Request Line</h1><span>Method:%s URL:%.*s Version: HTTP v%d.%d</span>",
		method_names[req->method],
		IZM_STR_P(req->url),
		req->major_ver, req->minor_ver);
	
	input += strlen(input);

	sprintf(input, "<h1>Request Headers</h1>");
	input += strlen(input);
	sprintf(input, "<ul style=\"list-style:none;\">");
	input += strlen(input);
		
	sprintf(input, "<li><strong>%.*s:</strong> %.*s</li>",
		IZM_STR_P(req->headers->field),
		IZM_STR_P(req->headers->value));
	input += strlen(input);
	for(struct izm_http_header *i = req->headers->next;
	    i != req->headers; i = i->next) {
		sprintf(input, "<li><strong>%.*s:</strong> %.*s</li>",
			IZM_STR_P(i->field),
			IZM_STR_P(i->value));

		input += strlen(input);
	}

	sprintf(input, "</ul>");
	input += strlen(input);
	strcpy(input, html_tail);
	input += strlen(html_tail);

	return input - out;
}

void
handle_client(void *_c)
{
	struct client *c = (struct client*)_c;
	int ret;
	int code;
	size_t body_size;
	char *body;
	
	struct izm_http_request *req = &c->parser.request;
	
	//izm_mempool_init(&c->pool);
	c->pool = izm_mempool_get(4096, 4); // XXX 20150718 Akvelog test
	
	izm_http_req_parser_init(&c->parser);
	c->parser.pool = c->pool;

	char *input = c->in_buff;
	size_t in_buff_size = 4096;

	do {
		int bread = p7_iowrap(read, P7_IOMODE_READ, c->sock, input, in_buff_size);
		if (bread == 0)
			goto _EXIT_ERROR;
		if (bread < 0) {
			perror("read");
			goto _EXIT_ERROR;
		}
		
		ret = izm_http_req_parser_run(
			&c->parser, input, bread);

		input += bread;
		in_buff_size -= bread;

		if(ret == IZM_CONTINUE && in_buff_size == 0)
			goto _EXIT_ERROR;
		else if (ret == IZM_ERROR)
			break;
		
	} while (ret == IZM_CONTINUE);

			
	struct izm_http_response *response =
		izm_mempool_alloc(c->pool, sizeof(struct izm_http_response));
	
	if (ret == IZM_OK) {
		response->code = 200;
		response->major_ver = req->major_ver;
		response->minor_ver = req->minor_ver;
		body_size = create_body(c->out_buff, req);
		body = c->out_buff;
	} else {
		response->code = 400;
		response->major_ver = 1;
		response->minor_ver = 1;
		body_size = sizeof(r400_body);
		body = r400_body;
	}
	

	response->headers = create_headers(body_size, c->pool);
		
	izm_http_response_writer_init(&c->writer);
	izm_http_response_writer_set_response(&c->writer, response);

	size_t head_len;

	do {
		ret = izm_http_response_write(&c->writer,
					      c->head_buff, 4096, &head_len);
		if (ret == IZM_ERROR)
			goto _EXIT_ERROR;
		int bwrote = p7_iowrap(write, P7_IOMODE_WRITE, c->sock, c->head_buff, (int)head_len);
		if(bwrote <= 0)
			goto _EXIT_ERROR;

	} while (ret == IZM_CONTINUE);
	
	if (p7_iowrap(write, P7_IOMODE_WRITE, c->sock, body, (int)body_size) <= 0)
		goto _EXIT_ERROR;

	izm_mempool_drain(c->pool);
	shutdown(c->sock, SHUT_RDWR);
	close(c->sock);
	free(c);
	/* struct izm_http_request *req = &c->parser.request; */
	/* printf("%s %.*s %d\n", method_names[req->method], */
	/*        IZM_STR_P(req->url), c->writer.response->code); */
	
	return;

_EXIT_ERROR:
	izm_mempool_drain(c->pool);
	close(c->sock);
	free(c);
	return;	
}

volatile uint64_t n_accept_count = 0;

void sigint_handler(int unused) {
    printf("Total: %lu clients handled\n", n_accept_count);
    exit(0);
}

int
main(int argc, char *argv[]) {
	int port = 21021;
	int workers = 5;
    signal(SIGINT, sigint_handler);

    /*
	if( argc >= 2 )
		sscanf(argv[1], "%d", &port);
	if( argc >= 3 )
		sscanf(argv[2], "%d", &workers);
        */
    int opt;
    while ((opt = getopt(argc, argv, "p:n:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'n':
                workers = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-n nworkers]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    __auto_type allocator = p7_root_alloc_get_allocator();
    allocator->allocator_.closure_ = malloc;
    allocator->deallocator_.closure_ = free;
    allocator->reallocator_.closure_ = realloc;

	void mempool_init_local(void *unused) {
        izm_mempool_init(32, 8192); // XXX 20150718 Akvelog test
    }
    void spinlock_init_local(void *unused) {
        p7_spinlock_tlinit((void *) 64); // XXX 20151017 Akvelog test
    }
    void kashiwara_init_local(void *unused) {
        mempool_init_local(NULL);
        spinlock_init_local(NULL);
    }
	//p7_init(workers, mempool_init_local, NULL);
    //p7_preinit_namespace_size(1024);
    //p7_init(workers, kashiwara_init_local, NULL);
    //p7_spinlock_preinit(64); // XXX 20151017 Akvelog test
    struct p7_init_config config;
    config.namespace_config.namespace_size = 1024;
    config.namespace_config.rwlock_granularity = 10;
    config.namespace_config.spintime = 400;
    config.pthread_config.nthreads = workers;
    config.pthread_config.at_startup = kashiwara_init_local;
    config.pthread_config.arg_startup = NULL;
    p7_init(config);

	struct sockaddr_in serv_addr;	
	
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		perror("socket");
		abort();
	}

	int optval = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval));
    fcntl(listen_sock, F_SETFL, fcntl(listen_sock, F_GETFL)|O_NONBLOCK);
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(listen_sock,
		(struct sockaddr*)&serv_addr,
		sizeof(serv_addr)) < 0) {
		perror("bind");
		abort();
	}
	
	listen(listen_sock, 32);

	int multi_accept_flag = 1;
	int coro_create_flag = 0;
	int cli_sock_queue[4096], *cli_sock_queue_curr = cli_sock_queue;
	struct sockaddr_in cli_addr_queue[4096], *cli_addr_queue_curr = cli_addr_queue;
	bzero(cli_addr_queue, sizeof(cli_addr_queue));
	while(1) {
		unsigned int caddrlen;
		//int cli_sock = p7_iowrap(accept, P7_IOMODE_READ, listen_sock, (struct sockaddr*)&cli_addr, &caddrlen);
        int cli_sock;
        struct sockaddr_in cli_addr;
        while ((cli_sock = accept(listen_sock, (struct sockaddr *) &cli_addr, &caddrlen)) != -1) {
            struct client *c = malloc(sizeof(struct client));
            c->sock = cli_sock;
            c->addr = cli_addr;
            p7_coro_create_async(handle_client, c, 4096);
            n_accept_count++;
        }
        //p7_coro_yield();
        p7_io_notify(listen_sock, P7_IOMODE_READ);
    }

    return 0;
}
