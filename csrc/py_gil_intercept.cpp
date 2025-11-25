#include "Python.h"
#include "frida_profiler.h"
#include "py_gil_stat.h"
#include "symbol_util.h"

struct _PythonGilListener {
  GObject parent;

  guint num_calls;
};

enum _PythonGilHookId { PYTHON_GIL_HOOK_TAKE_GIL, PYTHON_GIL_HOOK_DROP_GIL };

typedef struct _PythonGilListener PythonGilListener;
typedef enum _PythonGilHookId PythonGilHookId;

static void python_gil_listener_iface_init(gpointer g_iface,
                                           gpointer iface_data);

#define PYTHON_GIL_TYPE_LISTENER (python_gil_listener_get_type())
G_DECLARE_FINAL_TYPE(PythonGilListener, python_gil_listener, PYTHON_GIL,
                     LISTENER, GObject)
G_DEFINE_TYPE_EXTENDED(PythonGilListener, python_gil_listener, G_TYPE_OBJECT, 0,
                       G_IMPLEMENT_INTERFACE(GUM_TYPE_INVOCATION_LISTENER,
                                             python_gil_listener_iface_init))

static GumInterceptor *interceptor = NULL;
static GumInvocationListener *listener = NULL;
static PyGilStat *gilStat = NULL;
static pthread_mutex_t mutex;
static gil_monitor_config config;
static int inited = 0;

// so init
__attribute__((constructor)) static void py_gil_intercept_init() {
  pthread_mutex_init(&mutex, NULL);
}

static void python_gil_listener_on_enter(GumInvocationListener *listener,
                                         GumInvocationContext *ic) {
  PythonGilListener *self = PYTHON_GIL_LISTENER(listener);
  gpointer p = GUM_IC_GET_FUNC_DATA(ic, gpointer);
  PythonGilHookId hook_id = (PythonGilHookId)(gsize)p;

  pthread_t thread_id = pthread_self();
  switch (hook_id) {
  case PYTHON_GIL_HOOK_TAKE_GIL:
    gilStat->on_take_gil_enter(thread_id);
    break;
  case PYTHON_GIL_HOOK_DROP_GIL:
    gilStat->on_drop_gil_enter(thread_id);
    break;
  }

  self->num_calls++;
}

static void python_gil_listener_on_leave(GumInvocationListener *listener,
                                         GumInvocationContext *ic) {
  PythonGilListener *self = PYTHON_GIL_LISTENER(listener);
  gpointer p = GUM_IC_GET_FUNC_DATA(ic, gpointer);
  PythonGilHookId hook_id = (PythonGilHookId)(gsize)p;

  pthread_t thread_id = pthread_self();
  switch (hook_id) {
  case PYTHON_GIL_HOOK_TAKE_GIL:
    gilStat->on_take_gil_leave(thread_id);
    break;
  case PYTHON_GIL_HOOK_DROP_GIL:
    gilStat->on_drop_gil_leave(thread_id);
    break;
  }
}

static void python_gil_listener_class_init(PythonGilListenerClass *klass) {
  (void)PYTHON_GIL_IS_LISTENER;
  // (void) glib_autoptr_cleanup_PythonGilListener;
}

static void python_gil_listener_iface_init(gpointer g_iface,
                                           gpointer iface_data) {
  GumInvocationListenerInterface *iface =
      (GumInvocationListenerInterface *)g_iface;

  iface->on_enter = python_gil_listener_on_enter;
  iface->on_leave = python_gil_listener_on_leave;
}

static void python_gil_listener_init(PythonGilListener *self) {}

static int init_python_gil_interceptor_inner(GumAddress take_gil_address,
                                             GumAddress drop_gil_address,
                                             gil_monitor_config *config) {
  if (inited != 0) {
    fprintf(stderr, "[*] interceptor for take_gil & drop_dril already added\n");
    return 0;
  }
  if (take_gil_address == 0) {
    fprintf(stderr, "[*] take_gil symbol not found\n");
    return -1;
  }
  if (drop_gil_address == 0) {
    fprintf(stderr, "[*] drop_gil symbol not found\n");
    return -1;
  }
  interceptor = gum_interceptor_obtain();
  listener =
      (GumInvocationListener *)g_object_new(PYTHON_GIL_TYPE_LISTENER, NULL);
  gilStat = new PyGilStat();
  gilStat->start(config);

  gum_interceptor_begin_transaction(interceptor);
  gum_interceptor_attach(interceptor, GSIZE_TO_POINTER(take_gil_address),
                         listener, GSIZE_TO_POINTER(PYTHON_GIL_HOOK_TAKE_GIL));
  gum_interceptor_attach(interceptor, GSIZE_TO_POINTER(drop_gil_address),
                         listener, GSIZE_TO_POINTER(PYTHON_GIL_HOOK_DROP_GIL));
  gum_interceptor_end_transaction(interceptor);

  inited = 1;
  g_print("[*] add interceptor to take_gil & drop_dril successfully\n");
  return 0;
}

static int deinit_python_gil_interceptor_inner() {
  if (inited != 1) {
    fprintf(stderr, "[*] interceptor for take_gil & drop_dril not added\n");
    return 0;
  }
  gum_interceptor_detach(interceptor, listener);

  g_object_unref(listener);
  g_object_unref(interceptor);

  if (gilStat != NULL) {
    gilStat->stop();
    delete gilStat;
    gilStat = NULL;
  }

  inited = 0;
  listener = NULL;
  interceptor = NULL;
  g_print("[*] remove interceptor to take_gil & drop_dril successfully\n");
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

int init_py_gil_interceptor(PyObject *queue_obj,
                            unsigned long take_gil_symbol_addr,
                            unsigned long drop_gil_symbol_addr,
                            int take_cost_warning_threshold,
                            int hold_cost_warning_threshold, int stat_interval,
                            int max_stat_threads) {

  if (take_cost_warning_threshold > 0) {
    config.gil_take_warning_threshold = take_cost_warning_threshold;
  } else {
    config.gil_take_warning_threshold = 10;
  }
  if (hold_cost_warning_threshold > 0) {
    config.gil_hold_warning_threshold = hold_cost_warning_threshold;
  } else {
    config.gil_hold_warning_threshold = 10;
  }
  if (stat_interval > 1) {
    config.stat_interval = stat_interval;
  } else {
    config.stat_interval = stat_interval <= 0 ? 5 : 1;
  }
  if (max_stat_threads > 0) {
    config.gil_stat_max_threads =
        max_stat_threads > 1000 ? 1000 : max_stat_threads;
  } else {
    config.gil_stat_max_threads = 500;
  }
  pthread_mutex_lock(&mutex);
  int ret = init_python_gil_interceptor_inner(
      (GumAddress)get_symbol_address_by_nm_offset(take_gil_symbol_addr),
      (GumAddress)get_symbol_address_by_nm_offset(drop_gil_symbol_addr),
      &config);
  if (ret != 0) {
    pthread_mutex_unlock(&mutex);
    return ret;
  }
  gilStat->set_out_queue(queue_obj);
  pthread_mutex_unlock(&mutex);
  return ret;
}

int deinit_py_gil_interceptor() {
  pthread_mutex_lock(&mutex);
  int ret = deinit_python_gil_interceptor_inner();
  pthread_mutex_unlock(&mutex);
  return ret;
}

#ifdef __cplusplus
}
#endif
