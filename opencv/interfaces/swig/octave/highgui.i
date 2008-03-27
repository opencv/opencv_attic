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

// todo remove these..
#pragma SWIG nowarn=312,362,303,365,366,367,368,370,371,372,451,454,503

%{
	#include <cxtypes.h>
	#include <cv.h>
	#include <highgui.h>
	#include "octhelpers.h"
	#include "octcvseq.hpp"
%}
// include octave-specific files
%include "./octtypemaps.i"
%include "exception.i"

/* the wrapping code to enable the use of Octave-based mouse callbacks */
%header %{
	/* This encapsulates the octave callback and user_data for mouse callback */
	struct OctCvMouseCBData {
	  /*
		octave_value oct_func;
		void * user_data;
	  */
	};
	// This encapsulates the octave callback and user_data for mouse callback
	// C helper function which is responsible for calling
	// the Octave real trackbar callback function
    static void icvOctOnMouse (int event, int x, int y,
					 int flags, OctCvMouseCBData * param) {
      /*
		// Must ensure this thread has a lock on the interpreter
		PyGILState_STATE state = PyGILState_Ensure();

		octave_valueresult;

		// the argument of the callback ready to be passed to Octave code
		octave_valuearg1 = PyInt_FromLong (event);
		octave_valuearg2 = PyInt_FromLong (x);
		octave_valuearg3 = PyInt_FromLong (y);
		octave_valuearg4 = PyInt_FromLong (flags);
		octave_valuearg5 = (octave_value)param->user_data;  // assume this is already a PyObject

		// build the tuple for calling the Octave callback
		octave_valuearglist = Py_BuildValue ("(OOOOO)",
				arg1, arg2, arg3, arg4, arg5);

		// call the Octave callback
		result = PyEval_CallObject (param->py_func, arglist);

		 Errors in Octave callback get swallowed, so report them here
		if(!result){
			PyErr_Print();
			cvError( CV_StsInternal, "icvPyOnMouse", "", __FILE__, __LINE__);
		}

		// cleanup
		Py_XDECREF (result);

		// Release Interpreter lock
		PyGILState_Release(state);
      */
	}
%}
/**
 * adapt cvSetMouseCallback to use octave callback
 */
%rename (cvSetMouseCallbackOld) cvSetMouseCallback;
%rename (cvSetMouseCallback) cvSetMouseCallbackOct;
%inline %{
	void cvSetMouseCallbackOct( const char* window_name, octave_value on_mouse, void* param=NULL ){
	  /*
		// TODO potential memory leak if mouse callback is redefined
		PyCvMouseCBData * py_callback = new PyCvMouseCBData;
		py_callback->py_func = on_mouse;
		py_callback->user_data = param ? param : Py_None;
		cvSetMouseCallback( window_name, (CvMouseCallback) icvPyOnMouse, (void *) py_callback );
	  */
	}
%}



/**
 * The following code enables trackbar callbacks from octave.  Unfortunately, there is no 
 * way to distinguish which trackbar the event originated from, so must hard code a 
 * fixed number of unique c callback functions using the macros below
 */
%wrapper %{
    /* C helper function which is responsible for calling
       the Octave real trackbar callback function */
    static void icvOctOnTrackbar( octave_value oct_cb_func, int pos) {
      /*
	
      // Must ensure this thread has a lock on the interpreter
		PyGILState_STATE state = PyGILState_Ensure();

		octave_valueresult;

		// the argument of the callback ready to be passed to Octave code
		octave_valuearg1 = PyInt_FromLong (pos);

		// build the tuple for calling the Octave callback
		octave_valuearglist = Py_BuildValue ("(O)", arg1);

		// call the Octave callback
		result = PyEval_CallObject (py_cb_func, arglist);

		// Errors in Octave callback get swallowed, so report them here
		if(!result){
			PyErr_Print();
			cvError( CV_StsInternal, "icvPyOnTrackbar", "", __FILE__, __LINE__);
		}


		// cleanup
		Py_XDECREF (result);

		// Release Interpreter lock
		PyGILState_Release(state);
      */
	}

#define ICV_OCT_MAX_CB 10

	struct OctCvTrackbar {
		CvTrackbarCallback cv_func;
		octave_value oct_func;
		octave_value oct_pos;
	};

	static int my_trackbar_cb_size=0;
	extern OctCvTrackbar my_trackbar_cb_funcs[ICV_OCT_MAX_CB];
%}

