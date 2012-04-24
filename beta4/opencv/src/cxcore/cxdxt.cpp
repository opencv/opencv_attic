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

#include "_cxcore.h"

icvDFTInitAlloc_C_32fc_t icvDFTInitAlloc_C_32fc_p = 0;
icvDFTFree_C_32fc_t icvDFTFree_C_32fc_p = 0;
icvDFTGetBufSize_C_32fc_t icvDFTGetBufSize_C_32fc_p = 0;
icvDFTFwd_CToC_32fc_t icvDFTFwd_CToC_32fc_p = 0;
icvDFTInv_CToC_32fc_t icvDFTInv_CToC_32fc_p = 0;

icvDFTInitAlloc_C_64fc_t icvDFTInitAlloc_C_64fc_p = 0;
icvDFTFree_C_64fc_t icvDFTFree_C_64fc_p = 0;
icvDFTGetBufSize_C_64fc_t icvDFTGetBufSize_C_64fc_p = 0;
icvDFTFwd_CToC_64fc_t icvDFTFwd_CToC_64fc_p = 0;
icvDFTInv_CToC_64fc_t icvDFTInv_CToC_64fc_p = 0;

icvDFTInitAlloc_R_32f_t icvDFTInitAlloc_R_32f_p = 0;
icvDFTFree_R_32f_t icvDFTFree_R_32f_p = 0;
icvDFTGetBufSize_R_32f_t icvDFTGetBufSize_R_32f_p = 0;
icvDFTFwd_RToPack_32f_t icvDFTFwd_RToPack_32f_p = 0;
icvDFTInv_PackToR_32f_t icvDFTInv_PackToR_32f_p = 0;

icvDFTInitAlloc_R_64f_t icvDFTInitAlloc_R_64f_p = 0;
icvDFTFree_R_64f_t icvDFTFree_R_64f_p = 0;
icvDFTGetBufSize_R_64f_t icvDFTGetBufSize_R_64f_p = 0;
icvDFTFwd_RToPack_64f_t icvDFTFwd_RToPack_64f_p = 0;
icvDFTInv_PackToR_64f_t icvDFTInv_PackToR_64f_p = 0;

/*icvDCTFwdInitAlloc_32f_t icvDCTFwdInitAlloc_32f_p = 0;
icvDCTFwdFree_32f_t icvDCTFwdFree_32f_p = 0;
icvDCTFwdGetBufSize_32f_t icvDCTFwdGetBufSize_32f_p = 0;
icvDCTFwd_32f_t icvDCTFwd_32f_p = 0;

icvDCTInvInitAlloc_32f_t icvDCTInvInitAlloc_32f_p = 0;
icvDCTInvFree_32f_t icvDCTInvFree_32f_p = 0;
icvDCTInvGetBufSize_32f_t icvDCTInvGetBufSize_32f_p = 0;
icvDCTInv_32f_t icvDCTInv_32f_p = 0;

icvDCTFwdInitAlloc_64f_t icvDCTFwdInitAlloc_64f_p = 0;
icvDCTFwdFree_64f_t icvDCTFwdFree_64f_p = 0;
icvDCTFwdGetBufSize_64f_t icvDCTFwdGetBufSize_64f_p = 0;
icvDCTFwd_64f_t icvDCTFwd_64f_p = 0;

icvDCTInvInitAlloc_64f_t icvDCTInvInitAlloc_64f_p = 0;
icvDCTInvFree_64f_t icvDCTInvFree_64f_p = 0;
icvDCTInvGetBufSize_64f_t icvDCTInvGetBufSize_64f_p = 0;
icvDCTInv_64f_t icvDCTInv_64f_p = 0;*/

/****************************************************************************************\
                               Discrete Fourier Transform
\****************************************************************************************/

#define CV_MAX_LOCAL_DFT_SIZE  (1 << 15)

static const uchar log2tab[] = { 0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0 };
static int icvlog2( int n )
{
    int m = 0;
    int f = (n >= (1 << 16))*16;
    n >>= f;
    m += f;
    f = (n >= (1 << 8))*8;
    n >>= f;
    m += f;
    f = (n >= (1 << 4))*4;
    n >>= f;
    return m + f + log2tab[n];
}

static unsigned char icvRevTable[] =
{
  0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
  0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
  0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
  0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
  0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
  0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
  0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
  0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
  0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
  0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
  0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
  0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
  0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
  0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
  0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
  0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};

static const double icvDxtTab[][2] =
{
{ 1.00000000000000000, 0.00000000000000000 },
{-1.00000000000000000, 0.00000000000000000 },
{ 0.00000000000000000, 1.00000000000000000 },
{ 0.70710678118654757, 0.70710678118654746 },
{ 0.92387953251128674, 0.38268343236508978 },
{ 0.98078528040323043, 0.19509032201612825 },
{ 0.99518472667219693, 0.09801714032956060 },
{ 0.99879545620517241, 0.04906767432741802 },
{ 0.99969881869620425, 0.02454122852291229 },
{ 0.99992470183914450, 0.01227153828571993 },
{ 0.99998117528260111, 0.00613588464915448 },
{ 0.99999529380957619, 0.00306795676296598 },
{ 0.99999882345170188, 0.00153398018628477 },
{ 0.99999970586288223, 0.00076699031874270 },
{ 0.99999992646571789, 0.00038349518757140 },
{ 0.99999998161642933, 0.00019174759731070 },
{ 0.99999999540410733, 0.00009587379909598 },
{ 0.99999999885102686, 0.00004793689960307 },
{ 0.99999999971275666, 0.00002396844980842 },
{ 0.99999999992818922, 0.00001198422490507 },
{ 0.99999999998204725, 0.00000599211245264 },
{ 0.99999999999551181, 0.00000299605622633 },
{ 0.99999999999887801, 0.00000149802811317 },
{ 0.99999999999971945, 0.00000074901405658 },
{ 0.99999999999992983, 0.00000037450702829 },
{ 0.99999999999998246, 0.00000018725351415 },
{ 0.99999999999999567, 0.00000009362675707 },
{ 0.99999999999999889, 0.00000004681337854 },
{ 0.99999999999999978, 0.00000002340668927 },
{ 0.99999999999999989, 0.00000001170334463 },
{ 1.00000000000000000, 0.00000000585167232 },
{ 1.00000000000000000, 0.00000000292583616 }
};

#define icvBitRev(i,shift) \
   ((int)((((unsigned)icvRevTable[(i)&255] << 24)+ \
           ((unsigned)icvRevTable[((i)>> 8)&255] << 16)+ \
           ((unsigned)icvRevTable[((i)>>16)&255] <<  8)+ \
           ((unsigned)icvRevTable[((i)>>24)])) >> (shift)))

static int
icvDFTFactorize( int n, int* factors )
{
    int nf = 0, f, i, j;

    if( n <= 5 )
    {
        factors[0] = n;
        return 1;
    }
    
    f = (((n - 1)^n)+1) >> 1;
    if( f > 1 )
    {
        factors[nf++] = f;
        n = f == n ? 1 : n/f;
    }

    for( f = 3; n > 1; )
    {
        int d = n/f;
        if( d*f == n )
        {
            factors[nf++] = f;
            n = d;
        }
        else
        {
            f += 2;
            if( f*f > n )
                break;
        }
    }

    if( n > 1 )
        factors[nf++] = n;

    f = (factors[0] & 1) == 0;
    for( i = f; i < (nf+f)/2; i++ )
        CV_SWAP( factors[i], factors[nf-i-1+f], j );

    return nf;
}

static void
icvDFTInit( int n0, int nf, int* factors, int* itab, int elem_size, void* _wave, int inv_itab )
{
    int digits[34], radix[34];
    int n = factors[0], m = 0;
    int* itab0 = itab;
    int i, j, k;
    CvComplex64f w, w1;
    double t;

    if( n0 <= 5 )
    {
        itab[0] = 0;
        itab[n0-1] = n0-1;
        
        if( n0 != 4 )
        {
            for( i = 1; i < n0-1; i++ )
                itab[i] = i;
        }
        else
        {
            itab[1] = 2;
            itab[2] = 1;
        }
        if( n0 == 5 )
        {
            if( elem_size == sizeof(CvComplex64f) )
                ((CvComplex64f*)_wave)[0] = CvComplex64f(1.,0.);
            else
                ((CvComplex32f*)_wave)[0] = CvComplex32f(1.f,0.f);
        }
        if( n0 != 4 )
            return;
        m = 2;
    }
    else
    {
        radix[nf] = 1;
        digits[nf] = 0;
        for( i = 0; i < nf; i++ )
        {
            digits[i] = 0;
            radix[nf-i-1] = radix[nf-i]*factors[nf-i-1];
        }

        if( inv_itab && factors[0] != factors[nf-1] )
            itab = (int*)_wave;

        if( (n & 1) == 0 )
        {
            int a = radix[1], na2 = n*a>>1, na4 = na2 >> 1;
            m = icvlog2(n);
        
            if( n <= 2  )
            {
                itab[0] = 0;
                itab[1] = na2;
            }
            else if( n <= 256 )
            {
                int shift = 10 - m;
                for( i = 0; i <= n - 4; i += 4 )
                {
                    int j = (icvRevTable[i>>2]>>shift)*a;
                    itab[i] = j;
                    itab[i+1] = j + na2;
                    itab[i+2] = j + na4;
                    itab[i+3] = j + na2 + na4;
                }
            }
            else
            {
                int shift = 34 - m;
                for( i = 0; i < n; i += 4 )
                {
                    int i4 = i >> 2;
                    int j = icvBitRev(i4,shift)*a;
                    itab[i] = j;
                    itab[i+1] = j + na2;
                    itab[i+2] = j + na4;
                    itab[i+3] = j + na2 + na4;
                }
            }

            digits[1]++;
            for( i = n, j = radix[2];; )
            {
                for( k = 0; k < n; k++ )
                    itab[i+k] = itab[k] + j;
                if( (i += n) >= n0 )
                    break;
                j += radix[2];
                for( k = 1; ++digits[k] >= factors[k]; k++ )
                {
                    digits[k] = 0;
                    j += radix[k+2] - radix[k];
                }
            }
        }
        else
        {
            for( i = 0, j = 0;; )
            {
                itab[i] = j;
                if( ++i >= n0 )
                    break;
                j += radix[1];
                for( k = 0; ++digits[k] >= factors[k]; k++ )
                {
                    digits[k] = 0;
                    j += radix[k+2] - radix[k];
                }
            }
        }

        if( itab != itab0 )
        {
            itab0[0] = 0;
            for( i = n0 & 1; i < n0; i += 2 )
            {
                int k0 = itab[i];
                int k1 = itab[i+1];
                itab0[k0] = i;
                itab0[k1] = i+1;
            }
        }
    }

    if( (n0 & (n0-1)) == 0 )
    {
        w.re = w1.re = icvDxtTab[m][0];
        w.im = w1.im = -icvDxtTab[m][1];
    }
    else
    {
        t = -CV_PI*2/n0;
        w.im = w1.im = sin(t);
        w.re = w1.re = sqrt(1. - w1.im*w1.im);
    }
    n = (n0+1)/2;

    if( elem_size == sizeof(CvComplex64f) )
    {
        CvComplex64f* wave = (CvComplex64f*)_wave;

        wave[0].re = 1.;
        wave[0].im = 0.;

        if( (n0 & 1) == 0 )
        {
            wave[n].re = -1.;
            wave[n].im = 0;
        }

        for( i = 1; i < n; i++ )
        {
            wave[i] = w;
            wave[n0-i].re = w.re;
            wave[n0-i].im = -w.im;

            t = w.re*w1.re - w.im*w1.im;
            w.im = w.re*w1.im + w.im*w1.re;
            w.re = t;
        }
    }
    else
    {
        CvComplex32f* wave = (CvComplex32f*)_wave;
        assert( elem_size == sizeof(CvComplex32f) );
        
        wave[0].re = 1.f;
        wave[0].im = 0.f;

        if( (n0 & 1) == 0 )
        {
            wave[n].re = -1.f;
            wave[n].im = 0.f;
        }

        for( i = 1; i < n; i++ )
        {
            wave[i].re = (float)w.re;
            wave[i].im = (float)w.im;
            wave[n0-i].re = (float)w.re;
            wave[n0-i].im = (float)-w.im;

            t = w.re*w1.re - w.im*w1.im;
            w.im = w.re*w1.im + w.im*w1.re;
            w.re = t;
        }
    }
}


