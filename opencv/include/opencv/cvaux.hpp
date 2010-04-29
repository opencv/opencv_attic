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

#ifndef __OPENCV_AUX_HPP__
#define __OPENCV_AUX_HPP__

#ifdef __cplusplus

#include <iosfwd>
#include <limits>

class CV_EXPORTS CvImage
{
public:
    CvImage() : image(0), refcount(0) {}
    CvImage( CvSize size, int depth, int channels )
    {
        image = cvCreateImage( size, depth, channels );
        refcount = image ? new int(1) : 0;
    }

    CvImage( IplImage* img ) : image(img)
    {
        refcount = image ? new int(1) : 0;
    }

    CvImage( const CvImage& img ) : image(img.image), refcount(img.refcount)
    {
        if( refcount ) ++(*refcount);
    }

    CvImage( const char* filename, const char* imgname=0, int color=-1 ) : image(0), refcount(0)
    { load( filename, imgname, color ); }

    CvImage( CvFileStorage* fs, const char* mapname, const char* imgname ) : image(0), refcount(0)
    { read( fs, mapname, imgname ); }

    CvImage( CvFileStorage* fs, const char* seqname, int idx ) : image(0), refcount(0)
    { read( fs, seqname, idx ); }

    ~CvImage()
    {
        if( refcount && !(--*refcount) )
        {
            cvReleaseImage( &image );
            delete refcount;
        }
    }

    CvImage clone() { return CvImage(image ? cvCloneImage(image) : 0); }

    void create( CvSize size, int depth, int channels )
    {
        if( !image || !refcount ||
            image->width != size.width || image->height != size.height ||
            image->depth != depth || image->nChannels != channels )
            attach( cvCreateImage( size, depth, channels ));
    }

    void release() { detach(); }
    void clear() { detach(); }

    void attach( IplImage* img, bool use_refcount=true )
    {
        if( refcount && --*refcount == 0 )
        {
            cvReleaseImage( &image );
            delete refcount;
        }
        image = img;
        refcount = use_refcount && image ? new int(1) : 0;
    }

    void detach()
    {
        if( refcount && --*refcount == 0 )
        {
            cvReleaseImage( &image );
            delete refcount;
        }
        image = 0;
        refcount = 0;
    }

    bool load( const char* filename, const char* imgname=0, int color=-1 );
    bool read( CvFileStorage* fs, const char* mapname, const char* imgname );
    bool read( CvFileStorage* fs, const char* seqname, int idx );
    void save( const char* filename, const char* imgname, const int* params=0 );
    void write( CvFileStorage* fs, const char* imgname );

    void show( const char* window_name );
    bool is_valid() { return image != 0; }

    int width() const { return image ? image->width : 0; }
    int height() const { return image ? image->height : 0; }

    CvSize size() const { return image ? cvSize(image->width, image->height) : cvSize(0,0); }

    CvSize roi_size() const
    {
        return !image ? cvSize(0,0) :
            !image->roi ? cvSize(image->width,image->height) :
            cvSize(image->roi->width, image->roi->height);
    }

    CvRect roi() const
    {
        return !image ? cvRect(0,0,0,0) :
            !image->roi ? cvRect(0,0,image->width,image->height) :
            cvRect(image->roi->xOffset,image->roi->yOffset,
                   image->roi->width,image->roi->height);
    }

    int coi() const { return !image || !image->roi ? 0 : image->roi->coi; }

    void set_roi(CvRect roi) { cvSetImageROI(image,roi); }
    void reset_roi() { cvResetImageROI(image); }
    void set_coi(int coi) { cvSetImageCOI(image,coi); }
    int depth() const { return image ? image->depth : 0; }
    int channels() const { return image ? image->nChannels : 0; }
    int pix_size() const { return image ? ((image->depth & 255)>>3)*image->nChannels : 0; }

    uchar* data() { return image ? (uchar*)image->imageData : 0; }
    const uchar* data() const { return image ? (const uchar*)image->imageData : 0; }
    int step() const { return image ? image->widthStep : 0; }
    int origin() const { return image ? image->origin : 0; }

    uchar* roi_row(int y)
    {
        assert(0<=y);
        assert(!image ?
                1 : image->roi ?
                y<image->roi->height : y<image->height);

        return !image ? 0 :
            !image->roi ?
                (uchar*)(image->imageData + y*image->widthStep) :
                (uchar*)(image->imageData + (y+image->roi->yOffset)*image->widthStep +
                image->roi->xOffset*((image->depth & 255)>>3)*image->nChannels);
    }

    const uchar* roi_row(int y) const
    {
        assert(0<=y);
        assert(!image ?
                1 : image->roi ?
                y<image->roi->height : y<image->height);

        return !image ? 0 :
            !image->roi ?
                (const uchar*)(image->imageData + y*image->widthStep) :
                (const uchar*)(image->imageData + (y+image->roi->yOffset)*image->widthStep +
                image->roi->xOffset*((image->depth & 255)>>3)*image->nChannels);
    }

    operator const IplImage* () const { return image; }
    operator IplImage* () { return image; }

    CvImage& operator = (const CvImage& img)
    {
        if( img.refcount )
            ++*img.refcount;
        if( refcount && !(--*refcount) )
            cvReleaseImage( &image );
        image=img.image;
        refcount=img.refcount;
        return *this;
    }

protected:
    IplImage* image;
    int* refcount;
};


class CV_EXPORTS CvMatrix
{
public:
    CvMatrix() : matrix(0) {}
    CvMatrix( int rows, int cols, int type )
    { matrix = cvCreateMat( rows, cols, type ); }

    CvMatrix( int rows, int cols, int type, CvMat* hdr,
              void* data=0, int step=CV_AUTOSTEP )
    { matrix = cvInitMatHeader( hdr, rows, cols, type, data, step ); }

    CvMatrix( int rows, int cols, int type, CvMemStorage* storage, bool alloc_data=true );

    CvMatrix( int rows, int cols, int type, void* data, int step=CV_AUTOSTEP )
    { matrix = cvCreateMatHeader( rows, cols, type );
      cvSetData( matrix, data, step ); }

    CvMatrix( CvMat* m )
    { matrix = m; }

    CvMatrix( const CvMatrix& m )
    {
        matrix = m.matrix;
        addref();
    }

    CvMatrix( const char* filename, const char* matname=0, int color=-1 ) : matrix(0)
    {  load( filename, matname, color ); }

    CvMatrix( CvFileStorage* fs, const char* mapname, const char* matname ) : matrix(0)
    {  read( fs, mapname, matname ); }

    CvMatrix( CvFileStorage* fs, const char* seqname, int idx ) : matrix(0)
    {  read( fs, seqname, idx ); }

    ~CvMatrix()
    {
        release();
    }

    CvMatrix clone() { return CvMatrix(matrix ? cvCloneMat(matrix) : 0); }

    void set( CvMat* m, bool add_ref )
    {
        release();
        matrix = m;
        if( add_ref )
            addref();
    }

    void create( int rows, int cols, int type )
    {
        if( !matrix || !matrix->refcount ||
            matrix->rows != rows || matrix->cols != cols ||
            CV_MAT_TYPE(matrix->type) != type )
            set( cvCreateMat( rows, cols, type ), false );
    }

    void addref() const
    {
        if( matrix )
        {
            if( matrix->hdr_refcount )
                ++matrix->hdr_refcount;
            else if( matrix->refcount )
                ++*matrix->refcount;
        }
    }

    void release()
    {
        if( matrix )
        {
            if( matrix->hdr_refcount )
            {
                if( --matrix->hdr_refcount == 0 )
                    cvReleaseMat( &matrix );
            }
            else if( matrix->refcount )
            {
                if( --*matrix->refcount == 0 )
                    cvFree( &matrix->refcount );
            }
            matrix = 0;
        }
    }

    void clear()
    {
        release();
    }

    bool load( const char* filename, const char* matname=0, int color=-1 );
    bool read( CvFileStorage* fs, const char* mapname, const char* matname );
    bool read( CvFileStorage* fs, const char* seqname, int idx );
    void save( const char* filename, const char* matname, const int* params=0 );
    void write( CvFileStorage* fs, const char* matname );

    void show( const char* window_name );

    bool is_valid() { return matrix != 0; }

    int rows() const { return matrix ? matrix->rows : 0; }
    int cols() const { return matrix ? matrix->cols : 0; }

    CvSize size() const
    {
        return !matrix ? cvSize(0,0) : cvSize(matrix->rows,matrix->cols);
    }

