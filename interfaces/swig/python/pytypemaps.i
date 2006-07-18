/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/* if python sequence type, convert to CvMat or CvMatND */
%{
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
	else {
		SWIG_ConvertPtr(obj, (void**)&cvarr, 0, SWIG_POINTER_EXCEPTION);
	}
	return cvarr;
}
%}

%typemap(in) (CvArr *) (bool freearg=false){
	$1 = PyObject_to_CvArr($input, &freearg);
}
%typemap(freearg) (CvArr *) {
	if($1!=NULL && freearg$argnum){
		cvReleaseData( $1 );
		cvFree(&($1));
	}
}
%typecheck(SWIG_TYPECHECK_POINTER) CvArr * {
	void *ptr;
	if(PyList_Check($input) || PyTuple_Check($input)) {
		$1 = 1;
	}
	else if (SWIG_ConvertPtr($input, (void **) &ptr, 0, 0) == -1) {
		$1 = 0;
		PyErr_Clear();
	}
	else{
		$1 = 1;
	}
}

// for cvReshape, cvGetRow, where header is passed, then filled in
%typemap(in, numinputs=0) CvMat * OUTPUT (CvMat * header) {
	header = (CvMat *)malloc(sizeof(CvMat));
   	$1 = header;
}

%apply CvMat *OUTPUT {CvMat * header};
%apply CvMat *OUTPUT {CvMat * submat};

/* map scalar or sequence to CvScalar, CvPoint2D32f, CvPoint */
%typemap(in) (CvScalar) {
	CvScalar val;
	CvScalar * ptr;
	if( SWIG_ConvertPtr($input, (void **)&ptr, $descriptor( CvScalar * ), 0 ) == -1)
	{
		if(PyObject_AsDoubleArray($input, val.val, 4)==-1){
	    	SWIG_exception (SWIG_TypeError, "could not convert to CvScalar");
			return NULL;
		}
	}
	else{
		val = *ptr;
	}
	$1=val;
}
%typemap(in) (CvPoint) {
	CvPoint val;
	CvPoint *ptr;
	if( SWIG_ConvertPtr($input, (void**)&ptr, $descriptor(CvPoint *), 0) == -1) {
		if(PyObject_AsLongArray($input, (int *) &val, 2)==-1){
			SWIG_exception (SWIG_TypeError, "could not convert to CvPoint");
			return NULL;
		}
	}
	else{
		val = *ptr;
	}
	$1 = val;
}

%typemap(in) (CvPoint2D32f) {
    CvPoint2D32f val;
    CvPoint2D32f *ptr;
    if( SWIG_ConvertPtr($input, (void**)&ptr, $descriptor(CvPoint *), 0) == -1) {
        if(PyObject_AsFloatArray($input, (float *) &val, 2)==-1){
            SWIG_exception (SWIG_TypeError, "could not convert to CvPoint2D32f");
            return NULL;
        }
    }
    else{
        val = *ptr;
    }
    $1 = val;
}


/* typemap for cvGetDims */
%typemap(in) (const CvArr * arr, int * sizes = NULL) (CvArr * myarr, int mysizes[CV_MAX_DIM]){
	SWIG_Python_ConvertPtr($input, (void **) &myarr, 0, SWIG_POINTER_EXCEPTION);
	$1=myarr;
	$2=mysizes;
}

%typemap(argout) (const CvArr * arr, int * sizes = NULL) {
	int len = PyInt_AsLong( $result );
	PyObject * obj = PyTuple_FromIntArray( $2, len );
	Py_DECREF( $result );
	$result = obj;
}
				
/* map one list of integer to the two parameters dimension/sizes */
%typemap(in) (int dims, int* sizes) {
    int i;

    /* get the size of the dimention array */
    $1 = PyList_Size ($input);

    /* allocate the needed memory */
    $2 = (int *)malloc ($1 * sizeof (int));

    /* extract all the integer values from the list */
    for (i = 0; i < $1; i++) {
	PyObject *item = PyList_GetItem ($input, i);
	$2 [i] = (int)PyInt_AsLong (item);
    }
}

/* map one list of integer to the parameter idx of
   cvGetND, cvSetND, cvClearND, cvGetRealND, cvSetRealND and cvClearRealND */
