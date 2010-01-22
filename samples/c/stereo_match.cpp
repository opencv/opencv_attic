/*
 *  stereo_match.cpp
 *  calibration
 *
 *  Created by Victor  Eruhimov on 1/18/10.
 *  Copyright 2010 Argus Corp. All rights reserved.
 *
 */

#include <cv.h>
#include <highgui.h>
using namespace std;

void saveXYZ(const char* filename, CvMat* mat)
{
    const double max_z = 1.0e4;
    FILE* fp = fopen(filename, "wt");
    for(int y = 0; y < mat->rows; y++)
    {
        for(int x = 0; x < mat->cols; x++)
        {
            CvScalar point = cvGet2D(mat, y, x);
            if(fabs(point.val[2] - max_z) < FLT_EPSILON || fabs(point.val[2]) > max_z) continue;
            fprintf(fp, "%f %f %f\n", point.val[0], point.val[1], point.val[2]);
        }
    }
    fclose(fp);
    
    cvSave("temp_xyz.yml", mat);
}

void reproject(CvMat* Q, CvMat* disp)
{
    float xp1 = -cvGet2D(Q, 0, 3).val[0];
    float yp1 = -cvGet2D(Q, 1, 3).val[0]; 
    float f = cvGet2D(Q, 2, 3).val[0];
    float invs = cvGet2D(Q, 3, 2).val[0];
    
    double min_disp, max_disp;
    cvMinMaxLoc(disp, &min_disp, &max_disp, 0, 0);
    
    FILE* fp = fopen("xyz_temp.off", "wt");
    int count = 0;
    for(int y = 0; y < disp->rows; y++)
    {
        for(int x = 0; x < disp->cols; x++)
        {
            float d = cvGet2D(disp, y, x).val[0];
            if(fabs(d - min_disp) < FLT_EPSILON) 
            {
                count++;
                continue;
            }
            
            float X = (x - xp1)/(d*invs);
            float Y = (y - yp1)/(d*invs);
            float Z = f/(d*invs);
            
            fprintf(fp, "%f %f %f\n", X, Y, Z);
        }
    }
    fclose(fp);
    printf("Filtered out %d points\n", count);
}

inline const char* getarg(int argc, char* const* argv, int idx)
{
    return argc > idx ? argv[idx] : NULL;
}

