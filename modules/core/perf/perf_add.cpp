#include "perf_precomp.hpp"

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
