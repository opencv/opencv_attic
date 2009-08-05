#ifndef HAARFEATURES_H
#define HAARFEATURES_H

#include "features.h"

#define CV_HAAR_FEATURE_MAX      3
#define CV_HAAR_FEATURE_DESC_MAX 20


#define HFP_NAME "haarFeatureParams"
struct CvHaarFeatureParams : CvFeatureParams
{
    enum { BASIC = 0, CORE = 1, ALL = 2 };
     /* 0 - BASIC = Viola
     *  1 - CORE  = All upright
     *  2 - ALL   = All features
     */

    CvHaarFeatureParams() : mode(BASIC)
    { name = HFP_NAME; }
    CvHaarFeatureParams( CvSize _winSize, int _mode ) :
        CvFeatureParams( _winSize ), mode( _mode )
    { name = HFP_NAME; }
    virtual ~CvHaarFeatureParams()
    {}
    
    virtual void set( const CvFeatureParams* fp );
    virtual void write( CvFileStorage* fs ) const;

    virtual void printDefaults();  
    virtual void printAttrs();
    virtual bool scanAttr( const char* prm, const char* val);
    
    int mode;
};

struct CvHaarFeature1 : public CvFeature
{
    CvHaarFeature1();
    CvHaarFeature1( bool _tilted,
        int x0, int y0, int w0, int h0, float wt0,
        int x1, int y1, int w1, int h1, float wt1,
        int x2 CV_DEFAULT( 0 ), int y2 CV_DEFAULT( 0 ),
        int w2 CV_DEFAULT( 0 ), int h2 CV_DEFAULT( 0 ),
        float wt2 CV_DEFAULT( 0.0F ) ); 
    virtual ~CvHaarFeature1() {}

    virtual void write( CvFileStorage* fs ) const;
    float calc( const int* _sum_row, const int* _tilted_row );

    bool  tilted;
    struct
    {
        CvRect r;
        float weight;
    } rect[CV_HAAR_FEATURE_MAX];

    struct                      
    {
        int p0, p1, p2, p3;
    } fastRect[CV_HAAR_FEATURE_MAX];
};

class CvHaarCascadeData : public CvCascadeData
{
public:
    CvHaarCascadeData();
    virtual void setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgFileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams );

    virtual void clear();
    
    virtual void generateFeatures();
    void updateFastFeatures(int step); // offset - row step for the integral image ( weight + 1)

    virtual float calcFeature( int featureIdx, int sampleIdx );
protected:    
    virtual int fillPassedSamles( int first, int count, bool isPositive, int64& consumed );
    CvMat*  sum;         /* sum images (each row represents image) */
    CvMat*  tilted;      /* tilted sum images (each row represents image) */
    CvMat*  normfactor;  /* normalization factor */
};

float CV_INLINE CvHaarFeature1::calc( const int* _sum_row, const int* _tilted_row )
{
    const int* img = tilted ? _tilted_row : _sum_row;
    float ret = rect[0].weight * (img[fastRect[0].p0] - img[fastRect[0].p1] - img[fastRect[0].p2] + img[fastRect[0].p3] ) +
        rect[1].weight * (img[fastRect[1].p0] - img[fastRect[1].p1] - img[fastRect[1].p2] + img[fastRect[1].p3] );
    if( rect[2].weight != 0.0f )
        ret += rect[2].weight * (img[fastRect[2].p0] - img[fastRect[2].p1] - img[fastRect[2].p2] + img[fastRect[2].p3] );
    return ret;
}

float CV_INLINE CvHaarCascadeData::calcFeature( int featureIdx, int sampleIdx )
{
    int sumStep = sum->step / CV_ELEM_SIZE( sum->type );
    int tiltedStep = sum->step / CV_ELEM_SIZE( tilted->type );
    float nf, val;

    assert( features );
    nf = normfactor->data.fl[sampleIdx];
    val = ((CvHaarFeature1*)(features[featureIdx]))->calc( sum->data.i + sampleIdx * sumStep, 
        tilted->data.i + sampleIdx * tiltedStep );
    val = ( nf == 0.0F ) ? 0.0F : (val / nf);
    return val;
}
#endif
