#include "py_stack.h"
#include "symbol_util.h"

#ifdef __cplusplus
extern "C" {
#endif

void dump_threads(int fd, unsigned long _Py_DumpTracebackThreads_addr) {
  const char *(*f)(int, PyInterpreterState *, PyThreadState *) =
      (const char *(*)(int, PyInterpreterState *, PyThreadState *))
          get_symbol_address_by_nm_offset(_Py_DumpTracebackThreads_addr);
  PyThreadState *tstate = PyGILState_GetThisThreadState();
  f(fd, NULL, tstate);
}

#ifdef __cplusplus
}
#endif
