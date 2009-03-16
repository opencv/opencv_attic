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

/****************************************************************************************\
                                    Base Image Filter
\****************************************************************************************/

namespace cv
{

BaseRowFilter::BaseRowFilter() { ksize = anchor = -1; }
BaseRowFilter::~BaseRowFilter() {}

BaseColumnFilter::BaseColumnFilter() { ksize = anchor = -1; }
BaseColumnFilter::~BaseColumnFilter() {}
void BaseColumnFilter::reset() {}

BaseFilter::BaseFilter() { ksize = Size(-1,-1); anchor = Point(-1,-1); }
BaseFilter::~BaseFilter() {}
void BaseFilter::reset() {}

/*
 Various border types, image boundaries are denoted with '|'
 
    * BORDER_REPLICATE:     aaaaaa|abcdefgh|hhhhhhh
    * BORDER_REFLECT:       fedcba|abcdefgh|hgfedcb
    * BORDER_REFLECT_101:   gfedcb|abcdefgh|gfedcba
    * BORDER_WRAP:          cdefgh|abcdefgh|abcdefg        
    * BORDER_CONSTANT:      iiiiii|abcdefgh|iiiiiii  with some specified 'i'
*/
static int calcBorderPos( int p, int len, int borderType )
{
    if( (unsigned)p < (unsigned)len )
        ;
    else if( borderType == BORDER_REPLICATE )
        p = p < 0 ? 0 : len - 1;
    else if( borderType == BORDER_REFLECT || borderType == BORDER_REFLECT_101 )
    {
        int delta = borderType == BORDER_REFLECT;
        do
        {
            if( p < 0 )
                p = -p - 1 + delta;
            else
                p = len - 1 - (p - len) - delta;
        }
        while( (unsigned)p >= (unsigned)len );
    }
    else if( borderType == BORDER_WRAP )
    {
        if( p < 0 )
            p -= ((p-len+1)/len)*len;
        if( p >= len )
            p %= len;
    }
    else if( borderType == BORDER_CONSTANT )
        p = -1;
    else
        CV_Error( CV_StsBadArg, "Unknown/unsupported border type" );
    return p;
}


FilterEngine::FilterEngine()
{
    srcType = dstType = bufType = -1;
    rowBorderType = columnBorderType = BORDER_REPLICATE;
    bufStep = startY = startY0 = endY = rowCount = dstY = 0;
    maxWidth = 0;

    wholeSize = Size(-1,-1);
}
    

FilterEngine::FilterEngine( const Ptr<BaseFilter>& _filter2D,
                            const Ptr<BaseRowFilter>& _rowFilter,
                            const Ptr<BaseColumnFilter>& _columnFilter,
                            int _srcType, int _dstType, int _bufType,
                            int _rowBorderType, int _columnBorderType,
                            const Scalar& _borderValue )
{
    init(_filter2D, _rowFilter, _columnFilter, _srcType, _dstType, _bufType,
         _rowBorderType, _columnBorderType, _borderValue);
}
    
FilterEngine::~FilterEngine()
{
}


void FilterEngine::init( const Ptr<BaseFilter>& _filter2D,
                         const Ptr<BaseRowFilter>& _rowFilter,
                         const Ptr<BaseColumnFilter>& _columnFilter,
                         int _srcType, int _dstType, int _bufType,
                         int _rowBorderType, int _columnBorderType,
                         const Scalar& _borderValue )
{
    _srcType = CV_MAT_TYPE(_srcType);
    _bufType = CV_MAT_TYPE(_bufType);
    _dstType = CV_MAT_TYPE(_dstType);
        
    srcType = _srcType;
    int srcElemSize = getElemSize(srcType);
    dstType = _dstType;
    bufType = _bufType;
    
    filter2D = _filter2D;
    rowFilter = _rowFilter;
    columnFilter = _columnFilter;

    if( _columnBorderType < 0 )
        _columnBorderType = _rowBorderType;
    
    rowBorderType = _rowBorderType;
    columnBorderType = _columnBorderType;
    
    CV_Assert( columnBorderType != BORDER_WRAP );
    
    if( isSeparable() )
    {
        CV_Assert( rowFilter.obj && columnFilter.obj );
        ksize = Size(rowFilter->ksize, columnFilter->ksize);
        anchor = Point(rowFilter->anchor, columnFilter->anchor);
    }
    else
    {
        CV_Assert( bufType == srcType );
        ksize = filter2D->ksize;
        anchor = filter2D->anchor;
    }

    CV_Assert( 0 <= anchor.x && anchor.x < ksize.width &&
               0 <= anchor.y && anchor.y < ksize.height );

    borderElemSize = srcElemSize/(CV_MAT_DEPTH(srcType) >= CV_32S ? sizeof(int) : 1);
    borderTab.resize( std::max(ksize.width - 1, 1)*borderElemSize);
    
    maxWidth = bufStep = 0;
    constBorderRow.clear();

    if( rowBorderType == BORDER_CONSTANT || columnBorderType == BORDER_CONSTANT )
    {
        constBorderValue.resize(srcElemSize*(ksize.width - 1));
        scalarToRawData(_borderValue, &constBorderValue[0], srcType,
                        (ksize.width-1)*CV_MAT_CN(srcType));
    }

    wholeSize = Size(-1,-1);
}

    
int FilterEngine::start(Size _wholeSize, Rect _roi, int _maxBufRows)
{
    int i, j;
    
    wholeSize = _wholeSize;
    roi = _roi;
    CV_Assert( roi.x >= 0 && roi.y >= 0 && roi.width >= 0 && roi.height >= 0 &&
        roi.x + roi.width <= wholeSize.width &&
        roi.y + roi.height <= wholeSize.height );

    int esz = getElemSize(srcType);
    int bufElemSize = getElemSize(bufType);
    const uchar* constVal = !constBorderValue.empty() ? &constBorderValue[0] : 0;

    if( _maxBufRows < 0 )
        _maxBufRows = ksize.height + 3;
    _maxBufRows = std::max(_maxBufRows, std::max(anchor.y, ksize.height-anchor.y-1)*2+1);

    if( maxWidth < roi.width || _maxBufRows != (int)rows.size() )
    {
        rows.resize(_maxBufRows);
        maxWidth = std::max(maxWidth, roi.width);
        int cn = CV_MAT_CN(srcType);
        srcRow.resize(esz*(maxWidth + ksize.width - 1));
        if( columnBorderType == BORDER_CONSTANT )
        {
            constBorderRow.resize(getElemSize(bufType)*maxWidth);
            uchar* dst;
            int n = (int)constBorderValue.size()*esz, N;
            if( isSeparable() )
            {
                dst = &srcRow[0];
                N = (maxWidth + ksize.width - 1)*esz;
            }
            else
            {
                dst = &constBorderRow[0];
                N = maxWidth*esz;
            }
            
            for( i = 0; i < N; i += n )
            {
                n = std::min( n, N - i );
                memcpy( dst + i, constVal, n );
            }

            if( isSeparable() )
                (*rowFilter)(&srcRow[0] + anchor.x*esz, &constBorderRow[0], maxWidth, cn);
        }
        
        int maxBufStep = bufElemSize*(int)alignSize(maxWidth +
            (!isSeparable() ? ksize.width - 1 : 0),16);
        ringBuf.resize(maxBufStep*rows.size());
    }

    // adjust bufstep so that the used part of the ring buffer stays compact in memory
    bufStep = bufElemSize*(int)alignSize(roi.width + (!isSeparable() ? ksize.width - 1 : 0),16);

    dx1 = std::max(anchor.x - roi.x, 0);
    dx2 = std::max(ksize.width - anchor.x - 1 + roi.x + roi.width - wholeSize.width, 0);

    // recompute border tables
    if( dx1 > 0 || dx2 > 0 )
    {
        if( rowBorderType == BORDER_CONSTANT )
        {
            int nr = isSeparable() ? 1 : (int)rows.size();
            for( i = 0; i < nr; i++ )
            {
                uchar* dst = isSeparable() ? &srcRow[0] : &ringBuf[0] + bufStep*i;
                memcpy( dst, constVal, dx1*esz );
                memcpy( dst + (roi.width + ksize.width - 1 - dx2)*esz, constVal, dx2*esz );
            }
        }
        else
        {
            int btab_esz = borderElemSize, wholeWidth = wholeSize.width;
            int* btab = (int*)&borderTab[0];
            
            for( i = 0; i < dx1; i++ )
            {
                int p0 = calcBorderPos(i-dx1, wholeWidth, rowBorderType)*btab_esz;
                for( j = 0; j < btab_esz; j++ )
                    btab[i*btab_esz + j] = p0 + j;
            }

            for( i = 0; i < dx2; i++ )
            {
                int p0 = calcBorderPos(wholeWidth + i, wholeWidth, rowBorderType)*btab_esz;
                for( j = 0; j < btab_esz; j++ )
                    btab[(i + dx1)*btab_esz + j] = p0 + j;
            }
        }
    }

    rowCount = dstY = 0;
    startY = startY0 = std::max(roi.y - anchor.y, 0);
    endY = std::min(roi.y + roi.height + ksize.height - anchor.y - 1, wholeSize.height);
    /*firstStripeSize = std::min(roi.y + (int)rows.size() - anchor.y, endY) - startY;
    nextStripeSize = rows.size() - ksize.height + 1;*/
    if( columnFilter.obj )
        columnFilter->reset();
    if( filter2D.obj )
        filter2D->reset();

    return startY;
}


int FilterEngine::start(const Mat& src, const Rect& _srcRoi,
                        bool isolated, int maxBufRows)
{
    Rect srcRoi = _srcRoi;
    
    if( srcRoi == Rect(0,0,-1,-1) )
        srcRoi = Rect(0,0,src.cols,src.rows);
    
    CV_Assert( srcRoi.x >= 0 && srcRoi.y >= 0 &&
        srcRoi.width >= 0 && srcRoi.height >= 0 &&
        srcRoi.x + srcRoi.width <= src.cols &&
        srcRoi.y + srcRoi.height <= src.rows );

    Point ofs;
    Size wholeSize(src.cols, src.rows);
    if( !isolated )
        src.locateROI( wholeSize, ofs );
    start( wholeSize, srcRoi + ofs, maxBufRows );

    return startY - ofs.y;
}


int FilterEngine::remainingInputRows() const
{
    return endY - startY - rowCount;
}

int FilterEngine::remainingOutputRows() const
{
    return roi.height - dstY;
}

int FilterEngine::proceed( const uchar* src, int srcstep, int count,
                           uchar* dst, int dststep )
{
    CV_Assert( wholeSize.width > 0 && wholeSize.height > 0 );
    
    const int *btab = &borderTab[0];
    int esz = getElemSize(srcType), btab_esz = borderElemSize;
    uchar** brows = &rows[0];
    int bufRows = (int)rows.size();
    int cn = CV_MAT_CN(bufType);
    int width = roi.width, kwidth = ksize.width;
    int kheight = ksize.height, ay = anchor.y;
    int _dx1 = dx1, _dx2 = dx2;
    int width1 = roi.width + kwidth - 1;
    int xofs1 = std::min(roi.x, anchor.x);
    bool isSep = isSeparable();
    bool makeBorder = (_dx1 > 0 || _dx2 > 0) && rowBorderType != BORDER_CONSTANT;
    int dy = 0, i = 0;

    src -= xofs1*esz;
    count = std::min(count, remainingInputRows());

    CV_Assert( src && dst && count > 0 );

    for(;; dst += dststep*i, dy += i)
    {
        int dcount = bufRows - ay - startY - rowCount + roi.y;
        dcount = dcount > 0 ? dcount : bufRows - kheight + 1;
        dcount = std::min(dcount, count);
        count -= dcount;
        for( ; dcount-- > 0; src += srcstep )
        {
            int bi = (startY - startY0 + rowCount) % bufRows;
            uchar* brow = &ringBuf[0] + bi*bufStep;
            uchar* row = isSep ? &srcRow[0] : brow;
            
            if( ++rowCount > bufRows )
            {
                --rowCount;
                ++startY;
            }

            memcpy( row + _dx1*esz, src, (width1 - _dx2 - _dx1)*esz );

            if( makeBorder )
            {
                if( btab_esz*(int)sizeof(int) == esz )
                {
                    const int* isrc = (const int*)src;
                    int* irow = (int*)row;

                    for( i = 0; i < _dx1*btab_esz; i++ )
                        irow[i] = isrc[btab[i]];
                    for( i = 0; i < _dx2*btab_esz; i++ )
                        irow[i + (width1 - _dx2)*btab_esz] = isrc[btab[i+_dx1*btab_esz]];
                }
                else
                {
                    for( i = 0; i < _dx1*esz; i++ )
                        row[i] = src[btab[i]];
                    for( i = 0; i < _dx2*esz; i++ )
                        row[i + (width1 - _dx2)*esz] = src[btab[i+_dx1*esz]];
                }
            }
            
            if( isSep )
                (*rowFilter)(row, brow, width, CV_MAT_CN(srcType));
        }

        int max_i = std::min(bufRows, roi.height - (dstY + dy) + (kheight - 1));
        for( i = 0; i < max_i; i++ )
        {
            int srcY = calcBorderPos(dstY + dy + i + roi.y - ay,
                            wholeSize.height, columnBorderType);
            if( srcY < 0 ) // can happen only with constant border type
                brows[i] = &constBorderRow[0];
            else
            {
                CV_Assert( srcY >= startY );
                if( srcY >= startY + rowCount )
                    break;
                int bi = (srcY - startY0) % bufRows;
                brows[i] = &ringBuf[0] + bi*bufStep;
            }
        }
        if( i < kheight )
            break;
        i -= kheight - 1;
        if( isSeparable() )
            (*columnFilter)((const uchar**)brows, dst, dststep, i, roi.width*cn);
        else
            (*filter2D)((const uchar**)brows, dst, dststep, i, roi.width, cn);
    }

    dstY += dy;
    CV_Assert( dstY <= roi.height );
    return dy;
}


/*int FilterEngine::get(uchar* dst, int dststep, int maxCount)
{
    CV_Assert( wholeSize.width > 0 && wholeSize.height > 0 );
    
    int bufRows = (int)rows.size();
    if( startY > minY )
    {
        lostRows += startY - minY;
        dstY += startY - minY;
    }

    dstY += dy;
    if( dstY == roi.height )
    {
        // prepare for the next filtering operation
        // with the same parameters
        rowCount = lostRows = dstY = 0;
        startY = startY0;
    }

    return dy;
}*/

void FilterEngine::apply(const Mat& src, Mat& dst,
    const Rect& _srcRoi, Point dstOfs, bool isolated)
{
    CV_Assert( src.type() == srcType && dst.type() == dstType );
    
    Rect srcRoi = _srcRoi;
    if( srcRoi == Rect(0,0,-1,-1) )
        srcRoi = Rect(0,0,src.cols,src.rows);

    CV_Assert( dstOfs.x >= 0 && dstOfs.y >= 0 &&
        dstOfs.x + srcRoi.width <= dst.cols &&
        dstOfs.y + srcRoi.height <= dst.rows );

    int y = start(src, srcRoi, isolated);
    proceed( src.data + y*src.step, src.step, endY - startY, dst.data, dst.step );
}


/****************************************************************************************\
*                                 Separable linear filter                                *
\****************************************************************************************/

int getKernelType(const Mat& _kernel, Point anchor)
{
    CV_Assert( _kernel.channels() == 1 );
    int i, sz = _kernel.rows*_kernel.cols;

    Mat kernel;
    _kernel.convertTo(kernel, CV_64F);

    const double* coeffs = (double*)kernel.data;
    double sum = 0;
    int type = KERNEL_SMOOTH + KERNEL_INTEGER;
    if( (_kernel.rows == 1 || _kernel.cols == 1) &&
        anchor.x*2 + 1 == _kernel.cols &&
        anchor.y*2 + 1 == _kernel.rows )
        type |= (KERNEL_SYMMETRICAL + KERNEL_ASYMMETRICAL);

    for( i = 0; i < sz; i++ )
    {
        double a = coeffs[i], b = coeffs[sz - i - 1];
        if( a != b )
            type &= ~KERNEL_SYMMETRICAL;
        if( a != -b )
            type &= ~KERNEL_ASYMMETRICAL;
        if( a < 0 )
            type &= ~KERNEL_SMOOTH;
        if( a != saturate_cast<int>(a) )
            type &= ~KERNEL_INTEGER;
        sum += a;
    }

    if( fabs(sum - 1) > DBL_EPSILON*(fabs(sum) + 1) )
        type &= ~KERNEL_SMOOTH;
    return type;
}


template<typename ST, typename DT> struct RowFilter : public BaseRowFilter
{
    RowFilter( const Mat& _kernel, int _anchor )
    {
        if( _kernel.isContinuous() )
            kernel = _kernel;
        else
            _kernel.copyTo(kernel);
        anchor = _anchor;
        ksize = kernel.rows + kernel.cols - 1;
        CV_Assert( kernel.type() == DataType<DT>::type &&
                   (kernel.rows == 1 || kernel.cols == 1));
    }
    
    void operator()(const uchar* src, uchar* dst, int width, int cn)
    {
        int _ksize = ksize;
        const DT* kx = (const DT*)kernel.data;
        const ST* S;
        DT* D = (DT*)dst;
        int i = 0, k;
        width *= cn;

        for( ; i <= width - 4; i += 4 )
        {
            S = (const ST*)src + i;
            DT f = kx[0];
            DT s0 = f*S[0], s1 = f*S[1], s2 = f*S[2], s3 = f*S[3];

            for( k = 1; k < _ksize; k++ )
            {
                S += cn;
                f = kx[k];
                s0 += f*S[0]; s1 += f*S[1];
                s2 += f*S[2]; s3 += f*S[3];
            }
            
            D[i] = s0; D[i+1] = s1;
            D[i+2] = s2; D[i+3] = s3;
        }

        for( ; i < width; i++ )
        {
            S = (const ST*)src + i;
            DT s0 = kx[0]*S[0];
            for( k = 1; k < _ksize; k++ )
            {
                S += cn;
                s0 += kx[k]*S[0];
            }
            D[i] = s0;
        }
    }

    Mat kernel;
};


template<typename ST, typename DT> struct SymmRowFilter : public RowFilter<ST, DT>
{
    SymmRowFilter( const Mat& _kernel, int _anchor, int _symmetryType )
        : RowFilter<ST, DT>( _kernel, _anchor )
    {
        symmetryType = _symmetryType;
        CV_Assert( (symmetryType & (KERNEL_SYMMETRICAL | KERNEL_ASYMMETRICAL)) != 0 );
    }
    
    void operator()(const uchar* src, uchar* dst, int width, int cn)
    {
        int ksize2 = this->ksize/2, ksize2n = ksize2*cn;
        const DT* kx = (const DT*)this->kernel.data + ksize2;
        bool symmetrical = (symmetryType & KERNEL_SYMMETRICAL) != 0;
        const ST* S = (const ST*)src + ksize2n;
        DT* D = (DT*)dst;
        int i = 0, j, k;
        width *= cn;

        if( symmetrical )
        {
            for( ; i <= width - 4; i += 4, S += 4 )
            {
                DT f = kx[0];
                DT s0=f*S[0], s1=f*S[1],
                   s2=f*S[2], s3=f*S[3];
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                {
                    f = kx[k];
                    s0 += f*(S[j] + S[-j]);
                    s1 += f*(S[j+1] + S[-j+1]);
                    s2 += f*(S[j+2] + S[-j+2]);
                    s3 += f*(S[j+3] + S[-j+3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }

            for( ; i < width; i++, S++ )
            {
                DT s0 = kx[0]*S[0];
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                    s0 += kx[k]*(S[j] + S[-j]);
                D[i] = s0;
            }
        }
        else
        {
            for( ; i <= width - 4; i += 4, S += 4 )
            {
                DT s0 = 0, s1 = 0, s2 = 0, s3 = 0;
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                {
                    DT f = kx[k];
                    s0 += f*(S[j] - S[-j]);
                    s1 += f*(S[j+1] - S[-j+1]);
                    s2 += f*(S[j+2] - S[-j+2]);
                    s3 += f*(S[j+3] - S[-j+3]);
                }

                D[i] = s0; D[i+1] = s1;
                D[i+2] = s2; D[i+3] = s3;
            }

            for( ; i < width; i++, S++ )
            {
                DT s0 = 0;
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                    s0 += kx[k]*(S[j] - S[-j]);
                D[i] = s0;
            }
        }
    }

    int symmetryType;
};


template<typename ST, typename DT> struct SymmRowSmallFilter : public SymmRowFilter<ST, DT>
{
    SymmRowSmallFilter( const Mat& _kernel, int _anchor, int _symmetryType )
        : SymmRowFilter<ST, DT>( _kernel, _anchor, _symmetryType )
    {
        CV_Assert( this->ksize <= 5 );
    }
    
    void operator()(const uchar* src, uchar* dst, int width, int cn)
    {
        int ksize2 = this->ksize/2, ksize2n = ksize2*cn;
        const DT* kx = (const DT*)this->kernel.data + ksize2;
        bool symmetrical = (this->symmetryType & KERNEL_SYMMETRICAL) != 0;
        const ST* S = (const ST*)src + ksize2n;
        DT* D = (DT*)dst;
        int i = 0, j, k;
        width *= cn;

        if( symmetrical )
        {
            if( this->ksize == 1 && kx[0] == 1 )
            {
                for( ; i <= width - 2; i += 2 )
                {
                    DT s0 = S[i], s1 = S[i+1];
                    D[i] = s0; D[i+1] = s1;
                }
                S += i;
            }
            else if( this->ksize == 3 )
            {
                if( kx[0] == 2 && kx[1] == 1 )
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = S[-cn] + S[0]*2 + S[cn], s1 = S[1-cn] + S[1]*2 + S[1+cn];
                        D[i] = s0; D[i+1] = s1;
                    }
                else if( kx[0] == -2 && kx[1] == 1 )
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = S[-cn] - S[0]*2 + S[cn], s1 = S[1-cn] - S[1]*2 + S[1+cn];
                        D[i] = s0; D[i+1] = s1;
                    }
                else
                {
                    DT k0 = kx[0], k1 = kx[1];
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = S[0]*k0 + (S[-cn] + S[cn])*k1, s1 = S[1]*k0 + (S[1-cn] + S[1+cn])*k1;
                        D[i] = s0; D[i+1] = s1;
                    }
                }
            }
            else if( this->ksize == 5 )
            {
                DT k0 = kx[0], k1 = kx[1], k2 = kx[2];
                if( k0 == -2 && k1 == 0 && k2 == 1 )
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = -2*S[0] + S[-cn*2] + S[cn*2];
                        DT s1 = -2*S[1] + S[1-cn*2] + S[1+cn*2];
                        D[i] = s0; D[i+1] = s1;
                    }
                else
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = S[0]*k0 + (S[-cn] + S[cn])*k1 + (S[-cn*2] + S[cn*2])*k2;
                        DT s1 = S[1]*k0 + (S[1-cn] + S[1+cn])*k1 + (S[1-cn*2] + S[1+cn*2])*k2;
                        D[i] = s0; D[i+1] = s1;
                    }
            }

            for( ; i < width; i++, S++ )
            {
                DT s0 = kx[0]*S[0];
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                    s0 += kx[k]*(S[j] + S[-j]);
                D[i] = s0;
            }
        }
        else
        {
            if( this->ksize == 3 )
            {
                if( kx[0] == 0 && kx[1] == 1 )
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = S[cn] - S[-cn], s1 = S[1+cn] - S[1-cn];
                        D[i] = s0; D[i+1] = s1;
                    }
                else
                {
                    DT k1 = kx[1];
                    for( ; i <= width - 2; i += 2, S += 2 )
                    {
                        DT s0 = (S[cn] - S[-cn])*k1, s1 = (S[1+cn] - S[1-cn])*k1;
                        D[i] = s0; D[i+1] = s1;
                    }
                }
            }
            else if( this->ksize == 5 )
            {
                DT k1 = kx[1], k2 = kx[2];
                for( ; i <= width - 2; i += 2, S += 2 )
                {
                    DT s0 = (S[cn] - S[-cn])*k1 + (S[cn*2] - S[-cn*2])*k2;
                    DT s1 = (S[1+cn] - S[1-cn])*k1 + (S[1+cn*2] - S[1-cn*2])*k2;
                    D[i] = s0; D[i+1] = s1;
                }
            }

