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

// 2006-08-29  Roman Stanchak -- converted to use CvMat rather than IplImage


%{

/// Accessor to convert a Python string into the imageData.
void CvMat_imageData_set(CvMat * self, PyObject* object)
{
	char* py_string = PyString_AsString(object);

	if (self->type == CV_8UC3){
		// RGB case
		// The data is reordered beause OpenCV uses BGR instead of RGB

		for (long line = 0; line < self->rows; ++line)
			for (long pixel = 0; pixel < self->cols; ++pixel)
			{
				// In OpenCV the beginning of the lines are aligned
				// to 4 Bytes. So use step instead of cols.
				long position = line*self->step + pixel*3;
				long sourcepos = line*self->cols*3 + pixel*3;
				self->data.ptr[position  ] = py_string[sourcepos+2];
				self->data.ptr[position+1] = py_string[sourcepos+1];
				self->data.ptr[position+2] = py_string[sourcepos  ];
			}
	}
	else if (self->type == CV_8UC1)
	{
		// Grayscale 8bit case

		for (long line = 0; line < self->rows; ++line)
		{
			// In OpenCV the beginning of the lines are aligned
			// to 4 Bytes. So use step instead of cols.
			memcpy
				(
				 self->data.ptr + line*self->step,
				 py_string + line*self->cols,
				 self->step
				);
		}
	}
	else if (self->type == CV_32FC1 )
	{
		// Float 32bit case

		for (long line = 0; line < self->rows; ++line)
		{
			// here we don not have to care about alignment as the Floats are
			// as long as the alignment
			memcpy
				(
				 self->data.ptr + line*self->step,
				 py_string + line*self->cols*4,
				 self->step
				);
		}
	}
}

/// Accessor to convert the imageData into a Python string.
PyObject* CvMat_imageData_get(CvMat * self) 
{
	if (!self->data.ptr)
	{
		PyErr_SetString(PyExc_TypeError, "Data pointer of CvMat is NULL");
		return NULL;
	}		 
	return PyString_FromStringAndSize((const char *)self->data.ptr, self->rows*self->step);
}

%}

// add virtual member variable to CvMat
%extend CvMat {
	PyObject * imageData;
};