static const double icv_sin_120 = 0.86602540378443864676372317075294;
static const double icv_sin_45 = 0.70710678118654752440084436210485;
static const double icv_fft5_2 = 0.559016994374947424102293417182819;
static const double icv_fft5_3 = -0.951056516295153572116439333379382;
static const double icv_fft5_4 = -1.538841768587626701285145288018455;
static const double icv_fft5_5 = 0.363271264002680442947733378740309;

#define ICV_DFT_NO_PERMUTE 2
#define ICV_DFT_COMPLEX_INPUT_OR_OUTPUT 4

// mixed-radix complex discrete Fourier transform: double-precision version
static CvStatus CV_STDCALL
icvDFT_64fc( const CvComplex64f* src, CvComplex64f* dst, int n,
             int nf, int* factors, const int* itab,
             const CvComplex64f* wave, int tab_size,
             const void* spec, CvComplex64f* buf,
             int flags, double scale )
{
    int n0 = n, f_idx, nx;
    int inv = flags & CV_DXT_INVERSE;
    int dw0 = tab_size, dw;
    int i, j, k;
    CvComplex64f t;
    int tab_step;

    if( spec )
    {
        assert( icvDFTFwd_CToC_64fc_p != 0 && icvDFTInv_CToC_64fc_p != 0 );
        return !inv ?
            icvDFTFwd_CToC_64fc_p( src, dst, spec, buf ):
            icvDFTInv_CToC_64fc_p( src, dst, spec, buf );
    }

    tab_step = tab_size == n ? 1 : tab_size == n*2 ? 2 : tab_size/n;

    // 0. shuffle data
    if( dst != src )
    {
        assert( (flags & ICV_DFT_NO_PERMUTE) == 0 );
        if( !inv )
        {
            for( i = 0; i <= n - 2; i += 2, itab += 2*tab_step )
            {
                int k0 = itab[0], k1 = itab[tab_step];
                assert( (unsigned)k0 < (unsigned)n && (unsigned)k1 < (unsigned)n );
                dst[i] = src[k0]; dst[i+1] = src[k1];
            }

            if( i < n )
                dst[n-1] = src[n-1];
        }
        else
        {
            for( i = 0; i <= n - 2; i += 2, itab += 2*tab_step )
            {
                int k0 = itab[0], k1 = itab[tab_step];
                assert( (unsigned)k0 < (unsigned)n && (unsigned)k1 < (unsigned)n );
                t.re = src[k0].re; t.im = -src[k0].im;
                dst[i] = t;
                t.re = src[k1].re; t.im = -src[k1].im;
                dst[i+1] = t;
            }

            if( i < n )
            {
                t.re = src[n-1].re; t.im = -src[n-1].im;
                dst[i] = t;
            }
        }
    }
    else
    {
        if( (flags & ICV_DFT_NO_PERMUTE) == 0 )
        {
            if( factors[0] != factors[nf-1] )
                return CV_INPLACE_NOT_SUPPORTED_ERR;
            if( nf == 1 )
            {
                if( (n & 3) == 0 )
                {
                    int n2 = n/2;
                    CvComplex64f* dsth = dst + n2;
                
                    for( i = 0; i < n2; i += 2, itab += tab_step*2 )
                    {
                        j = itab[0];
                        assert( (unsigned)j < (unsigned)n2 );

                        CV_SWAP(dst[i+1], dsth[j], t);
                        if( j > i )
                        {
                            CV_SWAP(dst[i], dst[j], t);
                            CV_SWAP(dsth[i+1], dsth[j+1], t);
                        }
                    }
                }
                // else do nothing
            }
            else
            {
                for( i = 0; i < n; i++, itab += tab_step )
                {
                    j = itab[0];
                    assert( (unsigned)j < (unsigned)n );
                    if( j > i )
                        CV_SWAP(dst[i], dst[j], t);
                }
            }
        }

        if( inv )
        {
            for( i = 0; i <= n - 2; i += 2 )
            {
                double t0 = -dst[i].im;
                double t1 = -dst[i+1].im;
                dst[i].im = t0; dst[i+1].im = t1;
            }

            if( i < n )
                dst[n-1].im = -dst[n-1].im;
        }
    }

    n = 1;
    // 1. power-2 transforms
    if( (factors[0] & 1) == 0 )
    {
        // radix-4 transform
        for( ; n*4 <= factors[0]; )
        {
            nx = n;
            n *= 4;
            dw0 /= 4;

            for( i = 0; i < n0; i += n )
            {
                CvComplex64f* v0;
                CvComplex64f* v1;
                double r0, i0, r1, i1, r2, i2, r3, i3, r4, i4;

                v0 = dst + i;
                v1 = v0 + nx*2;

                r2 = v0[0].re; i2 = v0[0].im;
                r1 = v0[nx].re; i1 = v0[nx].im;
                
                r0 = r1 + r2; i0 = i1 + i2;
                r2 -= r1; i2 -= i1;

                i3 = v1[nx].re; r3 = v1[nx].im;
                i4 = v1[0].re; r4 = v1[0].im;

                r1 = i4 + i3; i1 = r4 + r3;
                r3 = r4 - r3; i3 = i3 - i4;

                v0[0].re = r0 + r1; v0[0].im = i0 + i1;
                v1[0].re = r0 - r1; v1[0].im = i0 - i1;
                v0[nx].re = r2 + r3; v0[nx].im = i2 + i3;
                v1[nx].re = r2 - r3; v1[nx].im = i2 - i3;

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    v0 = dst + i + j;
                    v1 = v0 + nx*2;

                    r2 = v0[nx].re*wave[dw*2].re - v0[nx].im*wave[dw*2].im;
                    i2 = v0[nx].re*wave[dw*2].im + v0[nx].im*wave[dw*2].re;
                    r0 = v1[0].re*wave[dw].im + v1[0].im*wave[dw].re;
                    i0 = v1[0].re*wave[dw].re - v1[0].im*wave[dw].im;
                    r3 = v1[nx].re*wave[dw*3].im + v1[nx].im*wave[dw*3].re;
                    i3 = v1[nx].re*wave[dw*3].re - v1[nx].im*wave[dw*3].im;

                    r1 = i0 + i3; i1 = r0 + r3;
                    r3 = r0 - r3; i3 = i3 - i0;
                    r4 = v0[0].re; i4 = v0[0].im;

                    r0 = r4 + r2; i0 = i4 + i2;
                    r2 = r4 - r2; i2 = i4 - i2;

                    v0[0].re = r0 + r1; v0[0].im = i0 + i1;
                    v1[0].re = r0 - r1; v1[0].im = i0 - i1;
                    v0[nx].re = r2 + r3; v0[nx].im = i2 + i3;
                    v1[nx].re = r2 - r3; v1[nx].im = i2 - i3;
                }
            }
        }

        for( ; n < factors[0]; )
        {
            // do the remaining radix-2 transform
            nx = n;
            n *= 2;
            dw0 /= 2;

            for( i = 0; i < n0; i += n )
            {
                CvComplex64f* v = dst + i;
                double r0 = v[0].re + v[nx].re;
                double i0 = v[0].im + v[nx].im;
                double r1 = v[0].re - v[nx].re;
                double i1 = v[0].im - v[nx].im;
                v[0].re = r0; v[0].im = i0;
                v[nx].re = r1; v[nx].im = i1;

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    v = dst + i + j;
                    r1 = v[nx].re*wave[dw].re - v[nx].im*wave[dw].im;
                    i1 = v[nx].im*wave[dw].re + v[nx].re*wave[dw].im;
                    r0 = v[0].re; i0 = v[0].im;

                    v[0].re = r0 + r1; v[0].im = i0 + i1;
                    v[nx].re = r0 - r1; v[nx].im = i0 - i1;
                }
            }
        }
    }

    // 2. all the other transforms
    for( f_idx = (factors[0]&1) ? 0 : 1; f_idx < nf; f_idx++ )
    {
        int factor = factors[f_idx];
        nx = n;
        n *= factor;
        dw0 /= factor;

        if( factor == 3 )
        {
            // radix-3
            for( i = 0; i < n0; i += n )
            {
                CvComplex64f* v = dst + i;

                double r1 = v[nx].re + v[nx*2].re;
                double i1 = v[nx].im + v[nx*2].im;
                double r0 = v[0].re;
                double i0 = v[0].im;
                double r2 = icv_sin_120*(v[nx].im - v[nx*2].im);
                double i2 = icv_sin_120*(v[nx*2].re - v[nx].re);
                v[0].re = r0 + r1; v[0].im = i0 + i1;
                r0 -= 0.5*r1; i0 -= 0.5*i1;
                v[nx].re = r0 + r2; v[nx].im = i0 + i2;
                v[nx*2].re = r0 - r2; v[nx*2].im = i0 - i2;

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    CvComplex64f* v = dst + i + j;

                    double r0 = v[nx].re*wave[dw].re - v[nx].im*wave[dw].im;
                    double i0 = v[nx].re*wave[dw].im + v[nx].im*wave[dw].re;
                    double i2 = v[nx*2].re*wave[dw*2].re - v[nx*2].im*wave[dw*2].im;
                    double r2 = v[nx*2].re*wave[dw*2].im + v[nx*2].im*wave[dw*2].re;
                    double r1 = r0 + i2, i1 = i0 + r2;
                    
                    r2 = icv_sin_120*(i0 - r2); i2 = icv_sin_120*(i2 - r0);
                    r0 = v[0].re; i0 = v[0].im;
                    v[0].re = r0 + r1; v[0].im = i0 + i1;
                    r0 -= 0.5*r1; i0 -= 0.5*i1;
                    v[nx].re = r0 + r2; v[nx].im = i0 + i2;
                    v[nx*2].re = r0 - r2; v[nx*2].im = i0 - i2;
                }
            }
        }
        else if( factor == 5 )
        {
            // radix-5
            for( i = 0; i < n0; i += n )
            {
                for( j = 0, dw = 0; j < nx; j++, dw += dw0 )
                {
                    CvComplex64f* v0 = dst + i + j;
                    CvComplex64f* v1 = v0 + nx*2;
                    CvComplex64f* v2 = v1 + nx*2;

                    double r0, i0, r1, i1, r2, i2, r3, i3, r4, i4, r5, i5;

                    r3 = v0[nx].re*wave[dw].re - v0[nx].im*wave[dw].im;
                    i3 = v0[nx].re*wave[dw].im + v0[nx].im*wave[dw].re;
                    r2 = v2[0].re*wave[dw*4].re - v2[0].im*wave[dw*4].im;
                    i2 = v2[0].re*wave[dw*4].im + v2[0].im*wave[dw*4].re;

                    r1 = r3 + r2; i1 = i3 + i2;
                    r3 -= r2; i3 -= i2;

                    r4 = v1[nx].re*wave[dw*3].re - v1[nx].im*wave[dw*3].im;
                    i4 = v1[nx].re*wave[dw*3].im + v1[nx].im*wave[dw*3].re;
                    r0 = v1[0].re*wave[dw*2].re - v1[0].im*wave[dw*2].im;
                    i0 = v1[0].re*wave[dw*2].im + v1[0].im*wave[dw*2].re;

                    r2 = r4 + r0; i2 = i4 + i0;
                    r4 -= r0; i4 -= i0;

                    r0 = v0[0].re; i0 = v0[0].im;
                    r5 = r1 + r2; i5 = i1 + i2;

                    v0[0].re = r0 + r5; v0[0].im = i0 + i5;

                    r0 -= 0.25*r5; i0 -= 0.25*i5;
                    r1 = icv_fft5_2*(r1 - r2); i1 = icv_fft5_2*(i1 - i2);
                    r2 = -icv_fft5_3*(i3 + i4); i2 = icv_fft5_3*(r3 + r4);

                    i3 *= -icv_fft5_5; r3 *= icv_fft5_5;
                    i4 *= -icv_fft5_4; r4 *= icv_fft5_4;

                    r5 = r2 + i3; i5 = i2 + r3;
                    r2 -= i4; i2 -= r4;
                    
                    r3 = r0 + r1; i3 = i0 + i1;
                    r0 -= r1; i0 -= i1;

                    v0[nx].re = r3 + r2; v0[nx].im = i3 + i2;
                    v2[0].re = r3 - r2; v2[0].im = i3 - i2;

                    v1[0].re = r0 + r5; v1[0].im = i0 + i5;
                    v1[nx].re = r0 - r5; v1[nx].im = i0 - i5;
                }
            }
        }
        else
        {
            // radix-"factor" - an odd number
            int p, q, factor2 = (factor - 1)/2;
            int d, dd, dw_f = tab_size/factor;
            CvComplex64f* a = buf;
            CvComplex64f* b = buf + factor2;

            for( i = 0; i < n0; i += n )
            {
                for( j = 0, dw = 0; j < nx; j++, dw += dw0 )
                {
                    CvComplex64f* v = dst + i + j;
                    CvComplex64f v_0 = v[0];
                    CvComplex64f vn_0 = v_0;

                    if( j == 0 )
                    {
                        for( p = 1, k = nx; p <= factor2; p++, k += nx )
                        {
                            double r0 = v[k].re + v[n-k].re;
                            double i0 = v[k].im - v[n-k].im;
                            double r1 = v[k].re - v[n-k].re;
                            double i1 = v[k].im + v[n-k].im;

                            vn_0.re += r0; vn_0.im += i1;
                            a[p-1].re = r0; a[p-1].im = i0;
                            b[p-1].re = r1; b[p-1].im = i1;
                        }
                    }
                    else
                    {
                        const CvComplex64f* wave_ = wave + dw*factor;
                        d = dw;

                        for( p = 1, k = nx; p <= factor2; p++, k += nx, d += dw )
                        {
                            double r2 = v[k].re*wave[d].re - v[k].im*wave[d].im;
                            double i2 = v[k].re*wave[d].im + v[k].im*wave[d].re;

                            double r1 = v[n-k].re*wave_[-d].re - v[n-k].im*wave_[-d].im;
                            double i1 = v[n-k].re*wave_[-d].im + v[n-k].im*wave_[-d].re;
                    
                            double r0 = r2 + r1;
                            double i0 = i2 - i1;
                            r1 = r2 - r1;
                            i1 = i2 + i1;

                            vn_0.re += r0; vn_0.im += i1;
                            a[p-1].re = r0; a[p-1].im = i0;
                            b[p-1].re = r1; b[p-1].im = i1;
                        }
                    }

                    v[0] = vn_0;

                    for( p = 1, k = nx; p <= factor2; p++, k += nx )
                    {
                        CvComplex64f s0 = v_0, s1 = v_0;
                        d = dd = dw_f*p;

                        for( q = 0; q < factor2; q++ )
                        {
                            double r0 = wave[d].re * a[q].re;
                            double i0 = wave[d].im * a[q].im;
                            double r1 = wave[d].re * b[q].im;
                            double i1 = wave[d].im * b[q].re;
            
                            s1.re += r0 + i0; s0.re += r0 - i0;
                            s1.im += r1 - i1; s0.im += r1 + i1;

                            d += dd;
                            d -= -(d >= tab_size) & tab_size;
                        }

                        v[k] = s0;
                        v[n-k] = s1;
                    }
                }
            }
        }
    }

    if( scale != 1. )
    {
        double re_scale = scale, im_scale = scale;
        if( inv )
            im_scale = -im_scale;

        for( i = 0; i < n0; i++ )
        {
            double t0 = dst[i].re*re_scale;
            double t1 = dst[i].im*im_scale;
            dst[i].re = t0;
            dst[i].im = t1;
        }
    }
    else if( inv )
    {
        for( i = 0; i <= n0 - 2; i += 2 )
        {
            double t0 = -dst[i].im;
            double t1 = -dst[i+1].im;
            dst[i].im = t0;
            dst[i+1].im = t1;
        }

        if( i < n0 )
            dst[n0-1].im = -dst[n0-1].im;
    }

    return CV_OK;
}


