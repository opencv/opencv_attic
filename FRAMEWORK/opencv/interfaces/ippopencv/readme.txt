This is the project for Visual Studio 6.0 for custom ippopencv DLL,
a subset of IPP libraries used by OpenCV.

How to build and use it:
------------------------

1. You need to have IPP 4.x or 5.x installed.
   <IPP_install_path>\include and
   <IPP_install_path>\lib must be added to the directory lists in
   Developer Studio (Tools->Options->Directories in case of Visual Studio 6.0)

2. Open ippopencv.dsw with Visual Studio 6.0 or Visual Studio .NET 2003/2005.
   In the latter case it will be automatically converted to .sln

3. Choose the appropriate configuration
   ("Release" for IPP 4.x, "Release IPP5" for IPP 5.0.x,
    "Release IPP5_1" for IPP 5.1.x or later versions)
   and build it.

4. You will get ippopencv<nnn>.dll in <OpenCV_install_path>\bin that you may
   redistribute with your applications instead of the full bunch of IPP DLLs
   (ipps, ippi etc.). The DLL will combine A6 (for Pentium III or compatible)
   and W7 (for Pentium 4, Pentium M or compatible) code and the best version
   will be automatically chosen for the particular CPU.

------------------------

Note, that in order to ship ippopencv*.dll with commercial products one
must obtain a commercial license for IPP. This tool is only a specialized
version of customdll example included into IPP distribution
(<IPP>/tools/customdll) and it is provided for OpenCV users convenience.

A note for Linux users: while the ippopencv project has not been ported on Linux yet,
it should be possible to build custom shared library using the example from IPP
for Linux distribution and opencvipp_funclist.h as the list of functions.
