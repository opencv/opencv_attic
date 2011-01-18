#ifndef _OPENCV_BOOSTSVM_H_
#define _OPENCV_BOOSTSVM_H_

#include "traincascade_features.h"
#include "ml.h"

struct CvCascadeBoostSVMParams : CvBoostSVMParams
{
    float minHitRate;
    float maxFalseAlarm;

    CvCascadeBoostSVMParams();
    CvCascadeBoostSVMParams( int boostType, float minHitRate, float maxFalseAlarm,
                             double weightTrimRate, int maxWeakCount, double cost, TermCriteria termCrit );
    virtual ~CvCascadeBoostSVMParams() {}
    void write( FileStorage &fs ) const;
    bool read( const FileNode &node );
    virtual void printDefaults() const;
    virtual void printAttrs() const;
    virtual bool scanAttr( const String prmName, const String val );
};


//class CvCascadeBoostSVM : public CvSVM
//{
//public:
//    virtual bool train( const CvFeatureEvaluator* _featureEvaluator, int _numSamples, 
//                        const CvCascadeBoostSVMParams& _params=CvCascadeBoostSVMParams() )
//    { }
//    virtual float predict( int sampleIdx, bool returnSum = false ) const {}
//
//    float getThreshold() const { return threshold; }; 
////    void write( FileStorage &fs, const Mat& featureMap ) const;
////    bool read( const FileNode &node, const CvFeatureEvaluator* _featureEvaluator,
////               const CvCascadeBoostParams& _params );
////    void markUsedFeaturesInMap( Mat& featureMap );
////protected:
////    virtual bool set_params( const CvBoostParams& _params );
////    virtual void update_weights( CvBoostTree* tree );
////    virtual bool isErrDesired();
//
//    float threshold;
//    float minHitRate, maxFalseAlarm;
//};










#endif