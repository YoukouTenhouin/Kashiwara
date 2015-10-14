#ifndef __IZUMO_TCP_CONNECTION_H__
#define __IZUMO_TCP_CONNECTION_H__

#include <sys/types.h>
#include <sys/socket.h>

#include <event/evloop.h>

struct izm_tcp_conn {
	int fd;
	int readable;
	int writable;
	int acceptable;
	int connected;
	int listened;
	int bound;
};



#endif	/* __IZUMO_TCP_CONNECTION_H__ */
