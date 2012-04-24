/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
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

/* ////////////////////////////////////////////////////////////////////
//
//  CvArr color space conversions
//
// */

/********************************* COPYRIGHT NOTICE *******************************\
  The function for RGB2Lab conversion is based on the MATLAB code
  RGB2Lab.m translated by Mark Ruzon from C code by Yossi Rubner, 23 September 1997.
  See the page [http://vision.stanford.edu/~ruzon/software/rgblab.html]
\**********************************************************************************/


/********************************* COPYRIGHT NOTICE *******************************\
  Original code for Bayer->BGR/RGB conversion is provided by Dirk Schaefer
  from MD-Mathematische Dienste GmbH. Below is the copyright notice:

    IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
    By downloading, copying, installing or using the software you agree
    to this license. If you do not agree to this license, do not download,
    install, copy or use the software.

    Contributors License Agreement:

      Copyright (c) 2002,
      MD-Mathematische Dienste GmbH
      Im Defdahl 5-10
      44141 Dortmund
      Germany
      www.md-it.de
  
    Redistribution and use in source and binary forms,
    with or without modification, are permitted provided
    that the following conditions are met: 

    Redistributions of source code must retain
    the above copyright notice, this list of conditions and the following disclaimer. 
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 
    The name of Contributor may not be used to endorse or promote products
    derived from this software without specific prior written permission. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.
\**********************************************************************************/

#include "_cv.h"

#define  ICV_CVT_BGR2RGB( src, dst )                        \
{                                                           \
    uchar t0 = (src)[0], t1 = (src)[1], t2 = (src)[2];      \
                                                            \
    (dst)[0] = t2;                                          \
    (dst)[1] = t1;                                          \
    (dst)[2] = t0;                                          \
}

#define  ICV_CVT_RGBA2BGR  ICV_CVT_BGR2RGB

#define  ICV_CVT_BGR2BGRA( src, dst )                       \
{                                                           \
    uchar t0 = (src)[0], t1 = (src)[1], t2 = (src)[2];      \
                                                            \
    (dst)[0] = t0;                                          \
    (dst)[1] = t1;                                          \
    (dst)[2] = t2;                                          \
    (dst)[3] = 0;                                           \
}

#define  ICV_CVT_BGRA2BGR( src, dst )                       \
{                                                           \
    uchar t0 = (src)[0], t1 = (src)[1], t2 = (src)[2];      \
                                                            \
    (dst)[0] = t0;                                          \
    (dst)[1] = t1;                                          \
    (dst)[2] = t2;                                          \
}

#define  ICV_CVT_BGR2RGBA( src, dst )                       \
{                                                           \
    uchar t0 = (src)[0], t1 = (src)[1], t2 = (src)[2];      \
                                                            \
    (dst)[0] = t2;                                          \
    (dst)[1] = t1;                                          \
    (dst)[2] = t0;                                          \
    (dst)[3] = 0;                                           \
}


#define  ICV_CVT_BGRA2RGBA( src, dst )                                  \
{                                                                       \
    uchar t0 = (src)[0], t1 = (src)[1], t2 = (src)[2], t3 = (src)[3];   \
                                                                        \
    (dst)[0] = t2;                                                      \
    (dst)[1] = t1;                                                      \
    (dst)[2] = t0;                                                      \
    (dst)[3] = t3;                                                      \
}


#define ICV_CVT_BGR2BGR565( src, dst )                      \
{                                                           \
    int  t = ((src)[0] >> 3)|(((src)[1] & ~3) << 3)|        \
             (((src)[2] & ~7) << 8);                        \
                                                            \
    ((ushort*)(dst))[0] = (ushort)t;                        \
}


#define ICV_CVT_RGB2BGR565( src, dst )                      \
{                                                           \
    int  t = ((src)[2] >> 3)|(((src)[1] & ~3) << 3)|        \
             (((src)[0] & ~7) << 8);                        \
                                                            \
    ((ushort*)(dst))[0] = (ushort)t;                        \
}


#define ICV_CVT_BGRA2BGR565( src, dst )                     \
{                                                           \
    int  t = ((src)[0] >> 3)|(((src)[1] & ~3) << 3)|        \
             (((src)[2] & ~7) << 8);                        \
                                                            \
    ((ushort*)(dst))[0] = (ushort)t;                        \
}


#define ICV_CVT_RGBA2BGR565( src, dst )                     \
{                                                           \
    int  t = ((src)[2] >> 3)|(((src)[1] & ~3) << 3)|        \
             (((src)[0] & ~7) << 8);                        \
                                                            \
    ((ushort*)(dst))[0] = (ushort)t;                        \
}


#define ICV_CVT_BGR5652BGR( src, dst )                      \
{                                                           \
    unsigned t = ((ushort*)(src))[0];                       \
                                                            \
    (dst)[0] = (uchar)(t << 3);                             \
    (dst)[1] = (uchar)((t >> 3) & ~3);                      \
    (dst)[2] = (uchar)((t >> 8) & ~7);                      \
}


