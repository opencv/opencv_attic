#include "perf_precomp.hpp"
#include "perf_testsets.h"

using namespace cv;
typedef perf::TestBaseWithParam<perf::iMat> math;

PERF_TEST_P(math, abs, ::testing::Values( TESTSET_1 )) 
{
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, ::perf::TestBase::WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) c=cv::abs(a);

    SANITY_CHECK(c);
}

