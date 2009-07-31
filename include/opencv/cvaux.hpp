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

#ifndef __CVAUX_HPP__
#define __CVAUX_HPP__

#ifdef __cplusplus

#include <iosfwd>

/****************************************************************************************\
*                                       CamShiftTracker                                  *
\****************************************************************************************/

class CV_EXPORTS CvCamShiftTracker
{
public:

    CvCamShiftTracker();
    virtual ~CvCamShiftTracker();

    /**** Characteristics of the object that are calculated by track_object method *****/
    float   get_orientation() const // orientation of the object in degrees
    { return m_box.angle; }
    float   get_length() const // the larger linear size of the object
    { return m_box.size.height; }
    float   get_width() const // the smaller linear size of the object
    { return m_box.size.width; }
    CvPoint2D32f get_center() const // center of the object
    { return m_box.center; }
    CvRect get_window() const // bounding rectangle for the object
    { return m_comp.rect; }

    /*********************** Tracking parameters ************************/
    int     get_threshold() const // thresholding value that applied to back project
    { return m_threshold; }

    int     get_hist_dims( int* dims = 0 ) const // returns number of histogram dimensions and sets
    { return m_hist ? cvGetDims( m_hist->bins, dims ) : 0; }

    int     get_min_ch_val( int channel ) const // get the minimum allowed value of the specified channel
    { return m_min_ch_val[channel]; }

    int     get_max_ch_val( int channel ) const // get the maximum allowed value of the specified channel
    { return m_max_ch_val[channel]; }

    // set initial object rectangle (must be called before initial calculation of the histogram)
    bool    set_window( CvRect window)
    { m_comp.rect = window; return true; }

    bool    set_threshold( int threshold ) // threshold applied to the histogram bins
    { m_threshold = threshold; return true; }

    bool    set_hist_bin_range( int dim, int min_val, int max_val );

    bool    set_hist_dims( int c_dims, int* dims );// set the histogram parameters

    bool    set_min_ch_val( int channel, int val ) // set the minimum allowed value of the specified channel
    { m_min_ch_val[channel] = val; return true; }
    bool    set_max_ch_val( int channel, int val ) // set the maximum allowed value of the specified channel
    { m_max_ch_val[channel] = val; return true; }

    /************************ The processing methods *********************************/
    // update object position
    virtual bool  track_object( const IplImage* cur_frame );

    // update object histogram
    virtual bool  update_histogram( const IplImage* cur_frame );

    // reset histogram
    virtual void  reset_histogram();

    /************************ Retrieving internal data *******************************/
    // get back project image
    virtual IplImage* get_back_project()
    { return m_back_project; }

    float query( int* bin ) const
    { return m_hist ? (float)cvGetRealND(m_hist->bins, bin) : 0.f; }

protected:

    // internal method for color conversion: fills m_color_planes group
    virtual void color_transform( const IplImage* img );

    CvHistogram* m_hist;

    CvBox2D    m_box;
    CvConnectedComp m_comp;

    float      m_hist_ranges_data[CV_MAX_DIM][2];
    float*     m_hist_ranges[CV_MAX_DIM];

    int        m_min_ch_val[CV_MAX_DIM];
    int        m_max_ch_val[CV_MAX_DIM];
    int        m_threshold;

    IplImage*  m_color_planes[CV_MAX_DIM];
    IplImage*  m_back_project;
    IplImage*  m_temp;
    IplImage*  m_mask;
};

/****************************************************************************************\
*                                   Adaptive Skin Detector                               *
\****************************************************************************************/

class CV_EXPORTS CvAdaptiveSkinDetector
{
private:
	enum {
		GSD_HUE_LT = 3,
		GSD_HUE_UT = 33,
		GSD_INTENSITY_LT = 15,
		GSD_INTENSITY_UT = 250
	};

	class CV_EXPORTS Histogram
	{
	private:
		enum {
			HistogramSize = (GSD_HUE_UT - GSD_HUE_LT + 1)
		};

	protected:
		int findCoverageIndex(double surfaceToCover, int defaultValue = 0);

	public:
		CvHistogram *fHistogram;
		Histogram();
		virtual ~Histogram();

		void findCurveThresholds(int &x1, int &x2, double percent = 0.05);
		void mergeWith(Histogram *source, double weight);
	};