            for( ; i < width; i++, S++ )
            {
                DT s0 = kx[0]*S[0];
                for( k = 1, j = cn; k <= ksize2; k++, j += cn )
                    s0 += kx[k]*(S[j] - S[-j]);
                D[i] = s0;
            }
        }
    }
};


template<class CastOp> struct ColumnFilter : public BaseColumnFilter
{
    typedef typename CastOp::type1 ST;
    typedef typename CastOp::rtype DT;
    
    ColumnFilter( const Mat& _kernel, int _anchor,
        double _delta, const CastOp& _castOp=CastOp() )
    {
        if( _kernel.isContinuous() )
            kernel = _kernel;
        else
            _kernel.copyTo(kernel);
        anchor = _anchor;
        ksize = kernel.rows + kernel.cols - 1;
        delta = saturate_cast<ST>(_delta);
        castOp0 = _castOp;
        CV_Assert( kernel.type() == DataType<ST>::type &&
                   (kernel.rows == 1 || kernel.cols == 1));
    }

    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width)
    {
        const ST* ky = (const ST*)kernel.data;
        ST _delta = delta;
        int _ksize = ksize;
        int i, k;
        CastOp castOp = castOp0;

        for( ; count--; dst += dststep, src++ )
        {
            DT* D = (DT*)dst;
            for( i = 0; i <= width - 4; i += 4 )
            {
                ST f = ky[0];
                const ST* S = (const ST*)src[0] + i;
                ST s0 = f*S[0] + _delta, s1 = f*S[1] + _delta,
                    s2 = f*S[2] + _delta, s3 = f*S[3] + _delta;

                for( k = 1; k < _ksize; k++ )
                {
                    S = (const ST*)src[k] + i; f = ky[k];
                    s0 += f*S[0]; s1 += f*S[1];
                    s2 += f*S[2]; s3 += f*S[3];
                }

                D[i] = castOp(s0); D[i+1] = castOp(s1);
                D[i+2] = castOp(s2); D[i+3] = castOp(s3);
            }

            for( ; i < width; i++ )
            {
                ST s0 = ky[0]*((const ST*)src[0])[i] + _delta;
                for( k = 1; k < _ksize; k++ )
                    s0 += ky[k]*((const ST*)src[k])[i];
                D[i] = castOp(s0);
            }
        }
    }

    Mat kernel;
    CastOp castOp0;
    ST delta;
};