    int type() const { return matrix ? CV_MAT_TYPE(matrix->type) : 0; }
    int depth() const { return matrix ? CV_MAT_DEPTH(matrix->type) : 0; }
    int channels() const { return matrix ? CV_MAT_CN(matrix->type) : 0; }
    int pix_size() const { return matrix ? CV_ELEM_SIZE(matrix->type) : 0; }

    uchar* data() { return matrix ? matrix->data.ptr : 0; }
    const uchar* data() const { return matrix ? matrix->data.ptr : 0; }
    int step() const { return matrix ? matrix->step : 0; }

    void set_data( void* data, int step=CV_AUTOSTEP )
    { cvSetData( matrix, data, step ); }

    uchar* row(int i) { return !matrix ? 0 : matrix->data.ptr + i*matrix->step; }
    const uchar* row(int i) const
    { return !matrix ? 0 : matrix->data.ptr + i*matrix->step; }

    operator const CvMat* () const { return matrix; }
    operator CvMat* () { return matrix; }

    CvMatrix& operator = (const CvMatrix& _m)
    {
        _m.addref();
        release();
        matrix = _m.matrix;
        return *this;
    }

protected:
    CvMat* matrix;
};

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

class CV_EXPORTS Octree
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

    Octree();
    Octree( const vector<Point3f>& points, int maxLevels = 10, int minPoints = 20 );
    virtual ~Octree();

    virtual void buildTree( const vector<Point3f>& points, int maxLevels = 10, int minPoints = 20 );
    virtual void getPointsWithinSphere( const Point3f& center, float radius,
                                        vector<Point3f>& points ) const;
    const vector<Node>& getNodes() const { return nodes; }
private:
    int minPoints;
    vector<Point3f> points;
    vector<Node> nodes;

	virtual void buildNext(size_t node_ind);
};


class CV_EXPORTS Mesh3D
{
public:
    struct EmptyMeshException {};

    Mesh3D();
    Mesh3D(const vector<Point3f>& vtx);
    ~Mesh3D();

    void buildOctree();
    void clearOctree();
    float estimateResolution(float tryRatio = 0.1f);
    void computeNormals(float normalRadius, int minNeighbors = 20);
    void computeNormals(const vector<int>& subset, float normalRadius, int minNeighbors = 20);

    void writeAsVrml(const String& file, const vector<Scalar>& colors = vector<Scalar>()) const;

    vector<Point3f> vtx;
    vector<Point3f> normals;
    float resolution;
    Octree octree;

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

    float T_GeometriccConsistency;
    float T_GroupingCorespondances;

    /* public interface */
    SpinImageModel();
    explicit SpinImageModel(const Mesh3D& mesh);
    ~SpinImageModel();

    void setLogger(std::ostream* log);
    void selectRandomSubset(float ratio);
    void setSubset(const vector<int>& subset);
    void compute();

    void match(const SpinImageModel& scene, vector< vector<Vec2i> >& result);

    Mat packRandomScaledSpins(bool separateScale = false, size_t xCount = 10, size_t yCount = 10) const;

    size_t getSpinCount() const { return spinImages.rows; }
    Mat getSpinImage(size_t index) const { return spinImages.row(index); }
    const Point3f& getSpinVertex(size_t index) const { return mesh.vtx[subset[index]]; }
    const Point3f& getSpinNormal(size_t index) const { return mesh.normals[subset[index]]; }

    const Mesh3D& getMesh() const { return mesh; }
    Mesh3D& getMesh() { return mesh; }

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

    void matchSpinToModel(const Mat& spin, vector<int>& indeces,
        vector<float>& corrCoeffs, bool useExtremeOutliers = true) const;

    void repackSpinImages(const vector<uchar>& mask, Mat& spinImages, bool reAlloc = true) const;

    vector<int> subset;
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

    virtual void setSVMDetector(const vector<float>& _svmdetector);

    virtual bool load(const String& filename, const String& objname=String());
    virtual void save(const String& filename, const String& objname=String()) const;

