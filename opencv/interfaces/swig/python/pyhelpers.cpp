#include "pyhelpers.h"

int PySwigObject_Check(PyObject *op);


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

#define CV_MAT_PRINT_1CH( fd, mat, type, format )  \
{                                                  \
	int rows = mat->rows;                          \
	int cols = mat->cols;                          \
	type * ptr;                                    \
	for(int i=0; i<rows; i++){                     \
		ptr = (type *) (mat->data.ptr+mat->step*i);\
		fprintf(fd, "[");                          \
		for(int j=0; j<cols; j++){                 \
			fprintf(fd, " ");                      \
			fprintf(fd, format, ptr[j]);           \
		}                                          \
		fprintf(fd, " ]\n");                       \
	}                                              \
}                                               

void cvArrPrint(CvArr * arr){
    CV_FUNCNAME( "cvArrPrint" );
	    
	__BEGIN__;
	CvMat * mat;
	CvMat stub;

	mat = cvGetMat(arr, &stub);
	
	int cn = CV_MAT_CN(mat->type);
	int depth = CV_MAT_DEPTH(mat->type);
	if(cn!=1){
		 CV_ERROR( CV_StsNotImplemented, "print is only implemented for single channel arrays");
	}
	switch(depth){
		case CV_8U:
			CV_MAT_PRINT_1CH(stdout, mat, uchar, "%o");
			break;
		case CV_8S:
			CV_MAT_PRINT_1CH(stdout, mat, char, "%d");
			break;
		case CV_16U:
			CV_MAT_PRINT_1CH(stdout, mat, ushort, "%o");
			break;
		case CV_16S:
			CV_MAT_PRINT_1CH(stdout, mat, short, "%d");
			break;
		case CV_32S:
			CV_MAT_PRINT_1CH(stdout, mat, int, "%d");
			break;
		case CV_32F:
			CV_MAT_PRINT_1CH(stdout, mat, float, "%f");
			break;
case CV_64F:
			CV_MAT_PRINT_1CH(stdout, mat, double, "%f");
			break;
	}

	__END__;
}

CvRect PySlice_to_CvRect(CvArr * src, PyObject * idx_object){
	CvSize sz = cvGetSize(src);
	//printf("Size %dx%d\n", sz.height, sz.width);
	int lower[2], upper[2];
	int len, start, stop, step, slicelength;

	if(PyInt_Check(idx_object) || PyLong_Check(idx_object)){
		lower[0] = PyLong_AsLong( idx_object );
		upper[0] = lower[0] + 1;
		lower[1] = 0;
		upper[1] = sz.width;
	}

	// 1. Slice
	else if(PySlice_Check(idx_object)){
		len = sz.height;
		if(PySlice_GetIndicesEx( (PySliceObject*)idx_object, len, &start, &stop, &step, &slicelength )!=0){
			printf("Error in PySlice_GetIndicesEx: returning NULL");
			PyErr_SetString(PyExc_Exception, "Error");
			return cvRect(0,0,0,0);
		}
		//printf("PySlice\n");
		lower[0] = start; // use c convention of start index = 0
		upper[0] = stop;    // use c convention
		lower[1] = 0;
		upper[1] = sz.width;
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
				lower[i] = PyLong_AsLong(o);
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

	lower[0] = MAX(0, lower[0]);
	lower[1] = MAX(0, lower[1]);
	upper[0] = MIN(sz.height, upper[0]);
	upper[1] = MIN(sz.width, upper[1]);
	assert(lower[0]<upper[0]);
	assert(lower[1]<upper[1]);
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

CvArr * PySequence_to_CvArr( PyObject * obj ){
	int dims[CV_MAX_DIM] = {1,1,1};
	int ndim=0;
	int type;
	PyObject * item;
	
	// figure out dimensions
	for(item = obj; 
		(PyTuple_Check(item) || PyList_Check(item));
		item = PySequence_GetItem(item, 0))
	{
		dims[ndim] = PySequence_Size( item ); 
		ndim++;
	}

	if(ndim==0) return NULL;
	
	// CvMat
	if(ndim<=2 || (ndim==3 && dims[2]<4)){
		type=CV_MAKETYPE(CV_32F, dims[2]);
		CvMat *m = cvCreateMat(dims[0], dims[1], type);
		for(int i=0; i<dims[0]; i++){
			PyObject * rowobj = PySequence_GetItem(obj, i);
			if( dims[1] > 1 ){
				// double check size
				assert((PyTuple_Check(rowobj) || PyList_Check(rowobj)) && 
						PySequence_Size(rowobj) == dims[1]);

				for(int j=0; j<dims[1]; j++){
					PyObject * colobj = PySequence_GetItem(rowobj, j);
					CvScalar val;
					if(dims[2]>1){
						// double check size
						assert((PyTuple_Check(colobj) || PyList_Check(colobj)) && 
								PySequence_Size(colobj) == dims[1]);
						for(int k=0; k<dims[2]; k++){
							PyObject * chobj = PySequence_GetItem( colobj, k );
							val.val[0] = PyFloat_AsDouble( chobj );
						}
					}
					else{
						val.val[0] = PyFloat_AsDouble( colobj );
					}
					cvSet2D( m, i, j, val );
				}
			}
			else{
				cvSet1D(m, i, cvScalar( PyFloat_AsDouble(rowobj) ));
			}
		}
		return (CvArr *) m;
	}

	// CvMatND
	return NULL;
	
}