#define ICV_CVT_BGR5652RGB( src, dst )                      \
{                                                           \
    unsigned t = ((ushort*)(src))[0];                       \
                                                            \
    (dst)[2] = (uchar)(t << 3);                             \
    (dst)[1] = (uchar)((t >> 3) & ~3);                      \
    (dst)[0] = (uchar)((t >> 8) & ~7);                      \
}


#define ICV_CVT_BGR5652BGRA( src, dst )                     \
{                                                           \
    unsigned t = ((ushort*)(src))[0];                       \
                                                            \
    (dst)[0] = (uchar)(t << 3);                             \
    (dst)[1] = (uchar)((t >> 3) & ~3);                      \
    (dst)[2] = (uchar)((t >> 8) & ~7);                      \
    (dst)[3] = 0;                                           \
}


#define ICV_CVT_BGR5652RGBA( src, dst )                     \
{                                                           \
    unsigned t = ((ushort*)(src))[0];                       \
                                                            \
    (dst)[2] = (uchar)(t << 3);                             \
    (dst)[1] = (uchar)((t >> 3) & ~3);                      \
    (dst)[0] = (uchar)((t >> 8) & ~7);                      \
    (dst)[3] = 0;                                           \
}


#define fix(x,n)      (int)((x)*(1 << (n)) + 0.5)
#define descale       CV_DESCALE

/* BGR/RGB -> Gray */
#define shift  10
#define cscGr  fix(0.212671,shift)
#define cscGg  fix(0.715160,shift)
#define cscGb  fix(0.072169,shift)

#define  ICV_CVT_BGR5652GRAY( src, dst )                        \
{                                                               \
    int t = ((ushort*)(src))[0];                                \
    t = ((t << 3) & 0xf8)*cscGb + ((t >> 3) & 0xfc)*cscGg +     \
        ((t >> 8) & 0xf8)*cscGr;                                \
    t = descale(t,shift);                                       \
    (dst)[0] = CV_FAST_CAST_8U(t);                              \
}

#define  ICV_CVT_BGRA2GRAY( src, dst )                          \
{                                                               \
    int t = (src)[0]*cscGb + (src)[1]*cscGg + (src)[2]*cscGr;   \
    t = descale(t,shift);                                       \
    (dst)[0] = CV_FAST_CAST_8U(t);                              \
}

#define  ICV_CVT_BGR2GRAY( src, dst )                           \
{                                                               \
    int t = (src)[0]*cscGb + (src)[1]*cscGg + (src)[2]*cscGr;   \
    t = descale(t,shift);                                       \
    (dst)[0] = CV_FAST_CAST_8U(t);                              \
}

#define  ICV_CVT_RGB2GRAY( src, dst )                           \
{                                                               \
    int t = (src)[0]*cscGr + (src)[1]*cscGg + (src)[2]*cscGb;   \
    t = descale(t,shift);                                       \
    (dst)[0] = CV_FAST_CAST_8U(t);                              \
}

#define  ICV_CVT_RGBA2GRAY( src, dst )                          \
{                                                               \
    int t = (src)[0]*cscGr + (src)[1]*cscGg + (src)[2]*cscGb;   \
    t = descale(t,shift);                                       \
    (dst)[0] = CV_FAST_CAST_8U(t);                              \
}

#define  ICV_CVT_GRAY2BGR565( src, dst )          \
{                                                 \
    int t = (src)[0];                             \
    t = (t >> 3)|((t & ~3) << 3)|((t & ~7) << 8); \
                                                  \
    ((ushort*)(dst))[0] = (ushort)t;              \
}


#define  ICV_CVT_GRAY2BGR( src, dst )       \
{                                           \
    uchar t = (src)[0];                     \
    (dst)[0] = (dst)[1] = (dst)[2] = t;     \
}

#define  ICV_CVT_GRAY2BGRA( src, dst )      \
{                                           \
    uchar t = (src)[0];                     \
    (dst)[0] = (dst)[1] = (dst)[2] = t;     \
    (dst)[3] = 0;                           \
}


/* BGR/RGB -> YCrCb */
#define yuvYr  fix(0.299,shift)
#define yuvYg  fix(0.587,shift)
#define yuvYb  fix(0.114,shift)
#define yuvCr  fix(0.713,shift)
#define yuvCb  fix(0.564,shift)


#define  ICV_CVT_BGR2YCrCb_EX( src, dst, blue_idx )         \
{                                                           \
    int b = (src)[blue_idx], r = (src)[2^blue_idx], y;      \
                                                            \
    y = descale(b*yuvYb + (src)[1]*yuvYg + r*yuvYr,shift);  \
    r = descale((r - y)*yuvCr + (128 << shift),shift);      \
    b = descale((b - y)*yuvCb + (128 << shift),shift);      \
                                                            \
    (dst)[0] = CV_FAST_CAST_8U(y);                          \
    (dst)[1] = CV_FAST_CAST_8U(r);                          \
    (dst)[2] = CV_FAST_CAST_8U(b);                          \
}


#define  ICV_CVT_BGR2YCrCb( src, dst )   ICV_CVT_BGR2YCrCb_EX( src, dst, 0 )
#define  ICV_CVT_RGB2YCrCb( src, dst )   ICV_CVT_BGR2YCrCb_EX( src, dst, 2 )


