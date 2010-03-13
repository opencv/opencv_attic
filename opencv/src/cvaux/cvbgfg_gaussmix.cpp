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

#include "_cvaux.h"
#include <float.h>

// to make sure we can use these short names
#undef K
#undef L
#undef T

// This is based on the "An Improved Adaptive Background Mixture Model for
// Real-time Tracking with Shadow Detection" by P. KaewTraKulPong and R. Bowden
// http://personal.ee.surrey.ac.uk/Personal/R.Bowden/publications/avbs01/avbs01.pdf
//
// The windowing method is used, but not the shadow detection. I make some of my
// own modifications which make more sense. There are some errors in some of their
// equations.
//

namespace cv
{
    
BackgroundSubtractor::~BackgroundSubtractor() {}
void BackgroundSubtractor::operator()(const Mat&, Mat&, double)
{
}

static const int defaultNMixtures = CV_BGFG_MOG_NGAUSSIANS;
static const int defaultHistory = CV_BGFG_MOG_WINDOW_SIZE;
static const double defaultBackgroundRatio = CV_BGFG_MOG_BACKGROUND_THRESHOLD;
static const double defaultVarThreshold = CV_BGFG_MOG_STD_THRESHOLD*CV_BGFG_MOG_STD_THRESHOLD;
    
BackgroundSubtractorMOG::BackgroundSubtractorMOG()
{
    frameSize = Size(0,0);
    frameType = 0;
    
    nframes = 0;
    nmixtures = defaultNMixtures;
    history = defaultHistory;
    varThreshold = defaultVarThreshold;
    backgroundRatio = defaultBackgroundRatio;
}
    
BackgroundSubtractorMOG::BackgroundSubtractorMOG(int _history, int _nmixtures, double _backgroundRatio)
{
    frameSize = Size(0,0);
    frameType = 0;
    
    nframes = 0;
    nmixtures = min(_nmixtures > 0 ? _nmixtures : defaultNMixtures, 8);
    history = _history > 0 ? _history : defaultHistory;
    varThreshold = defaultVarThreshold;
    backgroundRatio = min(_backgroundRatio > 0 ? _backgroundRatio : 0.95, 1.);
}
    
BackgroundSubtractorMOG::~BackgroundSubtractorMOG()
{
}


void BackgroundSubtractorMOG::initialize(Size _frameSize, int _frameType)
{
    frameSize = _frameSize;
    frameType = _frameType;
    nframes = 0;
    
    int nchannels = CV_MAT_CN(frameType);
    CV_Assert( CV_MAT_DEPTH(frameType) == CV_8U );
    
    // for each gaussian mixture of each pixel bg model we store ...
    // the mixture sort key (w/sum_of_variances), the mixture weight (w),
    // the mean (nchannels values) and
    // the diagonal covariance matrix (another nchannels values)
    bgmodel.create( frameSize.height, frameSize.width*nmixtures*(2 + 2*nchannels), CV_32F );
    const float w0 = (float)CV_BGFG_MOG_WEIGHT_INIT;
    const float var0 = (float)(CV_BGFG_MOG_SIGMA_INIT*CV_BGFG_MOG_SIGMA_INIT);
    const float sk0 = (float)(CV_BGFG_MOG_WEIGHT_INIT/(CV_BGFG_MOG_SIGMA_INIT*sqrt((double)nchannels)));
    
    for( int y = 0; y < frameSize.height; y++ )
    {
        float* mptr = bgmodel.ptr<float>(y);
        for( int x = 0; x < frameSize.width; x++ )
        {
            for( int k = 0; k < nmixtures; k++ )
            {
                mptr[0] = sk0;
                mptr[1] = w0;
                mptr += 2;
                for( int c = 0; c < nchannels; c++ )
                {
                    mptr[c] = 0;
                    mptr[c + nchannels] = var0;
                }
                mptr += nchannels*2;
            }
        }
    }
}

    
template<typename VT> struct MixData
{
    float sortKey;
    float weight;
    VT mean;
    VT var;
};

    
static void process8uC1( BackgroundSubtractorMOG& obj, const Mat& image, Mat& fgmask, double learningRate )
{
    int x, y, k, k1, rows = image.rows, cols = image.cols;
    float alpha = (float)learningRate, T = (float)obj.backgroundRatio, vT = (float)obj.varThreshold;
    int K = obj.nmixtures;
    
    const float w0 = (float)CV_BGFG_MOG_WEIGHT_INIT;
    const float sk0 = (float)(CV_BGFG_MOG_WEIGHT_INIT/CV_BGFG_MOG_SIGMA_INIT);
    const float var0 = (float)(CV_BGFG_MOG_SIGMA_INIT*CV_BGFG_MOG_SIGMA_INIT);
    
    for( y = 0; y < rows; y++ )
    {
        const uchar* src = image.ptr<uchar>(y);
        uchar* dst = fgmask.ptr<uchar>(y);
        MixData<float>* mptr = (MixData<float>*)obj.bgmodel.ptr(y);
        
        for( x = 0; x < cols; x++, mptr += K )
        {
            float wsum = 0, dw = 0;
            float pix = src[x];
            for( k = 0; k < K; k++ )
            {
                float w = mptr[k].weight;
                float mu = mptr[k].mean;
                float var = mptr[k].var;
                float diff = pix - mu, d2 = diff*diff;
                if( d2 < vT*var )
                {
                    dw = alpha*(1.f - w);
                    mptr[k].weight = w + dw;
                    mptr[k].mean = mu + alpha*diff;
                    mptr[k].var = var = max(var + alpha*(d2 - var), FLT_EPSILON);
                    mptr[k].sortKey = w/sqrt(var);
                    
                    for( k1 = k-1; k1 >= 0; k1-- )
                    {
                        if( mptr[k1].sortKey > mptr[k1+1].sortKey )
                            break;
                        std::swap( mptr[k1], mptr[k1+1] );
                    }
                    break;
                }
                wsum += w;
            }
            
            dst[x] = (uchar)(-(wsum >= T));
            wsum += dw;
            
            if( k == K ) // no appropriate gaussian mixture found at all, remove the weakest mixture and create a new one
            {
                wsum += w0 - mptr[K-1].weight;
                mptr[K-1].weight = w0;
                mptr[K-1].mean = pix;
                mptr[K-1].var = var0;
                mptr[K-1].sortKey = sk0;
            }
            else
                for( ; k < K; k++ )
                    wsum += mptr[k].weight;
            
            dw = 1.f/wsum;
            for( k = 0; k < K; k++ )
            {
                mptr[k].weight *= dw;
                mptr[k].sortKey *= dw;
            }
        }
    }
}

static void process8uC3( BackgroundSubtractorMOG& obj, const Mat& image, Mat& fgmask, double learningRate )
{
    int x, y, k, k1, rows = image.rows, cols = image.cols;
    float alpha = (float)learningRate, T = (float)obj.backgroundRatio, vT = (float)obj.varThreshold;
    int K = obj.nmixtures;
    
    const float w0 = (float)CV_BGFG_MOG_WEIGHT_INIT;
    const float sk0 = (float)(CV_BGFG_MOG_WEIGHT_INIT/CV_BGFG_MOG_SIGMA_INIT);
    const float var0 = (float)(CV_BGFG_MOG_SIGMA_INIT*CV_BGFG_MOG_SIGMA_INIT);
    
    for( y = 0; y < rows; y++ )
    {
        const uchar* src = image.ptr<uchar>(y);
        uchar* dst = fgmask.ptr<uchar>(y);
        MixData<Vec3f>* mptr = (MixData<Vec3f>*)obj.bgmodel.ptr(y);
        
        for( x = 0; x < cols; x++, mptr += K )
        {
            float wsum = 0, dw = 0;
            Vec3f pix(src[x*3], src[x*3+1], src[x*3+2]);
            for( k = 0; k < K; k++ )
            {
                float w = mptr[k].weight;
                Vec3f mu = mptr[k].mean[0];
                Vec3f var = mptr[k].var[0];
                Vec3f diff = pix - mu;
                float d2 = diff.dot(diff);
                if( d2 < vT*(var[0] + var[1] + var[2]) )
                {
                    dw = alpha*(1.f - w);
                    mptr[k].weight = w + dw;
                    mptr[k].mean = mu + alpha*diff;
                    var = Vec3f(max(var[0] + alpha*(diff[0]*diff[0] - var[0]), FLT_EPSILON),
                                max(var[1] + alpha*(diff[1]*diff[1] - var[1]), FLT_EPSILON),
                                max(var[2] + alpha*(diff[2]*diff[2] - var[2]), FLT_EPSILON));
                    mptr[k].var = var;
                    mptr[k].sortKey = w/sqrt(var[0] + var[1] + var[2]);
                    
                    for( k1 = k-1; k1 >= 0; k1-- )
                    {
                        if( mptr[k1].sortKey > mptr[k1+1].sortKey )
                            break;
                        std::swap( mptr[k1], mptr[k1+1] );
                    }
                    break;
                }
                wsum += w;
            }
            
            dst[x] = (uchar)(-(wsum >= T));
            wsum += dw;
            
            if( k == K ) // no appropriate gaussian mixture found at all, remove the weakest mixture and create a new one
            {
                wsum += w0 - mptr[K-1].weight;
                mptr[K-1].weight = w0;
                mptr[K-1].mean = pix;
                mptr[K-1].var = Vec3f(var0, var0, var0);
                mptr[K-1].sortKey = sk0;
            }
            else
                for( ; k < K; k++ )
                    wsum += mptr[k].weight;
            
            dw = 1.f/wsum;
            for( k = 0; k < K; k++ )
            {
                mptr[k].weight *= dw;
                mptr[k].sortKey *= dw;
            }
        }
    }
}

void BackgroundSubtractorMOG::operator()(const Mat& image, Mat& fgmask, double learningRate)
{
    bool needToInitialize = nframes == 0 || learningRate >= 1 || image.size() != frameSize || image.type() != frameType;
    
    if( needToInitialize )
        initialize(frameSize, frameType);
    
    CV_Assert( image.depth() == CV_8U );
    fgmask.create( image.size(), CV_8U );
    
    ++nframes;
    learningRate = learningRate >= 0 && nframes > 1 ? learningRate : 1./min( nframes, history );
    
    if( image.type() == CV_8UC1 )
        process8uC1( *this, image, fgmask, learningRate );
    else if( image.type() == CV_8UC3 )
        process8uC3( *this, image, fgmask, learningRate );
    else
        CV_Error( CV_StsUnsupportedFormat, "Only 1- and 3-channel 8-bit images are supported in BackgroundSubtractorMOG" );
}
    
}