%typemap(in) (int* idx) {
    int i;
    int size;

    /* get the size of the dimention array */
    size = PyList_Size ($input);

    /* allocate the needed memory */
    $1 = (int *)malloc (size * sizeof (int));

    /* extract all the integer values from the list */
    for (i = 0; i < size; i++) {
	PyObject *item = PyList_GetItem ($input, i);
	$1 [i] = (int)PyInt_AsLong (item);
    }
}

/* map a list of list of float to an matrix of floats*/
%typemap(in) float** ranges {
    int i1;
    int i2;
    int size1;
    int size2 = 0;

    /* get the number of lines of the matrix */
    size1 = PyList_Size ($input);

    /* allocate the correct number of lines for the destination matrix */
    $1 = (float **)malloc (size1 * sizeof (float *));

    for (i1 = 0; i1 < size1; i1++) {

	/* extract all the lines of the matrix */
	PyObject *list = PyList_GetItem ($input, i1);

	if (size2 == 0) {
	    /* size 2 wasn't computed before */
	    size2 = PyList_Size (list);
	} else if (size2 != PyList_Size (list)) {
	    /* the current line as a different size than the previous one */
	    /* so, generate an exception */
	    SWIG_exception (SWIG_ValueError, "Lines must be the same size");
	}

	/* allocate the correct number of rows for the current line */
	$1 [i1] = (float *)malloc (size2 * sizeof (float));

	/* extract all the float values of this row */
	for (i2 = 0; i2 < size2; i2++) {
	    PyObject *item = PyList_GetItem (list, i2);
	    $1 [i1][i2] = (float)PyFloat_AsDouble (item);
	}
    }
}

/**
 * map the output parameter of the cvGetMinMaxHistValue()
 * so, we can call cvGetMinMaxHistValue() in Python like:
 * min_value, max_value = cvGetMinMaxHistValue (hist, None, None)
 */
%apply float *OUTPUT {float *min_value};
%apply float *OUTPUT {float *max_value};
/**
 * map output parameters of cvMinMaxLoc
 */
%apply double *OUTPUT {double* min_val, double* max_val};

/**
 * the input argument of cvConvexHull2 "const CvArr *input" is converted from 
 * a list of CvPoint().
 */
%typemap(in) (const CvArr *input){
    int i;

    /* get the size of the input array */
    int size = PyList_Size ($input);

    /* allocate the points matrix necessary for calling cvConvexHull2 */
    CvPoint* points = (CvPoint *)malloc (size * sizeof (points[0]));
    CvMat pointMat = cvMat (1, size, CV_32SC2, points);
    $1 = &pointMat;

    /* allocat the output matrix to get the result of the call */
    int *hull = (int*)malloc (size * sizeof (hull[0]));
    hullMat2 = cvMat (1, size, CV_32SC1, hull);

    /* extract all the objects from the input list, and fill the
       points matrix */
    for (i = 0; i < size; i++) {

	/* get the current item */
	PyObject *item = PyList_GetItem ($input, i);

	/* convert from a Python CvPoint pointer to a C CvPoint pointer */
	CvPoint *p = NULL;
	SWIG_Python_ConvertPtr (item, (void **)&p, $descriptor(CvPoint *),
				SWIG_POINTER_EXCEPTION);

	/* extract the x and y positions */
	points [i].x = p->x;
	points [i].y = p->y;
    }
}

/**
 * the input argument of cvPolyLine "CvPoint** pts" is converted from 
 * a "list of list" (aka. an array) of CvPoint().
 * The next parameters "int* npts" and "int contours" are computed from
 * the givne list.
 */
