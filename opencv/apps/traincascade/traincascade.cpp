#include "cv.h"
#include "cascadeclassifier.h"

int main( int argc, char* argv[] )
{
    CvCascadeClassifier classifier;
    char* cascadeDirName = NULL;
    char* vecName = NULL;
    char* bgName  = NULL;
    int numPos    = 1000;
    int numNeg    = 1000;
    int numStages = 20;
    int numPrecalcVal = 10000,
        numPrecalcIdx = 50000;
    bool baseFormatSave = false;
    
    CvCascadeParams cascadeParams;

    CvCascadeBoostParams stageParams;
    
    CvFeatureParams* featureParams[] = { new CvHaarFeatureParams(),
                                         new CvLBPFeatureParams() }; 
    int fc = sizeof(featureParams)/sizeof(featureParams[0]);
    
    if( argc == 1 )
    {
        printf( "Usage: %s\n"
                "  -data <cascade_dir_name>\n"
                "  -vec <vec_file_name>\n"
                "  -bg <background_file_name>\n"
                "  [-numPos <number_of_positive_samples = %d>]\n"
                "  [-numNeg <number_of_negative_samples = %d>]\n"
                "  [-numStages <number_of_stages = %d>]\n"
                "  [-numPrecalcVal <number_of_precalculated_vals = %d>]\n"
                "  [-numPrecalcIdx <number_of_precalculated_idxs = %d>]\n"
                "  [-baseFormatSave]\n",
                argv[0], numPos, numNeg, numStages, numPrecalcVal, numPrecalcIdx );

        cascadeParams.printDefaults();

        printf("--boostStageParams--\n");
        stageParams.printDefault();

        for( int fi = 0; fi < fc; fi++ )
            featureParams[fi]->printDefaults();

        return 0;
    }

    for( int i = 1; i < argc; i++ )
    {
        bool set = false;
        if( !strcmp( argv[i], "-data" ) )
        {
            cascadeDirName = argv[++i];
        }
        else if( !strcmp( argv[i], "-vec" ) )
        {
            vecName = argv[++i];
        }
        else if( !strcmp( argv[i], "-bg" ) )
        {
            bgName = argv[++i];
        }
        else if( !strcmp( argv[i], "-numPos" ) )
        {
            numPos = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numNeg" ) )
        {
            numNeg = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numStages" ) )
        {
            numStages = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numPrecalcVal" ) )
        {
            numPrecalcVal = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-numPrecalcIdx" ) )
        {
            numPrecalcIdx = atoi( argv[++i] );
        }
        else if( !strcmp( argv[i], "-baseFormatSave" ) )
        {
            baseFormatSave = true;
        }
        else if ( cascadeParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( stageParams.scanAttr( argv[i], argv[i+1] ) ) { i++; }
        else if ( !set )
        {
            for( int fi = 0; fi < fc; fi++ )
            {
                set = featureParams[fi]->scanAttr(argv[i], argv[i+1]);          
                if ( !set )
                {
                    i++;
                    break;
                }
            }
        }
    }
  
    classifier.train( cascadeDirName,
                      vecName,
                      bgName, 
                      numPos, numNeg, 
                      numPrecalcVal, numPrecalcIdx,
                      numStages,
                      cascadeParams,
                      *featureParams[cascadeParams.featureType],
                      stageParams,
                      baseFormatSave );

    for( int fi = 0; fi < fc; fi++ )
        delete featureParams[fi];

    return 0;
}