static void CV_CDECL
icvReleaseGaussianBGModel( CvGaussBGModel** bg_model )
{
    if( !bg_model )
        CV_Error( CV_StsNullPtr, "" );
    
    if( *bg_model )
    {
        delete (cv::Mat*)((*bg_model)->g_point);
        cvReleaseImage( &(*bg_model)->background );
        cvReleaseImage( &(*bg_model)->foreground );
        cvReleaseMemStorage(&(*bg_model)->storage);
        memset( *bg_model, 0, sizeof(**bg_model) );
        delete *bg_model;
        *bg_model = 0;
    }
}


static int CV_CDECL
icvUpdateGaussianBGModel( IplImage* curr_frame, CvGaussBGModel*  bg_model, double learningRate )
{
    int region_count = 0;
    CvSeq *first_seq = NULL, *prev_seq = NULL, *seq = NULL;
    
    cv::Mat image = cv::cvarrToMat(curr_frame), mask = cv::cvarrToMat(bg_model->foreground);
    
    cv::BackgroundSubtractorMOG mog;
    mog.bgmodel = *(cv::Mat*)bg_model->g_point;
    mog.frameSize = mog.bgmodel.data ? cv::Size(cvGetSize(curr_frame)) : cv::Size();
    mog.frameType = image.type();

    mog.nframes = bg_model->countFrames;
    mog.history = bg_model->params.win_size;
    mog.nmixtures = bg_model->params.n_gauss;
    mog.varThreshold = bg_model->params.std_threshold;
    mog.backgroundRatio = bg_model->params.bg_threshold;
    
    mog(image, mask, learningRate);
    
    bg_model->countFrames = mog.nframes;
    if( ((cv::Mat*)bg_model->g_point)->data != mog.bgmodel.data )
        *((cv::Mat*)bg_model->g_point) = mog.bgmodel;
    
    //foreground filtering
    
    //filter small regions
    cvClearMemStorage(bg_model->storage);
    
    //cvMorphologyEx( bg_model->foreground, bg_model->foreground, 0, 0, CV_MOP_OPEN, 1 );
    //cvMorphologyEx( bg_model->foreground, bg_model->foreground, 0, 0, CV_MOP_CLOSE, 1 );
    
    cvFindContours( bg_model->foreground, bg_model->storage, &first_seq, sizeof(CvContour), CV_RETR_LIST );
    for( seq = first_seq; seq; seq = seq->h_next )
    {
        CvContour* cnt = (CvContour*)seq;
        if( cnt->rect.width * cnt->rect.height < bg_model->params.minArea )
        {
            //delete small contour
            prev_seq = seq->h_prev;
            if( prev_seq )
            {
                prev_seq->h_next = seq->h_next;
                if( seq->h_next ) seq->h_next->h_prev = prev_seq;
            }
            else
            {
                first_seq = seq->h_next;
                if( seq->h_next ) seq->h_next->h_prev = NULL;
            }
        }
        else
        {
            region_count++;
        }
    }
    bg_model->foreground_regions = first_seq;
    cvZero(bg_model->foreground);
    cvDrawContours(bg_model->foreground, first_seq, CV_RGB(0, 0, 255), CV_RGB(0, 0, 255), 10, -1);
    
    return region_count;
}

