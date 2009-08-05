#include "cascadeclassifier.h"
#include "_inner_functions.h"

#include <queue>
using namespace std;

static const char* stagetypes[] = { CC_BOOST };
static const char* featuretypes[] = { CC_HAAR, CC_LBP };

//---------------------------- CascadeParams --------------------------------------

void CvCascadeParams::write( CvFileStorage* fs ) const
{
    const char* stageTypeStr = 0;
    const char* featureTypeStr = 0;

    stageTypeStr = stageType == BOOST ? CC_BOOST : 0;
    if( stageTypeStr )
        cvWriteString( fs, CC_STAGE_TYPE, stageTypeStr );
    else
        cvWriteInt( fs, CC_STAGE_TYPE, stageType );

    featureTypeStr = featureType == HAAR ? CC_HAAR :
                        featureType == LBP ? CC_LBP : 0;
    if( stageTypeStr )
        cvWriteString( fs, CC_FEATURE_TYPE, featureTypeStr );
    else
        cvWriteInt( fs, CC_FEATURE_TYPE, featureType );
    cvWriteInt( fs, CC_HEIGHT, winSize.height );
    cvWriteInt( fs, CC_WIDTH, winSize.width );

}

bool CvCascadeParams::read( CvFileStorage* fs, CvFileNode* node )
{
    bool res = false;

     CV_FUNCNAME( "CvCascadeClassifier::read_cascadeParams" );
    __BEGIN__;

    const char* stageTypeStr;
    const char* featureTypeStr;

    CV_CALL( stageTypeStr = cvReadStringByName( fs, node, CC_STAGE_TYPE ) );
    if ( !stageTypeStr )
        EXIT;
    CV_CALL( stageType = strcmp( stageTypeStr, CC_BOOST ) == 0 ? 
            BOOST : cvReadIntByName( fs, node, CC_STAGE_TYPE ) );
    
    CV_CALL( featureTypeStr = cvReadStringByName( fs, node, CC_FEATURE_TYPE ) );
    if ( !featureTypeStr )
        EXIT;
    CV_CALL( featureType = strcmp( featureTypeStr, CC_HAAR ) == 0 ? HAAR :
                            strcmp( featureTypeStr, CC_LBP ) == 0 ? LBP : 
                            cvReadIntByName( fs, node, CC_FEATURE_TYPE ) );
    CV_CALL( winSize.height = cvReadIntByName( fs, node, CC_HEIGHT ) );
    CV_CALL( winSize.width = cvReadIntByName( fs, node, CC_WIDTH ) );
    if ( winSize.height <= 0 || winSize.width <= 0 )
        EXIT;

    res = true; 

    __END__;

    return res;
}

void CvCascadeParams::printDefaults()
{
    CvParams::printDefaults();
    printf("  [-stageType <" );
    int i;
    for( i = 0; i < (int)(sizeof(stagetypes)/sizeof(stagetypes[0])); i++ )
    {
        printf("%s%s", i ? " | " : "", stagetypes[i] );
        if ( i == defaultStageType )
            printf("(default)");
    }
    printf(">]\n" );

    printf("  [-featureType <{" );
    for( i = 0; i < (int)(sizeof(featuretypes)/sizeof(featuretypes[0])); i++ )
    {
        printf("%s%s", i ? ", " : "", featuretypes[i] );
        if ( i == defaultStageType )
            printf("(default)");
    }
    printf("}>]\n" );
    printf( "  [-w <sampleWidth = %d>]\n"
            "  [-h <sampleHeight = %d>]\n",
            winSize.width, winSize.height );
}

void CvCascadeParams::printAttrs()
{
    printf( "stageType: %s\n", stagetypes[stageType] );
    printf( "featureType: %s\n", featuretypes[featureType] );
    printf( "sampleWidth: %d\n", winSize.width );
    printf( "sampleHeight: %d\n", winSize.height );
}

