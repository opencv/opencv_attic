#ifndef BOOST_H
#define BOOST_H

#include "features.h"
#include "ml.h"

struct CvCascadeBoostParams : CvBoostParams
{
    float minHitRate;
    float maxFalseAlarm;
    
    CvCascadeBoostParams() : minHitRate( 0.995F), maxFalseAlarm( 0.5F )
    {  boost_type = CvBoost::GENTLE; }
    CvCascadeBoostParams( int _boostType,
        float _minHitRate, float _maxFalseAlarm,
        double _weightTrimRate, int _maxDepth, int _maxWeakCount, const float* priors = 0 );
    virtual ~CvCascadeBoostParams() {}
    void write( CvFileStorage* fs ) const;
    bool read( CvFileStorage* fs, CvFileNode* node );

    virtual void printDefault();
    virtual void printAttrs();
    virtual bool scanAttr( const char* prmName, const char* val);
};


struct CvCascadeBoostTrainData : CvDTreeTrainData
{
    CvCascadeBoostTrainData();
    CvCascadeBoostTrainData( CvCascadeData* _cascadeData );
    CvCascadeBoostTrainData( CvCascadeData* _cascadeData,
        int _numPrecalcVal, int _numPrecalcIdx, const CvDTreeParams& _params = CvDTreeParams() );
    virtual ~CvCascadeBoostTrainData();

    virtual void set_data( CvCascadeData* _cascadeData,
        int _numPrecalcVal, int _numPrecalcIdx, const CvDTreeParams& _params=CvDTreeParams(),
        bool _updateData=false );

    virtual void get_class_labels( CvDTreeNode* n, int* labelsBuf, const int** labels );
    virtual void get_cv_labels( CvDTreeNode* n, int* labelsBuf, const int** labels );
    virtual void get_sample_indices( CvDTreeNode* n, int* indicesBuf, const int** labels );
    
    virtual int get_ord_var_data( CvDTreeNode* n, int vi, float* ordValuesBuf, int* indicesBuf,
        const float** ordValues, const int** indices );
    virtual int get_cat_var_data( CvDTreeNode* n, int vi, int* catValuesBuf, const int** catValues );

    virtual float getVarValue( int vi, int si );

    virtual void clear();
    virtual void free_train_data();

    void precalculate();

    CvCascadeData* cascadeData;

    CvMat* valCache;    /* precalculated feature values (CV_32FC1) */
    int numPrecalcVal, numPrecalcIdx;
};

class CvCascadeBoostTree : public CvBoostTree
{
public:
    virtual CvDTreeNode* predict( int sampleIdx ) const;
    virtual void write( CvFileStorage* fs, const CvMat* featureMap );
    virtual void read( CvFileStorage* fs, CvFileNode* node, CvBoost* _ensemble,
        CvDTreeTrainData* _data );
    void markFeaturesInMap( CvMat* featureMap );
protected:
    void auxMarkFeaturesInMap( const CvDTreeNode* node, CvMat* featureMap );
    virtual void split_node_data( CvDTreeNode* n );
};

class CvCascadeBoost : public CvBoost
{
public:
    CvCascadeBoost();
    CvCascadeBoost( CvCascadeData* _cascadeData,
                    int _numPrecalcVal, int _numPrecalcIdx,
                    CvCascadeBoostParams _params=CvCascadeBoostParams() );
    virtual bool train( CvCascadeData* _cascadeData,
                    int _numPrecalcVal, int _numPrecalcIdx,
                    CvCascadeBoostParams _params=CvCascadeBoostParams(),
                    bool _update=false );

    virtual float predict( int sampleIdx, bool returnSum = false ) const;

    const CvCascadeBoostTrainData* get_data() const;
    float getThreshold() { return threshold; }; 

    virtual void write( CvFileStorage* fs, const CvMat* featureMap );
    virtual bool read( CvFileStorage* fs, CvFileNode* node, CvCascadeData* _cascadeData,
        CvCascadeBoostParams* _params );

    void markFeaturesInMap( CvMat* featureMap);
protected:
    virtual bool set_params( const CvBoostParams& _params );
    virtual void update_weights( CvBoostTree* tree );
    virtual bool isErrDesired();

    float threshold;
    float minHitRate, maxFalseAlarm;
};


#endif