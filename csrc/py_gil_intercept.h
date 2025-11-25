#include "frida-gum.h"
#include "py_gil_stat.h"

#ifndef __PY_GIL_INTERCEPT_H__
#define __PY_GIL_INTERCEPT_H__

#ifdef __cplusplus
extern "C" {
#endif

int init_py_gil_interceptor(unsigned long take_gil_symbol_addr,
                            unsigned long drop_gil_symbol_addr,
                            int take_cost_warning_threshold,
                            int hold_cost_warning_threshold, int stat_interval);

int deinit_py_gil_interceptor();

#ifdef __cplusplus
}
#endif

#endif
