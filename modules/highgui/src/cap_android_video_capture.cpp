/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"
//#include "test.h"


#include <android/log.h>

#include <dlfcn.h>



#if !defined(LOGD) && !defined(LOGI) && !defined(LOGE)
#define LOG_TAG "CV_WRITER_ANDROID"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif


#define DEFAULT_WRAPPER_PACKAGE_NAME "org.itseez.opencv"
#define DEFAULT_PATH_LIB_FOLDER "/data/data/" DEFAULT_WRAPPER_PACKAGE_NAME "/lib/"

#define ANDROID_VIDEO_LIBRARY_NAME "libandroidvideo.so"

typedef void* (*prepareVideoRecorder_t)(char* fileName, int width, int height);
typedef void (*destroyVideoRecorder_t)(void* context);
typedef bool (*writeVideRecorderNextFrame_t)(void* context, char* buffer, int size);


CvVideoWriter* cvCreateVideoWriter_Android(const char* filename, int fourcc, double fps, CvSize frameSize, int is_color );


/** Definition of the AndroidVideoLibConnector class for Android. ******
 * To avoid direct dependency on the Android headers this class is a proxy around
 * the functios provided by a separate libandroidvideo.so object. 
 * This class connects to library, checks that the required video writer can be created.
 * This class is a singleton.
****/
class AndroidVideoLibConnector
{
public:
    ~AndroidVideoLibConnector();

    static AndroidVideoLibConnector* getInstance();    

    bool connectToLib();


    /* Record control functions */
    void* prepareVideoRecorder(char* fileName, int width, int height);
    bool writeVideRecorderNextFrame(void* context, const IplImage* image);
    void destroyVideoRecorder(void* context);

private:
    AndroidVideoLibConnector();

    std::string getPathLibFolder();
    void* getSymbolAdress(void* libHandle, char* name);

    std::string pathLibFolder;
    bool mIsConnected;

    static AndroidVideoLibConnector* sInstance;

    prepareVideoRecorder_t prepareRecorder;
    destroyVideoRecorder_t destroyRecorder;
    writeVideRecorderNextFrame_t writeRecorderNextFrame;

    void* libHandle;

};

AndroidVideoLibConnector* AndroidVideoLibConnector::sInstance = NULL;

/************** AndroidVideoLibConnector methods implementation ********************/

AndroidVideoLibConnector::AndroidVideoLibConnector()
{
    /* Do this at the constructor. As this is a singleton it will be executed just once */
    pathLibFolder = getPathLibFolder();
    mIsConnected = false;
}

AndroidVideoLibConnector::~AndroidVideoLibConnector()
{
    /* TODO: Release the library handle */
}
bool AndroidVideoLibConnector::connectToLib()
{
    if (mIsConnected) return true;

    /* Not connected yet. Need to connect and initalize */
    std::string cur_path = pathLibFolder + ANDROID_VIDEO_LIBRARY_NAME;
    LOGD("try to load library '%s'", cur_path.c_str());
    libHandle = dlopen(cur_path.c_str(), RTLD_LAZY);
    if (libHandle) {
        LOGD("Loaded library '%s'", cur_path.c_str());
        mIsConnected = true;
        /* Now get the functions from the library: PREPARE, WRITE_FRAME, STOP */
        /*TODO: Check that all symbols are present */
        prepareRecorder = (prepareVideoRecorder_t)getSymbolAdress(libHandle, "prepareVideoRecorder");
        destroyRecorder = (destroyVideoRecorder_t)getSymbolAdress(libHandle, "destroyVideoRecorder");
        writeRecorderNextFrame = (writeVideRecorderNextFrame_t)getSymbolAdress(libHandle, "writeVideRecorderNextFrame");
        return true;

    } else {
        LOGD("AndroidVideoLibConnector::connectToLib ERROR: cannot dlopen video library %s, dlerror=\"%s\"",
             cur_path.c_str(), dlerror());
        return false;
    }

}

void* AndroidVideoLibConnector::getSymbolAdress(void* libHandle, char* symbolName)
{
    dlerror();
    void * pSymbol = dlsym(libHandle, symbolName);

    const char* error_dlsym_init=dlerror();
    if (error_dlsym_init) {
        LOGE("AndroidVideoLibConnector::getSymbolFromLib ERROR: cannot get symbol of the function '%s' from the camera wrapper library, dlerror=\"%s\"",
             symbolName, error_dlsym_init);
        return NULL;
    }
    return pSymbol;
}

std::string AndroidVideoLibConnector::getPathLibFolder()
{
    if (!pathLibFolder.empty())
        return pathLibFolder;

    Dl_info dl_info;
    if(0 != dladdr((void *)cvCreateVideoWriter_Android, &dl_info))
    {
        LOGD("Library name: %s", dl_info.dli_fname);
        LOGD("Library base address: %p", dl_info.dli_fbase);

        const char* libName=dl_info.dli_fname;
        while( ((*libName)=='/') || ((*libName)=='.') )
            libName++;

        char lineBuf[2048];
        FILE* file = fopen("/proc/self/smaps", "rt");

        if(file)
        {
            while (fgets(lineBuf, sizeof lineBuf, file) != NULL)
            {
                //verify that line ends with library name
                int lineLength = strlen(lineBuf);
                int libNameLength = strlen(libName);

                //trim end
                for(int i = lineLength - 1; i >= 0 && isspace(lineBuf[i]); --i)
                {
                    lineBuf[i] = 0;
                    --lineLength;
                }

                if (0 != strncmp(lineBuf + lineLength - libNameLength, libName, libNameLength))
                {
                    //the line does not contain the library name
                    continue;
                }

                //extract path from smaps line
                char* pathBegin = strchr(lineBuf, '/');
                if (0 == pathBegin)
                {
                    LOGE("Strange error: could not find path beginning in lin \"%s\"", lineBuf);
                    continue;
                }

                char* pathEnd = strrchr(pathBegin, '/');
                pathEnd[1] = 0;

                LOGD("Libraries folder found: %s", pathBegin);

                fclose(file);
                return pathBegin;
            }
            fclose(file);
            LOGE("Could not find library path.");
        }
        else
        {
            LOGE("Could not read /proc/self/smaps");
        }
    }
    else
    {
        LOGE("Could not get library name and base address.");
    }

    return DEFAULT_PATH_LIB_FOLDER ;
}


