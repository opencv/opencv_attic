#include "perf_precomp.hpp"

using namespace cv;
typedef perf::TestBaseWithParam<perf::iMat> math;

PERF_TEST_P(math, min, ::testing::Values( mVGA8SC1(), mqHD8SC1(), m720p8SC1(), mODD8SC1(), 
                                          mVGA8SC3(), mqHD8SC3(), m720p8SC3(), mODD8SC3(), 
                                          mVGA8SC4(), mqHD8SC4(), m720p8SC4(), mODD8SC4(),
                                          mVGA32SC1(), mqHD32SC1(), m720p32SC1(), mODD32SC1(), 
                                          mVGA32SC3(), mqHD32SC3(), m720p32SC3(), mODD32SC3(), 
                                          mVGA32SC4(), mqHD32SC4(), m720p32SC4(), mODD32SC4(),
                                          mVGA32FC1(), mqHD32FC1(), m720p32FC1(), mODD32FC1(), 
                                          mVGA32FC3(), mqHD32FC3(), m720p32FC3(), mODD32FC3(), 
                                          mVGA32FC4(), mqHD32FC4(), m720p32FC4(), mODD32FC4() )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::min(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, max, ::testing::Values( mVGA8SC1(), mqHD8SC1(), m720p8SC1(), mODD8SC1(), 
                                          mVGA8SC3(), mqHD8SC3(), m720p8SC3(), mODD8SC3(), 
                                          mVGA8SC4(), mqHD8SC4(), m720p8SC4(), mODD8SC4(),
                                          mVGA32SC1(), mqHD32SC1(), m720p32SC1(), mODD32SC1(), 
                                          mVGA32SC3(), mqHD32SC3(), m720p32SC3(), mODD32SC3(), 
                                          mVGA32SC4(), mqHD32SC4(), m720p32SC4(), mODD32SC4(),
                                          mVGA32FC1(), mqHD32FC1(), m720p32FC1(), mODD32FC1(), 
                                          mVGA32FC3(), mqHD32FC3(), m720p32FC3(), mODD32FC3(), 
                                          mVGA32FC4(), mqHD32FC4(), m720p32FC4(), mODD32FC4() )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Mat b = GetParam().makeMat(-16, 10, 8, 14);
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::max(a,b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, min__Scalar, ::testing::Values( mVGA8SC1(), mqHD8SC1(), m720p8SC1(), mODD8SC1(), 
                                                  mVGA8SC3(), mqHD8SC3(), m720p8SC3(), mODD8SC3(), 
                                                  mVGA8SC4(), mqHD8SC4(), m720p8SC4(), mODD8SC4(),
                                                  mVGA32SC1(), mqHD32SC1(), m720p32SC1(), mODD32SC1(), 
                                                  mVGA32SC3(), mqHD32SC3(), m720p32SC3(), mODD32SC3(), 
                                                  mVGA32SC4(), mqHD32SC4(), m720p32SC4(), mODD32SC4(),
                                                  mVGA32FC1(), mqHD32FC1(), m720p32FC1(), mODD32FC1(), 
                                                  mVGA32FC3(), mqHD32FC3(), m720p32FC3(), mODD32FC3(), 
                                                  mVGA32FC4(), mqHD32FC4(), m720p32FC4(), mODD32FC4() )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::min(a,(InputArray)b, c);

    SANITY_CHECK(c);
}

PERF_TEST_P(math, max__Scalar, ::testing::Values( mVGA8SC1(), mqHD8SC1(), m720p8SC1(), mODD8SC1(), 
                                                  mVGA8SC3(), mqHD8SC3(), m720p8SC3(), mODD8SC3(), 
                                                  mVGA8SC4(), mqHD8SC4(), m720p8SC4(), mODD8SC4(),
                                                  mVGA32SC1(), mqHD32SC1(), m720p32SC1(), mODD32SC1(), 
                                                  mVGA32SC3(), mqHD32SC3(), m720p32SC3(), mODD32SC3(), 
                                                  mVGA32SC4(), mqHD32SC4(), m720p32SC4(), mODD32SC4(),
                                                  mVGA32FC1(), mqHD32FC1(), m720p32FC1(), mODD32FC1(), 
                                                  mVGA32FC3(), mqHD32FC3(), m720p32FC3(), mODD32FC3(), 
                                                  mVGA32FC4(), mqHD32FC4(), m720p32FC4(), mODD32FC4() )){
    cv::Mat a = GetParam().makeMat(-20, 21, -22, 23);
    cv::Scalar b;
    cv::Mat c = GetParam().makeMat();

    declare.in(a, b, WARMUP_RNG)
        .out(c)
        .time(0.5);

    TEST_CYCLE(100) cv::max(a,(InputArray)b, c);

    SANITY_CHECK(c);
}
