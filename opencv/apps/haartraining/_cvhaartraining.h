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

/*
 * _cvhaartraining.h
 *
 * training of cascade of boosted classifiers based on haar features
 */

#ifndef __CVHAARTRAINING_H_
#define __CVHAARTRAINING_H_

#include <_cvcommon.h>
#include <cvclassifier.h>
#include <string.h>
#include <stdio.h>

#define CV_IMPL CV_EXTERN_C


/* parameters for tree cascade classifier training */

/* max number of clusters */
#define CV_MAX_CLUSTERS 3

/* term criteria for K-Means */
#define CV_TERM_CRITERIA() cvTermCriteria( CV_TERMCRIT_EPS, 1000, 1E-5 )

/* If CV_COL_ARRANGEMENT is defined then sample feature values are stored in column.
   Otherwise they are stored in row */
#define CV_COL_ARRANGEMENT

/* print statistic info */
#define CV_VERBOSE 1

#define CV_STAGE_CART_FILE_NAME "AdaBoostCARTHaarClassifier.txt"

#define CV_HAAR_FEATURE_MAX      3
#define CV_HAAR_FEATURE_DESC_MAX 20

typedef int    sum_type;
typedef double sqsum_type;
typedef short  idx_type;

#define CV_SUM_MAT_TYPE CV_32SC1
#define CV_SQSUM_MAT_TYPE CV_64FC1
#define CV_IDX_MAT_TYPE CV_16SC1

#define CV_STUMP_TRAIN_PORTION 100

#define CV_THRESHOLD_EPS (0.00001F)

typedef struct CvTHaarFeature
{
    char desc[CV_HAAR_FEATURE_DESC_MAX];
    int  tilted;
    struct
    {
        CvRect r;
        float weight;
    } rect[CV_HAAR_FEATURE_MAX];
} CvTHaarFeature;

typedef struct CvFastHaarFeature
{
    int tilted;
    struct
    {
        int p0, p1, p2, p3;
        float weight;
    } rect[CV_HAAR_FEATURE_MAX];
} CvFastHaarFeature;

typedef struct CvIntHaarFeatures
{
    CvSize winsize;
    int count;
    CvTHaarFeature* feature;
    CvFastHaarFeature* fastfeature;
} CvIntHaarFeatures;

CV_INLINE CvTHaarFeature cvHaarFeature( const char* desc,
                            int x0, int y0, int w0, int h0, float wt0,
                            int x1, int y1, int w1, int h1, float wt1,
                            int x2 CV_DEFAULT( 0 ), int y2 CV_DEFAULT( 0 ),
                            int w2 CV_DEFAULT( 0 ), int h2 CV_DEFAULT( 0 ),
                            float wt2 CV_DEFAULT( 0.0F ) );

CV_INLINE CvTHaarFeature cvHaarFeature( const char* desc,
                            int x0, int y0, int w0, int h0, float wt0,
                            int x1, int y1, int w1, int h1, float wt1,
                            int x2, int y2, int w2, int h2, float wt2 )
{
    CvTHaarFeature hf;

    assert( CV_HAAR_FEATURE_MAX >= 3 );
    assert( strlen( desc ) < CV_HAAR_FEATURE_DESC_MAX );

    strcpy( &(hf.desc[0]), desc );
    hf.tilted = ( hf.desc[0] == 't' );
    
    hf.rect[0].r.x = x0;
    hf.rect[0].r.y = y0;
    hf.rect[0].r.width  = w0;
    hf.rect[0].r.height = h0;
    hf.rect[0].weight   = wt0;
    
    hf.rect[1].r.x = x1;
    hf.rect[1].r.y = y1;
    hf.rect[1].r.width  = w1;
    hf.rect[1].r.height = h1;
    hf.rect[1].weight   = wt1;
    
    hf.rect[2].r.x = x2;
    hf.rect[2].r.y = y2;
    hf.rect[2].r.width  = w2;
    hf.rect[2].r.height = h2;
    hf.rect[2].weight   = wt2;
    
    return hf;
}

/* Prepared for training samples */
typedef struct CvHaarTrainingData
{
    CvSize winsize;     /* training image size */
    int    maxnum;      /* maximum number of samples */
    CvMat  sum;         /* sum images (each row represents image) */
    CvMat  tilted;      /* tilted sum images (each row represents image) */
    CvMat  normfactor;  /* normalization factor */
    CvMat  cls;         /* classes. 1.0 - object, 0.0 - background */
    CvMat  weights;     /* weights */

    CvMat* valcache;    /* precalculated feature values (CV_32FC1) */
    CvMat* idxcache;    /* presorted indices (CV_IDX_MAT_TYPE) */
} CvHaarTrainigData;