    virtual void compute(const Mat& img,
                         vector<float>& descriptors,
                         Size winStride=Size(), Size padding=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
    virtual void detect(const Mat& img, vector<Point>& foundLocations,
                        double hitThreshold=0, Size winStride=Size(),
                        Size padding=Size(),
                        const vector<Point>& searchLocations=vector<Point>()) const;
    virtual void detectMultiScale(const Mat& img, vector<Rect>& foundLocations,
                                  double hitThreshold=0, Size winStride=Size(),
                                  Size padding=Size(), double scale=1.05,
                                  int groupThreshold=2) const;
    virtual void computeGradient(const Mat& img, Mat& grad, Mat& angleOfs,
                                 Size paddingTL=Size(), Size paddingBR=Size()) const;

    static vector<float> getDefaultPeopleDetector();

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
    vector<float> svmDetector;
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

    virtual void compute(const Mat& img, vector<float>& descriptors, Size winStride=Size(),
                         const vector<Point>& locations=vector<Point>()) const;
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
    void warpWholeImage(const Mat& image, Mat& matT, Mat& buf,
                        Mat& warped, int border, RNG& rng) const;
    void generateRandomTransform(Point2f srcCenter, Point2f dstCenter,
                                 Mat& transform, RNG& rng, bool inverse=false) const;
	 void setAffineParam(double lambda, double theta, double phi);

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
    void operator()(const Mat& image, vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void operator()(const vector<Mat>& pyr, vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void getMostStable2D(const Mat& image, vector<KeyPoint>& keypoints,
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

typedef LDetector YAPE;

class CV_EXPORTS FernClassifier
{
public:
    FernClassifier();
    FernClassifier(const FileNode& node);
    FernClassifier(const vector<Point2f>& points,
                   const vector<Ptr<Mat> >& refimgs,
                   const vector<int>& labels=vector<int>(),
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
                                     const vector<KeyPoint>& keypoints,
                                     int _patchSize=PATCH_SIZE,
                                     int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                                     int _nstructs=DEFAULT_STRUCTS,
                                     int _structSize=DEFAULT_STRUCT_SIZE,
                                     int _nviews=DEFAULT_VIEWS,
                                     int _compressionMethod=COMPRESSION_NONE,
                                     const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const vector<Point2f>& points,
                       const vector<Ptr<Mat> >& refimgs,
                       const vector<int>& labels=vector<int>(),
                       int _nclasses=0, int _patchSize=PATCH_SIZE,
                       int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                       int _nstructs=DEFAULT_STRUCTS,
                       int _structSize=DEFAULT_STRUCT_SIZE,
                       int _nviews=DEFAULT_VIEWS,
                       int _compressionMethod=COMPRESSION_NONE,
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual int operator()(const Mat& img, Point2f kpt, vector<float>& signature) const;
    virtual int operator()(const Mat& patch, vector<float>& signature) const;
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
    vector<Feature> features;
    vector<int> classCounters;
    vector<float> posteriors;
};

class CV_EXPORTS PlanarObjectDetector
{
public:
    PlanarObjectDetector();
    PlanarObjectDetector(const FileNode& node);
    PlanarObjectDetector(const vector<Mat>& pyr, int _npoints=300,
                         int _patchSize=FernClassifier::PATCH_SIZE,
                         int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                         int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                         int _nviews=FernClassifier::DEFAULT_VIEWS,
                         const LDetector& detector=LDetector(),
                         const PatchGenerator& patchGenerator=PatchGenerator());
    virtual ~PlanarObjectDetector();
    virtual void train(const vector<Mat>& pyr, int _npoints=300,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const vector<Mat>& pyr, const vector<KeyPoint>& keypoints,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    Rect getModelROI() const;
    vector<KeyPoint> getModelPoints() const;
    const LDetector& getDetector() const;
    const FernClassifier& getClassifier() const;
    void setVerbose(bool verbose);

    void read(const FileNode& node);
    void write(FileStorage& fs, const String& name=String()) const;
    bool operator()(const Mat& image, Mat& H, vector<Point2f>& corners) const;
    bool operator()(const vector<Mat>& pyr, const vector<KeyPoint>& keypoints,
                    Mat& H, vector<Point2f>& corners, vector<int>* pairs=0) const;

protected:
    bool verbose;
    Rect modelROI;
    vector<KeyPoint> modelPoints;
    LDetector ldetector;
    FernClassifier fernClassifier;
};




// detect corners using FAST algorithm
CV_EXPORTS void FAST( const Mat& image, vector<KeyPoint>& keypoints, int threshold, bool nonmax_supression=true );


class CV_EXPORTS LevMarqSparse
{
public:
    LevMarqSparse();
    LevMarqSparse(int npoints, // number of points
            int ncameras, // number of cameras
            int nPointParams, // number of params per one point  (3 in case of 3D points)
            int nCameraParams, // number of parameters per one camera
            int nErrParams, // number of parameters in measurement vector
                            // for 1 point at one camera (2 in case of 2D projections)
            Mat& visibility, // visibility matrix. rows correspond to points, columns correspond to cameras
                             // 1 - point is visible for the camera, 0 - invisible
            Mat& P0, // starting vector of parameters, first cameras then points
            Mat& X, // measurements, in order of visibility. non visible cases are skipped
            TermCriteria criteria, // termination criteria

            // callback for estimation of Jacobian matrices
            void (CV_CDECL * fjac)(int i, int j, Mat& point_params,
                                   Mat& cam_params, Mat& A, Mat& B, void* data),
            // callback for estimation of backprojection errors
            void (CV_CDECL * func)(int i, int j, Mat& point_params,
                                   Mat& cam_params, Mat& estim, void* data),
            void* data // user-specific data passed to the callbacks
            );
    virtual ~LevMarqSparse();

    virtual void run( int npoints, // number of points
            int ncameras, // number of cameras
            int nPointParams, // number of params per one point  (3 in case of 3D points)
            int nCameraParams, // number of parameters per one camera
            int nErrParams, // number of parameters in measurement vector
                            // for 1 point at one camera (2 in case of 2D projections)
            Mat& visibility, // visibility matrix. rows correspond to points, columns correspond to cameras
                             // 1 - point is visible for the camera, 0 - invisible
            Mat& P0, // starting vector of parameters, first cameras then points
            Mat& X, // measurements, in order of visibility. non visible cases are skipped
            TermCriteria criteria, // termination criteria

            // callback for estimation of Jacobian matrices
            void (CV_CDECL * fjac)(int i, int j, Mat& point_params,
                                   Mat& cam_params, Mat& A, Mat& B, void* data),
            // callback for estimation of backprojection errors
            void (CV_CDECL * func)(int i, int j, Mat& point_params,
                                   Mat& cam_params, Mat& estim, void* data),
            void* data // user-specific data passed to the callbacks
            );

    virtual void clear();

    // useful function to do simple bundle adjastment tasks
    static void bundleAdjust(vector<Point3d>& points, //positions of points in global coordinate system (input and output)
                             const vector<vector<Point2d> >& imagePoints, //projections of 3d points for every camera
                             const vector<vector<int> >& visibility, //visibility of 3d points for every camera
                             vector<Mat>& cameraMatrix, //intrinsic matrices of all cameras (input and output)
                             vector<Mat>& R, //rotation matrices of all cameras (input and output)
                             vector<Mat>& T, //translation vector of all cameras (input and output)
                             vector<Mat>& distCoeffs, //distortion coefficients of all cameras (input and output)
                             const TermCriteria& criteria=
                             TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, DBL_EPSILON));

protected:
    virtual void optimize(); //main function that runs minimization

    //iteratively asks for measurement for visible camera-point pairs
    void ask_for_proj();
    //iteratively asks for Jacobians for every camera_point pair
    void ask_for_projac();

    CvMat* err; //error X-hX
    double prevErrNorm, errNorm;
    double lambda;
    CvTermCriteria criteria;
    int iters;

    CvMat** U; //size of array is equal to number of cameras
    CvMat** V; //size of array is equal to number of points
    CvMat** inv_V_star; //inverse of V*

    CvMat* A;
    CvMat* B;
    CvMat* W;

    CvMat* X; //measurement
    CvMat* hX; //current measurement extimation given new parameter vector

    CvMat* prevP; //current already accepted parameter.
    CvMat* P; // parameters used to evaluate function with new params
              // this parameters may be rejected

    CvMat* deltaP; //computed increase of parameters (result of normal system solution )

    CvMat** ea; // sum_i  AijT * e_ij , used as right part of normal equation
                // length of array is j = number of cameras
    CvMat** eb; // sum_j  BijT * e_ij , used as right part of normal equation
                // length of array is i = number of points

    CvMat** Yj; //length of array is i = num_points

    CvMat* S; //big matrix of block Sjk  , each block has size num_cam_params x num_cam_params

    CvMat* JtJ_diag; //diagonal of JtJ,  used to backup diagonal elements before augmentation

    CvMat* Vis_index; // matrix which element is index of measurement for point i and camera j

    int num_cams;
    int num_points;
    int num_err_param;
    int num_cam_param;
    int num_point_param;

    //target function and jacobian pointers, which needs to be initialized
    void (*fjac)(int i, int j, Mat& point_params, Mat& cam_params, Mat& A, Mat& B, void* data);
    void (*func)(int i, int j, Mat& point_params, Mat& cam_params, Mat& estim, void* data );

    void* data;
};

struct DefaultRngAuto
{
    const static uint64 def_state = (uint64)-1;
    const uint64 old_state;

    DefaultRngAuto() : old_state(theRNG().state) { theRNG().state = def_state; }
    ~DefaultRngAuto() { theRNG().state = old_state; }

    DefaultRngAuto& operator=(const DefaultRngAuto&);
};

/****************************************************************************************\
*            Calonder Descriptor														 *
\****************************************************************************************/

/*
A pseudo-random number generator usable with std::random_shuffle.
*/
typedef cv::RNG CalonderRng;
typedef unsigned int int_type;

//----------------------------
//randomized_tree.h

//class RTTester;

//namespace features {
static const size_t DEFAULT_REDUCED_NUM_DIM = 176;
static const float LOWER_QUANT_PERC = .03f;
static const float UPPER_QUANT_PERC = .92f;
static const int PATCH_SIZE = 32;
static const int DEFAULT_DEPTH = 9;
static const int DEFAULT_VIEWS = 5000;
struct RTreeNode;

struct BaseKeypoint
{
    int x;
    int y;
    IplImage* image;

    BaseKeypoint()
        : x(0), y(0), image(NULL)
    {}

    BaseKeypoint(int x, int y, IplImage* image)
        : x(x), y(y), image(image)
    {}
};

class CSMatrixGenerator {
public:
    typedef enum { PDT_GAUSS=1, PDT_BERNOULLI, PDT_DBFRIENDLY } PHI_DISTR_TYPE;
    ~CSMatrixGenerator();
    static float* getCSMatrix(int m, int n, PHI_DISTR_TYPE dt);     // do NOT free returned pointer

private:
    static float *cs_phi_;    // matrix for compressive sensing
    static int cs_phi_m_, cs_phi_n_;
};


template< typename T >
struct AlignedMemBlock
{
  AlignedMemBlock() : raw(NULL), data(NULL) { };

  // Alloc's an `a` bytes-aligned block good to hold `sz` elements of class T
  AlignedMemBlock(const int n, const int a)
  {
     alloc(n, a);
  }

  ~AlignedMemBlock()
  {
     free(raw);
  }

  void alloc(const int n, const int a)
  {
     uchar* raw = (uchar*)malloc(n*sizeof(T) + a);
     int delta = (a - uint64(raw)%a)%a;          // # bytes required for padding s.t. we get `a`-aligned
     data = reinterpret_cast<T*>(raw + delta);
  }

  // Methods to access the aligned data. NEVER EVER FREE A RETURNED POINTER!
  inline T* p() { return data; }
  inline T* operator()() { return data; }

private:
  T *raw;     // raw block, probably not aligned
  T *data;    // exposed data, aligned, DO NOT FREE
};

typedef AlignedMemBlock<float> FloatSignature;
typedef AlignedMemBlock<uchar> Signature;

class CV_EXPORTS RandomizedTree
{
public:
    friend class RTreeClassifier;
    //friend class ::RTTester;

    RandomizedTree();
    ~RandomizedTree();

    void train(std::vector<BaseKeypoint> const& base_set, cv::RNG &rng,
        int depth, int views, size_t reduced_num_dim, int num_quant_bits);

    void train(std::vector<BaseKeypoint> const& base_set, cv::RNG &rng,
        PatchGenerator &make_patch, int depth, int views, size_t reduced_num_dim,
        int num_quant_bits);

    // following two funcs are EXPERIMENTAL (do not use unless you know exactly what you do)
    static void quantizeVector(float *vec, int dim, int N, float bnds[2], int clamp_mode=0);
    static void quantizeVector(float *src, int dim, int N, float bnds[2], uchar *dst);

    // patch_data must be a 32x32 array (no row padding)
    float* getPosterior(uchar* patch_data);
    const float* getPosterior(uchar* patch_data) const;
    uchar* getPosterior2(uchar* patch_data);

    void read(const char* file_name, int num_quant_bits);
    void read(std::istream &is, int num_quant_bits);
    void write(const char* file_name) const;
    void write(std::ostream &os) const;

    inline int classes() { return classes_; }
    inline int depth() { return depth_; }

    inline void applyQuantization(int num_quant_bits) { makePosteriors2(num_quant_bits); }

    // debug
    void savePosteriors(std::string url, bool append=false);
    void savePosteriors2(std::string url, bool append=false);

private:
    int classes_;
    int depth_;
    int num_leaves_;
    std::vector<RTreeNode> nodes_;
    //float **posteriors_;       // 16-bytes aligned posteriors
    //uchar **posteriors2_;      // 16-bytes aligned posteriors
  FloatSignature *posteriors_;
  Signature  *posteriors2_;
  std::vector<int> leaf_counts_;

    void createNodes(int num_nodes, cv::RNG &rng);
    void allocPosteriorsAligned(int num_leaves, int num_classes);
    void freePosteriors(int which);    // which: 1=posteriors_, 2=posteriors2_, 3=both
    void init(int classes, int depth, cv::RNG &rng);
    void addExample(int class_id, uchar* patch_data);
    void finalize(size_t reduced_num_dim, int num_quant_bits);
    int getIndex(uchar* patch_data) const;
    inline float* getPosteriorByIndex(int index);
    inline uchar* getPosteriorByIndex2(int index);
    inline const float* getPosteriorByIndex(int index) const;
    //void makeRandomMeasMatrix(float *cs_phi, PHI_DISTR_TYPE dt, size_t reduced_num_dim);
    void convertPosteriorsToChar();
    void makePosteriors2(int num_quant_bits);
    void compressLeaves(size_t reduced_num_dim);
    void estimateQuantPercForPosteriors(float perc[2]);
};

struct RTreeNode
{
    short offset1, offset2;

    RTreeNode() {}

    RTreeNode(uchar x1, uchar y1, uchar x2, uchar y2)
        : offset1(y1*PATCH_SIZE + x1),
        offset2(y2*PATCH_SIZE + x2)
    {}

    //! Left child on 0, right child on 1
    inline bool operator() (uchar* patch_data) const
    {
        return patch_data[offset1] > patch_data[offset2];
    }
};



//} // namespace features
//----------------------------
//rtree_classifier.h
//class RTTester;

//namespace features {

class CV_EXPORTS RTreeClassifier
{
public:
    //friend class ::RTTester;
    static const int DEFAULT_TREES = 80;
    static const size_t DEFAULT_NUM_QUANT_BITS = 4;

  //static const int SIG_LEN = 176;

  RTreeClassifier();

    //modified
    void train(std::vector<BaseKeypoint> const& base_set,
        cv::RNG &rng,
        int num_trees = RTreeClassifier::DEFAULT_TREES,
        int depth = DEFAULT_DEPTH,
        int views = DEFAULT_VIEWS,
        size_t reduced_num_dim = DEFAULT_REDUCED_NUM_DIM,
        int num_quant_bits = DEFAULT_NUM_QUANT_BITS,
        bool print_status = true);

  void train(std::vector<BaseKeypoint> const& base_set,
        cv::RNG &rng,
        PatchGenerator &make_patch,
        int num_trees = DEFAULT_TREES,
        int depth = DEFAULT_DEPTH,
        int views = DEFAULT_VIEWS,
        size_t reduced_num_dim = DEFAULT_REDUCED_NUM_DIM,
        int num_quant_bits = DEFAULT_NUM_QUANT_BITS,
        bool print_status = true);

    // sig must point to a memory block of at least classes()*sizeof(float|uchar) bytes
    void getSignature(IplImage *patch, uchar *sig);
    void getSignature(IplImage *patch, float *sig);
    void getSparseSignature(IplImage *patch, float *sig, float thresh);
    // TODO: deprecated in favor of getSignature overload, remove
    void getFloatSignature(IplImage *patch, float *sig) { getSignature(patch, sig); }

    static int countNonZeroElements(float *vec, int n, double tol=1e-10);
    static inline void safeSignatureAlloc(uchar **sig, int num_sig=1, int sig_len=176);
    static inline uchar* safeSignatureAlloc(int num_sig=1, int sig_len=176);

    inline int classes() const { return classes_; }
    inline int original_num_classes() { return original_num_classes_; }

    void setQuantization(int num_quant_bits);
    void discardFloatPosteriors();

    void read(const char* file_name);
    void read(std::istream &is);
    void write(const char* file_name) const;
    void write(std::ostream &os) const;

    // experimental and debug
    void saveAllFloatPosteriors(std::string file_url);
    void saveAllBytePosteriors(std::string file_url);
    void setFloatPosteriorsFromTextfile_176(std::string url);
    float countZeroElements();

    std::vector<RandomizedTree> trees_;

private:
    int classes_;
    int num_quant_bits_;
    uchar **posteriors_;
    ushort *ptemp_;
    int original_num_classes_;
    bool keep_floats_;
};

CV_EXPORTS bool find4QuadCornerSubpix(const Mat& img, std::vector<Point2f>& corners, Size region_size);


class CV_EXPORTS BackgroundSubtractor
{
public:
    virtual ~BackgroundSubtractor();
    virtual void operator()(const Mat& image, Mat& fgmask, double learningRate=0);
};


class CV_EXPORTS BackgroundSubtractorMOG : public BackgroundSubtractor
{
public:
    BackgroundSubtractorMOG();
    BackgroundSubtractorMOG(int history, int nmixtures, double backgroundRatio, double noiseSigma=0);
    virtual ~BackgroundSubtractorMOG();
    virtual void operator()(const Mat& image, Mat& fgmask, double learningRate=0);

    virtual void initialize(Size frameSize, int frameType);

    Size frameSize;
    int frameType;
    Mat bgmodel;
    int nframes;
    int history;
    int nmixtures;
    double varThreshold;
    double backgroundRatio;
    double noiseSigma;
};


// CvAffinePose: defines a parameterized affine transformation of an image patch.
// An image patch is rotated on angle phi (in degrees), then scaled lambda1 times
// along horizontal and lambda2 times along vertical direction, and then rotated again
// on angle (theta - phi).
class CV_EXPORTS CvAffinePose
{
public:
    float phi;
    float theta;
    float lambda1;
    float lambda2;
};


class CV_EXPORTS OneWayDescriptor
{
public:
    OneWayDescriptor();
    ~OneWayDescriptor();

    // allocates memory for given descriptor parameters
    void Allocate(int pose_count, CvSize size, int nChannels);

    // GenerateSamples: generates affine transformed patches with averaging them over small transformation variations.
    // If external poses and transforms were specified, uses them instead of generating random ones
    // - pose_count: the number of poses to be generated
    // - frontal: the input patch (can be a roi in a larger image)
    // - norm: if nonzero, normalizes the output patch so that the sum of pixel intensities is 1
    void GenerateSamples(int pose_count, IplImage* frontal, int norm = 0);

    // GenerateSamplesFast: generates affine transformed patches with averaging them over small transformation variations.
    // Uses precalculated transformed pca components.
    // - frontal: the input patch (can be a roi in a larger image)
    // - pca_hr_avg: pca average vector
    // - pca_hr_eigenvectors: pca eigenvectors
    // - pca_descriptors: an array of precomputed descriptors of pca components containing their affine transformations
    //   pca_descriptors[0] corresponds to the average, pca_descriptors[1]-pca_descriptors[pca_dim] correspond to eigenvectors
    void GenerateSamplesFast(IplImage* frontal, CvMat* pca_hr_avg,
                             CvMat* pca_hr_eigenvectors, OneWayDescriptor* pca_descriptors);

    // sets the poses and corresponding transforms
    void SetTransforms(CvAffinePose* poses, CvMat** transforms);

    // Initialize: builds a descriptor.
    // - pose_count: the number of poses to build. If poses were set externally, uses them rather than generating random ones
    // - frontal: input patch. Can be a roi in a larger image
    // - feature_name: the feature name to be associated with the descriptor
    // - norm: if 1, the affine transformed patches are normalized so that their sum is 1
    void Initialize(int pose_count, IplImage* frontal, const char* feature_name = 0, int norm = 0);

    // InitializeFast: builds a descriptor using precomputed descriptors of pca components
    // - pose_count: the number of poses to build
    // - frontal: input patch. Can be a roi in a larger image
    // - feature_name: the feature name to be associated with the descriptor
    // - pca_hr_avg: average vector for PCA
    // - pca_hr_eigenvectors: PCA eigenvectors (one vector per row)
    // - pca_descriptors: precomputed descriptors of PCA components, the first descriptor for the average vector
    // followed by the descriptors for eigenvectors
    void InitializeFast(int pose_count, IplImage* frontal, const char* feature_name,
                        CvMat* pca_hr_avg, CvMat* pca_hr_eigenvectors, OneWayDescriptor* pca_descriptors);

    // ProjectPCASample: unwarps an image patch into a vector and projects it into PCA space
    // - patch: input image patch
    // - avg: PCA average vector
    // - eigenvectors: PCA eigenvectors, one per row
    // - pca_coeffs: output PCA coefficients
    void ProjectPCASample(IplImage* patch, CvMat* avg, CvMat* eigenvectors, CvMat* pca_coeffs) const;

    // InitializePCACoeffs: projects all warped patches into PCA space
    // - avg: PCA average vector
    // - eigenvectors: PCA eigenvectors, one per row
    void InitializePCACoeffs(CvMat* avg, CvMat* eigenvectors);

    // EstimatePose: finds the closest match between an input patch and a set of patches with different poses
    // - patch: input image patch
    // - pose_idx: the output index of the closest pose
    // - distance: the distance to the closest pose (L2 distance)
    void EstimatePose(IplImage* patch, int& pose_idx, float& distance) const;

    // EstimatePosePCA: finds the closest match between an input patch and a set of patches with different poses.
    // The distance between patches is computed in PCA space
    // - patch: input image patch
    // - pose_idx: the output index of the closest pose
    // - distance: distance to the closest pose (L2 distance in PCA space)
    // - avg: PCA average vector. If 0, matching without PCA is used
    // - eigenvectors: PCA eigenvectors, one per row
    void EstimatePosePCA(CvArr* patch, int& pose_idx, float& distance, CvMat* avg, CvMat* eigenvalues) const;

    // GetPatchSize: returns the size of each image patch after warping (2 times smaller than the input patch)
    CvSize GetPatchSize() const
    {
        return m_patch_size;
    }

    // GetInputPatchSize: returns the required size of the patch that the descriptor is built from
    // (2 time larger than the patch after warping)
    CvSize GetInputPatchSize() const
    {
        return cvSize(m_patch_size.width*2, m_patch_size.height*2);
    }

    // GetPatch: returns a patch corresponding to specified pose index
    // - index: pose index
    // - return value: the patch corresponding to specified pose index
    IplImage* GetPatch(int index);

    // GetPose: returns a pose corresponding to specified pose index
    // - index: pose index
    // - return value: the pose corresponding to specified pose index
    CvAffinePose GetPose(int index) const;

    // Save: saves all patches with different poses to a specified path
    void Save(const char* path);

    // ReadByName: reads a descriptor from a file storage
    // - fs: file storage
    // - parent: parent node
    // - name: node name
    // - return value: 1 if succeeded, 0 otherwise
    int ReadByName(CvFileStorage* fs, CvFileNode* parent, const char* name);

    // Write: writes a descriptor into a file storage
    // - fs: file storage
    // - name: node name
    void Write(CvFileStorage* fs, const char* name);

    // GetFeatureName: returns a name corresponding to a feature
    const char* GetFeatureName() const;

    // GetCenter: returns the center of the feature
    CvPoint GetCenter() const;

    void SetPCADimHigh(int pca_dim_high) {m_pca_dim_high = pca_dim_high;};
    void SetPCADimLow(int pca_dim_low) {m_pca_dim_low = pca_dim_low;};

    int GetPCADimLow() const;
    int GetPCADimHigh() const;

    CvMat** GetPCACoeffs() const {return m_pca_coeffs;}

protected:
    int m_pose_count; // the number of poses
    CvSize m_patch_size; // size of each image
    IplImage** m_samples; // an array of length m_pose_count containing the patch in different poses
    IplImage* m_input_patch;
    IplImage* m_train_patch;
    CvMat** m_pca_coeffs; // an array of length m_pose_count containing pca decomposition of the patch in different poses
    CvAffinePose* m_affine_poses; // an array of poses
    CvMat** m_transforms; // an array of affine transforms corresponding to poses

    std::string m_feature_name; // the name of the feature associated with the descriptor
    CvPoint m_center; // the coordinates of the feature (the center of the input image ROI)

    int m_pca_dim_high; // the number of descriptor pca components to use for generating affine poses
    int m_pca_dim_low; // the number of pca components to use for comparison
};


// OneWayDescriptorBase: encapsulates functionality for training/loading a set of one way descriptors
// and finding the nearest closest descriptor to an input feature
class CV_EXPORTS OneWayDescriptorBase
{
public:

    // creates an instance of OneWayDescriptor from a set of training files
    // - patch_size: size of the input (large) patch
    // - pose_count: the number of poses to generate for each descriptor
    // - train_path: path to training files
    // - pca_config: the name of the file that contains PCA for small patches (2 times smaller
    // than patch_size each dimension
    // - pca_hr_config: the name of the file that contains PCA for large patches (of patch_size size)
    // - pca_desc_config: the name of the file that contains descriptors of PCA components
    OneWayDescriptorBase(CvSize patch_size, int pose_count, const char* train_path = 0, const char* pca_config = 0,
                         const char* pca_hr_config = 0, const char* pca_desc_config = 0, int pyr_levels = 1,
                         int pca_dim_high = 100, int pca_dim_low = 100);

    ~OneWayDescriptorBase();

    // Allocate: allocates memory for a given number of descriptors
    void Allocate(int train_feature_count);

    // AllocatePCADescriptors: allocates memory for pca descriptors
    void AllocatePCADescriptors();

    // returns patch size
    CvSize GetPatchSize() const {return m_patch_size;};
    // returns the number of poses for each descriptor
    int GetPoseCount() const {return m_pose_count;};

    // returns the number of pyramid levels
    int GetPyrLevels() const {return m_pyr_levels;};

    // returns the number of descriptors
    int GetDescriptorCount() const {return m_train_feature_count;};

    // CreateDescriptorsFromImage: creates descriptors for each of the input features
    // - src: input image
    // - features: input features
    // - pyr_levels: the number of pyramid levels
    void CreateDescriptorsFromImage(IplImage* src, const std::vector<cv::KeyPoint>& features);

    // CreatePCADescriptors: generates descriptors for PCA components, needed for fast generation of feature descriptors
    void CreatePCADescriptors();

    // returns a feature descriptor by feature index
    const OneWayDescriptor* GetDescriptor(int desc_idx) const {return &m_descriptors[desc_idx];};

    // FindDescriptor: finds the closest descriptor
    // - patch: input image patch
    // - desc_idx: output index of the closest descriptor to the input patch
    // - pose_idx: output index of the closest pose of the closest descriptor to the input patch
    // - distance: distance from the input patch to the closest feature pose
    // - _scales: scales of the input patch for each descriptor
    // - scale_ranges: input scales variation (float[2])
    void FindDescriptor(IplImage* patch, int& desc_idx, int& pose_idx, float& distance, float* _scale = 0, float* scale_ranges = 0) const;

    // - patch: input image patch
    // - n: number of the closest indexes
    // - desc_idxs: output indexes of the closest descriptor to the input patch (n)
    // - pose_idx: output indexes of the closest pose of the closest descriptor to the input patch (n)
    // - distances: distance from the input patch to the closest feature pose (n)
    // - _scales: scales of the input patch
    // - scale_ranges: input scales variation (float[2])
    void FindDescriptor(IplImage* patch, int n, std::vector<int>& desc_idxs, std::vector<int>& pose_idxs,
                        std::vector<float>& distances, std::vector<float>& _scales, float* scale_ranges = 0) const;

    // FindDescriptor: finds the closest descriptor
    // - src: input image
    // - pt: center of the feature
    // - desc_idx: output index of the closest descriptor to the input patch
    // - pose_idx: output index of the closest pose of the closest descriptor to the input patch
    // - distance: distance from the input patch to the closest feature pose
    void FindDescriptor(IplImage* src, cv::Point2f pt, int& desc_idx, int& pose_idx, float& distance) const;

    // InitializePoses: generates random poses
    void InitializePoses();

    // InitializeTransformsFromPoses: generates 2x3 affine matrices from poses (initializes m_transforms)
    void InitializeTransformsFromPoses();

    // InitializePoseTransforms: subsequently calls InitializePoses and InitializeTransformsFromPoses
    void InitializePoseTransforms();

    // InitializeDescriptor: initializes a descriptor
    // - desc_idx: descriptor index
    // - train_image: image patch (ROI is supported)
    // - feature_label: feature textual label
    void InitializeDescriptor(int desc_idx, IplImage* train_image, const char* feature_label);

    void InitializeDescriptor(int desc_idx, IplImage* train_image, const cv::KeyPoint& keypoint, const char* feature_label);

    // InitializeDescriptors: load features from an image and create descriptors for each of them
    void InitializeDescriptors(IplImage* train_image, const vector<cv::KeyPoint>& features,
                               const char* feature_label = "", int desc_start_idx = 0);

    // LoadPCADescriptors: loads PCA descriptors from a file
    // - filename: input filename
    int LoadPCADescriptors(const char* filename);

    // SavePCADescriptors: saves PCA descriptors to a file
    // - filename: output filename
    void SavePCADescriptors(const char* filename);

    // SetPCAHigh: sets the high resolution pca matrices (copied to internal structures)
    void SetPCAHigh(CvMat* avg, CvMat* eigenvectors);

    // SetPCALow: sets the low resolution pca matrices (copied to internal structures)
    void SetPCALow(CvMat* avg, CvMat* eigenvectors);

    int GetLowPCA(CvMat** avg, CvMat** eigenvectors)
    {
        *avg = m_pca_avg;
        *eigenvectors = m_pca_eigenvectors;
        return m_pca_dim_low;
    };

    int GetPCADimLow() const {return m_pca_dim_low;};
    int GetPCADimHigh() const {return m_pca_dim_high;};

    void ConvertDescriptorsArrayToTree(); // Converting pca_descriptors array to KD tree


protected:
    CvSize m_patch_size; // patch size
    int m_pose_count; // the number of poses for each descriptor
    int m_train_feature_count; // the number of the training features
    OneWayDescriptor* m_descriptors; // array of train feature descriptors
    CvMat* m_pca_avg; // PCA average Vector for small patches
    CvMat* m_pca_eigenvectors; // PCA eigenvectors for small patches
    CvMat* m_pca_hr_avg; // PCA average Vector for large patches
    CvMat* m_pca_hr_eigenvectors; // PCA eigenvectors for large patches
    OneWayDescriptor* m_pca_descriptors; // an array of PCA descriptors

    cv::flann::Index* m_pca_descriptors_tree;
    CvMat* m_pca_descriptors_matrix;

    CvAffinePose* m_poses; // array of poses
    CvMat** m_transforms; // array of affine transformations corresponding to poses

    int m_pca_dim_high;
    int m_pca_dim_low;

    int m_pyr_levels;

};

class CV_EXPORTS OneWayDescriptorObject : public OneWayDescriptorBase
{
public:
    // creates an instance of OneWayDescriptorObject from a set of training files
    // - patch_size: size of the input (large) patch
    // - pose_count: the number of poses to generate for each descriptor
    // - train_path: path to training files
    // - pca_config: the name of the file that contains PCA for small patches (2 times smaller
    // than patch_size each dimension
    // - pca_hr_config: the name of the file that contains PCA for large patches (of patch_size size)
    // - pca_desc_config: the name of the file that contains descriptors of PCA components
    OneWayDescriptorObject(CvSize patch_size, int pose_count, const char* train_path, const char* pca_config,
                           const char* pca_hr_config = 0, const char* pca_desc_config = 0, int pyr_levels = 1);

    ~OneWayDescriptorObject();

    // Allocate: allocates memory for a given number of features
    // - train_feature_count: the total number of features
    // - object_feature_count: the number of features extracted from the object
    void Allocate(int train_feature_count, int object_feature_count);


    void SetLabeledFeatures(const vector<cv::KeyPoint>& features) {m_train_features = features;};
    vector<cv::KeyPoint>& GetLabeledFeatures() {return m_train_features;};
    const vector<cv::KeyPoint>& GetLabeledFeatures() const {return m_train_features;};
    vector<cv::KeyPoint> _GetLabeledFeatures() const;

    // IsDescriptorObject: returns 1 if descriptor with specified index is positive, otherwise 0
    int IsDescriptorObject(int desc_idx) const;

    // MatchPointToPart: returns the part number of a feature if it matches one of the object parts, otherwise -1
    int MatchPointToPart(CvPoint pt) const;

    // GetDescriptorPart: returns the part number of the feature corresponding to a specified descriptor
    // - desc_idx: descriptor index
    int GetDescriptorPart(int desc_idx) const;


    void InitializeObjectDescriptors(IplImage* train_image, const vector<cv::KeyPoint>& features,
                                     const char* feature_label, int desc_start_idx = 0, float scale = 1.0f,
                                     int is_background = 0);

    // GetObjectFeatureCount: returns the number of object features
    int GetObjectFeatureCount() const {return m_object_feature_count;};

protected:
    int* m_part_id; // contains part id for each of object descriptors
    vector<cv::KeyPoint> m_train_features; // train features
    int m_object_feature_count; // the number of the positive features

};

/****************************************************************************************\
*                                    FeatureDetector                                     *
\****************************************************************************************/

/*
 * Abstract base class for 2D image feature detectors.
 */
class CV_EXPORTS FeatureDetector
{
public:
    /*
     * Detect keypoints in an image.
     *
     * image        The image.
     * keypoints    The detected keypoints.
     * mask         Mask specifying where to look for keypoints (optional). Must be a char matrix with non-zero values in the region of interest.
     */
    void detect( const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask=Mat() ) const
    {
        detectImpl( image, mask, keypoints );
    }

protected:
    /*
     * Detect keypoints; detect() calls this. Must be implemented by the subclass.
     */
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const = 0;

    /*
     * Remove keypoints that are not in the mask.
     *
     * Helper function, useful when wrapping a library call for keypoint detection that
     * does not support a mask argument.
     */
    static void removeInvalidPoints( const Mat& mask, vector<KeyPoint>& keypoints );
};

class CV_EXPORTS FastFeatureDetector : public FeatureDetector
{
public:
    FastFeatureDetector( int _threshold, bool _nonmaxSuppression = true );

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    int threshold;
    bool nonmaxSuppression;
};


class CV_EXPORTS GoodFeaturesToTrackDetector : public FeatureDetector
{
public:
    GoodFeaturesToTrackDetector( int _maxCorners, double _qualityLevel, double _minDistance,
                                 int _blockSize=3, bool _useHarrisDetector=false, double _k=0.04 );
protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    int maxCorners;
    double qualityLevel;
    double minDistance;
    int blockSize;
    bool useHarrisDetector;
    double k;
};

class CV_EXPORTS MserFeatureDetector : public FeatureDetector
{
public:
    MserFeatureDetector( int delta, int minArea, int maxArea, float maxVariation, float minDiversity,
                         int maxEvolution, double areaThreshold, double minMargin, int edgeBlurSize );
protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    MSER mser;
};

class CV_EXPORTS StarFeatureDetector : public FeatureDetector
{
public:
    StarFeatureDetector( int maxSize=16, int responseThreshold=30, int lineThresholdProjected = 10,
                         int lineThresholdBinarized=8, int suppressNonmaxSize=5 );

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    StarDetector star;
};

class CV_EXPORTS SurfFeatureDetector : public FeatureDetector
{
public:
    SurfFeatureDetector( double hessianThreshold = 400., int octaves = 3, int octaveLayers = 4 );

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    SURF surf;
};


/****************************************************************************************\
*                                 DescriptorExtractor                                    *
\****************************************************************************************/

/*
 * Abstract base class for computing descriptors for image keypoints.
 *
 * In this interface we assume a keypoint descriptor can be represented as a
 * dense, fixed-dimensional vector of some basic type. Most descriptors used
 * in practice follow this pattern, as it makes it very easy to compute
 * distances between descriptors. Therefore we represent a collection of
 * descriptors as a cv::Mat, where each row is one keypoint descriptor.
 */
class CV_EXPORTS DescriptorExtractor
{
public:
    /*
     * Compute the descriptors for a set of keypoints in an image.
     *
     * Must be implemented by the subclass.
     *
     * image        The image.
     * keypoints    The keypoints. Keypoints for which a descriptor cannot be computed are removed.
     * descriptors  The descriptors. Row i is the descriptor for keypoint i.
     */
    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors ) const = 0;

protected:
    /*
     * Remove keypoints within border_pixels of an image edge.
     */
    static void removeBorderKeypoints( vector<KeyPoint>& keypoints,
                                       Size imageSize, int borderPixels );
};


class CV_EXPORTS SurfDescriptorExtractor : public DescriptorExtractor
{
public:
    SurfDescriptorExtractor( int nOctaves=4,
                             int nOctaveLayers=2, bool extended=false );

    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors) const;

protected:
    SURF surf;
};

/*template<typename T>
class CV_EXPORTS CalonderDescriptorExtractor : public DescriptorExtractor
{
public:
    CalonderDescriptorExtractor( const string& classifierFile )
    {
        classifier.read(classifierFile.c_str());
    }

    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors) const
    {
        // Cannot compute descriptors for keypoints on the image border.
        removeBorderKeypoints(keypoints, image.size(), BORDER_SIZE);

        // TODO Check 16-byte aligned
        descriptors.create( keypoints.size(), classifier.classes(), DataType<T>::type );
        PatchGenerator patchGen;
        RNG rng;

        for( size_t i = 0; i < keypoints.size(); i++ )
        {
            Mat patch;
            patchGen( image, keypoints[i].pt, patch, Size(PATCH_SIZE, PATCH_SIZE), rng );
            IplImage ipl = patch;
            classifier.getSignature( &ipl, descriptors.ptr<T>(i));
        }
    }

protected:
    static const int BORDER_SIZE = 16;
    RTreeClassifier classifier;
};*/

/****************************************************************************************\
*                                          Distance                                      *
\****************************************************************************************/
template<typename T>
struct CV_EXPORTS Accumulator
{
    typedef T Type;
};

template<> struct Accumulator<uint8_t>  { typedef uint32_t Type; };
template<> struct Accumulator<uint16_t> { typedef uint32_t Type; };
template<> struct Accumulator<int8_t>   { typedef int32_t Type; };
template<> struct Accumulator<int16_t>  { typedef int32_t Type; };

/*
 * Squared Euclidean distance functor
 */
template<class T>
struct CV_EXPORTS L2
{
    typedef T ValueType;
    typedef typename Accumulator<T>::Type ResultType;

    ResultType operator()( const T* a, const T* b, int size ) const
    {
        ResultType result = ResultType();
        for( int i = 0; i < size; i++ )
        {
            ResultType diff = a[i] - b[i];
            result += diff*diff;
        }
        return result;
    }
};

/****************************************************************************************\
*                                  DescriptorMatcher                                     *
\****************************************************************************************/
/*
 * Abstract base class for matching two sets of descriptors.
 */
class CV_EXPORTS DescriptorMatcher
{
public:
    /*
     * Add descriptors to the training set
     * descriptors Descriptors to add to the training set
     */
    void add( const Mat& descriptors );

    /*
     * Index the descriptors training set
     */
    void index();

    /*
     * Find the best match for each descriptor from a query set
     *
     * query         The query set of descriptors
     * matches       Indices of the closest matches from the training set
     */


    // TODO: remove vector<double>* distances = 0 ???
    void match( const Mat& query, vector<int>& matches, vector<double>* distances = 0 ) const;

    /*
     * Find the best matches between two descriptor sets, with constraints
     * on which pairs of descriptors can be matched.
     *
     * The mask describes which descriptors can be matched. descriptors_1[i]
     * can be matched with descriptors_2[j] only if mask.at<char>(i,j) is non-zero.
     *
     * query         The query set of descriptors
     * mask          Mask specifying permissible matches.
     * matches       Indices of the closest matches from the training set
     */
    void match( const Mat& query, const Mat& mask,
                vector<int>& matches, vector<double>* distances = 0 ) const;

    /*
     * Find the best keypoint matches for small view changes.
     *
     * This function will only match descriptors whose keypoints have close enough
     * image coordinates.
     *
     * keypoints_1   The first set of keypoints.
     * descriptors_1 The first set of descriptors.
     * keypoints_2   The second set of keypoints.
     * descriptors_2 The second set of descriptors.
     * maxDeltaX     The maximum horizontal displacement.
     * maxDeltaY     The maximum vertical displacement.
     * matches       The matches between both sets.
     */
    /*void matchWindowed( const vector<KeyPoint>& keypoints_1, const Mat& descriptors_1,
                        const vector<KeyPoint>& keypoints_2, const Mat& descriptors_2,
                        float maxDeltaX, float maxDeltaY, vector<Match>& matches) const;*/

protected:
    Mat train;

    /*
     * Find matches; match() calls this. Must be implemented by the subclass.
     * The mask may be empty.
     */
    virtual void matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                            const Mat& mask, vector<int>& matches, vector<double>& distances ) const = 0;

    static bool possibleMatch( const Mat& mask, int index_1, int index_2 )
    {
        return mask.empty() || mask.at<char>(index_1, index_2);
    }
};

inline void DescriptorMatcher::add( const Mat& descriptors )
{
    if( train.empty() )
    {
        train = descriptors;
    }
    else
    {
        // merge train and descriptors
        Mat m( train.rows + descriptors.rows, train.cols, CV_32F );
        Mat m1 = m.rowRange( 0, train.rows );
        train.copyTo( m1 );
        Mat m2 = m.rowRange( train.rows + 1, m.rows );
        descriptors.copyTo( m2 );
        train = m;
    }
}

inline void DescriptorMatcher::match( const Mat& query, vector<int>& matches, vector<double>* distances ) const
{
    if( distances )
        matchImpl( query, train, Mat(), matches, *distances );
    else
    {
        vector<double> innDistances;
        matchImpl( query, train, Mat(), matches, innDistances );
    }
}

inline void DescriptorMatcher::match( const Mat& query, const Mat& mask,
                                      vector<int>& matches, vector<double>* distances ) const
{
    if( distances )
        matchImpl( query, train, mask, matches, *distances );
    else
    {
        vector<double> innDistances;
        matchImpl( query, train, mask, matches, innDistances );
    }
}

/*
 * Brute-force descriptor matcher.
 *
 * For each descriptor in the first set, this matcher finds the closest
 * descriptor in the second set by trying each one.
 *
 * For efficiency, BruteForceMatcher is templated on the distance metric.
 * For float descriptors, a common choice would be features_2d::L2<float>.
 */
template<class Distance>
class CV_EXPORTS BruteForceMatcher : public DescriptorMatcher
{
public:
    BruteForceMatcher( Distance d = Distance() ) : distance(d) {}

protected:
   virtual void matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                           const Mat& mask, vector<int>& matches, vector<double>& distances) const;

   Distance distance;
};

template<class Distance>
void BruteForceMatcher<Distance>::matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                                             const Mat& mask, vector<int>& matches,
                                             vector<double>& distances) const
{
    typedef typename Distance::ValueType ValueType;
    typedef typename Distance::ResultType DistanceType;

    assert( mask.empty() || (mask.rows == descriptors_1.rows && mask.cols == descriptors_2.rows) );

    assert( descriptors_1.cols == descriptors_2.cols );
    assert( DataType<ValueType>::type == descriptors_1.type() );
    assert( DataType<ValueType>::type == descriptors_2.type() );

    int dimension = descriptors_1.cols;
    matches.clear();
    matches.reserve(descriptors_1.rows);

    for( int i = 0; i < descriptors_1.rows; i++ )
    {
        const ValueType* d1 = descriptors_1.ptr<ValueType>(i);
        int matchIndex = -1;
        DistanceType matchDistance = std::numeric_limits<DistanceType>::max();

        for( int j = 0; j < descriptors_2.rows; j++ )
        {
            if( possibleMatch(mask, i, j) )
            {
                const ValueType* d2 = descriptors_2.ptr<ValueType>(j);
                DistanceType curDistance = distance(d1, d2, dimension);
                if( curDistance < matchDistance )
                {
                    matchDistance = curDistance;
                    matchIndex = j;
                }
            }
        }

        if( matchIndex != -1 )
        {
            matches.push_back( matchIndex );
            distances.push_back( matchDistance );
        }
    }
}


/****************************************************************************************\
*                                GenericDescriptorMatch                                  *
\****************************************************************************************/
/*
 * A storage for sets of keypoints together with corresponding images and class IDs
 */
class CV_EXPORTS KeyPointCollection
{
public:
    // Adds keypoints from a single image to the storage
    // image    Source image
    // points   A vector of keypoints
    void add( const Mat& _image, const vector<KeyPoint>& _points );