template<class CastOp> struct SymmColumnFilter : public ColumnFilter<CastOp>
{
    typedef typename CastOp::type1 ST;
    typedef typename CastOp::rtype DT;

    SymmColumnFilter( const Mat& _kernel, int _anchor,
        double _delta, int _symmetryType, const CastOp& _castOp=CastOp() )
        : ColumnFilter<CastOp>( _kernel, _anchor, _delta, _castOp )
    {
        symmetryType = _symmetryType;
        CV_Assert( (symmetryType & (KERNEL_SYMMETRICAL | KERNEL_ASYMMETRICAL)) != 0 );
    }

    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width)
    {
        int ksize2 = this->ksize/2;
        const ST* ky = (const ST*)this->kernel.data + ksize2;
        int i, k;
        bool symmetrical = (symmetryType & KERNEL_SYMMETRICAL) != 0;
        ST _delta = this->delta;
        CastOp castOp = this->castOp0;

        src += ksize2;

        if( symmetrical )
        {
            for( ; count--; dst += dststep, src++ )
            {
                DT* D = (DT*)dst;

                for( i = 0; i <= width - 4; i += 4 )
                {
                    ST f = ky[0];
                    const ST* S = (const ST*)src[0] + i, *S2;
                    ST s0 = f*S[0] + _delta, s1 = f*S[1] + _delta,
                        s2 = f*S[2] + _delta, s3 = f*S[3] + _delta;

                    for( k = 1; k <= ksize2; k++ )
                    {
                        S = (const ST*)src[k] + i;
                        S2 = (const ST*)src[-k] + i;
                        f = ky[k];
                        s0 += f*(S[0] + S2[0]);
                        s1 += f*(S[1] + S2[1]);
                        s2 += f*(S[2] + S2[2]);
                        s3 += f*(S[3] + S2[3]);
                    }

                    D[i] = castOp(s0); D[i+1] = castOp(s1);
                    D[i+2] = castOp(s2); D[i+3] = castOp(s3);
                }

                for( ; i < width; i++ )
                {
                    ST s0 = ky[0]*((const ST*)src[0])[i] + _delta;
                    for( k = 1; k <= ksize2; k++ )
                        s0 += ky[k]*(((const ST*)src[k])[i] + ((const ST*)src[-k])[i]);
                    D[i] = castOp(s0);
                }
            }
        }
        else
        {
            for( ; count--; dst += dststep, src++ )
            {
                DT* D = (DT*)dst;

                for( i = 0; i <= width - 4; i += 4 )
                {
                    ST f = ky[0];
                    const ST *S, *S2;
                    ST s0 = _delta, s1 = _delta, s2 = _delta, s3 = _delta;

                    for( k = 1; k <= ksize2; k++ )
                    {
                        S = (const ST*)src[k] + i;
                        S2 = (const ST*)src[-k] + i;
                        f = ky[k];
                        s0 += f*(S[0] - S2[0]);
                        s1 += f*(S[1] - S2[1]);
                        s2 += f*(S[2] - S2[2]);
                        s3 += f*(S[3] - S2[3]);
                    }

                    D[i] = castOp(s0); D[i+1] = castOp(s1);
                    D[i+2] = castOp(s2); D[i+3] = castOp(s3);
                }

                for( ; i < width; i++ )
                {
                    ST s0 = _delta;
                    for( k = 1; k <= ksize2; k++ )
                        s0 += ky[k]*(((const ST*)src[k])[i] - ((const ST*)src[-k])[i]);
                    D[i] = castOp(s0);
                }
            }
        }
    }

    int symmetryType;
};


