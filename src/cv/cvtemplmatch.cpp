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

#include "_cv.h"

#define ICV_DECL_CROSSCORR_DIRECT( flavor, arrtype, corrtype, worktype )    \
static CvStatus CV_STDCALL                                                  \
icvCrossCorrDirect_##flavor##_CnR( const arrtype* img0, int imgstep,        \
    CvSize imgsize, const arrtype* templ0, int templstep, CvSize templsize, \
    corrtype* corr, int corrstep, CvSize corrsize, int cn )                 \
{                                                                           \
    int x, i, j;                                                            \
    double* corrrow = 0;                                                    \
                                                                            \
    if( templsize.height > 1 )                                              \
        corrrow = (double*)cvStackAlloc( corrsize.width*sizeof(corrrow[0]));\
    templsize.width *= cn;                                                  \
    imgsize.width *= cn;                                                    \
    corrstep /= sizeof(corr[0]);                                            \
    imgstep /= sizeof(img0[0]);                                             \
    templstep /= sizeof(templ0[0]);                                         \
                                                                            \
    for( ; corrsize.height--; corr += corrstep, img0 += imgstep )           \
    {                                                                       \
        const arrtype* img = img0;                                          \
        const arrtype* templ = templ0;                                      \
                                                                            \
        for( i = 0; i < templsize.height; i++, img += imgstep,              \
                                          templ += templstep )              \
        {                                                                   \
            for( x = 0; x < corrsize.width; x++, img += cn )                \
            {                                                               \
                worktype sum = 0;                                           \
                int len = MIN( templsize.width, imgsize.width - x*cn );     \
                double t;                                                   \
                for( j = 0; j <= len - 4; j += 4 )                          \
                    sum += (worktype)(img[j])*templ[j] +                    \
                           (worktype)(img[j+1])*templ[j+1] +                \
                           (worktype)(img[j+2])*templ[j+2] +                \
                           (worktype)(img[j+3])*templ[j+3];                 \
                                                                            \
                for( ; j < len; j++ )                                       \
                    sum += (worktype)(img[j])*templ[j];                     \
                t = sum;                                                    \
                if( i > 0 )                                                 \
                    t += corrrow[x];                                        \
                if( i < templsize.height-1 )                                \
                    corrrow[x] = t;                                         \
                else                                                        \
                    corr[x] = (corrtype)t;                                  \
            }                                                               \
                                                                            \
            img -= corrsize.width*cn;                                       \
        }                                                                   \
    }                                                                       \
                                                                            \
    return CV_OK;                                                           \
}


ICV_DECL_CROSSCORR_DIRECT( 8u32f, uchar, float, int )
ICV_DECL_CROSSCORR_DIRECT( 32f, float, float, double )
ICV_DECL_CROSSCORR_DIRECT( 64f, double, double, double )

typedef CvStatus (CV_STDCALL * CvCrossCorrDirectFunc)(
    const void* img, int imgstep, CvSize imgsize,
    const void* templ, int templstep, CvSize templsize,
    void* corr, int corrstep, CvSize corrsize, int cn );


