#include "symbol.h"
#include "Python.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#else
#include <link.h>
#endif

static const char *shared_lib_name = NULL;

#ifdef __APPLE__
static void resolve_symbol() {
  uint32_t i, count = _dyld_image_count();
  for (i = 0; i < count; i++) {
    const char *name = _dyld_get_image_name(i);
    if (strstr(name, "flight_profiler_agent.")) {
      shared_lib_name = name;
      break;
    }
  }
}
#else
static int dl_callback(struct dl_phdr_info *info, size_t size, void *data) {
  if (strstr(info->dlpi_name, "flight_profiler_agent.")) {
    shared_lib_name = info->dlpi_name;
  }
  return 0;
}
#endif

// so init
__attribute__((constructor)) static void init() {
// lookup for code-inject.so/code-inject.dylib
#ifdef __APPLE__
  resolve_symbol();
#else
  dl_iterate_phdr(dl_callback, NULL);
#endif
}

#ifdef __cplusplus
extern "C" {
#endif

void *get_symbol_addr(const char *func_name) {
  if (shared_lib_name != NULL) {
    void *handle = dlopen(shared_lib_name, RTLD_LAZY | RTLD_GLOBAL);
    if (handle != NULL) {
      void *addr = (void *)dlsym(handle, func_name);
      if (addr == NULL) {
        const char *err = dlerror();
        if (err != NULL) {
          fprintf(stderr, "dlsym find %s function failed, error message: %s\n",
                  func_name, err);
        }
      } else {
        return addr;
      }
    } else {
      const char *err = dlerror();
      if (err != NULL) {
        fprintf(stderr,
                "dlopen failed when init cpython module, error message: %s\n",
                err);
      }
    }
  } else {
    fprintf(stderr,
            "cpython module can not found flight_profiler_agent.so loaded\n");
  }
  return NULL;
}

#ifdef __cplusplus
}
#endif