    // Returns the total number of keypoints in the collection
    size_t calcKeypointCount() const;

    // Returns the keypoint by its global index
    KeyPoint getKeyPoint( int index ) const;

    vector<Mat> images;
    vector<vector<KeyPoint> > points;

    // global indices of the first points in each image,
    // startIndices.size() = points.size()
    vector<int> startIndices;
};

/*
 *   Abstract interface for a keypoint descriptor
 */
class CV_EXPORTS GenericDescriptorMatch
{
public:
    enum IndexType
    {
        NoIndex,
        KDTreeIndex
    };

    GenericDescriptorMatch() {}
    virtual ~GenericDescriptorMatch() {}

    // Adds keypoints to the training set (descriptors are supposed to be calculated here)
    virtual void add( KeyPointCollection& keypoints );

    // Adds keypoints from a single image to the training set (descriptors are supposed to be calculated here)
    virtual void add( const Mat& image, vector<KeyPoint>& points ) = 0;

    // Classifies test keypoints
    // image    The source image
    // points   Test keypoints from the source image
    virtual void classify( const Mat& image, vector<KeyPoint>& points );

    // Matches test keypoints to the training set
    // image        The source image
    // points       Test keypoints from the source image
    // class_ids    A vector to be filled with keypoint class_ids
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices ) = 0;

protected:
    KeyPointCollection collection;
};

