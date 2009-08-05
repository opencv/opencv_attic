#include "boost.h"
#include "_inner_functions.h"
#include "cascadeclassifier.h"
#include <queue>
using namespace std;


#define CV_CMP_FLT(i,j) (i < j)
//static CV_IMPLEMENT_QSORT_EX( icvSortInt, int, CV_CMP_FLT, const int* )
static CV_IMPLEMENT_QSORT_EX( icvSortFlt, float, CV_CMP_FLT, const float* )

#define CV_CMP_NUM_IDX(i,j) (aux[i] < aux[j])
static CV_IMPLEMENT_QSORT_EX( icvSortIntAux, int, CV_CMP_NUM_IDX, const float* )
static CV_IMPLEMENT_QSORT_EX( icvSortUShAux, unsigned short, CV_CMP_NUM_IDX, const float* )

#define CV_CMP_PAIRS(a,b) (*((a).i) < *((b).i))
//static CV_IMPLEMENT_QSORT_EX( icvSortPairs, CvPair16u32s, CV_CMP_PAIRS, int )

#define CV_CMP_NUM_PTR(a,b) (*(a) < *(b))
//static CV_IMPLEMENT_QSORT_EX( icvSortIntPtr, int*, CV_CMP_NUM_PTR, int )
//static CV_IMPLEMENT_QSORT_EX( icvSortDblPtr, double*, CV_CMP_NUM_PTR, int )

#define CV_THRESHOLD_EPS (0.00001F)

static const int MinBlockSize = 1 << 16;
static const int BlockSizeDelta = 1 << 10;

//----------------------------- CascadeBoostParams -------------------------------------------------

CvCascadeBoostParams::CvCascadeBoostParams( int _boostType,
        float _minHitRate, float _maxFalseAlarm,
        double _weightTrimRate, int _maxDepth, int _maxWeakCount, const float* priors ) :
    CvBoostParams( _boostType, _maxWeakCount, _weightTrimRate, _maxDepth, false, priors )
{
    boost_type = CvBoost::GENTLE;
    minHitRate = _minHitRate;
    maxFalseAlarm = _maxFalseAlarm;
}

void CvCascadeBoostParams::write( CvFileStorage* fs ) const
{
    CV_FUNCNAME( "CvCascadeBoostParams::write" );
    __BEGIN__;

    const char* boostTypeStr;    
    boostTypeStr = boost_type == CvBoost::DISCRETE ? CC_DISCRETE_BOOST : 
        boost_type == CvBoost::REAL ? CC_REAL_BOOST :
        boost_type == CvBoost::LOGIT ? CC_LOGIT_BOOST :
        boost_type == CvBoost::GENTLE ? CC_GENTLE_BOOST : 0;
    if( boostTypeStr )
    {
        CV_CALL( cvWriteString( fs, CC_BOOST_TYPE, boostTypeStr ) );
    }
    else
    {
        CV_CALL( cvWriteInt( fs, CC_BOOST_TYPE, boost_type ) );
    }

    CV_CALL( cvWriteReal( fs, CC_MINHITRATE, minHitRate ) );
    CV_CALL( cvWriteReal( fs, CC_MAXFALSEALARM, maxFalseAlarm ) );
    CV_CALL( cvWriteReal( fs, CC_TRIM_RATE, weight_trim_rate ) );
    CV_CALL( cvWriteInt( fs, CC_MAX_DEPTH, max_depth ) );
    CV_CALL( cvWriteInt( fs, CC_WEAK_COUNT, weak_count ) );

    __END__;
}

bool CvCascadeBoostParams::read( CvFileStorage* fs, CvFileNode* map )
{
    bool res = false;

    CV_FUNCNAME( "CvCascadeBoostParams::read" );
    __BEGIN__;

    const char* boostTypeStr;
    CV_CALL( boostTypeStr = cvReadStringByName( fs, map, CC_BOOST_TYPE ) );
    if ( !boostTypeStr )
        EXIT;
    CV_CALL( boost_type = strcmp( boostTypeStr, CC_DISCRETE_BOOST ) == 0 ? CvBoost::DISCRETE :
        strcmp( boostTypeStr, CC_REAL_BOOST ) == 0 ? CvBoost::REAL :
        strcmp( boostTypeStr, CC_LOGIT_BOOST ) == 0 ? CvBoost::LOGIT :
        strcmp( boostTypeStr, CC_GENTLE_BOOST ) == 0 ? CvBoost::GENTLE : cvReadIntByName( fs, map, CC_BOOST_TYPE ) );
    CV_CALL( minHitRate = (float)cvReadRealByName( fs, map, CC_MINHITRATE ) );
    CV_CALL( maxFalseAlarm = (float)cvReadRealByName( fs, map, CC_MAXFALSEALARM ) );
    CV_CALL( weight_trim_rate = cvReadRealByName( fs, map, CC_TRIM_RATE ) );
    CV_CALL( max_depth = cvReadIntByName( fs, map, CC_MAX_DEPTH ) );
    CV_CALL( weak_count = cvReadIntByName( fs, map, CC_WEAK_COUNT ) );
    if ( minHitRate <= 0 || minHitRate > 1 ||
         maxFalseAlarm <= 0 || maxFalseAlarm > 1 ||
         weight_trim_rate <= 0 || weight_trim_rate > 1 ||
         max_depth <= 0 ||
         weak_count <= 0)
        EXIT;
    
    res = true;

    __END__;

    return res;
}

void CvCascadeBoostParams::printDefault()
{
    printf( "  [-bt <{%s, %s, %s, %s (default)}>]\n"
        "  [-minHitRate <min_hit_rate> = %f]\n"
        "  [-maxFalseAlarmRate <max_false_alarm_rate = %f>]\n"
        "  [-weightTrimRate <weight_trim_rate = %f>]\n"
        "  [-maxDepth <max_depth_of_weak_tree = %d>]\n"
        "  [-maxWeakCount <max_weak_tree_count = %d>]\n",
        CC_DISCRETE_BOOST, CC_REAL_BOOST, CC_LOGIT_BOOST, CC_GENTLE_BOOST, 
        minHitRate, maxFalseAlarm, weight_trim_rate, max_depth, weak_count );
}

void CvCascadeBoostParams::printAttrs()
{
    const char* boostTypeStr;    
    boostTypeStr = boost_type == CvBoost::DISCRETE ? CC_DISCRETE_BOOST : 
        boost_type == CvBoost::REAL ? CC_REAL_BOOST :
        boost_type == CvBoost::LOGIT  ? CC_LOGIT_BOOST :
        boost_type == CvBoost::GENTLE ? CC_GENTLE_BOOST : 0;
    printf( "boostType: %s\n", boostTypeStr );
    printf( "minHitRate: %f\n", minHitRate );
    printf( "maxFalseAlarmRate: %f\n", maxFalseAlarm );
    printf( "weightTrimRate: %f\n", weight_trim_rate );
    printf( "maxTreeDepth: %d\n", max_depth );
    printf( "maxWeakCount: %d\n", weak_count );
}

