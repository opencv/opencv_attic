#ifndef PYHELPERS_H
#define PYHELPERS_H

#include <Python.h>
#include <cxcore.h>
#include <cv.h>

/** convert python index object (tuple, integer, or slice) to CvRect for subsequent cvGetSubMat call */
CvRect PySlice_to_CvRect(CvArr * src, PyObject * idx_object);

/** prints array to stdout 
 *  TODO: python __str__ and __repr__ return strings, so this should write to a string 
 */
void cvArrPrint( CvArr * mat );

/** Convert an integer array to python tuple */
PyObject * PyTuple_FromIntArray(int * arr, int len);
	
/** If result is not NULL or PyNone, release object and replace it with obj */
PyObject * SWIG_SetResult(PyObject * result, PyObject * obj);
	
/** helper function to append one or more objects to the swig $result array */
PyObject * SWIG_AppendResult(PyObject * result, PyObject ** to_add, int num);

/** helper function to convert python scalar or sequence to int, float or double arrays */
double PyObject_AsDouble(PyObject * obj);
long PyObject_AsLong(PyObject * obj);

int PyObject_AsDoubleArray(PyObject * obj, double * array, int len);
int PyObject_AsLongArray(  PyObject * obj, int * array, int len);
int PyObject_AsFloatArray(PyObject * obj, float * array, int len);

#endif //PYHELPERS_H
