#include <time.h>

#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

int strftime_with_millisec(struct timespec *ts, char *time_buffer, int len);

#ifdef __cplusplus
}
#endif

#endif