// mixed-radix complex discrete Fourier transform: single-precision version
static CvStatus CV_STDCALL
icvDFT_32fc( const CvComplex32f* src, CvComplex32f* dst, int n,
             int nf, int* factors, const int* itab,
             const CvComplex32f* wave, int tab_size,
             const void* spec, CvComplex32f* buf,
             int flags, double scale )
{
    int n0 = n, f_idx, nx;
    int inv = flags & CV_DXT_INVERSE;
    int dw0 = tab_size, dw;
    int i, j, k;
    CvComplex32f t;
    int tab_step = tab_size == n ? 1 : tab_size == n*2 ? 2 : tab_size/n;

    if( spec )
    {
        assert( icvDFTFwd_CToC_32fc_p != 0 && icvDFTInv_CToC_32fc_p != 0 );
        return !inv ?
            icvDFTFwd_CToC_32fc_p( src, dst, spec, buf ):
            icvDFTInv_CToC_32fc_p( src, dst, spec, buf );
    }

    // 0. shuffle data
    if( dst != src )
    {
        assert( (flags & ICV_DFT_NO_PERMUTE) == 0 );
        if( !inv )
        {
            for( i = 0; i <= n - 2; i += 2, itab += 2*tab_step )
            {
                int k0 = itab[0], k1 = itab[tab_step];
                assert( (unsigned)k0 < (unsigned)n && (unsigned)k1 < (unsigned)n );
                dst[i] = src[k0]; dst[i+1] = src[k1];
            }

            if( i < n )
                dst[n-1] = src[n-1];
        }
        else
        {
            for( i = 0; i <= n - 2; i += 2, itab += 2*tab_step )
            {
                int k0 = itab[0], k1 = itab[tab_step];
                assert( (unsigned)k0 < (unsigned)n && (unsigned)k1 < (unsigned)n );
                t.re = src[k0].re; t.im = -src[k0].im;
                dst[i] = t;
                t.re = src[k1].re; t.im = -src[k1].im;
                dst[i+1] = t;
            }

            if( i < n )
            {
                t.re = src[n-1].re; t.im = -src[n-1].im;
                dst[i] = t;
            }
        }
    }
    else
    {
        if( (flags & ICV_DFT_NO_PERMUTE) == 0 )
        {
            if( factors[0] != factors[nf-1] )
                return CV_INPLACE_NOT_SUPPORTED_ERR;
            if( nf == 1 )
            {
                if( (n & 3) == 0 )
                {
                    int n2 = n/2;
                    CvComplex32f* dsth = dst + n2;
                
                    for( i = 0; i < n2; i += 2, itab += tab_step*2 )
                    {
                        j = itab[0];
                        assert( (unsigned)j < (unsigned)n2 );

                        CV_SWAP(dst[i+1], dsth[j], t);
                        if( j > i )
                        {
                            CV_SWAP(dst[i], dst[j], t);
                            CV_SWAP(dsth[i+1], dsth[j+1], t);
                        }
                    }
                }
                // else do nothing
            }
            else
            {
                for( i = 0; i < n; i++, itab += tab_step )
                {
                    j = itab[0];
                    assert( (unsigned)j < (unsigned)n );
                    if( j > i )
                        CV_SWAP(dst[i], dst[j], t);
                }
            }
        }

        if( inv )
        {
            for( i = 0; i <= n - 2; i += 2 )
            {
                int t0 = *((int*)&dst[i].im) ^ 0x80000000;
                int t1 = *((int*)&dst[i+1].im) ^ 0x80000000;
                *((int*)&dst[i].im) = t0; *((int*)&dst[i+1].im) = t1;
            }

            if( i < n )
                *((int*)&dst[i].im) ^= 0x80000000;
        }
    }

    n = 1;
    // 1. power-2 transforms
    if( (factors[0] & 1) == 0 )
    {
        // radix-4 transform
        for( ; n*4 <= factors[0]; )
        {
            nx = n;
            n *= 4;
            dw0 /= 4;

            for( i = 0; i < n0; i += n )
            {
                CvComplex32f* v0;
                CvComplex32f* v1;
                double r0, i0, r1, i1, r2, i2, r3, i3, r4, i4;

                v0 = dst + i;
                v1 = v0 + nx*2;

                r2 = v0[0].re; i2 = v0[0].im;
                r1 = v0[nx].re; i1 = v0[nx].im;
                
                r0 = r1 + r2; i0 = i1 + i2;
                r2 -= r1; i2 -= i1;

                i3 = v1[nx].re; r3 = v1[nx].im;
                i4 = v1[0].re; r4 = v1[0].im;

                r1 = i4 + i3; i1 = r4 + r3;
                r3 = r4 - r3; i3 = i3 - i4;

                v0[0].re = (float)(r0 + r1); v0[0].im = (float)(i0 + i1);
                v1[0].re = (float)(r0 - r1); v1[0].im = (float)(i0 - i1);
                v0[nx].re = (float)(r2 + r3); v0[nx].im = (float)(i2 + i3);
                v1[nx].re = (float)(r2 - r3); v1[nx].im = (float)(i2 - i3);

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    v0 = dst + i + j;
                    v1 = v0 + nx*2;

                    r2 = v0[nx].re*wave[dw*2].re - v0[nx].im*wave[dw*2].im;
                    i2 = v0[nx].re*wave[dw*2].im + v0[nx].im*wave[dw*2].re;
                    r0 = v1[0].re*wave[dw].im + v1[0].im*wave[dw].re;
                    i0 = v1[0].re*wave[dw].re - v1[0].im*wave[dw].im;
                    r3 = v1[nx].re*wave[dw*3].im + v1[nx].im*wave[dw*3].re;
                    i3 = v1[nx].re*wave[dw*3].re - v1[nx].im*wave[dw*3].im;

                    r1 = i0 + i3; i1 = r0 + r3;
                    r3 = r0 - r3; i3 = i3 - i0;
                    r4 = v0[0].re; i4 = v0[0].im;

                    r0 = r4 + r2; i0 = i4 + i2;
                    r2 = r4 - r2; i2 = i4 - i2;

                    v0[0].re = (float)(r0 + r1); v0[0].im = (float)(i0 + i1);
                    v1[0].re = (float)(r0 - r1); v1[0].im = (float)(i0 - i1);
                    v0[nx].re = (float)(r2 + r3); v0[nx].im = (float)(i2 + i3);
                    v1[nx].re = (float)(r2 - r3); v1[nx].im = (float)(i2 - i3);
                }
            }
        }

        for( ; n < factors[0]; )
        {
            // do the remaining radix-2 transform
            nx = n;
            n *= 2;
            dw0 /= 2;

            for( i = 0; i < n0; i += n )
            {
                CvComplex32f* v = dst + i;
                double r0 = v[0].re + v[nx].re;
                double i0 = v[0].im + v[nx].im;
                double r1 = v[0].re - v[nx].re;
                double i1 = v[0].im - v[nx].im;
                v[0].re = (float)r0; v[0].im = (float)i0;
                v[nx].re = (float)r1; v[nx].im = (float)i1;

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    v = dst + i + j;
                    r1 = v[nx].re*wave[dw].re - v[nx].im*wave[dw].im;
                    i1 = v[nx].im*wave[dw].re + v[nx].re*wave[dw].im;
                    r0 = v[0].re; i0 = v[0].im;

                    v[0].re = (float)(r0 + r1); v[0].im = (float)(i0 + i1);
                    v[nx].re = (float)(r0 - r1); v[nx].im = (float)(i0 - i1);
                }
            }
        }
    }

    // 2. all the other transforms
    for( f_idx = (factors[0]&1) ? 0 : 1; f_idx < nf; f_idx++ )
    {
        int factor = factors[f_idx];
        nx = n;
        n *= factor;
        dw0 /= factor;

        if( factor == 3 )
        {
            // radix-3
            for( i = 0; i < n0; i += n )
            {
                CvComplex32f* v = dst + i;

                double r1 = v[nx].re + v[nx*2].re;
                double i1 = v[nx].im + v[nx*2].im;
                double r0 = v[0].re;
                double i0 = v[0].im;
                double r2 = icv_sin_120*(v[nx].im - v[nx*2].im);
                double i2 = icv_sin_120*(v[nx*2].re - v[nx].re);
                v[0].re = (float)(r0 + r1); v[0].im = (float)(i0 + i1);
                r0 -= 0.5*r1; i0 -= 0.5*i1;
                v[nx].re = (float)(r0 + r2); v[nx].im = (float)(i0 + i2);
                v[nx*2].re = (float)(r0 - r2); v[nx*2].im = (float)(i0 - i2);

                for( j = 1, dw = dw0; j < nx; j++, dw += dw0 )
                {
                    CvComplex32f* v = dst + i + j;

                    double r0 = v[nx].re*wave[dw].re - v[nx].im*wave[dw].im;
                    double i0 = v[nx].re*wave[dw].im + v[nx].im*wave[dw].re;
                    double i2 = v[nx*2].re*wave[dw*2].re - v[nx*2].im*wave[dw*2].im;
                    double r2 = v[nx*2].re*wave[dw*2].im + v[nx*2].im*wave[dw*2].re;
                    double r1 = r0 + i2, i1 = i0 + r2;
                    
                    r2 = icv_sin_120*(i0 - r2); i2 = icv_sin_120*(i2 - r0);
                    r0 = v[0].re; i0 = v[0].im;
                    v[0].re = (float)(r0 + r1); v[0].im = (float)(i0 + i1);
                    r0 -= 0.5*r1; i0 -= 0.5*i1;
                    v[nx].re = (float)(r0 + r2); v[nx].im = (float)(i0 + i2);
                    v[nx*2].re = (float)(r0 - r2); v[nx*2].im = (float)(i0 - i2);
                }
            }
        }
        else if( factor == 5 )
        {
            // radix-5
            for( i = 0; i < n0; i += n )
            {
                for( j = 0, dw = 0; j < nx; j++, dw += dw0 )
                {
                    CvComplex32f* v0 = dst + i + j;
                    CvComplex32f* v1 = v0 + nx*2;
                    CvComplex32f* v2 = v1 + nx*2;

                    double r0, i0, r1, i1, r2, i2, r3, i3, r4, i4, r5, i5;

                    r3 = v0[nx].re*wave[dw].re - v0[nx].im*wave[dw].im;
                    i3 = v0[nx].re*wave[dw].im + v0[nx].im*wave[dw].re;
                    r2 = v2[0].re*wave[dw*4].re - v2[0].im*wave[dw*4].im;
                    i2 = v2[0].re*wave[dw*4].im + v2[0].im*wave[dw*4].re;

                    r1 = r3 + r2; i1 = i3 + i2;
                    r3 -= r2; i3 -= i2;

                    r4 = v1[nx].re*wave[dw*3].re - v1[nx].im*wave[dw*3].im;
                    i4 = v1[nx].re*wave[dw*3].im + v1[nx].im*wave[dw*3].re;
                    r0 = v1[0].re*wave[dw*2].re - v1[0].im*wave[dw*2].im;
                    i0 = v1[0].re*wave[dw*2].im + v1[0].im*wave[dw*2].re;

                    r2 = r4 + r0; i2 = i4 + i0;
                    r4 -= r0; i4 -= i0;

                    r0 = v0[0].re; i0 = v0[0].im;
                    r5 = r1 + r2; i5 = i1 + i2;

                    v0[0].re = (float)(r0 + r5); v0[0].im = (float)(i0 + i5);

                    r0 -= 0.25*r5; i0 -= 0.25*i5;
                    r1 = icv_fft5_2*(r1 - r2); i1 = icv_fft5_2*(i1 - i2);
                    r2 = -icv_fft5_3*(i3 + i4); i2 = icv_fft5_3*(r3 + r4);

                    i3 *= -icv_fft5_5; r3 *= icv_fft5_5;
                    i4 *= -icv_fft5_4; r4 *= icv_fft5_4;

                    r5 = r2 + i3; i5 = i2 + r3;
                    r2 -= i4; i2 -= r4;
                    
                    r3 = r0 + r1; i3 = i0 + i1;
                    r0 -= r1; i0 -= i1;

                    v0[nx].re = (float)(r3 + r2); v0[nx].im = (float)(i3 + i2);
                    v2[0].re = (float)(r3 - r2); v2[0].im = (float)(i3 - i2);

                    v1[0].re = (float)(r0 + r5); v1[0].im = (float)(i0 + i5);
                    v1[nx].re = (float)(r0 - r5); v1[nx].im = (float)(i0 - i5);
                }
            }
        }
        else
        {
            // radix-"factor" - an odd number
            int p, q, factor2 = (factor - 1)/2;
            int d, dd, dw_f = tab_size/factor;
            CvComplex32f* a = buf;
            CvComplex32f* b = buf + factor2;

            for( i = 0; i < n0; i += n )
            {
                for( j = 0, dw = 0; j < nx; j++, dw += dw0 )
                {
                    CvComplex32f* v = dst + i + j;
                    CvComplex32f v_0 = v[0];
                    CvComplex64f vn_0 = v_0;

                    if( j == 0 )
                    {
                        for( p = 1, k = nx; p <= factor2; p++, k += nx )
                        {
                            double r0 = v[k].re + v[n-k].re;
                            double i0 = v[k].im - v[n-k].im;
                            double r1 = v[k].re - v[n-k].re;
                            double i1 = v[k].im + v[n-k].im;

                            vn_0.re += r0; vn_0.im += i1;
                            a[p-1].re = (float)r0; a[p-1].im = (float)i0;
                            b[p-1].re = (float)r1; b[p-1].im = (float)i1;
                        }
                    }
                    else
                    {
                        const CvComplex32f* wave_ = wave + dw*factor;
                        d = dw;

                        for( p = 1, k = nx; p <= factor2; p++, k += nx, d += dw )
                        {
                            double r2 = v[k].re*wave[d].re - v[k].im*wave[d].im;
                            double i2 = v[k].re*wave[d].im + v[k].im*wave[d].re;

                            double r1 = v[n-k].re*wave_[-d].re - v[n-k].im*wave_[-d].im;
                            double i1 = v[n-k].re*wave_[-d].im + v[n-k].im*wave_[-d].re;
                    
                            double r0 = r2 + r1;
                            double i0 = i2 - i1;
                            r1 = r2 - r1;
                            i1 = i2 + i1;

                            vn_0.re += r0; vn_0.im += i1;
                            a[p-1].re = (float)r0; a[p-1].im = (float)i0;
                            b[p-1].re = (float)r1; b[p-1].im = (float)i1;
                        }
                    }

                    v[0].re = (float)vn_0.re;
                    v[0].im = (float)vn_0.im;

                    for( p = 1, k = nx; p <= factor2; p++, k += nx )
                    {
                        CvComplex64f s0 = v_0, s1 = v_0;
                        d = dd = dw_f*p;

                        for( q = 0; q < factor2; q++ )
                        {
                            double r0 = wave[d].re * a[q].re;
                            double i0 = wave[d].im * a[q].im;
                            double r1 = wave[d].re * b[q].im;
                            double i1 = wave[d].im * b[q].re;
            
                            s1.re += r0 + i0; s0.re += r0 - i0;
                            s1.im += r1 - i1; s0.im += r1 + i1;

                            d += dd;
                            d -= -(d >= tab_size) & tab_size;
                        }

                        v[k].re = (float)s0.re;
                        v[k].im = (float)s0.im;
                        v[n-k].re = (float)s1.re;
                        v[n-k].im = (float)s1.im;
                    }
                }
            }
        }
    }

    if( scale != 1. )
    {
        double re_scale = scale, im_scale = scale;
        if( inv )
            im_scale = -im_scale;

        for( i = 0; i < n0; i++ )
        {
            double t0 = dst[i].re*re_scale;
            double t1 = dst[i].im*im_scale;
            dst[i].re = (float)t0;
            dst[i].im = (float)t1;
        }
    }
    else if( inv )
    {
        for( i = 0; i <= n0 - 2; i += 2 )
        {
            double t0 = -dst[i].im;
            double t1 = -dst[i+1].im;
            dst[i].im = (float)t0;
            dst[i+1].im = (float)t1;
        }

        if( i < n0 )
            dst[n0-1].im = -dst[n0-1].im;
    }

    return CV_OK;
}


