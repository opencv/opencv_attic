/* These functions need the SWIG_* functions defined in the wrapper */
%{

// convert a python sequence/array/list object into a c-array
#define PyObject_AsArrayImpl(func, ctype, ptype)                              \
	int func(PyObject * obj, ctype * array, int len){                         \
	CvMat * mat=NULL;                                                         \
	IplImage * im=NULL;                                                       \
	if(PyNumber_Check(obj)){                                                  \
		memset( array, 0, sizeof(ctype)*len );                                \
		array[0] = PyObject_As##ptype( obj );                                 \
	}                                                                         \
	else if(PySequence_Check(obj)){                                           \
		int seqsize = PySequence_Size(obj);                                   \
		for(int i=0; i<len && i<seqsize; i++){                                \
			if(i<seqsize){                                                    \
	            array[i] =  PyObject_As##ptype( PySequence_GetItem(obj, i) ); \
			}                                                                 \
			else{                                                             \
				array[i] = 0;                                                 \
			}                                                                 \
		}                                                                     \
	}                                                                         \
	else if( SWIG_ConvertPtr(obj, (void **)&mat, SWIGTYPE_p_CvMat, 0)!=-1 ||  \
	         SWIG_ConvertPtr(obj, (void **)&im, SWIGTYPE_p__IplImage, 0)!=-1) \
	{                                                                         \
		CvMat stub;                                                           \
		if(im) mat = cvGetMat(im, &stub);                                     \
		if( mat->rows!=1 && mat->cols!=1 ){                                   \
			PyErr_SetString( PyExc_TypeError,                                 \
			     "PyObject_As*Array: CvArr must be row or column vector" );   \
			return -1;                                                        \
		}                                                                     \
		if( mat->rows==1 && mat->cols==1 ){                                   \
			CvScalar val;                                                     \
			if( len!=CV_MAT_CN(mat->type) ){                                  \
				PyErr_SetString( PyExc_TypeError,                             \
				"PyObject_As*Array: CvArr channels != length" );              \
				return -1;                                                    \
			}                                                                 \
			val = cvGet1D(mat, 0);                                            \
			for(int i=0; i<len; i++){                                         \
				array[i] = (ctype) val.val[i];                                \
			}                                                                 \
		}                                                                     \
		else{                                                                 \
			mat = cvReshape(mat, &stub, -1, mat->rows*mat->cols);             \
			if( mat->rows != len ){                                           \
				PyErr_SetString( PyExc_TypeError,                             \
				 "PyObject_As*Array: CvArr rows or cols must equal length" ); \
				 return -1;                                                   \
			}                                                                 \
			for(int i=0; i<len; i++){                                         \
				CvScalar val = cvGet1D(mat, i);                               \
				array[i] = (ctype) val.val[0];                                \
			}                                                                 \
		}                                                                     \
	}                                                                         \
	else{                                                                     \
		PyErr_SetString( PyExc_TypeError,                                     \
				"PyObject_As*Array: Expected a number, sequence or CvArr" );  \
		return -1;                                                            \
	}                                                                         \
	return 0;                                                                 \
}

PyObject_AsArrayImpl( PyObject_AsFloatArray, float, Double );
PyObject_AsArrayImpl( PyObject_AsDoubleArray, double, Double );
PyObject_AsArrayImpl( PyObject_AsLongArray, int, Long );

static CvPoint PyObject_to_CvPoint(PyObject * obj){
	CvPoint val;
	CvPoint *ptr;
	CvPoint2D32f * ptr2D32f;
	CvScalar * scalar;

	if( SWIG_ConvertPtr(obj, (void**)&ptr, SWIGTYPE_p_CvPoint, 0) != -1) {
		return *ptr;
	}
	if( SWIG_ConvertPtr(obj, (void**)&ptr2D32f, SWIGTYPE_p_CvPoint2D32f, 0) != -1) {
		return cvPointFrom32f( *ptr2D32f );
	}
	if( SWIG_ConvertPtr(obj, (void**)&scalar, SWIGTYPE_p_CvScalar, 0) != -1) {
		return cvPointFrom32f(cvPoint2D32f( scalar->val[0], scalar->val[1] ));
	}
	if(PyObject_AsLongArray(obj, (int *) &val, 2) != -1){
		return val;
	}

	PyErr_SetString( PyExc_TypeError, "could not convert to CvPoint");
	return cvPoint(0,0);
}

static CvPoint2D32f PyObject_to_CvPoint2D32f(PyObject * obj){
    CvPoint2D32f val;
    CvPoint2D32f *ptr2D32f;
	CvPoint *ptr;
	CvScalar * scalar;
    if( SWIG_ConvertPtr(obj, (void**)&ptr2D32f, SWIGTYPE_p_CvPoint2D32f, 0) != -1) {
		return *ptr2D32f;
	}
	if( SWIG_ConvertPtr(obj, (void**)&ptr, SWIGTYPE_p_CvPoint, 0) != -1) {
		return cvPointTo32f(*ptr);
	}
	if( SWIG_ConvertPtr(obj, (void**)&scalar, SWIGTYPE_p_CvScalar, 0) != -1) {
		return cvPoint2D32f( scalar->val[0], scalar->val[1] );
	}
	if(PyObject_AsFloatArray(obj, (float *) &val, 2) != -1){
		return val;
	}
	PyErr_SetString(PyExc_TypeError, "could not convert to CvPoint2D32f");
	return cvPoint2D32f(0,0);
}

static CvScalar PyObject_to_CvScalar(PyObject * obj){
	CvScalar val;
	CvScalar * ptr;
	if( SWIG_ConvertPtr(obj, (void **)&ptr, SWIGTYPE_p_CvScalar, 0 ) == -1)
	{
		if(PyObject_AsDoubleArray(obj, val.val, 4)==-1){
			PyErr_SetString(PyExc_TypeError, "could not convert to CvScalar");
			return cvScalar(0);
		}
		return val;
	}
	return *ptr; 
}

/* if python sequence type, convert to CvMat or CvMatND */
static CvArr * PyObject_to_CvArr(PyObject * obj, bool * freearg){
	CvArr * cvarr;
	*freearg = false;

	// check if OpenCV type
	if( PySwigObject_Check(obj) ){
		SWIG_ConvertPtr(obj, (void**)&cvarr, 0, SWIG_POINTER_EXCEPTION);
	}
	else if(PyList_Check(obj) || PyTuple_Check(obj)){
		cvarr = PySequence_to_CvArr( obj );
		*freearg = (cvarr != NULL);
	}
	else if(PyLong_Check(obj) && PyLong_AsLong(obj)==0){
		return NULL;
	}
	else {
		SWIG_ConvertPtr(obj, (void**)&cvarr, 0, SWIG_POINTER_EXCEPTION);
	}
	return cvarr;
}
%}