bool CvCascadeBoostParams::scanAttr( const char* prmName, const char* val)
{
    bool res = true;

    if( !strcmp( prmName, "-bt" ) )
    {
        boost_type = !strcmp( val, CC_DISCRETE_BOOST ) ? CvBoost::DISCRETE :
            !strcmp( val, CC_REAL_BOOST ) ? CvBoost::REAL :
            !strcmp( val, CC_LOGIT_BOOST ) ? CvBoost::LOGIT :
            !strcmp( val, CC_GENTLE_BOOST ) ? CvBoost::GENTLE : -1;
        if (boost_type == -1)
            res = false;
    }
    else if( !strcmp( prmName, "-minHitRate" ) )
    {
        minHitRate = (float) atof( val );
    }
    else if( !strcmp( prmName, "-maxFalseAlarmRate" ) )
    {
        weight_trim_rate = (float) atof( val );
    }
    else if( !strcmp( prmName, "-weightTrimRate" ) )
    {
        weight_trim_rate = (float) atof( val );
    }
    else if( !strcmp( prmName, "-maxDepth" ) )
    {
        max_depth = atoi( val );
    }
    else if( !strcmp( prmName, "-maxWeakCount" ) )
    {
        weak_count = atoi( val );
    }
    else
        res = false;

    return res;        
}

//---------------------------- CascadeBoostTrainData -----------------------------

CvCascadeBoostTrainData::CvCascadeBoostTrainData()
{
    valCache = 0;
    cascadeData = 0;
    clear();
}

CvCascadeBoostTrainData::CvCascadeBoostTrainData( CvCascadeData* _cascadeData )
{
    CV_FUNCNAME( "CvCascadeBoostTrainData::CvCascadeBoostTrainData" );
    __BEGIN__;

    int maxSplitSize, treeBlockSize;

    is_classifier = true;
    var_all = var_count = _cascadeData->getNumFeatures();

    cascadeData = _cascadeData;
    shared = true;
    valCache = 0;

    max_c_count = MAX( 2, cascadeData->getMaxCatCount() );
    assert( max_c_count >= 2 );

    CV_CALL( var_type = cvCreateMat( 1, var_count + 2, CV_32SC1 ));

    if ( cascadeData->getMaxCatCount() > 0 ) 
    {
        numPrecalcIdx = 0;
        cat_var_count = var_count;
        ord_var_count = 0;
        for( int vi = 0; vi < var_count; vi++ )
        {
            var_type->data.i[vi] = vi;
        }    
    }
    else
    {
        cat_var_count = 0;
        ord_var_count = var_count;
        for( int vi = 1; vi <= var_count; vi++ )
        {
            var_type->data.i[vi-1] = -vi;
        }        
    }    
    var_type->data.i[var_count] = cat_var_count;
    var_type->data.i[var_count+1] = cat_var_count+1;
    
    max_c_count = MAX( 2, cascadeData->getMaxCatCount() );

    maxSplitSize = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,max_c_count - 33)/32)*sizeof(int),sizeof(void*));

    treeBlockSize = MAX((int)sizeof(CvDTreeNode)*8, maxSplitSize);
    treeBlockSize = MAX(treeBlockSize + BlockSizeDelta, MinBlockSize);
    CV_CALL( tree_storage = cvCreateMemStorage( treeBlockSize ));
    CV_CALL( node_heap = cvCreateSet( 0, sizeof(node_heap[0]),
            sizeof(CvDTreeNode), tree_storage ));
    CV_CALL( split_heap = cvCreateSet( 0, sizeof(split_heap[0]),
            maxSplitSize, tree_storage ));  
    __END__;
}

CvCascadeBoostTrainData::CvCascadeBoostTrainData( CvCascadeData* _cascadeData,
                                             int _numPrecalcVal, int _numPrecalcIdx, 
                                             const CvDTreeParams& _params )
{
    valCache = 0;
    set_data( _cascadeData, _numPrecalcVal, _numPrecalcIdx, _params );
}
 
CvCascadeBoostTrainData::~CvCascadeBoostTrainData()
{
    clear();
}

void CvCascadeBoostTrainData::set_data( CvCascadeData* _cascadeData,
                                      int _numPrecalcVal, int _numPrecalcIdx,
									  const CvDTreeParams& _params,
                                      bool _updateData )
{    

    CvCascadeBoostTrainData *data = 0;

    CV_FUNCNAME( "CvCascadeBoostTrainData::set_data" );
    __BEGIN__;

    int* idst = 0;
    unsigned short* udst = 0;
        
    //int totalCatCount = 0;
    int treeBlockSize, tempBlockSize, maxSplitSize, nvSize, size;
    int step = 0;
     
    if( _updateData && data_root )
    {
        CV_ERROR( CV_StsNotImplemented, "data update is not supported" );
    }
    clear();

    shared = true;
    have_labels = true;

    rng = cvRNG(-1);

    CV_CALL( set_params( _params ));

    assert( _cascadeData );

    cascadeData = _cascadeData;

    responses = cascadeData->getCls();
    // TODO: check responses: elements must be 0 or 1
    
	if( _numPrecalcVal < 0 || _numPrecalcIdx < 0)
    {
        CV_ERROR( CV_StsOutOfRange, "_numPrecalcVal <= 0 and _numPrecalcIdx must be positive or 0" );
	}
	
	var_count = var_all = cascadeData->getNumFeatures();
    sample_count = cascadeData->getNumSamples();
	
	numPrecalcVal = MIN( _numPrecalcVal, var_count );
	numPrecalcIdx = MIN( _numPrecalcIdx, var_count );
    
    is_buf_16u = false;     
    if (sample_count < 65536) 
        is_buf_16u = true;                               

    CV_CALL( valCache = cvCreateMat( numPrecalcVal ? numPrecalcVal : 1, sample_count, CV_32FC1 ) );
    
    CV_CALL( var_type = cvCreateMat( 1, var_count + 2, CV_32SC1 ));
    
    is_classifier = true;

    if ( cascadeData->getMaxCatCount() > 0 ) 
    {
        numPrecalcIdx = 0;
        cat_var_count = var_count;
        ord_var_count = 0;
        for( int vi = 0; vi < var_count; vi++ )
        {
            var_type->data.i[vi] = vi;
        }    
    }
    else
    {
        cat_var_count = 0;
        ord_var_count = var_count;
        for( int vi = 1; vi <= var_count; vi++ )
        {
            var_type->data.i[vi-1] = -vi;
        }        
    }
    var_type->data.i[var_count] = cat_var_count;
    var_type->data.i[var_count+1] = cat_var_count+1;
    
    work_var_count = ( cat_var_count ? var_count : numPrecalcIdx ) + 1;
    buf_size = (work_var_count + 1) * sample_count;
    buf_count = 2;
    
    if ( is_buf_16u )
    {
        CV_CALL( buf = cvCreateMat( buf_count, buf_size, CV_16UC1 ));
    }
    else
    {
        CV_CALL( buf = cvCreateMat( buf_count, buf_size, CV_32SC1 ));
    }

    size = cat_var_count + 1;
    CV_CALL( cat_count = cvCreateMat( 1, size, CV_32SC1 ));
        
    CV_CALL( pred_float_buf = (float*)cvAlloc(sample_count*sizeof(pred_float_buf[0])) );
    CV_CALL( pred_int_buf = (int*)cvAlloc(sample_count*sizeof(pred_int_buf[0])) );
    CV_CALL( resp_float_buf = (float*)cvAlloc(sample_count*sizeof(resp_float_buf[0])) );
    CV_CALL( resp_int_buf = (int*)cvAlloc(sample_count*sizeof(resp_int_buf[0])) );
    CV_CALL( cv_lables_buf = (int*)cvAlloc(sample_count*sizeof(cv_lables_buf[0])) );
    CV_CALL( sample_idx_buf = (int*)cvAlloc(sample_count*sizeof(sample_idx_buf[0])) );

    // precalculate valCache and set indices in buf
	precalculate();

    // now calculate the maximum size of split,
    // create memory storage that will keep nodes and splits of the decision tree
    // allocate root node and the buffer for the whole training data
    maxSplitSize = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,sample_count - 33)/32)*sizeof(int),sizeof(void*));
    treeBlockSize = MAX((int)sizeof(CvDTreeNode)*8, maxSplitSize);
    treeBlockSize = MAX(treeBlockSize + BlockSizeDelta, MinBlockSize);
    CV_CALL( tree_storage = cvCreateMemStorage( treeBlockSize ));
    CV_CALL( node_heap = cvCreateSet( 0, sizeof(*node_heap), sizeof(CvDTreeNode), tree_storage ));

    nvSize = var_count*sizeof(int);
    nvSize = cvAlign(MAX( nvSize, (int)sizeof(CvSetElem) ), sizeof(void*));
    tempBlockSize = nvSize;
    tempBlockSize = MAX( tempBlockSize + BlockSizeDelta, MinBlockSize );
    CV_CALL( temp_storage = cvCreateMemStorage( tempBlockSize ));
    CV_CALL( nv_heap = cvCreateSet( 0, sizeof(*nv_heap), nvSize, temp_storage ));
    
    CV_CALL( data_root = new_node( 0, sample_count, 0, 0 ));

    // set sample labels
    if (is_buf_16u)
        udst = (unsigned short*)(buf->data.s + work_var_count*sample_count);
    else
        idst = buf->data.i + work_var_count*sample_count;

    for (int si = 0; si < sample_count; si++)
    {
        if (udst)
            udst[si] = (unsigned short)si;
        else
            idst[si] = si;
    }

    max_c_count = MAX( 2, cascadeData->getMaxCatCount() );
    assert( max_c_count >= 2 );

    step = valCache->step / CV_ELEM_SIZE(valCache->type);
    for( int vi = 0; vi < var_count; vi++ )
    {
        data_root->set_num_valid(vi, sample_count);
    }
    for( int vi = 0; vi < cat_var_count; vi++ )
    {
        cat_count->data.i[vi] = max_c_count;
    }
    cat_count->data.i[cat_var_count] = 2;

    maxSplitSize = cvAlign(sizeof(CvDTreeSplit) +
        (MAX(0,max_c_count - 33)/32)*sizeof(int),sizeof(void*));
    CV_CALL( split_heap = cvCreateSet( 0, sizeof(*split_heap), maxSplitSize, tree_storage ));

    have_priors = is_classifier && params.priors;
    if( is_classifier ) // is_classifier == true
    {
        int m = get_num_classes();
        double sum = 0;
        CV_CALL( priors = cvCreateMat( 1, m, CV_64F ));
        for( int i = 0; i < m; i++ )
        {
            double val = have_priors ? params.priors[i] : 1.;
            if( val <= 0 )
                CV_ERROR( CV_StsOutOfRange, "Every class weight should be positive" );
            priors->data.db[i] = val;
            sum += val;
        }

        // normalize weights
        if( have_priors )
            cvScale( priors, priors, 1./sum );

        CV_CALL( priors_mult = cvCloneMat( priors ));
        CV_CALL( counts = cvCreateMat( 1, m, CV_32SC1 ));
    }

    CV_CALL( direction = cvCreateMat( 1, sample_count, CV_8UC1 ));
    CV_CALL( split_buf = cvCreateMat( 1, sample_count, CV_32SC1 ));

    __END__;

    if( data )
        delete data;
}

