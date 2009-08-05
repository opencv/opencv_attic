#ifndef LBP_H
#define LBP_H

#include "features.h"

#define LBPF_NAME "lbpFeatureParams"
struct CvLBPFeatureParams : CvFeatureParams
{
    CvLBPFeatureParams() 
        { maxCatCount = 256; name = LBPF_NAME; }
    CvLBPFeatureParams( CvSize _winSize ) :
        CvFeatureParams( _winSize )
        { maxCatCount = 256; name = LBPF_NAME; }
    virtual ~CvLBPFeatureParams()
    {}
};

struct CvLBPFeature : public CvFeature
{
    CvLBPFeature();
    CvLBPFeature( int x, int y, int _block_w, int _block_h  ); 
    virtual ~CvLBPFeature() {}

    virtual void write( CvFileStorage* fs ) const;

    float calc( const int* _sum_row, int offset );

    CvRect rect;
    int p[16];
};

class CvLBPCascadeData : public CvCascadeData
{
public:
    CvLBPCascadeData();
    virtual void setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgFileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams );
    virtual void clear();
    virtual void generateFeatures();
    void updateFastFeatures(int step); // offset - row step for the integral image ( weight + 1)
    virtual float calcFeature( int featureIdx, int sampleIdx );
protected:    
    virtual int fillPassedSamles( int first, int count, bool isPositive, int64& consumed );
    int sumCols;
    CvMat*  sum;         /* sum images (each row represents image) */
};


float CV_INLINE CvLBPFeature::calc( const int* _sum_row, int offset )
{
    int cval = _sum_row[p[5]] - _sum_row[p[6]] - _sum_row[p[9]] + _sum_row[p[10]];

    return (float)((_sum_row[p[0]] - _sum_row[p[1]] - _sum_row[p[4]] + _sum_row[p[5]] >= cval ? 128 : 0) |   // 0
           (_sum_row[p[1]] - _sum_row[p[2]] - _sum_row[p[5]] + _sum_row[p[6]] >= cval ? 64 : 0) |    // 1
           (_sum_row[p[2]] - _sum_row[p[3]] - _sum_row[p[6]] + _sum_row[p[7]] >= cval ? 32 : 0) |    // 2
           (_sum_row[p[6]] - _sum_row[p[7]] - _sum_row[p[10]] + _sum_row[p[11]] >= cval ? 16 : 0) |  // 5
           (_sum_row[p[10]] - _sum_row[p[11]] - _sum_row[p[14]] + _sum_row[p[15]] >= cval ? 8 : 0)|  // 8
           (_sum_row[p[9]] - _sum_row[p[10]] - _sum_row[p[13]] + _sum_row[p[14]] >= cval ? 4 : 0)|   // 7
           (_sum_row[p[8]] - _sum_row[p[9]] - _sum_row[p[12]] + _sum_row[p[13]] >= cval ? 2 : 0)|    // 6
           (_sum_row[p[4]] - _sum_row[p[5]] - _sum_row[p[8]] + _sum_row[p[9]] >= cval ? 1 : 0));      // 3
}

float CV_INLINE CvLBPCascadeData::calcFeature( int featureIdx, int sampleIdx )
{
    int sumStep = sum->step / CV_ELEM_SIZE( sum->type );
    assert( features );
    return ((CvLBPFeature*)(features[featureIdx]))->calc( sum->data.i + sampleIdx * sumStep,
        winSize.width + 1 );
}

#endif