/* Passed to callback functions */
typedef struct CvUserdata
{
    CvHaarTrainingData* trainingData;
    CvIntHaarFeatures* haarFeatures;
} CvUserdata;

CV_INLINE
CvUserdata cvUserdata( CvHaarTrainingData* trainingData,
                       CvIntHaarFeatures* haarFeatures );

CV_INLINE
CvUserdata cvUserdata( CvHaarTrainingData* trainingData,
                       CvIntHaarFeatures* haarFeatures )
{
    CvUserdata userdata;

    userdata.trainingData = trainingData;
    userdata.haarFeatures = haarFeatures;

    return userdata;
}


#define CV_INT_HAAR_CLASSIFIER_FIELDS()                                 \
    float (*eval)( CvIntHaarClassifier*, sum_type*, sum_type*, float ); \
    void  (*save)( CvIntHaarClassifier*, FILE* file );                  \
    void  (*release)( CvIntHaarClassifier** );

/* internal weak classifier*/
typedef struct CvIntHaarClassifier
{
    CV_INT_HAAR_CLASSIFIER_FIELDS()
} CvIntHaarClassifier;

/*
 * CART classifier
 */
typedef struct CvCARTHaarClassifier
{
    CV_INT_HAAR_CLASSIFIER_FIELDS()

    int count;
    int* compidx;
    CvTHaarFeature* feature;
    CvFastHaarFeature* fastfeature;
    float* threshold;
    int* left;
    int* right;
    float* val;
} CvCARTHaarClassifier;

/* internal stage classifier */
typedef struct CvStageHaarClassifier
{
    CV_INT_HAAR_CLASSIFIER_FIELDS()
        
    int count;
    float threshold;
    CvIntHaarClassifier** classifier;
} CvStageHaarClassifier;

/* internal cascade classifier */
typedef struct CvCascadeHaarClassifier
{
    CV_INT_HAAR_CLASSIFIER_FIELDS()

    int count;
    CvIntHaarClassifier** classifier;
} CvCascadeHaarClassifier;


/* internal tree cascade classifier node */
typedef struct CvTreeCascadeNode
{
    CvStageHaarClassifier* stage;

    struct CvTreeCascadeNode* next;
    struct CvTreeCascadeNode* child;
    struct CvTreeCascadeNode* parent;

    struct CvTreeCascadeNode* next_same_level;
    struct CvTreeCascadeNode* child_eval;
    int idx;
    int leaf;
} CvTreeCascadeNode;

/* internal tree cascade classifier */
typedef struct CvTreeCascadeClassifier
{
    CV_INT_HAAR_CLASSIFIER_FIELDS()

    CvTreeCascadeNode* root;      /* root of the tree */
    CvTreeCascadeNode* root_eval; /* root node for the filtering */

    int next_idx;
} CvTreeCascadeClassifier;


CV_INLINE float cvEvalFastHaarFeature( CvFastHaarFeature* feature,
                                       sum_type* sum, sum_type* tilted );

CV_INLINE float cvEvalFastHaarFeature( CvFastHaarFeature* feature,
                                       sum_type* sum, sum_type* tilted )
{
    sum_type* img = NULL;
    int i = 0;
    float ret = 0.0F;
    
    assert( feature );
    
    img = ( feature->tilted ) ? tilted : sum;

    assert( img );

    for( i = 0; feature->rect[i].weight != 0.0F && i < CV_HAAR_FEATURE_MAX; i++ )
    {
        ret += feature->rect[i].weight *
            ( img[feature->rect[i].p0] - img[feature->rect[i].p1] -
              img[feature->rect[i].p2] + img[feature->rect[i].p3] );
    }

    return ret;
}

typedef struct CvBackgroundData
{
    int    count;
    char** filename;
    int    last;
    int    round;
    CvSize winsize;
} CvBackgroundData;

typedef struct CvBackgroundReader
{
    CvMat   src;
    CvMat   img;
    CvPoint offset;
    float   scale;
    float   scalefactor;
    float   stepfactor;
    CvPoint point;
} CvBackgroundReader;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * icvCreateIntHaarFeatures
 *
 * Create internal representation of haar features
 *
 * mode:
 *  0 - BASIC = Viola
 *  1 - CORE  = All upright
 *  2 - ALL   = All features
 */