void CvCascadeBoostTrainData::clear()
{
    CvDTreeTrainData::clear();    
    cvReleaseMat( &valCache );
    cascadeData = 0;
    numPrecalcVal = numPrecalcIdx = 0;
}

void CvCascadeBoostTrainData::get_class_labels( CvDTreeNode* n, int* labelsBuf, const int** labels )
{
    int nodeSampleCount = n->sample_count; 
    int* sampleIndicesBuf = sample_idx_buf;
    const int* sampleIndices = 0;
    int rStep = CV_IS_MAT_CONT( responses->type ) ?
                1 : responses->step / CV_ELEM_SIZE( responses->type );

    get_sample_indices(n, sampleIndicesBuf, &sampleIndices);

    for( int si = 0; si < nodeSampleCount; si++ )
    {
        int sidx = sampleIndices[si];
        labelsBuf[si] = (int)responses->data.fl[sidx*rStep];
    }    
    *labels = labelsBuf;
}

void CvCascadeBoostTrainData::get_sample_indices( CvDTreeNode* n, int* indicesBuf, const int** indices )
{
    CvDTreeTrainData::get_cat_var_data( n, get_work_var_count(), indicesBuf, indices );
}

void CvCascadeBoostTrainData::get_cv_labels( CvDTreeNode* n, int* labels_buf, const int** labels )
{
    if (have_labels)
        CvDTreeTrainData::get_cat_var_data( n, get_work_var_count()- 1, labels_buf, labels );
}

int CvCascadeBoostTrainData::get_ord_var_data( CvDTreeNode* n, int vi, float* ordValuesBuf, int* indicesBuf,
        const float** ordValues, const int** indices )
{
	int valStep = valCache->step / CV_ELEM_SIZE(valCache->type);
    int nodeSampleCount = n->sample_count; 
    int* sampleIndicesBuf = sample_idx_buf;
    const int* sampleIndices = 0;
    get_sample_indices(n, sampleIndicesBuf, &sampleIndices);
    
	if ( vi < numPrecalcIdx )
	{
		if( !is_buf_16u )
	        *indices = buf->data.i + n->buf_idx*buf->cols + 
				vi*sample_count + n->offset;
	    else {
	        const unsigned short* shortIndices = (const unsigned short*)(buf->data.s + n->buf_idx*buf->cols + 
	            vi*sample_count + n->offset );
	        for( int i = 0; i < nodeSampleCount; i++ )
	            indicesBuf[i] = shortIndices[i];
	        *indices = indicesBuf;
	    }
		
		if ( vi < numPrecalcVal )
		{
			for( int i = 0; i < nodeSampleCount; i++ )
	        {
	            int idx = (*indices)[i];
	            idx = sampleIndices[idx];
	            ordValuesBuf[i] =  *(valCache->data.fl + vi * valStep + idx);
	        }
		}
		else
		{
			for( int i = 0; i < nodeSampleCount; i++ )
	        {
	            int idx = (*indices)[i];
	            idx = sampleIndices[idx];
				ordValuesBuf[i] = cascadeData->calcFeature( vi, idx);
			}
		}
    }
	else // vi >= numPrecalcIdx
	{
		// use sample_indices as temporary buffer for values
		if ( vi < numPrecalcVal )
		{
			for( int i = 0; i < nodeSampleCount; i++ )
	        {
	            int idx = sampleIndices[i];
				indicesBuf[i] = i;
	            ((float*)sampleIndices)[i] = *(valCache->data.fl + vi * valStep + idx);
	        }
		}
		else
		{
			for( int i = 0; i < nodeSampleCount; i++ )
	        {
	            int idx = sampleIndices[i];
				indicesBuf[i] = i;
				((float*)sampleIndices)[i] = cascadeData->calcFeature( vi, idx);
			}
		}
		icvSortIntAux( indicesBuf, sample_count, (float *)sampleIndices );
		for( int i = 0; i < nodeSampleCount; i++ )
	    {
			int idx = indicesBuf[i];
	        ordValuesBuf[i] = ((float*)sampleIndices)[idx];
		}
		*indices = indicesBuf;
	}
	
    *ordValues = ordValuesBuf;
    return 0;
}
 