	int nStartCounter, nFrameCount, nSkinHueLowerBound, nSkinHueUpperBound, nMorphingMethod, nSamplingDivider;
	double fHistogramMergeFactor, fHuePercentCovered;
	Histogram histogramHueMotion, skinHueHistogram;
	IplImage *imgHueFrame, *imgSaturationFrame, *imgLastGrayFrame, *imgMotionFrame, *imgFilteredFrame;
	IplImage *imgShrinked, *imgTemp, *imgGrayFrame, *imgHSVFrame;

protected:
	void initData(IplImage *src, int widthDivider, int heightDivider);
	void adaptiveFilter();

public:

	enum {
		MORPHING_METHOD_NONE = 0,
		MORPHING_METHOD_ERODE = 1,
		MORPHING_METHOD_ERODE_ERODE	= 2,
		MORPHING_METHOD_ERODE_DILATE = 3
	};

	CvAdaptiveSkinDetector(int samplingDivider = 1, int morphingMethod = MORPHING_METHOD_NONE);
	virtual ~CvAdaptiveSkinDetector();

	virtual void process(IplImage *inputBGRImage, IplImage *outputHueMask);
};


/****************************************************************************************\
*                                  Fuzzy MeanShift Tracker                               *
\****************************************************************************************/

class CV_EXPORTS CvFuzzyPoint {
public:
	double x, y, value;

	CvFuzzyPoint(double _x, double _y);
};

class CV_EXPORTS CvFuzzyCurve {
private:
    std::vector<CvFuzzyPoint> points;
	double value, centre;

	bool between(double x, double x1, double x2);

public:
	CvFuzzyCurve();
	~CvFuzzyCurve();

	void setCentre(double _centre);
	double getCentre();
	void clear();
	void addPoint(double x, double y);
	double calcValue(double param);
	double getValue();
	void setValue(double _value);
};

class CV_EXPORTS CvFuzzyFunction {
public:
    std::vector<CvFuzzyCurve> curves;

	CvFuzzyFunction();
	~CvFuzzyFunction();
	void addCurve(CvFuzzyCurve *curve, double value = 0);
	void resetValues();
	double calcValue();
	CvFuzzyCurve *newCurve();
};

class CV_EXPORTS CvFuzzyRule {
private:
	CvFuzzyCurve *fuzzyInput1, *fuzzyInput2;
	CvFuzzyCurve *fuzzyOutput;
public:
	CvFuzzyRule();
	~CvFuzzyRule();
	void setRule(CvFuzzyCurve *c1, CvFuzzyCurve *c2, CvFuzzyCurve *o1);
	double calcValue(double param1, double param2);
	CvFuzzyCurve *getOutputCurve();
};

class CV_EXPORTS CvFuzzyController {
private:
    std::vector<CvFuzzyRule*> rules;
public:
	CvFuzzyController();
	~CvFuzzyController();
	void addRule(CvFuzzyCurve *c1, CvFuzzyCurve *c2, CvFuzzyCurve *o1);
	double calcOutput(double param1, double param2);
};

class CV_EXPORTS CvFuzzyMeanShiftTracker
{
private:
	class FuzzyResizer
	{
	private:
		CvFuzzyFunction iInput, iOutput;
		CvFuzzyController fuzzyController;
	public:
		FuzzyResizer();
		int calcOutput(double edgeDensity, double density);
	};

	class SearchWindow
	{
	public:
		FuzzyResizer *fuzzyResizer;
		int x, y;
		int width, height, maxWidth, maxHeight, ellipseHeight, ellipseWidth;
		int ldx, ldy, ldw, ldh, numShifts, numIters;
		int xGc, yGc;
		long m00, m01, m10, m11, m02, m20;
		double ellipseAngle;
		double density;
		unsigned int depthLow, depthHigh;
		int verticalEdgeLeft, verticalEdgeRight, horizontalEdgeTop, horizontalEdgeBottom;

		SearchWindow();
		~SearchWindow();
		void setSize(int _x, int _y, int _width, int _height);
		void initDepthValues(IplImage *maskImage, IplImage *depthMap);
		bool shift();
		void extractInfo(IplImage *maskImage, IplImage *depthMap, bool initDepth);
		void getResizeAttribsEdgeDensityLinear(int &resizeDx, int &resizeDy, int &resizeDw, int &resizeDh);
		void getResizeAttribsInnerDensity(int &resizeDx, int &resizeDy, int &resizeDw, int &resizeDh);
		void getResizeAttribsEdgeDensityFuzzy(int &resizeDx, int &resizeDy, int &resizeDw, int &resizeDh);
		bool meanShift(IplImage *maskImage, IplImage *depthMap, int maxIteration, bool initDepth);
	};

public:
	enum TrackingState
	{
		tsNone 			= 0,
		tsSearching 	= 1,
		tsTracking 		= 2,
		tsSetWindow 	= 3,
		tsDisabled		= 10
	};

