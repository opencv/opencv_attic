#ifndef _IMAGESTORAGE_H
#define _IMAGESTORAGE_H

#include <highgui.h>

//------------------- Background reading ---------------------
struct CvBackgroundReader
{
    CvBackgroundReader();
    virtual ~CvBackgroundReader();

    CvMat   src;
    CvMat   img;
    CvPoint offset;
    float   scale;
    float   scaleFactor;
    float   stepFactor;
    CvPoint point;
};

struct CvBackgroundData
{
    CvBackgroundData();
    CvBackgroundData( const char* fileName, CvSize _winSize );
    virtual ~CvBackgroundData();
    bool getImage( CvMat* img, bool reset );
    bool getNext( bool reset);

    CvBackgroundReader* bgReader;
    int    count;
    char** fileName;
    int    last;
    int    round;
    CvSize winSize;
};

//--------------------- VecFile reading ------------------------
struct CvVecFile
{
    CvVecFile();
    CvVecFile( const char* _vecFileName );
    virtual ~CvVecFile();

    FILE*  input;
    int    count;
    int    vecSize;
    int    last;
    short* vector;
    int base;
};

//-----------------------  CvImageReader  -----------------------
class CvImageReader
{
public:
    CvImageReader( const char* _vecFileName, const char* _bgfileName, CvSize _winSize );
    virtual ~CvImageReader();

    bool getNegImage(CvMat* img, bool reset = false);
    bool getPosImage(CvMat* img, bool reset = false);
private:
    CvBackgroundData* bgData;
    CvVecFile* vecFile;
};

#endif