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

#include "_cv.h"
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <string.h>
#endif

/*
   determine processor type
*/
CvProcessorType
icvGetProcessorType( void )
{
    CvProcessorType proc_type = CV_PROC_GENERIC;

#ifdef  WIN32

    SYSTEM_INFO sys;
    GetSystemInfo( &sys );

    if( sys.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL &&
        sys.dwProcessorType == PROCESSOR_INTEL_PENTIUM )
    {
        int version = 0, features = 0, family = 0;
        int id = 0;

    #if _MSC_VER >= 1200 || defined __ICL || defined __BORLANDC__

        __asm
        {
            push ebx
            push esi
            push edi
            mov  eax, 1

    #ifndef __BORLANDC__
            _emit 0x0f
            _emit 0xa2
    #else
            db 0fh
            db 0a2h
    #endif
            pop edi
            pop esi
            pop ebx
            mov version, eax
            mov features, edx
        }

    #elif defined __GNUC__

        __asm__ __volatile__
        (
            "push %%ebx\n\t"    
            "push %%esi\n\t"
            "push %%edi\n\t"
            "movl $1, %%eax\n\t"
            "cpuid\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %1\n\t"
            "pop %%edi\n\t"
            "pop %%esi\n\t"
            "pop %%ebx\n\t"
            : "=a" (version),
              "=d" (features)
            :
        );

    #endif

        #define ICV_CPUID_M6     ((1<<15)|(1<<23)|6)  /* cmov + mmx */
        #define ICV_CPUID_A6     ((1<<25)|ICV_CPUID_M6) /* <all above> + xmm */
        #define ICV_CPUID_W7     ((1<<26)|ICV_CPUID_A6|(1<<3)|1) /* <all above> + emm */

        family = (version >> 8) & 15;
        if( family >= 6 && (features & (ICV_CPUID_M6 & ~6)) != 0 ) /* Pentium II or higher */
        {
            id = (features & ICV_CPUID_W7 & -256) | family;
        }

        switch( id )
        {
        case ICV_CPUID_W7:
            proc_type = CV_PROC_IA32_P4;
            break;
        case ICV_CPUID_A6:
            proc_type = CV_PROC_IA32_PIII;
            break;
        case ICV_CPUID_M6:
            proc_type = CV_PROC_IA32_PII;
            break;
        }
    }

#else
    char buffer[1000] = "";

    //reading /proc/cpuinfo file (proc file system must be supported)
    FILE *file = fopen( "/proc/cpuinfo", "r" );

    memset( buffer, 0, sizeof(buffer));

    if( file && fread( buffer, 1, 1000, file ))
    {
        if( strstr( buffer, "mmx" ) && strstr( buffer, "cmov" ))
        {
            proc_type = CV_PROC_IA32_PII;

            if( strstr( buffer, "xmm" ) || strstr( buffer, "sse" ))
            {
                proc_type = CV_PROC_IA32_PIII;

                if( strstr( buffer, "emm" ))
                    proc_type = CV_PROC_IA32_P4;
            }
        }
    }
#endif

    return proc_type;
}


CvProcessorType icvPreviousProcessor( const char* proc_type )
{
    char signature[100];
    int i;

    if( strlen( proc_type ) >= sizeof(signature))
        return CV_PROC_GENERIC;

    for( i = 0; proc_type[i]; i++ )
        signature[i] = (char)tolower( proc_type[i] );

    signature[i++] = '\0';

    if( !strcmp( signature, CV_PROC_IA32_P4 ))
        proc_type = CV_PROC_IA32_PIII;
    else if( !strcmp( signature, CV_PROC_IA32_PIII ))
        proc_type = CV_PROC_IA32_PII;
    else
        proc_type = CV_PROC_GENERIC;

    return proc_type;
}

/* End of file. */