/*
 *  OneWayDescriptorMatch
 */
class CV_EXPORTS OneWayDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        Params( int _poseCount = POSE_COUNT,
                Size _patchSize = Size(PATCH_WIDTH, PATCH_HEIGHT),
                string _trainPath = string(),
                string _pcaConfig = string(), string _pcaHrConfig = string(),
                string _pcaDescConfig = string(),
                float minScale = MIN_SCALE, float maxScale = MAX_SCALE,
                float stepScale = STEP_SCALE) :
            poseCount(_poseCount), patchSize(_patchSize), trainPath(_trainPath),
            pcaConfig(_pcaConfig), pcaHrConfig(_pcaHrConfig), pcaDescConfig(_pcaDescConfig) {}

        int poseCount;
        Size patchSize;
        string trainPath;
        string pcaConfig, pcaHrConfig, pcaDescConfig;

        float minScale, maxScale, stepScale;

        const static int POSE_COUNT = 500;
        const static int PATCH_WIDTH = 24;
        const static int PATCH_HEIGHT = 24;
        static const float MIN_SCALE = 1.f;
        static const float MAX_SCALE = 3.f;
        static const float STEP_SCALE = 1.15f;
    };

    OneWayDescriptorMatch();

    // Equivalent to calling PointMatchOneWay() followed by Initialize(_params)
    OneWayDescriptorMatch( const Params& _params );
    virtual ~OneWayDescriptorMatch();

    // Sets one way descriptor parameters
    void initialize( const Params& _params );

    // Calculates one way descriptors for a set of keypoints
    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    // Calculates one way descriptors for a set of keypoints
    virtual void add( KeyPointCollection& keypoints );

    // Matches a set of keypoints from a single image of the training set. A rectangle with a center in a keypoint
    // and size (patch_width/2*scale, patch_height/2*scale) is cropped from the source image for each
    // keypoint. scale is iterated from DescriptorOneWayParams::min_scale to DescriptorOneWayParams::max_scale.
    // The minimum distance to each training patch with all its affine poses is found over all scales.
    // The class ID of a match is returned for each keypoint. The distance is calculated over PCA components
    // loaded with DescriptorOneWay::Initialize, kd tree is used for finding minimum distances.
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices );

    // Classify a set of keypoints. The same as match, but returns point classes rather than indices
    virtual void classify( const Mat& image, vector<KeyPoint>& points );

