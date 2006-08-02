#include <cxcore.h>
#include <cv.h>
#include "cvshadow.h"

CvArr * cvCvtSeqToArray_Shadow( const CvSeq* seq, CvArr * elements, CvSlice slice){
        CvMat stub, *mat=(CvMat *)elements;
        if(!CV_IS_MAT(mat)){
            mat = cvGetMat(elements, &stub);
        }
        cvCvtSeqToArray( seq, mat->data.ptr, slice );
        return elements;
    }

double cvArcLength_Shadow( const CvSeq * seq, CvSlice slice, int is_closed){
    return cvArcLength( seq, slice, is_closed );
}
double cvArcLength_Shadow( const CvArr * arr, CvSlice slice, int is_closed){
    return cvArcLength( arr, slice, is_closed );
}

CvTypedSeq<CvRect> * cvHaarDetectObjects_Shadow( const CvArr* image, CvHaarClassifierCascade* cascade,
        CvMemStorage* storage, double scale_factor, int min_neighbors, int flags,
        CvSize min_size )
{
        return (CvTypedSeq<CvRect> *) cvHaarDetectObjects( image, cascade, storage, scale_factor,
                                                                min_neighbors, flags, min_size);
}

CvTypedSeq<CvConnectedComp> *  cvSegmentMotion_Shadow( const CvArr* mhi, CvArr* seg_mask, CvMemStorage* storage,
                                                                double timestamp, double seg_thresh ){
    return (CvTypedSeq<CvConnectedComp> *) cvSegmentMotion( mhi, seg_mask, storage, timestamp, seg_thresh );
}

CvTypedSeq<CvPoint> * cvApproxPoly_Shadow( const void* src_seq, int header_size, CvMemStorage* storage,
                                    int method, double parameter, int parameter2)
{
    return (CvTypedSeq<CvPoint> *) cvApproxPoly( src_seq, header_size, storage, method, parameter, parameter2 );
}

