/* These functions need the SWIG_* functions defined in the wrapper */
%{

static CvArr * PyObject_to_CvArr(PyObject * obj, bool * freearg);
static CvArr * PySequence_to_CvArr( PyObject * obj );

// convert a python sequence/array/list object into a c-array
#define PyObject_AsArrayImpl(func, ctype, ptype)                              \
	int func(PyObject * obj, ctype * array, int len){                         \
	void * mat_vptr=NULL;                                                     \
	void * im_vptr=NULL;                                                      \
	if(PyNumber_Check(obj)){                                                  \
		memset( array, 0, sizeof(ctype)*len );                                \
		array[0] = PyObject_As##ptype( obj );                                 \
	}                                                                         \
	else if(PyList_Check(obj) || PyTuple_Check(obj)){                         \
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
	else if( SWIG_ConvertPtr(obj, &mat_vptr, SWIGTYPE_p_CvMat, 0)!=-1 ||      \
	         SWIG_ConvertPtr(obj, &im_vptr, SWIGTYPE_p__IplImage, 0)!=-1)     \
	{                                                                         \
		CvMat * mat = (CvMat *) mat_vptr;                                     \
		CvMat stub;                                                           \
		if(im_vptr) mat = cvGetMat(im_vptr, &stub);                           \
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

/* Check if this object can be interpreted as a CvScalar */
static bool CvScalar_Check(PyObject * obj){
	void * vptr;
    CvScalar val;
	return SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvScalar,     0 ) != -1 ||
	       SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint2D32f, 0 ) != -1 ||
           SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint,      0 ) != -1 ||
	       PyObject_AsDoubleArray(obj, val.val, 4) !=-1;
}

static CvScalar PyObject_to_CvScalar(PyObject * obj){
	CvScalar val;
	CvScalar * ptr;
    CvPoint2D32f *ptr2D32f;
	CvPoint *pt_ptr;
	void * vptr;
	if( SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvScalar, 0 ) != -1)
	{
		ptr = (CvScalar *) vptr;
		return *ptr;
	}
	if( SWIG_ConvertPtr(obj, (void**)&ptr2D32f, SWIGTYPE_p_CvPoint2D32f, 0) != -1) {
        return cvScalar(ptr2D32f->x, ptr2D32f->y);
    }
    if( SWIG_ConvertPtr(obj, (void**)&pt_ptr, SWIGTYPE_p_CvPoint, 0) != -1) {
        return cvScalar(pt_ptr->x, pt_ptr->y);
    }
	if(PyObject_AsDoubleArray(obj, val.val, 4)!=-1){
		return val;
	}
	return cvScalar(-1,-1,-1,-1); 
}

/* if python sequence type, convert to CvMat or CvMatND */
static CvArr * PyObject_to_CvArr(PyObject * obj, bool * freearg){
	CvArr * cvarr;
	*freearg = false;

	// check if OpenCV type
	if( PySwigObject_Check(obj) ){
		SWIG_ConvertPtr(obj, &cvarr, 0, SWIG_POINTER_EXCEPTION);
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

static int PyObject_GetElemType(PyObject * obj){
	void *vptr;
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint, 0) != -1) return CV_32SC2;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvSize, 0) != -1) return CV_32SC2;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvRect, 0) != -1) return CV_32SC4;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvSize2D32f, 0) != -1) return CV_32FC2;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint2D32f, 0) != -1) return CV_32FC2;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint3D32f, 0) != -1) return CV_32FC3;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint2D64f, 0) != -1) return CV_64FC2;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvPoint3D64f, 0) != -1) return CV_64FC3;	
	if(SWIG_ConvertPtr(obj, &vptr, SWIGTYPE_p_CvScalar, 0) != -1) return CV_64FC4;	
	if(PyTuple_Check(obj) || PyList_Check(obj)) return CV_MAKE_TYPE(CV_32F, PySequence_Size( obj ));
	if(PyLong_Check(obj)) return CV_32S;
	return CV_32F;
}

// Would like this to convert Python lists to CvMat
// Also lists of CvPoints, CvScalars, CvMats? etc
static CvArr * PySequence_to_CvArr( PyObject * obj ){
	int dims[CV_MAX_DIM] = {1,1,1};
	int ndim=0;
	int cvtype;
	PyObject * item;
	
	// figure out dimensions
	for(item = obj; 
		(PyTuple_Check(item) || PyList_Check(item));
		item = PySequence_GetItem(item, 0))
	{
		dims[ndim] = PySequence_Size( item ); 
		ndim++;
	}

	
	if(ndim==0){
		PyErr_SetString(PyExc_TypeError, "Cannot convert an empty python object to a CvArr");
		return NULL;
	}
	
	cvtype = PyObject_GetElemType(item);
	// collapse last dim into NCH if we found a single channel, but the last dim is <=3
	if(CV_MAT_CN(cvtype)==1 && dims[ndim-1]>1 && dims[ndim-1]<4){
		cvtype=CV_MAKE_TYPE(cvtype, dims[ndim-1]);
		dims[ndim-1]=1;	
		ndim--;
	}
	
	if(cvtype==-1){
		PyErr_SetString(PyExc_TypeError, "Could not determine OpenCV element type of Python sequence");
		return NULL;
	}
	
	// CvMat
	if(ndim<=2){
		CvMat *m = cvCreateMat(dims[0], dims[1], cvtype);
		for(int i=0; i<dims[0]; i++){
			PyObject * rowobj = PySequence_GetItem(obj, i);
			if( dims[1] > 1 ){
				// double check size
				assert((PyTuple_Check(rowobj) || PyList_Check(rowobj)) && 
						PySequence_Size(rowobj) == dims[1]);

				for(int j=0; j<dims[1]; j++){
					PyObject * colobj = PySequence_GetItem(rowobj, j);
					cvSet2D( m, i, j, PyObject_to_CvScalar( colobj ) );
				}
			}
			else{
				cvSet1D(m, i, PyObject_to_CvScalar( rowobj ) );
			}
		}
		return (CvArr *) m;
	}

	// CvMatND
	PyErr_SetString(PyExc_TypeError, "Cannot convert Python Object to CvArr -- ndim > 3");
	return NULL;
	
}
%}