static void
icvCrossCorr( const CvArr* _img, const CvArr* _templ, CvArr* _corr )
{
    CvMat* dft_img = 0;
    CvMat* dft_templ = 0;
    CvMat* temp = 0;
    
    CV_FUNCNAME( "cvCrossCorr" );
    
    __BEGIN__;

    CvMat stub, *img = (CvMat*)_img;
    CvMat tstub, *templ = (CvMat*)_templ;
    CvMat cstub, *corr = (CvMat*)_corr;
    CvSize imgsize, templsize, corrsize, dftsize;
    int i, depth, cn, corr_type;
    double O_direct, O_fast;

    CV_CALL( img = cvGetMat( img, &stub ));
    CV_CALL( templ = cvGetMat( templ, &tstub ));
    CV_CALL( corr = cvGetMat( corr, &cstub ));

    if( CV_MAT_DEPTH( img->type ) != CV_8U &&
        CV_MAT_DEPTH( img->type ) != CV_32F )
        CV_ERROR( CV_StsUnsupportedFormat,
        "The function supports only 8u and 32f data types" );

    if( CV_MAT_TYPE( corr->type ) != CV_32FC1 &&
        CV_MAT_TYPE( corr->type ) != CV_64FC1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "The correlation output should be single-channel floating-point array" );

    if( !CV_ARE_TYPES_EQ( img, templ ))
        CV_ERROR( CV_StsUnmatchedSizes, "image and template should have the same type" );

    if( img->cols < templ->cols || img->rows < templ->rows )
    {
        CvMat* t;
        CV_SWAP( img, templ, t );
    }

    if( img->cols < templ->cols || img->rows < templ->rows )
        CV_ERROR( CV_StsUnmatchedSizes,
        "Neither of the two input arrays is smaller than the other" );

    if( corr->rows > img->rows + templ->rows - 1 ||
        corr->cols > img->cols + templ->cols - 1 )
        CV_ERROR( CV_StsUnmatchedSizes,
        "output image should not be greater than (W + w - 1)x(H + h - 1)" );

    depth = CV_MAT_DEPTH(img->type);
    cn = CV_MAT_CN(img->type);
    corr_type = CV_MAT_TYPE(corr->type);

    imgsize = cvGetMatSize(img);
    templsize = cvGetSize(templ);
    corrsize = cvGetSize(corr);
    dftsize.width = cvGetOptimalDFTSize(imgsize.width + templsize.width - 1);
    if( dftsize.width == 1 )
        dftsize.width = 2;
    dftsize.height = cvGetOptimalDFTSize(imgsize.height + templsize.height - 1);
    if( dftsize.width <= 0 || dftsize.height <= 0 )
        CV_ERROR( CV_StsOutOfRange, "the input arrays are too big" );

    // determine which method to use for correlation.
    O_direct = (double)corrsize.width * corrsize.height *
               (double)templsize.width * templsize.height; // approximate formulae

    // calculate it in two steps to avoid icc remark
    O_fast = (imgsize.height + templsize.height + corrsize.height)*
             (double)dftsize.width*log((double)dftsize.width);
    O_fast = (O_fast + 3*dftsize.width*(double)dftsize.height*log((double)dftsize.height))/CV_LOG2;

    if( O_direct < O_fast )
    {
        CvCrossCorrDirectFunc corr_func = 0;
        if( depth == CV_8U && corr_type == CV_32F )
            corr_func = (CvCrossCorrDirectFunc)icvCrossCorrDirect_8u32f_CnR;
        else if( depth == CV_32F && corr_type == CV_32F )
            corr_func = (CvCrossCorrDirectFunc)icvCrossCorrDirect_32f_CnR;
        else if( depth == CV_64F && corr_type == CV_64F )
            corr_func = (CvCrossCorrDirectFunc)icvCrossCorrDirect_64f_CnR;
        else
            CV_ERROR( CV_StsUnsupportedFormat,
            "Unsupported combination of input and output formats" );

        IPPI_CALL( corr_func( img->data.ptr, img->step, imgsize,
                              templ->data.ptr, templ->step, templsize,
                              corr->data.ptr, corr->step, corrsize, cn ));
    }
    else
    {
        CV_CALL( dft_img = cvCreateMat( dftsize.height, dftsize.width, corr_type ));
        CV_CALL( dft_templ = cvCreateMat( dftsize.height, dftsize.width, corr_type ));

        if( cn > 1 && depth != CV_MAT_DEPTH(corr_type) )
        {
            CV_CALL( temp = cvCreateMat( imgsize.height, imgsize.width, depth ));
        }

        for( i = 0; i < cn; i++ )
        {
            CvMat dftstub, srcstub;
            CvMat* src = img, *dst = &dftstub;
            CvMat* planes[] = { 0, 0, 0, 0 };
            cvGetSubRect( dft_img, dst, cvRect(0,0,img->cols,img->rows));
            
            if( cn > 1 )
            {
                planes[i] = temp ? temp : dst;
                cvSplit( img, planes[0], planes[1], planes[2], planes[3] );
                src = planes[i];
            }

            if( dst != src )
            {
                if( CV_ARE_TYPES_EQ(src, dst) )
                    cvCopy( src, dst );
                else
                    cvConvert( src, dst );
            }

            cvGetSubRect( dft_img, dst, cvRect(img->cols, 0,
                          dft_img->cols - img->cols, img->rows) );
            cvZero( dst );
            cvDFT( dft_img, dft_img, CV_DXT_FORWARD, img->rows );

            src = templ;
            cvGetSubRect( dft_templ, dst, cvRect(0,0,templ->cols,templ->rows));
            
            if( cn > 1 )
            {
                planes[i] = dst;
                if( temp )
                {
                    planes[i] = &srcstub;
                    cvGetSubRect( temp, planes[i], cvRect(0,0,templ->cols,templ->rows) );
                }
                cvSplit( templ, planes[0], planes[1], planes[2], planes[3] );
                src = planes[i];
            }

            if( dst != src )
            {
                if( CV_ARE_TYPES_EQ(src, dst) )
                    cvCopy( src, dst );
                else
                    cvConvert( src, dst );
            }

            cvGetSubRect( dft_templ, dst, cvRect(templ->cols, 0,
                          dft_templ->cols - templ->cols, templ->rows) );
            cvZero( dst );
            cvDFT( dft_templ, dft_templ, CV_DXT_FORWARD, templ->rows );

            cvMulSpectrums( dft_img, dft_templ, dft_img, CV_DXT_MUL_CONJ );
            cvDFT( dft_img, dft_img, CV_DXT_INV_SCALE );
            
            cvGetSubRect( dft_img, dst, cvRect(0,0,corr->cols,corr->rows) );
            if( i == 0 )
                cvCopy( dst, corr );
            else
                cvAdd( corr, dst, corr );
        }
    }

    __END__;

    cvReleaseMat( &dft_img );
    cvReleaseMat( &dft_templ );
    cvReleaseMat( &temp );
}


