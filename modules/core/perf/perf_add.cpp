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

#define SZ_ALL_VGA ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA)
#define SZ_ALL_GA  ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA, ::perf::szXGA, ::perf::szSXGA)
#define SZ_ALL_HD  ::testing::Values(::perf::sznHD, ::perf::szqHD, ::perf::sz720p, ::perf::sz1080p)

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
        WARMUP_WRITERNG,
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
        _declareHelper& time(int timeLimitSecs);
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

    int getTotalInputSize() const;
    int getTotalOutputSize() const;
    void reportResults(bool toJUnitXML = false);

private:
    typedef std::vector<std::pair<int, cv::Size> > SizeVector;
    typedef std::vector<int64> TimeVector;
    SizeVector inputData;
    SizeVector outputData;
    TimeVector times;
    int64 lastTime;
    int64 totalTime;
    int64 timeLimit;
    size_t nIters;
    size_t currentIter;

    static int64 timeLimitDefault;

    static void warmup(cv::Mat m, int wtype);
    static int getSizeInBytes(cv::InputArray a);
    static cv::Size getSize(cv::InputArray a);
    static void declareArray(SizeVector& sizes, cv::InputOutputArray a, int wtype = 0);

    friend class _declareHelper;
};

class TestBase2: public TestBase, public ::testing::WithParamInterface<cv::Size>
{
public:
};

#if ANDROID
int64 TestBase::timeLimitDefault = 20 * (int64)cv::getTickFrequency();
#else
int64 TestBase::timeLimitDefault = 10 * (int64)cv::getTickFrequency();
#endif

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
        ADD_FAILURE() << "Uninitialized input/output parameters are not allowed for performance tests.";
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

TestBase::_declareHelper& TestBase::_declareHelper::time(int timeLimitSecs)
{
    test->times.clear();
    test->currentIter = (unsigned int)-1;
    test->timeLimit = timeLimitSecs * (int64)cv::getTickFrequency();
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
    case WARMUP_WRITERNG:
        {
            cv::Mat mr = m.reshape(1);
            cv::randu(mr, cv::Scalar::all(-DBL_MAX), cv::Scalar::all(DBL_MAX));
        }
        return;
    default:
        return;
    }
}

int TestBase::getTotalInputSize() const
{
    int res = 0;
    for (SizeVector::const_iterator i = inputData.begin(); i != inputData.end(); ++i)
        res += i->first;
    return res;
}

int TestBase::getTotalOutputSize() const
{
    int res = 0;
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
    times.push_back(lastTime);
    totalTime += lastTime;
    lastTime = 0;
}

void TestBase::reportResults(bool toJUnitXML)
{
    int bytesin = getTotalInputSize();
    int bytesout = getTotalOutputSize();
    double freq = cv::getTickFrequency();

    if (bytesin > 0)
    {
        if (toJUnitXML)
            RecordProperty("bytesin", bytesin);
        else
            LOGD("bytesin  = %d", bytesin);
    }
    if (bytesout > 0)
    {
        if (toJUnitXML)
            RecordProperty("bytesout", bytesout);
        else
            LOGD("bytesout = %d", bytesout);
    }

    if (toJUnitXML)
        RecordProperty("frequency", cv::format("%.0f", freq).c_str());
    else
        LOGD("frequency = %.0f", freq);

    double gmean = 0.0;
    double mean = 0;
    double stddev = 0;
    int64 median = 0;
    int64 min = 0;
    if (times.size() > 0)
    {
        std::sort(times.begin(), times.end());

        min = times[0];
        median = times[times.size()/2];
        if ((times.size() & 1) == 0)
            median = (median + times[times.size()/2 - 1]) / 2;

        int n = 0;
        for(TimeVector::const_iterator i = times.begin(); i != times.end(); ++i)
        {
            double x = (double)*i;
            if(x > DBL_EPSILON)
                gmean += log(x);

            n = n + 1;
            double delta = x - mean;
            mean += delta / n;
            stddev += delta * (x - mean);
        }

        gmean = exp(gmean / times.size());
        if (n > 1)
            stddev = sqrt(stddev / (n - 1));
        else
            stddev = 0;
    }

    if (toJUnitXML)
    {
        RecordProperty("samples", (int)times.size());
        RecordProperty("min", cv::format("%lld", min).c_str());
        RecordProperty("median", cv::format("%lld", median).c_str());
        RecordProperty("gmean", cv::format("%.0f", gmean).c_str());
        RecordProperty("mean", cv::format("%.0f", mean).c_str());
        RecordProperty("stddev", cv::format("%.0f", stddev).c_str());
    }
    else
    {
        LOGD("samples = %7d", times.size());
        LOGD("min     = %7lld = %0.2fms", min, min * 1000.0 / freq);
        LOGD("median  = %7lld = %0.2fms", median, median * 1000.0 / freq);
        LOGD("gmean   = %7.0f = %0.2fms", gmean, gmean * 1000.0 / freq);
        LOGD("mean    = %7.0f = %0.2fms", mean, mean * 1000.0 / freq);
        LOGD("stddev  = %7.0f = %0.2fms", stddev, stddev * 1000.0 / freq);
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
}

void TestBase::TearDown()
{
    if (times.size() < 1)
        FAIL() << "No time measurements was performed. startTimer() and stopTimer() commands are required for performance tests.";

    reportResults(true);
    reportResults(false);
}

#define PERF_TEST(fixture, testname) TEST_F(fixture, testname)
#define PERF_TEST_P(fixture, testname) TEST_P(fixture, testname)
#define INSTANTIATE_PERF_TEST_P(fixture, params) INSTANTIATE_TEST_CASE_P(/*none*/, fixture, params)
#define INSTANTIATE_PERF_TEST_CASE_P(case_name, fixture, params) INSTANTIATE_TEST_CASE_P(case_name, fixture, params)

#define TEST_CYCLE(n) for(declare().iterations(n); startTimer(), next(); stopTimer())
#define SIMPLE_TEST_CYCLE() for(; startTimer(), next(); stopTimer())

}//namespace perf

//printer functions for cv classes
namespace cv {

void PrintTo(const Size& sz, ::std::ostream* os)
{
    *os << sz.width << "x" << sz.height;
}

}  // namespace cv

typedef perf::TestBase AddTest;
typedef perf::TestBase2 AddTest2;

PERF_TEST(AddTest, third)
{
    cv::Mat b(perf::sz720p, CV_8U, cv::Scalar(10));
    cv::Mat a(perf::sz720p, CV_8U, cv::Scalar(20));
    cv::Mat c(perf::sz720p, CV_8U, cv::Scalar(0));

    declare.in(a, b).out(c).maxTime(2);

    SIMPLE_TEST_CYCLE() cv::add(a, b, c);
//    while(next())
//    {
//        startTimer();
//        cv::add(a, b, c);
//        stopTimer();
//    }
}

PERF_TEST_P(AddTest2, DoesBlah) {
  cv::Size s = GetParam();
  printf("%dx%d\n", s.width, s.height);
  printf("%s\n", ::testing::PrintToString(s).c_str());
}

//INSTANTIATE_PERF_TEST_P(AddTest2, SZ_ALL_HD);
//INSTANTIATE_PERF_TEST_CASE_P(qqq, AddTest2, SZ_ALL_GA);

