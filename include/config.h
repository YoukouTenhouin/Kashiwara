#ifndef __IZM_CONFIG_H__
#define __IZM_CONFIG_H__

#define IZM_USE_REUSEPORT 1
#define IZM_USE_SENDFILE  1
#define IZM_USE_ACCEPT4   1

#define IZM_LISTEN_BACKLOG 128

#define IZM_DEBUG 1

#define IZM_MAX_CONNS 1024
#define IZM_MAX_EPOLL_EVS 1024

#define IZM_CLIENT_TIMEOUT 5000 /* in milliseconds */

#define IZM_HEAP_INIT_SIZE 1024
#define IZM_HEAP_MAX_SIZE 1024

#define IZM_MEMPOOL_CHUNK_SIZE 8192
#define IZM_MEMPOOL_LARGE_SIZE 4096
#define IZM_MEMPOOL_NFAILURE 4

/* Define one of these macros:
 * IZM_USE_EPOLL 
 * IZM_USE_SELECT */

#define IZM_USE_EPOLL 1

#endif	/* __IZM_CONFIG_H__ */