CV_IMPL void
cvMatchTemplate( const CvArr* _img, const CvArr* _templ, CvArr* _result, int method )
{
    CvMat* sum = 0;
    CvMat* sqsum = 0;
    
    CV_FUNCNAME( "cvMatchTemplate" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    int cn, i, j, k;
    CvMat stub, *img = (CvMat*)_img;
    CvMat tstub, *templ = (CvMat*)_templ;
    CvMat rstub, *result = (CvMat*)_result;
    CvScalar templ_mean = cvScalarAll(0);
    double templ_norm = 0, templ_sum2 = 0;
    
    int idx = 0, idx2 = 0;
    double *p0, *p1, *p2, *p3;
    double *q0, *q1, *q2, *q3;
    double inv_area;
    int sum_step, sqsum_step;
    int num_type = method == CV_TM_CCORR || method == CV_TM_CCORR_NORMED ? 0 :
                   method == CV_TM_CCOEFF || method == CV_TM_CCOEFF_NORMED ? 1 : 2;
    int is_normed = method == CV_TM_CCORR_NORMED ||
                    method == CV_TM_SQDIFF_NORMED ||
                    method == CV_TM_CCOEFF_NORMED;

    CV_CALL( img = cvGetMat( img, &stub, &coi1 ));
    CV_CALL( templ = cvGetMat( templ, &tstub, &coi2 ));
    CV_CALL( result = cvGetMat( result, &rstub ));

    if( CV_MAT_DEPTH( img->type ) != CV_8U &&
        CV_MAT_DEPTH( img->type ) != CV_32F )
        CV_ERROR( CV_StsUnsupportedFormat,
        "The function supports only 8u and 32f data types" );

    if( !CV_ARE_TYPES_EQ( img, templ ))
        CV_ERROR( CV_StsUnmatchedSizes, "image and template should have the same type" );

    if( CV_MAT_TYPE( result->type ) != CV_32FC1 )
        CV_ERROR( CV_StsUnsupportedFormat, "output image should have 32f type" );

    if( result->rows != img->rows - templ->rows + 1 ||
        result->cols != img->cols - templ->cols + 1 )
        CV_ERROR( CV_StsUnmatchedSizes, "output image should be (W - w + 1)x(H - h + 1)" );

    if( method < CV_TM_SQDIFF || method > CV_TM_CCOEFF_NORMED )
        CV_ERROR( CV_StsBadArg, "unknown comparison method" );

    cn = CV_MAT_CN(img->type);
    CV_CALL( icvCrossCorr( img, templ, result ));

    if( method == CV_TM_CCORR )
        EXIT;

    inv_area = 1./((double)templ->rows * templ->cols);

    CV_CALL( sum = cvCreateMat( img->rows + 1, img->cols + 1,
                                CV_MAKETYPE( CV_64F, cn )));
    if( method == CV_TM_CCOEFF )
    {
        CV_CALL( cvIntegral( img, sum, 0, 0 ));
        CV_CALL( templ_mean = cvAvg( templ ));
        q0 = q1 = q2 = q3 = 0;
    }
    else
    {
        CvScalar _templ_sdv = cvScalarAll(0);
        CV_CALL( sqsum = cvCreateMat( img->rows + 1, img->cols + 1,
                                      CV_MAKETYPE( CV_64F, cn )));
        CV_CALL( cvIntegral( img, sum, sqsum, 0 ));
        CV_CALL( cvAvgSdv( templ, &templ_mean, &_templ_sdv ));

        templ_norm = CV_SQR(_templ_sdv.val[0]) + CV_SQR(_templ_sdv.val[1]) +
                    CV_SQR(_templ_sdv.val[2]) + CV_SQR(_templ_sdv.val[3]);
        
        templ_sum2 = templ_norm +
                     CV_SQR(templ_mean.val[0]) + CV_SQR(templ_mean.val[1]) +
                     CV_SQR(templ_mean.val[2]) + CV_SQR(templ_mean.val[3]);

        if( num_type != 1 )
        {
            templ_mean = cvScalarAll(0);
            templ_norm = templ_sum2;
        }
        
        templ_sum2 /= inv_area;
        templ_norm = sqrt(templ_norm);
        templ_norm /= sqrt(inv_area); // care of accuracy here

        q0 = (double*)sqsum->data.ptr;
        q1 = q0 + templ->cols*cn;
        q2 = (double*)(sqsum->data.ptr + templ->rows*sqsum->step);
        q3 = q2 + templ->cols*cn;
    }

    p0 = (double*)sum->data.ptr;
    p1 = p0 + templ->cols*cn;
    p2 = (double*)(sum->data.ptr + templ->rows*sum->step);
    p3 = p2 + templ->cols*cn;

    sum_step = sum ? sum->step / sizeof(double) : 0;
    sqsum_step = sqsum ? sqsum->step / sizeof(double) : 0;

    for( i = 0; i < result->rows; i++ )
    {
        float* rrow = (float*)(result->data.ptr + i*result->step);
        idx = i * sum_step;
        idx2 = i * sqsum_step;

        for( j = 0; j < result->cols; j++, idx += cn, idx2 += cn )
        {
            double num = rrow[j], t;
            double wnd_mean2 = 0, wnd_sum2 = 0;
            
            if( num_type == 1 )
            {
                for( k = 0; k < cn; k++ )
                {
                    t = p0[idx+k] - p1[idx+k] - p2[idx+k] + p3[idx+k];
                    wnd_mean2 += CV_SQR(t);
                    num -= t*templ_mean.val[k];
                }

                wnd_mean2 *= inv_area;
            }

            if( is_normed || num_type == 2 )
            {
                for( k = 0; k < cn; k++ )
                {
                    t = q0[idx2+k] - q1[idx2+k] - q2[idx2+k] + q3[idx2+k];
                    wnd_sum2 += t;
                }

                if( num_type == 2 )
                    num = wnd_sum2 - 2*num + templ_sum2;
            }

            if( is_normed )
            {
                t = sqrt((wnd_sum2 - wnd_mean2))*templ_norm;
                if( t > 1e-3 )
                {
                    num /= t;
                    if( fabs(num) > 1. )
                        num = num > 0 ? 1 : -1;
                }
                else
                    num = num_type != 2;
            }

            rrow[j] = (float)num;
        }
    }
        
    __END__;

    cvReleaseMat( &sum );
    cvReleaseMat( &sqsum );
}

/* End of file. */
