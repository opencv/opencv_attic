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

static void
icvCrossCorr( const CvArr* _img, const CvArr* _templ, CvArr* _corr )
{
    const double block_scale = 4.5;
    const int min_block_size = 256;
    CvMat* dft_img = 0;
    CvMat* dft_templ = 0;
    CvMat* temp = 0;
    
    CV_FUNCNAME( "icvCrossCorr" );
    
    __BEGIN__;

    CvMat istub, *img = (CvMat*)_img;
    CvMat tstub, *templ = (CvMat*)_templ;
    CvMat cstub, *corr = (CvMat*)_corr;
    CvMat sstub, pstub, dstub, *src, *dst;
    CvSize imgsize, templsize, corrsize, dftsize, blocksize;
    CvMat* planes[] = { 0, 0, 0, 0 };
    int x, y, i, depth, cn, corr_type, yofs;
    int reuse_templ_spectrum = 0;

    CV_CALL( img = cvGetMat( img, &istub ));
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

    blocksize.width = cvRound(templsize.width*block_scale);
    blocksize.width = MAX( blocksize.width, min_block_size - templsize.width + 1 );
    blocksize.width = MIN( blocksize.width, corrsize.width );
    blocksize.height = cvRound(templsize.height*block_scale);
    blocksize.height = MAX( blocksize.height, min_block_size - templsize.height + 1 );
    blocksize.height = MIN( blocksize.height, corrsize.height );

    dftsize.width = cvGetOptimalDFTSize(blocksize.width + templsize.width - 1);
    if( dftsize.width == 1 )
        dftsize.width = 2;
    dftsize.height = cvGetOptimalDFTSize(blocksize.height + templsize.height - 1);
    if( dftsize.width <= 0 || dftsize.height <= 0 )
        CV_ERROR( CV_StsOutOfRange, "the input arrays are too big" );

    // recompute block size
    blocksize.width = dftsize.width - templsize.width + 1;
    blocksize.width = MIN( blocksize.width, corrsize.width );
    blocksize.height = dftsize.height - templsize.height + 1;
    blocksize.height = MIN( blocksize.height, corrsize.height );

    reuse_templ_spectrum = blocksize.width < corrsize.width ||
                           blocksize.height < corrsize.height; 

    CV_CALL( dft_img = cvCreateMat( dftsize.height, dftsize.width, corr_type ));
    CV_CALL( dft_templ = cvCreateMat( dftsize.height*
        (reuse_templ_spectrum ? cn : 1), dftsize.width, corr_type ));

    if( cn > 1 && depth != CV_MAT_DEPTH(corr_type) )
        CV_CALL( temp = cvCreateMat( blocksize.height + templsize.height - 1,
            blocksize.width + templsize.width - 1, depth ));

    if( reuse_templ_spectrum )
    {
        // compute DFT of each template plane
        for( i = 0; i < cn; i++ )
        {
            yofs = i*dftsize.height;

            src = templ;
            dst = cvGetSubRect( dft_templ, &dstub, cvRect(0,yofs,templ->cols,templ->rows));
        
            if( cn > 1 )
            {
                planes[i] = !temp ? dst :
                    cvGetSubRect( temp, &pstub, cvRect(0,0,templ->cols,templ->rows) );
                cvSplit( templ, planes[0], planes[1], planes[2], planes[3] );
                src = planes[i];
                planes[i] = 0;
            }

            if( dst != src )
            {
                if( CV_ARE_TYPES_EQ(src, dst) )
                    cvCopy( src, dst );
                else
                    cvConvert( src, dst );
            }

            cvGetSubRect( dft_templ, dst, cvRect(templ->cols, yofs,
                          dft_templ->cols - templ->cols, templ->rows) );
            cvZero( dst );
            cvGetSubRect( dft_templ, dst, cvRect(0,yofs,dftsize.width,dftsize.height) );
            cvDFT( dst, dst, CV_DXT_FORWARD, templ->rows );
        }
    }

    // calculate correlation by blocks
    for( y = 0; y < corrsize.height; y += blocksize.height )
    {
        for( x = 0; x < corrsize.width; x += blocksize.width )
        {
            CvSize csz = { blocksize.width, blocksize.height }, isz;
            csz.width = MIN( csz.width, corr->cols - x );
            csz.height = MIN( csz.height, corr->rows - y );
            isz.width = csz.width + templ->cols - 1;
            isz.height = csz.height + templ->rows - 1;

            for( i = 0; i < cn; i++ )
            {
                yofs = i*dftsize.height;

                src = cvGetSubRect( img, &sstub, cvRect(x,y,isz.width,isz.height) );
                dst = cvGetSubRect( dft_img, &dstub, cvRect(0,0,isz.width,isz.height));

                if( cn > 1 )
                {
                    planes[i] = !temp ? dst :
                        cvGetSubRect( temp, &pstub, cvRect(0,0,isz.width,isz.height));
                    cvSplit( src, planes[0], planes[1], planes[2], planes[3] );
                    src = planes[i];
                    planes[i] = 0;
                }

                if( dst != src )
                {
                    if( CV_ARE_TYPES_EQ(src, dst) )
                        cvCopy( src, dst );
                    else
                        cvConvert( src, dst );
                }

                cvGetSubRect( dft_img, dst, cvRect(isz.width, 0,
                      dftsize.width - isz.width,dftsize.height) );
                cvZero( dst );

                cvDFT( dft_img, dft_img, CV_DXT_FORWARD, isz.height );

                if( reuse_templ_spectrum )
                    cvGetSubRect( dft_templ, dst, cvRect(0,yofs,dftsize.width,dftsize.height) );
                else
                {
                    src = templ;
                    dst = cvGetSubRect( dft_templ, &dstub, cvRect(0,0,templ->cols,templ->rows));
        
                    if( cn > 1 )
                    {
                        planes[i] = !temp ? dst :
                            cvGetSubRect( temp, &pstub, cvRect(0,0,templ->cols,templ->rows) );
                        cvSplit( src, planes[0], planes[1], planes[2], planes[3] );
                        src = planes[i];
                        planes[i] = 0;
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
                    cvGetSubRect( dft_templ, dst, cvRect(0,0,dftsize.width,dftsize.height) );
                    cvDFT( dst, dst, CV_DXT_FORWARD, templ->rows );
                }

                cvMulSpectrums( dft_img, dst, dft_img, CV_DXT_MUL_CONJ );
                cvDFT( dft_img, dft_img, CV_DXT_INV_SCALE, csz.height );

                src = cvGetSubRect( dft_img, &sstub, cvRect(0,0,csz.width,csz.height) );
                dst = cvGetSubRect( corr, &dstub, cvRect(x,y,csz.width,csz.height) );

                if( i == 0 )
                    cvCopy( src, dst );
                else
                    cvAdd( src, dst, dst );
            }
        }
    }

    __END__;

    cvReleaseMat( &dft_img );
    cvReleaseMat( &dft_templ );
    cvReleaseMat( &temp );
}

/***************************** IPP Match Template Functions ******************************/

icvCrossCorrValid_Norm_8u32f_C1R_t  icvCrossCorrValid_Norm_8u32f_C1R_p = 0;
icvCrossCorrValid_NormLevel_8u32f_C1R_t  icvCrossCorrValid_NormLevel_8u32f_C1R_p = 0;
icvSqrDistanceValid_Norm_8u32f_C1R_t  icvSqrDistanceValid_Norm_8u32f_C1R_p = 0;
icvCrossCorrValid_Norm_32f_C1R_t  icvCrossCorrValid_Norm_32f_C1R_p = 0;
icvCrossCorrValid_NormLevel_32f_C1R_t  icvCrossCorrValid_NormLevel_32f_C1R_p = 0;
icvSqrDistanceValid_Norm_32f_C1R_t  icvSqrDistanceValid_Norm_32f_C1R_p = 0;

typedef CvStatus (CV_STDCALL * CvTemplMatchIPPFunc)
    ( const void* img, int imgstep, CvSize imgsize,
      const void* templ, int templstep, CvSize templsize,
      void* result, int rstep );

/*****************************************************************************************/

CV_IMPL void
cvMatchTemplate( const CvArr* _img, const CvArr* _templ, CvArr* _result, int method )
{
    CvMat* sum = 0;
    CvMat* sqsum = 0;
    
    CV_FUNCNAME( "cvMatchTemplate" );

    __BEGIN__;

    int coi1 = 0, coi2 = 0;
    int depth, cn;
    int i, j, k;
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

    depth = CV_MAT_DEPTH(img->type);
    cn = CV_MAT_CN(img->type);

    if( is_normed && cn == 1 && templ->rows > 8 && templ->cols > 8 &&
        img->rows > templ->cols && img->cols > templ->cols )
    {
        CvTemplMatchIPPFunc ipp_func =
            depth == CV_8U ?
            (method == CV_TM_SQDIFF_NORMED ? (CvTemplMatchIPPFunc)icvSqrDistanceValid_Norm_8u32f_C1R_p :
            method == CV_TM_CCORR_NORMED ? (CvTemplMatchIPPFunc)icvCrossCorrValid_Norm_8u32f_C1R_p :
            (CvTemplMatchIPPFunc)icvCrossCorrValid_NormLevel_8u32f_C1R_p) :
            (method == CV_TM_SQDIFF_NORMED ? (CvTemplMatchIPPFunc)icvSqrDistanceValid_Norm_32f_C1R_p :
            method == CV_TM_CCORR_NORMED ? (CvTemplMatchIPPFunc)icvCrossCorrValid_Norm_32f_C1R_p :
            (CvTemplMatchIPPFunc)icvCrossCorrValid_NormLevel_32f_C1R_p);

        if( ipp_func )
        {
            CvSize img_size = cvGetMatSize(img), templ_size = cvGetMatSize(templ);

            IPPI_CALL( ipp_func( img->data.ptr, img->step ? img->step : CV_STUB_STEP,
                                 img_size, templ->data.ptr,
                                 templ->step ? templ->step : CV_STUB_STEP,
                                 templ_size, result->data.ptr,
                                 result->step ? result->step : CV_STUB_STEP ));
            for( i = 0; i < result->rows; i++ )
            {
                float* rrow = (float*)(result->data.ptr + i*result->step);
                for( j = 0; j < result->cols; j++ )
                {
                    if( fabs(rrow[j]) > 1. )
                        rrow[j] = rrow[j] < 0 ? -1.f : 1.f;
                }
            }
            EXIT;
        }
    }

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
                t = sqrt(wnd_sum2 - wnd_mean2)*templ_norm;
                if( fabs(t) < DBL_EPSILON )
                    num = num_type == 2 ? 1 : -1;
                else
                {
                    num /= t;
                    if( fabs(num) > 1. )
                        num = num > 0 ? 1 : -1;
                }
            }

            rrow[j] = (float)num;
        }
    }
        
    __END__;

    cvReleaseMat( &sum );
    cvReleaseMat( &sqsum );
}

/* End of file. */
