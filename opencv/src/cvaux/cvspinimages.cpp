/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
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

#include "_cvaux.h"
#include "cxmisc.h"

namespace cv
{

static void initRotationMat(const Point3f& n, float out[9])
{
    double pitch = atan2(n.x, n.z);
    double pmat[] = { cos(pitch), 0, -sin(pitch) ,
                        0      , 1,      0      ,
                     sin(pitch), 0,  cos(pitch) };

    double roll = atan2((double)n.y, n.x * pmat[3*2+0] + n.z * pmat[3*2+2]);

    double rmat[] = { 1,     0,         0,
                     0, cos(roll), -sin(roll) ,
                     0, sin(roll),  cos(roll) };

    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            out[3*i+j] = (float)(rmat[3*i+0]*pmat[3*0+j] +
                rmat[3*i+1]*pmat[3*1+j] + rmat[3*i+2]*pmat[3*2+j]);
}

static void transform(const Point3f& in, float matrix[9], Point3f& out)
{
    out.x = in.x * matrix[3*0+0] + in.y * matrix[3*0+1] + in.z * matrix[3*0+2];
    out.y = in.x * matrix[3*1+0] + in.y * matrix[3*1+1] + in.z * matrix[3*1+2];
    out.z = in.x * matrix[3*2+0] + in.y * matrix[3*2+1] + in.z * matrix[3*2+2];
}

#if CV_SSE2
static void convertTransformMatrix(const float* matrix, float* sseMatrix)
{
    sseMatrix[0] = matrix[0]; sseMatrix[1] = matrix[3]; sseMatrix[2] = matrix[6]; sseMatrix[3] = 0;
    sseMatrix[4] = matrix[1]; sseMatrix[5] = matrix[4]; sseMatrix[6] = matrix[7]; sseMatrix[7] = 0;
    sseMatrix[8] = matrix[2]; sseMatrix[9] = matrix[5]; sseMatrix[10] = matrix[8]; sseMatrix[11] = 0;
}

static inline __m128 transformSSE(const __m128* matrix, const __m128& in)
{
    assert(((size_t)matrix & 15) == 0);
    __m128 a0 = _mm_mul_ps(_mm_load_ps((float*)(matrix+0)), _mm_shuffle_ps(in,in,_MM_SHUFFLE(0,0,0,0)));
    __m128 a1 = _mm_mul_ps(_mm_load_ps((float*)(matrix+1)), _mm_shuffle_ps(in,in,_MM_SHUFFLE(1,1,1,1)));
    __m128 a2 = _mm_mul_ps(_mm_load_ps((float*)(matrix+2)), _mm_shuffle_ps(in,in,_MM_SHUFFLE(2,2,2,2)));

    return _mm_add_ps(_mm_add_ps(a0,a1),a2);
}

static inline __m128i _mm_mullo_epi32_emul(const __m128i& a, __m128i& b)
{    
    __m128i pack = _mm_packs_epi32(a, a);
    return _mm_unpacklo_epi16(_mm_mullo_epi16(pack, b), _mm_mulhi_epi16(pack, b));    
}

#endif
                        
void computeNormals( const OctTree& octtree, 
                     const Vector<Point3f>& centers, 
                     Vector<Point3f>& normals, 
                     Vector<uchar>& mask, 
                     float normalRadius,
                     int minNeighbors)
{    
    size_t normals_size = centers.size();

    normals.resize(normals_size);
    mask.resize(normals_size);
    
    Vector<Point3f> buffer;
    buffer.reserve(128);
    SVD svd;

    for(size_t n = 0; n < normals_size; ++n)
    {
        const Point3f& center = centers[n];

        octtree.getPointsWithinSphere(center, normalRadius, buffer);

        int buf_size = (int)buffer.size();
        if (buf_size < minNeighbors)
        {
            mask[n] = 0;
            continue;
        }

        //find the mean point for normalization
        Point3f mean;
        mean.x = 0.f;
        mean.y = 0.f;
        mean.z = 0.f;

        for(int i = 0; i < buf_size; ++i)
        {
            const Point3f& p = buffer[i];
            mean.x += p.x;
            mean.y += p.y;
            mean.z += p.z;
        }

        mean.x = mean.x / buf_size;
        mean.y = mean.y / buf_size;
        mean.z = mean.z / buf_size;
            
        double pxpx = 0;
        double pypy = 0;
        double pzpz = 0;

        double pxpy = 0;
        double pxpz = 0;
        double pypz = 0;

        for(int i = 0; i < buf_size; ++i)
        {
            const Point3f& p = buffer[i];

            pxpx += (p.x - mean.x) * (p.x - mean.x);
            pypy += (p.y - mean.y) * (p.y - mean.y);
            pzpz += (p.z - mean.z) * (p.z - mean.z);

            pxpy += (p.x - mean.x) * (p.y - mean.y);
            pxpz += (p.x - mean.x) * (p.z - mean.z);

            pypz += (p.y - mean.y) * (p.z - mean.z);
        }

        //create and populate matrix with normalized nbrs
        double M_data[] = { pxpx, pxpy, pxpz, /**/ pxpy, pypy, pypz, /**/ pxpz, pypz, pzpz };
        Mat M(3, 3, CV_64F, M_data);

        svd(M, SVD::MODIFY_A);

        normals[n] = Point3f(
            (float)((double*)svd.vt.data)[6],
            (float)((double*)svd.vt.data)[7],
            (float)((double*)svd.vt.data)[8]);

        mask[n] = 1;        
    }
}

void computeSpinImages( const OctTree& octtree, 
                       const Vector<Point3f>& points,
                       const Vector<Point3f>& normals,
                       Vector<uchar>& mask,
                       Mat& spinImages,
                       float support, 
                       float pixelsPerMeter)
{    
    assert(normals.size() == points.size());
    assert(mask.size() == points.size());
    
    size_t points_size = points.size();
    mask.resize(points_size);

    int height = cvRound( ceil(support * pixelsPerMeter) );
    int width  = cvRound( ceil(support * pixelsPerMeter) );
    spinImages.create( (int)points_size, width*height, CV_32F );

    int nthreads = getNumThreads();
    int i;

    Vector< Vector<Point3f> > pointsInSpherePool(nthreads);
    for(i = 0; i < nthreads; i++)
        pointsInSpherePool[i].reserve(2048);

#ifdef _OPENMP
    #pragma omp parallel for num_threads(nthreads)
#endif
    for(i = 0; i < (int)points_size; ++i)
    {
        if (mask[i] == 0)
            continue;

        int t = cvGetThreadNum();
        Vector<Point3f>& pointsInSphere = pointsInSpherePool[t];
                
        const Point3f& center = points[i];
        octtree.getPointsWithinSphere(center, support * std::sqrt(2.f), pointsInSphere);

        size_t inSphere_size = pointsInSphere.size();
        if (inSphere_size == 0)
        {
            mask[i] = 0;
            continue;
        }

        const Point3f& normal = normals[i];
        
        float rotmat[9];
        initRotationMat(normal, rotmat);
#if CV_SSE2
        __m128 rotmatSSE[3];
        convertTransformMatrix(rotmat, (float*)rotmatSSE);
#endif
        Point3f new_center;
        transform(center, rotmat, new_center);

        Mat spinImage = spinImages.row(i).reshape(1, height);
        float* spinImageData = (float*)spinImage.data;
        int step = width;
        spinImage = Scalar(0.);

        float alpha, beta;
        size_t j = 0;
#if CV_SSE2
        __m128 center_x4 = _mm_set1_ps(new_center.x);
        __m128 center_y4 = _mm_set1_ps(new_center.y);
        __m128 center_z4 = _mm_set1_ps(new_center.z + support);
        __m128 ppm4 = _mm_set1_ps(pixelsPerMeter);
        __m128i height4m1 = _mm_set1_epi32(spinImage.rows-1);
        __m128i width4m1 = _mm_set1_epi32(spinImage.cols-1);
        assert( spinImage.step <= 0xffff );
        __m128i step4 = _mm_set1_epi16((short)step);
        __m128i zero4 = _mm_setzero_si128();
        __m128i one4i = _mm_set1_epi32(1);
        __m128 zero4f = _mm_setzero_ps();
        __m128 one4f = _mm_set1_ps(1.f);
        __m128 two4f = _mm_set1_ps(2.f);
        int CV_DECL_ALIGNED(16) o[4];

        for (; j <= inSphere_size - 5; j += 4)
        {
            __m128 pt0 = transformSSE(rotmatSSE, _mm_loadu_ps((float*)&pointsInSphere[j+0])); // x0 y0 z0 .
            __m128 pt1 = transformSSE(rotmatSSE, _mm_loadu_ps((float*)&pointsInSphere[j+1])); // x1 y1 z1 .
            __m128 pt2 = transformSSE(rotmatSSE, _mm_loadu_ps((float*)&pointsInSphere[j+2])); // x2 y2 z2 .
            __m128 pt3 = transformSSE(rotmatSSE, _mm_loadu_ps((float*)&pointsInSphere[j+3])); // x3 y3 z3 .

            __m128 z0 = _mm_unpackhi_ps(pt0, pt1); // z0 z1 . .
            __m128 z1 = _mm_unpackhi_ps(pt2, pt3); // z2 z3 . .
            __m128 beta4 = _mm_sub_ps(center_z4, _mm_movelh_ps(z0, z1)); // b0 b1 b2 b3
            
            __m128 xy0 = _mm_unpacklo_ps(pt0, pt1); // x0 x1 y0 y1
            __m128 xy1 = _mm_unpacklo_ps(pt2, pt3); // x2 x3 y2 y3
            __m128 x4 = _mm_movelh_ps(xy0, xy1); // x0 x1 x2 x3
            __m128 y4 = _mm_movehl_ps(xy1, xy0); // y0 y1 y2 y3

            x4 = _mm_sub_ps(x4, center_x4);
            y4 = _mm_sub_ps(y4, center_y4);
            __m128 alpha4 = _mm_sqrt_ps(_mm_add_ps(_mm_mul_ps(x4,x4),_mm_mul_ps(y4,y4)));
            
            __m128 n1f4 = _mm_mul_ps(_mm_div_ps(beta4, two4f), ppm4);  /* beta4 float */
            __m128 n2f4 = _mm_mul_ps(alpha4, ppm4); /* alpha4 float */

            /* floor */
            __m128i n1 = _mm_sub_epi32(_mm_cvttps_epi32( _mm_add_ps( n1f4, one4f ) ), one4i);
            __m128i n2 = _mm_sub_epi32(_mm_cvttps_epi32( _mm_add_ps( n2f4, one4f ) ), one4i);

            __m128 f1 = _mm_sub_ps( n1f4, _mm_cvtepi32_ps(n1) );  /* { beta4  }  */
            __m128 f2 = _mm_sub_ps( n2f4, _mm_cvtepi32_ps(n2) );  /* { alpha4 }  */

            __m128 f1f2 = _mm_mul_ps(f1, f2);  // f1 * f2                        
            __m128 omf1omf2 = _mm_add_ps(_mm_sub_ps(_mm_sub_ps(one4f, f2), f1), f1f2); // (1-f1) * (1-f2)
            
            __m128i mask = _mm_and_si128(
                _mm_andnot_si128(_mm_cmpgt_epi32(zero4, n1), _mm_cmpgt_epi32(height4m1, n1)),
                _mm_andnot_si128(_mm_cmpgt_epi32(zero4, n2), _mm_cmpgt_epi32(width4m1, n2)));

            __m128 maskf = _mm_cmpneq_ps(_mm_cvtepi32_ps(mask), zero4f);
                        
            __m128 v00 = _mm_and_ps(        omf1omf2       , maskf); // a00 b00 c00 d00
            __m128 v01 = _mm_and_ps( _mm_sub_ps( f2, f1f2 ), maskf); // a01 b01 c01 d01
            __m128 v10 = _mm_and_ps( _mm_sub_ps( f1, f1f2 ), maskf); // a10 b10 c10 d10
            __m128 v11 = _mm_and_ps(          f1f2         , maskf); // a11 b11 c11 d11

            __m128i ofs4 = _mm_and_si128(_mm_add_epi32(_mm_mullo_epi32_emul(n1, step4), n2), mask);
            _mm_store_si128((__m128i*)o, ofs4);

            __m128 t0 = _mm_unpacklo_ps(v00, v01); // a00 a01 b00 b01
            __m128 t1 = _mm_unpacklo_ps(v10, v11); // a10 a11 b10 b11
            __m128 u0 = _mm_movelh_ps(t0, t1); // a00 a01 a10 a11
            __m128 u1 = _mm_movehl_ps(t1, t0); // b00 b01 b10 b11

            __m128 x0 = _mm_loadl_pi(u0, (__m64*)(spinImageData+o[0])); // x00 x01
            x0 = _mm_loadh_pi(x0, (__m64*)(spinImageData+o[0]+step));   // x00 x01 x10 x11
            x0 = _mm_add_ps(x0, u0);
            _mm_storel_pi((__m64*)(spinImageData+o[0]), x0);
            _mm_storeh_pi((__m64*)(spinImageData+o[0]+step), x0);

            x0 = _mm_loadl_pi(x0, (__m64*)(spinImageData+o[1]));        // y00 y01
            x0 = _mm_loadh_pi(x0, (__m64*)(spinImageData+o[1]+step));   // y00 y01 y10 y11
            x0 = _mm_add_ps(x0, u1);
            _mm_storel_pi((__m64*)(spinImageData+o[1]), x0);
            _mm_storeh_pi((__m64*)(spinImageData+o[1]+step), x0);

            t0 = _mm_unpackhi_ps(v00, v01); // c00 c01 d00 d01
            t1 = _mm_unpackhi_ps(v10, v11); // c10 c11 d10 d11
            u0 = _mm_movelh_ps(t0, t1); // c00 c01 c10 c11
            u1 = _mm_movehl_ps(t1, t0); // d00 d01 d10 d11

            x0 = _mm_loadl_pi(x0, (__m64*)(spinImageData+o[2]));        // z00 z01
            x0 = _mm_loadh_pi(x0, (__m64*)(spinImageData+o[2]+step));   // z00 z01 z10 z11
            x0 = _mm_add_ps(x0, u0);
            _mm_storel_pi((__m64*)(spinImageData+o[2]), x0);
            _mm_storeh_pi((__m64*)(spinImageData+o[2]+step), x0);

            x0 = _mm_loadl_pi(x0, (__m64*)(spinImageData+o[3]));        // w00 w01
            x0 = _mm_loadh_pi(x0, (__m64*)(spinImageData+o[3]+step));   // w00 w01 w10 w11
            x0 = _mm_add_ps(x0, u1);
            _mm_storel_pi((__m64*)(spinImageData+o[3]), x0);
            _mm_storeh_pi((__m64*)(spinImageData+o[3]+step), x0);
        }
#endif
        for (; j < inSphere_size; ++j)
        {
            Point3f pt;
            transform(pointsInSphere[j], rotmat, pt);

            beta = -(pt.z - new_center.z - support);
            if (beta >= 2 * support || beta < 0)
                continue;

            alpha = std::sqrt( (new_center.x - pt.x) * (new_center.x - pt.x) + 
                               (new_center.y - pt.y) * (new_center.y - pt.y) ); 
            
            float n1f = beta  * pixelsPerMeter * 0.5f;
            float n2f = alpha * pixelsPerMeter;

            int n1 = cvFloor(n1f);
            int n2 = cvFloor(n2f);

            float f1 = n1f - n1;
            float f2 = n2f - n2;

            if  ((unsigned)n1 >= (unsigned)(spinImage.rows-1) || 
                 (unsigned)n2 >= (unsigned)(spinImage.cols-1))
                continue;

            float *cellptr = spinImageData + step * n1 + n2;
            float f1f2 = f1*f2;
            cellptr[0] += 1 - f1 - f2 + f1f2;
            cellptr[1] += f2 - f1f2;
            cellptr[step] += f1 - f1f2;
            cellptr[step+1] += f1f2;
        }
        mask[i] = 1;
    }
}

}
