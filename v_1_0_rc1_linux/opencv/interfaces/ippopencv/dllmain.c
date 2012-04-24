/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 1999-2004 Intel Corporation. All Rights Reserved.
//
*/

static const char* SET_LIB_ERR = "Set ipp library error";

#if defined( _WIN32 )
  #define STRICT
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#elif defined( linux )
#endif  /* _WIN32 */

/* Describe Intel CPUs and libraries */
typedef enum{CPU_PX=0, CPU_A6, CPU_W7, CPU_T7, CPU_NOMORE} cpu_enum;
typedef enum{LIB_PX=0, LIB_A6, LIB_W7, LIB_T7, LIB_NOMORE} lib_enum;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef __int64 int64;

#include "ippcore.h"

#if IPP < 500
#define ippGetCpuType ippCoreGetCpuType
#endif

static cpu_enum GetProcessorId()
{
   switch ( ippGetCpuType() ) {
      case ippCpuPP:
      case ippCpuPMX:
      case ippCpuPPR:  
      case ippCpuPII:   return CPU_PX;
      case ippCpuITP:   return CPU_A6;
      case ippCpuITP2:  return CPU_A6;
      case ippCpuPIII:  return CPU_A6;
      case ippCpuP4:
      case ippCpuCentrino:
      case ippCpuP4HT:  return CPU_W7;
      case ippCpuEM64T: return CPU_T7;
      case ippCpuP4HT2: return CPU_T7;
      default: return CPU_PX;
   }
}

#undef IPPAPI

#define IPPAPI(type,name,arg)
#include "ipp.h"


/* New cpu can use some libraries for old cpu */
static const lib_enum libUsage[][LIB_NOMORE+1] = {
     /*  LIB_T7, LIB_W7, LIB_A6, LIB_PX, LIB_NOMORE */
/*PX*/ {                 LIB_PX, LIB_NOMORE },
/*A6*/ {         LIB_A6, LIB_PX, LIB_NOMORE },
/*W7*/ { LIB_W7, LIB_A6, LIB_PX, LIB_NOMORE },
/*T7*/ { LIB_T7, LIB_W7, LIB_A6, LIB_PX, LIB_NOMORE }
};

#if !defined (PX) && !defined (A6) && !defined (W7) && !defined (T7)
  #error Are not defined the CPUs, following are allowed: PX, A6, W7, T7
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef PX
  #undef  IPPAPI
  #define IPPAPI(type,name,arg) extern type __stdcall px_##name arg;
  #define PX_NAME(name) (FARPROC)px_##name
  #include "opencvipp_funclist.h"
#else
  #define PX_NAME(name) NULL
#endif

#ifdef A6
  #undef  IPPAPI
  #define IPPAPI(type,name,arg) extern type __stdcall a6_##name arg;
  #define A6_NAME(name) (FARPROC)a6_##name
  #include "opencvipp_funclist.h"
#else
  #define A6_NAME(name) NULL
#endif

#ifdef W7
  #undef  IPPAPI
  #define IPPAPI(type,name,arg) extern type __stdcall w7_##name arg;
  #define W7_NAME(name) (FARPROC)w7_##name
  #include "opencvipp_funclist.h"
#else
  #define W7_NAME(name) NULL
#endif

#ifdef T7
  #undef  IPPAPI
  #define IPPAPI(type,name,arg) extern type __stdcall t7_##name arg;
  #define T7_NAME(name) (FARPROC)t7_##name
  #include "opencvipp_funclist.h"
#else
  #define T7_NAME(name) NULL
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
    static FARPROC d##name; \
    IPP_EXTERN_C __declspec(naked dllexport) void __stdcall name arg { __asm {jmp d##name } }
#include "opencvipp_funclist.h"


typedef struct _USER_Desc_t {
    FARPROC*            WorkAddr;
    FARPROC FuncAddr[CPU_NOMORE];
} USER_Desc_t;

static USER_Desc_t AddressBook[] = {
#undef  IPPAPI
#define IPPAPI(type,name,arg) &d##name, \
    PX_NAME(name), A6_NAME(name), W7_NAME(name), T7_NAME(name),
#include "opencvipp_funclist.h"
};

/* how large is the table of the functions */
static int sFuncCount  = sizeof( AddressBook ) / sizeof( AddressBook[0] );

/* fill ipp function address book in correspondence to the target cpu */
static BOOL SetLib( lib_enum lib )
{
   int i = 0;
   for ( i=0; i<sFuncCount; i++ )
      if( NULL == AddressBook[i].FuncAddr[lib] )
         return FALSE;
      else
        *(AddressBook[i].WorkAddr) = AddressBook[i].FuncAddr[lib];
   return TRUE;
}

static BOOL setCpuSpecificLib()
{
   char buf[256] = "";
   cpu_enum cpu = GetProcessorId();
   if( sFuncCount > 0 && cpu >= CPU_PX && cpu < CPU_NOMORE ) {

      const lib_enum* libs = libUsage[ cpu ];
      while( *libs < LIB_NOMORE )
         if( SetLib( *libs++ ) ) return TRUE;     /* SUCCESS EXIT */
   }
   /* if not found, then failed with error message */
   lstrcpy( buf, " No ipp matching to CPU was found during the Waterfall" );
   MessageBeep( MB_ICONSTOP );
   MessageBox( 0, buf, SET_LIB_ERR, MB_ICONSTOP | MB_OK );
   return FALSE;
}


BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason,
                    LPVOID lpvReserved )
{
    switch( fdwReason ) {
      case DLL_PROCESS_ATTACH: if( !setCpuSpecificLib() )return FALSE;

    default:
        hinstDLL;
        lpvReserved;
        break;
    }
    return TRUE;
}

/* //////////////////////// End of file "dllmain.c" ///////////////////////// */