	enum ResizeMethod {
		rmEdgeDensityLinear		= 0,
		rmEdgeDensityFuzzy		= 1,
		rmInnerDensity			= 2
	};

	enum {
		MinKernelMass			= 1000
	};

	SearchWindow kernel;
	int searchMode;

private:
	enum
	{
		MaxMeanShiftIteration 	= 5,
		MaxSetSizeIteration 	= 5
	};

	void findOptimumSearchWindow(SearchWindow &searchWindow, IplImage *maskImage, IplImage *depthMap, int maxIteration, int resizeMethod, bool initDepth);

public:
	CvFuzzyMeanShiftTracker();
	~CvFuzzyMeanShiftTracker();

	void track(IplImage *maskImage, IplImage *depthMap, int resizeMethod, bool resetSearch, int minKernelMass = MinKernelMass);
};


namespace cv
{

class CV_EXPORTS OctTree
{
public:    
    struct Node
    {
        Node() {}
        int begin, end;
        float x_min, x_max, y_min, y_max, z_min, z_max;		
        int maxLevels;
        bool isLeaf;
        int children[8];
    };

    OctTree();
    OctTree( const Vector<Point3f>& points, int maxLevels = 10, int minPoints = 20 );
    virtual ~OctTree();

    virtual void buildTree( const Vector<Point3f>& points, int maxLevels = 10, int minPoints = 20 );
    virtual void getPointsWithinSphere( const Point3f& center, float radius,
                                        Vector<Point3f>& points ) const;
    const Vector<Node>& getNodes() const { return nodes; }
private:
    int minPoints;
    Vector<Point3f> points;
    Vector<Node> nodes;
	
	virtual void buildNext(size_t node_ind);
};


class CV_EXPORTS Mesh3D
{
public:
    struct EmptyMeshException {};

    Mesh3D();
    Mesh3D(const Vector<Point3f>& vtx);
    ~Mesh3D();

    void buildOctTree();
    void clearOctTree();
    float estimateResolution(float tryRatio = 0.1f);        
    void computeNormals(float normalRadius, int minNeighbors = 20);
    void computeNormals(const Vector<int>& subset, float normalRadius, int minNeighbors = 20);
    
    void writeAsVrml(const String& file, const Vector<Scalar>& colors = Vector<Scalar>()) const;
    
    Vector<Point3f> vtx;
    Vector<Point3f> normals;
    float resolution;    
    OctTree octree;

    const static Point3f allzero;
};

class CV_EXPORTS SpinImageModel
{
public:
    
    /* model parameters, leave unset for default or auto estimate */
    float normalRadius;
    int minNeighbors;

    float binSize;
    int imageWidth;

    float lambda;                        
    float gamma;
    float Tgc;

    /* public interface */
    SpinImageModel();
    explicit SpinImageModel(const Mesh3D& mesh);
    ~SpinImageModel();

    void setLogger(std::ostream* log);
    void selectRandomSubset(float ratio);         
    void compute();

    Vector< Vector< Vec2i > > match(const SpinImageModel& scene); 

    Mat packRandomScaledSpins(bool separateScale = false, size_t xCount = 10, size_t yCount = 10) const;
    
    size_t getSpinCount() const { return spinImages.rows; }
    Mat getSpinImage(size_t index) const { return spinImages.row(index); }
    const Point3f& getSpinVertex(size_t index) const { return mesh.vtx[subset[index]]; }
    const Point3f& getSpinNormal(size_t index) const { return mesh.normals[subset[index]]; }

    const Mesh3D& getMesh() const { return mesh; }

    /* static utility functions */
    static bool spinCorrelation(const Mat& spin1, const Mat& spin2, float lambda, float& result);

    static Point2f calcSpinMapCoo(const Point3f& point, const Point3f& vertex, const Point3f& normal);