bool CvCascadeParams::scanAttr( const char* prmName, const char* val )
{
    bool res = true;

    if( !strcmp( prmName, "-stageType" ) )
    {
        for( int i = 0; i < (int)(sizeof(stagetypes)/sizeof(stagetypes[0])); i++ )
        {
            if( !strcmp( val, stagetypes[i] ) )
                stageType = i;
        }
    }
    else if( !strcmp( prmName, "-featureType" ) )
    {
        for( int i = 0; i < (int)(sizeof(featuretypes)/sizeof(featuretypes[0])); i++ )
        {
            if( !strcmp( val, featuretypes[i] ) )
                featureType = i;
        }
    }
    else if( !strcmp( prmName, "-w" ) )
    {
        winSize.width = atoi( val );
    }
    else if( !strcmp( prmName, "-h" ) )
    {
        winSize.height = atoi( val );
    }
    else
        res = false;

    return res;
}

//---------------------------- CascadeClassifier --------------------------------------

CvCascadeClassifier::CvCascadeClassifier()
{
    cascadeData = 0;
    featureParams = 0;
    stageParams = 0;
    stageClassifiers = 0;
}

CvCascadeClassifier::~CvCascadeClassifier()
{
    if ( cascadeData )
    {
        delete cascadeData;
        cascadeData = 0;
    }
    if ( featureParams )
    {
        delete featureParams;
        featureParams = 0;
    }
    if ( stageParams )
    {
        delete stageParams;
        stageParams = 0;
    }
    if( stageClassifiers )
    {
        for( int i = 0; i < numCurStages; i++ )
            delete stageClassifiers[i];
    }    
    cvFree( &stageClassifiers );
}

void CvCascadeClassifier::createStageParams()
{
    CV_FUNCNAME( "CvCascadeClassifier::createStageParams" );
    __BEGIN__;
    switch ( cascadeParams.stageType )
    {
        case CvCascadeParams::BOOST :
            stageParams = new CvCascadeBoostParams();
            break;
        default: CV_ERROR(CV_StsBadFunc, "unsupported stage type");
    }
    __END__;
}

void CvCascadeClassifier::createCurStage()
{
    CV_FUNCNAME( "CvCascadeClassifier::createCurStage" );
    __BEGIN__;
    switch ( cascadeParams.stageType )
    {
        case CvCascadeParams::BOOST :
            stageClassifiers[numCurStages] = new CvCascadeBoost();
            break;
        default: CV_ERROR(CV_StsBadFunc, "unsupported stage type");
    }
    __END__;
}

void CvCascadeClassifier::createFeatureParams()
{
    CV_FUNCNAME( "CvCascadeClassifier::createCurStage" );
    __BEGIN__;
    switch ( cascadeParams.featureType )
    {
        case CvCascadeParams::HAAR :
            featureParams = new CvHaarFeatureParams();
            break;
        case CvCascadeParams::LBP :
            featureParams = new CvLBPFeatureParams();
            break;
        default: CV_ERROR(CV_StsBadFunc, "unsupported feature type");
    }
    __END__;
}

void CvCascadeClassifier::createCascadeData()
{
    CV_FUNCNAME( "CvCascadeClassifier::createCascadeData" );
    __BEGIN__;
    switch ( cascadeParams.featureType )
    {
    case CvCascadeParams::HAAR :
        cascadeData = new CvHaarCascadeData();
        break;
    case CvCascadeParams::LBP :
        cascadeData = new CvLBPCascadeData();
        break;
    default: CV_ERROR(CV_StsBadFunc, "unsupported feature type");

    }
    __END__;
}

