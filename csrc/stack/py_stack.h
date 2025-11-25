#include "Python.h"
#ifndef __PY_STACK_H__
#define __PY_STACK_H__

#ifdef __cplusplus
extern "C" {
#endif

void dump_threads(int fd, unsigned long _Py_DumpTracebackThreads_addr);

#ifdef __cplusplus
}
#endif

#endif
