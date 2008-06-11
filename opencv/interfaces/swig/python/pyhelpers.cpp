#include "pyhelpers.h"
#include <iostream>
#include <sstream>

int PySwigObject_Check(PyObject *op);

/* Py_ssize_t for old Pythons */
#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

PyObject * PyTuple_FromIntArray(int * arr, int len){
	PyObject * obj = PyTuple_New(len);
	for(int i=0; i<len; i++){
		PyTuple_SetItem(obj, i, PyLong_FromLong( arr[i] ) );
	}
	return obj;
}

PyObject * SWIG_SetResult(PyObject * result, PyObject * obj){
	if(result){
		Py_DECREF(result);
	}
	result = PyTuple_New(1);
	PyTuple_SetItem(result, 0, obj);
	return result;
}

PyObject * SWIG_AppendResult(PyObject * result, PyObject ** to_add, int num){
	if ((!result) || (result == Py_None)) {
		/* no other results, so just add our values */

		/* if only one object, return that */
		if(num==1){
			return to_add[0];
		}
		
		/* create a new tuple to put in our new pointer python objects */
		result = PyTuple_New (num);

		/* put in our new pointer python objects */
		for(int i=0; i<num; i++){
			PyTuple_SetItem (result, i, to_add[i]);
		}	
	}
	else {
		/* we have other results, so add it to the end */

		if (!PyTuple_Check (result)) {
			/* previous result is not a tuple, so create one and put
			   previous result and current pointer in it */

			/* first, save previous result */
			PyObject *obj_save = result;

			/* then, create the tuple */
			result = PyTuple_New (1);

			/* finaly, put the saved value in the tuple */
			PyTuple_SetItem (result, 0, obj_save);
		}

		/* create a new tuple to put in our new pointer python object */
		PyObject *my_obj = PyTuple_New (num);

		/* put in our new pointer python object */
		for( int i=0; i<num ; i++ ){
			PyTuple_SetItem (my_obj, i, to_add[i]);
		}

		/* save the previous result */
		PyObject *obj_save = result;

		/* concat previous and our new result */
		result = PySequence_Concat (obj_save, my_obj);

		/* decrement the usage of no more used objects */
		Py_DECREF (obj_save);
		Py_DECREF (my_obj);
	}
	return result;
}

template <typename T>
void cv_arr_write(FILE * f, const char * fmt, T * data, size_t rows, size_t nch, size_t step){
    size_t i,j,k;
    char * cdata = (char *) data;
    char * chdelim1="", * chdelim2="";

    // only output channel parens if > 1
    if(nch>1){
        chdelim1="(";
        chdelim2=")";
    }

    fprintf(f,"[");
    for(i=0; i<rows; i++){
		fprintf(f, "[" );

        // first element
        // out<<chdelim1;
		fprintf(f, chdelim1);
        fprintf(f, fmt, ((T*)(cdata+i*step))[0]);
        for(k=1; k<nch; k++){
			fprintf(f, ", ");
			fprintf(f, fmt, ((T*)(cdata+i*step))[k]);
        }
		fprintf(f, chdelim2);

        // remaining elements
        for(j=nch*sizeof(T); j<step; j+=(nch*sizeof(T))){
			fprintf(f, ",%s", chdelim1);
        	fprintf(f, fmt, ((T*)(cdata+i*step+j))[0]);
            for(k=1; k<nch; k++){
				fprintf(f, ", ");
				fprintf(f, fmt, ((T*)(cdata+i*step+j))[k]);
            }
			fprintf(f, chdelim2);
        }
		fprintf(f, "]\n" );
    }
	fprintf(f, "]" );
}