int CvCascadeBoostTrainData::get_cat_var_data( CvDTreeNode* n, int vi, int* catValuesBuf, const int** catValues )
{
	int valStep = valCache->step / CV_ELEM_SIZE(valCache->type);
    int nodeSampleCount = n->sample_count; 
    int* sampleIndicesBuf = sample_idx_buf;
    const int* sampleIndices = 0;
    get_sample_indices(n, sampleIndicesBuf, &sampleIndices);

    if ( vi < numPrecalcVal )
	{
		for( int i = 0; i < nodeSampleCount; i++ )
        {
            int idx = sampleIndices[i];
            catValuesBuf[i] =  (int)*(valCache->data.fl + vi * valStep + idx);
        }
	}
	else
	{
		for( int i = 0; i < nodeSampleCount; i++ )
        {
            int idx = sampleIndices[i];
			catValuesBuf[i] = (int)cascadeData->calcFeature( vi, idx);
		}
	}
    *catValues = catValuesBuf;
    
    return 0; //TODO: return the number of non-missing values
}

float CvCascadeBoostTrainData::getVarValue( int vi, int si )
{
    if ( vi < numPrecalcVal && valCache )
    {
        int val_step = valCache->step / CV_ELEM_SIZE(valCache->type);
	    return *(valCache->data.fl + vi * val_step + si);
    }
	return cascadeData->calcFeature( vi, si);
}

void CvCascadeBoostTrainData::free_train_data()
{
    CvDTreeTrainData::free_train_data();

    cvReleaseMat( &valCache );
}

void CvCascadeBoostTrainData::precalculate()
{
    //CV_FUNCNAME( "CvCascadeBoostTrainData::precalculate" );
    __BEGIN__;

    double proctime;
    int minNum = MIN( numPrecalcVal, numPrecalcIdx);
    unsigned short* udst = (unsigned short*)buf->data.s;
    int* idst = buf->data.i;
    int valStep = valCache ? valCache->step / CV_ELEM_SIZE(valCache->type) : 0;
    
    assert( valCache );

    proctime = -TIME( 0 );
	
    for ( int fi = numPrecalcVal; fi < numPrecalcIdx; fi++)
    {
        for( int si = 0; si < sample_count; si++ )
        {
            float val = cascadeData->calcFeature( fi, si );
            CV_MAT_ELEM( *valCache, float, 0, si ) = val;
            if ( is_buf_16u )
                *(udst + fi*sample_count + si) = si;
            else
                *(idst + fi*sample_count + si) = si;
        }
        if ( is_buf_16u )
            icvSortUShAux( udst + fi*sample_count, sample_count, 
            valCache->data.fl );
        else
            icvSortIntAux( idst + fi*sample_count, sample_count,
            valCache->data.fl );
    }

    for ( int fi = 0; fi < minNum; fi++)
    {
        for( int si = 0; si < sample_count; si++ )
        {
            float val = cascadeData->calcFeature( fi, si );
            CV_MAT_ELEM( *valCache, float, fi, si ) = val;
            if ( is_buf_16u )
                *(udst + fi*sample_count + si) = si;
            else
                *(idst + fi*sample_count + si) = si;
        }
        if ( is_buf_16u )
            icvSortUShAux( udst + fi*sample_count, sample_count, 
            valCache->data.fl + fi*valStep);
        else
            icvSortIntAux( idst + fi*sample_count, sample_count,
            valCache->data.fl + fi*valStep );
    }

    for ( int fi = minNum; fi < numPrecalcVal; fi++)
    {
        for( int si = 0; si < sample_count; si++ )
        {
            float val = cascadeData->calcFeature( fi, si );
            CV_MAT_ELEM( *valCache, float, fi, si ) = val;
        }
    }
    
    printf( "Precalculation time: %.2f\n", (proctime + TIME( 0 )) );
    __END__;
}

//-------------------------------- CascadeBoostTree ----------------------------------------

CvDTreeNode* CvCascadeBoostTree::predict( int sampleIdx ) const
{
    CvDTreeNode* result = 0;
    
    CV_FUNCNAME( "CvCascadeBoostTree::predict" );
    __BEGIN__;

    CvDTreeNode* node = root;

    if( !node )
        CV_ERROR( CV_StsError, "The tree has not been trained yet" );

    if ( ((CvCascadeBoostTrainData*)data)->cascadeData->getMaxCatCount() == 0 ) // ordered
    {
       while( node->left )
        {
            CvDTreeSplit* split = node->split;
            int vi = split->var_idx;
            float val = ((CvCascadeBoostTrainData*)data)->getVarValue( vi, sampleIdx );
            int dir = val <= split->ord.c ? -1 : 1;
            node = dir < 0 ? node->left : node->right;
        }
    }
    else // categorical
    {
        while( node->left )
        {
            CvDTreeSplit* split = node->split;
            int vi = split->var_idx;
            int c = (int)((CvCascadeBoostTrainData*)data)->getVarValue( vi, sampleIdx );
            int dir = CV_DTREE_CAT_DIR(c, split->subset);
            node = dir < 0 ? node->left : node->right;
        }
    }
    result = node;

    __END__;

    return result;
}

void CvCascadeBoostTree::write( CvFileStorage* fs, const CvMat* featureMap )
{
    CV_FUNCNAME( "CvCascadeBoostTree::write" );
    __BEGIN__;

    int maxCatCount = ((CvCascadeBoostTrainData*)data)->cascadeData->getMaxCatCount();
    int subsetN = (maxCatCount + 31)/32;
    queue<CvDTreeNode*> internalNodesQueue;
    float* leafVals = (float *)cvAlloc( (int)pow(2.f, (float)ensemble->get_data()->params.max_depth) *
        sizeof(leafVals[0]) );
    int leafValIdx = 0;
    int internalNodeIdx = 1;
    CvDTreeNode* tempNode;

    assert( root );
    internalNodesQueue.push( root );

    CV_CALL( cvStartWriteStruct( fs, 0, CV_NODE_MAP ) );

    CV_CALL( cvStartWriteStruct( fs, CC_INTERNAL_NODES, CV_NODE_SEQ | CV_NODE_FLOW ) );
    while (!internalNodesQueue.empty())
    {
        int fidx;
        tempNode = internalNodesQueue.front();
        
        assert ( tempNode->left );
        if ( !tempNode->left->left && !tempNode->left->right) // left node is leaf
        {
            leafVals[-leafValIdx] = (float)tempNode->left->value;
            CV_CALL( cvWriteInt( fs, NULL, leafValIdx-- ) );
        }
        else
        {
            internalNodesQueue.push( tempNode->left );
            CV_CALL( cvWriteInt( fs, NULL, internalNodeIdx++ ) );
        }

        assert( tempNode->right );
        if ( !tempNode->right->left && !tempNode->right->right) // right node is leaf
        {
            leafVals[-leafValIdx] = (float)tempNode->right->value;
            CV_CALL( cvWriteInt( fs, NULL, leafValIdx-- ) );
        }
        else
        {
            internalNodesQueue.push( tempNode->right );
            CV_CALL( cvWriteInt( fs, NULL, internalNodeIdx++ ) );
        }
        fidx = tempNode->split->var_idx;
        fidx = featureMap ? featureMap->data.i[fidx] : fidx;
        CV_CALL( cvWriteInt( fs, NULL, fidx ) );
        if ( !maxCatCount )
        {
            CV_CALL( cvWriteReal( fs, NULL, tempNode->split->ord.c ) );
        }
        else
        {
            for( int i = 0; i < subsetN; i++ )
                CV_CALL( cvWriteInt( fs, NULL, tempNode->split->subset[i]) );
        }

        internalNodesQueue.pop();
    }
    CV_CALL( cvEndWriteStruct( fs ) ); //internalNodes

    CV_CALL( cvStartWriteStruct( fs, CC_LEAF_VALUES, CV_NODE_SEQ | CV_NODE_FLOW ) );
    for (int ni = 0; ni < -leafValIdx; ni++)
        CV_CALL( cvWriteReal( fs, NULL, leafVals[ni] ) );
    CV_CALL( cvEndWriteStruct( fs ) ); //leafValsues

    CV_CALL( cvEndWriteStruct( fs ) );
    cvFree( &leafVals );
    __END__;
}