template<class CastOp>
struct SymmColumnSmallFilter : public SymmColumnFilter<CastOp>
{
    typedef typename CastOp::type1 ST;
    typedef typename CastOp::rtype DT;
    
    SymmColumnSmallFilter( const Mat& _kernel, int _anchor,
                           double _delta, int _symmetryType,
                           const CastOp& _castOp=CastOp() )
        : SymmColumnFilter<CastOp>( _kernel, _anchor, _delta, _symmetryType, _castOp )
    {
        CV_Assert( this->ksize == 3 );
    }

    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width)
    {
        int ksize2 = this->ksize/2;
        const ST* ky = (const ST*)this->kernel.data + ksize2;
        int i;
        bool symmetrical = (this->symmetryType & KERNEL_SYMMETRICAL) != 0;
        bool is_1_2_1 = ky[0] == 1 && ky[1] == 2;
        bool is_1_m2_1 = ky[0] == 1 && ky[1] == -2;
        bool is_m1_0_1 = ky[1] == 1 || ky[1] == -1;
        ST f0 = ky[0], f1 = ky[1];
        ST _delta = this->delta;
        CastOp castOp = this->castOp0;

        src += ksize2;

        for( ; count--; dst += dststep, src++ )
        {
            DT* D = (DT*)dst;
            i = 0;
            const ST* S0 = (const ST*)src[-1];
            const ST* S1 = (const ST*)src[0];
            const ST* S2 = (const ST*)src[1];

            if( symmetrical )
            {
                if( is_1_2_1 )
                {
                    for( ; i <= width - 4; i += 4 )
                    {
                        ST s0 = S0[i] + S1[i]*2 + S2[i] + _delta;
                        ST s1 = S0[i+1] + S1[i+1]*2 + S2[i+1] + _delta;
                        D[i] = castOp(s0);
                        D[i+1] = castOp(s1);

                        s0 = S0[i+2] + S1[i+2]*2 + S2[i+2] + _delta;
                        s1 = S0[i+3] + S1[i+3]*2 + S2[i+3] + _delta;
                        D[i+2] = castOp(s0);
                        D[i+3] = castOp(s1);
                    }
                }
                else if( is_1_m2_1 )
                {
                    for( ; i <= width - 4; i += 4 )
                    {
                        ST s0 = S0[i] - S1[i]*2 + S2[i] + _delta;
                        ST s1 = S0[i+1] - S1[i+1]*2 + S2[i+1] + _delta;
                        D[i] = castOp(s0);
                        D[i+1] = castOp(s1);

                        s0 = S0[i+2] - S1[i+2]*2 + S2[i+2] + _delta;
                        s1 = S0[i+3] - S1[i+3]*2 + S2[i+3] + _delta;
                        D[i+2] = castOp(s0);
                        D[i+3] = castOp(s1);
                    }
                }
                else
                {
                    for( ; i <= width - 4; i += 4 )
                    {
                        ST s0 = (S0[i] + S2[i])*f1 + S1[i]*f0 + _delta;
                        ST s1 = (S0[i+1] + S2[i+1])*f1 + S1[i+1]*f0 + _delta;
                        D[i] = castOp(s0);
                        D[i+1] = castOp(s1);

                        s0 = (S0[i+2] + S2[i+2])*f1 + S1[i+2]*f0 + _delta;
                        s1 = (S0[i+3] + S2[i+3])*f1 + S1[i+3]*f0 + _delta;
                        D[i+2] = castOp(s0);
                        D[i+3] = castOp(s1);
                    }
                }

                for( ; i < width; i++ )
                    D[i] = castOp((S0[i] + S2[i])*f1 + S1[i]*f0 + _delta);
            }
            else
            {
                if( is_m1_0_1 )
                {
                    if( f1 < 0 )
                        std::swap(S0, S2);

                    for( ; i <= width - 4; i += 4 )
                    {
                        ST s0 = S2[i] - S0[i] + _delta;
                        ST s1 = S2[i+1] - S0[i+1] + _delta;
                        D[i] = castOp(s0);
                        D[i+1] = castOp(s1);

                        s0 = S2[i+2] - S0[i+2] + _delta;
                        s1 = S2[i+3] - S0[i+3] + _delta;
                        D[i+2] = castOp(s0);
                        D[i+3] = castOp(s1);
                    }

                    if( f1 < 0 )
                        std::swap(S0, S2);
                }
                else
                {
                    for( ; i <= width - 4; i += 4 )
                    {
                        ST s0 = (S2[i] - S0[i])*f1 + _delta;
                        ST s1 = (S2[i+1] - S0[i+1])*f1 + _delta;
                        D[i] = castOp(s0);
                        D[i+1] = castOp(s1);

                        s0 = (S2[i+2] - S0[i+2])*f1 + _delta;
                        s1 = (S2[i+3] - S0[i+3])*f1 + _delta;
                        D[i+2] = castOp(s0);
                        D[i+3] = castOp(s1);
                    }
                }

                for( ; i < width; i++ )
                    D[i] = castOp((S2[i] - S0[i])*f1 + _delta);
            }
        }
    }
};

