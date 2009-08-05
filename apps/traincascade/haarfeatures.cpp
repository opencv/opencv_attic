#include "haarfeatures.h"
#include "cascadeclassifier.h"

//------------------------------ HaarFeatureParams -----------------------------------

void CvHaarFeatureParams::set( const CvFeatureParams* fp )
{
    CvFeatureParams::set( fp );
    mode = ((const CvHaarFeatureParams*)fp)->mode;
}

void CvHaarFeatureParams::write( CvFileStorage* fs ) const
{
    CV_FUNCNAME( "CvHaarFeatureParams::write" );
    __BEGIN__;
    const char* mode_str;

    CvFeatureParams::write( fs );

    mode_str = mode == BASIC ? CC_MODE_BASIC : 
               mode == CORE ? CC_MODE_CORE :
               mode == ALL ? CC_MODE_ALL : 0;
    if ( mode_str )
    { CV_CALL( cvWriteString( fs, CC_MODE, mode_str ) ); }
    else
    { CV_CALL( cvWriteInt( fs, CC_MODE, mode ) ); }

    __END__;
}

void CvHaarFeatureParams::printDefaults()
{
    CvFeatureParams::printDefaults();
    printf("  [-mode <%s (default) | %s | %s>]\n", CC_MODE_BASIC, CC_MODE_CORE, CC_MODE_ALL );
}

void CvHaarFeatureParams::printAttrs()
{
    CvFeatureParams::printAttrs();
    const char* mode_str = mode == BASIC ? CC_MODE_BASIC : 
                           mode == CORE ? CC_MODE_CORE :
                           mode == ALL ? CC_MODE_ALL : 0;
    printf("mode: %s\n", mode_str);
}

bool CvHaarFeatureParams::scanAttr( const char* prmName, const char* val)
{
    if ( !CvFeatureParams::scanAttr( prmName, val ) )
    {
        if( !strcmp( prmName, "-mode" ) )
        {
            mode = !strcmp( val, CC_MODE_CORE ) ? CORE :
                   !strcmp( val, CC_MODE_ALL ) ? ALL :
                   !strcmp( val, CC_MODE_BASIC ) ? BASIC : -1;
            if (mode == -1)
                return false;
        }
        return false;
    }
    return true;
}
//--------------------------------- HaarFeature --------------------------------------

CV_INLINE CvHaarFeature1::CvHaarFeature1()
{
    tilted = 0;
    rect[0].r = rect[1].r = rect[2].r = cvRect(0,0,0,0);
    rect[0].weight = rect[1].weight = rect[2].weight = 0;
}

CV_INLINE CvHaarFeature1::CvHaarFeature1( bool _tilted,
                            int x0, int y0, int w0, int h0, float wt0,
                            int x1, int y1, int w1, int h1, float wt1,
                            int x2, int y2, int w2, int h2, float wt2 )
{
    assert( CV_HAAR_FEATURE_MAX >= 3 );

    tilted = _tilted;
    
    rect[0].r.x = x0;
    rect[0].r.y = y0;
    rect[0].r.width  = w0;
    rect[0].r.height = h0;
    rect[0].weight   = wt0;
    
    rect[1].r.x = x1;
    rect[1].r.y = y1;
    rect[1].r.width  = w1;
    rect[1].r.height = h1;
    rect[1].weight   = wt1;
    
    rect[2].r.x = x2;
    rect[2].r.y = y2;
    rect[2].r.width  = w2;
    rect[2].r.height = h2;
    rect[2].weight   = wt2;
}

CV_INLINE void CvHaarFeature1::write( CvFileStorage* fs ) const
{
    CV_FUNCNAME( "CvHaarFeature1::write" );
    __BEGIN__;

    CV_CALL( cvStartWriteStruct( fs, CC_RECTS, CV_NODE_SEQ ) );
    for( int ri = 0; ri < CV_HAAR_FEATURE_MAX && rect[ri].r.width != 0; ++ri )
    {
        CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_SEQ | CV_NODE_FLOW ) );
        CV_CALL( cvWriteInt(  fs, NULL, rect[ri].r.x ) );
        CV_CALL( cvWriteInt(  fs, NULL, rect[ri].r.y ) );
        CV_CALL( cvWriteInt(  fs, NULL, rect[ri].r.width ) );
        CV_CALL( cvWriteInt(  fs, NULL, rect[ri].r.height ) );
        CV_CALL( cvWriteReal( fs, NULL, rect[ri].weight ) );
        CV_CALL( cvEndWriteStruct( fs ) );
    }
    CV_CALL( cvEndWriteStruct( fs ) ); /* HF_RECTS */
    CV_CALL( cvWriteInt( fs, CC_TILTED, tilted ) );

    __END__;
}

//---------------------------- HaarCascadeData --------------------------------------

CvHaarCascadeData::CvHaarCascadeData()
{
    sum = tilted = normfactor = 0; 
}

