#include "cv.h"
#include "_imagestorage.h"
#include "_inner_functions.h"

//----------------------------------------  Background reading  ------------------------------------------------

CvBackgroundReader::CvBackgroundReader()
{
    src = cvMat( 1, 1, CV_8UC1, NULL );
    img = cvMat( 1, 1, CV_8UC1, NULL );
    offset = cvPoint( 0, 0 );
    scale       = 1.0F;
    scaleFactor = 1.4142135623730950488016887242097F;
    stepFactor  = 0.5F;
    point = offset;
}

CvBackgroundReader::~CvBackgroundReader()
{
    if( src.data.ptr != NULL )
    {
        cvFree( &src.data.ptr );
    }
    if( img.data.ptr != NULL )
    {
        cvFree( &img.data.ptr );
    }
}
 

CvBackgroundData::CvBackgroundData()
{
    fileName = 0; 
}

CvBackgroundData::CvBackgroundData( const char* _fileName, CvSize _winSize )
{
    const char* dir = NULL;    
    char full[CC_PATH_MAX];
    char* imgFileName = NULL;
    FILE* input = NULL;
    size_t len = 0, fileNamesLen = 0;

    assert( _fileName );
    
    dir = strrchr( _fileName, '\\' );
    if( dir == NULL )
    {
        dir = strrchr( _fileName, '/' );
    }
    if( dir == NULL )
    {
        imgFileName = &(full[0]);
    }
    else
    {
        strncpy( &(full[0]), _fileName, (dir - _fileName + 1) );
        imgFileName = &(full[(dir - _fileName + 1)]);
    }

    input = fopen( _fileName, "r" );
    if( input != NULL )
    {
        count = 0;
        fileNamesLen = 0;
        
        /* count */
        while( !feof( input ) )
        {
            *imgFileName = '\0';
            if( !fscanf( input, "%s", imgFileName ))
                break;
            len = strlen( imgFileName );
            if( len > 0 )
            {
                if( (*imgFileName) == '#' ) continue; /* comment */
                count++;
                fileNamesLen += sizeof( char ) * (strlen( &(full[0]) ) + 1);
            }
        }
        if( count > 0 )
        {   
            char* tmp;
            fseek( input, 0, SEEK_SET );
            fileNamesLen += sizeof( char* ) * count;
            fileName = (char**) cvAlloc( fileNamesLen );
            memset( (void*) fileName, 0, fileNamesLen );
            last = round = 0;
            winSize = _winSize;
            tmp = (char*)(fileName + count);
            count = 0;
            while( !feof( input ) )
            {
                *imgFileName = '\0';
                if( !fscanf( input, "%s", imgFileName ))
                    break;
                len = strlen( imgFileName );
                if( len > 0 )
                {
                    if( (*imgFileName) == '#' ) continue; /* comment */
                    fileName[count++] = tmp;
                    strcpy( tmp, &(full[0]) );
                    tmp += strlen( &(full[0]) ) + 1;
                }
            }
        }
        fclose( input );
    }

    bgReader = new CvBackgroundReader();
}

CvBackgroundData::~CvBackgroundData()
{
    delete bgReader;
    char* fb = (char*)fileName;
    cvFree( &fb );    
}

bool CvBackgroundData::getNext( bool reset )
{
    IplImage* img = NULL;
    size_t dataSize = 0;
    int i = 0;
    CvPoint offset = cvPoint(0,0);

    if( bgReader->src.data.ptr != NULL )
    {
        cvFree( &(bgReader->src.data.ptr) );
        bgReader->src.data.ptr = NULL;
    }
    if( bgReader->img.data.ptr != NULL )
    {
        cvFree( &(bgReader->img.data.ptr) );
        bgReader->img.data.ptr = NULL;
    }
    if ( reset )
        last = 0;
    {
        for( i = 0; i < count; i++ )
        {
            img = cvLoadImage( fileName[last++], 0 );
            if( !img )
                continue;
            round += last / count;
            round = round % (winSize.width * winSize.height);
            last %= count;

            offset.x = round % winSize.width;
            offset.y = round / winSize.width;

            offset.x = MIN( offset.x, img->width - winSize.width );
            offset.y = MIN( offset.y, img->height - winSize.height );
            
            if( img != NULL && img->depth == IPL_DEPTH_8U && img->nChannels == 1 &&
                offset.x >= 0 && offset.y >= 0 )
            {
                break;
            }
            if( img != NULL )
                cvReleaseImage( &img );
            img = NULL;
        }
    }
    if( img == NULL )
    {
        /* no appropriate image */
        return 0;
    }
    dataSize = sizeof( uchar ) * img->width * img->height;
    bgReader->src = cvMat( img->height, img->width, CV_8UC1, (void*) cvAlloc( dataSize ) );
    cvCopy( img, &bgReader->src, NULL );
    cvReleaseImage( &img );
    img = NULL;

    bgReader->offset = offset;
    bgReader->point = bgReader->offset;
    bgReader->scale = MAX(
        ((float) winSize.width + bgReader->point.x) / ((float) bgReader->src.cols),
        ((float) winSize.height + bgReader->point.y) / ((float) bgReader->src.rows) );
    
    bgReader->img = cvMat( (int) (bgReader->scale * bgReader->src.rows + 0.5F),
                         (int) (bgReader->scale * bgReader->src.cols + 0.5F),
                          CV_8UC1, (void*) cvAlloc( dataSize ) );
    cvResize( &(bgReader->src), &(bgReader->img) );

    return 1;
}