void* AndroidVideoLibConnector::prepareVideoRecorder(char* fileName, int width, int height)
{
    if (prepareRecorder != NULL)
    {
        void* res;
        res = prepareRecorder(fileName, width, height);

        return res; 
    }
    return NULL;
}
bool AndroidVideoLibConnector::writeVideRecorderNextFrame(void* context, const IplImage* image)
{
    if (writeRecorderNextFrame != NULL)
    {
        char* buffer = NULL;
        int size = 0;

        /*TODO: Convert the image from IplImage to an array of bytes of the RGBA format and pass it to the library for recording */
        return writeRecorderNextFrame(context, buffer, size);
    }
    return false;
}
/**
 * Method to destroy the recorder. It will stop recording, close the output file and release
 * reources associated with recorder.
 */
void AndroidVideoLibConnector::destroyVideoRecorder(void* context)
{
    if (destroyRecorder!= NULL)
    {
        destroyRecorder(context);        
    }
}

/**
 * AndroidVideoLibConnector is a singleton class in applications, as only one connection to lib required.
 * At the same time many video writers can be created at a time.
 */

AndroidVideoLibConnector* AndroidVideoLibConnector::AndroidVideoLibConnector::getInstance()
{
    if (sInstance == NULL)
    {
        sInstance = new AndroidVideoLibConnector();
    }
    return sInstance;
}



/**
 * Implemenation of the CvVideoWriter interface for Android framework.
 * It is using AndroidVideoLibConnector class to access native library communicating with
 * Android services and libs.
 */
class CvVideoWriter_Android : public CvVideoWriter
{
public:
    CvVideoWriter_Android(const char* filename, int fourcc, double fps, CvSize frameSize);
    virtual ~CvVideoWriter_Android();
    virtual bool writeFrame(const IplImage*);
    
    virtual bool prepare();

private:

    enum {

        STATE_INITIAL,
        STATE_CONNECTED,
        STATE_PREPARED
    };

    char* mFileName;
    int mFourcc;
    int mFps;
    int mWidth;
    int mHeight;


    int mState;
    void* mContext;

    AndroidVideoLibConnector* mConnector;

};

/********************** Implementation of the video writer functions ************************************/
CvVideoWriter_Android::CvVideoWriter_Android(const char* filename, int fourcc, double fps, CvSize frameSize)
{
    mFileName = strdup(filename);
    mFourcc = fourcc;
    mFps = (int)fps;
    mWidth = frameSize.width;
    mHeight = frameSize.height;
    mState = STATE_INITIAL;

    mConnector = AndroidVideoLibConnector::getInstance();

    LOGI("Instantiated Android Video Writer for %dx%d at %d FPS", mWidth, mHeight, mFps);
}
CvVideoWriter_Android::~CvVideoWriter_Android()
{
    if (mState == STATE_PREPARED)
    {
        mConnector->destroyVideoRecorder(mContext);
        mContext = NULL;
        mState = STATE_CONNECTED;
    }
    if (mState == STATE_CONNECTED)
    {
        /* Nothing specific to do right now */
	    mState = STATE_INITIAL;
    }

    if (mFileName) delete mFileName;

    LOGI("Deleted Android Video Writer");
}

bool CvVideoWriter_Android::prepare()
{   
    bool result = false;
    if (mConnector->connectToLib()) 
    {
        mState = STATE_CONNECTED;
        /* connected to library. It is safe to proceed */
        /*TODO: Add support for the FPS and fourcc checking */
        if ((mContext = mConnector->prepareVideoRecorder(mFileName, mWidth, mHeight)) != NULL)
        {
            mState = STATE_PREPARED;
            result = true;
        }
    }
    return result;
}

bool CvVideoWriter_Android::writeFrame(const IplImage* image)
{
    if (mState == STATE_PREPARED)
    {
        return mConnector->writeVideRecorderNextFrame(mContext, image);
    }
    else
    {
        /* Can't write frames to non initialized writer */
        return false;
    }
}


CvVideoWriter* cvCreateVideoWriter_Android(const char* filename, int fourcc, double fps, CvSize frameSize, int is_color )
{
    CvVideoWriter_Android* object = NULL;

    object = new CvVideoWriter_Android(filename, fourcc, fps, frameSize);
    if (object != NULL)
    {
        if (!object->prepare())
        {
            /* Failed to prepare object. Either lib is not linked or some parameters are not accepted. Destroy object */
            delete object;
            object = NULL;
        }

    }
    return object;

}

