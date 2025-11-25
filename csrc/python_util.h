#include "Python.h"

#ifndef __PYTHON_UTIL_H__
#define __PYTHON_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

PyObject *invoke_module_function(const char *module, const char *function,
                                 PyObject *args);
#ifdef __cplusplus
}
#endif

#endif
