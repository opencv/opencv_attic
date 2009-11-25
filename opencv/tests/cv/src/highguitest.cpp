/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//   * The name of the copyright holders may not be used to endorse or promote products
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

#include "cvtest.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include "cvaux.h"

using namespace cv;
using namespace std;



#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
    
#else

#define MARKERS1

#ifdef MARKERS
	#define marker(x) cout << (x)  << endl
#else
	#define marker(x) 
#endif

struct TempDirHolder
{
	const string temp_folder;
	TempDirHolder() : temp_folder(tmpnam(0)) { exec_cmd("mkdir " + temp_folder); }	
	~TempDirHolder() { exec_cmd("rm -rf " + temp_folder); }
	static void exec_cmd(const string& cmd) { marker(cmd); int res = system( cmd.c_str() ); (void)res; }
	
	TempDirHolder& operator=(const TempDirHolder&);
};


class CV_HighGuiTest : public CvTest
{
public:
    CV_HighGuiTest();
    ~CV_HighGuiTest();    
protected:    
    void run(int);
	
	bool ImagesTest(const string& dir, const string& tmp);
	bool VideoTest(const string& dir, const string& tmp);
};

CV_HighGuiTest::CV_HighGuiTest(): CvTest( "highgui", "?" )
{
    support_testing_modes = CvTS::CORRECTNESS_CHECK_MODE;
}
CV_HighGuiTest::~CV_HighGuiTest() {}

double PSNR(const Mat& m1, const Mat& m2)
{		
	Mat tmp;
	absdiff( m1.reshape(1), m2.reshape(1), tmp);
	multiply(tmp, tmp, tmp);
		
	double MSE =  1.0/(tmp.cols * tmp.rows) * sum(tmp)[0];
	
	return 20 * log10(255.0 / sqrt(MSE));	
}

bool CV_HighGuiTest::ImagesTest(const string& dir, const string& tmp)
{
	Mat image = imread(dir + "shared/baboon.jpg");
	
	if (image.empty())
	{
		 ts->set_failed_test_info(CvTS::FAIL_MISSING_TEST_DATA);
		 return false;
	}	
		
	const string exts[] = {"png", "bmp", "tiff", "jpg", "jp2", "ppm"/*, "ras"*/};	
	const size_t ext_num = sizeof(exts)/sizeof(exts[0]);	
	
	for(size_t i = 0; i < ext_num; ++i)
	{
		string full_name = tmp + "/img." + exts[i];
		marker(exts[i]);	
		
		marker("begin");	
			
		imwrite(full_name, image);			
		marker("begin++");	
		Mat loaded = imread(full_name);	
		if (loaded.empty())
		{
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;					
		}				
		marker("begin++++");	
						
		const double thresDbell = 20;		
		if (PSNR(loaded, image) < thresDbell)
		{
			
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;			
		}	
		
		
		FILE *f = fopen(full_name.c_str(), "rb");
		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);				
		vector<uchar> from_file(len);
		fseek(f, 0, SEEK_SET);
		size_t read = fread(&from_file[0], len, sizeof(vector<uchar>::value_type), f); (void)read;
		fclose(f);
		
		marker("0");
									
		vector<uchar> buf;		
		imencode("." + exts[i], image, buf);
		
		marker("0++");
		
		if (buf != from_file)
		{		
			marker("1");		
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;			
		}			
		
		marker("1++");				
		Mat buf_loaded = imdecode(buf, 1);
		marker("1--");				
		if (buf_loaded.empty())
		{
			marker("2");		
			if (exts[i] == "tiff" || exts[i] == "jp2") continue;			
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;				
		}
							
		if (PSNR(buf_loaded, image) < thresDbell)
		{			
			marker("3");
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;			
		}					
		marker("3--");	
	}  
	return true;		
}

bool CV_HighGuiTest::VideoTest(const string& dir, const string& tmp)
{	
	string src_file = dir + "shared/video_for_test.avi";		
	string tmp_name1 = tmp + "/img1.avi";
	string tmp_name2 = tmp + "/img2.avi";
	
	CvCapture* cap = cvCaptureFromFile(src_file.c_str());
	
	if (!cap)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;
	}
	
	CvVideoWriter* writer1 = 0;
	CvVideoWriter* writer2 = 0;
	
	while(1)
	{
		IplImage* img = cvQueryFrame( cap );
		if (!img)
			break;
		
		if (writer1 == 0)
			//cvCreateVideoWriter(tmp_name1.c_str(), CV_FOURCC('M','J','P','G'), 24, cvGetSize(img));		
			writer1 = cvCreateVideoWriter(tmp_name1.c_str(), CV_FOURCC_DEFAULT, 24, cvGetSize(img));							
			
		//if (writer1 == 0)
			//writer2 = cvCreateVideoWriter(tmp_name2.c_str(), CV_FOURCC('X','V','I','D'), 24, cvGetSize(img));	
			
		cvWriteFrame(writer1, img);
		//cvWriteFrame(writer2, img);
	}	
	
	marker("mid");
	
	cvReleaseVideoWriter( &writer1 );
	//cvReleaseVideoWriter( &writer2 );	
	cvReleaseCapture( &cap );
	
	marker("mid++");
	
	cap = cvCaptureFromFile(src_file.c_str());
	marker("mid1");
	CvCapture *cap1 = cvCaptureFromFile(tmp_name1.c_str());
	marker("mid2");
	//CvCapture *cap2 = cvCaptureFromFile(tmp_name2.c_str());
	
	marker("mid??");
	
	if (!cap1)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;			
	}
	
	marker("mid--");
	
	
	const double thresDbell = 20;	
	
	bool error = false;
	while(1)
	{		
		IplImage* ipl = cvQueryFrame( cap );
		IplImage* ipl1 = cvQueryFrame( cap1 );
		
		if (!ipl || !ipl1)
			break;
			
		Mat img(ipl);		
		Mat img1(ipl1);						
				
		if (PSNR(img1, img) < thresDbell)
		{		
			error = true;
			break;				
		}			
	}	
		
	cvReleaseCapture( &cap );
	cvReleaseCapture( &cap1 );
	//cvReleaseCapture( &cap2 );
	
	if (error)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;			
	}
	
	return true;		
}


void CV_HighGuiTest::run( int start_from )
{	   
    TempDirHolder th;
		
	if (!ImagesTest(ts->get_data_path(), th.temp_folder))
		return;
		
	if (!VideoTest(ts->get_data_path(), th.temp_folder))
		return;				
  
    ts->set_failed_test_info(CvTS::OK);
}

CV_HighGuiTest HighGui_test;


#endif

