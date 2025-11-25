#include "python_util.h"

#ifdef __cplusplus
extern "C" {
#endif

PyObject *invoke_module_function(const char *module, const char *function,
                                 PyObject *args) {
  PyObject *module_name = PyUnicode_DecodeFSDefault(module);
  // call threading.enumerate() to get all thread name
  PyObject *module_obj = PyImport_Import(module_name);
  Py_DECREF(module_name);

  if (module_obj != NULL) {
    PyObject *function_obj = PyObject_GetAttrString(module_obj, function);
    Py_DECREF(module_obj);

    if (function_obj != NULL) {
      PyObject *result = PyObject_CallObject(function_obj, args);
      Py_DECREF(function_obj);
      return result;
    }
  }
  return NULL;
}

#ifdef __cplusplus
}
#endif