template<typename ST, typename DT> struct Cast
{
    typedef ST type1;
    typedef DT rtype;

    DT operator()(ST val) const { return saturate_cast<DT>(val); }
};

template<typename ST, typename DT, int bits> struct FixedPtCast
{
    typedef ST type1;
    typedef DT rtype;
    enum { SHIFT = bits, DELTA = 1 << (bits-1) };

    DT operator()(ST val) const { return saturate_cast<DT>((val + DELTA)>>SHIFT); }
};

template<typename ST, typename DT> struct FixedPtCastEx
{
    typedef ST type1;
    typedef DT rtype;

    FixedPtCastEx() : SHIFT(0), DELTA(0) {}
    FixedPtCastEx(int bits) : SHIFT(bits), DELTA(bits ? 1 << (bits-1) : 0) {}
    DT operator()(ST val) const { return saturate_cast<DT>((val + DELTA)>>SHIFT); }
    int SHIFT, DELTA;
};

Ptr<BaseRowFilter> getLinearRowFilter( int srcType, int bufType,
                                          const Mat& kernel, int anchor,
                                          int symmetryType )
{
    int sdepth = CV_MAT_DEPTH(srcType), ddepth = CV_MAT_DEPTH(bufType);
    int cn = CV_MAT_CN(srcType);
    CV_Assert( cn == CV_MAT_CN(bufType) &&
        ddepth >= std::max(sdepth, CV_32S) &&
        kernel.type() == ddepth );

    if( !(symmetryType & (KERNEL_SYMMETRICAL|KERNEL_ASYMMETRICAL)) )
    {
        if( sdepth == CV_8U && ddepth == CV_32S )
            return Ptr<BaseRowFilter>(new RowFilter<uchar, int>(kernel, anchor));
        if( sdepth == CV_8U && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new RowFilter<uchar, float>(kernel, anchor));
        if( sdepth == CV_8U && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new RowFilter<uchar, double>(kernel, anchor));
        if( sdepth == CV_16U && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new RowFilter<ushort, float>(kernel, anchor));
        if( sdepth == CV_16U && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new RowFilter<ushort, double>(kernel, anchor));
        if( sdepth == CV_16S && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new RowFilter<short, float>(kernel, anchor));
        if( sdepth == CV_16S && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new RowFilter<short, double>(kernel, anchor));
        if( sdepth == CV_32F && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new RowFilter<float, float>(kernel, anchor));
        if( sdepth == CV_64F && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new RowFilter<double, double>(kernel, anchor));
    }
    else
    {
        int ksize = kernel.rows + kernel.cols - 1;
        if( ksize <= 5 )
        {
            if( sdepth == CV_8U && ddepth == CV_32S )
                return Ptr<BaseRowFilter>(new SymmRowSmallFilter<uchar, int>
                    (kernel, anchor, symmetryType));
            if( sdepth == CV_32F && ddepth == CV_32F )
                return Ptr<BaseRowFilter>(new SymmRowSmallFilter<float, float>
                    (kernel, anchor, symmetryType));
        }

        if( sdepth == CV_8U && ddepth == CV_32S )
            return Ptr<BaseRowFilter>(new SymmRowFilter<uchar, int>(kernel, anchor, symmetryType));
        if( sdepth == CV_8U && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<uchar, float>(kernel, anchor, symmetryType));
        if( sdepth == CV_8U && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<uchar, double>(kernel, anchor, symmetryType));
        if( sdepth == CV_16U && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<ushort, float>(kernel, anchor, symmetryType));
        if( sdepth == CV_16U && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<ushort, double>(kernel, anchor, symmetryType));
        if( sdepth == CV_16S && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<short, float>(kernel, anchor, symmetryType));
        if( sdepth == CV_16S && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<short, double>(kernel, anchor, symmetryType));
        if( sdepth == CV_32F && ddepth == CV_32F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<float, float>(kernel, anchor, symmetryType));
        if( sdepth == CV_64F && ddepth == CV_64F )
            return Ptr<BaseRowFilter>(new SymmRowFilter<double, double>(kernel, anchor, symmetryType));
    }

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of source format (=%d), and buffer format (=%d)",
        srcType, bufType));

    return Ptr<BaseRowFilter>(0);
}