#define  yuvRCr   fix(1.403,shift)
#define  yuvGCr   (-fix(0.344,shift))
#define  yuvGCb   (-fix(0.714,shift))
#define  yuvBCb   fix(1.773,shift)

#define  ICV_CVT_YCrCb2BGR_EX( src, dst, blue_idx )     \
{                                                       \
    int Y = (src)[0] << shift, Cr = (src)[1] - 128,     \
        Cb = (src)[2] - 128;                            \
    int b, g, r;                                        \
                                                        \
    b = descale( Y + yuvBCb*Cb, shift );                \
    g = descale( Y + yuvGCr*Cr + yuvGCb*Cb, shift );    \
    r = descale( Y + yuvRCr*Cr, shift );                \
                                                        \
    (dst)[blue_idx] = CV_CAST_8U(b);                    \
    (dst)[1] = CV_CAST_8U(g);                           \
    (dst)[blue_idx^2] = CV_CAST_8U(r);                  \
}

#define  ICV_CVT_YCrCb2BGR( src, dst )  ICV_CVT_YCrCb2BGR_EX( src, dst, 0 )
#define  ICV_CVT_YCrCb2RGB( src, dst )  ICV_CVT_YCrCb2BGR_EX( src, dst, 2 )

#define xyzXr  fix(0.412411,shift)
#define xyzXg  fix(0.357585,shift)
#define xyzXb  fix(0.180454,shift)

#define xyzYr  fix(0.212649,shift)
#define xyzYg  fix(0.715169,shift)
#define xyzYb  fix(0.072182,shift)

#define xyzZr  fix(0.019332,shift)
#define xyzZg  fix(0.119195,shift)
#define xyzZb  fix(0.950390,shift)


#define  ICV_CVT_BGR2XYZ_EX( src, dst, blue_idx )       \
{                                                       \
    int b = (src)[blue_idx], g = (src)[1],              \
        r = (src)[2^blue_idx];                          \
    int x, y, z;                                        \
                                                        \
    x = descale( b*xyzXb + g*xyzXg + r*xyzXr, shift );  \
    y = descale( b*xyzYb + g*xyzYg + r*xyzYr, shift );  \
    z = descale( b*xyzZb + g*xyzZg + r*xyzZr, shift );  \
                                                        \
    (dst)[0] = CV_FAST_CAST_8U(x);                      \
    (dst)[1] = CV_FAST_CAST_8U(y);                      \
    (dst)[2] = CV_FAST_CAST_8U(z);                      \
}

#define  ICV_CVT_BGR2XYZ( src, dst )  ICV_CVT_BGR2XYZ_EX( src, dst, 0 )
#define  ICV_CVT_RGB2XYZ( src, dst )  ICV_CVT_BGR2XYZ_EX( src, dst, 2 )

#define  xyzRx   fix(3.240479,shift)
#define  xyzRy   (-fix(1.53715,shift))
#define  xyzRz   (-fix(0.498535,shift))

#define  xyzGx   (-fix(0.969256,shift))
#define  xyzGy   fix(1.875991,shift)
#define  xyzGz   fix(0.041556,shift)

#define  xyzBx   fix(0.055648,shift)
#define  xyzBy   (-fix(0.204043,shift))
#define  xyzBz   fix(1.057311,shift)


#define  ICV_CVT_XYZ2BGR_EX( src, dst, blue_idx )       \
{                                                       \
    int x = (src)[0], y = (src)[1], z = (src)[2];       \
    int b, g, r;                                        \
                                                        \
    b = descale( x*xyzBx + y*xyzBy + z*xyzBz, shift );  \
    g = descale( x*xyzGx + y*xyzGy + z*xyzGz, shift );  \
    r = descale( x*xyzRx + y*xyzRy + z*xyzRz, shift );  \
                                                        \
    (dst)[blue_idx] = CV_CAST_8U(b);                    \
    (dst)[1] = CV_CAST_8U(g);                           \
    (dst)[blue_idx^2] = CV_CAST_8U(r);                  \
}


#define  ICV_CVT_XYZ2BGR( src, dst )  ICV_CVT_XYZ2BGR_EX( src, dst, 0 )
#define  ICV_CVT_XYZ2RGB( src, dst )  ICV_CVT_XYZ2BGR_EX( src, dst, 2 )

#undef hsv_shift
#define hsv_shift 12

