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


/****************************************************************************************/
/*                This part of code is used to generate DLL function body               */
/****************************************************************************************/

/* get all the function prototypes as in static library */
#include "_cv.h"

#if _MSC_VER >= 1200
#pragma warning( disable: 4115 )        /* type definition in () */
#pragma warning( disable: 4100 )        /* unreferenced formal paramter */
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <string.h>
#endif

#include <stdio.h>

#define __CV_DEFINE_USER_LIST__

/****************************************************************************************/
/*              Implement all dll functions via static library calls                    */
/****************************************************************************************/

/****************************************************************************************/
/*                               Make functions descriptions                            */
/****************************************************************************************/

typedef void (CV_CDECL * ipp_func_addr) (void);

#undef IPCVAPI
#define GETAPI(addr,name) { (ipp_func_addr*)&addr, #name, 0 },
#define IPCVAPI(type,name,arg) GETAPI( name, name )

struct func_desc
{
    ipp_func_addr *addr;
    const char *name;
    int loaded;
};

static func_desc ipp_func_desc[] =
{
#undef __IPCV_H_
#include "_ipcv.h"
#undef __IPCV_H_
    {0, 0}
};


#undef __IPCV_H_
#undef IPCVAPI
#define IPCVAPI(type,name,arg)                          \
    *(ipp_func_desc[i].addr) = (ipp_func_addr)name##_f; \
    ipp_func_desc[i].loaded = 0;                        \
    i++;

// set all the primitive function pointers to OpenCV code.
static void icvResetPointers()
{
    int i = 0;
#include "_ipcv.h"
}

/*
   determine processor type, load appropriate dll and
   initialize all function pointers
*/
#ifdef WIN32
#define DLL_PREFIX ""
#define DLL_SUFFIX ".dll"
#else
#define DLL_PREFIX "lib"
#define DLL_SUFFIX ".so"
#define LoadLibrary(name) dlopen(name, RTLD_LAZY)
#define FreeLibrary(name) dlclose(name)
#define GetProcAddress dlsym
typedef void* HMODULE;
#endif

#if 0 /*def _DEBUG*/
#define DLL_DEBUG_FLAG "d"
#else
#define DLL_DEBUG_FLAG ""
#endif

//#define VERBOSE_LOADING

#ifdef VERBOSE_LOADING
#define ICV_PRINTF(args)  printf args
#else
#define ICV_PRINTF(args)
#endif

typedef struct CvModuleInfo
{
    const char* basename;
    HMODULE handle;
    char name[100];
}
CvModuleInfo;

static CvModuleInfo modules[] =
{
    {"ippcv", 0, "" },
    {"optcv", 0, "" },
    { 0, 0, "" }
};

static const char* prefices[] = { "ippi", "ippcv", 0 };

CV_IMPL int
cvLoadPrimitives( const char* processor_type )
{
    int i, loaded_modules = 0, loaded_functions = 0;
    CvProcessorType proc_type = CvProcessorType(processor_type);
    
    icvResetPointers();

    if( !proc_type || strcmp(proc_type, CV_PROC_GENERIC) != 0 )
        proc_type = icvGetProcessorType();

    // try to load optimized dlls
    for( i = 0; modules[i].basename; i++ )
    {
        CvProcessorType proc = proc_type;

        // unload previously loaded optimized modules
        if( modules[i].handle )
        {
            FreeLibrary( modules[i].handle );
            modules[i].handle = 0;
        }
        
        while( strcmp( proc, CV_PROC_GENERIC ) != 0 )
        {
            sprintf( modules[i].name, DLL_PREFIX "%s%s" DLL_DEBUG_FLAG DLL_SUFFIX,
                     modules[i].basename, (const char*)proc );
            
            modules[i].handle = LoadLibrary( modules[i].name );
            if( modules[i].handle != 0 )
            {
                ICV_PRINTF(("%s loaded\n", modules[i].name )); 
                loaded_modules++;
                break;
            }
            
            proc = icvPreviousProcessor( proc );
        }
    }

    if( loaded_modules > 0 )
    {
        for( i = 0; ipp_func_desc[i].name != 0; i++ )
        {
        #if _MSC_VER >= 1200
            #pragma warning( disable: 4054 4055 )   /* converting pointers to code<->data */
        #endif
            char name[100];
            uchar* addr = 0;
            int j = 0, k = 0;

            if( ipp_func_desc[i].loaded )
                continue;

            for( j = 0; prefices[j] != 0; j++ )
            {
                strcpy( name, prefices[j] );
                strcat( name, ipp_func_desc[i].name + 3 );

                for( k = 0; modules[k].basename != 0; k++ )
                    if( modules[k].handle )
                    {
                        addr = (uchar*)GetProcAddress( modules[k].handle, name );
                        if( addr )
                            break;
                    }

                if( addr )
                    break;
            }

            if( addr )
            {
            #ifdef WIN32            
                while( *addr == 0xE9 )
                    addr += 5 + *((int*)(addr + 1));
            #endif
                *ipp_func_desc[i].addr = (ipp_func_addr)addr;
                ipp_func_desc[i].loaded = k+1; // store incremented index of the module
                                               // that contain the loaded function
                loaded_functions++;
                ICV_PRINTF(("%s:  \t%s\n", ipp_func_desc[i].name, modules[k].name ));
            }

            #if _MSC_VER >= 1200
                #pragma warning( default: 4054 4055 )
            #endif
        }
    }
    
#ifdef VERBOSE_LOADING
    {
    int not_loaded = 0;
    ICV_PRINTF(("\nTotal loaded: %d\n\n", loaded_functions ));
    printf( "***************************************************\nNot loaded ...\n\n" );
    for( i = 0; ipp_func_desc[i].name != 0; i++ )
        if( !ipp_func_desc[i].loaded )
        {
            ICV_PRINTF(( "%s\n", ipp_func_desc[i].name ));
            not_loaded++;
        }

    ICV_PRINTF(("\nTotal: %d\n", not_loaded ));
    }
#endif

    return loaded_functions;
}


static int loaded_functions = cvLoadPrimitives();


void
cvGetLibraryInfo( const char **_version, int *_loaded, const char **_dll_name )
{
    static const char* version = __DATE__;
    static char loaded_modules[1000] = "";

    if( _version )
        *_version = version;
    
    if( _loaded )
        *_loaded = loaded_functions;
    
    if( _dll_name )
    {
        int i;
        
        for( i = 0; modules[i].basename; i++ )
            if( modules[i].handle != 0 )
            {
                sprintf( loaded_modules + strlen(loaded_modules),
                         ", %s", modules[i].name );
            }

        *_dll_name = strlen(loaded_modules) == 0 ? "none" :
                     (const char*)(loaded_modules + 2); // skip ", "
    }
}


CV_IMPL int
cvFillInternalFuncsTable(void* tbl)
{
    int loaded = 0;

    func_desc* table = (func_desc*)tbl;
    assert(table);

    for( int i = 0; table[i].name; i++ )
    {
        int j;
        for( j = 0;
             ipp_func_desc[j].name && strcmp( ipp_func_desc[j].name, table[i].name );
             j++ );
        if( ipp_func_desc[j].name )
        {
            *table[i].addr = *ipp_func_desc[j].addr;
            table[i].loaded = 1;
            loaded++;
        }
    }

    return loaded;
}


/* End of file. */
