#include "perf_precomp.hpp"

typedef std::tr1::tuple<perf::MatInfo, cv::Size> resizeParams;
typedef perf::TestBaseWithParam<resizeParams> ResizeTest;

PERF_TEST_P(ResizeTest, upLinear, ::testing::Values(
                resizeParams(mVGA8UC1(), ::perf::szqHD),
                resizeParams(mVGA8UC1(), ::perf::sz720p),
                resizeParams(mVGA8UC4(), ::perf::sz720p)
                )) {
    cv::Mat src = std::tr1::get<0>(GetParam()).makeMat(20,21,22,23);
    cv::Size sz = std::tr1::get<1>(GetParam());
    cv::Mat dst = cv::Mat(sz, src.type());

    declare.in(src).out(dst);

    TEST_CYCLE(100) cv::resize(src, dst, sz);

    SANITY_CHECK(dst);
}

PERF_TEST_P(ResizeTest, downLinear, ::testing::Values(
                resizeParams(mVGA8UC1(), ::perf::szQVGA),
                resizeParams(mqHD8UC4(), ::perf::szVGA),
                resizeParams(m720p8UC1(), cv::Size(120 * ::perf::sz720p.width / ::perf::sz720p.height, 120)),//face detection min_face_size = 20%
                resizeParams(m720p8UC4(), ::perf::szVGA),
                resizeParams(m720p8UC4(), ::perf::szQVGA)
                )) {
    cv::Mat src = std::tr1::get<0>(GetParam()).makeMat(20,21,22,23);
    cv::Size sz = std::tr1::get<1>(GetParam());
    cv::Mat dst = cv::Mat(sz, src.type());

    declare.in(src).out(dst);

    TEST_CYCLE(100) cv::resize(src, dst, sz);

    SANITY_CHECK(dst);
}

