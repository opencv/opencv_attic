#ifndef CASCADECLASSIFIER_H
#define CASCADECLASSIFIER_H

#include "features.h"
#include "haarfeatures.h"
#include "lbpfeatures.h"
#include "boost.h"
#include "cv.h"
#include "cxcore.h"

#define CC_CASCADE_FILENAME "cascade.xml"
#define CC_PARAMS_FILENAME "params.xml"

#define CC_CASCADE_PARAMS "cascadeParams"
#define CC_STAGE_TYPE "stageType"
#define CC_FEATURE_TYPE "featureType"
#define CC_HEIGHT "height"
#define CC_WIDTH  "width"

#define CC_STAGE_NUM    "stageNum"
#define CC_STAGES       "stages"
#define CC_STAGE_PARAMS "stageParams"

#define CC_BOOST            "BOOST"
#define CC_BOOST_TYPE       "boostType"
#define CC_DISCRETE_BOOST   "DAB"
#define CC_REAL_BOOST       "RAB"
#define CC_LOGIT_BOOST      "LB"
#define CC_GENTLE_BOOST     "GAB"
#define CC_MINHITRATE       "minHitRate"
#define CC_MAXFALSEALARM    "maxFalseAlarm"
#define CC_TRIM_RATE        "weightTrimRate"
#define CC_MAX_DEPTH        "maxDepth"
#define CC_WEAK_COUNT       "maxWeakCount"
#define CC_STAGE_THRESHOLD  "stageThreshold"
#define CC_WEAK_CLASSIFIERS "weakClassifiers"
#define CC_INTERNAL_NODES   "internalNodes"
#define CC_LEAF_VALUES      "leafValues"

#define CC_FEATURES "features"
#define CC_FEATURE_PARAMS "featureParams"
#define CC_MAX_CAT_COUNT  "maxCatCount"

#define CC_HAAR        "HAAR"
#define CC_MODE        "mode"
#define CC_MODE_BASIC  "BASIC"
#define CC_MODE_CORE   "CORE"
#define CC_MODE_ALL    "ALL"
#define CC_RECTS       "rects"
#define CC_TILTED      "tilted"

#define CC_LBP  "LBP"
#define CC_RECT "rect"

#define CV_NEW_SAVE_FORMAT 0
#define CV_OLD_SAVE_FORMAT 1

struct CvCascadeParams : CvParams
{
    enum { BOOST = 0 };
    enum { HAAR = 0, LBP = 1 };
    
    static const int defaultStageType = BOOST;
    static const int defaultFeatureType = HAAR;

    CvCascadeParams() : stageType( defaultStageType ), featureType( defaultFeatureType ), winSize( cvSize(24, 24) )
    { name = CC_CASCADE_PARAMS; }
    CvCascadeParams( int _stageType, int _featureType ) :
        stageType( _stageType ), featureType( _featureType ), winSize( cvSize(24, 24) )
    { name = CC_CASCADE_PARAMS; }
    virtual ~CvCascadeParams() {}
    void write( CvFileStorage* fs ) const;
    bool read( CvFileStorage* fs, CvFileNode* node );

    void printDefaults();
    void printAttrs();    
    bool scanAttr( const char* prmName, const char* val );

    int stageType;
    int featureType;
    CvSize winSize;
};

class CvCascadeClassifier
{
public:
    CvCascadeClassifier();
    virtual ~CvCascadeClassifier();

    virtual bool train( const char* _cascadeDirName,
                        const char* _vecFileName,
                        const char* _bgfileName, 
                        int _numPos, int _numNeg, 
                        int _numPrecalcVal, int _numPrecalcIdx,
                        int _numStages,
                        const CvCascadeParams& _cascadeParams,
                        const CvFeatureParams& _featureParams,
                        const CvCascadeBoostParams& _stageParams,
                        bool baseFormatSave = false );
    virtual int predict( int sampleIdx );
    virtual bool save( const char* cascadeDirName, bool baseFormat = false );
    const CvCascadeParams* getParams() const { return &cascadeParams; }
protected:     
    void createStageParams();
    void createCurStage();
    void createFeatureParams();
    void createCascadeData();

    virtual void writeParams( CvFileStorage* fs ) const;
    virtual void writeStages( CvFileStorage* fs, const CvMat* featureMap ) const;
    virtual void writeFeatures( CvFileStorage* fs, const CvMat* featureMap ) const;
    
    virtual bool readParams( CvFileStorage* fs, CvFileNode* node );
    virtual bool readStages( CvFileStorage* fs, CvFileNode* node );

    virtual bool loadTempInfo( const char* cascadeDirName, const char* _vecFileName, const char* _bgFileName, 
        int _numPos, int _numNeg );
    void markFeaturesInMap( CvMat* featureMap);

    int numStages, numCurStages;

    CvCascadeData* cascadeData;    
    CvCascadeBoost** stageClassifiers;

    CvCascadeParams cascadeParams;
    CvCascadeBoostParams* stageParams;
    CvFeatureParams* featureParams;
};

#endif