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


void CV_IOTest::run( int start_from )
{
    double ranges[][2] = {{0, 256}, {-128, 128}, {0, 65536}, {-32768, 32768},
        {-1000000, 1000000}, {-10, 10}, {-10, 10}};
    CvRNG* rng = ts->get_rng();
    RNG rng0;
    test_case_count = 2;
    int progress = 0;
    
    for( int idx = 0; idx < test_case_count; idx++ )
    {
        ts->update_context( this, idx, false );
        char buf[L_tmpnam+16];
        char* filename = tmpnam(buf);
        strcat(filename, idx % 2 ? ".yml" : ".xml");
        
        FileStorage fs(filename, FileStorage::WRITE);
        
        int test_int = (int)cvTsRandInt(rng);
        double test_real = (cvTsRandInt(rng)%2?1:-1)*exp(cvTsRandReal(rng)*18-9);
        string test_string = "vw wv23424rtrt@#$@$%$%&%IJUKYILFD@#$@%$&*&() ";
        
        int depth = cvTsRandInt(rng) % (CV_64F+1);
        int cn = cvTsRandInt(rng) % 4 + 1;
        Mat test_mat(cvTsRandInt(rng)%30, cvTsRandInt(rng)%30, CV_MAKETYPE(depth, cn));
        
        rng0.fill(test_mat, CV_RAND_UNI, Scalar::all(ranges[depth][0]), Scalar::all(ranges[depth][1]));
        if( depth >= CV_32F )
        {
            exp(test_mat, test_mat);
            Mat test_mat_scale(test_mat.size(), test_mat.type());
            rng0.fill(test_mat_scale, CV_RAND_UNI, Scalar::all(-1), Scalar::all(1));
            multiply(test_mat, test_mat_scale, test_mat);
        }
        
        fs << "test_int" << test_int << "test_real" << test_real << "test_string" << test_string;
        fs << "test_mat" << test_mat;
        
        fs << "test_list" << "[" << 0.0000000000001 << 2 << CV_PI << -3435345 << "2-502 2-029 3egegeg" <<
            "{:" << "month" << 12 << "day" << 31 << "year" << 1969 << "}" << "]";
        fs << "test_map" << "{" << "x" << 1 << "y" << 2 << "width" << 100 << "height" << 200 <<
            "lbp" << "[:" << 0 << 1 << 1 << 0 << 1 << 1 << 0 << 1 << "]" << "}";
        
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
            fabs(real_real - test_real) > DBL_EPSILON ||
            real_string != test_string )
        {
            ts->printf( CvTS::LOG, "the read scalars are not correct\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        
        CvMat* m = (CvMat*)fs["test_mat"].readObj();
        if( !m || !CV_IS_MAT(m) || m->rows != test_mat.rows || m->cols != test_mat.cols ||
            cv::norm(cvarrToMat(m), test_mat, NORM_INF) != 0 )
        {
            ts->printf( CvTS::LOG, "the read matrix is not correct\n" );
            ts->set_failed_test_info( CvTS::FAIL_INVALID_OUTPUT );
            return;
        }
        if( m && CV_IS_MAT(m))
            cvReleaseMat(&m);
        
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
        
        int real_lbp_val = ((int)tm_lbp[0]<<0) + ((int)tm_lbp[1]<<1) +
            ((int)tm_lbp[2]<<2) + ((int)tm_lbp[3]<<3) +
            ((int)tm_lbp[4]<<4) + ((int)tm_lbp[5]<<5) +
            ((int)tm_lbp[6]<<6) + ((int)tm_lbp[7]<<7);
        
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
        unlink(filename);
        progress = update_progress( progress, idx, test_case_count, 0 );
    }
}

CV_IOTest opencv_io_test;
