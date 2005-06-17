This folder contains libraries and headers of a fewvery popular still image codecs used by highgui.
The libraries and headers are only to build Win32 and Win64 versions of OpenCV.
On UNIX systems all the libraries are automatically detected by configure script.

------------------------------------------------------------------------------------
libjpeg 6b (6.2) - The Independent JPEG Group's JPEG software.             Copyright (C) 1994-1997, Thomas G. Lane.
             See IGJ home page http://www.ijg.org
             for details and links to the source code

             HAVE_JPEG preprocessor flag must be set to make highgui use libjpeg.             On UNIX systems configure script takes care of it.

------------------------------------------------------------------------------------
libpng 1.2.1 - Portable Network Graphics library.
               Copyright (C) 1998-2001, Glenn Randers-Pehrson.
               See libpng home page http://www.libpng.org
               for details and links to the source code

               HAVE_PNG preprocessor flag must be set to make highgui use libpng.               On UNIX systems configure script takes care of it.

------------------------------------------------------------------------------------
libtiff 3.5.7 - Tag Image File Format (TIFF) Software
                Copyright (c) 1988-1997 Sam Leffler
                Copyright (c) 1991-1997 Silicon Graphics, Inc.
                See libtiff home page http://www.libtiff.org
                for details and links to the source code

                HAVE_TIFF preprocessor flag must be set to make highgui use libtiff.
                On UNIX systems configure script takes care of it.

                In this build support for ZIP (LZ77 compression), JPEG and LZW
                are included.
------------------------------------------------------------------------------------
zlib 1.1.4 - General purpose LZ77 compression library
             Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
             See zlib home page http://www.gzip.org/zlib
             for details and links to the source code

             No preprocessor definition is needed to make highgui use this library -             it is included automatically if either libpng or libtiff are used.

------------------------------------------------------------------------------------

The folder lib also contains libvfw_*.a import libraries that enable to
build vfw camera/avi capture code of highgui with Mingw compiler.

The files have been taken from http://mywebpage.netscape.com/PtrPck/multimedia.htm site.Besides the libraries, you will also need the following headers that are missing in Mingw:
msacm.h, mmreg.h, vfw.h, vfwmsgs.h, verinfo.h, verinfo.ver, digitalv.h.
Copy them to <GCC_HOME>\include
You may take them, for example, from Microsoft Platform SDK or
Borland C++ 5.5 free compiler.

------------------------------------------------------------------------------------