    static float geometricConsistency(const Point3f& pointScene1, const Point3f& normalScene1,
                                      const Point3f& pointModel1, const Point3f& normalModel1,   
                                      const Point3f& pointScene2, const Point3f& normalScene2,                               
                                      const Point3f& pointModel2, const Point3f& normalModel2);

    static float groupingCreteria(const Point3f& pointScene1, const Point3f& normalScene1,
                                  const Point3f& pointModel1, const Point3f& normalModel1,
                                  const Point3f& pointScene2, const Point3f& normalScene2,                               
                                  const Point3f& pointModel2, const Point3f& normalModel2, 
                                  float gamma);
protected:       
    void defaultParams();

    void matchSpinToModel(const Mat& spin, Vector<int>& indeces, 
        Vector<float>& corrCoeffs, bool useExtremeOutliers = true) const; 

    void repackSpinImages(const Vector<uchar>& mask, Mat& spinImages, bool reAlloc = true) const;
             
    Vector<int> subset;
    Mesh3D mesh;
    Mat spinImages;
    std::ostream* out;
};

class CV_EXPORTS TickMeter
{
public:
    TickMeter();
    void start();    
    void stop();

    int64 getTimeTicks() const;
    double getTimeMicro() const;
    double getTimeMilli() const;
    double getTimeSec()   const;
    int64 getCounter() const;

    void reset();
private:
    int64 counter;
    int64 sumTime;
    int64 startTime;
};

CV_EXPORTS std::ostream& operator<<(std::ostream& out, const TickMeter& tm);

/****************************************************************************************\
*            HOG (Histogram-of-Oriented-Gradients) Descriptor and Object Detector        *
\****************************************************************************************/

struct CV_EXPORTS HOGDescriptor
{
public:
    enum { L2Hys=0 };

    HOGDescriptor() : winSize(64,128), blockSize(16,16), blockStride(8,8),
        cellSize(8,8), nbins(9), derivAperture(1), winSigma(-1),
        histogramNormType(L2Hys), L2HysThreshold(0.2), gammaCorrection(true)
    {}

    HOGDescriptor(Size _winSize, Size _blockSize, Size _blockStride,
        Size _cellSize, int _nbins, int _derivAperture=1, double _winSigma=-1,
        int _histogramNormType=L2Hys, double _L2HysThreshold=0.2, bool _gammaCorrection=false)
        : winSize(_winSize), blockSize(_blockSize), blockStride(_blockStride), cellSize(_cellSize),
        nbins(_nbins), derivAperture(_derivAperture), winSigma(_winSigma),
        histogramNormType(_histogramNormType), L2HysThreshold(_L2HysThreshold),
        gammaCorrection(_gammaCorrection)
    {}

    HOGDescriptor(const String& filename)
    {
        load(filename);
    }

    virtual ~HOGDescriptor() {}

    size_t getDescriptorSize() const;
    bool checkDetectorSize() const;
    double getWinSigma() const;

    virtual void setSVMDetector(const Vector<float>& _svmdetector);

    virtual bool load(const String& filename, const String& objname=String());
    virtual void save(const String& filename, const String& objname=String()) const;

    virtual void compute(const Mat& img,
                         Vector<float>& descriptors,
                         Size winStride=Size(), Size padding=Size(),
                         const Vector<Point>& locations=Vector<Point>()) const;
    virtual void detect(const Mat& img, Vector<Point>& foundLocations,
                        double hitThreshold=0, Size winStride=Size(),
                        Size padding=Size(),
                        const Vector<Point>& searchLocations=Vector<Point>()) const;
    virtual void detectMultiScale(const Mat& img, Vector<Rect>& foundLocations,
                                  double hitThreshold=0, Size winStride=Size(),
                                  Size padding=Size(), double scale=1.05,
                                  int groupThreshold=2) const;
    virtual void computeGradient(const Mat& img, Mat& grad, Mat& angleOfs,
                                 Size paddingTL=Size(), Size paddingBR=Size()) const;
    virtual void normalizeBlockHistogram(Vector<float>& histogram) const;

    static Vector<float> getDefaultPeopleDetector();

