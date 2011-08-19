#include "perf_precomp.hpp"

#ifdef ANDROID
#define PERF_TESTS_LOG_TAG "OpenCV_perf"
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, PERF_TESTS_LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, PERF_TESTS_LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, PERF_TESTS_LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, PERF_TESTS_LOG_TAG, __VA_ARGS__))
#else
#define LOGD(_str, ...) do{printf(_str , ## __VA_ARGS__); printf("\n");fflush(stdout);} while(0)
#define LOGI(_str, ...) do{printf(_str , ## __VA_ARGS__); printf("\n");fflush(stdout);} while(0)
#define LOGW(_str, ...) do{printf(_str , ## __VA_ARGS__); printf("\n");fflush(stdout);} while(0)
#define LOGE(_str, ...) do{printf(_str , ## __VA_ARGS__); printf("\n");fflush(stdout);} while(0)
#endif

namespace perf
{

const cv::Size szQVGA = cv::Size(320, 240);
const cv::Size szVGA = cv::Size(640, 480);
const cv::Size szSVGA = cv::Size(800, 600);
const cv::Size szXGA = cv::Size(1024, 768);
const cv::Size szSXGA = cv::Size(1280, 1024);

const cv::Size sznHD = cv::Size(640, 360);
const cv::Size szqHD = cv::Size(960, 540);
const cv::Size sz720p = cv::Size(1280, 720);
const cv::Size sz1080p = cv::Size(1920, 1080);

const cv::Size szODD = cv::Size(127, 61);

#define SZ_ALL_VGA ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA)
#define SZ_ALL_GA  ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA, ::perf::szXGA, ::perf::szSXGA)
#define SZ_ALL_HD  ::testing::Values(::perf::sznHD, ::perf::szqHD, ::perf::sz720p, ::perf::sz1080p)
#define SZ_ALL  ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA, ::perf::szXGA, ::perf::szSXGA, ::perf::sznHD, ::perf::szqHD, ::perf::sz720p, ::perf::sz1080p)

#define SZ_TYPICAL  ::testing::Values(::perf::szVGA, ::perf::szqHD, ::perf::sz720p, ::perf::szODD)

typedef struct MatInfo
{
    enum {
        Zeros,
        Ones,
        Eye,
        Diag,
        Fill,
        Rng
    };

    cv::Size size;
    int type;
    int kind;
    cv::Range roix, roiy;

    MatInfo(cv::Size sz, int type = CV_8UC1, int kind = Fill, cv::Range roix = cv::Range::all(), cv::Range roiy = cv::Range::all());

    cv::Mat makeMat() const;
    cv::Mat makeMat(double v1, double v2 = 0, double v3 =0, double v4 = 0) const;

} MatInfo, iMat;

#define mQVGA8UC1(...)  ::perf::MatInfo(::perf::szQVGA,  CV_8UC1 , ## __VA_ARGS__)
#define mVGA8UC1(...)   ::perf::MatInfo(::perf::szVGA,   CV_8UC1 , ## __VA_ARGS__)
#define mSVGA8UC1(...)  ::perf::MatInfo(::perf::szSVGA,  CV_8UC1 , ## __VA_ARGS__)
#define mXGA8UC1(...)   ::perf::MatInfo(::perf::szXGA,   CV_8UC1 , ## __VA_ARGS__)
#define mSXGA8UC1(...)  ::perf::MatInfo(::perf::szSXGA,  CV_8UC1 , ## __VA_ARGS__)
#define mnHD8UC1(...)   ::perf::MatInfo(::perf::sznHD,   CV_8UC1 , ## __VA_ARGS__)
#define mqHD8UC1(...)   ::perf::MatInfo(::perf::szqHD,   CV_8UC1 , ## __VA_ARGS__)
#define m720p8UC1(...)  ::perf::MatInfo(::perf::sz720p,  CV_8UC1 , ## __VA_ARGS__)
#define m1080p8UC1(...) ::perf::MatInfo(::perf::sz1080p, CV_8UC1 , ## __VA_ARGS__)

#define mQVGA8UC3(...)  ::perf::MatInfo(::perf::szQVGA,  CV_8UC3 , ## __VA_ARGS__)
#define mVGA8UC3(...)   ::perf::MatInfo(::perf::szVGA,   CV_8UC3 , ## __VA_ARGS__)
#define mSVGA8UC3(...)  ::perf::MatInfo(::perf::szSVGA,  CV_8UC3 , ## __VA_ARGS__)
#define mXGA8UC3(...)   ::perf::MatInfo(::perf::szXGA,   CV_8UC3 , ## __VA_ARGS__)
#define mSXGA8UC3(...)  ::perf::MatInfo(::perf::szSXGA,  CV_8UC3 , ## __VA_ARGS__)
#define mnHD8UC3(...)   ::perf::MatInfo(::perf::sznHD,   CV_8UC3 , ## __VA_ARGS__)
#define mqHD8UC3(...)   ::perf::MatInfo(::perf::szqHD,   CV_8UC3 , ## __VA_ARGS__)
#define m720p8UC3(...)  ::perf::MatInfo(::perf::sz720p,  CV_8UC3 , ## __VA_ARGS__)
#define m1080p8UC3(...) ::perf::MatInfo(::perf::sz1080p, CV_8UC3 , ## __VA_ARGS__)

#define mQVGA8UC4(...)  ::perf::MatInfo(::perf::szQVGA,  CV_8UC4 , ## __VA_ARGS__)
#define mVGA8UC4(...)   ::perf::MatInfo(::perf::szVGA,   CV_8UC4 , ## __VA_ARGS__)
#define mSVGA8UC4(...)  ::perf::MatInfo(::perf::szSVGA,  CV_8UC4 , ## __VA_ARGS__)
#define mXGA8UC4(...)   ::perf::MatInfo(::perf::szXGA,   CV_8UC4 , ## __VA_ARGS__)
#define mSXGA8UC4(...)  ::perf::MatInfo(::perf::szSXGA,  CV_8UC4 , ## __VA_ARGS__)
#define mnHD8UC4(...)   ::perf::MatInfo(::perf::sznHD,   CV_8UC4 , ## __VA_ARGS__)
#define mqHD8UC4(...)   ::perf::MatInfo(::perf::szqHD,   CV_8UC4 , ## __VA_ARGS__)
#define m720p8UC4(...)  ::perf::MatInfo(::perf::sz720p,  CV_8UC4 , ## __VA_ARGS__)
#define m1080p8UC4(...) ::perf::MatInfo(::perf::sz1080p, CV_8UC4 , ## __VA_ARGS__)

#define mQVGA32FC1(...)  ::perf::MatInfo(::perf::szQVGA,  CV_32FC1 , ## __VA_ARGS__)
#define mVGA32FC1(...)   ::perf::MatInfo(::perf::szVGA,   CV_32FC1 , ## __VA_ARGS__)
#define mSVGA32FC1(...)  ::perf::MatInfo(::perf::szSVGA,  CV_32FC1 , ## __VA_ARGS__)
#define mXGA32FC1(...)   ::perf::MatInfo(::perf::szXGA,   CV_32FC1 , ## __VA_ARGS__)
#define mSXGA32FC1(...)  ::perf::MatInfo(::perf::szSXGA,  CV_32FC1 , ## __VA_ARGS__)
#define mnHD32FC1(...)   ::perf::MatInfo(::perf::sznHD,   CV_32FC1 , ## __VA_ARGS__)
#define mqHD32FC1(...)   ::perf::MatInfo(::perf::szqHD,   CV_32FC1 , ## __VA_ARGS__)
#define m720p32FC1(...)  ::perf::MatInfo(::perf::sz720p,  CV_32FC1 , ## __VA_ARGS__)
#define m1080p32FC1(...) ::perf::MatInfo(::perf::sz1080p, CV_32FC1 , ## __VA_ARGS__)

#define mQVGA32FC3(...)  ::perf::MatInfo(::perf::szQVGA,  CV_32FC3 , ## __VA_ARGS__)
#define mVGA32FC3(...)   ::perf::MatInfo(::perf::szVGA,   CV_32FC3 , ## __VA_ARGS__)
#define mSVGA32FC3(...)  ::perf::MatInfo(::perf::szSVGA,  CV_32FC3 , ## __VA_ARGS__)
#define mXGA32FC3(...)   ::perf::MatInfo(::perf::szXGA,   CV_32FC3 , ## __VA_ARGS__)
#define mSXGA32FC3(...)  ::perf::MatInfo(::perf::szSXGA,  CV_32FC3 , ## __VA_ARGS__)
#define mnHD32FC3(...)   ::perf::MatInfo(::perf::sznHD,   CV_32FC3 , ## __VA_ARGS__)
#define mqHD32FC3(...)   ::perf::MatInfo(::perf::szqHD,   CV_32FC3 , ## __VA_ARGS__)
#define m720p32FC3(...)  ::perf::MatInfo(::perf::sz720p,  CV_32FC3 , ## __VA_ARGS__)
#define m1080p32FC3(...) ::perf::MatInfo(::perf::sz1080p, CV_32FC3 , ## __VA_ARGS__)

#define mQVGA32FC4(...)  ::perf::MatInfo(::perf::szQVGA,  CV_32FC4 , ## __VA_ARGS__)
#define mVGA32FC4(...)   ::perf::MatInfo(::perf::szVGA,   CV_32FC4 , ## __VA_ARGS__)
#define mSVGA32FC4(...)  ::perf::MatInfo(::perf::szSVGA,  CV_32FC4 , ## __VA_ARGS__)
#define mXGA32FC4(...)   ::perf::MatInfo(::perf::szXGA,   CV_32FC4 , ## __VA_ARGS__)
#define mSXGA32FC4(...)  ::perf::MatInfo(::perf::szSXGA,  CV_32FC4 , ## __VA_ARGS__)
#define mnHD32FC4(...)   ::perf::MatInfo(::perf::sznHD,   CV_32FC4 , ## __VA_ARGS__)
#define mqHD32FC4(...)   ::perf::MatInfo(::perf::szqHD,   CV_32FC4 , ## __VA_ARGS__)
#define m720p32FC4(...)  ::perf::MatInfo(::perf::sz720p,  CV_32FC4 , ## __VA_ARGS__)
#define m1080p32FC4(...) ::perf::MatInfo(::perf::sz1080p, CV_32FC4 , ## __VA_ARGS__)

#define mODD8U(...) ::perf::MatInfo(::perf::szODD, CV_8UC1 , ## __VA_ARGS__)
#define mODD32F(...) ::perf::MatInfo(::perf::szODD, CV_32FC1 , ## __VA_ARGS__)

class Regression
{
public:
    static Regression add;

