#include "info.h"

#ifndef LIB_STRING
    #define LIB_STRING "libtbb.so;libopencv_java.so"
#endif

const char* GetLibraryList()
{
    return LIB_STRING;
}