void CvCascadeBoostTree::read( CvFileStorage* fs, CvFileNode* node, CvBoost* _ensemble,
                                CvDTreeTrainData* _data )
{
    CV_FUNCNAME( "CvCascadeBoostTree::read" );
    __BEGIN__;

    int maxCatCount = ((CvCascadeBoostTrainData*)_data)->cascadeData->getMaxCatCount();
    int subsetN = (maxCatCount + 31)/32;
    int step = 3 + ( maxCatCount>0 ? subsetN : 1 );
    
    queue<CvDTreeNode*> internalNodesQueue;

    CvSeq* internalNodes, *leafValsues;
    CvDTreeNode* prntNode, *cldNode;
    int intIdx, leafIdx;

    clear();
    data = _data;
    ensemble = _ensemble;
    pruned_tree_idx = 0;

    // read tree nodes
    CV_CALL( internalNodes = ((CvFileNode*)cvGetFileNodeByName( fs, node, CC_INTERNAL_NODES ))->data.seq );
    CV_CALL( leafValsues = ((CvFileNode*)cvGetFileNodeByName( fs, node, CC_LEAF_VALUES ))->data.seq );
    
    intIdx = internalNodes->total;
    leafIdx = leafValsues->total;
    for( int i = 0; i < internalNodes->total/step; i++)
    {
        prntNode = data->new_node( 0, 0, 0, 0 );
        if ( maxCatCount > 0 )
        {
            prntNode->split = data->new_split_cat( 0, 0 );
            for( int j = subsetN-1; j>=0; j--)
            {
                int c = ((CvFileNode*)cvGetSeqElem( internalNodes, --intIdx ))->data.i;
                prntNode->split->subset[j] = c;
            }
        }
        else
        {
            float split_value = (float)((CvFileNode*)cvGetSeqElem( internalNodes, --intIdx ))->data.f;
            prntNode->split = data->new_split_ord( 0, split_value, 0, 0, 0);
        }
        int split_var_idx = ((CvFileNode*)cvGetSeqElem( internalNodes, --intIdx ))->data.i;
        prntNode->split->var_idx = split_var_idx;
        int ridx = ((CvFileNode*)cvGetSeqElem( internalNodes, --intIdx ))->data.i;
        int lidx = ((CvFileNode*)cvGetSeqElem( internalNodes, --intIdx ))->data.i;
                
        if ( ridx <= 0)
        {
            float leafValsue = (float)((CvFileNode*)cvGetSeqElem( leafValsues, --leafIdx ))->data.f;
            prntNode->right = cldNode = data->new_node( 0, 0, 0, 0 );
            cldNode->value = leafValsue;
            cldNode->parent = prntNode;            
        }
        else
        {
            prntNode->right = internalNodesQueue.front(); 
            prntNode->right->parent = prntNode;
            internalNodesQueue.pop();
        }

        if ( lidx <= 0)
        {
            float leafValsue = (float)((CvFileNode*)cvGetSeqElem( leafValsues, --leafIdx ))->data.f;
            prntNode->left = cldNode = data->new_node( 0, 0, 0, 0 );
            cldNode->value = leafValsue;
            cldNode->parent = prntNode;            
        }
        else
        {
            prntNode->left = internalNodesQueue.front();
            prntNode->left->parent = prntNode;
            internalNodesQueue.pop();
        }

        internalNodesQueue.push( prntNode );
    }

    root = internalNodesQueue.front();
    internalNodesQueue.pop();

    __END__;
}

