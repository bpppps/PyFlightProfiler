#include "frida_profiler.h"
#include <assert.h>
static pthread_mutex_t mutex;
static int inited = 0;

// so init
__attribute__((constructor)) static void frida_profiler_init() {
  pthread_mutex_init(&mutex, NULL);
}

#ifdef __cplusplus
extern "C" {
#endif

int init_frida_gum() {
  pthread_mutex_lock(&mutex);
  if (inited != 0) {
    pthread_mutex_unlock(&mutex);
    fprintf(stderr, "[*] frida gum already inited\n");
    return 0;
  }
  gum_init_embedded();
  inited = 1;
  pthread_mutex_unlock(&mutex);
  g_print("[*] init frida gum successfully\n");
  return 0;
}

int deinit_frida_gum() {
  pthread_mutex_lock(&mutex);
  if (inited != 1) {
    // pthread_mutex_unlock(&mutex);
    fprintf(stderr, "[*] frida gum not inited\n");
    return 0;
  }
  gum_deinit_embedded();
  inited = 0;
  pthread_mutex_unlock(&mutex);
  g_print("[*] deinit frida gum successfully\n");
  return 0;
}

#ifdef __cplusplus
}
#endif
