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



/// Accessor to convert a Python string into the imageData.
%extend _IplImage
{
    void imageData_set(PyObject* object)
        {
        char* py_string = PyString_AsString(object);
        
        if ((self->nChannels == 3) && ((self->depth & 0xff) == IPL_DEPTH_8U))
            {
            // RGB case
            // The data is reordered beause OpenCV uses BGR instead of RGB
            
            for (long line = 0; line < self->height; ++line)
                for (long pixel = 0; pixel < self->width; ++pixel)
                    {
                    // In OpenCV the beginning of the lines are aligned
                    // to 4 Bytes. So use widthStep instead of width.
                    long position = line*self->widthStep + pixel*3;
                    long sourcepos = line*self->width*3 + pixel*3;
                    self->imageData[position  ] = py_string[sourcepos+2];
                    self->imageData[position+1] = py_string[sourcepos+1];
                    self->imageData[position+2] = py_string[sourcepos  ];
                    }
            }
        else if ((self->nChannels == 1) && ((self->depth & 0xff) == IPL_DEPTH_8U))
            {
            // Grayscale 8bit case
            
            for (long line = 0; line < self->height; ++line)
                {
                // In OpenCV the beginning of the lines are aligned
                // to 4 Bytes. So use widthStep instead of width.
                memcpy
                    (
                    self->imageData + line*self->widthStep,
                    py_string + line*self->width,
                    self->widthStep
                    );
                }
            }
        else if ((self->nChannels == 1) && ((self->depth & 0xff) == IPL_DEPTH_32F))
            {
            // Float 32bit case
            
            for (long line = 0; line < self->height; ++line)
                {
                // here we don not have to care about alignment as the Floats are
                // as long as the alignment
                memcpy
                    (
                    self->imageData + line*self->widthStep,
                    py_string + line*self->width*4,
                    self->widthStep
                    );
                }
            }
        }
}


/// Accessor to convert the imageData into a Python string.
%extend _IplImage
{
    PyObject* imageData_get() 
    {
        if (self->depth % 8 != 0)
            return 0;
        if (!self->imageData)
            return 0;
        return PyString_FromStringAndSize(self->imageData, self->imageSize);
    }
}   

