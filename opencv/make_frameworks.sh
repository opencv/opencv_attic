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
if test -x Makefile; then make distclean; fi

# (re-)create directories
rm -rf build_ppc build_i386 OpenCV.framework
mkdir build_ppc
mkdir build_i386

# find out how many parallel processes we want to use for make
# see http://freshmeat.net/projects/kernbench/, we use a slightly different 'optimum' guess
export parallel_jobs=$((2 * `sysctl -n hw.ncpu` + 1))

# this setting defines where the framework should be installed
export FRAMEWORK_INSTALL_PATH="@executable_path/../Frameworks"
#export FRAMEWORK_INSTALL_PATH="/Library/Frameworks"
#export FRAMEWORK_INSTALL_PATH="/Users/your_login_name_here/Library/Frameworks"

# build powerpc version
echo "Building ppc version of the OpenCV framework"
echo "============================================"
cd build_ppc && ../configure --build=`arch` --host="powerpc-apple-darwin8" CXXFLAGS="-arch ppc" --without-python --without-swig --disable-apps && make -j $parallel_jobs framework FRAMEWORK_ARCH=ppc

# build intel version
echo "Building i386 version of the OpenCV framework"
echo "============================================="
if test -d ../build_i386; then cd ../build_i386; fi
../configure --build=`arch` --host="i686-apple-darwin8" CXXFLAGS="-arch i386"  --without-python --without-swig --disable-apps && make -j $parallel_jobs framework FRAMEWORK_ARCH=i386

# build universal version
echo "Creating universal Framework"
echo "============================================="
if test -d ../build_i386; then cd .. ; fi
cp -Rp build_ppc/OpenCV.framework ./
lipo -create build_ppc/OpenCV.framework/OpenCV build_i386/OpenCV.framework/OpenCV -output OpenCV.framework/Versions/A/OpenCV

# finalize
echo "Done!"
open .
