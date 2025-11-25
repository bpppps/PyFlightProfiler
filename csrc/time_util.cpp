#include "time_util.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int strftime_with_millisec(struct timespec *ts, char *time_buffer, int len) {
  int millisec;
  struct tm *tm_info;
  struct timeval tv;
  tv.tv_sec = ts->tv_sec;
  tv.tv_usec = lrint(ts->tv_nsec / 1000.0);
  millisec = lrint(ts->tv_nsec / 1000000.0);
  if (millisec >= 1000) {
    millisec -= 1000;
    tv.tv_sec++;
  }
  tm_info = localtime(&tv.tv_sec);
  if (len >= 24) {
    int str_len = (int)strftime(time_buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(time_buffer + str_len, sizeof(time_buffer) - str_len, ".%03d",
             millisec);
    return 23;
  } else {
    char buffer[24];
    int str_len = (int)strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buffer + str_len, sizeof(buffer) - str_len, ".%03d", millisec);
    memcpy(time_buffer, buffer, len);
    time_buffer[len - 1] = (char)0;
    return len - 1;
  }
}

#ifdef __cplusplus
}
#endif
