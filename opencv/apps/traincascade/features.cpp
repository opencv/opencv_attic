#include "features.h"
#include "cascadeclassifier.h"

//---------------------------- FeatureParams --------------------------------------

CvFeatureParams::CvFeatureParams() : maxCatCount( 0 )
{ name = CC_FEATURE_PARAMS; }

CvFeatureParams::CvFeatureParams( CvSize _winSize ) : maxCatCount( 0 )
{ name = CC_FEATURE_PARAMS; }

void CvFeatureParams::set( const CvFeatureParams* fp )
{
    maxCatCount = fp->maxCatCount;
}

void CvFeatureParams::write( CvFileStorage* fs ) const
{
    CV_FUNCNAME( "CvFeatureParams::write" );
    __BEGIN__;

    CV_CALL( cvWriteInt( fs, CC_MAX_CAT_COUNT, maxCatCount ) );

    __END__;
}

bool CvFeatureParams::read( CvFileStorage* fs, CvFileNode* map )
{
    bool res = false;

    CV_FUNCNAME( "CvFeatureParams::read" );
    __BEGIN__;

    CV_CALL( maxCatCount = cvReadIntByName( fs, map, CC_MAX_CAT_COUNT ) );

    res = true;

    __END__;
    return res;
}

//---------------------------- CascadeData --------------------------------------

CvCascadeData::CvCascadeData()
{
    cls = 0;
    cascade = 0;
    imgreader = 0;
    features = 0;
}

CvCascadeData::~CvCascadeData()
{
    clear();
}

void CvCascadeData::setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgFileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams )
{
    numPos = _numPos;
    numNeg = _numNeg;

    cascade = _cascade;
    winSize = _cascade->getParams()->winSize;
    featureParams = _featureParams;

    maxCatCount = _featureParams->maxCatCount;

    cls = numPos + numNeg == 0 ? 0 : cvCreateMat( numPos + numNeg, 1, CV_32FC1 );
    
    imgreader = _vecFileName && _bgFileName ?
        new CvImageReader( _vecFileName, _bgFileName, winSize ) : 0;
    assert( !features );
    generateFeatures();
}

void CvCascadeData::clear()
{
    cvReleaseMat( &cls );
    if ( imgreader )
    {
        delete imgreader;
        imgreader = 0;
    } 

    if ( features )
    {
        for( int i = 0; i < numFeatures; i++ )
            if ( features[i] ) 
                delete features[i];
        cvFree( &features );
        features = 0;
    }   
}

void CvCascadeData::calcNormfactor( const CvMat* _sum, const CvMat* _sqSum, float& _normfactor )
{
    CvRect normrect = cvRect( 1, 1, _sum->cols - 3, _sum->rows - 3 );
    int p0, p1, p2, p3;
    int   valSum   = 0;
    double valSqSum = 0;
    double area = 0.0;
    int offest = _sum->step/sizeof(_sum->data.i[0]);

    CV_SUM_OFFSETS( p0, p1, p2, p3, normrect, offest )
    
    area = normrect.width * normrect.height;
    valSum = _sum->data.i[p0] - _sum->data.i[p1]
           - _sum->data.i[p2] + _sum->data.i[p3];
    valSqSum = _sqSum->data.db[p0]
             - _sqSum->data.db[p1]
             - _sqSum->data.db[p2]
             + _sqSum->data.db[p3];

    _normfactor = (float) sqrt( (double) (area * valSqSum - (double)valSum * valSum) );
}

float CvCascadeData::getCls( int sampleIdx ) const
{
    int clsStep = cls->step / CV_ELEM_SIZE(cls->type);
    assert( cls );
    return cls->data.fl[sampleIdx*clsStep];
}

bool CvCascadeData::updateForNextStage( double& acceptanceRatio )
{
    int64 posConsumed = 0, negConsumed = 0;
    int posCount = fillPassedSamles( 0, numPos, true, posConsumed ), negCount;
    if( !posCount )
        return false;

    negCount = fillPassedSamles( numPos, numNeg, false, negConsumed );
    if ( !negCount )
        return false;
    acceptanceRatio = CCOUNTER_DIV(negCount, negConsumed);
    printf( "POS count : consumed  %d : %d\n", posCount, (int)posConsumed );
    printf( "NEG count : acceptanceRatio %d : %f\n", negCount, acceptanceRatio );

    numImg = posCount + negCount;
    return true;
}

void CvCascadeData::writeFeature( CvFileStorage* fs, int fi )
{
    if ((fi < numFeatures) && ( fi >= 0))
        features[fi]->write( fs );
}

void CvCascadeData::writeFeatures( CvFileStorage* fs, const CvMat* featureMap )
{
    CV_FUNCNAME( "CvCascadeData::writeFeatures" );
    __BEGIN__;

    CV_CALL( cvStartWriteStruct( fs, CC_FEATURES, CV_NODE_SEQ ) );
    for ( int fi = 0; fi < featureMap->cols; fi++ )
    {
        if ( featureMap->data.i[fi] >= 0 )
        {
            CV_CALL( cvStartWriteStruct( fs, 0, CV_NODE_MAP ) );
            features[fi]->write( fs );
            CV_CALL( cvEndWriteStruct( fs ) );
        }
    }
    CV_CALL( cvEndWriteStruct( fs ) );

    __END__;
}