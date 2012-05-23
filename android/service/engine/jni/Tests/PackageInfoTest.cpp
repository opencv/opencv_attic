#include "HardwareDetector.h"
#include "IPackageManager.h"
#include "IOpenCVEngine.h"
#include "PackageInfo.h"
#include <gtest/gtest.h>
#include <set>
#include <string>
#include <vector>

using namespace std;

TEST(PackageInfo, FullNameArmv7)
{
    PackageInfo info("2.3", PLATFORM_UNKNOWN, ARCH_ARMv7);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_armv7", name.c_str());
}

TEST(PackageInfo, FullNameArmv7Neon)
{
    PackageInfo info("2.3", PLATFORM_UNKNOWN, ARCH_ARMv7 | FEATURES_HAS_NEON);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_armv7_neon", name.c_str());
}

TEST(PackageInfo, FullNameArmv7VFPv3)
{
    PackageInfo info("2.3", PLATFORM_UNKNOWN, ARCH_ARMv7 | FEATURES_HAS_VFPv3);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_armv7_vfpv3", name.c_str());
}

TEST(PackageInfo, FullNameArmv7VFPv3Neon)
{
    PackageInfo info("2.3", PLATFORM_UNKNOWN, ARCH_ARMv7 | FEATURES_HAS_VFPv3 | FEATURES_HAS_NEON);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_armv7_vfpv3_neon", name.c_str());
}

TEST(PackageInfo, FullNameX86SSE2)
{
    PackageInfo info("2.3", PLATFORM_UNKNOWN, ARCH_X86 | FEATURES_HAS_SSE2);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_x86_sse2", name.c_str());
}

TEST(PackageInfo, Armv7VFPv3NeonFromFullName)
{
    PackageInfo info("org.opencv.lib_v23_armv7_vfpv3_neon", "/data/data/org.opencv.lib_v23_armv7_vfpv3_neon");
    EXPECT_EQ("2.3", info.GetVersion());
    EXPECT_EQ(ARCH_ARMv7 | FEATURES_HAS_VFPv3 | FEATURES_HAS_NEON, info.GetCpuID());    
}

TEST(PackageInfo, X86SSE2FromFullName)
{
    PackageInfo info("org.opencv.lib_v24_x86_sse2", "/data/data/org.opencv.lib_v24_x86_sse2");
    EXPECT_EQ(PLATFORM_UNKNOWN, info.GetPlatform());
    EXPECT_EQ(ARCH_X86 | FEATURES_HAS_SSE2, info.GetCpuID());
    EXPECT_EQ("2.4", info.GetVersion());
}

TEST(PackageInfo, Tegra2FromFullName)
{
    PackageInfo info("org.opencv.lib_v23_tegra2", "/data/data/org.opencv.lib_v23_tegra2");
    EXPECT_EQ("2.3", info.GetVersion());
    EXPECT_EQ(PLATFORM_TEGRA2, info.GetPlatform());
}

TEST(PackageInfo, Tegra3FromFullName)
{
    PackageInfo info("org.opencv.lib_v24_tegra3", "/data/data/org.opencv.lib_v24_tegra3");
    EXPECT_EQ("2.4", info.GetVersion());
    EXPECT_EQ(PLATFORM_TEGRA3, info.GetPlatform());
}

TEST(PackageInfo, FullNameTegra3)
{
    PackageInfo info("2.3", PLATFORM_TEGRA3, 0);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_tegra3", name.c_str());
}

TEST(PackageInfo, FullNameTegra2)
{
    PackageInfo info("2.3", PLATFORM_TEGRA2, 0);
    string name = info.GetFullName();
    EXPECT_STREQ("org.opencv.lib_v23_tegra2", name.c_str());
}

TEST(PackageInfo, Comparator1)
{
    PackageInfo info1("2.3", PLATFORM_TEGRA2, 0);
    PackageInfo info2("org.opencv.lib_v23_tegra2", "/data/data/org.opencv.lib_v23_tegra2");
    EXPECT_TRUE(info1 == info2);
}

TEST(PackageInfo, Comparator2)
{
    PackageInfo info1("2.4", PLATFORM_UNKNOWN, ARCH_ARMv7 | FEATURES_HAS_NEON | FEATURES_HAS_VFPv3);
    PackageInfo info2("org.opencv.lib_v24_armv7_vfpv3_neon", "/data/data/org.opencv.lib_v24_armv7_vfpv3_neon");
    EXPECT_TRUE(info1 == info2);
}