Ptr<BaseColumnFilter> getLinearColumnFilter( int bufType, int dstType,
                                             const Mat& kernel, int anchor,
                                             int symmetryType, double delta, 
                                             int bits )
{
    int sdepth = CV_MAT_DEPTH(bufType), ddepth = CV_MAT_DEPTH(dstType);
    int cn = CV_MAT_CN(dstType);
    CV_Assert( cn == CV_MAT_CN(bufType) &&
        sdepth >= std::max(ddepth, CV_32S) &&
        kernel.type() == sdepth );

    if( !(symmetryType & (KERNEL_SYMMETRICAL|KERNEL_ASYMMETRICAL)) )
    {
        if( ddepth == CV_8U && sdepth == CV_32S )
            return Ptr<BaseColumnFilter>(new ColumnFilter<FixedPtCastEx<int, uchar> >
            (kernel, anchor, delta, FixedPtCastEx<int, uchar>(bits)));
        if( ddepth == CV_8U && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<float, uchar> >(kernel, anchor, delta));
        if( ddepth == CV_8U && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<double, uchar> >(kernel, anchor, delta));
        if( ddepth == CV_16U && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<float, ushort> >(kernel, anchor, delta));
        if( ddepth == CV_16U && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<double, ushort> >(kernel, anchor, delta));
        if( ddepth == CV_16S && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<float, short> >(kernel, anchor, delta));
        if( ddepth == CV_16S && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<double, short> >(kernel, anchor, delta));
        if( ddepth == CV_32F && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<float, float> >(kernel, anchor, delta));
        if( ddepth == CV_64F && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new ColumnFilter<Cast<double, double> >(kernel, anchor, delta));
    }
    else
    {
        int ksize = kernel.rows + kernel.cols - 1;
        if( ksize == 3 )
        {
            if( ddepth == CV_8U && sdepth == CV_32S )
                return Ptr<BaseColumnFilter>(new SymmColumnSmallFilter<FixedPtCastEx<int, uchar> >
                    (kernel, anchor, delta, symmetryType, FixedPtCastEx<int, uchar>(bits)));
            if( ddepth == CV_16S && sdepth == CV_32S && bits == 0 )
                return Ptr<BaseColumnFilter>(new SymmColumnSmallFilter<Cast<int, short> >
                    (kernel, anchor, delta, symmetryType));
            if( ddepth == CV_32F && sdepth == CV_32F )
                return Ptr<BaseColumnFilter>(new SymmColumnSmallFilter<Cast<float, float> >
                    (kernel, anchor, delta, symmetryType));
        }
        if( ddepth == CV_8U && sdepth == CV_32S )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<FixedPtCastEx<int, uchar> >
                (kernel, anchor, delta, symmetryType, FixedPtCastEx<int, uchar>(bits)));
        if( ddepth == CV_8U && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<float, uchar> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_8U && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<double, uchar> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_16U && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<float, ushort> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_16U && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<double, ushort> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_16S && sdepth == CV_32S )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<int, short> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_16S && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<float, short> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_16S && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<double, short> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_32F && sdepth == CV_32F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<float, float> >
                (kernel, anchor, delta, symmetryType));
        if( ddepth == CV_64F && sdepth == CV_64F )
            return Ptr<BaseColumnFilter>(new SymmColumnFilter<Cast<double, double> >
                (kernel, anchor, delta, symmetryType));
    }

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of buffer format (=%d), and destination format (=%d)",
        bufType, dstType));

    return Ptr<BaseColumnFilter>(0);
}


