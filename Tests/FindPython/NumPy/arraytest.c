#include "Python.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <math.h>

#include "numpy/arrayobject.h"

static PyObject* vecsq(PyObject* self, PyObject* args);

static PyMethodDef arraytestMethods[] = { { "vecsq", vecsq, METH_VARARGS },
                                          { NULL, NULL } };

static PyObject* vecsq(PyObject* self, PyObject* args)
{
  PyArrayObject *vecin, *vecout;
  npy_intp dims[2];
  double *cin, *cout;
  int i, j, n, m;

  if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &vecin))
    return NULL;

  n = dims[0] = PyArray_NDIM(vecin);
  vecout = (PyArrayObject*)PyArray_SimpleNew(1, dims, NPY_DOUBLE);

  cin = (double*)PyArray_DATA(vecin);
  cout = (double*)PyArray_DATA(vecout);

  for (i = 0; i < n; i++) {
    cout[i] = cin[i] * cin[i];
  }
  return PyArray_Return(vecout);
}

#if defined(PYTHON2)
PyMODINIT_FUNC init_C_arraytest(void)
{
  (void)Py_InitModule("arraytest2", arraytestMethods);
  import_array();
}
#endif

#if defined(PYTHON3)
static struct PyModuleDef arraytestmodule = {
  PyModuleDef_HEAD_INIT, "arraytest3", /* name of module */
  NULL,                                /* module documentation, may be NULL */
  -1, /* size of per-interpreter state of the module,
         or -1 if the module keeps state in global variables. */
  arraytestMethods
};

PyMODINIT_FUNC PyInit_C_arraytest(void)
{
  PyObject* po = PyModule_Create(&arraytestmodule);
  import_array();
  return po;
}
#endif