static const int div_table[] = {
    0, 1044480, 522240, 348160, 261120, 208896, 174080, 149211,
    130560, 116053, 104448, 94953, 87040, 80345, 74606, 69632,
    65280, 61440, 58027, 54973, 52224, 49737, 47476, 45412,
    43520, 41779, 40172, 38684, 37303, 36017, 34816, 33693,
    32640, 31651, 30720, 29842, 29013, 28229, 27486, 26782,
    26112, 25475, 24869, 24290, 23738, 23211, 22706, 22223,
    21760, 21316, 20890, 20480, 20086, 19707, 19342, 18991,
    18651, 18324, 18008, 17703, 17408, 17123, 16846, 16579,
    16320, 16069, 15825, 15589, 15360, 15137, 14921, 14711,
    14507, 14308, 14115, 13926, 13743, 13565, 13391, 13221,
    13056, 12895, 12738, 12584, 12434, 12288, 12145, 12006,
    11869, 11736, 11605, 11478, 11353, 11231, 11111, 10995,
    10880, 10768, 10658, 10550, 10445, 10341, 10240, 10141,
    10043, 9947, 9854, 9761, 9671, 9582, 9495, 9410,
    9326, 9243, 9162, 9082, 9004, 8927, 8852, 8777,
    8704, 8632, 8561, 8492, 8423, 8356, 8290, 8224,
    8160, 8097, 8034, 7973, 7913, 7853, 7795, 7737,
    7680, 7624, 7569, 7514, 7461, 7408, 7355, 7304,
    7253, 7203, 7154, 7105, 7057, 7010, 6963, 6917,
    6872, 6827, 6782, 6739, 6695, 6653, 6611, 6569,
    6528, 6487, 6447, 6408, 6369, 6330, 6292, 6254,
    6217, 6180, 6144, 6108, 6073, 6037, 6003, 5968,
    5935, 5901, 5868, 5835, 5803, 5771, 5739, 5708,
    5677, 5646, 5615, 5585, 5556, 5526, 5497, 5468,
    5440, 5412, 5384, 5356, 5329, 5302, 5275, 5249,
    5222, 5196, 5171, 5145, 5120, 5095, 5070, 5046,
    5022, 4998, 4974, 4950, 4927, 4904, 4881, 4858,
    4836, 4813, 4791, 4769, 4748, 4726, 4705, 4684,
    4663, 4642, 4622, 4601, 4581, 4561, 4541, 4522,
    4502, 4483, 4464, 4445, 4426, 4407, 4389, 4370,
    4352, 4334, 4316, 4298, 4281, 4263, 4246, 4229,
    4212, 4195, 4178, 4161, 4145, 4128, 4112, 4096
};


#define  ICV_CVT_BGR2HSV_EX( src, dst, blue_idx )                           \
{                                                                           \
    int b = (src)[blue_idx], g = (src)[1], r = (src)[2^blue_idx];           \
    int h, s, v;                                                            \
                                                                            \
    int vmin, diff;                                                         \
    int vr, vg;                                                             \
                                                                            \
    v = CV_IMAX( r, g );                                                    \
    v = CV_IMAX( v, b );                                                    \
    vmin = CV_IMIN( r, g );                                                 \
    vmin = CV_IMIN( vmin, b );                                              \
                                                                            \
    diff = v - vmin;                                                        \
    vr = v == r ? -1 : 0;                                                   \
    vg = v == g ? -1 : 0;                                                   \
                                                                            \
    s = diff * div_table[v] >> hsv_shift;                                   \
    h = (vr & (g - b)) +                                                    \
        (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff)))); \
    h = ((h * div_table[diff] * 15 + (1 << (hsv_shift + 6))) >> (7 + hsv_shift))\
        + (h < 0 ? 30*6 : 0);                                               \
                                                                            \
    (dst)[0] = (uchar)h;                                                    \
    (dst)[1] = (uchar)s;                                                    \
    (dst)[2] = (uchar)v;                                                    \
}

#define  ICV_CVT_BGR2HSV( src, dst )  ICV_CVT_BGR2HSV_EX( src, dst, 0 )
#define  ICV_CVT_RGB2HSV( src, dst )  ICV_CVT_BGR2HSV_EX( src, dst, 2 )

#undef lab_shift
#define lab_shift 10

#define labXr  fix(0.43391,lab_shift)
#define labXg  fix(0.37622,lab_shift)
#define labXb  fix(0.18986,lab_shift)

#define labYr  fix(0.212649,lab_shift)
#define labYg  fix(0.715169,lab_shift)
#define labYb  fix(0.072182,lab_shift)

#define labZr  fix(0.017756,lab_shift)
#define labZg  fix(0.109478,lab_shift)
#define labZb  fix(0.872915,lab_shift)

#define labT   fix(2.258279,lab_shift)

#define labSmallScale fix(31.27,lab_shift)
#define labSmallShift fix(141.24138,lab_shift)
#define labLScale fix(295.8,lab_shift)
#define labLShift fix(41779.2,lab_shift)