protected:
    Ptr<OneWayDescriptorBase> base;
    Params params;
};

/*
 *  CalonderDescriptorMatch
 */
class CV_EXPORTS CalonderDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        static const int DEFAULT_NUM_TREES = 80;
        static const int DEFAULT_DEPTH = 9;
        static const int DEFAULT_VIEWS = 5000;
        static const size_t DEFAULT_REDUCED_NUM_DIM = 176;
        static const size_t DEFAULT_NUM_QUANT_BITS = 4;
        static const int DEFAULT_PATCH_SIZE = PATCH_SIZE;

        Params( const RNG& _rng = RNG(), const PatchGenerator& _patchGen = PatchGenerator(),
                int _numTrees=DEFAULT_NUM_TREES,
                int _depth=DEFAULT_DEPTH,
                int _views=DEFAULT_VIEWS,
                size_t _reducedNumDim=DEFAULT_REDUCED_NUM_DIM,
                int _numQuantBits=DEFAULT_NUM_QUANT_BITS,
                bool _printStatus=true,
                int _patchSize=DEFAULT_PATCH_SIZE );
        Params( const string& _filename );

        RNG rng;
        PatchGenerator patchGen;
        int numTrees;
        int depth;
        int views;
        int patchSize;
        size_t reducedNumDim;
        int numQuantBits;
        bool printStatus;

        string filename;
    };

    CalonderDescriptorMatch();

    CalonderDescriptorMatch( const Params& _params );
    virtual ~CalonderDescriptorMatch();

    void initialize( const Params& _params );

    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void match( const Mat& image, vector<KeyPoint>& keypoints, vector<int>& indices );

    virtual void classify( const Mat& image, vector<KeyPoint>& keypoints );

