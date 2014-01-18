#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include "utils.h"
#include "fw.h"

static PyObject *_from_csv(PyObject *self, PyObject *args) {
    int n;
    char *filename;
    PyObject *matrix_obj, *_matrix;

    PyArg_ParseTuple(args, "sOi", &filename, &matrix_obj, &n);

    _matrix = PyArray_FROM_OTF(matrix_obj, NPY_INT, NPY_ARRAY_OUT_ARRAY);
    int *matrix = PyArray_DATA ((PyArrayObject *)_matrix);
    from_csv (matrix, filename, n);
    Py_DECREF(_matrix);

    Py_RETURN_NONE;
}

static PyObject *_to_csv(PyObject *self, PyObject *args) {
    int n;
    char *filename;
    PyObject *matrix_obj, *_matrix;

    PyArg_ParseTuple(args, "sOi", &filename, &matrix_obj, &n);

    _matrix = PyArray_FROM_OTF(matrix_obj, NPY_INT, NPY_ARRAY_IN_ARRAY);
    int *matrix = PyArray_DATA ((PyArrayObject *)_matrix);
    to_csv (matrix, filename, n);
    Py_DECREF(_matrix);

    Py_RETURN_NONE;
}

static PyMethodDef module_methods[] = {
    {"from_csv", _from_csv, METH_VARARGS, "poop"},
    {"to_csv", _to_csv, METH_VARARGS, "poop"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initwrapper(void)
{
    PyObject *m = Py_InitModule3("wrapper", module_methods, "poop");
    if (m == NULL)
        return;

    import_array();
}