#include "precomp.hpp"

using namespace perf;

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

/*****************************************************************************************\
*                                   ::perf::MatInfo
\*****************************************************************************************/
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


/*****************************************************************************************\
*                                   ::perf::Regression
\*****************************************************************************************/

Regression& Regression::instance()
{
    static Regression single;
    return single;
}

Regression& Regression::add(const std::string& name, cv::InputArray array, double eps)
{
    return instance()(name, array, eps);
}

void Regression::Init(const std::string& testSuitName, const std::string& ext)
{
    instance().init(testSuitName, ext);
}

void Regression::init(const std::string& testSuitName, const std::string& ext)
{
    if (!storageInPath.empty())
    {
        LOGE("Subsequent initialisation of Regression utility is not allowed.");
        return;
    }

    const char *data_path_dir = getenv("OPENCV_TEST_DATA_PATH");
    const char *path_separator = "/";

    if (data_path_dir)
    {
        int len = strlen(data_path_dir)-1;
        if (len < 0) len = 0;
        std::string path_base = (data_path_dir[0] == 0 ? std::string(".") : std::string(data_path_dir))
                + (data_path_dir[len] == '/' || data_path_dir[len] == '\\' ? "" : path_separator)
                + "perf"
                + path_separator;

        storageInPath = path_base + testSuitName + ext;
        storageOutPath = path_base + testSuitName;
    }
    else
    {
        storageInPath = testSuitName + ext;
        storageOutPath = testSuitName;
    }

    if (storageIn.open(storageInPath, cv::FileStorage::READ))
    {
        rootIn = storageIn.root();
        if (storageInPath.length() > 3 && storageInPath.substr(storageInPath.length()-3) == ".gz")
            storageOutPath += "_new";
        storageOutPath += ext;
    }
    else
        storageOutPath = storageInPath;
}

Regression::Regression() : regRNG(cv::getTickCount())//this rng should be really random
{
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
    if (!storageOut.isOpened() && !storageOutPath.empty())
    {
        int mode = (storageIn.isOpened() && storageInPath == storageOutPath)
                ? cv::FileStorage::APPEND : cv::FileStorage::WRITE;
        storageOut.open(storageOutPath, mode);
        if (!storageOut.isOpened())
        {
            LOGE("Could not open \"%s\" file for writing", storageOutPath.c_str());
            storageOutPath.clear();
        }
        else if (mode == cv::FileStorage::WRITE && !rootIn.empty())
        {
            //TODO: write content of rootIn node into the storageOut
        }
    }
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
        ASSERT_EQ((int)node["len"], (int)array.total()) << "Vector " << node.name() << " has unexpected length";
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


/*****************************************************************************************\
*                                ::perf::performance_metrics
\*****************************************************************************************/
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


/*****************************************************************************************\
*                                   ::perf::TestBase
\*****************************************************************************************/
int64 TestBase::timeLimitDefault = 0;
int64 TestBase::_timeadjustment = 0;

void TestBase::Init()
{
#if ANDROID
    timeLimitDefault = 2 * (int64)cv::getTickFrequency();
#else
    timeLimitDefault = 1 * (int64)cv::getTickFrequency();
#endif

    _timeadjustment = _calibrate();
}

int64 TestBase::_calibrate()
{
    class _helper : public ::perf::TestBase
    {
        public:
        performance_metrics& getMetrics() { return calcMetrics(); }
        virtual void TestBody() {}
        virtual void PerfTestBody()
        {
            //the whole system warmup
            SetUp();
            cv::Mat a(2048, 2048, CV_32S, cv::Scalar(1));
            cv::Mat b(2048, 2048, CV_32S, cv::Scalar(2));
            declare.time(30);
            double s = 0;
            for(declare.iterations(20); startTimer(), next(); stopTimer())
                s+=a.dot(b);
            declare.time(s);

            //self calibration
            SetUp();
            for(declare.iterations(1000); startTimer(), next(); stopTimer()){}
        }
    };

    _timeadjustment = 0;
    _helper h;
    h.PerfTestBody();
    double compensation = h.getMetrics().min;
    LOGD("Time compensation is %.0f", compensation);
    return (int64)compensation;
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

#if defined(ANDROID) && defined(USE_ANDROID_LOGGING)
        LOGD("[ FAILED   ] %s.%s", test_info->test_case_name(), test_info->name());
#else
        LOGD(" ");
#endif

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
    //TODO: make seed configurable
    cv::theRNG().state = 809564;//this rng should generate same numbers for each run
}

void TestBase::TearDown()
{
    validateMetrics();
    reportMetrics(!HasFailure());
}

/*****************************************************************************************\
*                          ::perf::TestBase::_declareHelper
\*****************************************************************************************/
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

/*****************************************************************************************\
*                                  ::perf::PrintTo
\*****************************************************************************************/
namespace perf
{

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

} //namespace perf

/*****************************************************************************************\
*                                  ::cv::PrintTo
\*****************************************************************************************/
namespace cv {

void PrintTo(const Size& sz, ::std::ostream* os)
{
    *os << "Size:" << sz.width << "x" << sz.height;
}

}  // namespace cv