/* FFT of real vector
   output vector format:
     re(0), re(1), im(1), ... , re(n/2-1), im((n+1)/2-1) [, re((n+1)/2)] OR ...
     re(0), 0, re(1), im(1), ..., re(n/2-1), im((n+1)/2-1) [, re((n+1)/2), 0] */
#define ICV_REAL_DFT( flavor, datatype )                                \
static CvStatus CV_STDCALL                                              \
icvRealDFT_##flavor( const datatype* src, datatype* dst,                \
                     int n, int nf, int* factors, const int* itab,      \
                     const CvComplex##flavor* wave, int tab_size,       \
                     const void* spec, CvComplex##flavor* buf,          \
                     int flags, double scale )                          \
{                                                                       \
    int complex_output = (flags & ICV_DFT_COMPLEX_INPUT_OR_OUTPUT) != 0;\
    int j, n2 = n >> 1;                                                 \
    dst += complex_output;                                              \
                                                                        \
    if( spec )                                                          \
    {                                                                   \
        icvDFTFwd_RToPack_##flavor##_p( src, dst, spec, buf );          \
        goto finalize;                                                  \
    }                                                                   \
                                                                        \
    assert( tab_size == n );                                            \
                                                                        \
    if( n == 1 )                                                        \
    {                                                                   \
        dst[0] = (datatype)(src[0]*scale);                              \
    }                                                                   \
    else if( n == 2 )                                                   \
    {                                                                   \
        double t = (src[0] + src[1])*scale;                             \
        dst[1] = (datatype)((src[0] - src[1])*scale);                   \
        dst[0] = (datatype)t;                                           \
    }                                                                   \
    else if( n & 1 )                                                    \
    {                                                                   \
        dst -= complex_output;                                          \
        CvComplex##flavor* _dst = (CvComplex##flavor*)dst;              \
        _dst[0].re = (datatype)(src[0]*scale);                          \
        _dst[0].im = 0;                                                 \
        for( j = 1; j < n; j += 2 )                                     \
        {                                                               \
            double t0 = src[itab[j]]*scale;                             \
            double t1 = src[itab[j+1]]*scale;                           \
            _dst[j].re = (datatype)t0;                                  \
            _dst[j].im = 0;                                             \
            _dst[j+1].re = (datatype)t1;                                \
            _dst[j+1].im = 0;                                           \
        }                                                               \
        icvDFT_##flavor##c( _dst, _dst, n, nf, factors, itab, wave,     \
                            tab_size, 0, buf, ICV_DFT_NO_PERMUTE, 1. ); \
        if( !complex_output )                                           \
            dst[1] = dst[0];                                            \
        return CV_OK;                                                   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        double t0, t;                                                   \
        double h1_re, h1_im, h2_re, h2_im;                              \
        double scale2 = scale*0.5;                                      \
        factors[0] >>= 1;                                               \
                                                                        \
        icvDFT_##flavor##c( (CvComplex##flavor*)src,                    \
                            (CvComplex##flavor*)dst, n2,                \
                            nf - (factors[0] == 1),                     \
                            factors + (factors[0] == 1),                \
                            itab, wave, tab_size, 0, buf, 0, 1. );      \
        factors[0] <<= 1;                                               \
                                                                        \
        t = dst[0] - dst[1];                                            \
        dst[0] = (datatype)((dst[0] + dst[1])*scale);                   \
        dst[1] = (datatype)(t*scale);                                   \
                                                                        \
        t0 = dst[n2];                                                   \
        t = dst[n-1];                                                   \
        dst[n-1] = dst[1];                                              \
                                                                        \
        for( j = 2, wave++; j < n2; j += 2, wave++ )                    \
        {                                                               \
            /* calc odd */                                              \
            h2_re = scale2*(dst[j+1] + t);                              \
            h2_im = scale2*(dst[n-j] - dst[j]);                         \
                                                                        \
            /* calc even */                                             \
            h1_re = scale2*(dst[j] + dst[n-j]);                         \
            h1_im = scale2*(dst[j+1] - t);                              \
                                                                        \
            /* rotate */                                                \
            t = h2_re*wave->re - h2_im*wave->im;                        \
            h2_im = h2_re*wave->im + h2_im*wave->re;                    \
            h2_re = t;                                                  \
            t = dst[n-j-1];                                             \
                                                                        \
            dst[j-1] = (datatype)(h1_re + h2_re);                       \
            dst[n-j-1] = (datatype)(h1_re - h2_re);                     \
            dst[j] = (datatype)(h1_im + h2_im);                         \
            dst[n-j] = (datatype)(h2_im - h1_im);                       \
        }                                                               \
                                                                        \
        if( j <= n2 )                                                   \
        {                                                               \
            dst[n2-1] = (datatype)(t0*scale);                           \
            dst[n2] = (datatype)(-t*scale);                             \
        }                                                               \
    }                                                                   \
                                                                        \
finalize:                                                               \
    if( complex_output )                                                \
    {                                                                   \
        dst[-1] = dst[0];                                               \
        dst[0] = 0;                                                     \
        if( (n & 1) == 0 )                                              \
            dst[n] = 0;                                                 \
    }                                                                   \
                                                                        \
    return CV_OK;                                                       \
}


/* Inverse FFT of complex conjugate-symmetric vector
   input vector format:
      re[0], re[1], im[1], ... , re[n/2-1], im[n/2-1], re[n/2] OR
      re(0), 0, re(1), im(1), ..., re(n/2-1), im((n+1)/2-1) [, re((n+1)/2), 0] */