CvIntHaarFeatures* icvCreateIntHaarFeatures( CvSize winsize,
                                             int mode CV_DEFAULT( 0 ),
                                             int symmetric CV_DEFAULT( 0 ) );

void icvReleaseIntHaarFeaturesCache( CvIntHaarFeatures** intHaarFeatures );

void icvReleaseIntHaarFeatures( CvIntHaarFeatures** intHaarFeatures );

/*
 * icvConvertToFastHaarFeature
 *
 * Convert to fast representation of haar features
 *
 * haarFeature     - input array
 * fastHaarFeature - output array
 * size            - size of arrays
 * step            - row step for the integral image
 */
void icvConvertToFastHaarFeature( CvTHaarFeature* haarFeature,
                                  CvFastHaarFeature* fastHaarFeature,
                                  int size, int step );


/*
 * icvCreateHaarTrainingData
 *
 * Create haar training data used in stage training
 */
CvHaarTrainigData* icvCreateHaarTrainingData( CvSize winsize, int maxnumsamples );

void icvReleaseHaarTrainingData( CvHaarTrainigData** haarTrainingData );

/*
 * icvCreateCARTStageClassifier
 *
 * Create stage classifier with trees as weak classifiers
 * data             - haar training data. It must be created and filled before call
 * minhitrate       - desired min hit rate
 * maxfalsealarm    - desired max false alarm rate
 * symmetric        - if not 0 it is assumed that samples are vertically symmetric
 * numprecalculated - number of features that will be precalculated. Each precalculated
 *   feature need (number_of_samples*(sizeof( float ) + sizeof( short ))) bytes of memory
 * weightfraction   - weight trimming parameter
 * numsplits        - number of binary splits in each tree
 * boosttype        - type of applied boosting algorithm
 * stumperror       - type of used error if Discrete AdaBoost algorithm is applied
 * maxsplits        - maximum total number of splits in all weak classifiers.
 *   If it is not 0 then NULL returned if total number of splits exceeds <maxsplits>.
 */
CvIntHaarClassifier* icvCreateCARTStageClassifier( CvHaarTrainingData* data,
                                                   CvMat* sampleIdx,
                                                   CvIntHaarFeatures* haarFeatures,
                                                   float minhitrate,
                                                   float maxfalsealarm,
                                                   int   symmetric,
                                                   float weightfraction,
                                                   int numsplits,
                                                   CvBoostType boosttype,
                                                   CvStumpError stumperror,
                                                   int maxsplits );

void icvGetTrainingDataCallback( CvMat* mat, CvMat* sampleIdx, CvMat*,
                                 int first, int num, void* userdata );

CvBackgroundData* icvCreateBackgroundData( const char* filename, CvSize winsize );

void icvReleaseBackgroundData( CvBackgroundData** data );


CvBackgroundReader* icvCreateBackgroundReader();

void icvReleaseBackgroundReader( CvBackgroundReader** reader );

void icvGetNextFromBackgroundData( CvBackgroundData* data,
                                   CvBackgroundReader* reader );


extern CvBackgroundReader* cvbgreader;
#ifdef _OPENMP
#pragma omp threadprivate(cvbgreader)
#endif /* _OPENMP */

extern CvBackgroundData* cvbgdata;

/*
 * icvInitBackgroundReaders
 *
 * Initialize background reading process.
 * <cvbgreader> and <cvbgdata> are initialized.
 * Must be called before any usage of background
 *
 * filename - name of background description file
 * winsize  - size of images will be obtained from background
 *
 * return 1 on success, 0 otherwise.
 */
int icvInitBackgroundReaders( const char* filename, CvSize winsize );

/*
 * icvDestroyBackgroundReaders
 *
 * Finish backgournd reading process
 */
void icvDestroyBackgroundReaders();

/*
 * icvGetBackgroundImage
 *
 * Get an image from background
 * <img> must be allocated and have size, previously passed to icvInitBackgroundReaders
 *
 * Usage example:
 * icvInitBackgroundReaders( "bg.txt", cvSize( 24, 24 ) );
 * ...
 * #pragma omp parallel
 * {
 *     ...
 *     icvGetBackgourndImage( cvbgdata, cvbgreader, img );
 *     ...
 * }
 * ...
 * icvDestroyBackgroundReaders();
 */
void icvGetBackgroundImage( CvBackgroundData* data,
                            CvBackgroundReader* reader,
                            CvMat* img );

/*
 * icvGetAuxImages
 *
 * Get sum, tilted, sqsum images and calculate normalization factor
 * All images must be allocated.
 */