%typemap(in) (CvPoint** pts, int* npts, int contours){
    int i;
    int j;
    int size2 = -1;
    CvPoint **points = NULL;
    int *nb_vertex = NULL;

    /* get the number of polylines input array */
    int size1 = PyList_Size ($input);
    $3 = size1;

    for (i = 0; i < size1; i++) {

	/* get the current item */
	PyObject *line = PyList_GetItem ($input, i);

	/* get the size of the current polyline */
	size2 = PyList_Size (line);

	if (points == NULL) {
	    /* create the points array */
	    points = (CvPoint **)malloc (size1 * sizeof (CvPoint *));

	    /* point to the created array for passing info to the C function */
	    $1 = points;

	    /* create the array for memorizing the vertex */
	    nb_vertex = (int *)malloc (size1 * sizeof (int));
	    $2 = nb_vertex;

	}

	/* allocate the necessary memory to store the points */
	points [i] = (CvPoint *)malloc (size2 * sizeof (CvPoint));
	    
	/* memorize the size of the polyline in the vertex list */
	nb_vertex [i] = size2;

	for (j = 0; j < size2; j++) {
	    /* get the current item */
	    PyObject *item = PyList_GetItem (line, j);

	    /* convert from a Python CvPoint pointer to a C CvPoint pointer */
	    CvPoint *p = NULL;
	    SWIG_Python_ConvertPtr (item, (void **)&p, $descriptor(CvPoint *),
				    SWIG_POINTER_EXCEPTION);

	    /* extract the x and y positions */
	    points [i][j].x = p->x;
	    points [i][j].y = p->y;
	}
    }
}

/**
 * what we need to cleanup for the input argument of cvConvexHull2
 * "const CvArr *input"
 */
%typemap(freearg) (const CvArr *input){
    free (((CvMat *)$1)->data.i);
}

/**
 * say we want to ignore the output parameter of cvConvexHull2
 */
%typemap(in, numinputs=0) (void *hull_storage) (CvMat hullMat) {
    $1 = &hullMat;
}

/**
 * convert the function output parameter hull_storage to a Python
 * list
 */
%typemap(argout) (void *hull_storage) {
    int i;
    PyObject *to_return;
    
    /* create the list to return */
    to_return = PyList_New (hullMat$argnum.cols);

    /* extract all the integer values of the result, and add it to the
       final resulting list */
    for (i = 0; i < hullMat$argnum.cols; i++) {
	PyList_SetItem (to_return, i,
			PyInt_FromLong (hullMat$argnum.data.i [i]));
    }

    /* some cleanup */
    free (hullMat$argnum.data.i);

    /* we can now return the value */
    $result = to_return;
}

/**
 * this is mainly an "output parameter"
 * So, just allocate the memory as input
 */
%typemap (in, numinputs=0) (CvSeq **first_contour) {
    CvSeq *seq = (CvSeq *)malloc (sizeof (CvSeq));
    $1 = &seq;
}

/**
 * return the finded contours with all the others parametres
 */
%typemap(argout) (CvSeq **first_contour) {
    PyObject *to_add;

    /* extract the pointer we want to add to the returned tuple */
    to_add = SWIG_NewPointerObj (*$1, $descriptor(CvSeq *), 0);

	$result = SWIG_AppendResult($result, &to_add, 1);
}

/**
 * IplImage ** image can be either one IplImage or one array of IplImage
 * (for example like in cvCalcHist() )
 * From Python, the array of IplImage can be a tuple.
 */
%typemap(in) (IplImage** image) {

    IplImage * one_image;

    /* first, check if this is just one IplImage */
    /* if this is just one IplImage, one_image will receive it */
    if ((SWIG_ConvertPtr($input, (void **) &one_image,
			 $descriptor(IplImage *),
			 0)) != -1) {

	/* Yes, just one IplImage, so pass it to the called function */
	$1 = &one_image;

    } else if PyTuple_Check ($input) {

	/* This is a tuple, so we need to test each element and pass
	   them to the called function */

	IplImage ** many_images;
	int i;

	/* get the size of the tuple */
	int nb = PyTuple_Size ($input);

	/* allocate the necessary place */
	many_images = (IplImage **)malloc (nb * sizeof (IplImage *));

	for (i = 0; i < nb; i++) {

	    /* convert the current tuple element to a IplImage *, and
	       store to many_images [i] */
	    SWIG_ConvertPtr(PyTuple_GetItem ($input, i),
			    (void **) &(many_images [i]),
			    $descriptor(IplImage *),
			    0);

	    /* check that the current item is a correct type */
	    if (SWIG_arg_fail ($argnum)) {
		/* incorrect ! */
		SWIG_fail;
	    }
	}

	/* what to give to the called function */
	$1 = many_images;

    } else {
	/* not a IplImage, not a tuple, this is wrong */
	return 0;
    }
}

/**
 * Map the CvFont * parameter from the cvInitFont() as an output parameter
 */