bool CvCascadeClassifier::train( const char* _cascadeDirName,
                                        const char* _vecFileName,
                                        const char* _bgfileName, 
                                        int _numPos, int _numNeg, 
                                        int _numPrecalcVal, int _numPrecalcIdx,
                                        int _numStages,
                                        const CvCascadeParams& _cascadeParams,
                                        const CvFeatureParams& _featureParams,
                                        const CvCascadeBoostParams& _stageParams,
                                        bool baseFormatSave )
{   
    bool res = false;

    CV_FUNCNAME( "CvCascadeClassifier::train" );
    __BEGIN__;

    char* nullname = (char*)"(NULL)";
    CvFileStorage* fs;
    char buf[200];
    //int consumed = 0;
    double tempLeafFARate, requiredLeafFARate;
        
    assert( _cascadeDirName );    

    // TODO: check input params
    // TODO: other stages
    cascadeParams = _cascadeParams;
    numStages = _numStages;
    numCurStages = 0;

    if( !(_bgfileName && _vecFileName) )
        CV_ERROR( CV_StsBadArg, "_bgfileName or _vecFileName is NULL" );
    if ( !loadTempInfo( _cascadeDirName, _vecFileName, _bgfileName,  _numPos, _numNeg ) )
    {
        createStageParams();
        *stageParams = _stageParams;

        createFeatureParams();
        featureParams->set( &_featureParams );

        createCascadeData();
        cascadeData->setData( this, _vecFileName, _bgfileName, _numPos, _numNeg, featureParams );   

        stageClassifiers = (CvCascadeBoost**) cvAlloc( numStages * sizeof(stageClassifiers[0]) );
	    memset( stageClassifiers, 0, sizeof(stageClassifiers[0])*numStages);    

        // save params
        sprintf( buf, "%s/%s", _cascadeDirName, CC_PARAMS_FILENAME );
        fs = cvOpenFileStorage( buf, 0, CV_STORAGE_WRITE );
        if ( !fs )
            EXIT;
        writeParams( fs );
        cvReleaseFileStorage( &fs );
    }
    // print used parameters
    printf( "cascadeDirName: %s\n", ((_cascadeDirName == NULL) ? nullname : _cascadeDirName ) );
    printf( "vecFileName: %s\n", ((_vecFileName == NULL) ? nullname : _vecFileName ) );
    printf( "bgFileName: %s\n", ((_bgfileName == NULL) ? nullname : _bgfileName ) );
    printf( "numPos: %d\n", _numPos );
    printf( "numNeg: %d\n", _numNeg );
    printf( "numStages: %d\n", numStages );
    printf( "numPrecalcValues : %d\n", _numPrecalcVal );
    printf( "numPrecalcIndices : %d\n", _numPrecalcIdx );
    cascadeParams.printAttrs();
    stageParams->printAttrs();
    featureParams->printAttrs();
    if ( numCurStages > 1 )
        printf("\nStages %d-%d are loaded\n", 0, numCurStages-1);
    else if ( numCurStages == 1)
        printf("\nStage 0 is loaded\n");
    
    requiredLeafFARate = pow( (double) stageParams->maxFalseAlarm, (double) numStages ) / (double)stageParams->max_depth;
    
    for( ; numCurStages < numStages; numCurStages++ )
    {
        printf( "\nStages\n" );
        for( int i = 0; i <= numCurStages; i++ ) printf( "+---" );
        printf( "+\n" );
        for( int i = 0; i <= numCurStages; i++ ) printf( "|%3d", i );
        printf( "|\n" );
        for( int i = 0; i <= numCurStages; i++ ) printf( "+---" );
        printf( "+\n\n" );

        if ( !cascadeData->updateForNextStage( tempLeafFARate ) ) 
        {
            printf("Train dataset for temp stage can not be filled\n");
            printf("\n===Cascade training ended===\n");
            break;
        }
        
        if( tempLeafFARate <= requiredLeafFARate )
        {
            printf( "Required leaf false alarm rate achieved. "
                    "Branch training terminated.\n" );
            break;
        }

        createCurStage();
        stageClassifiers[numCurStages]->train( cascadeData, _numPrecalcVal, _numPrecalcIdx, *stageParams );

        sprintf( buf, "%s/%d%s", _cascadeDirName, numCurStages, ".xml" );
        fs = cvOpenFileStorage( buf, 0, CV_STORAGE_WRITE );
        if ( !fs )
            EXIT;
        stageClassifiers[numCurStages]->write( fs, 0);
        cvReleaseFileStorage( &fs );
    }

    save( _cascadeDirName, baseFormatSave );

    cascadeData->clear();
    res = true;

    __END__;
    return res;
}

