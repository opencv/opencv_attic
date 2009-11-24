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

struct TempDirHolder
{
	const string temp_folder;
	TempDirHolder() : temp_folder(tmpnam(0)) 
	{
		string cmd = "mkdir " + temp_folder;	
	    //cout << "Createing dir: " << cmd << endl;    
	    int res = system( cmd.c_str() );	
		(void)res;
	} 
	~TempDirHolder()
	{
		string cmd  = "rm -rf " + temp_folder;
		//cout << "Deleting dir: " << cmd << endl;
		int res = system( cmd.c_str() );		
		(void)res;
	}
	
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
		
	const string exts[] = {"png", "bmp", "tiff", "jpg"};	
	const size_t ext_num = sizeof(exts)/sizeof(exts[0]);	
	
	for(size_t i = 0; i < ext_num; ++i)
	{
		string full_name = tmp + "/img." + exts[i];
				
		imwrite(full_name, image);			
		Mat loaded = imread(full_name);					
				
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
									
		vector<uchar> buf;		
		imencode("." + exts[i], image, buf);
		
		if (buf != from_file)
		{			
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;			
		}			
						
		Mat buf_loaded = imdecode(buf, 1);
		if (buf_loaded.empty())
		{
			//if (exts[i] == "tiff") continue;
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;				
		}
						
		if (PSNR(buf_loaded, image) < thresDbell)
		{			
			ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
			return false;			
		}					
	}  
	return true;		
}

bool CV_HighGuiTest::VideoTest(const string& dir, const string& tmp)
{	
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