/* 1024*(([0..511]./255)**(1./3)) */
static ushort icvLabCubeRootTab[] = {
   0,  161,  203,  232,  256,  276,  293,  308,  322,  335,  347,  359,  369,  379,  389,  398,
 406,  415,  423,  430,  438,  445,  452,  459,  465,  472,  478,  484,  490,  496,  501,  507,
 512,  517,  523,  528,  533,  538,  542,  547,  552,  556,  561,  565,  570,  574,  578,  582,
 586,  590,  594,  598,  602,  606,  610,  614,  617,  621,  625,  628,  632,  635,  639,  642,
 645,  649,  652,  655,  659,  662,  665,  668,  671,  674,  677,  680,  684,  686,  689,  692,
 695,  698,  701,  704,  707,  710,  712,  715,  718,  720,  723,  726,  728,  731,  734,  736,
 739,  741,  744,  747,  749,  752,  754,  756,  759,  761,  764,  766,  769,  771,  773,  776,
 778,  780,  782,  785,  787,  789,  792,  794,  796,  798,  800,  803,  805,  807,  809,  811,
 813,  815,  818,  820,  822,  824,  826,  828,  830,  832,  834,  836,  838,  840,  842,  844,
 846,  848,  850,  852,  854,  856,  857,  859,  861,  863,  865,  867,  869,  871,  872,  874,
 876,  878,  880,  882,  883,  885,  887,  889,  891,  892,  894,  896,  898,  899,  901,  903,
 904,  906,  908,  910,  911,  913,  915,  916,  918,  920,  921,  923,  925,  926,  928,  929,
 931,  933,  934,  936,  938,  939,  941,  942,  944,  945,  947,  949,  950,  952,  953,  955,
 956,  958,  959,  961,  962,  964,  965,  967,  968,  970,  971,  973,  974,  976,  977,  979,
 980,  982,  983,  985,  986,  987,  989,  990,  992,  993,  995,  996,  997,  999, 1000, 1002,
1003, 1004, 1006, 1007, 1009, 1010, 1011, 1013, 1014, 1015, 1017, 1018, 1019, 1021, 1022, 1024,
1025, 1026, 1028, 1029, 1030, 1031, 1033, 1034, 1035, 1037, 1038, 1039, 1041, 1042, 1043, 1044,
1046, 1047, 1048, 1050, 1051, 1052, 1053, 1055, 1056, 1057, 1058, 1060, 1061, 1062, 1063, 1065,
1066, 1067, 1068, 1070, 1071, 1072, 1073, 1074, 1076, 1077, 1078, 1079, 1081, 1082, 1083, 1084,
1085, 1086, 1088, 1089, 1090, 1091, 1092, 1094, 1095, 1096, 1097, 1098, 1099, 1101, 1102, 1103,
1104, 1105, 1106, 1107, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1117, 1118, 1119, 1120, 1121,
1122, 1123, 1124, 1125, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1138, 1139,
1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149, 1150, 1151, 1152, 1154, 1155, 1156,
1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172,
1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188,
1189, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199, 1200, 1201, 1202, 1203, 1204,
1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1213, 1214, 1215, 1215, 1216, 1217, 1218, 1219,
1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1230, 1231, 1232, 1233, 1234,
1235, 1236, 1237, 1238, 1239, 1240, 1241, 1242, 1242, 1243, 1244, 1245, 1246, 1247, 1248, 1249,
1250, 1251, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1259, 1260, 1261, 1262, 1263,
1264, 1265, 1266, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1273, 1274, 1275, 1276, 1277,
1278, 1279, 1279, 1280, 1281, 1282, 1283, 1284, 1285, 1285, 1286, 1287, 1288, 1289, 1290, 1291
};

#define  ICV_CVT_BGR2Lab_EX( src, dst, blue_idx )                   \
{                                                                   \
    int b = (src)[blue_idx], g = (src)[1],                          \
        r = (src)[2^blue_idx];                                      \
    int x, y, z, f;                                                 \
    int l, a;                                                       \
                                                                    \
    x = b*labXb + g*labXg + r*labXr;                                \
    y = b*labYb + g*labYg + r*labYr;                                \
    z = b*labZb + g*labZg + r*labZr;                                \
                                                                    \
    f = x > labT;                                                   \
    x = descale( x, lab_shift );                                    \
                                                                    \
    if( f )                                                         \
        assert( (unsigned)x < 512 ), x = icvLabCubeRootTab[x];      \
    else                                                            \
        x = descale(x*labSmallScale + labSmallShift,lab_shift);     \
                                                                    \
    f = z > labT;                                                   \
    z = descale( z, lab_shift );                                    \
                                                                    \
    if( f )                                                         \
        assert( (unsigned)z < 512 ), z = icvLabCubeRootTab[z];      \
    else                                                            \
        z = descale(z*labSmallScale + labSmallShift,lab_shift);     \
                                                                    \
    f = y > labT;                                                   \
    y = descale( y, lab_shift );                                    \
                                                                    \
    if( f )                                                         \
    {                                                               \
        assert( (unsigned)y < 512 ), y = icvLabCubeRootTab[y];      \
        l = descale(y*labLScale - labLShift, 2*lab_shift );         \
    }                                                               \
    else                                                            \
    {                                                               \
        l = descale(y*9,lab_shift);                                 \
        y = descale(y*labSmallScale + labSmallShift,lab_shift);     \
    }                                                               \
                                                                    \
    a = descale( 500*(x - y), lab_shift ) + 128;                    \
    b = descale( 200*(y - z), lab_shift ) + 128;                    \
                                                                    \
    (dst)[0] = CV_CAST_8U(l);                                       \
    (dst)[1] = CV_CAST_8U(a);                                       \
    (dst)[2] = CV_CAST_8U(b);                                       \
}


#define  ICV_CVT_BGR2Lab( src, dst )   ICV_CVT_BGR2Lab_EX( src, dst, 0 )
#define  ICV_CVT_RGB2Lab( src, dst )   ICV_CVT_BGR2Lab_EX( src, dst, 2 )


