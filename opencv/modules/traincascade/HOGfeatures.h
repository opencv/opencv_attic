#ifndef _OPENCV_HOGFEATURES_H_
#define _OPENCV_HOGFEATURES_H_

#include "traincascade_features.h"

//#define TEST_INTHIST_BUILD
//#define TEST_FEAT_CALC

#define N_BINS 9
#define N_CELLS 4

#define HOGF_NAME "HOGFeatureParams"
struct CvHOGFeatureParams : public CvFeatureParams
{
    CvHOGFeatureParams(); 
};

class CvHOGEvaluator : public CvFeatureEvaluator
{
public:
    virtual ~CvHOGEvaluator() {}
    virtual void init(const CvFeatureParams *_featureParams,
        int _maxSampleCount, Size _winSize );
    virtual void setImage(const Mat& img, uchar clsLabel, int idx);
    virtual void integralHistogram( const Mat& srcImage, vector<Mat> &histogram, int nbins ) const;
    virtual float operator()(int varIdx, int sampleIdx) const;
    virtual void writeFeatures( FileStorage &fs, const Mat& featureMap ) const;
protected:
    virtual void generateFeatures();

    class Feature
    {
    public:
        Feature();
        Feature( int offset, int x, int y, int cellW, int cellH ); 
        float calc( const vector<Mat> &_hists, size_t y ) const;
        float calc( const vector<Mat> &_hists, size_t y, int featComponent ) const;
        void write( FileStorage &fs ) const;
        void write( FileStorage &fs, int varIdx ) const;

        Rect rect[N_CELLS]; //cells

        struct
        {
            int p0, p1, p2, p3;
        } fastRect[N_CELLS];
    };
    vector<Feature> features;

    vector<Mat> hist;
};

inline float CvHOGEvaluator::operator()(int varIdx, int sampleIdx) const
{
    int featureIdx = varIdx / (N_BINS * N_CELLS);
    int componentIdx = varIdx % (N_BINS * N_CELLS);
    return features[featureIdx].calc( hist, sampleIdx, componentIdx); 
}

inline float CvHOGEvaluator::Feature::calc(const vector<Mat> &_hists, size_t y) const
{
    int vecSize = N_CELLS * N_BINS;
    float *res = new float[vecSize];
    float sum = 0.f;

    for (int bin = 0; bin < N_BINS; bin++)
    {
        const float* hist = _hists[bin].ptr<float>(y);
        for (int cell = 0; cell < N_CELLS; cell++)
        {
            res[9*cell + bin] = hist[fastRect[cell].p0] - hist[fastRect[cell].p1] - hist[fastRect[cell].p2] + hist[fastRect[cell].p3];
            sum += res[9*cell + bin];
        }
    }

#ifdef TEST_FEAT_CALC
    string fileName = "E:/OpenCVs/HaarTraining/model/7_featureVectors.txt";
    FILE* featureData = fopen(fileName.c_str(), "w");
    for (int i = 0; i < vecSize; i++)
    {
        fprintf(featureData, "feature[%d] %.0f \n", i, res[i]);
    }
    fclose(featureData);
#endif

    //L1 - normalization
    for (int i = 0; i < vecSize; i++)
    {
        res[i] /= (sum + 0.001f);
    }

    float tt = res[0];
    delete [] res;
    return tt;
}

inline float CvHOGEvaluator::Feature::calc( const vector<Mat> & _hists, size_t y, int featComponent ) const
{
    int vecSize = N_CELLS * N_BINS;
    float *res = new float[vecSize];
    float sum = 0.f;
    float featVal;

    for (int bin = 0; bin < N_BINS; bin++)
    {
        const float* hist = _hists[bin].ptr<float>(y);
        for (int cell = 0; cell < N_CELLS; cell++)
        {
            res[9*cell + bin] = hist[fastRect[cell].p0] - hist[fastRect[cell].p1] - hist[fastRect[cell].p2] + hist[fastRect[cell].p3];
            sum += res[9*cell + bin];
        }
    }
    //L1 - normalization
    for (int i = 0; i < vecSize; i++)
    {
        res[i] /= (sum + 0.001f);
    }

    featVal = res[featComponent];
    delete [] res;
    return featVal;
}

#endif // _OPENCV_HOGFEATURES_H_