#define ICV_CCS_IDFT( flavor, datatype )                                \
static CvStatus CV_STDCALL                                              \
icvCCSIDFT_##flavor( const datatype* src, datatype* dst,                \
                     int n, int nf, int* factors, const int* itab,      \
                     const CvComplex##flavor* wave, int tab_size,       \
                     const void* spec, CvComplex##flavor* buf,          \
                     int flags, double scale )                          \
{                                                                       \
    int complex_input = (flags & ICV_DFT_COMPLEX_INPUT_OR_OUTPUT) != 0; \
    int j, k, n2 = (n+1) >> 1;                                          \
    double save_s1 = 0.;                                                \
    double t0, t1, t2, t3, t;                                           \
                                                                        \
    assert( tab_size == n );                                            \
                                                                        \
    if( complex_input )                                                 \
    {                                                                   \
        assert( src != dst );                                           \
        save_s1 = src[1];                                               \
        ((datatype*)src)[1] = src[0];                                   \
        src++;                                                          \
    }                                                                   \
                                                                        \
    if( spec )                                                          \
    {                                                                   \
        icvDFTInv_PackToR_##flavor##_p( src, dst, spec, buf );          \
        goto finalize;                                                  \
    }                                                                   \
                                                                        \
    if( n == 1 )                                                        \
    {                                                                   \
        dst[0] = (datatype)(src[0]*scale);                              \
    }                                                                   \
    else if( n == 2 )                                                   \
    {                                                                   \
        t = (src[0] + src[1])*scale;                                    \
        dst[1] = (datatype)((src[0] - src[1])*scale);                   \
        dst[0] = (datatype)t;                                           \
    }                                                                   \
    else if( n & 1 )                                                    \
    {                                                                   \
        CvComplex##flavor* _src = (CvComplex##flavor*)(src-1);          \
        CvComplex##flavor* _dst = (CvComplex##flavor*)dst;              \
                                                                        \
        _dst[0].re = src[0];                                            \
        _dst[0].im = 0;                                                 \
        for( j = 1; j < n2; j++ )                                       \
        {                                                               \
            int k0 = itab[j], k1 = itab[n-j];                           \
            t0 = _src[j].re; t1 = _src[j].im;                           \
            _dst[k0].re = (datatype)t0; _dst[k0].im = (datatype)-t1;    \
            _dst[k1].re = (datatype)t0; _dst[k1].im = (datatype)t1;     \
        }                                                               \
                                                                        \
        icvDFT_##flavor##c( _dst, _dst, n, nf, factors, itab, wave,     \
                            tab_size, 0, buf, ICV_DFT_NO_PERMUTE, 1. ); \
        dst[0] = (datatype)(dst[0]*scale);                              \
        for( j = 1; j < n; j += 2 )                                     \
        {                                                               \
            t0 = dst[j*2]*scale;                                        \
            t1 = dst[j*2+2]*scale;                                      \
            dst[j] = (datatype)t0;                                      \
            dst[j+1] = (datatype)t1;                                    \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        int inplace = src == dst;                                       \
        const CvComplex##flavor* w = wave;                              \
                                                                        \
        t = src[1];                                                     \
        t0 = (src[0] + src[n-1]);                                       \
        t1 = (src[n-1] - src[0]);                                       \
        dst[0] = (datatype)t0;                                          \
        dst[1] = (datatype)t1;                                          \
                                                                        \
        for( j = 2, w++; j < n2; j += 2, w++ )                          \
        {                                                               \
            double h1_re, h1_im, h2_re, h2_im;                          \
                                                                        \
            h1_re = (t + src[n-j-1]);                                   \
            h1_im = (src[j] - src[n-j]);                                \
                                                                        \
            h2_re = (t - src[n-j-1]);                                   \
            h2_im = (src[j] + src[n-j]);                                \
                                                                        \
            t = h2_re*w->re + h2_im*w->im;                              \
            h2_im = h2_im*w->re - h2_re*w->im;                          \
            h2_re = t;                                                  \
                                                                        \
            t = src[j+1];                                               \
            t0 = h1_re - h2_im;                                         \
            t1 = -h1_im - h2_re;                                        \
            t2 = h1_re + h2_im;                                         \
            t3 = h1_im - h2_re;                                         \
                                                                        \
            if( inplace )                                               \
            {                                                           \
                dst[j] = (datatype)t0;                                  \
                dst[j+1] = (datatype)t1;                                \
                dst[n-j] = (datatype)t2;                                \
                dst[n-j+1]= (datatype)t3;                               \
            }                                                           \
            else                                                        \
            {                                                           \
                int j2 = j >> 1;                                        \
                k = itab[j2];                                           \
                dst[k] = (datatype)t0;                                  \
                dst[k+1] = (datatype)t1;                                \
                k = itab[n2-j2];                                        \
                dst[k] = (datatype)t2;                                  \
                dst[k+1]= (datatype)t3;                                 \
            }                                                           \
        }                                                               \
                                                                        \
        if( j <= n2 )                                                   \
        {                                                               \
            t0 = t*2;                                                   \
            t1 = src[n2]*2;                                             \
                                                                        \
            if( inplace )                                               \
            {                                                           \
                dst[n2] = (datatype)t0;                                 \
                dst[n2+1] = (datatype)t1;                               \
            }                                                           \
            else                                                        \
            {                                                           \
                k = itab[n2];                                           \
                dst[k*2] = (datatype)t0;                                \
                dst[k*2+1] = (datatype)t1;                              \
            }                                                           \
        }                                                               \
                                                                        \
        factors[0] >>= 1;                                               \
        icvDFT_##flavor##c( (CvComplex##flavor*)dst,                    \
                            (CvComplex##flavor*)dst, n2,                \
                            nf - (factors[0] == 1),                     \
                            factors + (factors[0] == 1), itab,          \
                            wave, tab_size, 0, buf,                     \
                            inplace ? 0 : ICV_DFT_NO_PERMUTE, 1. );     \
        factors[0] <<= 1;                                               \
                                                                        \
        for( j = 0; j < n; j += 2 )                                     \
        {                                                               \
            t0 = dst[j]*scale;                                          \
            t1 = dst[j+1]*(-scale);                                     \
            dst[j] = (datatype)t0;                                      \
            dst[j+1] = (datatype)t1;                                    \
        }                                                               \
    }                                                                   \
                                                                        \
finalize:                                                               \
    if( complex_input )                                                 \
        ((datatype*)src)[0] = (datatype)save_s1;                        \
                                                                        \
    return CV_OK;                                                       \
}


ICV_REAL_DFT( 64f, double )
ICV_CCS_IDFT( 64f, double )
ICV_REAL_DFT( 32f, float )
ICV_CCS_IDFT( 32f, float )


static void
icvCopyColumn( const uchar* _src, int src_step,
               uchar* _dst, int dst_step,
               int len, int elem_size )
{
    int i, t0, t1;
    const int* src = (const int*)_src;
    int* dst = (int*)_dst;
    src_step /= sizeof(src[0]);
    dst_step /= sizeof(dst[0]);

    if( elem_size == sizeof(int) )
    {
        for( i = 0; i < len; i++, src += src_step, dst += dst_step )
            dst[0] = src[0];
    }
    else if( elem_size == sizeof(int)*2 )
    {
        for( i = 0; i < len; i++, src += src_step, dst += dst_step )
        {
            t0 = src[0]; t1 = src[1];
            dst[0] = t0; dst[1] = t1;
        }
    }
    else if( elem_size == sizeof(int)*4 )
    {
        for( i = 0; i < len; i++, src += src_step, dst += dst_step )
        {
            t0 = src[0]; t1 = src[1];
            dst[0] = t0; dst[1] = t1;
            t0 = src[2]; t1 = src[3];
            dst[2] = t0; dst[3] = t1;
        }
    }
}


static void
icvCopyFrom2Columns( const uchar* _src, int src_step,
                     uchar* _dst0, uchar* _dst1,
                     int len, int elem_size )
{
    int i, t0, t1;
    const int* src = (const int*)_src;
    int* dst0 = (int*)_dst0;
    int* dst1 = (int*)_dst1;
    src_step /= sizeof(src[0]);

    if( elem_size == sizeof(int) )
    {
        for( i = 0; i < len; i++, src += src_step )
        {
            t0 = src[0]; t1 = src[1];
            dst0[i] = t0; dst1[i] = t1;
        }
    }
    else if( elem_size == sizeof(int)*2 )
    {
        for( i = 0; i < len*2; i += 2, src += src_step )
        {
            t0 = src[0]; t1 = src[1];
            dst0[i] = t0; dst0[i+1] = t1;
            t0 = src[2]; t1 = src[3];
            dst1[i] = t0; dst1[i+1] = t1;
        }
    }
    else if( elem_size == sizeof(int)*4 )
    {
        for( i = 0; i < len*4; i += 4, src += src_step )
        {
            t0 = src[0]; t1 = src[1];
            dst0[i] = t0; dst0[i+1] = t1;
            t0 = src[2]; t1 = src[3];
            dst0[i+2] = t0; dst0[i+3] = t1;
            t0 = src[4]; t1 = src[5];
            dst1[i] = t0; dst1[i+1] = t1;
            t0 = src[6]; t1 = src[7];
            dst1[i+2] = t0; dst1[i+3] = t1;
        }
    }
}


static void
icvCopyTo2Columns( const uchar* _src0, const uchar* _src1,
                   uchar* _dst, int dst_step,
                   int len, int elem_size )
{
    int i, t0, t1;
    const int* src0 = (const int*)_src0;
    const int* src1 = (const int*)_src1;
    int* dst = (int*)_dst;
    dst_step /= sizeof(dst[0]);

    if( elem_size == sizeof(int) )
    {
        for( i = 0; i < len; i++, dst += dst_step )
        {
            t0 = src0[i]; t1 = src1[i];
            dst[0] = t0; dst[1] = t1;
        }
    }
    else if( elem_size == sizeof(int)*2 )
    {
        for( i = 0; i < len*2; i += 2, dst += dst_step )
        {
            t0 = src0[i]; t1 = src0[i+1];
            dst[0] = t0; dst[1] = t1;
            t0 = src1[i]; t1 = src1[i+1];
            dst[2] = t0; dst[3] = t1;
        }
    }
    else if( elem_size == sizeof(int)*4 )
    {
        for( i = 0; i < len*4; i += 4, dst += dst_step )
        {
            t0 = src0[i]; t1 = src0[i+1];
            dst[0] = t0; dst[1] = t1;
            t0 = src0[i+2]; t1 = src0[i+3];
            dst[2] = t0; dst[3] = t1;
            t0 = src1[i]; t1 = src1[i+1];
            dst[4] = t0; dst[5] = t1;
            t0 = src1[i+2]; t1 = src1[i+3];
            dst[6] = t0; dst[7] = t1;
        }
    }
}


static void
icvExpandCCS( uchar* _ptr, int len, int elem_size )
{
    int i;
    _ptr -= elem_size;
    memcpy( _ptr, _ptr + elem_size, elem_size );
    memset( _ptr + elem_size, 0, elem_size );
    if( (len & 1) == 0 )
        memset( _ptr + (len+1)*elem_size, 0, elem_size );

    if( elem_size == sizeof(float) )
    {
        CvComplex32f* ptr = (CvComplex32f*)_ptr;

        for( i = 1; i < (len+1)/2; i++ )
        {
            CvComplex32f t;
            t.re = ptr[i].re;
            t.im = -ptr[i].im;
            ptr[len-i] = t;
        }
    }
    else
    {
        CvComplex64f* ptr = (CvComplex64f*)_ptr;

        for( i = 1; i < (len+1)/2; i++ )
        {
            CvComplex64f t;
            t.re = ptr[i].re;
            t.im = -ptr[i].im;
            ptr[len-i] = t;
        }
    }
}


typedef CvStatus (CV_STDCALL *CvDFTFunc)(
     const void* src, void* dst, int n, int nf, int* factors,
     const int* itab, const void* wave, int tab_size,
     const void* spec, void* buf, int inv, double scale );