CV_IMPL CvBGStatModel*
cvCreateGaussianBGModel( IplImage* first_frame, CvGaussBGStatModelParams* parameters )
{
    CvGaussBGStatModelParams params;
    
    CV_Assert( CV_IS_IMAGE(first_frame) );
    
    //init parameters
    if( parameters == NULL )
    {                        /* These constants are defined in cvaux/include/cvaux.h: */
        params.win_size      = CV_BGFG_MOG_WINDOW_SIZE;
        params.bg_threshold  = CV_BGFG_MOG_BACKGROUND_THRESHOLD;

        params.std_threshold = CV_BGFG_MOG_STD_THRESHOLD;
        params.weight_init   = CV_BGFG_MOG_WEIGHT_INIT;

        params.variance_init = CV_BGFG_MOG_SIGMA_INIT*CV_BGFG_MOG_SIGMA_INIT;
        params.minArea       = CV_BGFG_MOG_MINAREA;
        params.n_gauss       = CV_BGFG_MOG_NGAUSSIANS;
    }
    else
        params = *parameters;
    
    CvGaussBGModel* bg_model = new CvGaussBGModel;
    memset( bg_model, 0, sizeof(*bg_model) );
    bg_model->type = CV_BG_MODEL_MOG;
    bg_model->release = (CvReleaseBGStatModel)icvReleaseGaussianBGModel;
    bg_model->update = (CvUpdateBGStatModel)icvUpdateGaussianBGModel;
    
    bg_model->params = params;
    
    //prepare storages
    bg_model->g_point = (CvGaussBGPoint*)new cv::Mat();
    
    bg_model->background = cvCreateImage(cvSize(first_frame->width,
        first_frame->height), IPL_DEPTH_8U, first_frame->nChannels);
    bg_model->foreground = cvCreateImage(cvSize(first_frame->width,
        first_frame->height), IPL_DEPTH_8U, 1);
    
    bg_model->storage = cvCreateMemStorage();
    
    bg_model->countFrames = 0;
    
    icvUpdateGaussianBGModel( first_frame, bg_model, 1 );
    
    return (CvBGStatModel*)bg_model;
}

/* End of file. */