int CvCascadeClassifier::predict( int sampleIdx )
{
    //CvCascadeBoost* cur_classifier = stageClassifiers[0];
    for (int i = 0; i < numCurStages; i++ )
    {
        if ( stageClassifiers[i]->predict( sampleIdx ) == 0.f )
        {
            return 0;
        }
    }
    return 1;
}

void CvCascadeClassifier::writeParams( CvFileStorage* fs ) const
{
    cascadeParams.write( fs );

    cvStartWriteStruct( fs, CC_STAGE_PARAMS, CV_NODE_MAP );
    stageParams->write( fs );
    cvEndWriteStruct( fs );

    cvStartWriteStruct( fs, CC_FEATURE_PARAMS, CV_NODE_MAP );
    featureParams->write( fs );
    cvEndWriteStruct( fs );
}

void CvCascadeClassifier::writeFeatures( CvFileStorage* fs, const CvMat* featureMap ) const
{
    cascadeData->writeFeatures( fs, featureMap ); 
}

void CvCascadeClassifier::writeStages( CvFileStorage* fs, const CvMat* featureMap ) const
{
    CV_FUNCNAME( "CvCascadeClassifier::writeStages" );
    __BEGIN__;

    const char stage_str[] = "stage";
    char cmnt[30];
    CV_CALL( cvStartWriteStruct( fs, CC_STAGES, CV_NODE_SEQ ) ); 
    for( int i = 0; i < numCurStages; i++ )
    {
        sprintf( cmnt, "%s %i", stage_str, i );
        CV_CALL( cvWriteComment( fs, cmnt, 0 ) );
        cvStartWriteStruct( fs, 0, CV_NODE_MAP );
        stageClassifiers[i]->write( fs, featureMap );
        cvEndWriteStruct( fs );
    }
    CV_CALL( cvEndWriteStruct( fs ) );

    __END__;
}

bool CvCascadeClassifier::readParams( CvFileStorage* fs, CvFileNode* _node )
{
    bool res = false;

    //CV_FUNCNAME( "CvCascadeClassifier::readParams" );
    __BEGIN__;

    CvFileNode *node;

    if ( !cascadeParams.read( fs, _node ) )
        EXIT;
    
    createStageParams();
    node = cvGetFileNodeByName( fs, _node, CC_STAGE_PARAMS);
    if ( !node )
        EXIT;
    if ( !stageParams->read( fs, node ) )
        EXIT;
    
    createFeatureParams();
    node = cvGetFileNodeByName( fs, _node, CC_FEATURE_PARAMS);
    if ( !node )
        EXIT;
    if ( !featureParams->read( fs, node ) )
        EXIT;
    
    res = true;

    __END__;

    return res;
}

bool CvCascadeClassifier::readStages( CvFileStorage* fs, CvFileNode* _node )
{
    bool res = false;

    CV_FUNCNAME( "CvCascadeClassifier::readStages" );
    __BEGIN__;

    CvFileNode* node = cvGetFileNodeByName( fs, _node, CC_STAGES );
    CvSeq* stageClassifiers_seq;
    if ( !node )
        EXIT;
    CV_CALL( stageClassifiers = (CvCascadeBoost**) cvAlloc( numStages * sizeof(stageClassifiers[0]) ) );
    memset( stageClassifiers, 0, sizeof(stageClassifiers[0])*numStages);    

    stageClassifiers_seq = node->data.seq;
    for( numCurStages = 0; numCurStages < MIN( stageClassifiers_seq->total, numStages ); numCurStages++ )
    {
        createCurStage();
        node = ((CvFileNode*)cvGetSeqElem( stageClassifiers_seq, numCurStages ));
        if ( !stageClassifiers[numCurStages]->read( fs, node, cascadeData, stageParams ) )
            EXIT;
    }
    res = true;

    __END__;

    return res;
}