%typemap (in, numinputs=1) (CvFont* font, int font_face) {
    $1 = (CvFont *)malloc (sizeof (CvFont));
    $2 = (int)PyInt_AsLong ($input); 
    if (SWIG_arg_fail($argnum)) SWIG_fail;
}
%typemap(argout) (CvFont* font, int font_face) {
    PyObject *to_add;

    /* extract the pointer we want to add to the returned tuple */
    to_add = SWIG_NewPointerObj ($1, $descriptor(CvFont *), 0);

	$result = SWIG_AppendResult($result, &to_add, 1);
}

/**
 * these are output parameters for cvGetTextSize
 */
%typemap (in, numinputs=0) (CvSize* text_size, int* baseline) {
    CvSize *size = (CvSize *)malloc (sizeof (CvSize));
    int *baseline = (int *)malloc (sizeof (int));
    $1 = size;
    $2 = baseline;
}

/**
 * return the finded parameters for cvGetTextSize
 */
%typemap(argout) (CvSize* text_size, int* baseline) {
    PyObject * to_add[2];

    /* extract the pointers we want to add to the returned tuple */
    to_add [0] = SWIG_NewPointerObj ($1, $descriptor(CvSize *), 0);
    to_add [1] = PyInt_FromLong (*$2);

    $result = SWIG_AppendResult($result, to_add, 2);
}


/**
 * curr_features is output parameter for cvCalcOpticalFlowPyrLK
 */
%typemap (in, numinputs=1) (CvPoint2D32f* curr_features, int count)
     (int tmpCount) {
    /* as input, we only need the size of the wanted features */

    /* memorize the size of the wanted features */
    tmpCount = (int)PyInt_AsLong ($input);

    /* create the array for the C call */
    $1 = (CvPoint2D32f *) malloc(tmpCount * sizeof (CvPoint2D32f));

    /* the size of the array for the C call */
    $2 = tmpCount;
}

/**
 * the features returned by cvCalcOpticalFlowPyrLK
 */
%typemap(argout) (CvPoint2D32f* curr_features, int count) {
    int i;
    PyObject *to_add;
    
    /* create the list to return */
    to_add = PyList_New (tmpCount$argnum);

    /* extract all the points values of the result, and add it to the
       final resulting list */
    for (i = 0; i < tmpCount$argnum; i++) {
	PyList_SetItem (to_add, i,
			SWIG_NewPointerObj (&($1 [i]),
					    $descriptor(CvPoint2D32f *), 0));
    }

	$result = SWIG_AppendResult($result, &to_add, 1);
}

/**
 * status is an output parameters for cvCalcOpticalFlowPyrLK
 */
%typemap (in, numinputs=1) (char *status) (int tmpCountStatus){
    /* as input, we still need the size of the status array */

    /* memorize the size of the status array */
    tmpCountStatus = (int)PyInt_AsLong ($input);

    /* create the status array for the C call */
    $1 = (char *)malloc (tmpCountStatus * sizeof (char));
}

/**
 * the status returned by cvCalcOpticalFlowPyrLK
 */
%typemap(argout) (char *status) {
    int i;
    PyObject *to_add;

    /* create the list to return */
    to_add = PyList_New (tmpCountStatus$argnum);

    /* extract all the integer values of the result, and add it to the
       final resulting list */
    for (i = 0; i < tmpCountStatus$argnum; i++) {
		PyList_SetItem (to_add, i, PyBool_FromLong ($1 [i]));
    }

	$result = SWIG_AppendResult($result, &to_add, 1); 
}

/* map one list of points to the two parameters dimenssion/sizes
 for cvCalcOpticalFlowPyrLK */
%typemap(in) (CvPoint2D32f* prev_features) {
    int i;
    int size;

    /* get the size of the input array */
    size = PyList_Size ($input);

    /* allocate the needed memory */
    $1 = (CvPoint2D32f *)malloc (size * sizeof (CvPoint2D32f));

    /* extract all the points values from the list */
    for (i = 0; i < size; i++) {
	PyObject *item = PyList_GetItem ($input, i);

	CvPoint2D32f *p = NULL;
	SWIG_Python_ConvertPtr (item, (void **)&p,
				$descriptor(CvPoint2D32f*),
				SWIG_POINTER_EXCEPTION);
	$1 [i].x = p->x;
	$1 [i].y = p->y;
    }
}

/**
 * the corners returned by cvGoodFeaturesToTrack
 */
