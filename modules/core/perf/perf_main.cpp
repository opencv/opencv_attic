#include "perf_precomp.hpp"

int main(int argc, char **argv)
{
    //cvtest::TS::ptr()->init(resourcesubdir);
    ::perf::Regression::Init("core");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
