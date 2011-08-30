#include "perf_precomp.hpp"
#include "perf_testsets.h"

using namespace cv;
typedef perf::TestBaseWithParam<perf::iMat> math;

PERF_TEST_P(math, min, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::min(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, max, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::max(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, min__Scalar, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::min(a,(InputArray)b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, max__Scalar, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::max(a,(InputArray)b, c);

    SANITY_CHECK(c);
}