protected:
    void trainRTreeClassifier();
    Mat extractPatch( const Mat& image, const Point& pt, int patchSize ) const;
    void calcBestProbAndMatchIdx( const Mat& image, const Point& pt,
                                  float& bestProb, int& bestMatchIdx, float* signature );

    Ptr<RTreeClassifier> classifier;
    Params params;
};

/*
 *  FernDescriptorMatch
 */
class CV_EXPORTS FernDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        Params( int _nclasses=0,
                int _patchSize=FernClassifier::PATCH_SIZE,
                int _signatureSize=FernClassifier::DEFAULT_SIGNATURE_SIZE,
                int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                int _nviews=FernClassifier::DEFAULT_VIEWS,
                int _compressionMethod=FernClassifier::COMPRESSION_NONE,
                const PatchGenerator& patchGenerator=PatchGenerator() );

        Params( const string& _filename );

        int nclasses;
        int patchSize;
        int signatureSize;
        int nstructs;
        int structSize;
        int nviews;
        int compressionMethod;
        PatchGenerator patchGenerator;

        string filename;
    };

    FernDescriptorMatch();

    FernDescriptorMatch( const Params& _params );
    virtual ~FernDescriptorMatch();

    void initialize( const Params& _params );

    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void match( const Mat& image, vector<KeyPoint>& keypoints, vector<int>& indices );

    virtual void classify( const Mat& image, vector<KeyPoint>& keypoints );

