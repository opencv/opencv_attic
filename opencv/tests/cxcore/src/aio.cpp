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

#include "cxcoretest.h"
#include <stdio.h>

using namespace cv;

class CV_IOTest : public CvTest
{
public:
    CV_IOTest();
protected:
    void run(int);
};


CV_IOTest::CV_IOTest():
CvTest( "io", "cvOpenFileStorage, cvReleaseFileStorage, cvRead*, cvWrite*, cvStartWriteStruct, cvEndWriteStruct" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}


static SparseMat cvTsGetRandomSparseMat(int dims, const int* sz, int type,
                                        int nzcount, double a, double b, CvRNG* rng)
{
    SparseMat m(dims, sz, type);
    int i, j;
    CV_Assert(CV_MAT_CN(type) == 1);
    for( i = 0; i < nzcount; i++ )
    {
        int idx[CV_MAX_DIM];
        for( j = 0; j < dims; j++ )
            idx[j] = cvTsRandInt(rng) % sz[j];
        double val = cvTsRandReal(rng)*(b - a) + a;
        uchar* ptr = m.ptr(idx, true, 0);
        if( type == CV_8U )
            *(uchar*)ptr = saturate_cast<uchar>(val);
        else if( type == CV_8S )
            *(schar*)ptr = saturate_cast<schar>(val);
        else if( type == CV_16U )
            *(ushort*)ptr = saturate_cast<ushort>(val);
        else if( type == CV_16S )
            *(short*)ptr = saturate_cast<short>(val);
        else if( type == CV_32S )
            *(int*)ptr = saturate_cast<int>(val);
        else if( type == CV_32F )
            *(float*)ptr = saturate_cast<float>(val);
        else
            *(double*)ptr = saturate_cast<double>(val);
    }
    
    return m;
}

static bool cvTsCheckSparse(const CvSparseMat* m1, const CvSparseMat* m2, double eps)
{
    CvSparseMatIterator it1;
    CvSparseNode* node1;
    int depth = CV_MAT_DEPTH(m1->type);
    
    if( m1->heap->active_count != m2->heap->active_count ||
        m1->dims != m2->dims || CV_MAT_TYPE(m1->type) != CV_MAT_TYPE(m2->type) )
        return false;
    
    for( node1 = cvInitSparseMatIterator( m1, &it1 );
         node1 != 0; node1 = cvGetNextSparseNode( &it1 ))
    {
        uchar* v1 = (uchar*)CV_NODE_VAL(m1,node1);
        uchar* v2 = cvPtrND( m2, CV_NODE_IDX(m1,node1), 0, 0, &node1->hashval );
        if( !v2 )
            return false;
        if( depth == CV_8U || depth == CV_8S )
        {
            if( *v1 != *v2 )
                return false;
        }
        else if( depth == CV_16U || depth == CV_16S )
        {
            if( *(ushort*)v1 != *(ushort*)v2 )
                return false;
        }
        else if( depth == CV_32S )
        {
            if( *(int*)v1 != *(int*)v2 )
                return false;
        }
        else if( depth == CV_32F )
        {
            if( fabs(*(float*)v1 - *(float*)v2) > eps*(fabs(*(float*)v2) + 1) )
                return false;
        }
        else if( fabs(*(double*)v1 - *(double*)v2) > eps*(fabs(*(double*)v2) + 1) )
            return false;
    }
    
    return true;
}

