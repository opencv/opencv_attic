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

#include "opencv2/gpu/gpu.hpp"

namespace cv 
{
    namespace gpu
    {
        class CV_EXPORTS GlTexture
        {
        public:
	        GlTexture();

            GlTexture(int rows, int cols, int type);
            GlTexture(const Size& size, int type);

            explicit GlTexture(const GpuMat& mat);

            GlTexture(const GlTexture& other);

            ~GlTexture();

            GlTexture& operator =(const GlTexture& other);

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }
            void release();

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat, Stream& stream = Stream::Null());

            GpuMat map(Stream& stream = Stream::Null());
            void unmap(Stream& stream = Stream::Null());

            int rows() const;
            int cols() const;
            Size size() const;
            bool empty() const;

            int type() const;
            int depth() const;
            int channels() const;
            int elemSize() const;
            int elemSize1() const;

            void swap(GlTexture& other);

        private:
            class Impl;
            Impl* impl_;

            int* refcount_;
        };

        static inline void swap(GlTexture& a, GlTexture& b) { a.swap(b); }

        class CV_EXPORTS GlVertexBuffer
        {
        public:
	        GlVertexBuffer();

            GlVertexBuffer(int rows, int cols, int type);
            GlVertexBuffer(const Size& size, int type);

            explicit GlVertexBuffer(const GpuMat& mat);

            GlVertexBuffer(const GlVertexBuffer& other);

            ~GlVertexBuffer();

            GlVertexBuffer& operator =(const GlVertexBuffer& other);

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }
            void release();

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat, Stream& stream = Stream::Null());

            GpuMat map(Stream& stream = Stream::Null());
            void unmap(Stream& stream = Stream::Null());

            int rows() const;
            int cols() const;
            Size size() const;
            bool empty() const;

            int type() const;
            int depth() const;
            int channels() const;
            int elemSize() const;
            int elemSize1() const;

            void swap(GlVertexBuffer& other);

        private:
            class Impl;
            Impl* impl_;

            int* refcount_;
        };

        static inline void swap(GlVertexBuffer& a, GlVertexBuffer& b) { a.swap(b); }

        class CV_EXPORTS GlColorBuffer
        {
        public:
	        GlColorBuffer();

            GlColorBuffer(int rows, int cols, int type);
            GlColorBuffer(const Size& size, int type);

            explicit GlColorBuffer(const GpuMat& mat);

            GlColorBuffer(const GlColorBuffer& other);

            ~GlColorBuffer();

            GlColorBuffer& operator =(const GlColorBuffer& other);

            void create(int rows, int cols, int type);
            void create(const Size& size, int type) { create(size.height, size.width, type); }
            void release();

            void bind() const;
            void unbind() const;

            void copyFrom(const GpuMat& mat, Stream& stream = Stream::Null());

            GpuMat map(Stream& stream = Stream::Null());
            void unmap(Stream& stream = Stream::Null());

            int rows() const;
            int cols() const;
            Size size() const;
            bool empty() const;

            int type() const;
            int depth() const;
            int channels() const;
            int elemSize() const;
            int elemSize1() const;

            void swap(GlColorBuffer& other);

        private:
            class Impl;
            Impl* impl_;

            int* refcount_;
        };

        static inline void swap(GlColorBuffer& a, GlColorBuffer& b) { a.swap(b); }

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

        //! set a CUDA device to use OpenGL interoperability
        CV_EXPORTS void setGlDevice(int device = 0);

        CV_EXPORTS void imshow(const std::string& windowName, const GpuMat& img);

        CV_EXPORTS void imshow(const std::string& windowName, const GlTexture& tex);

        CV_EXPORTS void imshow(const std::string& windowName, const GpuMat& img, const std::string& text,            
            const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void imshow(const std::string& windowName, const GlTexture& tex, const std::string& text,            
            const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        class CV_EXPORTS Camera
        {
        public:
            Camera();

            void lookAt(const Point3d& eye, const Point3d& center, const Point3d& up);

            void setScale(const Point3d& scale);

            void setProjectionMatrix(const Mat& projectionMatrix);
            void setPerspectiveProjection(double fov, double zNear = 0.1, double zFar = 1000.0);
            void setOrthoProjection(double left = 0, double right = 1, double bottom = 1, double top = 0, double zNear = -1, double zFar = 1);

        protected:
            Point3d eye_;
            Point3d center_;
            Point3d up_;
            
            Point3d scale_;

            Mat projectionMatrix_;

            bool perspectiveProjection_;

            double fov_;

            double left_;
            double right_;
            double bottom_;
            double top_;

            double zNear_;
            double zFar_;
        };
        
        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color = Scalar::all(255));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color = Scalar::all(255));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors);
        
        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color,
            const GpuMat& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors,
            const GpuMat& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors,
            const GpuMat& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color,
            const GpuMat& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors,
            const GpuMat& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));
        
        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color,
            const GpuMat& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors,
            const GpuMat& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors,
            const GpuMat& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color,
            const GpuMat& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors,
            const GpuMat& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);




        

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color,
            const GlTexture& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors,
            const GlTexture& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors,
            const GlTexture& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color,
            const GlTexture& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors,
            const GlTexture& img, const Point2d& topLeft = Point2d(0.75, 0.0), const Point2d& bottonRight = Point2d(1.0, 0.25));
        
        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const Scalar& color,
            const GlTexture& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GpuMat& colors,
            const GlTexture& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GpuMat& points, const Camera& camera, const GlColorBuffer& colors,
            const GlTexture& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const Scalar& color,
            const GlTexture& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);

        CV_EXPORTS void pointCloudShow(const std::string& windowName, const GlVertexBuffer& points, const Camera& camera, const GlColorBuffer& colors,
            const GlTexture& img, const Point2d& topLeft, const Point2d& bottonRight, 
            const std::string& text, const Point2d& textLoc = Point2d(), const Scalar& textColor = Scalar::all(255), int textFont = FONT_9_BY_15);
    }
}

#endif // __OPENCV_HIGHGUI_GPU_HPP__
