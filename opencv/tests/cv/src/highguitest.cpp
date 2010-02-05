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
#include <iostream>
#include "cvaux.h"

using namespace cv;
using namespace std;



//#if defined WIN32 || defined _WIN32 || defined WIN64 || defined _WIN64
#if 0
    
#else

#define MARKERS

#ifdef MARKERS
	#define marker(x) cout << (x)  << endl
#else
	#define marker(x) 
#endif

struct TempDirHolder
{
	const string temp_folder;
	TempDirHolder() : temp_folder(tmpnam(0)) { cout << "1temp-1" << endl; exec_cmd("mkdir " + temp_folder); cout << "1temp-2" << endl;}	
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
	bool VideoTest(const string& dir, const string& tmp, int fourcc);
	
	bool GuiTest(const string& dir, const string& tmp);
};

CV_HighGuiTest::CV_HighGuiTest(): CvTest( "z-highgui", "?" )
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
	int code = CvTS::OK;
    cout << "2-1" << endl;
	Mat image = imread(dir + "shared/baboon.jpg");
    cout << "2-2" << endl;
	
	if (image.empty())
	{
		 ts->set_failed_test_info(CvTS::FAIL_MISSING_TEST_DATA);
		 return false;
	}	
		
	const string exts[] = {"png", "bmp", "tiff", "jpg", "jp2", "ppm", "ras"};	
	const size_t ext_num = sizeof(exts)/sizeof(exts[0]);	
	
	for(size_t i = 0; i < ext_num; ++i)
	{
        cout << "2-#" << i << "\t1" << endl;
		ts->printf(CvTS::LOG, "ext=%s\n", exts[i].c_str());
        string ext = exts[i];
		string full_name = tmp + "/img." + ext;
		marker(exts[i]);	
        cout << "2-#" << i << "\t2" << endl;
		
		imwrite(full_name, image);			
		Mat loaded = imread(full_name);	
		if (loaded.empty())
		{
            ts->printf(CvTS::LOG, "Reading failed at fmt=%s\n", ext.c_str());
			code = CvTS::FAIL_MISMATCH;
			continue;
		}			
        cout << "2-#" << i << "\t3" << endl;
						
		const double thresDbell = 20;
		double psnr = PSNR(loaded, image);
		if (psnr < thresDbell)
		{
			ts->printf(CvTS::LOG, "Reading image from file: too big difference (=%g) with fmt=%s\n", psnr, ext.c_str());
			code = CvTS::FAIL_BAD_ACCURACY;
			continue;			
		}	
        cout << "2-#" << i << "\t4" << endl;
		
		FILE *f = fopen(full_name.c_str(), "rb");
		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);				
		vector<uchar> from_file(len);
		fseek(f, 0, SEEK_SET);
		size_t read = fread(&from_file[0], len, sizeof(vector<uchar>::value_type), f); (void)read;
		fclose(f);

        cout << "2-#" << i << "\t5" << endl;
		
		vector<uchar> buf;		
		imencode("." + exts[i], image, buf);
		
		if (buf != from_file)
		{
            ts->printf(CvTS::LOG, "Encoding failed with fmt=%s\n", ext.c_str());
			code = CvTS::FAIL_MISMATCH;
			continue;			
		}			
        cout << "2-#" << i << "\t6" << endl;
		
		Mat buf_loaded = imdecode(Mat(buf), 1);
		if (buf_loaded.empty())
		{
			ts->printf(CvTS::LOG, "Decoding failed with fmt=%s\n", ext.c_str());
            code = CvTS::FAIL_MISMATCH;
			continue;				
		}

        cout << "2-#" << i << "\t7" << endl;
		
        psnr = PSNR(buf_loaded, image);
		if (psnr < thresDbell)
		{
			ts->printf(CvTS::LOG, "Decoding image from memory: too small PSNR (=%gdb) with fmt=%s\n", psnr, ext.c_str());
			code = CvTS::FAIL_MISMATCH;
			continue;			
		}					
	}
    cout << "2-e" << endl;
	ts->set_failed_test_info(code);  
	return code == CvTS::OK;		
}