protected:
    void trainFernClassifier();
    void calcBestProbAndMatchIdx( const Mat& image, const Point2f& pt,
                                  float& bestProb, int& bestMatchIdx, vector<float>& signature );
    Ptr<FernClassifier> classifier;
    Params params;
};

/****************************************************************************************\
*                                VectorDescriptorMatch                                   *
\****************************************************************************************/

/*
 *  An abstract class used for matching descriptors that can be described as vectors in a finite-dimensional space
 */
template<class Extractor, class Matcher>
class CV_EXPORTS VectorDescriptorMatch : public GenericDescriptorMatch
{
public:
    using GenericDescriptorMatch::add;

    VectorDescriptorMatch( const Extractor& _extractor = Extractor(), const Matcher& _matcher = Matcher() ) :
                           extractor(_extractor), matcher(_matcher) {}

    ~VectorDescriptorMatch() {}

    // Builds flann index
    void index();

    // Calculates descriptors for a set of keypoints from a single image
    virtual void add( const Mat& image, vector<KeyPoint>& keypoints )
    {
        Mat descriptors;
        extractor.compute( image, keypoints, descriptors );
        matcher.add( descriptors );

        collection.add( Mat(), keypoints );
    };

    // Matches a set of keypoints with the training set
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& keypointIndices )
    {
        Mat descriptors;
        extractor.compute( image, points, descriptors );

        matcher.match( descriptors, keypointIndices );
    };

protected:
    Extractor extractor;
    Matcher matcher;
    vector<int> classIds;
};

// A factory function for creating an arbitrary descriptor matcher at runtime
/*inline GenericDescriptorMatch* createDescriptorMatcher(std::string extractor = "SURF", std::string matcher = "BruteForce")
{
    GenericDescriptorMatch* descriptors = NULL;

    if(matcher.compare("BruteForce") != 0)
    {
        // only one matcher exists now
        return 0;
    }

    if(extractor.compare("SURF"))
    {
        descriptors = new DescriptorMatchVector<features_2d::SurfDescriptorExtractor, features_2d::BruteForceMatcher<features_2d::L2<float> > >();
    }
    else if(extractor.compare("OneWay"))
    {
        descriptors = new DescriptorMatchOneWay();
    }
}*/

}

#endif /* __cplusplus */

#endif /* __CVAUX_HPP__ */

/* End of file. */
