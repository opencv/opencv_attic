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


// include python-specific files
%include "./nointpb.i"
%include "./pytypemaps.i"

%wrapper %{

    /* the wrapping code to enable the use of Python-based callbacks */

    /* a global variable to store the callback... Very uggly */
    static PyObject *my_cb_func = NULL;

    /* the internal C callback function which is responsible to call
       the Python real callback function */
    static void _internal_cb_func (int pos) {
	PyObject *result;

	/* the argument of the callback ready to be passed to Python code */
	PyObject *arg1 = PyInt_FromLong (pos);

	/* build the tuple for calling the Python callback */
	PyObject *arglist = Py_BuildValue ("(O)", arg1);

	/* call the Python callback */
	result = PyEval_CallObject (my_cb_func, arglist);

	/* cleanup */
	Py_XDECREF (result);
    }
%}

/**
 * typemap to memorize the Python callback when doing cvCreateTrackbar ()
 */
%typemap(in) CvTrackbarCallback {

    /* memorize the Python address of the callback function */
    my_cb_func = (PyObject *) $input;

    /* prepare to call the C function who will register the callback */
    $1 = (CvTrackbarCallback) _internal_cb_func;
}

%include "../general/highgui.i"

%pythoncode 
%{

__doc__ = """HighGUI provides minimalistic user interface parts and video input/output.

Dependent on the platform it was compiled on, this library provides methods
to draw a window for image display, capture video from a camera or framegrabber
or read/write video streams from/to the file system.

This wrapper was semi-automatically created from the C/C++ headers and therefore
contains no Python documentation. Because all identifiers are identical to their
C/C++ counterparts, you can consult the standard manuals that come with OpenCV.
"""

%}
