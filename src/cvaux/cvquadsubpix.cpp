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

#include <limits>
#include <utility>
#include <algorithm>

#include <cvaux.h>
#include <highgui.h>

#define _SUBPIX_VERBOSE

namespace cv {
    
    
void drawCircles(Mat& img, const vector<Point2f>& corners, const vector<float>& radius)
{
    for(size_t i = 0; i < corners.size(); i++)
    {
        circle(img, corners[i], radius[i], CV_RGB(255, 0, 0));
    }
}
    
int histQuantile(const MatND& hist, float quantile)
{
    if(hist.dims > 1) return -1; // works for 1D histograms only
    
    float cur_sum = 0;
    float total_sum = sum(hist).val[0];
    float quantile_sum = total_sum*quantile;
    for(int j = 0; j < hist.size[0]; j++)
    {
        cur_sum += hist.at<float>(j);
        if(cur_sum > quantile_sum)
        {
            return j;
        }
    }
    
    return hist.size[0] - 1;
}
    
bool is_smaller(const std::pair<int, float>& p1, const std::pair<int, float>& p2)
{
    return p1.second < p2.second;
}

void orderContours(const vector<vector<Point> >& contours, Point2f point, vector<std::pair<int, float> >& order)
{
    order.clear();
    for(size_t i = 0; i < contours.size(); i++)
    {
        double min_dist = std::numeric_limits<double>::max();
        for(size_t j = 0; j < contours[i].size(); j++)
        {
            double dist = norm(Point2f(contours[i][j].x, contours[i][j].y) - point);
            min_dist = MIN(min_dist, dist);
        }
        order.push_back(std::pair<int, float>(i, min_dist));
    }
    
    std::sort(order.begin(), order.end(), is_smaller);
}

// fit second order curve to a set of 2D points
void fitCurve2Order(const vector<Point2f>& points, vector<float>& curve)
{
    // TBD
}
    
void findCurvesCross(const vector<float>& curve1, const vector<float>& curve2, Point2f& cross_point)
{
}
    
void findLinesCrossPoint(Point2f origin1, Point2f dir1, Point2f origin2, Point2f dir2, Point2f& cross_point)
{
    float det = dir2.x*dir1.y - dir2.y*dir1.x;
    Point2f offset = origin2 - origin1;
    
    float alpha = (dir2.x*offset.y - dir2.y*offset.x)/det;
    cross_point = origin1 + dir1*alpha;
}
    
void findCorner(const vector<Point>& contour, Point2f point, Point2f& corner)
{
    // find the nearest point
    double min_dist = std::numeric_limits<double>::max();
    int min_idx = -1;
    
    Rect brect = boundingRect(contour);
    
    // find corner idx
    for(size_t i = 0; i < contour.size(); i++)
    {
        double dist = norm(Point2f(contour[i].x, contour[i].y) - point);
        if(dist < min_dist)
        {
            min_dist = dist;
            min_idx = i;
        }
    }
    assert(min_idx >= 0);
    
    // temporary solution, have to make something more precise
    corner = contour[min_idx];
    return;
    
#if 0
    // find the points corresponding to neighboring sides of the quadrangle
    int idx;

    for(idx = min_idx; idx != min_idx; idx = (idx + 1) % contour.size())
    {
        if(fabs(contour[idx].x - point.x) > brect.width*0.5f || 
           fabs(contour[idx].y - point.y) > brect.height*0.5f)
        {
            break;
        }
    }
    assert(idx == min_idx);
    size_t idx1 = idx;
    vector<float> curve1;
    vector<Point2f> border_points1;
    for(size_t i = min_idx; i != idx1; i = (i + 1) % contour.size())
    {
        border_points1.push_back(contour[i]);
    }
    fitCurve2Order(border_points1, curve1);
    
    for(idx = min_idx; idx != min_idx; idx = (idx - 1) % contour.size())
    {
        if(fabs(contour[idx].x - point.x) > brect.width*0.5f || 
           fabs(contour[idx].y - point.y) > brect.height*0.5f)
        {
            break;
        }
    }
    assert(idx == min_idx);
    size_t idx2 = idx;
    vector<float> curve2;
    vector<Point2f> border_points2;
    for(size_t i = min_idx; i != idx2; i = (i - 1) % contour.size())
    {
        border_points2.push_back(contour[i]);
    }
    fitCurve2Order(border_points2, curve2);
    
    findCurvesCross(curve1, curve2, corner);
#endif
}

bool find4QuadCornerSubpix(const Mat& img, std::vector<Point2f>& corners, Size region_size, std::vector<Point2f>& ground_truth_corners)
{
    const int nbins = 256;
    float ranges[] = {0, 256};
    const float* _ranges = ranges;
    MatND hist;
    
#if defined(_SUBPIX_VERBOSE)
    vector<float> radius;
    radius.assign(corners.size(), 0.0f);
#endif //_SUBPIX_VERBOSE
    
    
    Mat black_comp, white_comp;
    for(size_t i = 0; i < corners.size(); i++)
    {
        size_t ground_truth_idx = 0;
        double min_ground_truth_dist = std::numeric_limits<double>::max();
        for(size_t idx = 0; idx < ground_truth_corners.size(); idx++)
        {
            float dist = norm(ground_truth_corners[idx] - corners[i]);
            if(dist < min_ground_truth_dist)
            {
                min_ground_truth_dist = dist;
                ground_truth_idx = idx;
            }
        }
        
        int channels = 0;
        Rect roi(corners[i].x - region_size.width, corners[i].y - region_size.height, region_size.width*2, region_size.height*2);
        Mat img_roi = img(roi);
        calcHist(&img_roi, 1, &channels, Mat(), hist, 1, &nbins, &_ranges);
        
        int black_thresh = histQuantile(hist, 0.45f);
        int white_thresh = histQuantile(hist, 0.55f);
        
        threshold(img, black_comp, black_thresh, 255.0, CV_THRESH_BINARY_INV);
        threshold(img, white_comp, white_thresh, 255.0, CV_THRESH_BINARY);
        
        const int erode_count = 2;
        erode(black_comp, black_comp, Mat(), Point(-1, -1), erode_count);
        erode(white_comp, white_comp, Mat(), Point(-1, -1), erode_count);

#if 0
        namedWindow("black", 1);
        imshow("black", black_comp);
        namedWindow("white", 1);
        imshow("white", white_comp);
        cvWaitKey(0);
        imwrite("black.jpg", black_comp);
        imwrite("white.jpg", white_comp);
#endif
        
        
        vector<vector<Point> > white_contours, black_contours;
        vector<Vec4i> white_hierarchy, black_hierarchy;
        findContours(black_comp, black_contours, black_hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        findContours(white_comp, white_contours, white_hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        
        if(black_contours.size() < 10 || white_contours.size() < 10) continue;
        
        // find two white and black blobs that are close to the input point
        vector<std::pair<int, float> > white_order, black_order;
        orderContours(black_contours, corners[i], black_order);
        orderContours(white_contours, corners[i], white_order);

        const float max_dist = 10.0f;
        if(black_order[0].second > max_dist || black_order[1].second > max_dist || 
           white_order[0].second > max_dist || white_order[1].second > max_dist)
        {
            continue; // there will be no improvement in this corner position
        }
        
        const vector<Point>* quads[4] = {&black_contours[black_order[0].first], &black_contours[black_order[1].first], 
                                         &white_contours[white_order[0].first], &white_contours[white_order[1].first]};
        Point2f quad_corners[4];
        for(int k = 0; k < 4; k++)
        {
            findCorner(*quads[k], corners[i], quad_corners[k]);
            quad_corners[k] += Point2f(0.5f, 0.5f);
        }
        
        double old_dist = norm(corners[i] - ground_truth_corners[ground_truth_idx]);
        // cross two lines
        Point2f origin1 = quad_corners[0];
        Point2f dir1 = quad_corners[1] - quad_corners[0];
        Point2f origin2 = quad_corners[2];
        Point2f dir2 = quad_corners[3] - quad_corners[2];
        findLinesCrossPoint(origin1, dir1, origin2, dir2, corners[i]);

        double new_dist = norm(corners[i] - ground_truth_corners[ground_truth_idx]);
      
#if defined(_SUBPIX_VERBOSE)
#if 1
//        printf("Corner position: %f,%f, true position: %f,%f\n", corners[i].x, corners[i].y, 
//               ground_truth_corners[ground_truth_idx].x, ground_truth_corners[ground_truth_idx].y);
        printf("Improved error for corner %d from %f to %f\n", (int)i, old_dist, new_dist);
#endif
        radius[i] = norm(corners[i] - ground_truth_corners[ground_truth_idx])*6;
     
#if 1
        Mat img1 = img(Rect(corners[i].x - 30, corners[i].y - 30, 60, 60));
        Mat test(img1.size(), CV_32FC3);
        cvtColor(img1, test, CV_GRAY2RGB);
        line(test, quad_corners[0] - corners[i] + Point2f(30, 30), quad_corners[1] - corners[i] + Point2f(30, 30), cvScalar(0, 255, 0));
        line(test, quad_corners[2] - corners[i] + Point2f(30, 30), quad_corners[3] - corners[i] + Point2f(30, 30), cvScalar(0, 255, 0));
        vector<vector<Point> > contrs;
        contrs.resize(1);
        for(int k = 0; k < 4; k++)
        {
            contrs[0] = *quads[k];
            drawContours(test, contrs, 0, CV_RGB(0, 0, 255), 1, 1, vector<Vec4i>(), 2, Point2f(30, 30) - corners[i]);
            circle(test, quad_corners[k] - corners[i] + Point2f(30, 30), 1, CV_RGB(255, 0, 0));
        }
        namedWindow("1", 1);
        imshow("1", test);
        imwrite("test.jpg", test);
        waitKey(0);
#endif
#endif //_SUBPIX_VERBOSE
        
    }
    
#if defined(_SUBPIX_VERBOSE)
    Mat test(img.size(), CV_32FC3);
    cvtColor(img, test, CV_GRAY2RGB);
    drawCircles(test, corners, radius);

    namedWindow("corners", 1);
    imshow("corners", test);
    waitKey();
#endif //_SUBPIX_VERBOSE
    
    return true;
}
    
}; // namespace std
