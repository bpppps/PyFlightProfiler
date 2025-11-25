#include "Python.h"
#include <list>
#include <map>
#include <pthread.h>
#include <unordered_map>
#ifndef __PY_GIL_STAT_H__
#define __PY_GIL_STAT_H__

typedef struct _gil_statistics {
  struct timespec last_gil_take_start_time;
  struct timespec last_gil_take_success_time;
  struct timespec last_gil_drop_start_time;
  // nano second
  unsigned long last_gil_take_cost;
  // nano second
  unsigned long gil_take_total_cost;
  unsigned long gil_take_count;

  // nano second
  unsigned long gil_drop_total_cost;
  unsigned long gil_drop_count;

  // nano second
  unsigned long gil_hold_total;
} gil_statistics;

typedef struct _gil_warning {
  // 0: take 1:hold
  int8_t type;
  // nano second
  unsigned long cost;
  // nano second
  unsigned long start_ns;
  // nano second
  unsigned long end_ns;
  pthread_t thread_id;
  char time[24];
  char thread_name[16];
} gil_warning;

typedef struct _gil_monitor_config {
  // millisecond
  unsigned int gil_take_warning_threshold;
  // millisecond
  unsigned int gil_hold_warning_threshold;
  // second
  unsigned int stat_interval;
  unsigned int gil_stat_max_threads;
} gil_monitor_config;

class PyGilStat {
public:
  PyGilStat();

public:
  int start(gil_monitor_config *config);
  int stop();
  void set_out_queue(PyObject *out_queue);
  void on_take_gil_enter(pthread_t p);
  void on_take_gil_leave(pthread_t p);
  void on_drop_gil_enter(pthread_t p);
  void on_drop_gil_leave(pthread_t p);

private:
  void start_python_stat_thread();
  void send(const char *msg, PyThreadState *tstate);
  void send_end();

private:
  static void boot_entry(void *boot_raw);
  static std::map<unsigned long, char *> *
  dump_thread_name(void *boot_raw, PyThreadState *tstate);
  // dump gil statistic group by thread
  static void dump_gil_stat(void *boot_raw, PyThreadState *tstate,
                            std::map<unsigned long, char *> *thread_name_map);
  // dump gil take or hold timeout records
  static void
  dump_gil_warning(void *boot_raw, PyThreadState *tstate,
                   std::map<unsigned long, char *> *thread_name_map);

private:
  std::unordered_map<pthread_t, gil_statistics *> *stat_map;
  std::list<gil_warning *> *warning_list;
  gil_monitor_config *config;
  unsigned long stat_thread_id;
  bool running_flag;
  PyObject *py_out_queue;
  pthread_mutex_t queue_mutex;
  pthread_mutex_t stat_map_mutex;
  pthread_mutex_t warning_list_mutex;
};

#endif
