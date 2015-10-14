#ifndef __IZM_TIME_H__
#define __IZM_TIME_H__

#include <sys/time.h>

typedef unsigned long long izm_time_t;

/* In milliseconds */
static inline izm_time_t
izm_time_now()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (izm_time_t)(tv.tv_sec) * 1000 +
		(izm_time_t)(tv.tv_usec) / 1000;	
}

#endif	/* __IZM_TIME_H__ */
