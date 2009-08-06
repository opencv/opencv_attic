#ifndef FEATURES_H
#define FEATURES_H

#include "_imagestorage.h"
#include "cxcore.h"
#include "cv.h"
#include "ml.h"
#include <stdio.h>

typedef uint64 ccounter_t;
#define CCOUNTER_DIV(cc0, cc1) ( ((cc1) == 0) ? 0 : ( ((double)(cc0))/(double)(int64)(cc1) ) )

#define CV_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x + w, y) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

#define CV_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);

//-------------------------------------- Params ---------------------------------------------

struct CvParams
{
    CvParams() : name( "params" ) {}
    virtual ~CvParams() {}
    // from|to file
    virtual void write( CvFileStorage* fs ) const = 0;
    virtual bool read( CvFileStorage* fs, CvFileNode* node ) = 0;

    // from|to screen
    virtual void printDefaults()
    { printf( "--%s--\n", name ); };
    virtual void printAttrs(){};
    virtual bool scanAttr( const char* prmName, const char* val ){ return false; }
    const char* name;
};

//----------------------------------  FeatureParams  ----------------------------------------
struct CvFeatureParams : CvParams
{
    CvFeatureParams();
    CvFeatureParams( CvSize _winSize );
    virtual ~CvFeatureParams()
    {}

    virtual void set( const CvFeatureParams* fp );

    virtual void write( CvFileStorage* fs ) const;
    virtual bool read( CvFileStorage* fs, CvFileNode* node );

    int maxCatCount; // 0 in case of numerical features
};

//----------------------------------  Features  ----------------------------------------------

struct CvFeature
{
    CvFeature() {}
    virtual ~CvFeature() {}
    virtual void write( CvFileStorage* fs ) const = 0;
};

//----------------------------------  CascadeData ----------------------------------------

class CvCascadeClassifier;

class CvCascadeData
{
public:
    CvCascadeData();
    virtual ~CvCascadeData();

    virtual void setData( CvCascadeClassifier* _cascade,
         const char* _vecFileName, const char* _bgFileName, 
         int _numPos, int _numNeg, const CvFeatureParams* _featureParams );
    virtual void clear();
    virtual void generateFeatures() = 0;
    virtual float calcFeature( int featureIdx, int sampleIdx ) = 0;

    static void calcNormfactor( const CvMat* _sum, const CvMat* _sqSum, float& _normFactor );

    virtual bool updateForNextStage( double& acceptanceRatio );    

    int getMaxCatCount() const { return featureParams->maxCatCount; }
    CvSize getWinSize() const { return winSize; }
    int getNumFeatures() const { return numFeatures; }
    int getNumSamples() const { return numImg; }
    float getCls( int sampleIdx ) const;
    const CvMat* getCls() const { return cls; }    
    
    virtual void writeFeature( CvFileStorage* fs, int fi ); // for old file format
    virtual void writeFeatures( CvFileStorage* fs, const CvMat* featureMap );

protected:    
    virtual int fillPassedSamles( int first, int count, bool isPositive, int64& consumed ) = 0;
    int maxCatCount;
    int numFeatures;
    int numPos, numNeg, numImg;
    CvSize winSize;

    CvMat* cls; /* classes. 1.0 - object, 0.0 - background */

    CvCascadeClassifier* cascade;
    const CvFeatureParams* featureParams;
    CvImageReader* imgreader;
	CvFeature** features;
};

#endif