    Size winSize;
    Size blockSize;
    Size blockStride;
    Size cellSize;
    int nbins;
    int derivAperture;
    double winSigma;
    int histogramNormType;
    double L2HysThreshold;
    bool gammaCorrection;
    Vector<float> svmDetector;
};


class CV_EXPORTS SelfSimDescriptor
{
public:
    SelfSimDescriptor();
    SelfSimDescriptor(int _ssize, int _lsize,
        int _startDistanceBucket=DEFAULT_START_DISTANCE_BUCKET,
        int _numberOfDistanceBuckets=DEFAULT_NUM_DISTANCE_BUCKETS,
        int _nangles=DEFAULT_NUM_ANGLES);
	SelfSimDescriptor(const SelfSimDescriptor& ss);
	virtual ~SelfSimDescriptor();
    SelfSimDescriptor& operator = (const SelfSimDescriptor& ss);

    size_t getDescriptorSize() const;
    Size getGridSize( Size imgsize, Size winStride ) const;

    virtual void compute(const Mat& img, Vector<float>& descriptors, Size winStride=Size(),
                         const Vector<Point>& locations=Vector<Point>()) const;
    virtual void computeLogPolarMapping(Mat& mappingMask) const;
    virtual void SSD(const Mat& img, Point pt, Mat& ssd) const;

	int smallSize;
	int largeSize;
    int startDistanceBucket;
    int numberOfDistanceBuckets;
    int numberOfAngles;

