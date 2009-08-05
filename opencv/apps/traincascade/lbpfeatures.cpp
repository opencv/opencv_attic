#include "lbpfeatures.h"
#include "cascadeclassifier.h"

//------------------------------------- LBPFeature -------------------------------------

CvLBPFeature::CvLBPFeature()
{
    rect = cvRect(0, 0, 0, 0);
}

CvLBPFeature::CvLBPFeature( int x, int y, int _blockWidth, int _blockHeight )
{
    rect = cvRect(x, y, _blockWidth, _blockHeight);
}

CV_INLINE void CvLBPFeature::write( CvFileStorage* fs ) const
{
    CV_FUNCNAME( "CvLBPFeature::write" );
    __BEGIN__;

    CV_CALL( cvStartWriteStruct( fs, CC_RECT, CV_NODE_SEQ ) );
    
    CV_CALL( cvWriteInt( fs, NULL, rect.x ) );
    CV_CALL( cvWriteInt( fs, NULL, rect.y ) );
    CV_CALL( cvWriteInt( fs, NULL, rect.width ) );
    CV_CALL( cvWriteInt( fs, NULL, rect.height ) );
    
    CV_CALL( cvEndWriteStruct( fs ) ); /* LBP_RECT */
    
    __END__;
}

//------------------------------------- LBPCascadeData ---------------------------------

CvLBPCascadeData::CvLBPCascadeData()
{
    sum = 0;
}

void CvLBPCascadeData::setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgfileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams  )
{
    CvSize ws = _cascade->getParams()->winSize;
    int sumCols = (ws.width + 1) * (ws.height + 1);

    sum = cvCreateMat( _numPos + _numNeg, sumCols, CV_32SC1);
    CvCascadeData::setData( _cascade, _vecFileName, _bgfileName, _numPos, _numNeg, _featureParams );
}

void CvLBPCascadeData::clear()
{
    cvReleaseMat( &sum );

    CvCascadeData::clear();
}

void CvLBPCascadeData::generateFeatures()
{
    CvLBPFeature* lbpFeature;

    CvMemStorage* storage = NULL;
    CvSeq* seq = NULL;
    CvSeqWriter writer;
    
    storage = cvCreateMemStorage();
    cvStartWriteSeq( 0, sizeof( CvSeq ), sizeof( lbpFeature ), storage, &writer );

    for( int x = 0; x < winSize.width; x++ )
    {
        for( int y = 0; y < winSize.height; y++ )
        {
            for( int w = 1; w <= winSize.width / 3; w++ )
            {
                for( int h = 1; h <= winSize.height / 3; h++ )
                {
                    if ( (x+3*w <= winSize.width) && (y+3*h <= winSize.height) )
                    {
                        lbpFeature = new CvLBPFeature( x, y, w, h );
                        CV_WRITE_SEQ_ELEM( lbpFeature, writer );
                    }
                }
            }
        }
    }
    seq = cvEndWriteSeq( &writer );
    numFeatures = seq->total;
    features = (CvFeature**) cvAlloc( sizeof( CvLBPFeature* ) * numFeatures );
        
    cvCvtSeqToArray( seq, (CvArr*)features );
    updateFastFeatures( winSize.width + 1 );
    cvReleaseMemStorage( &storage );
}

void CvLBPCascadeData::updateFastFeatures(int offset)
{
    for( int fi = 0; fi < numFeatures; fi++ )
    {
        CvLBPFeature* tempFeature = (CvLBPFeature*)(features[fi]);
        int *p = tempFeature->p;
        CvRect tr;
        tr = tempFeature->rect;
        CV_SUM_OFFSETS( p[0], p[1], p[4], p[5], tr, offset )
        tr.x += 2*tempFeature->rect.width;
        CV_SUM_OFFSETS( p[2], p[3], p[6], p[7], tr, offset )
        tr.y +=2*tempFeature->rect.height;
        CV_SUM_OFFSETS( p[10], p[11], p[14], p[15], tr, offset )
        tr.x -= 2*tempFeature->rect.width;
        CV_SUM_OFFSETS( p[8], p[9], p[12], p[13], tr, offset )
    }
}

int CvLBPCascadeData::fillPassedSamles( int first, int count, bool isPositive, int64& consumed )
{
    int getcount = 0;

    //CV_FUNCNAME( "CvLBPCascadeData::fillPassedSamles" );
    __BEGIN__;

    bool reset = true;
    CvMat innSum, img;
    int step = sum->step / CV_ELEM_SIZE(sum->type),
        clsStep = cls->step / CV_ELEM_SIZE(cls->type);

    assert( first + count <= numPos + numNeg );
   
    img = cvMat( winSize.height, winSize.width, CV_8UC1,
        cvStackAlloc( sizeof( uchar ) * winSize.height * winSize.width ) );
    innSum = cvMat( winSize.height + 1, winSize.width + 1,
                     CV_32SC1, NULL );

    for( int i = first; i < first + count; i++ )
    {
        for( ; ; )
        {
            bool is_get_img = isPositive ? imgreader->getPosImage( &img, reset ) :
                                           imgreader->getNegImage( &img, reset );
            reset = false;
            if( !is_get_img ) 
                EXIT;

            innSum.data.i = sum->data.i + i * step;
            cvIntegralImage( &img, &innSum, 0, 0 );
            consumed++;

            if( cascade->predict( i ) == 1.0F )
            {
                getcount++;
                cls->data.fl[i*clsStep] = isPositive ? 1.f : 0.f;
                break;
            }
        }
    }

     __END__;
    return getcount;
}