void cvArrPrint(CvArr * arr){
    CV_FUNCNAME( "cvArrPrint" );
	    
	__BEGIN__;
	CvMat * mat;
	CvMat stub;

	mat = cvGetMat(arr, &stub);
	
	int cn = CV_MAT_CN(mat->type);
	int depth = CV_MAT_DEPTH(mat->type);
	int step = MAX(mat->step, cn*mat->cols*CV_ELEM_SIZE(depth));


	switch(depth){
		case CV_8U:
			cv_arr_write(stdout, "%u", (uchar *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_8S:
			cv_arr_write(stdout, "%d", (char *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_16U:
			cv_arr_write(stdout, "%u", (ushort *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_16S:
			cv_arr_write(stdout, "%d", (short *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_32S:
			cv_arr_write(stdout, "%d", (int *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_32F:
			cv_arr_write(stdout, "%f", (float *)mat->data.ptr, mat->rows, cn, step);
			break;
		case CV_64F:
			cv_arr_write(stdout, "%g", (double *)mat->data.ptr, mat->rows, cn, step);
			break;
		default:
			CV_ERROR( CV_StsError, "Unknown element type");
			break;
	}

	__END__;
}

// deal with negative array indices
int PyLong_AsIndex( PyObject * idx_object, int len ){
	int idx = PyLong_AsLong( idx_object );
	if(idx<0) return len+idx;
	return idx;
}

CvRect PySlice_to_CvRect(CvArr * src, PyObject * idx_object){
	CvSize sz = cvGetSize(src);
	//printf("Size %dx%d\n", sz.height, sz.width);
	int lower[2], upper[2];
	Py_ssize_t len, start, stop, step, slicelength;

	if(PyInt_Check(idx_object) || PyLong_Check(idx_object)){
		// if array is a row vector, assume index into columns
		if(sz.height>1){
			lower[0] = PyLong_AsIndex( idx_object, sz.height );
			upper[0] = lower[0] + 1;
			lower[1] = 0;
			upper[1] = sz.width;
		}
		else{
			lower[0] = 0;
			upper[0] = sz.height;
			lower[1] = PyLong_AsIndex( idx_object, sz.width );
			upper[1] = lower[1]+1;
		}
	}

	// 1. Slice
	else if(PySlice_Check(idx_object)){
		len = sz.height;
		if(PySlice_GetIndicesEx( (PySliceObject*)idx_object, len, &start, &stop, &step, &slicelength )!=0){
			printf("Error in PySlice_GetIndicesEx: returning NULL");
			PyErr_SetString(PyExc_Exception, "Error");
			return cvRect(0,0,0,0);
		}
		// if array is a row vector, assume index bounds are into columns
		if(sz.height>1){
			lower[0] = (int) start; // use c convention of start index = 0
			upper[0] = (int) stop;    // use c convention
			lower[1] = 0;
			upper[1] = sz.width;
		}
		else{
			lower[1] = (int) start; // use c convention of start index = 0
			upper[1] = (int) stop;    // use c convention
			lower[0] = 0;
			upper[0] = sz.height;
		}
	}

	// 2. Tuple
	else if(PyTuple_Check(idx_object)){
		//printf("PyTuple{\n");
		if(PyObject_Length(idx_object)!=2){
			//printf("Expected a sequence of length 2: returning NULL");
			PyErr_SetString(PyExc_ValueError, "Expected a sequence with 2 elements");
			return cvRect(0,0,0,0);
		}
		for(int i=0; i<2; i++){
			PyObject *o = PyTuple_GetItem(idx_object, i);

			// 2a. Slice -- same as above
			if(PySlice_Check(o)){
				//printf("PySlice\n");
				len = (i==0 ? sz.height : sz.width);
				if(PySlice_GetIndicesEx( (PySliceObject*)o, len, &start, &stop, &step, &slicelength )!=0){
					PyErr_SetString(PyExc_Exception, "Error");
					printf("Error in PySlice_GetIndicesEx: returning NULL");
					return cvRect(0,0,0,0);
				}
				//printf("PySlice_GetIndecesEx(%d, %d, %d, %d, %d)\n", len, start, stop, step, slicelength);
				lower[i] = start;
				upper[i] = stop;

			}

			// 2b. Integer
			else if(PyInt_Check(o) || PyLong_Check(o)){
				//printf("PyInt\n");
				lower[i] = PyLong_AsIndex(o, i==0 ? sz.height : sz.width);
				upper[i] = lower[i]+1;
			}

			else {
				PyErr_SetString(PyExc_TypeError, "Expected a sequence of slices or integers");
				printf("Expected a slice or int as sequence item: returning NULL");
				return cvRect(0,0,0,0);
			}
		}
	}

	else {
		PyErr_SetString( PyExc_TypeError, "Expected a slice or sequence");
		printf("Expected a slice or sequence: returning NULL");
		return cvRect(0,0,0,0);
	}

	//lower[0] = MAX(0, lower[0]);
	//lower[1] = MAX(0, lower[1]);
	//upper[0] = MIN(sz.height, upper[0]);
	//upper[1] = MIN(sz.width, upper[1]);
	//printf("Slice=%d %d %d %d\n", lower[0], upper[0], lower[1], upper[1]);
	return cvRect(lower[1],lower[0], upper[1]-lower[1], upper[0]-lower[0]);
}

double PyObject_AsDouble(PyObject * obj){
	if(PyNumber_Check(obj)){
		if(PyFloat_Check(obj)){
			return PyFloat_AsDouble(obj);
		}
		else if(PyInt_Check(obj) || PyLong_Check(obj)){
			return (double) PyLong_AsLong(obj);
		}
	}
	PyErr_SetString( PyExc_TypeError, "Could not convert python object to Double");
	return -1;
}

long PyObject_AsLong(PyObject * obj){
    if(PyNumber_Check(obj)){
        if(PyFloat_Check(obj)){
            return (long) PyFloat_AsDouble(obj);
        }
        else if(PyInt_Check(obj) || PyLong_Check(obj)){
            return PyLong_AsLong(obj);
        }
    }
	PyErr_SetString( PyExc_TypeError, "Could not convert python object to Long");
	return -1;
}

