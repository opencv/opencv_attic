#include "HOGfeatures.h"
#include "cascadeclassifier.h"


CvHOGFeatureParams::CvHOGFeatureParams()
{
    maxCatCount = 0;
    name = HOGF_NAME;
    featSize = N_BINS * N_CELLS;
}

void CvHOGEvaluator::init(const CvFeatureParams *_featureParams, int _maxSampleCount, Size _winSize)
{
    CV_Assert( _maxSampleCount > 0);
    int cols = (_winSize.width + 1) * (_winSize.height + 1);
    for (int bin = 0; bin < N_BINS; bin++)
    {
        hist.push_back(Mat(_maxSampleCount, cols, CV_32FC1));
    }
    //normSum = Mat( (int)_maxSampleCount, cols, CV_32FC1, 0.f );
    normSum.create( (int)_maxSampleCount, cols, CV_64FC1 );
    CvFeatureEvaluator::init( _featureParams, _maxSampleCount, _winSize );
}

void CvHOGEvaluator::setImage(const Mat &img, uchar clsLabel, int idx)
{
    CV_DbgAssert( !hist.empty());
    CvFeatureEvaluator::setImage( img, clsLabel, idx );
    vector<Mat> integralHist;
    for (int bin = 0; bin < N_BINS; bin++)
    {
        integralHist.push_back( Mat(winSize.height + 1, winSize.width + 1, hist[bin].type(), hist[bin].ptr<float>((int)idx)) );
    }
    Mat integralNorm(winSize.height + 1, winSize.width + 1, normSum.type(), normSum.ptr<double>((int)idx));
    integralHistogram(img, integralHist, integralNorm, (int)N_BINS);
}

//void CvHOGEvaluator::writeFeatures( FileStorage &fs, const Mat& featureMap ) const
//{
//    _writeFeatures( features, fs, featureMap );
//}

void CvHOGEvaluator::writeFeatures( FileStorage &fs, const Mat& featureMap ) const
{
    int featIdx;
    int componentIdx;
    const Mat_<int>& featureMap_ = (const Mat_<int>&)featureMap;
    fs << FEATURES << "[";
    for ( int fi = 0; fi < featureMap.cols; fi++ )
        if ( featureMap_(0, fi) >= 0 )
        {
            fs << "{";
            featIdx = fi / getFeatureSize();
            componentIdx = fi % getFeatureSize();
            features[featIdx].write( fs, componentIdx );
            fs << "}";
        }
    fs << "]";  
}

void CvHOGEvaluator::generateFeatures()
{
    int offset = winSize.width + 1;
    Size blockStep;
    int x, y, t, w, h;

    for (t = 8; t <= winSize.width/2; t+=8) //t = size of a cell. blocksize = 4*cellSize
    {
        blockStep = Size(4,4);
        w = 2*t; //width of a block
        h = 2*t; //height of a block
        for (x = 0; x <= winSize.width - w; x += blockStep.width)
        {
            for (y = 0; y <= winSize.height - h; y += blockStep.height)
            {
                features.push_back(Feature(offset, x, y, t, t));
            }
        }
        w = 2*t;
        h = 4*t;
        for (x = 0; x <= winSize.width - w; x += blockStep.width)
        {
            for (y = 0; y <= winSize.height - h; y += blockStep.height)
            {
                features.push_back(Feature(offset, x, y, t, 2*t));
            }
        }
        w = 4*t;
        h = 2*t; 
        for (x = 0; x <= winSize.width - w; x += blockStep.width)
        {
            for (y = 0; y <= winSize.height - h; y += blockStep.height)
            {     
                features.push_back(Feature(offset, x, y, 2*t, t));
            }
        }
    }

    numFeatures = (int)features.size();
}

CvHOGEvaluator::Feature::Feature()
{
    for (int i = 0; i < N_CELLS; i++)
    {
        rect[i] = Rect(0, 0, 0, 0);
    }
}