bool CvCascadeClassifier::save( const char* cascadeDirName, bool baseFormat )
{
    bool res = false;

    CV_FUNCNAME( "CvCascadeClassifier::save" );
    __BEGIN__;

    cv::String cascadeName = cv::FileStorage::getDefaultObjectName(CC_CASCADE_FILENAME);
    cv::String fileName = cv::String(cascadeDirName) + '/' + cv::String(CC_CASCADE_FILENAME);
    CvFileStorage* fs = cvOpenFileStorage( fileName.c_str(), 0, CV_STORAGE_WRITE );

    if ( !fs )
        EXIT;
    
    if ( !baseFormat )
    {
        CvMat* featureMap = cvCreateMat( 1, cascadeData->getNumFeatures(), CV_32SC1 ); 
        cvSet( featureMap, cvScalar(-1) );
        markFeaturesInMap( featureMap );

        cvStartWriteStruct(fs, cascadeName.c_str(), CV_NODE_MAP, "opencv-cascade-classifier");
        writeParams( fs );
        cvWriteInt( fs, CC_STAGE_NUM, numCurStages );
        writeStages( fs, featureMap);
        writeFeatures(fs, featureMap);
        cvReleaseMat( &featureMap );
        cvEndWriteStruct(fs);
    }
    else
    {
        char buf[256];
        CvSeq* weak;
        if ( cascadeParams.featureType != CvCascadeParams::HAAR )
            CV_ERROR( CV_StsBadFunc, "old file format is used for Haar-like features only");
        cvStartWriteStruct(fs, cascadeName.c_str(), CV_NODE_MAP, CV_TYPE_NAME_HAAR );
        CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_SIZE_NAME, CV_NODE_SEQ | CV_NODE_FLOW ) );
        CV_CALL( cvWriteInt( fs, NULL, cascadeParams.winSize.width ) );
        CV_CALL( cvWriteInt( fs, NULL, cascadeParams.winSize.height ) );
        CV_CALL( cvEndWriteStruct( fs ) ); /* ICV_HAAR_SIZE_NAME */

        CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_STAGES_NAME, CV_NODE_SEQ ) );
        for( int si = 0; si < numCurStages; si++ )
        {
            CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_MAP ) );
            sprintf( buf, "stage %d", si );
            CV_CALL( cvWriteComment( fs, buf, 1 ) );
            weak = stageClassifiers[si]->get_weak_predictors();

            CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_TREES_NAME, CV_NODE_SEQ ) );
            for( int wi = 0; wi < weak->total; wi++ )
            {
                int inner_node_idx = -1, total_inner_node_idx = -1;
                queue<const CvDTreeNode*> inner_nodes_queue;
                CvCascadeBoostTree* tree = *((CvCascadeBoostTree**) cvGetSeqElem( weak, wi ));
                
                CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_SEQ ) );
                sprintf( buf, "tree %d", wi );
                CV_CALL( cvWriteComment( fs, buf, 1 ) );

                const CvDTreeNode* tempNode;
                
                inner_nodes_queue.push( tree->get_root() );
                total_inner_node_idx++;
                
                while (!inner_nodes_queue.empty())
                {
                    tempNode = inner_nodes_queue.front();
                    inner_node_idx++;

                    CV_CALL( cvStartWriteStruct( fs, NULL, CV_NODE_MAP ) );
                    if( inner_node_idx != 0 )
                    {
                        sprintf( buf, "node %d", inner_node_idx );
                    }
                    else
                    {
                        sprintf( buf, "root node" );
                    }
                    CV_CALL( cvWriteComment( fs, buf, 1 ) );
                    
                    CV_CALL( cvStartWriteStruct( fs, ICV_HAAR_FEATURE_NAME, CV_NODE_MAP ) );
                    cascadeData->writeFeature( fs, tempNode->split->var_idx );
                    CV_CALL( cvEndWriteStruct( fs ) ); /* feature */

                    CV_CALL( cvWriteReal( fs, ICV_HAAR_THRESHOLD_NAME, tempNode->split->ord.c) );

                    if( tempNode->left->left || tempNode->left->right )
                    {
                        inner_nodes_queue.push( tempNode->left );
                        total_inner_node_idx++;
                        CV_CALL( cvWriteInt( fs, ICV_HAAR_LEFT_NODE_NAME, total_inner_node_idx ) );
                    }
                    else
                    {
                        CV_CALL( cvWriteReal( fs, ICV_HAAR_LEFT_VAL_NAME, tempNode->left->value ) );
                    }

                    if( tempNode->right->left || tempNode->right->right )
                    {
                        inner_nodes_queue.push( tempNode->right );
                        total_inner_node_idx++;
                        CV_CALL( cvWriteInt( fs, ICV_HAAR_RIGHT_NODE_NAME, total_inner_node_idx ) );
                    }
                    else
                    {
                        CV_CALL( cvWriteReal( fs, ICV_HAAR_RIGHT_VAL_NAME, tempNode->right->value ) );
                    }

                    CV_CALL( cvEndWriteStruct( fs ) );
                    inner_nodes_queue.pop();
                }
                CV_CALL( cvEndWriteStruct( fs ) ); 
            }

            CV_CALL( cvEndWriteStruct( fs ) ); /* trees */

            CV_CALL( cvWriteReal( fs, ICV_HAAR_STAGE_THRESHOLD_NAME,
                                  stageClassifiers[si]->getThreshold() ) );

            CV_CALL( cvWriteInt( fs, ICV_HAAR_PARENT_NAME, si-1 ) );
            CV_CALL( cvWriteInt( fs, ICV_HAAR_NEXT_NAME, -1) );

            CV_CALL( cvEndWriteStruct( fs ) ); /* stage */
        } /* for each stage */

        CV_CALL( cvEndWriteStruct( fs ) ); /* stages */
        cvEndWriteStruct(fs);
    }

    cvReleaseFileStorage( &fs );
    res = true;

    __END__;

    return res;
}