CV_IMPL void
cvDFT( const CvArr* srcarr, CvArr* dstarr, int flags )
{
    static CvDFTFunc dft_tbl[6];
    static int inittab = 0;
    
    void* buffer = 0;
    int local_alloc = 1;
    int depth = -1;
    void *spec_c = 0, *spec_r = 0, *spec = 0;
    
    CV_FUNCNAME( "cvDFT" );

    __BEGIN__;

    int prev_len = 0, buf_size = 0, stage = 0;
    int nf = 0, inv = (flags & CV_DXT_INVERSE) != 0;
    int real_transform = 0;
    CvMat *src = (CvMat*)srcarr, *dst = (CvMat*)dstarr;
    CvMat srcstub, dststub, *src0;
    int complex_elem_size, elem_size;
    int factors[34], inplace_transform = 0;
    int ipp_norm_flag = 0;

    if( !inittab )
    {
        dft_tbl[0] = (CvDFTFunc)icvDFT_32fc;
        dft_tbl[1] = (CvDFTFunc)icvRealDFT_32f;
        dft_tbl[2] = (CvDFTFunc)icvCCSIDFT_32f;
        dft_tbl[3] = (CvDFTFunc)icvDFT_64fc;
        dft_tbl[4] = (CvDFTFunc)icvRealDFT_64f;
        dft_tbl[5] = (CvDFTFunc)icvCCSIDFT_64f;
        inittab = 1;
    }

    if( !CV_IS_MAT( src ))
    {
        int coi = 0;
        CV_CALL( src = cvGetMat( src, &srcstub, &coi ));

        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT( dst ))
    {
        int coi = 0;
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));

        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    src0 = src;
    elem_size = icvPixSize[src->type & CV_MAT_DEPTH_MASK];
    complex_elem_size = elem_size*2;

    // check types and sizes
    if( !CV_ARE_DEPTHS_EQ(src, dst) )
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    depth = CV_MAT_DEPTH(src->type);
    if( depth < CV_32F )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Only 32fC1, 32fC2, 64fC1 and 64fC2 formats are supported" );

    if( CV_ARE_CNS_EQ(src, dst) )
    {
        if( CV_MAT_CN(src->type) > 2 )
            CV_ERROR( CV_StsUnsupportedFormat,
            "Only 32fC1, 32fC2, 64fC1 and 64fC2 formats are supported" );

        if( !CV_ARE_SIZES_EQ(src, dst) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        real_transform = CV_MAT_CN(src->type) == 1;
        if( !real_transform )
            elem_size = complex_elem_size;
    }
    else if( !inv && CV_MAT_CN(src->type) == 1 && CV_MAT_CN(dst->type) == 2 )
    {
        if( (src->cols != 1 || dst->cols != 1 ||
            src->rows/2+1 != dst->rows && src->rows != dst->rows) &&
            (src->cols/2+1 != dst->cols || src->rows != dst->rows) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        real_transform = 1;
    }
    else if( inv && CV_MAT_CN(src->type) == 2 && CV_MAT_CN(dst->type) == 1 )
    {
        if( (src->cols != 1 || dst->cols != 1 ||
            dst->rows/2+1 != src->rows && src->rows != dst->rows) &&
            (dst->cols/2+1 != src->cols || src->rows != dst->rows) )
            CV_ERROR( CV_StsUnmatchedSizes, "" );
        real_transform = 1;
    }
    else
        CV_ERROR( CV_StsUnmatchedFormats,
        "Incorrect or unsupported combination of input & output formats" );

    // determine, which transform to do first - row-wise
    // (stage 0) or column-wise (stage 1) transform
    if( !(flags & CV_DXT_ROWS) && src->rows > 1 &&
        (src->cols == 1 && !CV_IS_MAT_CONT(src->type & dst->type) ||
        src->cols > 1 && inv && real_transform) )
        stage = 1;

    ipp_norm_flag = !(flags & CV_DXT_SCALE) ? 8 : (flags & CV_DXT_INVERSE) ? 2 : 1;

    for(;;)
    {
        double scale = 1;
        uchar* wave = 0;
        int* itab = 0;
        uchar* ptr;
        int i, len, count, sz = 0;
        int use_buf = 0, odd_real = 0;
        CvDFTFunc dft_func;

        if( stage == 0 ) // row-wise transform
        {
            len = !inv ? src->cols : dst->cols;
            count = src->rows;
            if( len == 1 && !(flags & CV_DXT_ROWS) )
            {
                len = !inv ? src->rows : dst->rows;
                count = 1;
            }
            odd_real = real_transform && (len & 1);
        }
        else
        {
            len = dst->rows;
            count = !inv ? src0->cols : dst->cols;
            sz = 2*len*complex_elem_size;
        }

        spec = 0;
        if( len*count >= 64 && icvDFTInitAlloc_R_32f_p != 0 ) // use IPP DFT if available
        {
            int ipp_sz = 0;
            
            if( real_transform && stage == 0 )
            {
                if( depth == CV_32F )
                {
                    if( spec_r )
                        IPPI_CALL( icvDFTFree_R_32f_p( spec_r ));
                    IPPI_CALL( icvDFTInitAlloc_R_32f_p(
                        &spec_r, len, ipp_norm_flag, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_R_32f_p( spec_r, &ipp_sz ));
                }
                else
                {
                    if( spec_r )
                        IPPI_CALL( icvDFTFree_R_64f_p( spec_r ));
                    IPPI_CALL( icvDFTInitAlloc_R_64f_p(
                        &spec_r, len, ipp_norm_flag, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_R_64f_p( spec_r, &ipp_sz ));
                }
                spec = spec_r;
            }
            else
            {
                if( depth == CV_32F )
                {
                    if( spec_c )
                        IPPI_CALL( icvDFTFree_C_32fc_p( spec_c ));
                    IPPI_CALL( icvDFTInitAlloc_C_32fc_p(
                        &spec_c, len, ipp_norm_flag, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_C_32fc_p( spec_c, &ipp_sz ));
                }
                else
                {
                    if( spec_c )
                        IPPI_CALL( icvDFTFree_C_64fc_p( spec_c ));
                    IPPI_CALL( icvDFTInitAlloc_C_64fc_p(
                        &spec_c, len, ipp_norm_flag, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_C_64fc_p( spec_c, &ipp_sz ));
                }
                spec = spec_c;
            }

            sz += ipp_sz;
        }
        else
        {
            if( len != prev_len )
                nf = icvDFTFactorize( len, factors );

            inplace_transform = factors[0] == factors[nf-1];
            sz += len*(complex_elem_size + sizeof(int));
            i = nf > 1 && (factors[0] & 1) == 0;
            if( (factors[i] & 1) != 0 && factors[i] > 5 )
                sz += (factors[i]+1)*complex_elem_size;

            if( stage == 0 && (src->data.ptr == dst->data.ptr && !inplace_transform || odd_real) ||
                stage == 1 && !inplace_transform )
            {
                use_buf = 1;
                sz += len*complex_elem_size;
            }
        }

        if( sz > buf_size )
        {
            prev_len = 0; // because we release the buffer, 
                          // force recalculation of
                          // twiddle factors and permutation table
            if( !local_alloc && buffer )
                cvFree( &buffer );
            if( sz <= CV_MAX_LOCAL_DFT_SIZE )
            {
                buf_size = sz = CV_MAX_LOCAL_DFT_SIZE;
                buffer = cvStackAlloc(sz + 32);
                local_alloc = 1;
            }
            else
            {
                CV_CALL( buffer = cvAlloc(sz + 32) );
                buf_size = sz;
                local_alloc = 0;
            }
        }

        ptr = (uchar*)buffer;
        if( !spec )
        {
            wave = ptr;
            ptr += len*complex_elem_size;
            itab = (int*)ptr;
            ptr = (uchar*)cvAlignPtr( ptr + len*sizeof(int), 16 );

            if( len != prev_len || (!inplace_transform && inv && real_transform))
                icvDFTInit( len, nf, factors, itab, complex_elem_size,
                            wave, stage == 0 && inv && real_transform );
            // otherwise reuse the tables calculated on the previous stage
        }

        if( stage == 0 )
        {
            uchar* tmp_buf = 0;
            int dptr_offset = 0;
            int dst_full_len = len*elem_size;
            int _flags = inv + (CV_MAT_CN(src->type) != CV_MAT_CN(dst->type) ?
                         ICV_DFT_COMPLEX_INPUT_OR_OUTPUT : 0);
            if( use_buf )
            {
                tmp_buf = ptr;
                ptr += len*complex_elem_size;
                if( odd_real && !inv && len > 1 &&
                    !(_flags & ICV_DFT_COMPLEX_INPUT_OR_OUTPUT))
                    dptr_offset = elem_size;
            }

            if( !inv && (_flags & ICV_DFT_COMPLEX_INPUT_OR_OUTPUT) )
                dst_full_len += (len & 1) ? elem_size : complex_elem_size;

            dft_func = dft_tbl[(!real_transform ? 0 : !inv ? 1 : 2) + (depth == CV_64F)*3];

            if( count > 1 && !(flags & CV_DXT_ROWS) && (!inv || !real_transform) )
                stage = 1;
            else if( flags & CV_DXT_SCALE )
                scale = 1./(len * (flags & CV_DXT_ROWS ? 1 : count));

            for( i = 0; i < count; i++ )
            {
                uchar* sptr = src->data.ptr + i*src->step;
                uchar* dptr0 = dst->data.ptr + i*dst->step;
                uchar* dptr = dptr0;

                if( tmp_buf )
                    dptr = tmp_buf;
                
                dft_func( sptr, dptr, len, nf, factors, itab, wave, len, spec, ptr, _flags, scale );
                if( dptr != dptr0 )
                    memcpy( dptr0, dptr + dptr_offset, dst_full_len );
            }

            if( stage != 1 )
                break;
            src = dst;
        }
        else
        {
            int a = 0, b = count;
            uchar *buf0, *buf1, *dbuf0, *dbuf1;
            uchar* sptr0 = src->data.ptr;
            uchar* dptr0 = dst->data.ptr;
            buf0 = ptr;
            ptr += len*complex_elem_size;
            buf1 = ptr;
            ptr += len*complex_elem_size;
            dbuf0 = buf0, dbuf1 = buf1;
            
            if( use_buf )
            {
                dbuf1 = ptr;
                dbuf0 = buf1;
                ptr += len*complex_elem_size;
            }

            dft_func = dft_tbl[(depth == CV_64F)*3];

            if( real_transform && inv && src->cols > 1 )
                stage = 0;
            else if( flags & CV_DXT_SCALE )
                scale = 1./(len * count);

            if( real_transform )
            {
                int even;
                a = 1;
                even = (count & 1) == 0;
                b = (count+1)/2;
                if( !inv )
                {
                    memset( buf0, 0, len*complex_elem_size );
                    icvCopyColumn( sptr0, src->step, buf0, complex_elem_size, len, elem_size );
                    sptr0 += CV_MAT_CN(dst->type)*elem_size;
                    if( even )
                    {
                        memset( buf1, 0, len*complex_elem_size );
                        icvCopyColumn( sptr0 + (count-2)*elem_size, src->step,
                                       buf1, complex_elem_size, len, elem_size );
                    }
                }
                else if( CV_MAT_CN(src->type) == 1 )
                {
                    icvCopyColumn( sptr0, src->step, buf0 + elem_size, elem_size, len, elem_size );
                    icvExpandCCS( buf0 + elem_size, len, elem_size );
                    if( even )
                    {
                        icvCopyColumn( sptr0 + (count-1)*elem_size, src->step,
                                       buf1 + elem_size, elem_size, len, elem_size );
                        icvExpandCCS( buf1 + elem_size, len, elem_size );
                    }
                    sptr0 += elem_size;
                }
                else
                {
                    icvCopyColumn( sptr0, src->step, buf0, complex_elem_size, len, complex_elem_size );
                    //memcpy( buf0 + elem_size, buf0, elem_size );
                    //icvExpandCCS( buf0 + elem_size, len, elem_size );
                    if( even )
                    {
                        icvCopyColumn( sptr0 + b*complex_elem_size, src->step,
                                       buf1, complex_elem_size, len, complex_elem_size );
                        //memcpy( buf0 + elem_size, buf0, elem_size );
                        //icvExpandCCS( buf0 + elem_size, len, elem_size );
                    }
                    sptr0 += complex_elem_size;
                }
                
                if( even )
                    IPPI_CALL( dft_func( buf1, dbuf1, len, nf, factors, itab,
                                         wave, len, spec, ptr, inv, scale ));
                IPPI_CALL( dft_func( buf0, dbuf0, len, nf, factors, itab,
                                     wave, len, spec, ptr, inv, scale ));

                if( CV_MAT_CN(dst->type) == 1 )
                {
                    if( !inv )
                    {
                        // copy the half of output vector to the first/last column.
                        // before doing that, defgragment the vector
                        memcpy( dbuf0 + elem_size, dbuf0, elem_size );
                        icvCopyColumn( dbuf0 + elem_size, elem_size, dptr0,
                                       dst->step, len, elem_size );
                        if( even )
                        {
                            memcpy( dbuf1 + elem_size, dbuf1, elem_size );
                            icvCopyColumn( dbuf1 + elem_size, elem_size,
                                           dptr0 + (count-1)*elem_size,
                                           dst->step, len, elem_size );
                        }
                        dptr0 += elem_size;
                    }
                    else
                    {
                        // copy the real part of the complex vector to the first/last column
                        icvCopyColumn( dbuf0, complex_elem_size, dptr0, dst->step, len, elem_size );
                        if( even )
                            icvCopyColumn( dbuf1, complex_elem_size, dptr0 + (count-1)*elem_size,
                                           dst->step, len, elem_size );
                        dptr0 += elem_size;
                    }
                }
                else
                {
                    assert( !inv );
                    icvCopyColumn( dbuf0, complex_elem_size, dptr0,
                                   dst->step, len, complex_elem_size );
                    if( even )
                        icvCopyColumn( dbuf1, complex_elem_size,
                                       dptr0 + b*complex_elem_size,
                                       dst->step, len, complex_elem_size );
                    dptr0 += complex_elem_size;
                }
            }

            for( i = a; i < b; i += 2 )
            {
                if( i+1 < b )
                {
                    icvCopyFrom2Columns( sptr0, src->step, buf0, buf1, len, complex_elem_size );
                    IPPI_CALL( dft_func( buf1, dbuf1, len, nf, factors, itab,
                                         wave, len, spec, ptr, inv, scale ));
                }
                else
                    icvCopyColumn( sptr0, src->step, buf0, complex_elem_size, len, complex_elem_size );

                IPPI_CALL( dft_func( buf0, dbuf0, len, nf, factors, itab,
                                     wave, len, spec, ptr, inv, scale ));

                if( i+1 < b )
                    icvCopyTo2Columns( dbuf0, dbuf1, dptr0, dst->step, len, complex_elem_size );
                else
                    icvCopyColumn( dbuf0, complex_elem_size, dptr0, dst->step, len, complex_elem_size );
                sptr0 += 2*complex_elem_size;
                dptr0 += 2*complex_elem_size;
            }

            if( stage != 0 )
                break;
            src = dst;
        }
    }

    __END__;

    if( buffer && !local_alloc )
        cvFree( (void**)&buffer );

    if( spec_c )
    {
        if( depth == CV_32F )
            icvDFTFree_C_32fc_p( spec_c );
        else
            icvDFTFree_C_64fc_p( spec_c );
    }

    if( spec_r )
    {
        if( depth == CV_32F )
            icvDFTFree_R_32f_p( spec_r );
        else
            icvDFTFree_R_64f_p( spec_r );
    }
}


CV_IMPL void
cvMulSpectrums( const CvArr* srcAarr, const CvArr* srcBarr,
                CvArr* dstarr, int flags )
{
    CV_FUNCNAME( "cvMulSpectrums" );

    __BEGIN__;

    CvMat stubA, *srcA = (CvMat*)srcAarr;
    CvMat stubB, *srcB = (CvMat*)srcBarr;
    CvMat dststub, *dst = (CvMat*)dstarr;
    int stepA, stepB, stepC;
    int type, cn, is_1d;
    int j, j0, j1, k, rows, cols, ncols;

    if( !CV_IS_MAT(srcA))
        CV_CALL( srcA = cvGetMat( srcA, &stubA, 0 ));

    if( !CV_IS_MAT(srcB))
        CV_CALL( srcB = cvGetMat( srcB, &stubB, 0 ));

    if( !CV_IS_MAT(dst))
        CV_CALL( dst = cvGetMat( dst, &dststub, 0 ));

    if( !CV_ARE_TYPES_EQ( srcA, srcB ) || !CV_ARE_TYPES_EQ( srcA, dst ))
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( !CV_ARE_SIZES_EQ( srcA, dst ) || !CV_ARE_SIZES_EQ( srcA, dst ))
        CV_ERROR( CV_StsUnmatchedSizes, "" );

    type = CV_MAT_TYPE( dst->type );
    cn = CV_MAT_CN(type);
    rows = srcA->rows;
    cols = srcA->cols;
    is_1d = (flags & CV_DXT_ROWS) ||
            (rows == 1 || cols == 1 &&
             CV_IS_MAT_CONT( srcA->type & srcB->type & dst->type ));

    if( is_1d && !(flags & CV_DXT_ROWS) )
        cols = cols + rows - 1, rows = 1;
    ncols = cols*cn;
    j0 = cn == 1;
    j1 = ncols - (cols % 2 == 0 && cn == 1);

    if( CV_MAT_DEPTH(type) == CV_32F )
    {
        float* dataA = srcA->data.fl;
        float* dataB = srcB->data.fl;
        float* dataC = dst->data.fl;

        stepA = srcA->step/sizeof(dataA[0]);
        stepB = srcB->step/sizeof(dataB[0]);
        stepC = dst->step/sizeof(dataC[0]);

        if( !is_1d && cn == 1 )
        {
            for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
            {
                if( k == 1 )
                    dataA += cols - 1, dataB += cols - 1, dataC += cols - 1;
                dataC[0] = dataA[0]*dataB[0];
                if( rows % 2 == 0 )
                    dataC[(rows-1)*stepC] = dataA[(rows-1)*stepA]*dataB[(rows-1)*stepB];
                if( !(flags & CV_DXT_MUL_CONJ) )
                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        double re = (double)dataA[j*stepA]*dataB[j*stepB] -
                                    (double)dataA[(j+1)*stepA]*dataB[(j+1)*stepB];
                        double im = (double)dataA[j*stepA]*dataB[(j+1)*stepB] +
                                    (double)dataA[(j+1)*stepA]*dataB[j*stepB];
                        dataC[j*stepC] = (float)re; dataC[(j+1)*stepC] = (float)im;
                    }
                else
                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        double re = (double)dataA[j*stepA]*dataB[j*stepB] +
                                    (double)dataA[(j+1)*stepA]*dataB[(j+1)*stepB];
                        double im = (double)dataA[(j+1)*stepA]*dataB[j*stepB] -
                                    (double)dataA[j*stepA]*dataB[(j+1)*stepB];
                        dataC[j*stepC] = (float)re; dataC[(j+1)*stepC] = (float)im;
                    }
                if( k == 1 )
                    dataA -= cols - 1, dataB -= cols - 1, dataC -= cols - 1;
            }
        }

        for( ; rows--; dataA += stepA, dataB += stepB, dataC += stepC )
        {
            if( is_1d && cn == 1 )
            {
                dataC[0] = dataA[0]*dataB[0];
                if( cols % 2 == 0 )
                    dataC[j1] = dataA[j1]*dataB[j1];
            }

            if( !(flags & CV_DXT_MUL_CONJ) )
                for( j = j0; j < j1; j += 2 )
                {
                    double re = (double)dataA[j]*dataB[j] - (double)dataA[j+1]*dataB[j+1];
                    double im = (double)dataA[j+1]*dataB[j] + (double)dataA[j]*dataB[j+1];
                    dataC[j] = (float)re; dataC[j+1] = (float)im;
                }
            else
                for( j = j0; j < j1; j += 2 )
                {
                    double re = (double)dataA[j]*dataB[j] + (double)dataA[j+1]*dataB[j+1];
                    double im = (double)dataA[j+1]*dataB[j] - (double)dataA[j]*dataB[j+1];
                    dataC[j] = (float)re; dataC[j+1] = (float)im;
                }
        }
    }
    else if( CV_MAT_DEPTH(type) == CV_64F )
    {
        double* dataA = srcA->data.db;
        double* dataB = srcB->data.db;
        double* dataC = dst->data.db;

        stepA = srcA->step/sizeof(dataA[0]);
        stepB = srcB->step/sizeof(dataB[0]);
        stepC = dst->step/sizeof(dataC[0]);

        if( !is_1d && cn == 1 )
        {
            for( k = 0; k < (cols % 2 ? 1 : 2); k++ )
            {
                if( k == 1 )
                    dataA += cols - 1, dataB += cols - 1, dataC += cols - 1;
                dataC[0] = dataA[0]*dataB[0];
                if( rows % 2 == 0 )
                    dataC[(rows-1)*stepC] = dataA[(rows-1)*stepA]*dataB[(rows-1)*stepB];
                if( !(flags & CV_DXT_MUL_CONJ) )
                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        double re = dataA[j*stepA]*dataB[j*stepB] -
                                    dataA[(j+1)*stepA]*dataB[(j+1)*stepB];
                        double im = dataA[j*stepA]*dataB[(j+1)*stepB] +
                                    dataA[(j+1)*stepA]*dataB[j*stepB];
                        dataC[j*stepC] = re; dataC[(j+1)*stepC] = im;
                    }
                else
                    for( j = 1; j <= rows - 2; j += 2 )
                    {
                        double re = dataA[j*stepA]*dataB[j*stepB] +
                                    dataA[(j+1)*stepA]*dataB[(j+1)*stepB];
                        double im = dataA[(j+1)*stepA]*dataB[j*stepB] -
                                    dataA[j*stepA]*dataB[(j+1)*stepB];
                        dataC[j*stepC] = re; dataC[(j+1)*stepC] = im;
                    }
                if( k == 1 )
                    dataA -= cols - 1, dataB -= cols - 1, dataC -= cols - 1;
            }
        }

        for( ; rows--; dataA += stepA, dataB += stepB, dataC += stepC )
        {
            if( is_1d && cn == 1 )
            {
                dataC[0] = dataA[0]*dataB[0];
                if( cols % 2 == 0 )
                    dataC[j1] = dataA[j1]*dataB[j1];
            }

            if( !(flags & CV_DXT_MUL_CONJ) )
                for( j = j0; j < j1; j += 2 )
                {
                    double re = dataA[j]*dataB[j] - dataA[j+1]*dataB[j+1];
                    double im = dataA[j+1]*dataB[j] + dataA[j]*dataB[j+1];
                    dataC[j] = re; dataC[j+1] = im;
                }
            else
                for( j = j0; j < j1; j += 2 )
                {
                    double re = dataA[j]*dataB[j] + dataA[j+1]*dataB[j+1];
                    double im = dataA[j+1]*dataB[j] - dataA[j]*dataB[j+1];
                    dataC[j] = re; dataC[j+1] = im;
                }
        }
    }
    else
    {
        CV_ERROR( CV_StsUnsupportedFormat, "Only 32f and 64f types are supported" );
    }

    __END__;
}


/****************************************************************************************\
                               Discrete Cosine Transform
\****************************************************************************************/

/* DCT is calculated using DFT, as described here:
   http://www.ece.utexas.edu/~bevans/courses/ee381k/lectures/09_DCT/lecture9/:
*/
#define ICV_DCT_FWD( flavor, datatype )                                 \
static CvStatus CV_STDCALL                                              \
icvDCT_fwd_##flavor( const datatype* src, int src_step, datatype* dft_src,\
                     datatype* dft_dst, datatype* dst, int dst_step,    \
                     int n, int nf, int* factors, const int* itab,      \
                     const CvComplex##flavor* dft_wave,                 \
                     const CvComplex##flavor* dct_wave,                 \
                     const void* spec, CvComplex##flavor* buf )         \
{                                                                       \
    int j, n2 = n >> 1;                                                 \
                                                                        \
    src_step /= sizeof(src[0]);                                         \
    dst_step /= sizeof(dst[0]);                                         \
    datatype* dst1 = dst + (n-1)*dst_step;                              \
                                                                        \
    if( n == 1 )                                                        \
    {                                                                   \
        dst[0] = src[0];                                                \
        return CV_OK;                                                   \
    }                                                                   \
                                                                        \
    for( j = 0; j < n2; j++, src += src_step*2 )                        \
    {                                                                   \
        dft_src[j] = src[0];                                            \
        dft_src[n-j-1] = src[src_step];                                 \
    }                                                                   \
                                                                        \
    icvRealDFT_##flavor( dft_src, dft_dst, n, nf, factors,              \
                         itab, dft_wave, n, spec, buf, 0, 1.0 );        \
    src = dft_dst;                                                      \
                                                                        \
    dst[0] = (datatype)(src[0]*dct_wave->re*icv_sin_45);                \
    dst += dst_step;                                                    \
    for( j = 1, dct_wave++; j < n2; j++, dct_wave++,                    \
                                    dst += dst_step, dst1 -= dst_step ) \
    {                                                                   \
        double t0 = dct_wave->re*src[j*2-1] - dct_wave->im*src[j*2];    \
        double t1 = -dct_wave->im*src[j*2-1] - dct_wave->re*src[j*2];   \
        dst[0] = (datatype)t0;                                          \
        dst1[0] = (datatype)t1;                                         \
    }                                                                   \
                                                                        \
    dst[0] = (datatype)(src[n-1]*dct_wave->re);                         \
    return CV_OK;                                                       \
}


#define ICV_DCT_INV( flavor, datatype )                                 \
static CvStatus CV_STDCALL                                              \
icvDCT_inv_##flavor( const datatype* src, int src_step, datatype* dft_src,\
                     datatype* dft_dst, datatype* dst, int dst_step,    \
                     int n, int nf, int* factors, const int* itab,      \
                     const CvComplex##flavor* dft_wave,                 \
                     const CvComplex##flavor* dct_wave,                 \
                     const void* spec, CvComplex##flavor* buf )         \
{                                                                       \
    int j, n2 = n >> 1;                                                 \
                                                                        \
    src_step /= sizeof(src[0]);                                         \
    dst_step /= sizeof(dst[0]);                                         \
    const datatype* src1 = src + (n-1)*src_step;                        \
                                                                        \
    if( n == 1 )                                                        \
    {                                                                   \
        dst[0] = src[0];                                                \
        return CV_OK;                                                   \
    }                                                                   \
                                                                        \
    dft_src[0] = (datatype)(src[0]*2*dct_wave->re*icv_sin_45);          \
    src += src_step;                                                    \
    for( j = 1, dct_wave++; j < n2; j++, dct_wave++,                    \
                                    src += src_step, src1 -= src_step ) \
    {                                                                   \
        double t0 = dct_wave->re*src[0] - dct_wave->im*src1[0];         \
        double t1 = -dct_wave->im*src[0] - dct_wave->re*src1[0];        \
        dft_src[j*2-1] = (datatype)t0;                                  \
        dft_src[j*2] = (datatype)t1;                                    \
    }                                                                   \
                                                                        \
    dft_src[n-1] = (datatype)(src[0]*2*dct_wave->re);                   \
    icvCCSIDFT_##flavor( dft_src, dft_dst, n, nf, factors, itab,        \
                         dft_wave, n, spec, buf, CV_DXT_INVERSE, 1.0 ); \
                                                                        \
    for( j = 0; j < n2; j++, dst += dst_step*2 )                        \
    {                                                                   \
        dst[0] = dft_dst[j];                                            \
        dst[dst_step] = dft_dst[n-j-1];                                 \
    }                                                                   \
    return CV_OK;                                                       \
}