bool CvBackgroundData::getImage( CvMat* img, bool reset )
{
    CvMat mat;

    assert( img != NULL );
    assert( CV_MAT_TYPE( img->type ) == CV_8UC1 );
    assert( img->cols == winSize.width );
    assert( img->rows == winSize.height );

    if( bgReader->img.data.ptr == NULL )
    {
        if ( !getNext( reset ) ) 
            return 0;
    }

    mat = cvMat( winSize.height, winSize.width, CV_8UC1 );
    cvSetData( &mat, (void*) (bgReader->img.data.ptr + bgReader->point.y * bgReader->img.step
                              + bgReader->point.x * sizeof( uchar )), bgReader->img.step );

    cvCopy( &mat, img, 0 );
    if( (int) ( bgReader->point.x + (1.0F + bgReader->stepFactor ) * winSize.width )
            < bgReader->img.cols )
    {
        bgReader->point.x += (int) (bgReader->stepFactor * winSize.width);
    }
    else
    {
        bgReader->point.x = bgReader->offset.x;
        if( (int) ( bgReader->point.y + (1.0F + bgReader->stepFactor ) * winSize.height )
                < bgReader->img.rows )
        {
            bgReader->point.y += (int) (bgReader->stepFactor * winSize.height);
        }
        else
        {
            bgReader->point.y = bgReader->offset.y;
            bgReader->scale *= bgReader->scaleFactor;
            if( bgReader->scale <= 1.0F )
            {
                bgReader->img = cvMat( (int) (bgReader->scale * bgReader->src.rows),
                                     (int) (bgReader->scale * bgReader->src.cols),
                                      CV_8UC1, (void*) (bgReader->img.data.ptr) );
                cvResize( &(bgReader->src), &(bgReader->img) );
            }
            else
            {
                if ( !getNext( reset ) ) 
                    return 0;
            }
        }
    }

    return 1;
}

//------------------------------------------- VecFile reading ------------------------------------------------

CvVecFile::CvVecFile()
{
    input = 0;
    vector = 0;
}

CvVecFile::CvVecFile( const char* _vecFileName )
{
    CV_FUNCNAME( "CvVecFile::CvVecFile" );
    __BEGIN__;

    short tmp = 0;  

    input = NULL;
    if( _vecFileName ) input = fopen( _vecFileName, "rb" );

    if( input != NULL )
    {
        fread( &count, sizeof( count ), 1, input );
        fread( &vecSize, sizeof( vecSize ), 1, input );
        fread( &tmp, sizeof( tmp ), 1, input );
        fread( &tmp, sizeof( tmp ), 1, input );
        base = sizeof( count ) + sizeof( vecSize ) + sizeof( tmp ) + sizeof( tmp );
        if( !feof( input ) )
        {
            last = 0;
            CV_CALL( vector = (short*) cvAlloc( sizeof( *vector ) * vecSize ) );
        }
    }
    else
        CV_ERROR( CV_StsNullPtr, "vecfile can not be opened" );

    __END__;
}

CvVecFile::~CvVecFile()
{
    fclose( input );
    cvFree( &vector );
}

//-------------------------------------------  CvImageReader --------------------------------------------------

CvImageReader::CvImageReader( const char* _vecFileName, const char* _bgfileName, CvSize _winSize )
{
    bgData = new CvBackgroundData( _bgfileName, _winSize );
    vecFile = new CvVecFile( _vecFileName );
}

CvImageReader::~CvImageReader()
{
    delete bgData;
    delete vecFile;
}

bool CvImageReader::getNegImage(CvMat* img, bool reset )
{
    return bgData->getImage( img, reset );
}

bool CvImageReader::getPosImage(CvMat* img, bool reset )
{
    uchar tmp = 0;
    int r = 0;
    int c = 0;

    assert( img->rows * img->cols == vecFile->vecSize );
    
    if ( reset )
    {
        vecFile->last = 0;
        fseek( vecFile->input, vecFile->base, SEEK_SET );
    }

    fread( &tmp, sizeof( tmp ), 1, vecFile->input );
    fread( vecFile->vector, sizeof( short ), vecFile->vecSize, vecFile->input );

    if( feof( vecFile->input ) || vecFile->last++ >= vecFile->count )
    {
        return 0;
    }
    
    for( r = 0; r < img->rows; r++ )
    {
        for( c = 0; c < img->cols; c++ )
        {
            CV_MAT_ELEM( *img, uchar, r, c ) = 
                (uchar) vecFile->vector[r * img->cols + c];
        }
    }
    return 1;
}