CvHOGEvaluator::Feature::Feature( int offset, int x, int y, int cellW, int cellH )
{
    rect[0] = Rect(x, y, cellW, cellH); //cell0
    rect[1] = Rect(x+cellW, y, cellW, cellH); //cell1
    rect[2] = Rect(x, y+cellH, cellW, cellH); //cell2
    rect[3] = Rect(x+cellW, y+cellH, cellW, cellH); //cell3

    for (int i = 0; i < N_CELLS; i++)
    {
        CV_SUM_OFFSETS(fastRect[i].p0, fastRect[i].p1, fastRect[i].p2, fastRect[i].p3, rect[i], offset);
    }
}

void CvHOGEvaluator::Feature::write(FileStorage &fs) const
{
    fs << CC_RECTS << "[";
    for( int i = 0; i < N_CELLS; i++ )
    {
        fs << "[:" << rect[i].x << rect[i].y << rect[i].width << rect[i].height << "]";
    }
    fs << "]";
}

//cell and bin idx writing
//void CvHOGEvaluator::Feature::write(FileStorage &fs, int varIdx) const
//{
//    int featComponent = varIdx % (N_CELLS * N_BINS);
//    int cellIdx = featComponent / N_BINS;
//    int binIdx = featComponent % N_BINS;
//
//    fs << CC_RECTS << "[:" << rect[cellIdx].x << rect[cellIdx].y << 
//        rect[cellIdx].width << rect[cellIdx].height << binIdx << "]";
//}

//cell[0] and featComponent idx writing. By cell[0] it's possible to recover all block
//All block is nessesary for block normalization
void CvHOGEvaluator::Feature::write(FileStorage &fs, int featComponentIdx) const
{
    fs << CC_RECT << "[:" << rect[0].x << rect[0].y << 
        rect[0].width << rect[0].height << featComponentIdx << "]";
}