/* Callback table entry */
%define %ICV_OCT_CB_TAB_ENTRY(idx)
	{(CvTrackbarCallback) icvOctTrackbarCB##idx, octave_value(), octave_value() }
%enddef

/* Table of callbacks */
%define %ICV_OCT_CB_TAB
%wrapper %{
	OctCvTrackbar my_trackbar_cb_funcs[ICV_OCT_MAX_CB] = {
		%ICV_OCT_CB_TAB_ENTRY(0),
		%ICV_OCT_CB_TAB_ENTRY(1),
		%ICV_OCT_CB_TAB_ENTRY(2),
		%ICV_OCT_CB_TAB_ENTRY(3),
		%ICV_OCT_CB_TAB_ENTRY(4),
		%ICV_OCT_CB_TAB_ENTRY(5),
		%ICV_OCT_CB_TAB_ENTRY(6),
		%ICV_OCT_CB_TAB_ENTRY(7),
		%ICV_OCT_CB_TAB_ENTRY(8),
		%ICV_OCT_CB_TAB_ENTRY(9)
	};
%}	 
%enddef

/* Callback definition */
%define %ICV_OCT_CB_IMPL(idx) 
%wrapper %{
static void icvOctTrackbarCB##idx(int pos){                                      
  /*
	if(!my_trackbar_cb_funcs[idx].py_func) return;
	icvPyOnTrackbar( my_trackbar_cb_funcs[idx].py_func, pos );
  */
}                                                                               
%}
%enddef


%ICV_OCT_CB_IMPL(0);
%ICV_OCT_CB_IMPL(1);
%ICV_OCT_CB_IMPL(2);
%ICV_OCT_CB_IMPL(3);
%ICV_OCT_CB_IMPL(4);
%ICV_OCT_CB_IMPL(5);
%ICV_OCT_CB_IMPL(6);
%ICV_OCT_CB_IMPL(7);
%ICV_OCT_CB_IMPL(8);
%ICV_OCT_CB_IMPL(9);

%ICV_OCT_CB_TAB;


/**
 * typemap to memorize the Octave callback when doing cvCreateTrackbar ()
 */
%typemap(in) CvTrackbarCallback {
  /*
	if(my_trackbar_cb_size == ICV_OCT_MAX_CB){
		SWIG_exception(SWIG_IndexError, "Exceeded maximum number of trackbars");
	}

	my_trackbar_cb_size++;

	// memorize the Octave address of the callback function
	my_trackbar_cb_funcs[my_trackbar_cb_size-1].oct_func = (octave_value) $input;

	// prepare to call the C function who will register the callback
	$1 = my_trackbar_cb_funcs[ my_trackbar_cb_size-1 ].cv_func;
*/
}

/**
 * typemap so that cvWaitKey returns a character in all cases except -1
 */
%rename (cvWaitKeyC) cvWaitKey;
%rename (cvWaitKey) cvWaitKeyOct;
%inline %{
	octave_value cvWaitKeyOct(int delay=0){
	  /*
		// In order for the event processing thread to run a octave callback
		// it must acquire the global interpreter lock, but  cvWaitKey blocks, so
		// this thread can never release the lock. So release it here.
		OctThreadState * thread_state = OctEval_SaveThread();
		int res = cvWaitKey(delay);
		OctEval_RestoreThread( thread_state );
		
		char str[2]={(char)res,0};
		if(res==-1){
			return OctLong_FromLong(-1);
		}
		return OctString_FromString(str);
	  */
	  return octave_value();
	}
%}
// HighGUI Octave module initialization
// needed for callbacks to work in a threaded environment 
%init %{
  /*
	OctEval_InitThreads();
  */
%}


%include "../general/highgui.i"