%typemap (in, numinputs=1) (CvPoint2D32f* corners, int* corner_count)
     (int tmpCount) {
    /* as input, we still need the size of the corners array */

    /* memorize the size of the status corners */
    tmpCount = (int)PyInt_AsLong ($input);

    /* create the corners array for the C call */
    $1 = (CvPoint2D32f *)malloc (tmpCount * sizeof (CvPoint2D32f));

    /* the size of the array for the C call */
    $2 = &tmpCount;
}

/**
 * the corners returned by cvGoodFeaturesToTrack
 */
%typemap(argout) (CvPoint2D32f* corners, int* corner_count) {
    int i;
    PyObject *to_add;
    
    /* create the list to return */
    to_add = PyList_New (tmpCount$argnum);

    /* extract all the integer values of the result, and add it to the
       final resulting list */
    for (i = 0; i < tmpCount$argnum; i++) {
	PyList_SetItem (to_add, i,
			SWIG_NewPointerObj (&($1 [i]),
					    $descriptor(CvPoint2D32f *), 0));
    }

    $result = SWIG_AppendResult($result, &to_add, 1);
}

/* map one list of points to the two parameters dimension/sizes
   for cvFindCornerSubPix */
%typemap(in, numinputs=1) (CvPoint2D32f* corners, int count)
     (int cornersCount, CvPoint2D32f* corners){
    int i;

	if(!PyList_Check($input)){
		PyErr_SetString(PyExc_TypeError, "cvFindCornerSubPix: Expected a list");
		return NULL;
	}

    /* get the size of the input array */
    cornersCount = PyList_Size ($input);
    $2 = cornersCount;

    /* allocate the needed memory */
    corners = (CvPoint2D32f *)malloc ($2 * sizeof (CvPoint2D32f));
    $1 = corners;

    /* the size of the array for the C call */

    /* extract all the points values from the list */
    for (i = 0; i < $2; i++) {
	PyObject *item = PyList_GetItem ($input, i);

	CvPoint2D32f *p = NULL;
	SWIG_Python_ConvertPtr (item, (void **)&p,
				$descriptor(CvPoint2D32f*),
				SWIG_POINTER_EXCEPTION);
	$1 [i].x = p->x;
	$1 [i].y = p->y;
    }

}

/**
 * the corners returned by cvFindCornerSubPix
 */
%typemap(argout) (CvPoint2D32f* corners, int count) {
    int i;
    PyObject *to_add;

    /* create the list to return */
    to_add = PyList_New (cornersCount$argnum);

    /* extract all the corner values of the result, and add it to the
       final resulting list */
    for (i = 0; i < cornersCount$argnum; i++) {
	PyList_SetItem (to_add, i,
			SWIG_NewPointerObj (&(corners$argnum [i]),
					    $descriptor(CvPoint2D32f *), 0));
    }

	$result = SWIG_AppendResult( $result, &to_add, 1);
}

#if 0
/**
 * return the corners for cvFindChessboardCorners
 * TODO: fix this to work simultaneously with cvFindCornerSubPix
 */
%typemap(in, numinputs=1) (CvSize pattern_size, CvPoint2D32f * corners, int * corner_count) 
     (int tmpCount) {
	CvSize * pattern_size;
	if( SWIG_ConvertPtr($input, (void **)&pattern_size, $descriptor( CvSize * ), SWIG_POINTER_EXCEPTION ) == -1){
		return NULL;
	}

    tmpCount = pattern_size->width*pattern_size->height;
	CvPoint2D32f * c = (CvPoint2D32f *) malloc(sizeof(CvPoint2D32f)*tmpCount);
	$1 = *pattern_size;
	$2 = c;
	$3 = NULL;
}
%typemap(argout) (CvPoint2D32f * corners, int * corner_count) {
    PyObject *to_add_1;

    /* extract the pointers we want to add to the returned tuple */
    to_add_1 = SWIG_NewPointerObj ($1, $descriptor(CvPoint2D32f *), 0);

	$result = SWIG_AppendResult($result, &to_add_1, 1);
}
#endif

/**
 * return the matrices for cvCameraCalibrate
 */
%typemap(in, numinputs=0) (CvMat * intrinsic_matrix, CvMat * distortion_coeffs)
{
	$1 = cvCreateMat(3,3,CV_32F);
	$2 = cvCreateMat(4,1,CV_32F);
}