#define  ICV_COLORCVT_FUNC( cvt_case, flavor, arrtype, src_cn, dst_cn )         \
static CvStatus CV_STDCALL                                                      \
icvCvt##_##cvt_case##_##flavor( const arrtype* src, int srcstep,                \
                                arrtype* dst, int dststep, CvSize size )        \
{                                                                               \
    for( ; size.height--; (char*&)src += srcstep, (char*&)dst += dststep )      \
    {                                                                           \
        int i;                                                                  \
        for( i = 0; i < size.width; i++ )                                       \
            ICV_CVT_##cvt_case( src + i*(src_cn), dst + i*(dst_cn) );           \
    }                                                                           \
                                                                                \
    return CV_OK;                                                               \
}


ICV_COLORCVT_FUNC( BGR2RGB, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( BGR2BGRA, 8u_C3C4R, uchar, 3, 4 )
ICV_COLORCVT_FUNC( BGRA2BGR, 8u_C4C3R, uchar, 4, 3 )
ICV_COLORCVT_FUNC( BGR2RGBA, 8u_C3C4R, uchar, 3, 4 )
ICV_COLORCVT_FUNC( BGRA2RGBA, 8u_C4C4R, uchar, 4, 4 )
ICV_COLORCVT_FUNC( RGBA2BGR, 8u_C4C3R, uchar, 4, 3 )
ICV_COLORCVT_FUNC( BGR2GRAY, 8u_C3C1R, uchar, 3, 1 )
ICV_COLORCVT_FUNC( RGB2GRAY, 8u_C3C1R, uchar, 3, 1 )
ICV_COLORCVT_FUNC( BGRA2GRAY, 8u_C4C1R, uchar, 4, 1 )
ICV_COLORCVT_FUNC( RGBA2GRAY, 8u_C4C1R, uchar, 4, 1 )
ICV_COLORCVT_FUNC( GRAY2BGR, 8u_C1C3R, uchar, 1, 3 )
ICV_COLORCVT_FUNC( GRAY2BGRA, 8u_C1C4R, uchar, 1, 4 )
ICV_COLORCVT_FUNC( GRAY2BGR565, 8u_C1C2R, uchar, 1, 2 )
ICV_COLORCVT_FUNC( BGR5652GRAY, 8u_C2C1R, uchar, 2, 1 )
ICV_COLORCVT_FUNC( BGR2BGR565, 8u_C3C2R, uchar, 3, 2 )
ICV_COLORCVT_FUNC( RGB2BGR565, 8u_C3C2R, uchar, 3, 2 )
ICV_COLORCVT_FUNC( BGRA2BGR565, 8u_C4C2R, uchar, 4, 2 )
ICV_COLORCVT_FUNC( RGBA2BGR565, 8u_C4C2R, uchar, 4, 2 )
ICV_COLORCVT_FUNC( BGR5652BGR, 8u_C2C3R, uchar, 2, 3 )
ICV_COLORCVT_FUNC( BGR5652RGB, 8u_C2C3R, uchar, 2, 3 )
ICV_COLORCVT_FUNC( BGR5652BGRA, 8u_C2C4R, uchar, 2, 4 )
ICV_COLORCVT_FUNC( BGR5652RGBA, 8u_C2C4R, uchar, 2, 4 )

ICV_COLORCVT_FUNC( BGR2XYZ, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( RGB2XYZ, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( XYZ2BGR, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( XYZ2RGB, 8u_C3R, uchar, 3, 3 )

ICV_COLORCVT_FUNC( BGR2YCrCb, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( RGB2YCrCb, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( YCrCb2BGR, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( YCrCb2RGB, 8u_C3R, uchar, 3, 3 )

ICV_COLORCVT_FUNC( BGR2HSV, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( RGB2HSV, 8u_C3R, uchar, 3, 3 )

ICV_COLORCVT_FUNC( BGR2Lab, 8u_C3R, uchar, 3, 3 )
ICV_COLORCVT_FUNC( RGB2Lab, 8u_C3R, uchar, 3, 3 )


static CvStatus
icvBayer2BGR_8u_C1C3R( const uchar* bayer, int bayerStep,
                       uchar *dst, int dstStep,
                       CvSize size, int code )
{
    int blue = code == CV_BayerBG2BGR || code == CV_BayerGB2BGR ? -1 : 1;
    int start_with_green = code == CV_BayerGB2BGR || code == CV_BayerGR2BGR;

    if( size.width < 2 || size.height < 2 )
        return CV_BADSIZE_ERR;

    dst += dstStep + 3 + 1;
    size.height -= 2;
    size.width -= 2;

    for( ; size.height--; bayer += bayerStep, dst += dstStep )
    {
        int t0, t1;
        const uchar* bayerEnd = bayer + size.width;

        if( start_with_green )
        {
            t0 = (bayer[0] + bayer[bayerStep*2] + 1) >> 1;
            t1 = (bayer[bayerStep] + bayer[bayerStep+2] + 1) >> 1;
            dst[-blue] = (uchar)t0;
            dst[0] = bayer[bayerStep+1];
            dst[blue] = (uchar)t1;
            bayer++;
            dst += 3;
        }

        if( blue > 0 )
        {
            for( ; bayer <= bayerEnd - 2; bayer += 2, dst += 6 )
            {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                      bayer[bayerStep*2+2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
                dst[-1] = (uchar)t0;
                dst[0] = (uchar)t1;
                dst[1] = bayer[bayerStep+1];

                t0 = (bayer[2] + bayer[bayerStep*2+2] + 1) >> 1;
                t1 = (bayer[bayerStep+1] + bayer[bayerStep+3] + 1) >> 1;
                dst[2] = (uchar)t0;
                dst[3] = bayer[bayerStep+2];
                dst[4] = (uchar)t1;
            }
        }
        else
        {
            for( ; bayer <= bayerEnd - 2; bayer += 2, dst += 6 )
            {
                t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                      bayer[bayerStep*2+2] + 2) >> 2;
                t1 = (bayer[1] + bayer[bayerStep] +
                      bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
                dst[1] = (uchar)t0;
                dst[0] = (uchar)t1;
                dst[-1] = bayer[bayerStep+1];

                t0 = (bayer[2] + bayer[bayerStep*2+2] + 1) >> 1;
                t1 = (bayer[bayerStep+1] + bayer[bayerStep+3] + 1) >> 1;
                dst[4] = (uchar)t0;
                dst[3] = bayer[bayerStep+2];
                dst[2] = (uchar)t1;
            }
        }

        if( bayer < bayerEnd )
        {
            t0 = (bayer[0] + bayer[2] + bayer[bayerStep*2] +
                  bayer[bayerStep*2+2] + 2) >> 2;
            t1 = (bayer[1] + bayer[bayerStep] +
                  bayer[bayerStep+2] + bayer[bayerStep*2+1]+2) >> 2;
            dst[-blue] = (uchar)t0;
            dst[0] = (uchar)t1;
            dst[blue] = bayer[bayerStep+1];
            bayer++;
            dst += 3;
        }

        bayer -= size.width;
        dst -= size.width*3;

        blue = -blue;
        start_with_green = !start_with_green;
    }

    return CV_OK;
}


#define  ICV_BAYER_FUNC( cvt_case )                                 \
static CvStatus CV_STDCALL                                          \
icvCvt_##cvt_case##_8u_C1C3R( const uchar* src, int srcstep,        \
                             uchar* dst, int dststep, CvSize size ) \
{                                                                   \
    return icvBayer2BGR_8u_C1C3R( src, srcstep, dst, dststep,       \
                                  size, CV_##cvt_case );            \
}


ICV_BAYER_FUNC( BayerBG2BGR )
ICV_BAYER_FUNC( BayerGB2BGR )
ICV_BAYER_FUNC( BayerRG2BGR )
ICV_BAYER_FUNC( BayerGR2BGR )


typedef struct CvColorCvtFuncEntry
{
    void* func;
    char  src_cn;
    char  dst_cn;
    char  pixel_wise;
}
CvColorCvtFuncEntry;


static void icvInitColorCvtTable( CvColorCvtFuncEntry* tab )
{
    #define ICV_ADD_CVT_FUNC( cvt_case, scn, dcn )                                \
        tab[CV_##cvt_case].func = (void*)icvCvt_##cvt_case##_8u_C##scn##C##dcn##R;\
        tab[CV_##cvt_case].src_cn = scn; tab[CV_##cvt_case].dst_cn = dcn;         \
        tab[CV_##cvt_case].pixel_wise = 1;

    #define ICV_ADD_CVT_FUNC_C3( cvt_case )                                     \
        tab[CV_##cvt_case].func = (void*)icvCvt_##cvt_case##_8u_C3R;            \
        tab[CV_##cvt_case].src_cn = tab[CV_##cvt_case].dst_cn = 3;              \
        tab[CV_##cvt_case].pixel_wise = 1;

    #define ICV_ADD_BAYER_FUNC( cvt_case )                                      \
        tab[CV_##cvt_case].func = (void*)icvCvt_##cvt_case##_8u_C1C3R;          \
        tab[CV_##cvt_case].src_cn = 1; tab[CV_##cvt_case].dst_cn = 3;           \
        tab[CV_##cvt_case].pixel_wise = 0;


    ICV_ADD_CVT_FUNC( BGR2BGRA, 3, 4 )
    ICV_ADD_CVT_FUNC( BGRA2BGR, 4, 3 )
    ICV_ADD_CVT_FUNC( BGR2RGBA, 3, 4 )
    ICV_ADD_CVT_FUNC( BGRA2RGBA, 4, 4 )
    ICV_ADD_CVT_FUNC( RGBA2BGR, 4, 3 )
    ICV_ADD_CVT_FUNC( BGR5652GRAY, 2, 1 )
    ICV_ADD_CVT_FUNC( BGR2GRAY, 3, 1 )
    ICV_ADD_CVT_FUNC( RGB2GRAY, 3, 1 )
    ICV_ADD_CVT_FUNC( BGRA2GRAY, 4, 1 )
    ICV_ADD_CVT_FUNC( RGBA2GRAY, 4, 1 )
    ICV_ADD_CVT_FUNC( GRAY2BGR565, 1, 2 )
    ICV_ADD_CVT_FUNC( GRAY2BGR, 1, 3 )
    ICV_ADD_CVT_FUNC( GRAY2BGRA, 1, 4 )
    ICV_ADD_CVT_FUNC( BGR2BGR565, 3, 2 )
    ICV_ADD_CVT_FUNC( RGB2BGR565, 3, 2 )
    ICV_ADD_CVT_FUNC( BGRA2BGR565, 4, 2 )
    ICV_ADD_CVT_FUNC( RGBA2BGR565, 4, 2 )
    ICV_ADD_CVT_FUNC( BGR5652BGR, 2, 3 )
    ICV_ADD_CVT_FUNC( BGR5652RGB, 2, 3 )
    ICV_ADD_CVT_FUNC( BGR5652BGRA, 2, 4 )
    ICV_ADD_CVT_FUNC( BGR5652RGBA, 2, 4 )
    

    ICV_ADD_CVT_FUNC_C3( BGR2RGB )    
    ICV_ADD_CVT_FUNC_C3( BGR2XYZ )
    ICV_ADD_CVT_FUNC_C3( RGB2XYZ )
    ICV_ADD_CVT_FUNC_C3( XYZ2BGR )
    ICV_ADD_CVT_FUNC_C3( XYZ2RGB )

    ICV_ADD_CVT_FUNC_C3( BGR2YCrCb )
    ICV_ADD_CVT_FUNC_C3( RGB2YCrCb )
    ICV_ADD_CVT_FUNC_C3( YCrCb2BGR )
    ICV_ADD_CVT_FUNC_C3( YCrCb2RGB )

    ICV_ADD_CVT_FUNC_C3( BGR2HSV )
    ICV_ADD_CVT_FUNC_C3( RGB2HSV )

    ICV_ADD_CVT_FUNC_C3( BGR2Lab )
    ICV_ADD_CVT_FUNC_C3( RGB2Lab )

    ICV_ADD_BAYER_FUNC( BayerBG2BGR )
    ICV_ADD_BAYER_FUNC( BayerGB2BGR )
    ICV_ADD_BAYER_FUNC( BayerRG2BGR )
    ICV_ADD_BAYER_FUNC( BayerGR2BGR )
}


/* dst(idx) = Conversin(src(idx)) */
CV_IMPL void
cvCvtColor( const CvArr* srcarr, CvArr* dstarr, int colorcvt_code )
{
    static CvColorCvtFuncEntry cvttab[CV_COLORCVT_MAX];
    static int inittab = 0;

    CV_FUNCNAME( "cvCvtColor" );

    __BEGIN__;
    
    CvMat srcstub, *src = (CvMat*)srcarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    CvSize size;
    CvFunc2D_2A func;
    int src_step, dst_step;
    int src_cn, dst_cn;
    
    CV_CALL( src = cvGetMat( srcarr, &srcstub ));
    CV_CALL( dst = cvGetMat( dstarr, &dststub ));
    
    if( !inittab )
    {
        icvInitColorCvtTable( cvttab );
        inittab = 1;
    }

    if( !CV_ARE_SIZES_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    if( !CV_ARE_DEPTHS_EQ( src, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( CV_MAT_DEPTH( src->type ) != CV_8U )
        CV_ERROR( CV_StsUnsupportedFormat, "" );

    if( (unsigned)colorcvt_code >= CV_COLORCVT_MAX )
        CV_ERROR( CV_StsBadFlag, "" );

    src_cn = CV_MAT_CN( src->type );
    dst_cn = CV_MAT_CN( dst->type );

    // check colorcvt_code correctness in some typical cases and fix if neccessary
    switch( colorcvt_code )
    {
    case CV_GRAY2BGR:
        colorcvt_code = dst_cn == 3 ? CV_GRAY2BGR :
                        dst_cn == 4 ? CV_GRAY2BGRA : CV_GRAY2BGR565;
        break;
    case CV_BGR2GRAY:
        colorcvt_code = src_cn == 3 ? CV_BGR2GRAY :
                        src_cn == 4 ? CV_BGRA2GRAY : CV_BGR5652GRAY;
        break;
    case CV_RGB2GRAY:
        colorcvt_code = src_cn == 3 ? CV_RGB2GRAY :
                        src_cn == 4 ? CV_RGBA2GRAY : CV_BGR5652GRAY;
        break;
    }

    func = (CvFunc2D_2A)cvttab[colorcvt_code].func;

    if( !func )
        CV_ERROR( CV_StsBadFlag, "" );

    if( src_cn != cvttab[colorcvt_code].src_cn ||
        dst_cn != cvttab[colorcvt_code].dst_cn )
        CV_ERROR( CV_BadNumChannels, "" );

    size = icvGetMatSize( src );
    src_step = src->step;
    dst_step = dst->step;

    if( cvttab[colorcvt_code].pixel_wise &&
        CV_IS_MAT_CONT( src->type & dst->type ))
    {
        size.width *= size.height;
        src_step = dst_step = CV_STUB_STEP;
        size.height = 1;
    }

    IPPI_CALL( func( src->data.ptr, src_step, dst->data.ptr, dst_step, size ));

    __END__;
}

/* End of file. */