bool CV_HighGuiTest::VideoTest(const string& dir, const string& tmp, int fourcc)
{	
	string src_file = dir + "shared/video_for_test.avi";		
	string tmp_name = tmp + "/video.avi";
		
	CvCapture* cap = cvCaptureFromFile(src_file.c_str());
	
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t1"<< endl;
	if (!cap)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;
	}
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2"<< endl;
	
	CvVideoWriter* writer = 0;
	
    int counter = 0;
	for(;;)
	{
		IplImage* img = cvQueryFrame( cap );

        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2 + " << counter++<< endl;
		if (!img)
			break;
		
		if (writer == 0)			
		{
            cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2 + 0" << counter<< endl;
			writer = cvCreateVideoWriter(tmp_name.c_str(), fourcc, 24, cvGetSize(img));					
            cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2 + 1" << counter<< endl;
			if (writer == 0)
			{
				marker("can't craete writer");
				cvReleaseCapture( &cap );
				ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
				return false;				
			}
		}
				
        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2 ?" << counter<< endl;
		cvWriteFrame(writer, img);		
        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t2 ??" << counter<< endl;
	}	
		

    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t3"<< endl;
	cvReleaseVideoWriter( &writer );	
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t3.5"<< endl;
	cvReleaseCapture( &cap );
	
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t4"<< endl;
	marker("mid++");
	
	cap = cvCaptureFromFile(src_file.c_str());
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t5"<< endl;
	marker("mid1");
	CvCapture *saved = cvCaptureFromFile(tmp_name.c_str());		
	if (!saved)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;			
	}

    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t6"<< endl;
	

	const double thresDbell = 20;	
	
	bool error = false;
    counter = 0;
	for(;;)
	{		

        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t6 + " << counter++ << endl; ;
		IplImage* ipl = cvQueryFrame( cap );
		IplImage* ipl1 = cvQueryFrame( saved );

        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t6 + ??" << counter << endl; ;
		
		if (!ipl || !ipl1)
			break;
			
		Mat img(ipl);		
		Mat img1(ipl1);						
				
        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t6 + !" << counter << endl; ;
		if (PSNR(img1, img) < thresDbell)
		{		
			error = true;
			break;				
		}			
        cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t6 + !!" << counter << endl; ;
	}	
		
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t7" << endl; ;
	cvReleaseCapture( &cap );
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t8" << endl; ;
	cvReleaseCapture( &saved );
    cout << "Dir = " << dir << "tmp = " << tmp << "fourcc = " << fourcc << "\t9" << endl; ;
		
	if (error)
	{
		ts->set_failed_test_info(CvTS::FAIL_MISMATCH);
		return false;			
	}
	
	return true;		
}


void CV_HighGuiTest::run( int /*start_from */)
{	   
    cout << "1" << endl;
    TempDirHolder th;
    cout << "2" << endl;
		
	if (!ImagesTest(ts->get_data_path(), th.temp_folder))
		return;

    cout << "3" << endl;
#if defined WIN32 || defined __linux__

#if !defined HAVE_GSTREAMER || defined HAVE_GSTREAMER_APP  
	if (!VideoTest(ts->get_data_path(), th.temp_folder, CV_FOURCC_DEFAULT))
		return;	

    cout << "4" << endl;

	if (!VideoTest(ts->get_data_path(), th.temp_folder, CV_FOURCC('M', 'J', 'P', 'G')))
		return;

    cout << "5" << endl;
    
	if (!VideoTest(ts->get_data_path(), th.temp_folder, CV_FOURCC('M', 'P', 'G', '2')))
		return;				

    cout << "6" << endl;
#endif
	//if (!VideoTest(ts->get_data_path(), th.temp_folder, CV_FOURCC('D', 'X', '5', '0')))		return;				
	
	if (!GuiTest(ts->get_data_path(), th.temp_folder))
		return;

    cout << "7" << endl;
#endif
    ts->set_failed_test_info(CvTS::OK);
}

void Foo(int /*k*/, void* /*z*/)
{
	
}

bool CV_HighGuiTest::GuiTest(const string& /*dir*/, const string&/* tmp*/)
{	
    cout << "GUI 1" << endl;
	namedWindow("Win");
    cout << "GUI 2" << endl;
	Mat m(30, 30, CV_8U);	
	m = Scalar(128);	
    cout << "GUI 3" << endl;
	imshow("Win", m);	
    cout << "GUI 4" << endl;
	int value = 50;
    cout << "GUI 5" << endl;
	createTrackbar( "trackbar", "Win", &value, 100, Foo, &value);	
    cout << "GUI 6" << endl;
	getTrackbarPos( "trackbar", "Win" );	
    cout << "GUI 7" << endl;
	waitKey(500);		
    cout << "GUI 8" << endl;
	cvDestroyAllWindows();
    cout << "GUI 9" << endl;
	
	return true;
}

CV_HighGuiTest HighGui_test;


#endif

