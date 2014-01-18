#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include "utils.h"
#include "fw.h"

static PyObject *_from_csv(PyObject *self, PyObject *args) {
    int n;
    char *filename;
    PyObject *_matrix;

    PyArg_ParseTuple(args, "i", &n);
    PyArg_ParseTuple(args, "s", &filename);
    PyArg_ParseTuple(args, "o", &_matrix);

    _matrix = PyArray_FROM_OT(_matrix, NPY_DOUBLE);
    int *matrix = PyArray_DATA ((PyArrayObject *)_matrix);
    from_csv (matrix, filename, n);

    return Py_BuildValue("s", NULL);
}

static PyMethodDef module_methods[] = {
    {"from_csv", _from_csv, METH_VARARGS, "poop"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initwrapper(void)
{
    PyObject *m = Py_InitModule3("wrapper", module_methods, "poop");
    if (m == NULL)
        return;

    import_array();
}
