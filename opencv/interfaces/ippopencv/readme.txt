This is the project for custom ippopencv DLL,
a subset of IPP libraries used by OpenCV.

How to build and use it:
------------------------

1. You need to have IPP 5.1 or later installed.
   <IPP_install_path>\include and
   <IPP_install_path>\lib must be set in your IDE or INCLUDE & LIB environment variables

2. Open the CMake-generated project and build it.

3. You will get ippopencv<opencv_ver>.dll in <OpenCV_install_path>\bin that you may
   redistribute with your applications instead of the full bunch of IPP DLLs
   (ipps, ippi etc.).

------------------------

Note, that in order to ship ippopencv*.dll with commercial products one
must obtain a commercial license for IPP. This tool is only a specialized
version of customdll example included into IPP distribution
(<IPP>/tools/customdll) and it is provided for OpenCV users convenience.

A note for Linux users: while the ippopencv project has not been ported on Linux yet,
it should be possible to build custom shared library using the example from IPP
for Linux distribution and opencvipp_funclist.h as the list of functions.
