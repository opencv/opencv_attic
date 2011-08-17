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
#define SZ_ALL  ::testing::Values(::perf::szQVGA, ::perf::szVGA, ::perf::szSVGA, ::perf::szXGA, ::perf::szSXGA, ::perf::sznHD, ::perf::szqHD, ::perf::sz720p, ::perf::sz1080p)

#define SZ_TYPICAL  ::testing::Values(::perf::szVGA, ::perf::szqHD, ::perf::sz720p, cv::Size(127,63))

typedef struct performance_metrics
{
    size_t bytesIn;
    size_t bytesOut;
    unsigned int samples;
    unsigned int outliers;
    double gmean;
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

class TestBase2: public TestBase, public ::testing::WithParamInterface<cv::Size>
{
public:
    virtual void PerfTestBody() {}
};

performance_metrics::performance_metrics()
{
    bytesIn = 0;
    bytesOut = 0;
    samples = 0;
    outliers = 0;
    gmean = 0;
    mean = 0;
    stddev = 0;
    median = 0;
    min = 0;
    frequency = 0;
}

#if ANDROID
int64 TestBase::timeLimitDefault = 20 * (int64)cv::getTickFrequency();
#else
int64 TestBase::timeLimitDefault = 10 * (int64)cv::getTickFrequency();
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

TestBase::_declareHelper& TestBase::_declareHelper::time(double timeLimitSecs)
{
    test->times.clear();
    test->currentIter = (unsigned int)-1;
    test->timeLimit = (int64)(timeLimitSecs * cv::getTickFrequency());
    return *this;
}

bool TestBase::next()
{
    //printf("%u %u %d\n", currentIter, nIters, currentIter - nIters);
    //if (++currentIter >= nIters) return false;
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
        return;
    }
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

    //estimate mean and stddev
    double mean = 0;
    double stddev = 0;
    int n = 0;
    for(TimeVector::const_iterator i = times.begin(); i != times.end(); ++i)
    {
        double x = (double)*i;
        n = n + 1;
        double delta = x - mean;
        mean += delta / n;
        stddev += delta * (x - mean);
    }
    stddev = n > 1 ? sqrt(stddev / (n - 1)) : 0;

    TimeVector::const_iterator start = times.begin();
    TimeVector::const_iterator end = times.end();

    //filter outliers
    int offset = 0;
    if (stddev > DBL_EPSILON)
    {
        double minout = mean - 3 * stddev;
        double maxout = mean + 5 * stddev;
        while(*start < minout) ++start, ++metrics.outliers, ++offset;
        do --end, ++metrics.outliers; while(*end > maxout);
        ++end, --metrics.outliers;
    }

    metrics.min = (double)*start;
    //calc final metrics
    n = 0;
    mean = 0;
    stddev = 0;
    double gmean = 0.0;
    for(; start != end; ++start)
    {
        double x = (double)*start;
        if(x > DBL_EPSILON) gmean += log(x);
        n = n + 1;
        double delta = x - mean;
        mean += delta / n;
        stddev += delta * (x - mean);
    }

    metrics.mean = mean;
    metrics.gmean = exp(gmean / n);
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

    if (m.stddev > DBL_EPSILON)
    {
        EXPECT_GT(m.min, 2 * m.stddev)
          << "Test results are not reliable (deviation is bigger than a half of measured time interval).";
    }

    EXPECT_LE(m.outliers, m.samples * 0.03)
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
        RecordProperty("mean", cv::format("%.0f", m.mean).c_str());
        RecordProperty("stddev", cv::format("%.0f", m.stddev).c_str());
    }
    else
    {
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        const char* type_param = test_info->type_param();
        const char* value_param = test_info->value_param();

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
}

void TestBase::TearDown()
{
//    reportMetrics();

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
    *os << sz.width << "x" << sz.height;
}

}  // namespace cv

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

typedef perf::TestBase2 math;

PERF_TEST_P( math, add8u, SZ_TYPICAL) {
    cv::Size sz = GetParam();

    cv::Mat a(sz, CV_8U, cv::Scalar(20));
    cv::Mat b(sz, CV_8U, cv::Scalar(10));
    cv::Mat c(sz, CV_8U, cv::Scalar(0));

    declare.in(a, b, WARMUP_RNG)
        .out(c, WARMUP_RNG)
        .time(0.5);

    TEST_CYCLE(100) cv::add(a, b, c);
}

PERF_TEST_P( math, add8uc4, SZ_TYPICAL) {
    cv::Size sz = GetParam();

    cv::Mat a(sz, CV_8UC4, cv::Scalar(20));
    cv::Mat b(sz, CV_8UC4, cv::Scalar(10));
    cv::Mat c(sz, CV_8UC4, cv::Scalar(0));

    declare.in(a, b, WARMUP_RNG)
        .out(c, WARMUP_RNG)
        .time(0.5);

    TEST_CYCLE(100) cv::add(a, b, c);
}

PERF_TEST_P( math, DISABLED_sub8u, SZ_TYPICAL) {
    cv::Size sz = GetParam();

    cv::Mat a(sz, CV_8U, cv::Scalar(20));
    cv::Mat b(sz, CV_8U, cv::Scalar(10));
    cv::Mat c(sz, CV_8U, cv::Scalar(0));

    declare.in(a, b, WARMUP_RNG)
        .out(c, WARMUP_RNG)
        .time(0.5);

    TEST_CYCLE(100) cv::subtract(a, b, c);
}

PERF_TEST_P( math, sub8uc4, SZ_TYPICAL) {
    cv::Size sz = GetParam();

    cv::Mat a(sz, CV_8UC4, cv::Scalar(20));
    cv::Mat b(sz, CV_8UC4, cv::Scalar(10));
    cv::Mat c(sz, CV_8UC4, cv::Scalar(0));

    declare.in(a, b, WARMUP_RNG)
        .out(c, WARMUP_RNG)
        .time(0.5);
    ADD_FAILURE() << "just for test";
    FAIL() << "just for fan";

    TEST_CYCLE(100) cv::subtract(a, b, c);
}

