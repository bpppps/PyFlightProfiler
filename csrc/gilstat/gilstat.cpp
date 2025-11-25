#include "Python.h"
#include "symbol.h"

static int (*init_func)(PyObject *, unsigned long, unsigned long, int, int, int,
                        int) = NULL;
static int (*deinit_func)() = NULL;

static PyObject *init_gil_interceptor(PyObject *self, PyObject *args) {
  if (init_func == NULL) {
    return Py_BuildValue("i", -1);
  }
  PyObject *queue_obj;
  unsigned long take_addr, drop_addr;
  int take_threshold, hold_threshold, stat_interval, max_stat_threads;
  if (!PyArg_ParseTuple(args, "OLLiiii", &queue_obj, &take_addr, &drop_addr,
                        &take_threshold, &hold_threshold, &stat_interval,
                        &max_stat_threads)) {
    return Py_BuildValue("i", -1);
  }
  int ret = init_func(queue_obj, take_addr, drop_addr, take_threshold,
                      hold_threshold, stat_interval, max_stat_threads);
  return Py_BuildValue("i", ret);
}

static PyObject *deinit_gil_interceptor(PyObject *self, PyObject *args) {
  if (deinit_func == NULL) {
    return Py_BuildValue("i", -1);
  }
  int ret = deinit_func();
  return Py_BuildValue("i", ret);
}

static PyMethodDef gilstat_module_methods[] = {
    {"init_gil_interceptor", (PyCFunction)init_gil_interceptor, METH_VARARGS,
     "init gil interceptor"},
    {"deinit_gil_interceptor", (PyCFunction)deinit_gil_interceptor,
     METH_VARARGS, "deinit gil interceptor"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef gilstat_module = {
    PyModuleDef_HEAD_INIT,
    // name of module
    "gilstat_C",
    // module documentation
    NULL,
    // size of per-interpreter state of the module, or -1 if the module keeps
    // state in global variables
    -1, gilstat_module_methods};

// will be called when python module first loaded
PyMODINIT_FUNC PyInit_gilstat_C(void) {
  init_func = (int (*)(PyObject *, unsigned long, unsigned long, int, int, int,
                       int))get_symbol_addr("init_py_gil_interceptor");
  deinit_func = (int (*)())get_symbol_addr("deinit_py_gil_interceptor");

  return PyModule_Create(&gilstat_module);
}
