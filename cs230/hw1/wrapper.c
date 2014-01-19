#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include "utils.h"
#include "fw.h"

#ifdef OLD_NUMPY
#define NPY_ARRAY_OUT_ARRAY NPY_OUT_ARRAY
#define NPY_ARRAY_IN_ARRAY NPY_IN_ARRAY
#define NPY_ARRAY_ENSURECOPY NPY_ENSURECOPY
#endif

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

    double time = fw_serial(adj, n);

    return Py_BuildValue("Nd", matrix_pyarr, time);
}

static PyObject *_fw_parallel(PyObject *self, PyObject *args) {
    int n, t;
    PyObject *matrix_obj, *matrix_pyarr;

    PyArg_ParseTuple(args, "Oii", &matrix_obj, &n, &t);
    matrix_pyarr = PyArray_FROM_OTF(matrix_obj, NPY_INT, NPY_ARRAY_IN_ARRAY | NPY_ARRAY_ENSURECOPY);
    int *adj = PyArray_DATA((PyArrayObject *) matrix_pyarr);

#ifdef TIME_WAIT
    struct final_wait_data *times = fw_parallel(adj, n, t);
    PyObject *retval = Py_BuildValue("Nddd", matrix_pyarr, times->wo_ratio, times->o_mean, times->o_std);
    free (times);
    return retval;
#else
    double time = fw_parallel(adj, n, t);
    return Py_BuildValue("Nd", matrix_pyarr, time);
#endif
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