ICV_DCT_FWD( 64f, double )
ICV_DCT_INV( 64f, double )
ICV_DCT_FWD( 32f, float )
ICV_DCT_INV( 32f, float )

static void
icvDCTInit( int n, int elem_size, void* _wave, int inv )
{
    static const double icvDctScale[] =
    {
    0.707106781186547570, 0.500000000000000000, 0.353553390593273790,
    0.250000000000000000, 0.176776695296636890, 0.125000000000000000,
    0.088388347648318447, 0.062500000000000000, 0.044194173824159223,
    0.031250000000000000, 0.022097086912079612, 0.015625000000000000,
    0.011048543456039806, 0.007812500000000000, 0.005524271728019903,
    0.003906250000000000, 0.002762135864009952, 0.001953125000000000,
    0.001381067932004976, 0.000976562500000000, 0.000690533966002488,
    0.000488281250000000, 0.000345266983001244, 0.000244140625000000,
    0.000172633491500622, 0.000122070312500000, 0.000086316745750311,
    0.000061035156250000, 0.000043158372875155, 0.000030517578125000
    };

    int i;
    CvComplex64f w, w1;
    double t, scale;
    
    if( n == 1 )
        return;

    assert( (n&1) == 0 );

    if( (n & (n - 1)) == 0 )
    {
        int m = icvlog2(n);
        scale = (!inv ? 2 : 1)*icvDctScale[m];
        w1.re = icvDxtTab[m+2][0];
        w1.im = -icvDxtTab[m+2][1];
    }
    else
    {
        t = 1./(2*n);
        scale = (!inv ? 2 : 1)*sqrt(t);
        w1.im = sin(-CV_PI*t);
        w1.re = sqrt(1. - w1.im*w1.im);
    }
    n >>= 1;
    
    if( elem_size == sizeof(CvComplex64f) )
    {
        CvComplex64f* wave = (CvComplex64f*)_wave;

        w.re = scale;
        w.im = 0.;

        for( i = 0; i <= n; i++ )
        {
            wave[i] = w;
            t = w.re*w1.re - w.im*w1.im;
            w.im = w.re*w1.im + w.im*w1.re;
            w.re = t;
        }
    }
    else
    {
        CvComplex32f* wave = (CvComplex32f*)_wave;
        assert( elem_size == sizeof(CvComplex32f) );
        
        w.re = (float)scale;
        w.im = 0.f;

        for( i = 0; i <= n; i++ )
        {
            wave[i].re = (float)w.re;
            wave[i].im = (float)w.im;
            t = w.re*w1.re - w.im*w1.im;
            w.im = w.re*w1.im + w.im*w1.re;
            w.re = t;
        }
    }
}