void CvHaarCascadeData::setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgfileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams  )
{
    int maxNumSamples = _numPos + _numNeg;
    CvSize ws = _cascade->getParams()->winSize;
    int sumCols = (ws.width + 1) * (ws.height + 1);

    sum = cvCreateMat( maxNumSamples, sumCols, CV_32SC1);
    tilted = cvCreateMat( maxNumSamples, sumCols, CV_32SC1);
    normfactor = cvCreateMat( 1, maxNumSamples, CV_32FC1 );

    CvCascadeData::setData( _cascade, _vecFileName, _bgfileName, _numPos, _numNeg, _featureParams );
}

void CvHaarCascadeData::clear()
{
    cvReleaseMat( &sum );
    cvReleaseMat( &tilted );
    cvReleaseMat( &normfactor );
    CvCascadeData::clear();
}

void CvHaarCascadeData::generateFeatures()
{
    CvHaarFeature1* haarFeature;
    CvMemStorage* storage = cvCreateMemStorage();
    CvSeq* seq = NULL;
    CvSeqWriter writer;
    int mode = ((const CvHaarFeatureParams*)featureParams)->mode;

    cvStartWriteSeq( 0, sizeof( CvSeq ), sizeof( haarFeature ), storage, &writer );

    for( int x = 0; x < winSize.width; x++ )
    {
        for( int y = 0; y < winSize.height; y++ )
        {
            for( int dx = 1; dx <= winSize.width; dx++ )
            {
                for( int dy = 1; dy <= winSize.height; dy++ )
                {
                    // haar_x2
                    if ( (x+dx*2 <= winSize.width) && (y+dy <= winSize.height) )
                    {
                        if ( x+x+dx*2 <= winSize.width ) 
                        {
                            haarFeature = new CvHaarFeature1( false,
                                x,    y, dx*2, dy, -1,
                                x+dx, y, dx  , dy, +2 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }
                    // haar_y2
                    if ( (x+dx*2 <= winSize.height) && (y+dy <= winSize.width) ) 
                    {
                        if ( y+y+dy <= winSize.width ) {
                            haarFeature = new CvHaarFeature1( false,
                                y, x,    dy, dx*2, -1,
                                y, x+dx, dy, dx,   +2 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }

                    // haar_x3
                    if ( (x+dx*3 <= winSize.width) && (y+dy <= winSize.height) )
                    {
                        if ( x+x+dx*3 <= winSize.width )
                        {
                            haarFeature = new CvHaarFeature1( false,
                                x,    y, dx*3, dy, -1,
                                x+dx, y, dx,   dy, +3 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }
                    // haar_y3
                    if ( (x+dx*3 <= winSize.height) && (y+dy <= winSize.width) ) {
                        if ( y+y+dy <= winSize.width ) {
                            haarFeature = new CvHaarFeature1( false,
                                y, x,    dy, dx*3, -1,
                                y, x+dx, dy, dx,   +3 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }
                    if( mode != CvHaarFeatureParams::BASIC )
                    {
                        // haar_x4
                        if ( (x+dx*4 <= winSize.width) && (y+dy <= winSize.height) ) 
                        {
                            if ( x+x+dx*4 <= winSize.width ) {
                                haarFeature = new CvHaarFeature1( false,
                                    x,    y, dx*4, dy, -1,
                                    x+dx, y, dx*2, dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // haar_y4
                        if ( (x+dx*4 <= winSize.height) && (y+dy <= winSize.width ) ) 
                        {
                            if ( y+y+dy <= winSize.width ) {
                                haarFeature = new CvHaarFeature1( false,
                                    y, x,    dy, dx*4, -1,
                                    y, x+dx, dy, dx*2, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                    }
                    // x2_y2
                    if ( (x+dx*2 <= winSize.width) && (y+dy*2 <= winSize.height) ) 
                    {
                        if ( x+x+dx*2 <= winSize.width ) 
                        {
                            haarFeature = new CvHaarFeature1( false,
                                x   , y,    dx*2, dy*2, -1,
                                x   , y   , dx  , dy,   +2,
                                x+dx, y+dy, dx  , dy,   +2 );
                            CV_WRITE_SEQ_ELEM( haarFeature, writer );
                        }
                    }
                    if (mode != CvHaarFeatureParams::BASIC) 
                    {                
                        if ( (x+dx*3 <= winSize.width) && (y+dy*3 <= winSize.height) ) 
                        {
                            if ( x+x+dx*3 <= winSize.width )  
                            {
                                haarFeature = new CvHaarFeature1( false,
                                    x   , y,    dx*3, dy*3, -1,
                                    x+dx, y+dy, dx  , dy  , +9);
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                    }
                    if (mode == CvHaarFeatureParams::ALL) 
                    {                
                        // tilted haar_x2                                      (x, y, w, h, b, weight)
                        if ( (x+2*dx <= winSize.width) && (y+2*dx+dy <= winSize.height) && (x-dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x, y, dx*2, dy, -1,
                                    x, y, dx  , dy, +2 );
                                    CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // tilted haar_y2                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winSize.width) && (y+dx+2*dy <= winSize.height) && (x-2*dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x, y, dx, 2*dy, -1,
                                    x, y, dx,   dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // tilted haar_x3                                   (x, y, w, h, b, weight)
                        if ( (x+3*dx <= winSize.width) && (y+3*dx+dy <= winSize.height) && (x-dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x,    y,    dx*3, dy, -1,
                                    x+dx, y+dx, dx  , dy, +3 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // tilted haar_y3                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winSize.width) && (y+dx+3*dy <= winSize.height) && (x-3*dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x,    y,    dx, 3*dy, -1,
                                    x-dy, y+dy, dx,   dy, +3 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // tilted haar_x4                                   (x, y, w, h, b, weight)
                        if ( (x+4*dx <= winSize.width) && (y+4*dx+dy <= winSize.height) && (x-dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x,    y,    dx*4, dy, -1,
                                    x+dx, y+dx, dx*2, dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                        // tilted haar_y4                                      (x, y, w, h, b, weight)
                        if ( (x+dx <= winSize.width) && (y+dx+4*dy <= winSize.height) && (x-4*dy>= 0) ) 
                        {
                            if ( x <= (winSize.width / 2) ) 
                            {
                                haarFeature = new CvHaarFeature1( true,
                                    x,    y,    dx, 4*dy, -1,
                                    x-dy, y+dy, dx, 2*dy, +2 );
                                CV_WRITE_SEQ_ELEM( haarFeature, writer );
                            }
                        }
                    }
                }
            }
        }
    }
    seq = cvEndWriteSeq( &writer );
    numFeatures = seq->total;
    features = (CvFeature**) cvAlloc( sizeof( CvHaarFeature1* ) * numFeatures );
    cvCvtSeqToArray( seq, (CvArr*)features );
    updateFastFeatures( winSize.width + 1 );

    cvReleaseMemStorage( &storage );
}

void CvHaarCascadeData::updateFastFeatures(int step)
{
    for( int fi = 0; fi < numFeatures; fi++)
    {
        if( !((CvHaarFeature1*)features[fi])->tilted )
        {
            for( int j = 0; j < CV_HAAR_FEATURE_MAX; j++ )
            {
                if( ((CvHaarFeature1*)features[fi])->rect[j].weight == 0.0F )
                {
                    break;
                }
                CV_SUM_OFFSETS( ((CvHaarFeature1*)features[fi])->fastRect[j].p0,
                                ((CvHaarFeature1*)features[fi])->fastRect[j].p1,
                                ((CvHaarFeature1*)features[fi])->fastRect[j].p2,
                                ((CvHaarFeature1*)features[fi])->fastRect[j].p3,
                                ((CvHaarFeature1*)features[fi])->rect[j].r, step )
            }
        }
        else
        {
            for( int j = 0; j < CV_HAAR_FEATURE_MAX; j++ )
            {
                if( ((CvHaarFeature1*)features[fi])->rect[j].weight == 0.0F )
                {
                    break;
                }
                CV_TILTED_OFFSETS( ((CvHaarFeature1*)features[fi])->fastRect[j].p0,
                                    ((CvHaarFeature1*)features[fi])->fastRect[j].p1,
                                    ((CvHaarFeature1*)features[fi])->fastRect[j].p2,
                                    ((CvHaarFeature1*)features[fi])->fastRect[j].p3,
                                    ((CvHaarFeature1*)features[fi])->rect[j].r, step )
            }
        }
    }
}

int CvHaarCascadeData::fillPassedSamles( int first, int count, bool isPositive, int64& consumed )
{
    int getcount = 0;

    //CV_FUNCNAME( "CvHaarCascadeData::fillPassedSamles" );
    __BEGIN__;

    bool reset = true;
    CvMat innSum, innSqSum, innTilted, img;
    int step = sum->step / CV_ELEM_SIZE(sum->type),
        clsStep = cls->step / CV_ELEM_SIZE(cls->type);

    assert( first + count <= numPos + numNeg );
   
    img = cvMat( winSize.height, winSize.width, CV_8UC1,
        cvStackAlloc( sizeof( uchar ) * winSize.height * winSize.width ) );

    innSum = cvMat( winSize.height + 1, winSize.width + 1,
                     CV_32SC1, NULL );

    innSqSum = cvMat( winSize.height + 1, winSize.width + 1, CV_64F,
        cvStackAlloc( (winSize.height + 1) * (winSize.width + 1) * CV_ELEM_SIZE(CV_64F)) );

    innTilted = cvMat( winSize.height + 1, winSize.width + 1,
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
            innTilted.data.i = tilted->data.i + i * step;

            cvIntegralImage( &img, &innSum, &innSqSum, &innTilted );
            calcNormfactor( &innSum, &innSqSum, normfactor->data.fl[i]);
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