int main(int argc, char** argv)
{
    if(argc < 5)
    {
        printf("Usage: stereo_match <img1> <img2> <intrinsic_filename> <extrinsic_filename> [<disparity>] [<point_cloud>]\n");
        return 0;
    }
    const char* img1_filename = argv[1];
    const char* img2_filename = argv[2];
    const char* intrinsic_filename = argv[3];
    const char* extrinsic_filename = argv[4];
    const char* disparity_filename = getarg(argc, argv, 5);
    const char* point_cloud_filename = getarg(argc, argv, 6);
        
    CvMemStorage* storage = cvCreateMemStorage();
    // reading intrinsic parameters
    CvFileStorage* fstorage = cvOpenFileStorage(intrinsic_filename, storage, CV_STORAGE_READ);
    if(!fstorage)
    {
        printf("Failed to open file %s\n", intrinsic_filename);
        return 0;
    }
    CvMat* M1 = (CvMat*)cvReadByName(fstorage, NULL, "M1");
    CvMat* D1 = (CvMat*)cvReadByName(fstorage, NULL, "D1");
    CvMat* M2 = (CvMat*)cvReadByName(fstorage, NULL, "M2");
    CvMat* D2 = (CvMat*)cvReadByName(fstorage, NULL, "D2");
    cvReleaseFileStorage(&fstorage);
    if(!M1 || !D1 || !M2 || !D2)
    {
        printf("Failed to read intrinsic parameters from %s\n", intrinsic_filename);
        return 0;
    }
    
    // reading extrinsic parameters
    fstorage = cvOpenFileStorage(extrinsic_filename, storage, CV_STORAGE_READ);
    if(!fstorage)
    {
        printf("Failed to open file %s\n", extrinsic_filename);
        return 0;
    }
    CvMat* R1 = (CvMat*)cvReadByName(fstorage, NULL, "R1");
    CvMat* P1 = (CvMat*)cvReadByName(fstorage, NULL, "P1");
    CvMat* R2 = (CvMat*)cvReadByName(fstorage, NULL, "R2");
    CvMat* P2 = (CvMat*)cvReadByName(fstorage, NULL, "P2");
    CvMat* Q = (CvMat*)cvReadByName(fstorage, NULL, "Q");
    cvReleaseFileStorage(&fstorage);
    if(!R1 || !P1 || !R2 || !P2 || !Q)
    {
        printf("Failed to read extrinsic parameters from %s\n", extrinsic_filename);
        return 0;
    }
    
    CvMat* mx1 = NULL;
    CvMat* mx2 = NULL;
    CvMat* my1 = NULL;
    CvMat* my2 = NULL;
    CvMat* img1_rect = NULL;
    CvMat* img2_rect = NULL;
            
    IplImage* img1 = cvLoadImage(img1_filename, CV_LOAD_IMAGE_GRAYSCALE);
    IplImage* img2 = cvLoadImage(img2_filename, CV_LOAD_IMAGE_GRAYSCALE);
    
#if 0
    IplImage* _img1 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    IplImage* _img2 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
                                    
    cvResize(img1, _img1);
    cvResize(img2, _img2);
    cvReleaseImage(&img1);
    cvReleaseImage(&img2);
    img1 = _img1;
    img2 = _img2;
#endif
    
    CvSize imageSize = cvGetSize(img1);
    
    if(mx1 == NULL)
    {
        mx1 = cvCreateMat(imageSize.height, imageSize.width, CV_32F);
        mx2 = cvCreateMat(imageSize.height, imageSize.width, CV_32F);
        my1 = cvCreateMat(imageSize.height, imageSize.width, CV_32F);
        my2 = cvCreateMat(imageSize.height, imageSize.width, CV_32F);
        cvInitUndistortRectifyMap(M1, D1, R1, P1, mx1, my1);
        cvInitUndistortRectifyMap(M2, D2, R2, P2, mx2, my2);
        
        img1_rect = cvCreateMat(imageSize.height, imageSize.width, CV_8U);
        img2_rect = cvCreateMat(imageSize.height, imageSize.width, CV_8U);
    }
    
    cvRemap(img1, img1_rect, mx1, my1);
    cvRemap(img2, img2_rect, mx2, my2);
    
    CvMat* disp = cvCreateMat( imageSize.height,
                              imageSize.width, CV_16S );
    CvMat* vdisp = cvCreateMat( imageSize.height,
                               imageSize.width, CV_8U );
    CvMat* dispn = cvCreateMat( imageSize.height, imageSize.width, CV_32F );
    
    CvStereoBMState *BMState = cvCreateStereoBMState();
    assert(BMState != 0);
    BMState->preFilterSize=31;
    BMState->preFilterCap=31;
    BMState->SADWindowSize=15;//255;
    BMState->minDisparity=-192;
    BMState->numberOfDisparities=192;
    BMState->textureThreshold=10;
    BMState->uniquenessRatio=15;
    
#if 1
    BMState->minDisparity *= imageSize.width/640;
    BMState->numberOfDisparities *= imageSize.width/640;
#endif
    
    int64 _time1 = cvGetTickCount();
    cvFindStereoCorrespondenceBM( img1_rect, img2_rect, disp,
                                 BMState);
    int64 _time2 = cvGetTickCount();
    printf("Time elapsed: %f\n", float(_time2 - _time1)/cvGetTickFrequency()*1e-6);
        
    // hack to get rid of small-scale noise
    cvErode(disp, disp, NULL, 2);
    cvDilate(disp, disp, NULL, 2);
    
    // should be removed when BM algorithm starts returning disparities rather than disparities*16
    cvConvertScale(disp, dispn, 1.0/16);
    

    if(point_cloud_filename)
    {
        CvMat* xyz = cvCreateMat(disp->rows, disp->cols, CV_32FC3);
        cvReprojectImageTo3D(dispn, xyz, Q, 1);
        
        saveXYZ(point_cloud_filename, xyz);
        cvReleaseMat(&xyz);
    }
    
    cvNormalize( disp, vdisp, 0, 256, CV_MINMAX );
    cvNamedWindow( "disparity" );
    IplImage* vdisp1 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
    cvResize(vdisp, vdisp1);
    cvShowImage( "disparity", vdisp1 );
    cvWaitKey();
    
    if(disparity_filename)
    {
        cvSaveImage(disparity_filename, vdisp);
    }
    
    cvReleaseImage(&vdisp1);
    cvReleaseMat(&disp);
    cvReleaseMat(&dispn);
    cvReleaseMat(&vdisp);
    cvReleaseImage(&img1);
    cvReleaseImage(&img2);
    
    cvReleaseMat(&M1);
    cvReleaseMat(&D1);
    cvReleaseMat(&M2);
    cvReleaseMat(&D2);
    
    cvReleaseMat(&R1);
    cvReleaseMat(&P1);
    cvReleaseMat(&R2);
    cvReleaseMat(&P2);
    cvReleaseMat(&Q);
    
    cvReleaseMat(&mx1);
    cvReleaseMat(&my1);
    cvReleaseMat(&mx2);
    cvReleaseMat(&my2);
    cvReleaseMat(&img1_rect);
    cvReleaseMat(&img2_rect);
    
    cvReleaseMemStorage(&storage);
    
}