void CvHOGEvaluator::integralHistogram(const Mat &srcImage, vector<Mat> &histogram, Mat& norm, int nbins) const
{
    int x, y, ch;

    Mat src;

    Mat Dx(srcImage.rows, srcImage.cols, CV_32F);
    Mat Dy(srcImage.rows, srcImage.cols, CV_32F);
    Mat Mag(srcImage.rows, srcImage.cols, CV_32F);
    Mat Angle(srcImage.rows, srcImage.cols, CV_32F);
    Mat Bins(srcImage.rows, srcImage.cols, CV_8S);

    //Adding borders for correct gradient computation
    copyMakeBorder(srcImage, src, 1, 1, 1, 1, BORDER_REPLICATE);

    //Differential computing along both dimensions
    for (y = 1; y < src.rows - 1; y++)
    {
        for (x = 1; x < src.cols - 1; x++)
        {
            Dx.at<float>(y-1, x-1) = (float)(src.at<uchar>(y, x+1) - src.at<uchar>(y, x-1));
            Dy.at<float>(y-1, x-1) = (float)(src.at<uchar>(y+1, x) - src.at<uchar>(y-1, x));
        }
    }
    //Computing of magnitudes and angles for all vectors
    cartToPolar(Dx, Dy, Mag, Angle);

#ifdef TEST_INTHIST_BUILD
    //for checking an integral histogram building 
    Mag = Mat(src.rows, src.cols, CV_32F, 1.f);
#endif

    //Angles adjusting for 9 bins
    float angleScale = (float)(nbins / CV_PI);
    float angle;
    int bidx;
    for (y = 0; y < Angle.rows; y++)
    {
        for (x = 0; x < Angle.cols; x++)
        {
            angle = Angle.at<float>(y,x)*angleScale - 0.5f;
            bidx = cvFloor(angle);

            angle -= bidx;
            if (bidx < 0)
            {
                bidx += nbins;
            }
            else if (bidx >= nbins)
            {
                bidx -= nbins;
            }
            Bins.at<char>(y,x) = (char)bidx;
        }
    }
    //Creating integral for magnitudes (useful for normaliztion calculation)
    integral(Mag, norm);
    //for( int i = 0; i <48; i++ )
    //    float tt = norm.at<double>(i,1);

    //Creating integral HOG
    int matIdx;
    for (ch = 0; ch < nbins; ch++)
    {
        for (int i = 0; i < histogram[ch].cols; i++)
            histogram[ch].at<float>(0, i) = 0.f; 
        for (int i = 0; i < histogram[ch].rows; i++)
            histogram[ch].at<float>(i, 0) = 0.f; 
    }
    for (y = 1; y <= Bins.rows; y++)
    {
        Mat strSums(1, nbins, CV_32F, 0.f); //суммы элементов (магнитуд) в y-ой строке для всех 9 матриц
        for (x = 1; x <= Bins.cols; x++)
        {
            matIdx = Bins.at<char>(y-1,x-1); //индекс матрицы в которую надо положить рассматриваемый элемент
            strSums.at<float>(matIdx) += Mag.at<float>(y-1,x-1);
            for (ch=0; ch<nbins; ch++)
            {
                float elem = histogram[ch].at<float>(y-1,x) + strSums.at<float>(ch);
                histogram[ch].at<float>(y,x) = elem;
                //norm.at<float>(y,x) += elem; //Creating integral for magnitudes
            }
        }
    }

#ifdef TEST_INTHIST_BUILD
    int i, j;
    FILE* dataFile;
    //Input image outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/0_inputImage.txt", "w");
    for (i=0; i<srcImage.rows; i++)
    {
        for (j=0; j<srcImage.cols; j++)
        {
            fprintf(dataFile, "%d ", srcImage.at<uchar>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //input image with border outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/1_inputImageWithBorders.txt", "w");
    for (i=0; i<src.rows; i++)
    {
        for (j=0; j<src.cols; j++)
        {
            fprintf(dataFile, "%d ", src.at<uchar>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Dx outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/2_Dx.txt", "w");
    for (i=0; i<Dx.rows; i++)
    {
        for (j=0; j<Dx.cols; j++)
        {
            fprintf(dataFile, "%.1f ", Dx.at<float>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Dy outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/3_Dy.txt", "w");
    for (i=0; i<Dy.rows; i++)
    {
        for (j=0; j<Dy.cols; j++)
        {
            fprintf(dataFile, "%.1f ", Dy.at<float>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Magnitudes outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/4_Mag.txt", "w");
    for (i=0; i<Mag.rows; i++)
    {
        for (j=0; j<Mag.cols; j++)
        {
            fprintf(dataFile, "%.2f ", Mag.at<float>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Angle outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/5_Angle.txt", "w");
    for (i=0; i<Angle.rows; i++)
    {
        for (j=0; j<Angle.cols; j++)
        {
            fprintf(dataFile, "%.2f ", Angle.at<float>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Bins outputting
    dataFile = fopen("E:/OpenCVs/HaarTraining/model/6_Bins.txt", "w");
    for (i=0; i<Bins.rows; i++)
    {
        for (j=0; j<Bins.cols; j++)
        {
            fprintf(dataFile, "%d ", Bins.at<char>(i,j));
        }
        fprintf(dataFile, "\n");
    }
    fclose(dataFile);

    //Histogram outputting
    for (ch = 0; ch < nbins; ch++)
    {
        char dirFileName[256] = "E:/OpenCVs/HaarTraining/model/";
        char histFileName[256];
        sprintf(histFileName, "_%d_hist.txt", ch);
        strcat(dirFileName, histFileName);
        dataFile = fopen(dirFileName, "w");
        for (i=0; i<histogram[ch].rows; i++)
        {
            for (j=0; j<histogram[ch].cols; j++)
            {
                fprintf(dataFile, "%.0f ", histogram[ch].at<float>(i,j));
            }
            fprintf(dataFile, "\n");
        }
        fclose(dataFile);
    }

#endif
}