    Regression& operator() (const std::string& name, cv::InputArray array, double eps = DBL_EPSILON);

private:
    Regression();
    ~Regression();

    Regression(const Regression&);
    Regression& operator=(const Regression&);

    cv::RNG regRNG;//own random numbers generator to make collection and verification work identical
    std::string storagePath;
    cv::FileStorage storageIn;
    cv::FileStorage storageOut;
    cv::FileNode rootIn;
    std::string currentTestNodeName;

    static std::string getCurrentTestNodeName();
    cv::FileStorage& write();

    static bool isVector(cv::InputArray a);

    void write(cv::InputArray array);
    void write(cv::Mat m);
    void verify(cv::FileNode node, cv::InputArray array, double eps);
    void verify(cv::FileNode node, cv::Mat actual, double eps, std::string argname);
    double getElem(cv::Mat& m, int x, int y, int cn = 0);
};

#define SANITY_CHECK(array) ::perf::Regression::add(#array, array)

typedef struct performance_metrics
{
    size_t bytesIn;
    size_t bytesOut;
    unsigned int samples;
    unsigned int outliers;
    double gmean;
    double gstddev;//stddev for log(time)
    double mean;
    double stddev;
    double median;
    double min;
    double frequency;

    performance_metrics();
} performance_metrics;

class TestBase: public ::testing::Test
{
public:
    TestBase();

protected:
    virtual void SetUp();
    virtual void TearDown();