Ptr<FilterEngine> createSeparableLinearFilter(
    int _srcType, int _dstType,
    const Mat& _rowKernel, const Mat& _columnKernel,
    Point _anchor, double _delta,
    int _rowBorderType, int _columnBorderType,
    const Scalar& _borderValue )
{
    _srcType = CV_MAT_TYPE(_srcType);
    _dstType = CV_MAT_TYPE(_dstType);
    int sdepth = CV_MAT_DEPTH(_srcType), ddepth = CV_MAT_DEPTH(_dstType);
    int cn = CV_MAT_CN(_srcType);
    CV_Assert( cn == CV_MAT_CN(_dstType) );
    int rsize = _rowKernel.rows + _rowKernel.cols - 1;
    int csize = _columnKernel.rows + _columnKernel.cols - 1;
    if( _anchor.x < 0 )
        _anchor.x = rsize/2;
    if( _anchor.y < 0 )
        _anchor.y = csize/2;
    int rtype = getKernelType(_rowKernel,
        _rowKernel.rows == 1 ? Point(_anchor.x, 0) : Point(0, _anchor.x));
    int ctype = getKernelType(_columnKernel,
        _columnKernel.rows == 1 ? Point(_anchor.y, 0) : Point(0, _anchor.y));
    Mat rowKernel, columnKernel;

    int bdepth = std::max(CV_32F,std::max(sdepth, ddepth));
    int bits = 0;

    if( sdepth == CV_8U &&
        ((rtype == KERNEL_SMOOTH+KERNEL_SYMMETRICAL &&
          ctype == KERNEL_SMOOTH+KERNEL_SYMMETRICAL &&
          ddepth == CV_8U) ||
         ((rtype & (KERNEL_SYMMETRICAL+KERNEL_ASYMMETRICAL)) &&
          (ctype & (KERNEL_SYMMETRICAL+KERNEL_ASYMMETRICAL)) &&
          (rtype & ctype & KERNEL_INTEGER) &&
          ddepth == CV_16S)) )
    {
        bdepth = CV_32S;
        bits = ddepth == CV_8U ? 8 : 0;
        _rowKernel.convertTo( rowKernel, CV_32S, 1 << bits );
        _columnKernel.convertTo( columnKernel, CV_32S, 1 << bits );
        bits *= 2;
        _delta *= (1 << bits);
    }
    else
    {
        if( _rowKernel.type() != bdepth )
            _rowKernel.convertTo( rowKernel, bdepth );
        else
            rowKernel = _rowKernel;
        if( _columnKernel.type() != bdepth )
            _columnKernel.convertTo( columnKernel, bdepth );
        else
            columnKernel = _columnKernel;
    }

    int _bufType = CV_MAKETYPE(bdepth, cn);
    Ptr<BaseRowFilter> _rowFilter = getLinearRowFilter(
        _srcType, _bufType, rowKernel, _anchor.x, rtype);
    Ptr<BaseColumnFilter> _columnFilter = getLinearColumnFilter(
        _bufType, _dstType, columnKernel, _anchor.y, ctype, _delta, bits );

    return Ptr<FilterEngine>( new FilterEngine(Ptr<BaseFilter>(0), _rowFilter, _columnFilter,
        _srcType, _dstType, _bufType, _rowBorderType, _columnBorderType, _borderValue ));
}


/****************************************************************************************\
*                               Non-separable linear filter                              *
\****************************************************************************************/

void preprocess2DKernel( const Mat& kernel, Vector<Point>& coords, Vector<uchar>& coeffs )
{
    int i, j, k, nz = countNonZero(kernel), ktype = kernel.type();
    if(nz == 0)
        nz = 1;
    CV_Assert( ktype == CV_8U || ktype == CV_32S || ktype == CV_32F || ktype == CV_64F );
    coords.resize(nz);
    coeffs.resize(nz*getElemSize(ktype));
    uchar* _coeffs = &coeffs[0];

    for( i = k = 0; i < kernel.rows; i++ )
    {
        const uchar* krow = kernel.data + kernel.step*i;
        for( j = 0; j < kernel.cols; j++ )
        {
            if( ktype == CV_8U )
            {
                uchar val = krow[j];
                if( val == 0 )
                    continue;
                coords[k] = Point(j,i);
                _coeffs[k++] = val;
            }
            else if( ktype == CV_32S )
            {
                int val = ((const int*)krow)[j];
                if( val == 0 )
                    continue;
                coords[k] = Point(j,i);
                ((int*)_coeffs)[k++] = val;
            }
            else if( ktype == CV_32F )
            {
                float val = ((const float*)krow)[j];
                if( val == 0 )
                    continue;
                coords[k] = Point(j,i);
                ((float*)_coeffs)[k++] = val;
            }
            else
            {
                double val = ((const double*)krow)[j];
                if( val == 0 )
                    continue;
                coords[k] = Point(j,i);
                ((double*)_coeffs)[k++] = val;
            }
        }
    }
}


template<typename ST, class CastOp> struct Filter2D : public BaseFilter
{
    typedef typename CastOp::type1 KT;
    typedef typename CastOp::rtype DT;
    
    Filter2D( const Mat& _kernel, Point _anchor,
        double _delta, const CastOp& _castOp=CastOp() )
    {
        anchor = _anchor;
        ksize = _kernel.size();
        delta = saturate_cast<KT>(_delta);
        castOp0 = _castOp;
        CV_Assert( _kernel.type() == DataType<KT>::type );
        preprocess2DKernel( _kernel, coords, coeffs );
        ptrs.resize( coords.size() );
    }

    void operator()(const uchar** src, uchar* dst, int dststep, int count, int width, int cn)
    {
        KT _delta = delta;
        const Point* pt = &coords[0];
        const KT* kf = (const KT*)&coeffs[0];
        const ST** kp = (const ST**)&ptrs[0];
        int i, k, nz = (int)coords.size();
        CastOp castOp = castOp0;

        width *= cn;
        for( ; count > 0; count--, dst += dststep, src++ )
        {
            DT* D = (DT*)dst;

            for( k = 0; k < nz; k++ )
                kp[k] = (const ST*)src[pt[k].y] + pt[k].x*cn;

            i = 0;
            for( ; i <= width - 4; i += 4 )
            {
                KT s0 = _delta, s1 = _delta, s2 = _delta, s3 = _delta;

                for( k = 0; k < nz; k++ )
                {
                    const ST* sptr = kp[k] + i;
                    KT f = kf[k];
                    s0 += f*sptr[0];
                    s1 += f*sptr[1];
                    s2 += f*sptr[2];
                    s3 += f*sptr[3];
                }

                D[i] = castOp(s0); D[i+1] = castOp(s1);
                D[i+2] = castOp(s2); D[i+3] = castOp(s3);
            }

            for( ; i < width; i++ )
            {
                KT s0 = _delta;
                for( k = 0; k < nz; k++ )
                    s0 += kf[k]*kp[k][i];
                D[i] = castOp(s0);
            }
        }
    }

    Vector<Point> coords;
    Vector<uchar> coeffs;
    Vector<uchar*> ptrs;
    KT delta;
    CastOp castOp0;
};