void icvGetAuxImages( CvMat* img, CvMat* sum, CvMat* tilted,
                      CvMat* sqsum, float* normfactor );

typedef int (*CvGetHaarTrainingDataCallback)( CvMat* img, void* userdata );

/*
 * icvGetHaarTrainingData
 *
 * Fill <data> with samples, passed <cascade>
 */
int icvGetHaarTrainingData( CvHaarTrainingData* data, int first, int count,
                            CvIntHaarClassifier* cascade,
                            CvGetHaarTrainingDataCallback callback, void* userdata,
                            int* consumed );
/*
 * icvGetHaarTrainingDataFromBG
 *
 * Fill <data> with background samples, passed <cascade>
 * Background reading process must be initialized before call.
 */
int icvGetHaarTrainingDataFromBG( CvHaarTrainingData* data, int first, int count,
                                  CvIntHaarClassifier* cascade, int* consumed );

typedef struct CvVecFile
{
    FILE*  input;
    int    count;
    int    vecsize;
    int    last;
    short* vector;
} CvVecFile;

int icvGetHaarTraininDataFromVecCallback( CvMat* img, void* userdata );

/*
 * icvGetHaarTrainingDataFromVec
 *
 * Fill <data> with samples from .vec file, passed <cascade>
 */
int icvGetHaarTrainingDataFromVec( CvHaarTrainingData* data, int first, int count,                                   
                                   CvIntHaarClassifier* cascade,
                                   const char* filename,
                                   int* consumed );

CvIntHaarClassifier* icvCreateCARTHaarClassifier( int count );

void icvReleaseHaarClassifier( CvIntHaarClassifier** classifier );

void icvInitCARTHaarClassifier( CvCARTHaarClassifier* carthaar, CvCARTClassifier* cart,
                                CvIntHaarFeatures* intHaarFeatures );

float icvEvalCARTHaarClassifier( CvIntHaarClassifier* classifier,
                                 sum_type* sum, sum_type* tilted, float normfactor );

CvIntHaarClassifier* icvCreateStageHaarClassifier( int count, float threshold );

void icvReleaseStageHaarClassifier( CvIntHaarClassifier** classifier );

float icvEvalStageHaarClassifier( CvIntHaarClassifier* classifier,
                                  sum_type* sum, sum_type* tilted, float normfactor );

CvIntHaarClassifier* icvCreateCascadeHaarClassifier( int count );

void icvReleaseCascadeHaarClassifier( CvIntHaarClassifier** classifier );

float icvEvalCascadeHaarClassifier( CvIntHaarClassifier* classifier,
                                    sum_type* sum, sum_type* tilted, float normfactor );

void icvSaveHaarFeature( CvTHaarFeature* feature, FILE* file );

void icvLoadHaarFeature( CvTHaarFeature* feature, FILE* file );

void icvSaveCARTHaarClassifier( CvIntHaarClassifier* classifier, FILE* file );

CvIntHaarClassifier* icvLoadCARTHaarClassifier( FILE* file, int step );

void icvSaveStageHaarClassifier( CvIntHaarClassifier* classifier, FILE* file );

CvIntHaarClassifier* icvLoadCARTStageHaarClassifier( const char* filename, int step );


/* tree cascade classifier */

float icvEvalTreeCascadeClassifier( CvIntHaarClassifier* classifier,
                                    sum_type* sum, sum_type* tilted, float normfactor );

void icvSetLeafNode( CvTreeCascadeClassifier* tree, CvTreeCascadeNode* leaf );

float icvEvalTreeCascadeClassifierFilter( CvIntHaarClassifier* classifier, sum_type* sum,
                                          sum_type* tilted, float normfactor );

CvTreeCascadeNode* icvCreateTreeCascadeNode();

void icvReleaseTreeCascadeNodes( CvTreeCascadeNode** node );

void icvReleaseTreeCascadeClassifier( CvIntHaarClassifier** classifier );

/* Prints out current tree structure to <stdout> */
void icvPrintTreeCascade( CvTreeCascadeNode* root );

/* Loads tree cascade classifier */
CvIntHaarClassifier* icvLoadTreeCascadeClassifier( const char* filename, int step,
                                                   int* splits );

/* Finds leaves belonging to maximal level and connects them via leaf->next_same_level */
CvTreeCascadeNode* icvFindDeepestLeaves( CvTreeCascadeClassifier* tree );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CVHAARTRAINING_H_ */