void CvCascadeBoostTree::split_node_data( CvDTreeNode* node )
{
    int i, n = node->sample_count, nl, nr, scount = data->sample_count;
    char* dir = (char*)data->direction->data.ptr;
    CvDTreeNode *left = 0, *right = 0;
    int* newIdx = data->split_buf->data.i;
    int newBufIdx = data->get_child_buf_idx( node );
    int workVarCount = data->get_work_var_count();
    CvMat* buf = data->buf;
    int* tempBuf = (int*)cvStackAlloc(n*sizeof(tempBuf[0]));
    bool splitInputData;

    complete_node_dir(node);

    for( i = nl = nr = 0; i < n; i++ )
    {
        int d = dir[i];
        // initialize new indices for splitting ordered variables
        newIdx[i] = (nl & (d-1)) | (nr & -d); // d ? ri : li
        nr += d;
        nl += d^1;
    }

    node->left = left = data->new_node( node, nl, newBufIdx, node->offset );
    node->right = right = data->new_node( node, nr, newBufIdx, node->offset + nl );

    splitInputData = node->depth + 1 < data->params.max_depth &&
        (node->left->sample_count > data->params.min_sample_count ||
        node->right->sample_count > data->params.min_sample_count);

	// split ordered variables, keep both halves sorted.
    for( int vi = 0; vi < ((CvCascadeBoostTrainData*)data)->numPrecalcIdx; vi++ )
    {
        int ci = data->get_var_type(vi);
        int n1 = node->get_num_valid(vi);
        int *src_idx_buf = data->pred_int_buf;
        const int* src_idx = 0;
        float *src_val_buf = data->pred_float_buf;
        const float* src_val = 0;
        
        if( ci >= 0 || !splitInputData )
            continue;

        data->get_ord_var_data(node, vi, src_val_buf, src_idx_buf, &src_val, &src_idx);

        for(i = 0; i < n; i++)
            tempBuf[i] = src_idx[i];

        if (data->is_buf_16u)
        {
            unsigned short *ldst, *rdst, *ldst0, *rdst0;
            ldst0 = ldst = (unsigned short*)(buf->data.s + left->buf_idx*buf->cols + 
                vi*scount + left->offset);
            rdst0 = rdst = (unsigned short*)(ldst + nl);

            // split sorted
            for( i = 0; i < n1; i++ )
            {
                int idx = tempBuf[i];
                int d = dir[idx];
                idx = newIdx[idx];
                if (d)
                {
                    *rdst = (unsigned short)idx;
                    rdst++;
                }
                else
                {
                    *ldst = (unsigned short)idx;
                    ldst++;
                }
            }
            assert( n1 == n);

            left->set_num_valid(vi, (int)(ldst - ldst0));
            right->set_num_valid(vi, (int)(rdst - rdst0));
        }   
        else
        {
            int *ldst0, *ldst, *rdst0, *rdst;
            ldst0 = ldst = buf->data.i + left->buf_idx*buf->cols + 
                vi*scount + left->offset;
            rdst0 = rdst = buf->data.i + right->buf_idx*buf->cols + 
                vi*scount + right->offset;

            // split sorted
            for( i = 0; i < n1; i++ )
            {
                int idx = tempBuf[i];
                int d = dir[idx];
                idx = newIdx[idx];
                if (d)
                {
                    *rdst = idx;
                    rdst++;
                }
                else
                {
                    *ldst = idx;
                    ldst++;
                }
            }

            left->set_num_valid(vi, (int)(ldst - ldst0));
            right->set_num_valid(vi, (int)(rdst - rdst0));

            assert( n1 == n);
        }  
    }

    // split cv_labels using newIdx relocation table
    int *src_lbls_buf = data->pred_int_buf;
    const int* src_lbls = 0;
    data->get_cv_labels(node, src_lbls_buf, &src_lbls);

    for(i = 0; i < n; i++)
        tempBuf[i] = src_lbls[i];

    if (data->is_buf_16u)
    {
        unsigned short *ldst = (unsigned short *)(buf->data.s + left->buf_idx*buf->cols + 
            (workVarCount-1)*scount + left->offset);
        unsigned short *rdst = (unsigned short *)(buf->data.s + right->buf_idx*buf->cols + 
            (workVarCount-1)*scount + right->offset);            
        
        for( i = 0; i < n; i++ )
        {
            int d = dir[i];
            int idx = tempBuf[i];
            if (d)
            {
                *rdst = (unsigned short)idx;
                rdst++;
            }
            else
            {
                *ldst = (unsigned short)idx;
                ldst++;
            }
        }

    }
    else
    {
        int *ldst = buf->data.i + left->buf_idx*buf->cols + 
            (workVarCount-1)*scount + left->offset;
        int *rdst = buf->data.i + right->buf_idx*buf->cols + 
            (workVarCount-1)*scount + right->offset;
        
        for( i = 0; i < n; i++ )
        {
            int d = dir[i];
            int idx = tempBuf[i];
            if (d)
            {
                *rdst = idx;
                rdst++;
            }
            else
            {
                *ldst = idx;
                ldst++;
            }
            
        }
    }        
    for( int vi = 0; vi < data->var_count; vi++ )
    {
        left->set_num_valid(vi, (int)(nl));
        right->set_num_valid(vi, (int)(nr));
    }

    // split sample indices
    int *sampleIdx_src_buf = data->sample_idx_buf;
    const int* sampleIdx_src = 0;
    data->get_sample_indices(node, sampleIdx_src_buf, &sampleIdx_src);

    for(i = 0; i < n; i++)
        tempBuf[i] = sampleIdx_src[i];

    if (data->is_buf_16u)
    {
        unsigned short* ldst = (unsigned short*)(buf->data.s + left->buf_idx*buf->cols + 
            workVarCount*scount + left->offset);
        unsigned short* rdst = (unsigned short*)(buf->data.s + right->buf_idx*buf->cols + 
            workVarCount*scount + right->offset);
        for (i = 0; i < n; i++)
        {
            int d = dir[i];
            unsigned short idx = (unsigned short)tempBuf[i];
            if (d)
            {
                *rdst = idx;
                rdst++;
            }
            else
            {
                *ldst = idx;
                ldst++;
            }
        }
    }
    else
    {
        int* ldst = buf->data.i + left->buf_idx*buf->cols + 
            workVarCount*scount + left->offset;
        int* rdst = buf->data.i + right->buf_idx*buf->cols + 
            workVarCount*scount + right->offset;
        for (i = 0; i < n; i++)
        {
            int d = dir[i];
            int idx = tempBuf[i];
            if (d)
            {
                *rdst = idx;
                rdst++;
            }
            else
            {
                *ldst = idx;
                ldst++;
            }
        }
    }

    // deallocate the parent node data that is not needed anymore
    data->free_node_data(node); 
}

void CvCascadeBoostTree::markFeaturesInMap( CvMat* featureMap )
{
    auxMarkFeaturesInMap( root, featureMap );
}

void CvCascadeBoostTree::auxMarkFeaturesInMap( const CvDTreeNode* node, CvMat* featureMap)
{
    if ( node && node->split )
    {
        featureMap->data.i[node->split->var_idx] = 1;
        auxMarkFeaturesInMap( node->left, featureMap );
        auxMarkFeaturesInMap( node->right, featureMap );
    }
}
//----------------------------------- CascadeBoost --------------------------------------

CvCascadeBoost::CvCascadeBoost()
{
    data = 0;
    weak = 0;
    active_vars = active_vars_abs = orig_response = sum_response = weak_eval =
        subsample_mask = weights = subtree_weights = 0;
    have_active_cat_vars = have_subsample = false;
    threshold = -1;

    clear();
}

CvCascadeBoost::CvCascadeBoost( CvCascadeData* _cascadeData,
                    int _numPrecalcVal, int _numPrecalcIdx,
                    CvCascadeBoostParams _params )
{
    weak = 0;
    data = 0;
    orig_response = sum_response = weak_eval = subsample_mask = weights = 0;
    threshold = -1;

    train( _cascadeData, _numPrecalcVal, _numPrecalcIdx, _params );
}

bool CvCascadeBoost::set_params( const CvBoostParams& _params )
{
    minHitRate = ((CvCascadeBoostParams&)_params).minHitRate;
    maxFalseAlarm = ((CvCascadeBoostParams&)_params).maxFalseAlarm;
    return ( ( minHitRate > 0 ) && ( minHitRate < 1) &&
        ( maxFalseAlarm > 0 ) && ( maxFalseAlarm < 1) && 
        CvBoost::set_params( _params ));
}

bool CvCascadeBoost::train( CvCascadeData* _cascadeData,
                    int _numPrecalcVal, int _numPrecalcIdx,
                    CvCascadeBoostParams _params,
                    bool _update )
{
    bool ok = false;
    CvMemStorage* storage = 0;

    CV_FUNCNAME( "CvCascadeBoost::train" );
    __BEGIN__;

    set_params( _params );

    cvReleaseMat( &active_vars );
    cvReleaseMat( &active_vars_abs );

    if( !_update || !data )
    {
        clear();
        data = new CvCascadeBoostTrainData( _cascadeData, _numPrecalcVal, _numPrecalcIdx, _params );

        /*if( data->get_num_classes() != 2 )
            CV_ERROR( CV_StsNotImplemented,
            "Boosted trees can only be used for 2-class classification." );*/
        CV_CALL( storage = cvCreateMemStorage() );
        weak = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvBoostTree*), storage );
        storage = 0;
    }
    else
    {
        ((CvCascadeBoostTrainData*)data)->set_data( _cascadeData, _numPrecalcVal, _numPrecalcIdx, _params );
    }

    if ( (_params.boost_type == LOGIT) || (_params.boost_type == GENTLE) )
        data->do_responses_copy();
    
    update_weights( 0 );

    printf( "+----+---------+---------+\n" );
    printf( "|  N |    HR   |    FA   |\n" );
    printf( "+----+---------+---------+\n" );

    do
    {
        CvCascadeBoostTree* tree = new CvCascadeBoostTree;
        if( !tree->train( data, subsample_mask, this ) )
        {
            // TODO: may be should finish the loop (!!!)
            assert(0);
            delete tree;
            continue;
        }
        cvSeqPush( weak, &tree );
        update_weights( tree );
        trim_weights();
    }
    while( !isErrDesired() && (weak->total < params.weak_count) );

    get_active_vars(); 
    data->is_classifier = true;
    ok = true;

    data->free_train_data();

    __END__;

    return ok;
}

