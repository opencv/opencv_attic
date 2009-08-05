#ifndef _INNER_FUNCTIONS_H
#define _INNER_FUNCTIONS_H

#include "cv.h"
#include "ml.h"

#define CC_PATH_MAX 512

#include <ctime>
#ifdef _WIN32
#define TIME( arg ) (((double) clock()) / CLOCKS_PER_SEC)
#else
#define TIME( arg ) (time( arg ))
#endif /* _WIN32 */

void cvGetSortedIndices( CvMat* val, CvMat* idx, int sortcols CV_DEFAULT( 0 ) );

CvMat*
cvPreprocessIndexArray( const CvMat* idx_arr, int data_arr_size, bool check_for_duplicates );

static inline double
log_ratio( double val )
{
    const double eps = 1e-5;

    val = MAX( val, eps );
    val = MIN( val, 1. - eps );
    return log( val/(1. - val) );
}

// For old Haar Classifier file
#define ICV_HAAR_SIZE_NAME            "size"
#define ICV_HAAR_STAGES_NAME          "stages"
#define ICV_HAAR_TREES_NAME             "trees"
#define ICV_HAAR_FEATURE_NAME             "feature"
#define ICV_HAAR_RECTS_NAME                 "rects"
#define ICV_HAAR_TILTED_NAME                "tilted"
#define ICV_HAAR_THRESHOLD_NAME           "threshold"
#define ICV_HAAR_LEFT_NODE_NAME           "left_node"
#define ICV_HAAR_LEFT_VAL_NAME            "left_val"
#define ICV_HAAR_RIGHT_NODE_NAME          "right_node"
#define ICV_HAAR_RIGHT_VAL_NAME           "right_val"
#define ICV_HAAR_STAGE_THRESHOLD_NAME   "stage_threshold"
#define ICV_HAAR_PARENT_NAME            "parent"
#define ICV_HAAR_NEXT_NAME              "next"

#endif