#include "perf_precomp.hpp"
#include "perf_testsets.h"

using namespace cv;
typedef perf::TestBaseWithParam<perf::iMat> math;

PERF_TEST_P(math, bitwise_not, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_not(a, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, bitwise_and, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_and(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, bitwise_or, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_or(a,b, c);

    SANITY_CHECK(c);
}
PERF_TEST_P(math, bitwise_xor, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_xor(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, bitwise_and__Scalar, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_and(a,(InputArray)b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, bitwise_or__Scalar, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_or(a,(InputArray)b, c);

    SANITY_CHECK(c);
}
PERF_TEST_P(math, bitwise_xor__Scalar, ::testing::Values( TESTSET_1 )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::bitwise_xor(a,(InputArray)b, c);

    SANITY_CHECK(c);
}
