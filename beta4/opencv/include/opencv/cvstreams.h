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

#ifndef _CVSTREAMS_H_
#define _CVSTREAMS_H_

#ifdef WIN32
#include <streams.h>  /* !!! IF YOU'VE GOT AN ERROR HERE, PLEASE READ BELOW !!! */
/***************** How to get Developer Studio understand streams.h ****************\

You need DirectShow SDK that is now a part of DirectX SDK.

1. Download DirectX SDK from msdn.microsoft.com/directx/
   (It's huge, but you can download it by parts).
   If it doesn't work for you, consider HighGUI that can capture video via VFW or MIL

2. Install it TOGETHER WITH SAMPLES.

3. Open <DirectXSDKInstallFolder>\samples\C++\DirectShow\BaseClasses\baseclasses.{dsw|sln}.
   If there is no such file, it is that you either didn't install samples or the path has changed,
   in the latter case search for streams.h and open a workspace file located in the same folder.

4. Build the library in both Release in Debug configurations.

5. Copy the built libraries (in DirectX 9.x they are called strmbase.lib and strmbasd.lib)
   to <DirectXSDKInstallFolder>\lib.

6. In Developer Studio add the following paths:
      <DirectXSDKInstallFolder>\include
      <DirectXSDKInstallFolder>\samples\C++\DirectShow\BaseClasses
    to the includes' search path (at Tools->Options->Directories->Include files
                                  in case of Developer Studio 6.0)
   Add
      <DirectXSDKInstallFolder>\lib
   to the libraries' search path (at Tools->Options->Directories->Library files
                                  in case of Developer Studio 6.0)

   NOTE: PUT THE ADDED LINES ON THE VERY TOP OF THE LISTS, OTHERWISE YOU WILL STILL GET
   COMPILER OR LINKER ERRORS. This is necessary, because Developer Studio 6.0 includes some
   older DirectX headers and libs that conflict with new DirectX SDK versions. 

7. Enjoy!

\***********************************************************************************/

#endif

#endif /*_CVSTREAMS_H_*/

