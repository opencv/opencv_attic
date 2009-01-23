/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 1999-2004 Intel Corporation. All Rights Reserved.
//
*/

#define ADD_W7
#define ADD_V8
#define ADD_P8

/* Describe Intel CPUs and libraries */
enum{ LIB_W7=0, LIB_V8, LIB_P8, LIB_MAX };

#if defined( _WIN32 )
  #define STRICT
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#elif defined( linux )
#endif  /* _WIN32 */

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef __int64 int64;

#include "ippcore.h"

#undef IPPAPI

#define IPPAPI(type,name,arg)
#include "ipp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ADD_W7
    #undef  IPPAPI
    #define IPPAPI(type,name,arg) extern type __stdcall w7_##name arg;
    #define W7_NAME(name) (FARPROC)w7_##name
    #include "opencvipp_funclist.h"
#else
    #define W7_NAME(name) NULL
#endif

#ifdef ADD_V8 
    #undef  IPPAPI
    #define IPPAPI(type,name,arg) extern type __stdcall v8_##name arg;
    #define V8_NAME(name) (FARPROC)v8_##name
    #include "opencvipp_funclist.h"
#else
    #define V8_NAME(name) NULL
#endif

#ifdef ADD_P8 
    #undef  IPPAPI
    #define IPPAPI(type,name,arg) extern type __stdcall p8_##name arg;
    #define P8_NAME(name) (FARPROC)p8_##name
    #include "opencvipp_funclist.h"
#else
    #define P8_NAME(name) NULL
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define IPP_EXTERN_C extern "C"
#else
#define IPP_EXTERN_C
#endif

#undef  IPPAPI
#define IPPAPI(type,name,arg) \
    static FARPROC d##name=0; \
    IPP_EXTERN_C __declspec(naked dllexport) void __stdcall name arg { __asm {jmp d##name } }
#include "opencvipp_funclist.h"

typedef struct _USER_Desc_t {
    FARPROC*            WorkAddr;
    FARPROC FuncAddr[LIB_MAX];
} USER_Desc_t;

static USER_Desc_t AddressBook[] = {
#undef  IPPAPI
#define IPPAPI(type,name,arg) &d##name, \
    W7_NAME(name), V8_NAME(name), P8_NAME(name),
#include "opencvipp_funclist.h"
};

#include "ippdefs.h"

/* fill ipp function address book in correspondence to the target cpu */
static BOOL setCpuSpecificLib()
{
    Ipp64u mask = 0;
    Ipp32u cpuInfoRegs[4] = {0,0,0,0};
    int i, j, lib_id, nfuncs = (int)(sizeof(AddressBook)/sizeof(AddressBook[0]));
    ippGetCpuFeatures( &mask, cpuInfoRegs );
    lib_id = (mask & ippCPUID_SSE41) ? LIB_P8 :
             (mask & ippCPUID_SSSE3) ? LIB_V8 :
             (mask & ippCPUID_SSE2) ? LIB_W7 : -1;
    
    for( i = lib_id; i >= 0; i++ )
    {
        for( j = 0; j < nfuncs; j++ )
        {
            if( !AddressBook[j].FuncAddr[i] )
                break;
            *(AddressBook[j].WorkAddr) = AddressBook[j].FuncAddr[i];
        }
        if( j >= nfuncs )
            break;
    }
    if( i >= 0 )
        return TRUE;
    MessageBox( 0, "No ipp matching to CPU was found during the Waterfall",
        "opencv_ipp loading error", MB_ICONSTOP | MB_OK );
    return FALSE;
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    switch( fdwReason ) {
      case DLL_PROCESS_ATTACH: return setCpuSpecificLib();

    default:
        hinstDLL;
        lpvReserved;
        break;
    }
    return TRUE;
}

/* //////////////////////// End of file "dllmain.c" ///////////////////////// */