float CvCascadeBoost::predict( int sampleIdx, bool returnSum ) const
{
    //float* buf = 0;
    float value = -FLT_MAX;

    CV_FUNCNAME( "CvCascadeBoost::predict" );
    __BEGIN__;

    int i, varCount;
    CvSeqReader reader;
    double sum = 0;

    CvSize winSize = ((CvCascadeBoostTrainData*)data)->cascadeData->getWinSize();
    //int maxCatCount = ((CvCascadeBoostTrainData*)data)->cascadeData->getMaxCatCount();
    varCount = ((CvCascadeBoostTrainData*)data)->cascadeData->getNumFeatures();

    if( !weak )
        CV_ERROR( CV_StsError, "The boosted tree ensemble has not been trained yet" );

    cvStartReadSeq( weak, &reader );
    cvSetSeqReaderPos( &reader, 0 );

    for( i = 0; i < weak->total; i++ )
    {
        CvBoostTree* wtree;
        CV_READ_SEQ_ELEM( wtree, reader );
        sum += ((CvCascadeBoostTree*)wtree)->predict(sampleIdx)->value;
    }
    if( returnSum )
        value = (float)sum;
    else
    {
        int clsIdx = sum < threshold - CV_THRESHOLD_EPS ? 0 : 1;
        value = (float)clsIdx;
    }

    __END__;

    return value;
}

const CvCascadeBoostTrainData* CvCascadeBoost::get_data() const
{
    return (CvCascadeBoostTrainData*)data;
}

void CvCascadeBoost::update_weights( CvBoostTree* tree )
{
    CV_FUNCNAME( "CvCascadeBoost::update_weights" );
    __BEGIN__;

    int i, n = data->sample_count;
    double sumW = 0.;
    int step = 0;
    float* fdata = 0;
    int *sampleIdxBuf;
    const int* sampleIdx = 0;
    if ( (params.boost_type == LOGIT) || (params.boost_type == GENTLE) )
    {
        step = CV_IS_MAT_CONT(data->responses_copy->type) ?
            1 : data->responses_copy->step / CV_ELEM_SIZE(data->responses_copy->type);
        fdata = data->responses_copy->data.fl;
        sampleIdxBuf = (int*)cvStackAlloc(data->sample_count*sizeof(sampleIdxBuf[0]));
        data->get_sample_indices( data->data_root, sampleIdxBuf, &sampleIdx );    
    }
    CvMat* buf = data->buf;
    if( !tree ) // before training the first tree, initialize weights and other parameters
    {
        int n = data->sample_count;
        int* classLabelsBuf = data->resp_int_buf;
        const int* classLabels = 0;
        data->get_class_labels(data->data_root, classLabelsBuf, &classLabels);
        // in case of logitboost and gentle adaboost each weak tree is a regression tree,
        // so we need to convert class labels to floating-point values
        float* responses_buf = data->resp_float_buf;
        const float* responses = 0;
        data->get_ord_responses(data->data_root, responses_buf, &responses);
        
        double w0 = 1./n;
        double p[2] = { 1, 1 };

        cvReleaseMat( &orig_response );
        cvReleaseMat( &sum_response );
        cvReleaseMat( &weak_eval );
        cvReleaseMat( &subsample_mask );
        cvReleaseMat( &weights );

        CV_CALL( orig_response = cvCreateMat( 1, n, CV_32S ));
        CV_CALL( weak_eval = cvCreateMat( 1, n, CV_64F ));
        CV_CALL( subsample_mask = cvCreateMat( 1, n, CV_8U ));
        CV_CALL( weights = cvCreateMat( 1, n, CV_64F ));
        CV_CALL( subtree_weights = cvCreateMat( 1, n + 2, CV_64F ));

        if( data->have_priors )
        {
            // compute weight scale for each class from their prior probabilities
            int c1 = 0;
            for( i = 0; i < n; i++ )
                c1 += classLabels[i];
            p[0] = data->priors->data.db[0]*(c1 < n ? 1./(n - c1) : 0.);
            p[1] = data->priors->data.db[1]*(c1 > 0 ? 1./c1 : 0.);
            p[0] /= p[0] + p[1];
            p[1] = 1. - p[0];
        }

        if (data->is_buf_16u)
        {
            unsigned short* labels = (unsigned short*)(buf->data.s + data->data_root->buf_idx*buf->cols + 
                data->data_root->offset + (data->work_var_count-1)*data->sample_count);
            for( i = 0; i < n; i++ )
            {
                // save original categorical responses {0,1}, convert them to {-1,1}
                orig_response->data.i[i] = classLabels[i]*2 - 1;
                // make all the samples active at start.
                // later, in trim_weights() deactivate/reactive again some, if need
                subsample_mask->data.ptr[i] = (uchar)1;
                // make all the initial weights the same.
                weights->data.db[i] = w0*p[classLabels[i]];
                // set the labels to find (from within weak tree learning proc)
                // the particular sample weight, and where to store the response.
                labels[i] = (unsigned short)i;
            }
        }
        else
        {
            int* labels = buf->data.i + data->data_root->buf_idx*buf->cols + 
                data->data_root->offset + (data->work_var_count-1)*data->sample_count;

            for( i = 0; i < n; i++ )
            {
                // save original categorical responses {0,1}, convert them to {-1,1}
                orig_response->data.i[i] = classLabels[i]*2 - 1;
                // make all the samples active at start.
                // later, in trim_weights() deactivate/reactive again some, if need
                subsample_mask->data.ptr[i] = (uchar)1;
                // make all the initial weights the same.
                weights->data.db[i] = w0*p[classLabels[i]];
                // set the labels to find (from within weak tree learning proc)
                // the particular sample weight, and where to store the response.
                labels[i] = i;
            }
        }

        if( params.boost_type == LOGIT )
        {
            CV_CALL( sum_response = cvCreateMat( 1, n, CV_64F ));

            for( i = 0; i < n; i++ )
            {
                sum_response->data.db[i] = 0;
                fdata[sampleIdx[i]*step] = orig_response->data.i[i] > 0 ? 2.f : -2.f;
            }

            // in case of logitboost each weak tree is a regression tree.
            // the target function values are recalculated for each of the trees
            data->is_classifier = false;
        }
        else if( params.boost_type == GENTLE )
        {
            for( i = 0; i < n; i++ )
                fdata[sampleIdx[i]*step] = (float)orig_response->data.i[i];

            data->is_classifier = false;
        }
    }
    else
    {
        // at this moment, for all the samples that participated in the training of the most
        // recent weak classifier we know the responses. For other samples we need to compute them
        if( have_subsample )
        {
            // invert the subsample mask
            cvXorS( subsample_mask, cvScalar(1.), subsample_mask );
            
            // run tree through all the non-processed samples
            for( i = 0; i < n; i++ )
                if( subsample_mask->data.ptr[i] )
                {
                    weak_eval->data.db[i] = ((CvCascadeBoostTree*)tree)->predict( i )->value;
                }
        }

        // now update weights and other parameters for each type of boosting
        if( params.boost_type == DISCRETE )
        {
            // Discrete AdaBoost:
            //   weak_eval[i] (=f(x_i)) is in {-1,1}
            //   err = sum(w_i*(f(x_i) != y_i))/sum(w_i)
            //   C = log((1-err)/err)
            //   w_i *= exp(C*(f(x_i) != y_i))

            double C, err = 0.;
            double scale[] = { 1., 0. };

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i];
                sumW += w;
                err += w*(weak_eval->data.db[i] != orig_response->data.i[i]);
            }

            if( sumW != 0 )
                err /= sumW;
            C = err = -log_ratio( err );
            scale[1] = exp(err);

            sumW = 0;
            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i]*
                    scale[weak_eval->data.db[i] != orig_response->data.i[i]];
                sumW += w;
                weights->data.db[i] = w;
            }

            tree->scale( C );
        }
        else if( params.boost_type == REAL )
        {
            // Real AdaBoost:
            //   weak_eval[i] = f(x_i) = 0.5*log(p(x_i)/(1-p(x_i))), p(x_i)=P(y=1|x_i)
            //   w_i *= exp(-y_i*f(x_i))

            for( i = 0; i < n; i++ )
                weak_eval->data.db[i] *= -orig_response->data.i[i];

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i]*weak_eval->data.db[i];
                sumW += w;
                weights->data.db[i] = w;
            }
        }
        else if( params.boost_type == LOGIT )
        {
            // LogitBoost:
            //   weak_eval[i] = f(x_i) in [-z_max,z_max]
            //   sum_response = F(x_i).
            //   F(x_i) += 0.5*f(x_i)
            //   p(x_i) = exp(F(x_i))/(exp(F(x_i)) + exp(-F(x_i))=1/(1+exp(-2*F(x_i)))
            //   reuse weak_eval: weak_eval[i] <- p(x_i)
            //   w_i = p(x_i)*1(1 - p(x_i))
            //   z_i = ((y_i+1)/2 - p(x_i))/(p(x_i)*(1 - p(x_i)))
            //   store z_i to the data->data_root as the new target responses

            const double lbWeightThresh = FLT_EPSILON;
            const double lbZMax = 10.;
            float* responsesBuf = data->resp_float_buf;
            const float* responses = 0;
            data->get_ord_responses(data->data_root, responsesBuf, &responses);

            for( i = 0; i < n; i++ )
            {
                double s = sum_response->data.db[i] + 0.5*weak_eval->data.db[i];
                sum_response->data.db[i] = s;
                weak_eval->data.db[i] = -2*s;
            }

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double p = 1./(1. + weak_eval->data.db[i]);
                double w = p*(1 - p), z;
                w = MAX( w, lbWeightThresh );
                weights->data.db[i] = w;
                sumW += w;
                if( orig_response->data.i[i] > 0 )
                {
                    z = 1./p;
                    fdata[sampleIdx[i]*step] = (float)MIN(z, lbZMax);
                }
                else
                {
                    z = 1./(1-p);
                    fdata[sampleIdx[i]*step] = (float)-MIN(z, lbZMax);
                }
            }
        }
        else
        {
            // Gentle AdaBoost:
            //   weak_eval[i] = f(x_i) in [-1,1]
            //   w_i *= exp(-y_i*f(x_i))
            assert( params.boost_type == GENTLE );

            for( i = 0; i < n; i++ )
                weak_eval->data.db[i] *= -orig_response->data.i[i];

            cvExp( weak_eval, weak_eval );

            for( i = 0; i < n; i++ )
            {
                double w = weights->data.db[i] * weak_eval->data.db[i];
                weights->data.db[i] = w;
                sumW += w;
            }
        }
    }

    // renormalize weights
    if( sumW > FLT_EPSILON )
    {
        sumW = 1./sumW;
        for( i = 0; i < n; ++i )
            weights->data.db[i] *= sumW;
    }

    __END__;
}

