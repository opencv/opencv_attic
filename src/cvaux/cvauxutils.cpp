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

#include "_cvaux.h"
#include <time.h>

typedef int64 (CV_CDECL * rdtsc_func)(void);

/* helper functions for RNG initialization and accurate time measurement: x86 only */
CV_IMPL  int64  cvGetTickCount( void )
{
#ifndef WIN32
    return clock();
#else
    static const char code[] = "\x0f\x31\xc3";
    rdtsc_func func = (rdtsc_func)(void*)code;
    return func();
#endif
}


CV_IMPL  double  cvGetTickFrequency()
{
#ifndef WIN32
    return CLOCKS_PER_SEC*1e-6;
#else
    int64 clocks1, clocks2;
    volatile int t;
    int dt = 100;
    int frequency = 0, old_frequency;

    do
    {
        old_frequency = frequency;
        t = clock();
        while( t==clock() );
        t = clock();

        clocks1 = cvGetTickCount();
        while( dt+t>clock() );
        clocks2 = cvGetTickCount();

        frequency = (int)(((double)(clocks2 - clocks1))/(1e3*dt)+.5) + 10;
        if( frequency % 50 <= 16 )
            frequency = (frequency/50)*50;
        else
            frequency = (frequency/100)*100 + ((frequency % 100)/33)*33;
    }
    while( frequency != old_frequency );
    return (double)frequency;
#endif
}

/* End of file. */
