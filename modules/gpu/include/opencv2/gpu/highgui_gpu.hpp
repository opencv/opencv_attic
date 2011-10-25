/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

#ifndef __OPENCV_HIGHGUI_GPU_HPP__
#define __OPENCV_HIGHGUI_GPU_HPP__

#include <string>
#include <vector>
#include <utility>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/gpu/gpu.hpp"

namespace cv 
{
    namespace gpu
    {
        //! set a CUDA device to use OpenGL interoperability
        CV_EXPORTS void setGLDevice(int device = 0);

        CV_EXPORTS void imshow(const std::string& windowName, const GpuMat& img);

        enum
        {
            FONT_8_BY_13 = 0,
            FONT_9_BY_15,
            FONT_TIMES_ROMAN_10,
            FONT_TIMES_ROMAN_24,
            FONT_HELVETICA_10,
            FONT_HELVETICA_12,
            FONT_HELVETICA_18
        };

        CV_EXPORTS void imshow(const std::string& windowName, const GpuMat& img, const std::string& text,            
            const Point& textLoc = Point(0, 0), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);
















        class CV_EXPORTS GlResource
        {
        public:
            GlResource();
            GlResource(unsigned int buffer);

            ~GlResource();
            
            void registerBuffer(unsigned int buffer);
            void unregisterBuffer();

            void copyFrom(const GpuMat& mat);

            GpuMat map(int rows, int cols, int type);
            GpuMat map(const Size& sz, int type) { return map(sz.height, sz.width, type); }

            void unmap();

        private:
            class Impl;
            Impl* impl_;
            int* refcount_;
        };

        class CV_EXPORTS GlBuffer
        {
        public:
            explicit GlBuffer(unsigned int buf_type) : rows_(0), cols_(0), type_(0), buf_type_(buf_type), buffer_(0) {}
            GlBuffer(int rows, int cols, int type, unsigned int buf_type) : rows_(0), cols_(0), type_(0), buf_type_(buf_type), buffer_(0) { create(rows, cols, type); }
            GlBuffer(const Size& size, int type, unsigned int buf_type) : rows_(0), cols_(0), type_(0), buf_type_(buf_type), buffer_(0) { create(size, type); }
            ~GlBuffer() { release(); }

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }
            void release();

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat) { create(mat.rows, mat.cols, mat.type()); res_.copyFrom(mat); }

            GpuMat map() { return res_.map(rows_, cols_, type_); }
            void unmap() { res_.unmap(); }

            int rows() const { return rows_; }
            int cols() const { return cols_; }
            Size size() const { return Size(cols_, rows_); }
            bool empty() const { return rows_ == 0 || cols_ == 0; }

            int type() const { return type_; }
            int depth() const { return CV_MAT_DEPTH(type_); }
            int channels() const { return CV_MAT_CN(type_); }
            int elemSize() const { return CV_ELEM_SIZE(type_); }
            int elemSize1() const { return CV_ELEM_SIZE1(type_); }

        private:
            int rows_;
            int cols_;
            int type_;

            unsigned int buf_type_;
            unsigned int buffer_;

            GlResource res_;

            GlBuffer(const GlBuffer&);
            GlBuffer& operator =(const GlBuffer&);
        };

        class CV_EXPORTS Texture2D : private GlBuffer
        {
        public:
	        Texture2D();
            Texture2D(int rows, int cols, int type);
            Texture2D(const Size& size, int type);
            ~Texture2D() { release(); }

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }
            void release();

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat) { create(mat.rows, mat.cols, mat.type()); GlBuffer::copyFrom(mat); }

            using GlBuffer::map;
            using GlBuffer::unmap;
            
            using GlBuffer::rows;
            using GlBuffer::cols;
            using GlBuffer::size;
            using GlBuffer::empty;
            
            using GlBuffer::type;
            using GlBuffer::depth;
            using GlBuffer::channels;
            using GlBuffer::elemSize;
            using GlBuffer::elemSize1;

        private:
            unsigned int tex_;

            int internalFormat_;
            int format_;

            Texture2D(const Texture2D&);
            Texture2D& operator =(const Texture2D&);
        };

        class CV_EXPORTS VertexBuffer : private GlBuffer
        {
        public:
	        VertexBuffer();
            VertexBuffer(int rows, int cols, int type);
            VertexBuffer(const Size& size, int type);

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat) { create(mat.rows, mat.cols, mat.type()); GlBuffer::copyFrom(mat); }

            using GlBuffer::map;
            using GlBuffer::unmap;
            
            using GlBuffer::rows;
            using GlBuffer::cols;
            using GlBuffer::size;
            using GlBuffer::empty;
            
            using GlBuffer::type;
            using GlBuffer::depth;
            using GlBuffer::channels;
            using GlBuffer::elemSize;
            using GlBuffer::elemSize1;

        private:
            VertexBuffer(const VertexBuffer&);
            VertexBuffer& operator =(const VertexBuffer&);
        };

        class CV_EXPORTS ColorBuffer : private GlBuffer
        {
        public:
	        ColorBuffer();
            ColorBuffer(int rows, int cols, int type);
            ColorBuffer(const Size& size, int type);

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat) { create(mat.rows, mat.cols, mat.type()); GlBuffer::copyFrom(mat); }

            using GlBuffer::map;
            using GlBuffer::unmap;
            
            using GlBuffer::rows;
            using GlBuffer::cols;
            using GlBuffer::size;
            using GlBuffer::empty;
            
            using GlBuffer::type;
            using GlBuffer::depth;
            using GlBuffer::channels;
            using GlBuffer::elemSize;
            using GlBuffer::elemSize1;

        private:
            ColorBuffer(const ColorBuffer&);
            ColorBuffer& operator =(const ColorBuffer&);
        };
    }
}

#endif // __OPENCV_HIGHGUI_GPU_HPP__