    enum { DEFAULT_SMALL_SIZE = 5, DEFAULT_LARGE_SIZE = 41,
        DEFAULT_NUM_ANGLES = 20, DEFAULT_START_DISTANCE_BUCKET = 3,
        DEFAULT_NUM_DISTANCE_BUCKETS = 7 };
};

    
class CV_EXPORTS PatchGenerator
{
public:
    PatchGenerator();
    PatchGenerator(double _backgroundMin, double _backgroundMax,
                   double _noiseRange, bool _randomBlur=true,
                   double _lambdaMin=0.6, double _lambdaMax=1.5,
                   double _thetaMin=-CV_PI, double _thetaMax=CV_PI,
                   double _phiMin=-CV_PI, double _phiMax=CV_PI );
    void operator()(const Mat& image, Point2f pt, Mat& patch, Size patchSize, RNG& rng) const;
    void operator()(const Mat& image, const Mat& transform, Mat& patch,
                    Size patchSize, RNG& rng) const;
    void warpWholeImage(const Mat& image, Mat& _T, Mat& buf,
                        Mat& warped, int border, RNG& rng) const;
    void generateRandomTransform(Point2f srcCenter, Point2f dstCenter,
                                 Mat& transform, RNG& rng, bool inverse=false) const;
    double backgroundMin, backgroundMax;
    double noiseRange;
    bool randomBlur;
    double lambdaMin, lambdaMax;
    double thetaMin, thetaMax;
    double phiMin, phiMax;
};


class CV_EXPORTS LDetector
{
public:    
    LDetector();
    LDetector(int _radius, int _threshold, int _nOctaves,
              int _nViews, double _baseFeatureSize, double _clusteringDistance);
    void operator()(const Mat& image, Vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void operator()(const Vector<Mat>& pyr, Vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void getMostStable2D(const Mat& image, Vector<KeyPoint>& keypoints,
                         int maxCount, const PatchGenerator& patchGenerator) const;
    void setVerbose(bool verbose);
    
    void read(const FileNode& node);
    void write(FileStorage& fs, const String& name=String()) const;
    
    int radius;
    int threshold;
    int nOctaves;
    int nViews;
    bool verbose;
    
    double baseFeatureSize;
    double clusteringDistance;
};


class CV_EXPORTS FernClassifier
{
public:
    FernClassifier();
    FernClassifier(const FileNode& node);
    FernClassifier(const Vector<Point2f>& points,
                   const Vector<Ptr<Mat> >& refimgs,
                   const Vector<int>& labels=Vector<int>(),
                   int _nclasses=0, int _patchSize=PATCH_SIZE,
                   int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                   int _nstructs=DEFAULT_STRUCTS,
                   int _structSize=DEFAULT_STRUCT_SIZE,
                   int _nviews=DEFAULT_VIEWS,
                   int _compressionMethod=COMPRESSION_NONE,
                   const PatchGenerator& patchGenerator=PatchGenerator());
    virtual ~FernClassifier();
    virtual void read(const FileNode& n);
    virtual void write(FileStorage& fs, const String& name=String()) const;
    virtual void trainFromSingleView(const Mat& image,
                                     const Vector<KeyPoint>& keypoints,
                                     int _patchSize=PATCH_SIZE,
                                     int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                                     int _nstructs=DEFAULT_STRUCTS,
                                     int _structSize=DEFAULT_STRUCT_SIZE,
                                     int _nviews=DEFAULT_VIEWS,
                                     int _compressionMethod=COMPRESSION_NONE,
                                     const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const Vector<Point2f>& points,
                       const Vector<Ptr<Mat> >& refimgs,
                       const Vector<int>& labels=Vector<int>(),
                       int _nclasses=0, int _patchSize=PATCH_SIZE,
                       int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                       int _nstructs=DEFAULT_STRUCTS,
                       int _structSize=DEFAULT_STRUCT_SIZE,
                       int _nviews=DEFAULT_VIEWS,
                       int _compressionMethod=COMPRESSION_NONE,
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual int operator()(const Mat& img, Point2f kpt, Vector<float>& signature) const;
    virtual int operator()(const Mat& patch, Vector<float>& signature) const;
    virtual void clear();
    void setVerbose(bool verbose);
    
    int getClassCount() const;
    int getStructCount() const;
    int getStructSize() const;
    int getSignatureSize() const;
    int getCompressionMethod() const;
    Size getPatchSize() const;    
    
    struct Feature
    {
        uchar x1, y1, x2, y2;
        Feature() : x1(0), y1(0), x2(0), y2(0) {}
        Feature(int _x1, int _y1, int _x2, int _y2)
        : x1((uchar)_x1), y1((uchar)_y1), x2((uchar)_x2), y2((uchar)_y2)
        {}
        template<typename _Tp> bool operator ()(const Mat_<_Tp>& patch) const
        { return patch(y1,x1) > patch(y2, x2); }
    };
    
    enum
    {
        PATCH_SIZE = 31,
        DEFAULT_STRUCTS = 50,
        DEFAULT_STRUCT_SIZE = 9,
        DEFAULT_VIEWS = 5000,
        DEFAULT_SIGNATURE_SIZE = 176,
        COMPRESSION_NONE = 0,
        COMPRESSION_RANDOM_PROJ = 1,
        COMPRESSION_PCA = 2,
        DEFAULT_COMPRESSION_METHOD = COMPRESSION_NONE
    };
    
protected:
    virtual void prepare(int _nclasses, int _patchSize, int _signatureSize,
                         int _nstructs, int _structSize,
                         int _nviews, int _compressionMethod);
    virtual void finalize(RNG& rng);
    virtual int getLeaf(int fidx, const Mat& patch) const;
    
    bool verbose;
    int nstructs;
    int structSize;
    int nclasses;
    int signatureSize;
    int compressionMethod;
    int leavesPerStruct;
    Size patchSize;
    Vector<Feature> features;
    Vector<int> classCounters;
    Vector<float> posteriors;
};

class CV_EXPORTS PlanarObjectDetector
{
public:
    PlanarObjectDetector();
    PlanarObjectDetector(const FileNode& node);
    PlanarObjectDetector(const Vector<Mat>& pyr, int _npoints=300,
                         int _patchSize=FernClassifier::PATCH_SIZE,
                         int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                         int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                         int _nviews=FernClassifier::DEFAULT_VIEWS,
                         const LDetector& detector=LDetector(),
                         const PatchGenerator& patchGenerator=PatchGenerator()); 
    virtual ~PlanarObjectDetector();
    virtual void train(const Vector<Mat>& pyr, int _npoints=300,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const Vector<Mat>& pyr, const Vector<KeyPoint>& keypoints,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    Rect getModelROI() const;
    Vector<KeyPoint> getModelPoints() const;
    const LDetector& getDetector() const;
    const FernClassifier& getClassifier() const;
    void setVerbose(bool verbose);
    
    void read(const FileNode& node);
    void write(FileStorage& fs, const String& name=String()) const;
    bool operator()(const Mat& image, Mat& H, Vector<Point2f>& corners) const;
    bool operator()(const Vector<Mat>& pyr, const Vector<KeyPoint>& keypoints,
                    Mat& H, Vector<Point2f>& corners, Vector<int>* pairs=0) const;
    
protected:
    bool verbose;
    Rect modelROI;
    Vector<KeyPoint> modelPoints;
    LDetector ldetector;
    FernClassifier fernClassifier;
};
    
}

#endif /* __cplusplus */

#endif /* __CVAUX_HPP__ */

/* End of file. */
