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
protected:
    void declareIn(cv::Mat m, int wtype = WARMUP_READ);
    void declareOut(cv::Mat m, int wtype = WARMUP_WRITE);
    void warmup(cv::Mat m, int wtype = WARMUP_READ);

    enum
    {
        WARMUP_READ,
        WARMUP_WRITE,
        WARMUP_WRITERNG,
        WARMUP_NONE
    };

    virtual void SetUp();
    virtual void TearDown();

    void startTimer();
    void stopTimer();

    void declareIterations(int n)
    {
        times.clear();
        times.reserve(n);
    }

    int getInputSize() const;
    int getOutputSize() const;
    void reportTestResults(bool toJUnitXML = false);

private:
    typedef std::vector<std::pair<int, cv::Size> > SizeVector;
    typedef std::vector<int64> TimeVector;
    SizeVector inputData;
    SizeVector outputData;
    TimeVector times;
    int64 lastTime;
};

class TestBase2: public TestBase, public ::testing::WithParamInterface<cv::Size>
{
public:
};


void TestBase::declareIn(cv::Mat m, int wtype)
{
    if (!times.empty()) return;
    int bytes = m.total() * m.elemSize();
    inputData.push_back(std::pair<int, cv::Size>(bytes, m.size()));
    warmup(m, wtype);
}

void TestBase::declareOut(cv::Mat m, int wtype)
{
    if (!times.empty()) return;
    int bytes = m.total() * m.elemSize();
    outputData.push_back(std::pair<int, cv::Size>(bytes, m.size()));
    warmup(m, wtype);
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

int TestBase::getInputSize() const
{
    int res = 0;
    for (SizeVector::const_iterator i = inputData.begin(); i != inputData.end(); ++i)
        res += i->first;
    return res;
}

int TestBase::getOutputSize() const
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
    times.push_back(time - lastTime);
    lastTime = 0;
}

void TestBase::reportTestResults(bool toJUnitXML)
{
    int bytesin = getInputSize();
    int bytesout = getOutputSize();
    double freq = cv::getTickFrequency();

    if (bytesin > 0)
    {
        if (toJUnitXML)
            RecordProperty("bytesin", bytesin);
        else
            LOGD("bytesin=%d", bytesin);
    }
    if (bytesout > 0)
    {
        if (toJUnitXML)
            RecordProperty("bytesout", bytesout);
        else
            LOGD("bytesout=%d", bytesout);
    }

    if (toJUnitXML)
        RecordProperty("frequency", cv::format("%.0f", freq).c_str());
    else
        LOGD("frequency = %.0f", freq);
}

void TestBase::SetUp()
{
    lastTime = 0;
    times.clear();
}

void TestBase::TearDown()
{
    if (times.size() < 1)
        FAIL() << "No time measurements was performed. startTimer() and stopTimer() commands are required for performance tests.";

    reportTestResults(true);
    reportTestResults(false);
}

#define PERF_TEST(fixture, testname) TEST_F(fixture, testname)
#define PERF_TEST_P(fixture, testname) TEST_P(fixture, testname)
#define INSTANTIATE_PERF_TEST_P(fixture, params) INSTANTIATE_TEST_CASE_P(/*none*/, fixture, params)
#define INSTANTIATE_PERF_TEST_CASE_P(case_name, fixture, params) INSTANTIATE_TEST_CASE_P(case_name, fixture, params)

#define TEST_CYCLE(n) declareIterations(n);for(int _it_counter = 0; startTimer(), _it_counter < n; stopTimer(), _it_counter++ )
#define SIMPLE_TEST_CYCLE() TEST_CYCLE(10)

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

    declareIn(a);
    declareIn(b);
    declareOut(c);

    SIMPLE_TEST_CYCLE()
    {
        cv::add(a, b, c);
    }
}

PERF_TEST_P(AddTest2, DoesBlah) {
  cv::Size s = GetParam();
  printf("%dx%d\n", s.width, s.height);
  printf("%s\n", ::testing::PrintToString(s).c_str());
}

//INSTANTIATE_PERF_TEST_P(AddTest2, SZ_ALL_HD);
//INSTANTIATE_PERF_TEST_CASE_P(qqq, AddTest2, SZ_ALL_GA);

