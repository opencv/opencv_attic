#! /bin/bash

# 2007-05-23, Mark Asbach <asbach@ient.rwth-aachen.de>

# This shell script makes use of VPATH builds and the special 'framework' Makefile target of
# the top level Makefile to create a universal binary framework out of native frameworks.
# No manual './configure && make && make install' is necessary if the framework is sufficient
# for you. However, Python bindings are not generated this way because of issues with 
# different python versions and universal binaries.
#
# If you need python wrappers and standard unix install, you should do a manual VPATH
# build with all the settings you like.
#
# The resulting framework is created as a Private Framework that must be copied into your
# application bundle by a dedicated 'Copy Files' build step. See the demo XCode project
# included with OpenCV and/or have a look at
# http://developer.apple.com/documentation/MacOSX/Conceptual/BPFrameworks/Tasks/CreatingFrameworks.html#//apple_ref/doc/uid/20002258-106880-BAJJBIEF

# the current directory should not be configured
if test -e Makefile; then make distclean; fi

# (re-)create directories
rm -rf build_ppc build_ppc64 build_i386 build_x86_64 OpenCV.framework
mkdir build_ppc
mkdir build_i386
#mkdir build_ppc64
#mkdir build_x86_64

# find out how many parallel processes we want to use for make
# see http://freshmeat.net/projects/kernbench/, we use a slightly different 'optimum' guess
export parallel_jobs=$((2 * `sysctl -n hw.ncpu` + 1))

# this setting defines where the framework should be installed
export FRAMEWORK_INSTALL_PATH="@executable_path/../Frameworks"
#export FRAMEWORK_INSTALL_PATH="/Library/Frameworks"
#export FRAMEWORK_INSTALL_PATH="/Users/your_login_name_here/Library/Frameworks"

# set up a couple of additional build settings
export SETTINGS="CC=gcc-4.2 CXX=g++-4.2 --without-python --without-octave --disable-apps --build=`arch`"
#export SETTINGS="--without-python --without-octave --build=`arch`"
export SYSROOT="--sysroot=/Developer/SDKs/MacOSX10.5.sdk"
#export SYSROOT="" #--iwithsysroot=/Developer/SDKs/MacOSX10.5.sdk"

# build powerpc version
echo "Building ppc version of the OpenCV framework"
echo "============================================"
cd build_ppc \
 && ../configure --host=ppc-apple-darwin9 $SETTINGS CPPFLAGS="$SYSROOT" CFLAGS="-arch ppc" CXXFLAGS="-arch ppc" LDFLAGS="$SYSROOT -arch ppc"\
 && make -j $parallel_jobs framework FRAMEWORK_ARCH=ppc

# build powerpc 64bit version
#echo "Building 64bit ppc version of the OpenCV framework"
#echo "============================================"
#if test -d ../build_ppc64; then cd ../build_ppc64; fi
#../configure --host=ppc64-apple-darwin9 $SETTINGS --without-quicktime --without-carbon CPPFLAGS="$SYSROOT" CXXFLAGS="-arch ppc64" LDFLAGS="$SYSROOT -arch ppc64"\
# && make -j $parallel_jobs framework FRAMEWORK_ARCH=ppc64

# options from -O2
#" -fthread-jumps -fcrossjumping -foptimize-sibling-calls -fcse-follow-jumps -fcse-skip-blocks -fgcse -fgcse-lm -fexpensive-optimizations -fstrength-reduce -frerun-cse-after-loop -frerun-loop-opt -fcaller-saves -fpeephole2 -fschedule-insns -fschedule-insns2  -fsched-spec -fregmove -fdelete-null-pointer-checks -freorder-functions -funit-at-a-time -falign-functions -falign-jumps -falign-loops -falign-labels -ftree-pre"
# build intel version
echo "Building i386 version of the OpenCV framework"
echo "============================================="
if test -d ../build_i386; then cd ../build_i386; fi
../configure --host=i386-apple-darwin9 $SETTINGS --enable-sse CPPFLAGS="$SYSROOT" CFLAGS="-arch i386" CXXFLAGS="-arch i386" LDFLAGS="$SYSROOT -arch i386"\
 && make -j $parallel_jobs framework FRAMEWORK_ARCH=i386

# build intel version
#echo "Building x86_64 version of the OpenCV framework"
#echo "============================================="
#if test -d ../build_x86_64; then cd ../build_x86_64; fi
#../configure --host=x86_64-apple-darwin9 $SETTINGS --without-quicktime --without-carbon CPPFLAGS="$SYSROOT" CXXFLAGS="-arch x86_64" LDFLAGS="$SYSROOT -arch x86_64"\
#  && make -j $parallel_jobs framework FRAMEWORK_ARCH=x86_64

# build universal version
echo "Creating universal Framework"
echo "============================================="
if test -d ../build_i386; then cd .. ; fi
cp -Rp build_ppc/OpenCV.framework ./
lipo -create build_ppc/OpenCV.framework/OpenCV build_i386/OpenCV.framework/OpenCV -output OpenCV.framework/Versions/B/OpenCV
#lipo -create build_ppc/OpenCV.framework/OpenCV build_ppc64/OpenCV.framework/OpenCV build_i386/OpenCV.framework/OpenCV build_x86_64/OpenCV.framework/OpenCV -output OpenCV.framework/Versions/A/OpenCV

# finalize
echo "Done!"
open .