void CV_IOTest::run( int start_from )
{
    double ranges[][2] = {{0, 256}, {-128, 128}, {0, 65536}, {-32768, 32768},
        {-1000000, 1000000}, {-10, 10}, {-10, 10}};
    CvRNG* rng = ts->get_rng();
    RNG rng0;
    test_case_count = 2;
    int progress = 0;
    MemStorage storage(cvCreateMemStorage(0));
    
    for( int idx = 0; idx < test_case_count; idx++ )
    {
        ts->update_context( this, idx, false );
        progress = update_progress( progress, idx, test_case_count, 0 );
        
        cvClearMemStorage(storage);
        
        char buf[L_tmpnam+16];
        char* filename = tmpnam(buf);
        strcat(filename, idx % 2 ? ".yml" : ".xml");
        
        FileStorage fs(filename, FileStorage::WRITE);
        
        int test_int = (int)cvTsRandInt(rng);
        double test_real = (cvTsRandInt(rng)%2?1:-1)*exp(cvTsRandReal(rng)*18-9);
        string test_string = "vw wv23424rtrt@#$@$%$%&%IJUKYILFD@#$@%$&*&() ";
        
        int depth = cvTsRandInt(rng) % (CV_64F+1);
        int cn = cvTsRandInt(rng) % 4 + 1;
        Mat test_mat(cvTsRandInt(rng)%30+1, cvTsRandInt(rng)%30+1, CV_MAKETYPE(depth, cn));
        
        rng0.fill(test_mat, CV_RAND_UNI, Scalar::all(ranges[depth][0]), Scalar::all(ranges[depth][1]));
        if( depth >= CV_32F )
        {
            exp(test_mat, test_mat);
            Mat test_mat_scale(test_mat.size(), test_mat.type());
            rng0.fill(test_mat_scale, CV_RAND_UNI, Scalar::all(-1), Scalar::all(1));
            multiply(test_mat, test_mat_scale, test_mat);
        }
        
        CvSeq* seq = cvCreateSeq(test_mat.type(), sizeof(CvSeq),
            test_mat.elemSize(), storage);
        cvSeqPushMulti(seq, test_mat.data, test_mat.cols*test_mat.rows); 
        
        depth = cvTsRandInt(rng) % (CV_64F+1);
        cn = cvTsRandInt(rng) % 4 + 1;
        int sz[] = {cvTsRandInt(rng)%10+1, cvTsRandInt(rng)%10+1, cvTsRandInt(rng)%10+1};
        MatND test_mat_nd(3, sz, CV_MAKETYPE(depth, cn));
        Mat plain = test_mat_nd;
        
        rng0.fill(plain, CV_RAND_UNI, Scalar::all(ranges[depth][0]), Scalar::all(ranges[depth][1]));
        if( depth >= CV_32F )
        {
            exp(test_mat_nd, test_mat_nd);
            MatND test_mat_scale(test_mat_nd.dims, test_mat_nd.size, test_mat_nd.type());
            plain = test_mat_scale;
            rng0.fill(plain, CV_RAND_UNI, Scalar::all(-1), Scalar::all(1));
            multiply(test_mat_nd, test_mat_scale, test_mat_nd);
        }
        
        int ssz[] = {cvTsRandInt(rng)%10+1, cvTsRandInt(rng)%10+1,
            cvTsRandInt(rng)%10+1,cvTsRandInt(rng)%10+1};
        SparseMat test_sparse_mat = cvTsGetRandomSparseMat(4, ssz, cvTsRandInt(rng)%(CV_64F+1),
            cvTsRandInt(rng) % 10000, 0, 100, rng);
            
        fs << "test_int" << test_int << "test_real" << test_real << "test_string" << test_string;
        fs << "test_mat" << test_mat;
        fs << "test_mat_nd" << test_mat_nd;
        fs << "test_sparse_mat" << test_sparse_mat;
        
        fs << "test_list" << "[" << 0.0000000000001 << 2 << CV_PI << -3435345 << "2-502 2-029 3egegeg" <<
            "{:" << "month" << 12 << "day" << 31 << "year" << 1969 << "}" << "]";
        fs << "test_map" << "{" << "x" << 1 << "y" << 2 << "width" << 100 << "height" << 200 <<
            "lbp" << "[:" << 0 << 1 << 1 << 0 << 1 << 1 << 0 << 1 << "]" << "}";
        fs.writeObj("test_seq", seq);
        
        fs.release();
        
        if(!fs.open(filename, FileStorage::READ))
        {
            ts->printf( CvTS::LOG, "filename %s can not be read\n", filename );
            ts->set_failed_test_info( CvTS::FAIL_MISSING_TEST_DATA );
            return;
        }
        
        int real_int = (int)fs["test_int"];
        double real_real = (double)fs["test_real"];
        string real_string = (string)fs["test_string"];
        
        if( real_int != test_int ||
            fabs(real_real - test_real) > DBL_EPSILON*(fabs(test_real)+1) ||
            real_string != test_string )
        {
            ts->printf( CvTS::LOG, "the read scalars are not correct\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        CvMat* m = (CvMat*)fs["test_mat"].readObj();
        CvMat _test_mat = test_mat;
        double max_diff = 0;
        CvMat stub1, _test_stub1;
        cvReshape(m, &stub1, 1, 0);
        cvReshape(&_test_mat, &_test_stub1, 1, 0);
        CvPoint pt = {0,0};
        
        if( !m || !CV_IS_MAT(m) || m->rows != test_mat.rows || m->cols != test_mat.cols ||
            cvTsCmpEps( &stub1, &_test_stub1, &max_diff, 0, &pt, true) < 0 )
        {
            ts->printf( CvTS::LOG, "the read matrix is not correct: (%.20g vs %.20g) at (%d,%d)\n",
                       cvGetReal2D(&stub1, pt.y, pt.x), cvGetReal2D(&_test_stub1, pt.y, pt.x),
                       pt.y, pt.x );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        if( m && CV_IS_MAT(m))
            cvReleaseMat(&m);

        CvMatND* m_nd = (CvMatND*)fs["test_mat_nd"].readObj();
        CvMatND _test_mat_nd = test_mat_nd;
        
        if( !m_nd || !CV_IS_MATND(m_nd) )
        {
            ts->printf( CvTS::LOG, "the read nd-matrix is not correct\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        CvMat stub, _test_stub;
        cvGetMat(m_nd, &stub, 0, 1);
        cvGetMat(&_test_mat_nd, &_test_stub, 0, 1);
        cvReshape(&stub, &stub1, 1, 0);
        cvReshape(&_test_stub, &_test_stub1, 1, 0);
        
        if( !CV_ARE_TYPES_EQ(&stub, &_test_stub) ||
            !CV_ARE_SIZES_EQ(&stub, &_test_stub) ||
            //cvNorm(&stub, &_test_stub, CV_L2) != 0 ) 
            cvTsCmpEps( &stub1, &_test_stub1, &max_diff, 0, &pt, true) < 0 )
        {
            ts->printf( CvTS::LOG, "the read nd matrix is not correct: (%.20g vs %.20g) at (%d,%d)\n",
                       cvGetReal2D(&stub1, pt.y, pt.x), cvGetReal2D(&_test_stub1, pt.y, pt.x),
                       pt.y, pt.x );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        if( m_nd && CV_IS_MATND(m_nd))
            cvReleaseMatND(&m_nd);
            
        CvSparseMat* m_s = (CvSparseMat*)fs["test_sparse_mat"].readObj();
        CvSparseMat* _test_sparse = (CvSparseMat*)test_sparse_mat;
        
        if( !m_s || !CV_IS_SPARSE_MAT(m_s) || !cvTsCheckSparse(m_s, _test_sparse,
            depth==CV_32F?FLT_EPSILON:DBL_EPSILON))
        {
            ts->printf( CvTS::LOG, "the read sparse matrix is not correct\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        if( m_s && CV_IS_SPARSE_MAT(m_s))
            cvReleaseSparseMat(&m_s);
        cvReleaseSparseMat(&_test_sparse);
        
        FileNode tl = fs["test_list"];
        if( tl.type() != FileNode::SEQ || tl.size() != 6 ||
           fabs((double)tl[0] - 0.0000000000001) >= DBL_EPSILON ||
           (int)tl[1] != 2 ||
           fabs((double)tl[2] - CV_PI) >= DBL_EPSILON ||
           (int)tl[3] != -3435345 ||
           (string)tl[4] != "2-502 2-029 3egegeg" ||
            tl[5].type() != FileNode::MAP || tl[5].size() != 3 ||
            (int)tl[5]["month"] != 12 ||
            (int)tl[5]["day"] != 31 ||
            (int)tl[5]["year"] != 1969 )
        {
            ts->printf( CvTS::LOG, "the test list is incorrect\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        FileNode tm = fs["test_map"];
        FileNode tm_lbp = tm["lbp"];
        
        int real_x = (int)tm["x"];
        int real_y = (int)tm["y"];
        int real_width = (int)tm["width"];
        int real_height = (int)tm["height"];
        
        
        int real_lbp_val = 0;
        FileNodeIterator it = tm_lbp.begin();
        for( int k = 0; k < 8; k++, ++it )
            real_lbp_val |= (int)*it << k;
        
        if( tm.type() != FileNode::MAP || tm.size() != 5 ||
            real_x != 1 ||
            real_y != 2 ||
            real_width != 100 ||
            real_height != 200 ||
            tm_lbp.type() != FileNode::SEQ ||
            tm_lbp.size() != 8 ||
            real_lbp_val != 0xb6 )
        {
            ts->printf( CvTS::LOG, "the test map is incorrect\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        fs.release();
        #ifdef _MSC_VER
            _unlink(filename);
        #else
            unlink(filename);
        #endif
    }
}

CV_IOTest opencv_io_test;
