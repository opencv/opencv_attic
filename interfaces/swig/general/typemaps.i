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


// 2004-03-16, Gabriel Schreiber <schreiber@ient.rwth-aachen.de>
//             Mark Asbach       <asbach@ient.rwth-aachen.de>
//             Institute of Communications Engineering, RWTH Aachen University


/**
 * We set our own error handling function using cvRedirectError.
 * (See also error.h)
 * This function throws an error (OpenCV itself never throws) which 
 * we catch here. The error handling function (SendErrorToPython)
 * sets the Python error.
 * We need to return 0 here instead of an PyObject to tell Python
 * that an error has occured.
 */
%exception
    {
    try { $action } 
    catch (...) 
        {
        return NULL;
        } 
    }


/* include exception.i, so we can generate exceptions when we found errors */
%include "exception.i"

/* map one list of integer to the two parameters dimention/sizes */
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