    enum
    {
        WARMUP_READ,
        WARMUP_WRITE,
        WARMUP_RNG,
        WARMUP_NONE
    };

    void startTimer();
    void stopTimer();

    class _declareHelper
    {
    public:
        _declareHelper& in(cv::InputOutputArray a1, int wtype = WARMUP_READ);
        _declareHelper& in(cv::InputOutputArray a1, cv::InputOutputArray a2, int wtype = WARMUP_READ);
        _declareHelper& in(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, int wtype = WARMUP_READ);
        _declareHelper& in(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, cv::InputOutputArray a4, int wtype = WARMUP_READ);

        _declareHelper& out(cv::InputOutputArray a1, int wtype = WARMUP_WRITE);
        _declareHelper& out(cv::InputOutputArray a1, cv::InputOutputArray a2, int wtype = WARMUP_WRITE);
        _declareHelper& out(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, int wtype = WARMUP_WRITE);
        _declareHelper& out(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, cv::InputOutputArray a4, int wtype = WARMUP_WRITE);

        _declareHelper& iterations(int n);
        _declareHelper& time(double timeLimitSecs);
    private:
        TestBase* test;
        _declareHelper(TestBase* t);
        _declareHelper(const _declareHelper&);
        _declareHelper& operator=(const _declareHelper&);
        friend class TestBase;
    };

    static void warmup(cv::InputOutputArray a, int wtype = WARMUP_READ);

    _declareHelper declare;
    bool next();

    performance_metrics& calcMetrics();
    void reportMetrics(bool toJUnitXML = false);

    virtual void PerfTestBody() = 0;

private:

    performance_metrics metrics;
    void validateMetrics();

    unsigned int getTotalInputSize() const;
    unsigned int getTotalOutputSize() const;

    typedef std::vector<std::pair<int, cv::Size> > SizeVector;
    typedef std::vector<int64> TimeVector;
    SizeVector inputData;
    SizeVector outputData;
    TimeVector times;
    int64 lastTime;
    int64 totalTime;
    int64 timeLimit;
    unsigned int nIters;
    unsigned int currentIter;

    static int64 timeLimitDefault;

    static void warmup(cv::Mat m, int wtype);
    static int getSizeInBytes(cv::InputArray a);
    static cv::Size getSize(cv::InputArray a);
    static void declareArray(SizeVector& sizes, cv::InputOutputArray a, int wtype = 0);

    static int64 _timeadjustment;
    static int64 _calibrate();

