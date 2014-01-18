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

static PyObject *_fw_serial(PyObject *self, PyObject *args) {
    int n;
    PyObject *matrix_obj, *matrix_pyarr;

    PyArg_ParseTuple(args, "Oi", &matrix_obj, &n);
    matrix_pyarr = PyArray_FROM_OTF(matrix_obj, NPY_INT, NPY_ARRAY_IN_ARRAY | NPY_ARRAY_ENSURECOPY);
    int *adj = PyArray_DATA((PyArrayObject *) matrix_pyarr);

    fw_serial(adj, n);

    return matrix_pyarr;
}

static PyObject *_fw_parallel(PyObject *self, PyObject *args) {
    int n, t;
    PyObject *matrix_obj, *matrix_pyarr;

    PyArg_ParseTuple(args, "Oii", &matrix_obj, &n, &t);
    matrix_pyarr = PyArray_FROM_OTF(matrix_obj, NPY_INT, NPY_ARRAY_IN_ARRAY | NPY_ARRAY_ENSURECOPY);
    int *adj = PyArray_DATA((PyArrayObject *) matrix_pyarr);

    fw_parallel(adj, n, t);

    return matrix_pyarr;
}

static PyMethodDef module_methods[] = {
    {"from_csv", _from_csv, METH_VARARGS, "poop"},
    {"to_csv", _to_csv, METH_VARARGS, "poop"},
    {"fw_serial", _fw_serial, METH_VARARGS, "poop"},
    {"fw_parallel", _fw_parallel, METH_VARARGS, "poop"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initwrapper(void)
{
    PyObject *m = Py_InitModule3("wrapper", module_methods, "poop");
    if (m == NULL)
        return;

    import_array();
}