bool CvCascadeBoost::isErrDesired()
{
    int sCount = data->sample_count, numPos = 0, numNeg = 0, numFalse = 0, numPosTrue = 0;
    float* eval = (float*) cvStackAlloc( sizeof(eval[0]) * sCount );
    float hitRate, falseAlarm;
    int thresholdIdx;
    for( int i = 0; i < sCount; i++ )
    {
        if( ((CvCascadeBoostTrainData*)data)->cascadeData->getCls( i ) == 1.0F )
        {
            eval[numPos] = predict( i, true );
            numPos++;
        }
    }
    icvSortFlt( eval, numPos, 0 );
    thresholdIdx = (int)((1.0F - minHitRate) * numPos);
    threshold = eval[ thresholdIdx ];
    numPosTrue = numPos - thresholdIdx;
    for( int i = thresholdIdx - 1; i >= 0; i--)
    {
        if ( abs( eval[i] - threshold) < FLT_EPSILON )
            numPosTrue++;
    }
    hitRate = ((float) numPosTrue) / ((float) numPos);
    for( int i = 0; i < sCount; i++ )
    {
        if( ((CvCascadeBoostTrainData*)data)->cascadeData->getCls( i ) == 0.0F )
        {
            numNeg++;
            if( predict( i ) )
                numFalse++;
        }
    }
    falseAlarm = ((float) numFalse) / ((float) numNeg);

    printf( "|%4d|%9f|%9f|\n", weak->total, hitRate, falseAlarm );
    printf( "+----+---------+---------+\n" );

    return falseAlarm <= maxFalseAlarm;
}

void CvCascadeBoost::write( CvFileStorage* fs, const CvMat* featureMap )
{
    const char treeStr[] = "tree";
    char cmnt[30];
    
    CvCascadeBoostTree* weakTree;
    cvWriteInt( fs, CC_WEAK_COUNT, weak->total );
    cvWriteReal( fs, CC_STAGE_THRESHOLD, threshold );
    cvStartWriteStruct( fs, CC_WEAK_CLASSIFIERS, CV_NODE_SEQ );
    for( int wi = 0; wi < weak->total; wi++)
    {
        sprintf( cmnt, "%s %i", treeStr, wi );
        cvWriteComment( fs, cmnt, 0 );
        weakTree = *((CvCascadeBoostTree**) cvGetSeqElem( weak, wi ));
        weakTree->write( fs, featureMap );
    }
    cvEndWriteStruct( fs ); // weak_classifiers
}

bool CvCascadeBoost::read( CvFileStorage* fs, CvFileNode* node, CvCascadeData* _cascadeData,
                            CvCascadeBoostParams* _params )
{
    bool res = false;
    CV_FUNCNAME( "CvCascadeBoost::read" );
    __BEGIN__;

    CvSeqReader reader;
    CvFileNode* tempNode;
    CvMemStorage* storage;
    int i, numTrees;

    clear();
    
    data = new CvCascadeBoostTrainData( _cascadeData );
    params = *_params;
    minHitRate = _params->minHitRate;
    maxFalseAlarm = _params->maxFalseAlarm;
    CV_CALL( tempNode = cvGetFileNodeByName( fs, node, CC_STAGE_THRESHOLD ) );
    if ( !tempNode )
        EXIT;
    threshold = (float)tempNode->data.f;
    CV_CALL( tempNode = cvGetFileNodeByName( fs, node, CC_WEAK_CLASSIFIERS ) ); 
    if ( !tempNode )
        EXIT;
    cvStartReadSeq( tempNode->data.seq, &reader );
    numTrees = tempNode->data.seq->total;

    CV_CALL( storage = cvCreateMemStorage() );
    weak = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvBoostTree*), storage );

    for( i = 0; i < numTrees; i++ )
    {
        CvCascadeBoostTree* tree = new CvCascadeBoostTree();
        CV_CALL(tree->read( fs, (CvFileNode*)reader.ptr, this, data ));
        CV_NEXT_SEQ_ELEM( reader.seq->elem_size, reader );
        cvSeqPush( weak, &tree );
    }

    res = true;

    __END__;

    return res;
}

void CvCascadeBoost::markFeaturesInMap( CvMat* featureMap )
{
    CvCascadeBoostTree* weakTree;
    for( int wi = 0; wi < weak->total; wi++ )
    {
        weakTree = *((CvCascadeBoostTree**) cvGetSeqElem( weak, wi ));
        weakTree->markFeaturesInMap( featureMap );
    }
}