Ptr<BaseFilter> getLinearFilter(int srcType, int dstType,
                                const Mat& _kernel, Point anchor,
                                double delta, int bits)
{
    int sdepth = CV_MAT_DEPTH(srcType), ddepth = CV_MAT_DEPTH(dstType);
    int cn = CV_MAT_CN(srcType), kdepth = _kernel.depth();
    CV_Assert( cn == CV_MAT_CN(dstType) && ddepth >= sdepth );

    anchor = normalizeAnchor(anchor, _kernel.size());

    if( sdepth == CV_8U && ddepth == CV_8U && kdepth == CV_32S )
        return Ptr<BaseFilter>(new Filter2D<uchar, FixedPtCastEx<int, uchar> >
            (_kernel, anchor, delta, FixedPtCastEx<int, uchar>(bits)));
    if( sdepth == CV_8U && ddepth == CV_16S && kdepth == CV_32S )
        return Ptr<BaseFilter>(new Filter2D<uchar, FixedPtCastEx<int, short> >
            (_kernel, anchor, delta, FixedPtCastEx<int, short>(bits)));

    kdepth = sdepth == CV_64F || ddepth == CV_64F ? CV_64F : CV_32F;
    Mat kernel;
    if( _kernel.type() == kdepth )
        kernel = _kernel;
    else
        _kernel.convertTo(kernel, kdepth, _kernel.type() == CV_32S ? 1./(1 << bits) : 1.);
    
    if( sdepth == CV_8U && ddepth == CV_8U )
        return Ptr<BaseFilter>(new Filter2D<uchar, Cast<float, uchar> >(kernel, anchor, delta));
    if( sdepth == CV_8U && ddepth == CV_16U )
        return Ptr<BaseFilter>(new Filter2D<uchar, Cast<float, ushort> >(kernel, anchor, delta));
    if( sdepth == CV_8U && ddepth == CV_16S )
        return Ptr<BaseFilter>(new Filter2D<uchar, Cast<float, short> >(kernel, anchor, delta));
    if( sdepth == CV_8U && ddepth == CV_32F )
        return Ptr<BaseFilter>(new Filter2D<uchar, Cast<float, float> >(kernel, anchor, delta));
    if( sdepth == CV_8U && ddepth == CV_64F )
        return Ptr<BaseFilter>(new Filter2D<uchar, Cast<double, double> >(kernel, anchor, delta));

    if( sdepth == CV_16U && ddepth == CV_16U )
        return Ptr<BaseFilter>(new Filter2D<ushort, Cast<float, ushort> >(kernel, anchor, delta));
    if( sdepth == CV_16U && ddepth == CV_32F )
        return Ptr<BaseFilter>(new Filter2D<ushort, Cast<float, float> >(kernel, anchor, delta));
    if( sdepth == CV_16U && ddepth == CV_64F )
        return Ptr<BaseFilter>(new Filter2D<ushort, Cast<double, double> >(kernel, anchor, delta));

    if( sdepth == CV_16S && ddepth == CV_16S )
        return Ptr<BaseFilter>(new Filter2D<short, Cast<float, short> >(kernel, anchor, delta));
    if( sdepth == CV_16S && ddepth == CV_32F )
        return Ptr<BaseFilter>(new Filter2D<short, Cast<float, float> >(kernel, anchor, delta));
    if( sdepth == CV_16S && ddepth == CV_64F )
        return Ptr<BaseFilter>(new Filter2D<short, Cast<double, double> >(kernel, anchor, delta));

    if( sdepth == CV_32F && ddepth == CV_32F )
        return Ptr<BaseFilter>(new Filter2D<float, Cast<float, float> >(kernel, anchor, delta));
    if( sdepth == CV_64F && ddepth == CV_64F )
        return Ptr<BaseFilter>(new Filter2D<double, Cast<double, double> >(kernel, anchor, delta));

    CV_Error_( CV_StsNotImplemented,
        ("Unsupported combination of source format (=%d), and destination format (=%d)",
        srcType, dstType));

    return Ptr<BaseFilter>(0);
}


Ptr<FilterEngine> createLinearFilter( int _srcType, int _dstType, const Mat& _kernel,
                         Point _anchor, double _delta,
                         int _rowBorderType, int _columnBorderType,
                         const Scalar& _borderValue )
{
    _srcType = CV_MAT_TYPE(_srcType);
    _dstType = CV_MAT_TYPE(_dstType);
    int sdepth = CV_MAT_DEPTH(_srcType), ddepth = CV_MAT_DEPTH(_dstType);
    int cn = CV_MAT_CN(_srcType);
    CV_Assert( cn == CV_MAT_CN(_dstType) );

    Mat kernel = _kernel;
    int ktype = _kernel.depth() == CV_32S ? KERNEL_INTEGER : getKernelType(_kernel, _anchor);
    int bits = 0;

    if( sdepth == CV_8U && (ddepth == CV_8U || ddepth == CV_16S) &&
        _kernel.rows*_kernel.cols <= (1 << 10) )
    {
        bits = (ktype & KERNEL_INTEGER) ? 0 : 11;
        _kernel.convertTo(kernel, CV_32S, 1 << bits);
    }
    
    Ptr<BaseFilter> _filter2D = getLinearFilter(_srcType, _dstType,
        kernel, _anchor, _delta, bits);

    return Ptr<FilterEngine>(new FilterEngine(_filter2D, Ptr<BaseRowFilter>(0),
        Ptr<BaseColumnFilter>(0), _srcType, _dstType, _srcType,
        _rowBorderType, _columnBorderType, _borderValue ));
}


void filter2D( const Mat& src, Mat& dst, int ddepth,
               const Mat& kernel, Point anchor,
               double delta, int borderType )
{
    const int dft_filter_size = 100;

    if( ddepth < 0 )
        ddepth = src.depth();

    dst.create( src.size(), CV_MAKETYPE(ddepth, src.channels()) );
    anchor = normalizeAnchor(anchor, kernel.size());

    if( kernel.cols*kernel.rows >= dft_filter_size &&
        kernel.cols <= src.cols && kernel.rows <= src.rows )
    {
        Mat temp;
        if( src.data != dst.data )
            temp = src;
        else
            src.copyTo(temp);
        crossCorr( temp, kernel, dst, anchor, delta, borderType );
        return;
    }

    Ptr<FilterEngine> f = createLinearFilter(src.type(), dst.type(), kernel,
                                             anchor, delta, borderType );
    f->apply(src, dst);
}


void sepFilter2D( const Mat& src, Mat& dst, int ddepth,
                  const Mat& kernelX, const Mat& kernelY, Point anchor,
                  double delta, int borderType )
{
    if( ddepth < 0 )
        ddepth = src.depth();

    dst.create( src.size(), CV_MAKETYPE(ddepth, src.channels()) );

    Ptr<FilterEngine> f = createSeparableLinearFilter(src.type(),
        dst.type(), kernelX, kernelY, anchor, delta, borderType );
    f->apply(src, dst);
}

}


CV_IMPL void
cvFilter2D( const CvArr* srcarr, CvArr* dstarr, const CvMat* _kernel, CvPoint anchor )
{
    cv::Mat src = cv::cvarrToMat(srcarr), dst = cv::cvarrToMat(dstarr);
    cv::Mat kernel = cv::cvarrToMat(_kernel);

    CV_Assert( src.size() == dst.size() && src.channels() == dst.channels() );

    cv::filter2D( src, dst, dst.depth(), kernel, anchor, 0, cv::BORDER_REPLICATE );
}

/* End of file. */