    friend class _declareHelper;
};

template<typename T>
class TestBaseWithParam: public TestBase, public ::testing::WithParamInterface<T>
{
};


Regression Regression::add;

Regression::Regression() : regRNG(809564)
{
    //const char *data_path_dir = getenv("OPENCV_TEST_DATA_PATH");

    storagePath = "test.xml";
    if (storageIn.open(storagePath, cv::FileStorage::READ))
        rootIn = storageIn.root();
}

Regression::~Regression()
{
    if (storageIn.isOpened())
        storageIn.release();
    if (storageOut.isOpened())
    {
        if (!currentTestNodeName.empty())
            storageOut << "}";
        storageOut.release();
    }
}

cv::FileStorage& Regression::write()
{
    if (!storageOut.isOpened())
        storageOut.open(storagePath, storageIn.isOpened() ? cv::FileStorage::APPEND : cv::FileStorage::WRITE);
    return storageOut;
}

std::string Regression::getCurrentTestNodeName()
{
    const ::testing::TestInfo* const test_info =
      ::testing::UnitTest::GetInstance()->current_test_info();

    if (test_info == 0)
        return "undefined";

    std::string nodename = std::string(test_info->test_case_name()) + "--" + test_info->name();
    size_t idx = nodename.find_first_of('/');
    if (idx != std::string::npos)
        nodename.erase(idx);

    const char* type_param = test_info->type_param();
    if (type_param != 0)
        (nodename += "--") += type_param;

    const char* value_param = test_info->value_param();
    if (value_param != 0)
        (nodename += "--") += value_param;

    for(size_t i = 0; i < nodename.length(); ++i)
        if (!isalnum(nodename[i]) && '_' != nodename[i])
            nodename[i] = '-';

    return nodename;
}

bool Regression::isVector(cv::InputArray a)
{
    return a.kind() == cv::_InputArray::STD_VECTOR_MAT || a.kind() == cv::_InputArray::STD_VECTOR_VECTOR;
}

double Regression::getElem(cv::Mat& m, int y, int x, int cn)
{
    switch (m.depth())
    {
    case CV_8U: return *(m.ptr<unsigned char>(y, x) + cn);
    case CV_8S: return *(m.ptr<signed char>(y, x) + cn);
    case CV_16U: return *(m.ptr<unsigned short>(y, x) + cn);
    case CV_16S: return *(m.ptr<signed short>(y, x) + cn);
    case CV_32S: return *(m.ptr<signed int>(y, x) + cn);
    case CV_32F: return *(m.ptr<float>(y, x) + cn);
    case CV_64F: return *(m.ptr<double>(y, x) + cn);
    default: return 0;
    }
}

void Regression::write(cv::Mat m)
{
    double min, max;
    cv::minMaxLoc(m, &min, &max);
    write() << "min" << min << "max" << max;

    write() << "last" << "{" << "x" << m.cols-1 << "y" << m.rows-1
        << "val" << getElem(m, m.rows-1, m.cols-1, m.channels()-1) << "}";

    int x, y, cn;
    x = regRNG.uniform(0, m.cols);
    y = regRNG.uniform(0, m.rows);
    cn = regRNG.uniform(0, m.channels());
    write() << "rng1" << "{" << "x" << x << "y" << y;
    if(cn > 0) write() << "cn" << cn;
    write() << "val" << getElem(m, y, x, cn) << "}";

    x = regRNG.uniform(0, m.cols);
    y = regRNG.uniform(0, m.rows);
    cn = regRNG.uniform(0, m.channels());
    write() << "rng2" << "{" << "x" << x << "y" << y;
    if (cn > 0) write() << "cn" << cn;
    write() << "val" << getElem(m, y, x, cn) << "}";
}

void Regression::verify(cv::FileNode node, cv::Mat actual, double eps, std::string argname)
{
    double actualmin, actualmax;
    cv::minMaxLoc(actual, &actualmin, &actualmax);

    ASSERT_NEAR((double)node["min"], actualmin, eps)
            << argname << " has unexpected minimal value";
    ASSERT_NEAR((double)node["max"], actualmax, eps)
            << argname << " has unexpected maximal value";

    cv::FileNode last = node["last"];
    double actualLast = getElem(actual, actual.rows - 1, actual.cols - 1, actual.channels() - 1);
    ASSERT_EQ((int)last["x"], actual.cols - 1)
            << argname << " has unexpected number of columns";
    ASSERT_EQ((int)last["y"], actual.rows - 1)
            << argname << " has unexpected number of rows";
    ASSERT_NEAR((double)last["val"], actualLast, eps)
            << argname << " has unexpected value of last element";

    cv::FileNode rng1 = node["rng1"];
    int x1 = rng1["x"];
    int y1 = rng1["y"];
    int cn1 = rng1["cn"];
    ASSERT_NEAR((double)rng1["val"], getElem(actual, y1, x1, cn1), eps)
            << argname << " has unexpected value of ["<< x1 << ":" << y1 << ":" << cn1 <<"] element";

    cv::FileNode rng2 = node["rng2"];
    int x2 = rng2["x"];
    int y2 = rng2["y"];
    int cn2 = rng2["cn"];
    ASSERT_NEAR((double)rng2["val"], getElem(actual, y2, x2, cn2), eps)
            << argname << " has unexpected value of ["<< x2 << ":" << y2 << ":" << cn2 <<"] element";
}

void Regression::write(cv::InputArray array)
{
    write() << "kind" << array.kind();
    write() << "type" << array.type();
    if (isVector(array))
    {
        int total = array.total();
        int idx = regRNG.uniform(0, total);
        write() << "len" << total;
        write() << "idx" << idx;

        cv::Mat m = array.getMat(idx);

        if (m.total() * m.channels() < 26) //5x5 or smaller
            write() << "val" << m;
        else
            write(m);
    }
    else
    {
        if (array.total() * array.channels() < 26) //5x5 or smaller
            write() << "val" << array.getMat();
        else
            write(array.getMat());
    }
}

void Regression::verify(cv::FileNode node, cv::InputArray array, double eps)
{
    ASSERT_EQ((int)node["kind"], array.kind()) << "Argument " << node.name() << " has unexpected kind";
    ASSERT_EQ((int)node["type"], array.type()) << "Argument " << node.name() << " has unexpected type";

    cv::FileNode valnode = node["val"];
    if (isVector(array))
    {
        ASSERT_EQ((int)node["len"], array.total()) << "Vector " << node.name() << " has unexpected length";
        int idx = node["idx"];

        cv::Mat actual = array.getMat(idx);

        if (valnode.isNone())
        {
            ASSERT_LE((size_t)26, actual.total() * (size_t)actual.channels())
                    << node.name() << "[" <<  idx << "] has unexpected number of elements";
            verify(node, actual, eps, cv::format("%s[%d]", node.name().c_str(), idx));
        }
        else
        {
            cv::Mat expected;
            valnode >> expected;

            ASSERT_EQ(expected.size(), actual.size())
                    << node.name() << "[" <<  idx<< "] has unexpected size";

            cv::Mat diff;
            cv::absdiff(expected, actual, diff);
            if (!cv::checkRange(diff, true, 0, 0, eps))
                FAIL() << "Difference between argument "
                       << node.name() << "[" <<  idx << "] and expected value is bugger than " << eps;
        }
    }
    else
    {
        if (valnode.isNone())
        {
            ASSERT_LE((size_t)26, array.total() * (size_t)array.channels())
                    << "Argument " << node.name() << " has unexpected number of elements";
            verify(node, array.getMat(), eps, "Argument " + node.name());
        }
        else
        {
            cv::Mat expected;
            valnode >> expected;
            cv::Mat actual = array.getMat();

            ASSERT_EQ(expected.size(), actual.size())
                    << "Argument " << node.name() << " has unexpected size";

            cv::Mat diff;
            cv::absdiff(expected, actual, diff);
            if (!cv::checkRange(diff, true, 0, 0, eps))
                FAIL() << "Difference between argument " << node.name()
                       << " and expected value is bugger than " << eps;
        }
    }
}

Regression& Regression::operator() (const std::string& name, cv::InputArray array, double eps)
{
    std::string nodename = getCurrentTestNodeName();

    cv::FileNode n = rootIn[nodename];
    if(n.isNone())
    {
        if (nodename != currentTestNodeName)
        {
            if (!currentTestNodeName.empty())
                write() << "}";
            currentTestNodeName = nodename;

            write() << nodename << "{";
        }
        write() << name << "{";
        write(array);
        write() << "}";
    }
    else
    {
        cv::FileNode this_arg = n[name];
        if (!this_arg.isMap())
            ADD_FAILURE() << "No regression data for " << name << " argument";
        else
            verify(this_arg, array, eps);
    }
    return *this;
}

void randu(cv::Mat& m)
{
    if (m.depth() < CV_32F)
    {
        int minmax[] = {0, 256};
        cv::Mat mr = cv::Mat(m.rows, m.cols * m.elemSize(), CV_8U, m.ptr(), m.step[0]);
        cv::randu(mr, cv::Mat(1, 1, CV_32S, minmax), cv::Mat(1, 1, CV_32S, minmax + 1));
    }
    else
    {
        double minmax[] = {-DBL_MAX, DBL_MAX};
        cv::Mat mr = m.reshape(1);
        cv::randu(mr, cv::Mat(1, 1, CV_64F, minmax), cv::Mat(1, 1, CV_64F, minmax + 1));
    }
}

MatInfo::MatInfo(cv::Size sz, int _type, int _kind, cv::Range _roix, cv::Range _roiy)
{
    size = sz;
    type = _type;
    kind = _kind;
    roix = _roix;
    roiy = _roiy;
}

cv::Mat MatInfo::makeMat() const
{
    cv::Mat m;
    switch(kind)
    {
    case Ones:
        m = cv::Mat::ones(size, type);
        break;
    case Eye:
    case Diag:
        m = cv::Mat::eye(size, type);
        break;
    case Rng:
        m = cv::Mat(size, type);
        randu(m);
        break;
    case Fill:
    case Zeros:
    default:
        m = cv::Mat::zeros(size, type);
        break;
    };

    if (roix.start > 0 || roix.end < m.cols || roiy.start > 0 || roiy.end < m.rows)
        return m(roiy, roix);
    return m;
}

cv::Mat MatInfo::makeMat(double v1, double v2, double v3, double v4) const
{
    cv::Mat m;
    cv::Scalar s(v1,v2,v3,v4);
    switch(kind)
    {
    case Ones:
        m = cv::Mat::ones(size, type);
        break;
    case Eye:
        m = cv::Mat::eye(size, type);
        break;
    case Diag:
        m = cv::Mat::zeros(size, type);
        m.diag().setTo(s);
        break;
    case Rng:
        m = cv::Mat(size, type);
        randu(m);
        break;
    case Fill:
        m = cv::Mat(size, type, s);
        break;
    case Zeros:
    default:
        m = cv::Mat::zeros(size, type);
        break;
    };

    if (roix.start > 0 || roix.end < m.cols || roiy.start > 0 || roiy.end < m.rows)
        return m(roiy, roix);
    return m;
}

performance_metrics::performance_metrics()
{
    bytesIn = 0;
    bytesOut = 0;
    samples = 0;
    outliers = 0;
    gmean = 0;
    gstddev = 0;
    mean = 0;
    stddev = 0;
    median = 0;
    min = 0;
    frequency = 0;
}

#if ANDROID
int64 TestBase::timeLimitDefault = 2 * (int64)cv::getTickFrequency();
#else
int64 TestBase::timeLimitDefault = 1 * (int64)cv::getTickFrequency();
#endif

int64 TestBase::_timeadjustment = TestBase::_calibrate();

int64 TestBase::_calibrate()
{
    class _helper : public ::perf::TestBase
    {
        public:
        performance_metrics& getMetrics() { return calcMetrics(); }
        virtual void TestBody() {}
        virtual void PerfTestBody()
        {
            SetUp();
            for(declare.iterations(1000); startTimer(), next(); stopTimer()){}
        }
    };

    _timeadjustment = 0;
    _helper h;
    h.PerfTestBody();
    double compensation = h.getMetrics().min;
    LOGD("Time comensation is %.0f", compensation);
    return (int64)compensation;
}


TestBase::_declareHelper& TestBase::_declareHelper::in(cv::InputOutputArray a1, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->inputData, a1, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::in(cv::InputOutputArray a1, cv::InputOutputArray a2, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->inputData, a1, wtype);
    TestBase::declareArray(test->inputData, a2, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::in(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->inputData, a1, wtype);
    TestBase::declareArray(test->inputData, a2, wtype);
    TestBase::declareArray(test->inputData, a3, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::in(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, cv::InputOutputArray a4, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->inputData, a1, wtype);
    TestBase::declareArray(test->inputData, a2, wtype);
    TestBase::declareArray(test->inputData, a3, wtype);
    TestBase::declareArray(test->inputData, a4, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::out(cv::InputOutputArray a1, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->outputData, a1, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::out(cv::InputOutputArray a1, cv::InputOutputArray a2, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->outputData, a1, wtype);
    TestBase::declareArray(test->outputData, a2, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::out(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->outputData, a1, wtype);
    TestBase::declareArray(test->outputData, a2, wtype);
    TestBase::declareArray(test->outputData, a3, wtype);
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::out(cv::InputOutputArray a1, cv::InputOutputArray a2, cv::InputOutputArray a3, cv::InputOutputArray a4, int wtype)
{
    if (!test->times.empty()) return *this;
    TestBase::declareArray(test->outputData, a1, wtype);
    TestBase::declareArray(test->outputData, a2, wtype);
    TestBase::declareArray(test->outputData, a3, wtype);
    TestBase::declareArray(test->outputData, a4, wtype);
    return *this;
}

TestBase::_declareHelper::_declareHelper(TestBase* t) : test(t)
{
}

TestBase::TestBase(): declare(this)
{
}

void TestBase::declareArray(SizeVector& sizes, cv::InputOutputArray a, int wtype)
{
    if (!a.empty())
    {
        sizes.push_back(std::pair<int, cv::Size>(getSizeInBytes(a), getSize(a)));
        warmup(a, wtype);
    }
    else if (a.kind() != cv::_InputArray::NONE)
        ADD_FAILURE() << "Uninitialized input/output parameters are not allowed for performance tests";
}

void TestBase::warmup(cv::InputOutputArray a, int wtype)
{
    if (a.empty()) return;
    if (a.kind() != cv::_InputArray::STD_VECTOR_MAT && a.kind() != cv::_InputArray::STD_VECTOR_VECTOR)
        warmup(a.getMat(), wtype);
    else
    {
        size_t total = a.total();
        for (size_t i = 0; i < total; ++i)
            warmup(a.getMat(i), wtype);
    }
}

int TestBase::getSizeInBytes(cv::InputArray a)
{
    if (a.empty()) return 0;
    int total = (int)a.total();
    if (a.kind() != cv::_InputArray::STD_VECTOR_MAT && a.kind() != cv::_InputArray::STD_VECTOR_VECTOR)
        return total * CV_ELEM_SIZE(a.type());

    int size = 0;
    for (int i = 0; i < total; ++i)
        size += (int)a.total(i) * CV_ELEM_SIZE(a.type(i));

    return size;
}

cv::Size TestBase::getSize(cv::InputArray a)
{
    if (a.kind() != cv::_InputArray::STD_VECTOR_MAT && a.kind() != cv::_InputArray::STD_VECTOR_VECTOR)
        return a.size();
    return cv::Size();
}

TestBase::_declareHelper& TestBase::_declareHelper::iterations(int n)
{
    test->times.clear();
    test->times.reserve(n);
    test->nIters = n;
    test->currentIter = (unsigned int)-1;
    return *this;
}

TestBase::_declareHelper& TestBase::_declareHelper::time(double timeLimitSecs)
{
    test->times.clear();
    test->currentIter = (unsigned int)-1;
    test->timeLimit = (int64)(timeLimitSecs * cv::getTickFrequency());
    return *this;
}

bool TestBase::next()
{
    return ++currentIter < nIters && totalTime < timeLimit;
}

void TestBase::warmup(cv::Mat m, int wtype)
{
    switch(wtype)
    {
    case WARMUP_READ:
        cv::sum(m.reshape(1));
        return;
    case WARMUP_WRITE:
        m.reshape(1).setTo(cv::Scalar::all(0));
        return;
    case WARMUP_RNG:
        randu(m);
        return;
    default:
        return;
    }
}

unsigned int TestBase::getTotalInputSize() const
{
    unsigned int res = 0;
    for (SizeVector::const_iterator i = inputData.begin(); i != inputData.end(); ++i)
        res += i->first;
    return res;
}

unsigned int TestBase::getTotalOutputSize() const
{
    unsigned int res = 0;
    for (SizeVector::const_iterator i = outputData.begin(); i != outputData.end(); ++i)
        res += i->first;
    return res;
}

void TestBase::startTimer()
{
    lastTime = cv::getTickCount();
}

void TestBase::stopTimer()
{
    int64 time = cv::getTickCount();
    if (lastTime == 0)
        ADD_FAILURE() << "stopTimer() is called before startTimer()";
    lastTime = time - lastTime;
    totalTime += lastTime;
    lastTime -= _timeadjustment;
    if (lastTime < 0) lastTime = 0;
    times.push_back(lastTime);
    lastTime = 0;
}

performance_metrics& TestBase::calcMetrics()
{
    if ((metrics.samples == (unsigned int)currentIter) || times.size() == 0)
        return metrics;

    metrics.bytesIn = getTotalInputSize();
    metrics.bytesOut = getTotalOutputSize();
    metrics.frequency = cv::getTickFrequency();
    metrics.samples = (unsigned int)times.size();
    metrics.outliers = 0;

    std::sort(times.begin(), times.end());

    //estimate mean and stddev for log(time)
    double gmean = 0;
    double gstddev = 0;
    int n = 0;
    for(TimeVector::const_iterator i = times.begin(); i != times.end(); ++i)
    {
        double x = (double)*i;
        if (x < DBL_EPSILON) continue;
        double lx = log(x);

        ++n;
        double delta = lx - gmean;
        gmean += delta / n;
        gstddev += delta * (lx - gmean);
    }

    gstddev = n > 1 ? sqrt(gstddev / (n - 1)) : 0;

    TimeVector::const_iterator start = times.begin();
    TimeVector::const_iterator end = times.end();

    //filter outliers assuming log-normal distribution
    //http://stackoverflow.com/questions/1867426/modeling-distribution-of-performance-measurements
    int offset = 0;
    if (gstddev > DBL_EPSILON)
    {
        double minout = exp(gmean - 3 * gstddev);
        double maxout = exp(gmean + 3 * gstddev);
        while(*start < minout) ++start, ++metrics.outliers, ++offset;
        do --end, ++metrics.outliers; while(*end > maxout);
        ++end, --metrics.outliers;
    }

    metrics.min = (double)*start;
    //calc final metrics
    n = 0;
    gmean = 0;
    gstddev = 0;
    double mean = 0;
    double stddev = 0;
    int m = 0;
    for(; start != end; ++start)
    {
        double x = (double)*start;
        if (x > DBL_EPSILON)
        {
            double lx = log(x);
            ++m;
            double gdelta = lx - gmean;
            gmean += gdelta / m;
            gstddev += gdelta * (lx - gmean);
        }
        ++n;
        double delta = x - mean;
        mean += delta / n;
        stddev += delta * (x - mean);
    }

    metrics.mean = mean;
    metrics.gmean = exp(gmean);
    metrics.gstddev = m > 1 ? sqrt(gstddev / (m - 1)) : 0;
    metrics.stddev = n > 1 ? sqrt(stddev / (n - 1)) : 0;
    metrics.median = n % 2
            ? (double)times[offset + n / 2]
            : 0.5 * (times[offset + n / 2] + times[offset + n / 2 - 1]);

    return metrics;
}

void TestBase::validateMetrics()
{
    performance_metrics& m = calcMetrics();

    if (HasFailure()) return;

    ASSERT_GE(m.samples, 1u)
      << "No time measurements was performed.\nstartTimer() and stopTimer() commands are required for performance tests.";

    EXPECT_GE(m.samples, 10u)
      << "Only a few samples are collected.\nPlease increase number of iterations or/and time limit to get reliable performance measurements.";

    if (m.gstddev > DBL_EPSILON)
    {
        EXPECT_GT(/*m.gmean * */1., /*m.gmean * */ 2 * sinh(m.gstddev))
          << "Test results are not reliable ((mean-sigma,mean+sigma) deviation interval is bigger than measured time interval).";
    }

    EXPECT_LE(m.outliers, std::max(m.samples * 0.06, 1.))
      << "Test results are not reliable (too many outliers).";
}

void TestBase::reportMetrics(bool toJUnitXML)
{
    performance_metrics& m = calcMetrics();

    if (toJUnitXML)
    {
        RecordProperty("bytesIn", (int)m.bytesIn);
        RecordProperty("bytesOut", (int)m.bytesOut);
        RecordProperty("samples", (int)m.samples);
        RecordProperty("outliers", (int)m.outliers);
        RecordProperty("frequency", cv::format("%.0f", m.frequency).c_str());
        RecordProperty("min", cv::format("%.0f", m.min).c_str());
        RecordProperty("median", cv::format("%.0f", m.median).c_str());
        RecordProperty("gmean", cv::format("%.0f", m.gmean).c_str());
        RecordProperty("gstddev", cv::format("%.6f", m.gstddev).c_str());
        RecordProperty("mean", cv::format("%.0f", m.mean).c_str());
        RecordProperty("stddev", cv::format("%.0f", m.stddev).c_str());
    }
    else
    {
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        const char* type_param = test_info->type_param();
        const char* value_param = test_info->value_param();

        LOGD("");
        if (type_param)  LOGD("type      =%11s", type_param);
        if (value_param) LOGD("param     =%11s", value_param);

        LOGD("bytesIn   =%11lu", m.bytesIn);
        LOGD("bytesOut  =%11lu", m.bytesOut);
        LOGD("samples   =%11u",  m.samples);
        LOGD("outliers  =%11u",  m.outliers);
        LOGD("frequency =%11.0f", m.frequency);
        LOGD("min       =%11.0f = %.2fms", m.min, m.min * 1e3 / m.frequency);
        LOGD("median    =%11.0f = %.2fms", m.median, m.median * 1e3 / m.frequency);
        LOGD("gmean     =%11.0f = %.2fms", m.gmean, m.gmean * 1e3 / m.frequency);
        LOGD("gstddev   =%11.8f = %.2fms for 97%% dispersion interval", m.gstddev, m.gmean * 2 * sinh(m.gstddev * 3) * 1e3 / m.frequency);
        LOGD("mean      =%11.0f = %.2fms", m.mean, m.mean * 1e3 / m.frequency);
        LOGD("stddev    =%11.0f = %.2fms", m.stddev, m.stddev * 1e3 / m.frequency);
    }
}

void TestBase::SetUp()
{
    lastTime = 0;
    totalTime = 0;
    nIters = (unsigned int)-1;
    currentIter = (unsigned int)-1;
    timeLimit = timeLimitDefault;
    times.clear();
    cv::theRNG().state = 4321;//TODO: make seed configurable
}

void TestBase::TearDown()
{
    validateMetrics();
    reportMetrics(!HasFailure());
}

#define PERF_PROXY_NAMESPACE_NAME_(test_case_name, test_name) \
  test_case_name##_##test_name##_perf_namespace_proxy

#define PERF_TEST(test_case_name, test_name)\
    namespace PERF_PROXY_NAMESPACE_NAME_(test_case_name, test_name) {\
     class TestBase {/*compile error for this class means that you are trying to use perf::TestBase as a fixture*/};\
     class test_case_name : public ::perf::TestBase {\
      public:\
       test_case_name() {}\
      protected:\
       virtual void PerfTestBody();\
     };\
     TEST_F(test_case_name, test_name){\
      try {\
       PerfTestBody();\
      }catch(cv::Exception e) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws:\n  " << e.what(); }\
      catch(...) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws."; }\
     }\
    }\
    void PERF_PROXY_NAMESPACE_NAME_(test_case_name, test_name)::test_case_name::PerfTestBody()

#define PERF_TEST_F(fixture, testname) \
    namespace PERF_PROXY_NAMESPACE_NAME_(fixture, testname) {\
     class TestBase {/*compile error for this class means that you are trying to use perf::TestBase as a fixture*/};\
     class fixture : public ::fixture {\
      public:\
       fixture() {}\
      protected:\
       virtual void PerfTestBody();\
     };\
     TEST_F(fixture, testname){\
      try {\
       PerfTestBody();\
      }catch(cv::Exception e) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws:\n  " << e.what(); }\
      catch(...) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws."; }\
     }\
    }\
    void PERF_PROXY_NAMESPACE_NAME_(fixture, testname)::fixture::PerfTestBody()

#define PERF_TEST_P(fixture, name, params)  \
    class fixture##_##name : public ::fixture {\
     public:\
      fixture##_##name() {}\
     protected:\
      virtual void PerfTestBody();\
    };\
    TEST_P(fixture##_##name, name /*perf*/){\
     try {\
      PerfTestBody();\
     }catch(cv::Exception e) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws:\n  " << e.what(); }\
     catch(...) { FAIL() << "Expected: PerfTestBody() doesn't throw an exception.\n  Actual: it throws."; }\
    }\
    INSTANTIATE_TEST_CASE_P(/*none*/, fixture##_##name, params);\
    void fixture##_##name::PerfTestBody()


#define TEST_CYCLE(n) for(declare.iterations(n); startTimer(), next(); stopTimer())
#define SIMPLE_TEST_CYCLE() for(; startTimer(), next(); stopTimer())

}//namespace perf

//printer functions for cv classes
namespace cv {

void PrintTo(const Size& sz, ::std::ostream* os)
{
    *os << "Size:" << sz.width << "x" << sz.height;
}

}  // namespace cv

//printer functions for perf classes
namespace perf {


void PrintTo(const MatInfo& mi, ::std::ostream* os)
{
    *os << "MatInfo:";
    switch(mi.kind)
    {
    case MatInfo::Ones:
        *os << "ones";
        break;
    case MatInfo::Eye:
        *os << "eye";
        break;
    case MatInfo::Diag:
        *os << "diag";
        break;
    case MatInfo::Rng:
        *os << "rng";
        break;
    case MatInfo::Fill:
        //*os << "fill";
        break;
    case MatInfo::Zeros:
        *os << "zeros";
        break;
    default:
        break;
    };

    switch (CV_MAT_DEPTH(mi.type))
    {
    case CV_8U:
        *os << "8UC";
        break;
    case CV_8S:
        *os << "8SC";
        break;
    case CV_16U:
        *os << "16UC";
        break;
    case CV_16S:
        *os << "16SC";
        break;
    case CV_32S:
        *os << "32SC";
        break;
    case CV_32F:
        *os << "32FC";
        break;
    case CV_64F:
        *os << "64FC";
        break;
    default:
        *os << "XXC";
        break;
    };
    *os << CV_MAT_CN(mi.type);

    *os << "x";

    if (mi.size == szQVGA)
        *os << "QVGA";
    else if (mi.size == szVGA)
        *os << "VGA";
    else if (mi.size == szSVGA)
        *os << "SVGA";
    else if (mi.size == szXGA)
        *os << "XGA";
    else if (mi.size == szSXGA)
        *os << "SXGA";
    else if (mi.size == sznHD)
        *os << "nHD";
    else if (mi.size == szqHD)
        *os << "qHD";
    else if (mi.size == sz720p)
        *os << "720p";
    else if (mi.size == sz1080p)
        *os << "1080p";
    else
        *os << mi.size.width << "x" << mi.size.height;

    if (mi.roix.start > 0 || mi.roix.end < mi.size.width || mi.roiy.start > 0 || mi.roiy.end < mi.size.height)
    {
        cv::Range x = mi.roix & cv::Range(0, mi.size.width);
        cv::Range y = mi.roiy & cv::Range(0, mi.size.height);
        *os << "{" << x.start << "," << y.start << ";" << x.end << "," << y.end << "}";
    }
}

}  // namespace perf

/*PERF_TEST(math, add)
{
    cv::Size sz = ::perf::sz720p;
    cv::Mat b(sz, CV_8U, cv::Scalar(10));
    cv::Mat a(sz, CV_8U, cv::Scalar(20));
    cv::Mat c(sz, CV_8U, cv::Scalar(0));

    declare.in(a, b, WARMUP_RNG)
        .out(c, WARMUP_RNG)
        .time(0.5);

    SIMPLE_TEST_CYCLE() cv::add(a, b, c);
}*/

typedef perf::TestBaseWithParam<perf::iMat> math;

PERF_TEST_P(math, add, ::testing::Values(mVGA8UC1(), mqHD8UC1(), mODD8U(), mVGA32FC4(), mqHD8UC4(), m720p8UC4(), m720p32FC1(), mODD32F())) {
    cv::Mat a = GetParam().makeMat(20,21,22,23);
    cv::Mat b = GetParam().makeMat(10,9,8,7);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::add(a, b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, sub, ::testing::Values(mVGA8UC1(), mqHD8UC1(), mODD8U(), mVGA32FC4(), mqHD8UC4(), m720p8UC4(), m720p32FC1(), mODD32F())) {
    cv::Mat a = GetParam().makeMat(20,21,22,23);
    cv::Mat b = GetParam().makeMat(10,9,8,7);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::subtract(a, b, c);

    SANITY_CHECK(c);
}
