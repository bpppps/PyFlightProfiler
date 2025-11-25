#include "Python.h"
#include "symbol.h"

void (*dump_threads_function)(int, unsigned long) = NULL;

static PyObject *dump_all_threads_stack(PyObject *self, PyObject *args) {
  int fd;
  unsigned long _Py_DumpTracebackThreads_addr;
  if (!PyArg_ParseTuple(args, "iL", &fd, &_Py_DumpTracebackThreads_addr)) {
    return Py_BuildValue("i", -1);
  }
  dump_threads_function(fd, _Py_DumpTracebackThreads_addr);
  return Py_BuildValue("i", 0);
}

static PyMethodDef stack_module_methods[] = {
    {"dump_all_threads_stack", (PyCFunction)dump_all_threads_stack,
     METH_VARARGS, "dump all thread stack"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef stack_module = {
    PyModuleDef_HEAD_INIT,
    // name of module
    "stack_C",
    // module documentation
    NULL,
    // size of per-interpreter state of the module, or -1 if the module keeps
    // state in global variables
    -1, stack_module_methods};

// will be called when python module first loaded
PyMODINIT_FUNC PyInit_stack_C(void) {
  dump_threads_function =
      (void (*)(int, unsigned long))get_symbol_addr("dump_threads");
  return PyModule_Create(&stack_module);
}