bool CvCascadeClassifier::loadTempInfo( const char* cascadeDirName,
      const char* _vecFileName, const char* _bgfileName, int _numPos, int _numNeg )
{
    bool res = false;

    CV_FUNCNAME( "CvCascadeClassifier::loadTempInfo" );
    __BEGIN__;

    char buf[200];

    sprintf( buf, "%s/%s", cascadeDirName, CC_PARAMS_FILENAME );
    // features are not read
    CvFileStorage* fs = cvOpenFileStorage( buf, 0, CV_STORAGE_READ );
    if ( !fs )
        EXIT;

    if ( !readParams( fs, 0 ) )
        EXIT;

    createCascadeData();
    cascadeData->setData( this, _vecFileName, _bgfileName, _numPos, _numNeg, featureParams );
    cvReleaseFileStorage( &fs );
    
    CV_CALL( stageClassifiers = (CvCascadeBoost**) cvAlloc( numStages * sizeof(stageClassifiers[0]) ) );
    memset( stageClassifiers, 0, sizeof(stageClassifiers[0])*numStages);    

    numCurStages = 0;
    for ( int si = 0; si < numStages; si++ )
    {
        sprintf( buf, "%s/%d%s", cascadeDirName, si, ".xml" );
        fs = cvOpenFileStorage( buf, 0, CV_STORAGE_READ );
        if ( !fs )
            break;
        createCurStage();
        if ( !stageClassifiers[si]->read( fs, 0, cascadeData, stageParams ) )
        {
            delete stageClassifiers[si];
            if ( fs )
                cvReleaseFileStorage( &fs );
            break;
        }
        cvReleaseFileStorage( &fs );
        numCurStages++;
    }

    res = true;

    __END__;

    return res;
}

void CvCascadeClassifier::markFeaturesInMap( CvMat* featureMap )
{
    int idx = 0;
    for( int si = 0; si < numCurStages; si++ )
        stageClassifiers[si]->markFeaturesInMap( featureMap );
    
    for( int fi = 0; fi < cascadeData->getNumFeatures(); fi++ )
        if ( featureMap->data.i[fi] >= 0 )
            featureMap->data.i[fi] = idx++;
}