typedef CvStatus (CV_STDCALL * CvDCTFunc)(
                const void* src, int src_step, void* dft_src,
                void* dft_dst, void* dst, int dst_step, int n,
                int nf, int* factors, const int* itab, const void* dft_wave,
                const void* dct_wave, const void* spec, void* buf );

CV_IMPL void
cvDCT( const CvArr* srcarr, CvArr* dstarr, int flags )
{
    static CvDCTFunc dct_tbl[4];
    static int inittab = 0;
    
    void* buffer = 0;
    int local_alloc = 1;
    int inv = (flags & CV_DXT_INVERSE) != 0, depth = -1;
    void *spec_dft = 0, *spec = 0;
    
    CV_FUNCNAME( "cvDCT" );

    __BEGIN__;

    double scale = 1.;
    int prev_len = 0, buf_size = 0, nf = 0, stage, end_stage;
    CvMat *src = (CvMat*)srcarr, *dst = (CvMat*)dstarr;
    uchar *src_dft_buf = 0, *dst_dft_buf = 0;
    uchar *dft_wave = 0, *dct_wave = 0;
    int* itab = 0;
    uchar* ptr = 0;
    CvMat srcstub, dststub;
    int complex_elem_size, elem_size;
    int factors[34], inplace_transform;
    int i, len, count;
    CvDCTFunc dct_func;

    if( !inittab )
    {
        dct_tbl[0] = (CvDCTFunc)icvDCT_fwd_32f;
        dct_tbl[1] = (CvDCTFunc)icvDCT_inv_32f;
        dct_tbl[2] = (CvDCTFunc)icvDCT_fwd_64f;
        dct_tbl[3] = (CvDCTFunc)icvDCT_inv_64f;
        inittab = 1;
    }

    if( !CV_IS_MAT( src ))
    {
        int coi = 0;
        CV_CALL( src = cvGetMat( src, &srcstub, &coi ));

        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    if( !CV_IS_MAT( dst ))
    {
        int coi = 0;
        CV_CALL( dst = cvGetMat( dst, &dststub, &coi ));

        if( coi != 0 )
            CV_ERROR( CV_BadCOI, "" );
    }

    depth = CV_MAT_DEPTH(src->type);
    elem_size = icvPixSize[depth];
    complex_elem_size = elem_size*2;

    // check types and sizes
    if( !CV_ARE_TYPES_EQ(src, dst) )
        CV_ERROR( CV_StsUnmatchedFormats, "" );

    if( depth < CV_32F || CV_MAT_CN(src->type) != 1 )
        CV_ERROR( CV_StsUnsupportedFormat,
        "Only 32fC1 and 64fC1 formats are supported" );

    dct_func = dct_tbl[inv + (depth == CV_64F)*2];

    if( (flags & CV_DXT_ROWS) || src->rows == 1 ||
        src->cols == 1 && CV_IS_MAT_CONT(src->type & dst->type))
    {
        stage = end_stage = 0;
    }
    else
    {
        stage = src->cols == 1;
        end_stage = 1;
    }

    for( ; stage <= end_stage; stage++ )
    {
        uchar *sptr = src->data.ptr, *dptr = dst->data.ptr;
        int sstep0, sstep1, dstep0, dstep1;
        
        if( stage == 0 )
        {
            len = src->cols;
            count = src->rows;
            if( len == 1 && !(flags & CV_DXT_ROWS) )
            {
                len = src->rows;
                count = 1;
            }
            sstep0 = src->step;
            dstep0 = dst->step;
            sstep1 = dstep1 = elem_size;
        }
        else
        {
            len = dst->rows;
            count = dst->cols;
            sstep1 = src->step;
            dstep1 = dst->step;
            sstep0 = dstep0 = elem_size;
        }

        if( len != prev_len )
        {
            int sz;

            if( len > 1 && (len & 1) )
                CV_ERROR( CV_StsNotImplemented, "Odd-size DCT\'s are not implemented" );

            sz = len*elem_size;
            sz += (len/2 + 1)*complex_elem_size;

            spec = 0;
            inplace_transform = 1;
            if( len*count >= 64 && icvDFTInitAlloc_R_32f_p )
            {
                int ipp_sz = 0;
                if( depth == CV_32F )
                {
                    if( spec_dft )
                        IPPI_CALL( icvDFTFree_R_32f_p( spec_dft ));
                    IPPI_CALL( icvDFTInitAlloc_R_32f_p( &spec_dft, len, 8, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_R_32f_p( spec_dft, &ipp_sz ));
                }
                else
                {
                    if( spec_dft )
                        IPPI_CALL( icvDFTFree_R_64f_p( spec_dft ));
                    IPPI_CALL( icvDFTInitAlloc_R_64f_p( &spec_dft, len, 8, cvAlgHintNone ));
                    IPPI_CALL( icvDFTGetBufSize_R_64f_p( spec_dft, &ipp_sz ));
                }
                spec = spec_dft;
                sz += ipp_sz;
            }
            else
            {
                sz += len*(complex_elem_size + sizeof(int)) + complex_elem_size;

                nf = icvDFTFactorize( len, factors );
                inplace_transform = factors[0] == factors[nf-1];

                i = nf > 1 && (factors[0] & 1) == 0;
                if( (factors[i] & 1) != 0 && factors[i] > 5 )
                    sz += (factors[i]+1)*complex_elem_size;

                if( !inplace_transform )
                    sz += len*elem_size;
            }

            if( sz > buf_size )
            {
                if( !local_alloc && buffer )
                    cvFree( &buffer );
                if( sz <= CV_MAX_LOCAL_DFT_SIZE )
                {
                    buf_size = sz = CV_MAX_LOCAL_DFT_SIZE;
                    buffer = cvStackAlloc(sz + 32);
                    local_alloc = 1;
                }
                else
                {
                    CV_CALL( buffer = cvAlloc(sz + 32) );
                    buf_size = sz;
                    local_alloc = 0;
                }
            }

            ptr = (uchar*)buffer;
            if( !spec )
            {
                dft_wave = ptr;
                ptr += len*complex_elem_size;
                itab = (int*)ptr;
                ptr = (uchar*)cvAlignPtr( ptr + len*sizeof(int), 16 );
                icvDFTInit( len, nf, factors, itab, complex_elem_size, dft_wave, inv );
            }
                
            dct_wave = ptr;
            ptr += (len/2 + 1)*complex_elem_size;
            src_dft_buf = dst_dft_buf = ptr;
            ptr += len*elem_size;
            if( !inplace_transform )
            {
                dst_dft_buf = ptr;
                ptr += len*elem_size;
            }
            icvDCTInit( len, complex_elem_size, dct_wave, inv );
            if( !inv )
                scale += scale;
            prev_len = len;
        }
        // otherwise reuse the tables calculated on the previous stage

        for( i = 0; i < count; i++ )
        {
            dct_func( sptr + i*sstep0, sstep1, src_dft_buf, dst_dft_buf,
                      dptr + i*dstep0, dstep1, len, nf, factors,
                      itab, dft_wave, dct_wave, spec, ptr );
        }
        src = dst;
    }

    __END__;

    if( spec_dft )
    {
        if( depth == CV_32F )
            icvDFTFree_R_32f_p( spec_dft );
        else
            icvDFTFree_R_64f_p( spec_dft );
    }

    if( buffer && !local_alloc )
        cvFree( (void**)&buffer );
}

/* End of file. */
