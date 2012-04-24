; -- OpenCV script for Inno Setup 2.0 (or later) Installer --

[Setup]
AppName=Intel(R) Open Source Computer Vision Library
AppVerName=Intel(R) Open Source Computer Vision Library 1.0
AppCopyright=Copyright (C) 2000-2006 Intel Corporation
DefaultDirName={pf}\OpenCV
DefaultGroupName=OpenCV
;UninstallDisplayIcon={app}\apps\CamShiftDemo\res\CamShiftDemo.ico
SourceDir=..
Compression=bzip/9
LicenseFile="docs\license.txt"
OutputBaseFilename=OpenCV_1.0
WizardImageFile=utils/splash.bmp
SetupIconFile=utils/opencv.ico
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Dirs]

; workspaces
Name: "{app}\_make"
;Name: "{app}\_make\cbuilderx"

; cxcore
Name: "{app}\cxcore"
Name: "{app}\cxcore\include"
Name: "{app}\cxcore\src"

; cv
Name: "{app}\cv"
Name: "{app}\cv\include"
Name: "{app}\cv\src"

; cvaux
Name: "{app}\cvaux"
Name: "{app}\cvaux\include"
Name: "{app}\cvaux\src"
Name: "{app}\cvaux\src\vs"

; ml
Name: "{app}\ml"
Name: "{app}\ml\include"
Name: "{app}\ml\src"


; training data
Name: "{app}\data"
Name: "{app}\data\haarcascades"

; otherlibs
Name: "{app}\otherlibs"
Name: "{app}\otherlibs\_graphics"
Name: "{app}\otherlibs\_graphics\include"
Name: "{app}\otherlibs\_graphics\include\jasper"
Name: "{app}\otherlibs\_graphics\lib"
Name: "{app}\otherlibs\_graphics\src"
Name: "{app}\otherlibs\_graphics\src\libjasper"
Name: "{app}\otherlibs\_graphics\src\libjpeg"
Name: "{app}\otherlibs\_graphics\src\libpng"
Name: "{app}\otherlibs\_graphics\src\libtiff"
Name: "{app}\otherlibs\_graphics\src\zlib"
Name: "{app}\otherlibs\highgui"
Name: "{app}\otherlibs\cvcam"
Name: "{app}\otherlibs\cvcam\include"
Name: "{app}\otherlibs\cvcam\sample"
Name: "{app}\otherlibs\cvcam\src"
Name: "{app}\otherlibs\cvcam\src\windows"
Name: "{app}\otherlibs\cvcam\src\unix"

; interfaces
Name: "{app}\interfaces"
;Name: "{app}\interfaces\ch"
;Name: "{app}\interfaces\ch\c"
;Name: "{app}\interfaces\ch\c\handmade"
;Name: "{app}\interfaces\ch\c\handmade\cv"
;Name: "{app}\interfaces\ch\c\handmade\cvcam"
;Name: "{app}\interfaces\ch\c\handmade\cxcore"
;Name: "{app}\interfaces\ch\c\handmade\highgui"
;Name: "{app}\interfaces\ch\chfcreate"
;Name: "{app}\interfaces\ch\chfhandmade"
;Name: "{app}\interfaces\ch\demos"
;Name: "{app}\interfaces\ch\demos\data"
;Name: "{app}\interfaces\ch\demos\data\haarcascades"

Name: "{app}\interfaces\ipp"

Name: "{app}\interfaces\swig"
Name: "{app}\interfaces\swig\filtered"
Name: "{app}\interfaces\swig\general"
Name: "{app}\interfaces\swig\python"
Name: "{app}\interfaces\swig\python\build"
Name: "{app}\interfaces\swig\python\build\lib.win32-2.5"
Name: "{app}\interfaces\swig\python\build\lib.win32-2.5\opencv"

; documentation
Name: "{app}\docs"
Name: "{app}\docs\ref"
Name: "{app}\docs\ref\pics"
Name: "{app}\docs\papers"
Name: "{app}\docs\vidsurv"

; sample code
Name: "{app}\samples"
Name: "{app}\samples\c"
Name: "{app}\samples\python"

; batch tests
Name: "{app}\tests"
Name: "{app}\tests\cxts"
Name: "{app}\tests\cxcore"
Name: "{app}\tests\cxcore\src"
Name: "{app}\tests\cv"
Name: "{app}\tests\cv\src"
Name: "{app}\tests\cv\testdata"
Name: "{app}\tests\cv\testdata\cameracalibration"
;Name: "{app}\tests\cv\testdata\gesturerecognition"
Name: "{app}\tests\cv\testdata\optflow"
Name: "{app}\tests\cv\testdata\snakes"
Name: "{app}\tests\python"
Name: "{app}\tests\python\highgui"

; utilities
Name: "{app}\utils"
Name: "{app}\utils\cvinfo"

; direct show filters
Name: "{app}\filters"
Name: "{app}\filters\CalibFilter"
Name: "{app}\filters\ProxyTrans"
Name: "{app}\filters\SyncFilter"
;Name: "{app}\filters\Tracker3dFilter"
;Name: "{app}\filters\Tracker3dFilter\include"
;Name: "{app}\filters\Tracker3dFilter\src"
;Name: "{app}\filters\Tracker3dFilter\trackers"
;Name: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"
;Name: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"
;Name: "{app}\filters\Tracker3dFilter\data"
;Name: "{app}\filters\Tracker3dFilter\data\CameraCalibration"
;Name: "{app}\filters\Tracker3dFilter\data\Tracking"

; applications
Name: "{app}\apps\"
Name: "{app}\apps\HaarTraining"
Name: "{app}\apps\HaarTraining\include"
Name: "{app}\apps\HaarTraining\make"
Name: "{app}\apps\HaarTraining\src"
Name: "{app}\apps\HaarTraining\doc"

; precompiled binaries
Name: "{app}\bin"
Name: "{app}\lib"


[Files]

; root
Source: "README"; DestDir: "{app}"
Source: "ChangeLog"; DestDir: "{app}"
Source: "TODO"; DestDir: "{app}"
Source: "AUTHORS"; DestDir: "{app}"
Source: "THANKS"; DestDir: "{app}"
Source: "INSTALL"; DestDir: "{app}"

; _make
Source: "_make\opencv*.dsw"; DestDir: "{app}\_make"
Source: "_make\opencv*.sln"; DestDir: "{app}\_make"
Source: "_make\makefile.*"; DestDir: "{app}\_make"
Source: "_make\*.mak"; DestDir: "{app}\_make"
;Source: "_make\cbuilderx\*.cbx"; DestDir: "{app}\_make\cbuilderx"
;Source: "_make\cbuilderx\*.bpgr"; DestDir: "{app}\_make\cbuilderx"

; cxcore
Source: "cxcore\include\*.h*"; DestDir: "{app}\cxcore\include"
Source: "cxcore\include\Makefile.*"; DestDir: "{app}\cxcore\include"
Source: "cxcore\src\*.c*"; DestDir: "{app}\cxcore\src"
Source: "cxcore\src\*.h*"; DestDir: "{app}\cxcore\src"
Source: "cxcore\src\Makefile.*"; DestDir: "{app}\cxcore\src"
Source: "cxcore\src\*.rc"; DestDir: "{app}\cxcore\src"
Source: "cxcore\src\*.dsp"; DestDir: "{app}\cxcore\src"
Source: "cxcore\src\*.vcproj"; DestDir: "{app}\cxcore\src"
Source: "cxcore\Makefile.*"; DestDir: "{app}\cxcore"


; cv
Source: "cv\include\*.h*"; DestDir: "{app}\cv\include"
Source: "cv\include\Makefile.*"; DestDir: "{app}\cv\include"
Source: "cv\src\*.c*"; DestDir: "{app}\cv\src"
Source: "cv\src\*.h*"; DestDir: "{app}\cv\src"
Source: "cv\src\Makefile.*"; DestDir: "{app}\cv\src"
Source: "cv\src\*.rc"; DestDir: "{app}\cv\src"
Source: "cv\src\*.dsp"; DestDir: "{app}\cv\src"
Source: "cv\src\*.vcproj"; DestDir: "{app}\cv\src"
Source: "cv\Makefile.*"; DestDir: "{app}\cv"

; cvaux
Source: "cvaux\include\*.h*"; DestDir: "{app}\cvaux\include"
Source: "cvaux\include\Makefile.*"; DestDir: "{app}\cvaux\include"
Source: "cvaux\src\*.c*"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\vs\*.c*"; DestDir: "{app}\cvaux\src\vs"
Source: "cvaux\src\*.h*"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\Makefile.*"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.rc"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.dsp"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.vcproj"; DestDir: "{app}\cvaux\src"
Source: "cvaux\Makefile.*"; DestDir: "{app}\cvaux"

; ml
Source: "ml\include\*.h*"; DestDir: "{app}\ml\include"
Source: "ml\src\*.c*"; DestDir: "{app}\ml\src"
Source: "ml\src\*.h*"; DestDir: "{app}\ml\src"
Source: "ml\src\Makefile.*"; DestDir: "{app}\ml\src"
Source: "ml\src\*.rc"; DestDir: "{app}\ml\src"
Source: "ml\src\*.dsp"; DestDir: "{app}\ml\src"
Source: "ml\src\*.vcproj"; DestDir: "{app}\ml\src"
Source: "ml\Makefile.*"; DestDir: "{app}\ml"

Source: "data\*.txt"; DestDir: "{app}\data"
Source: "data\haarcascades\*.xml"; DestDir: "{app}\data\haarcascades"

; otherlibs
Source: "otherlibs\Makefile.*"; DestDir: "{app}\otherlibs"

; graphic libraries
Source: "otherlibs\_graphics\include\*.h"; DestDir: "{app}\otherlibs\_graphics\include"
Source: "otherlibs\_graphics\include\jasper\*.h"; DestDir: "{app}\otherlibs\_graphics\include\jasper"
Source: "otherlibs\_graphics\lib\*.a"; DestDir: "{app}\otherlibs\_graphics\lib"
Source: "otherlibs\_graphics\lib\lib*.lib"; DestDir: "{app}\otherlibs\_graphics\lib"
Source: "otherlibs\_graphics\lib\zlib*.lib"; DestDir: "{app}\otherlibs\_graphics\lib"
Source: "otherlibs\_graphics\readme.txt"; DestDir: "{app}\otherlibs\_graphics"

Source: "otherlibs\_graphics\src\*.dsw"; DestDir: "{app}\otherlibs\_graphics\src"
Source: "otherlibs\_graphics\src\*.sln"; DestDir: "{app}\otherlibs\_graphics\src"

Source: "otherlibs\_graphics\src\libjasper\readme"; DestDir: "{app}\otherlibs\_graphics\src\libjasper"
Source: "otherlibs\_graphics\src\libjasper\license"; DestDir: "{app}\otherlibs\_graphics\src\libjasper"

Source: "otherlibs\_graphics\src\libjpeg\*.c"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"
Source: "otherlibs\_graphics\src\libjpeg\*.h"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"
Source: "otherlibs\_graphics\src\libjpeg\*.dsp"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"
Source: "otherlibs\_graphics\src\libjpeg\*.vcproj"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"
Source: "otherlibs\_graphics\src\libjpeg\makefile.*"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"
Source: "otherlibs\_graphics\src\libjpeg\readme"; DestDir: "{app}\otherlibs\_graphics\src\libjpeg"

Source: "otherlibs\_graphics\src\libpng\*.c"; DestDir: "{app}\otherlibs\_graphics\src\libpng"
Source: "otherlibs\_graphics\src\libpng\*.dsp"; DestDir: "{app}\otherlibs\_graphics\src\libpng"
Source: "otherlibs\_graphics\src\libpng\*.vcproj"; DestDir: "{app}\otherlibs\_graphics\src\libpng"
Source: "otherlibs\_graphics\src\libpng\readme"; DestDir: "{app}\otherlibs\_graphics\src\libpng"

Source: "otherlibs\_graphics\src\libtiff\*.c*"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"
Source: "otherlibs\_graphics\src\libtiff\*.h*"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"
Source: "otherlibs\_graphics\src\libtiff\*.dsp"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"
Source: "otherlibs\_graphics\src\libtiff\*.vcproj"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"
Source: "otherlibs\_graphics\src\libtiff\*.def"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"
Source: "otherlibs\_graphics\src\libtiff\Makefile.*"; DestDir: "{app}\otherlibs\_graphics\src\libtiff"

Source: "otherlibs\_graphics\src\zlib\*.c"; DestDir: "{app}\otherlibs\_graphics\src\zlib"
Source: "otherlibs\_graphics\src\zlib\*.h"; DestDir: "{app}\otherlibs\_graphics\src\zlib"
Source: "otherlibs\_graphics\src\zlib\*.dsp"; DestDir: "{app}\otherlibs\_graphics\src\zlib"
Source: "otherlibs\_graphics\src\zlib\*.vcproj"; DestDir: "{app}\otherlibs\_graphics\src\zlib"
Source: "otherlibs\_graphics\src\zlib\readme"; DestDir: "{app}\otherlibs\_graphics\src\zlib"

; otherlibs: highgui
Source: "otherlibs\highgui\*.c*"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.h*"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.rc"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.dsp"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.vcproj"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\Makefile.*"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.sh"; DestDir: "{app}\otherlibs\highgui"

; otherlibs: cvcam
Source: "otherlibs\cvcam\include\*.h*"; DestDir: "{app}\otherlibs\cvcam\include"
Source: "otherlibs\cvcam\include\Makefile.*"; DestDir: "{app}\otherlibs\cvcam\include"
Source: "otherlibs\cvcam\sample\*.c*"; DestDir: "{app}\otherlibs\cvcam\sample"
Source: "otherlibs\cvcam\sample\*.ds*"; DestDir: "{app}\otherlibs\cvcam\sample"
Source: "otherlibs\cvcam\src\windows\*.c*"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\windows\*.h*"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\windows\*.rc"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\windows\*.txt"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\windows\*.dsp"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\windows\*.vcproj"; DestDir: "{app}\otherlibs\cvcam\src\windows"
Source: "otherlibs\cvcam\src\unix\*.c*"; DestDir: "{app}\otherlibs\cvcam\src\unix"
Source: "otherlibs\cvcam\src\unix\*.h*"; DestDir: "{app}\otherlibs\cvcam\src\unix"
Source: "otherlibs\cvcam\src\unix\Makefile.*"; DestDir: "{app}\otherlibs\cvcam\src\unix"
Source: "otherlibs\cvcam\Makefile.*"; DestDir: "{app}\otherlibs\cvcam"
Source: "otherlibs\cvcam\src\Makefile.*"; DestDir: "{app}\otherlibs\cvcam\src"

; interfaces
Source: "interfaces\Makefile.*"; DestDir: "{app}\interfaces"

;Source: "interfaces\ch\Makefile"; DestDir: "{app}\interfaces\ch"
;Source: "interfaces\ch\pkg*.ch"; DestDir: "{app}\interfaces\ch"
;Source: "interfaces\ch\readme.txt"; DestDir: "{app}\interfaces\ch"
;Source: "interfaces\ch\c\Makefile*"; DestDir: "{app}\interfaces\ch\c"
;Source: "interfaces\ch\c\handmade\cv\*.c"; DestDir: "{app}\interfaces\ch\c\handmade\cv"
;Source: "interfaces\ch\c\handmade\cvcam\*.c"; DestDir: "{app}\interfaces\ch\c\handmade\cvcam"
;Source: "interfaces\ch\c\handmade\cxcore\*.c"; DestDir: "{app}\interfaces\ch\c\handmade\cxcore"
;Source: "interfaces\ch\c\handmade\highgui\*.c"; DestDir: "{app}\interfaces\ch\c\handmade\highgui"
;Source: "interfaces\ch\chfhandmade\*.chf"; DestDir: "{app}\interfaces\ch\chfhandmade"
;Source: "interfaces\ch\demos\*.ch"; DestDir: "{app}\interfaces\ch\demos"

Source: "interfaces\ipp\*.c"; DestDir: "{app}\interfaces\ipp"
Source: "interfaces\ipp\*.h"; DestDir: "{app}\interfaces\ipp"
Source: "interfaces\ipp\*.def"; DestDir: "{app}\interfaces\ipp"
Source: "interfaces\ipp\*.ds*"; DestDir: "{app}\interfaces\ipp"
Source: "interfaces\ipp\*.py"; DestDir: "{app}\interfaces\ipp"
Source: "interfaces\ipp\*.txt"; DestDir: "{app}\interfaces\ipp"

Source: "interfaces\swig\Makefile.*"; DestDir: "{app}\interfaces\swig"
Source: "interfaces\swig\filtered\Makefile.*"; DestDir: "{app}\interfaces\swig\filtered"
Source: "interfaces\swig\filtered\*.h"; DestDir: "{app}\interfaces\swig\filtered"
Source: "interfaces\swig\general\*.i"; DestDir: "{app}\interfaces\swig\general"
Source: "interfaces\swig\general\Makefile.*"; DestDir: "{app}\interfaces\swig\general"
Source: "interfaces\swig\python\Makefile.*"; DestDir: "{app}\interfaces\swig\python"
Source: "interfaces\swig\python\*.py"; DestDir: "{app}\interfaces\swig\python"
Source: "interfaces\swig\python\*.i"; DestDir: "{app}\interfaces\swig\python"
Source: "interfaces\swig\python\*.c*"; DestDir: "{app}\interfaces\swig\python"
Source: "interfaces\swig\python\*.h*"; DestDir: "{app}\interfaces\swig\python"
Source: "interfaces\swig\python\build\lib.win32-2.5\opencv\*.py*"; DestDir: "{app}\interfaces\swig\python\build\lib.win32-2.5\opencv"

;Source: "interfaces\matlab\ReadMe.txt"; DestDir: "{app}\interfaces\matlab"
;Source: "interfaces\matlab\src\*.c*"; DestDir: "{app}\interfaces\matlab\src"
;Source: "interfaces\matlab\src\*.h*"; DestDir: "{app}\interfaces\matlab\src"
;Source: "interfaces\matlab\src\*.dsp"; DestDir: "{app}\interfaces\matlab\src"
;Source: "interfaces\matlab\src\*.def"; DestDir: "{app}\interfaces\matlab\src"
;Source: "interfaces\matlab\toolbox\*.*"; DestDir: "{app}\interfaces\matlab\toolbox"
;Source: "interfaces\matlab\toolbox\opencv\*.m"; DestDir: "{app}\interfaces\matlab\toolbox\opencv"
;Source: "interfaces\matlab\toolbox\opencv\cvwrap.dll"; DestDir: "{app}\interfaces\matlab\toolbox\opencv"
;Source: "interfaces\matlab\toolbox\opencv\cvdemos\*.m*"; DestDir: "{app}\interfaces\matlab\toolbox\opencv\cvdemos"

; documentation
Source: "docs\*.htm*"; DestDir: "{app}\docs"
Source: "docs\*.jp*"; DestDir: "{app}\docs"
Source: "docs\*.txt"; DestDir: "{app}\docs"
Source: "docs\*.pdf"; DestDir: "{app}\docs"
Source: "docs\*.rtf"; DestDir: "{app}\docs"
Source: "docs\Makefile.*"; DestDir: "{app}\docs"
Source: "docs\ref\*.htm*"; DestDir: "{app}\docs\ref"
Source: "docs\ref\*.css"; DestDir: "{app}\docs\ref"
;Source: "docs\ref\Makefile.*"; DestDir: "{app}\docs\ref"
Source: "docs\ref\pics\*.jp*"; DestDir: "{app}\docs\ref\pics"
Source: "docs\ref\pics\*.png"; DestDir: "{app}\docs\ref\pics"
;Source: "docs\ref\pics\Makefile.*"; DestDir: "{app}\docs\ref\pics"
Source: "docs\papers\*.pdf"; DestDir: "{app}\docs\papers"
Source: "docs\papers\*.ps"; DestDir: "{app}\docs\papers"
Source: "docs\vidsurv\*.doc"; DestDir: "{app}\docs\vidsurv"

; sample code
Source: "samples\makefile.*"; DestDir: "{app}\samples"
Source: "samples\c\*.c*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.sh"; DestDir: "{app}\samples\c"
Source: "samples\c\*.jp*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.png"; DestDir: "{app}\samples\c"
Source: "samples\c\*.dsp"; DestDir: "{app}\samples\c"
Source: "samples\c\*.vcproj"; DestDir: "{app}\samples\c"
Source: "samples\c\Makefile.*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.exe"; DestDir: "{app}\samples\c"
Source: "samples\c\*.data"; DestDir: "{app}\samples\c"

Source: "samples\python\*.py"; DestDir: "{app}\samples\python"
Source: "samples\python\Makefile.*"; DestDir: "{app}\samples\python"

; batch tests
Source: "tests\Makefile.*"; DestDir: "{app}\tests"
Source: "tests\cv\Makefile.*"; DestDir: "{app}\tests\cv"
Source: "tests\cv\src\*.c*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.h*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\Makefile.*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.dsp"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.vcproj"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.inc"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\testdata\cameracalibration\*.*"; DestDir: "{app}\tests\cv\testdata\cameracalibration"
;Source: "tests\cv\testdata\gesturerecognition\*.*"; DestDir: "{app}\tests\cv\testdata\gesturerecognition"
Source: "tests\cv\testdata\optflow\*.*"; DestDir: "{app}\tests\cv\testdata\optflow"
Source: "tests\cv\testdata\snakes\*.*"; DestDir: "{app}\tests\cv\testdata\snakes"
Source: "tests\cxts\*.c*"; DestDir: "{app}\tests\cxts"
Source: "tests\cxts\*.h*"; DestDir: "{app}\tests\cxts"
Source: "tests\cxts\*.dsp"; DestDir: "{app}\tests\cxts"
Source: "tests\cxts\*.vcproj"; DestDir: "{app}\tests\cxts"
Source: "tests\cxts\Makefile.*"; DestDir: "{app}\tests\cxts"
Source: "tests\cxcore\src\*.c*"; DestDir: "{app}\tests\cxcore\src"
Source: "tests\cxcore\src\*.h*"; DestDir: "{app}\tests\cxcore\src"
Source: "tests\cxcore\src\*.dsp"; DestDir: "{app}\tests\cxcore\src"
Source: "tests\cxcore\src\*.vcproj"; DestDir: "{app}\tests\cxcore\src"
Source: "tests\cxcore\src\Makefile.*"; DestDir: "{app}\tests\cxcore\src"
Source: "tests\python\Makefile.*"; DestDir: "{app}\tests\python"
Source: "tests\python\highgui\*.py"; DestDir: "{app}\tests\python\highgui"

; utilities
Source: "utils\*.cmd"; DestDir: "{app}\utils"
Source: "utils\*.py"; DestDir: "{app}\utils"
Source: "utils\*.iss"; DestDir: "{app}\utils"
Source: "utils\*.bmp"; DestDir: "{app}\utils"
Source: "utils\*.ico"; DestDir: "{app}\utils"
Source: "utils\cvinfo\*.c*"; DestDir: "{app}\utils\cvinfo"
Source: "utils\cvinfo\*.dsp"; DestDir: "{app}\utils\cvinfo"
Source: "utils\cvinfo\*.vcproj"; DestDir: "{app}\utils\cvinfo"

; direct show filters
;Source: "filters\*.*"; DestDir: "{app}\filters"
Source: "filters\CalibFilter\*.c*"; DestDir: "{app}\filters\CalibFilter"
Source: "filters\CalibFilter\*.h*"; DestDir: "{app}\filters\CalibFilter"
Source: "filters\CalibFilter\*.rc"; DestDir: "{app}\filters\CalibFilter"
Source: "filters\CalibFilter\*.def"; DestDir: "{app}\filters\CalibFilter"
Source: "filters\CalibFilter\*.dsp"; DestDir: "{app}\filters\CalibFilter"
Source: "filters\CalibFilter\*.vcproj"; DestDir: "{app}\filters\CalibFilter"

Source: "filters\ProxyTrans\*.c*"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.h*"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.rc"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.def"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.dsp"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.vcproj"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\ProxyTrans\*.txt"; DestDir: "{app}\filters\ProxyTrans"

Source: "filters\SyncFilter\*.c*"; DestDir: "{app}\filters\SyncFilter"
Source: "filters\SyncFilter\*.h*"; DestDir: "{app}\filters\SyncFilter"
;Source: "filters\ProxyTrans\*.rc"; DestDir: "{app}\filters\ProxyTrans"
Source: "filters\SyncFilter\*.def"; DestDir: "{app}\filters\SyncFilter"
Source: "filters\SyncFilter\*.dsp"; DestDir: "{app}\filters\SyncFilter"
Source: "filters\SyncFilter\*.vcproj"; DestDir: "{app}\filters\SyncFilter"

;Source: "filters\Tracker3dFilter\*.*"; DestDir: "{app}\filters\Tracker3dFilter"
;Source: "filters\Tracker3dFilter\include\*.h*"; DestDir: "{app}\filters\Tracker3dFilter\include"
;Source: "filters\Tracker3dFilter\src\*.c*"; DestDir: "{app}\filters\Tracker3dFilter\src"
;Source: "filters\Tracker3dFilter\src\*.h*"; DestDir: "{app}\filters\Tracker3dFilter\src"
;Source: "filters\Tracker3dFilter\src\*.rc"; DestDir: "{app}\filters\Tracker3dFilter\src"
;Source: "filters\Tracker3dFilter\src\*.def"; DestDir: "{app}\filters\Tracker3dFilter\src"
;Source: "filters\Tracker3dFilter\src\*.dsp"; DestDir: "{app}\filters\Tracker3dFilter\src"

;Source: "filters\Tracker3dFilter\trackers\*.*"; DestDir: "{app}\filters\Tracker3dFilter\trackers"
;Source: "filters\Tracker3dFilter\trackers\BlobTracker\*.c*"; DestDir: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"
;Source: "filters\Tracker3dFilter\trackers\BlobTracker\*.h*"; DestDir: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"
;Source: "filters\Tracker3dFilter\trackers\BlobTracker\*.rc"; DestDir: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"
;Source: "filters\Tracker3dFilter\trackers\BlobTracker\*.def"; DestDir: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"
;Source: "filters\Tracker3dFilter\trackers\BlobTracker\*.dsp"; DestDir: "{app}\filters\Tracker3dFilter\trackers\BlobTracker"

;Source: "filters\Tracker3dFilter\trackers\CamShiftTracker\*.c*"; DestDir: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"
;Source: "filters\Tracker3dFilter\trackers\CamShiftTracker\*.h*"; DestDir: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"
;Source: "filters\Tracker3dFilter\trackers\CamShiftTracker\*.rc"; DestDir: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"
;Source: "filters\Tracker3dFilter\trackers\CamShiftTracker\*.def"; DestDir: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"
;Source: "filters\Tracker3dFilter\trackers\CamShiftTracker\*.dsp"; DestDir: "{app}\filters\Tracker3dFilter\trackers\CamShiftTracker"

;Source: "filters\Tracker3dFilter\data\*.*"; DestDir: "{app}\filters\Tracker3dFilter\data"
;Source: "filters\Tracker3dFilter\data\CameraCalibration\*.*"; DestDir: "{app}\filters\Tracker3dFilter\data\CameraCalibration"
;Source: "filters\Tracker3dFilter\data\Tracking\*.*"; DestDir: "{app}\filters\Tracker3dFilter\data\Tracking"


; applications
;Source: "apps\HaarFaceDetect\*.c*"; DestDir: "{app}\apps\HaarFaceDetect"
;Source: "apps\HaarFaceDetect\*.h*"; DestDir: "{app}\apps\HaarFaceDetect"
;Source: "apps\HaarFaceDetect\*.dsp"; DestDir: "{app}\apps\HaarFaceDetect"
;Source: "apps\HaarFaceDetect\*.zip"; DestDir: "{app}\apps\HaarFaceDetect"
;Source: "apps\HaarFaceDetect\Makefile.*"; DestDir: "{app}\apps\HaarFaceDetect"
;Source: "apps\HaarFaceDetect\SampleBase\*.*"; DestDir: "{app}\apps\HaarFaceDetect\SampleBase"

Source: "apps\HaarTraining\*.*"; DestDir: "{app}\apps\HaarTraining"
Source: "apps\HaarTraining\include\*.h*"; DestDir: "{app}\apps\HaarTraining\include"
Source: "apps\HaarTraining\make\*.ds*"; DestDir: "{app}\apps\HaarTraining\make"
Source: "apps\HaarTraining\make\*.vcproj"; DestDir: "{app}\apps\HaarTraining\make"
Source: "apps\HaarTraining\make\*.sln"; DestDir: "{app}\apps\HaarTraining\make"
Source: "apps\HaarTraining\src\*.c*"; DestDir: "{app}\apps\HaarTraining\src"
Source: "apps\HaarTraining\src\*.h*"; DestDir: "{app}\apps\HaarTraining\src"
Source: "apps\HaarTraining\src\*.h*"; DestDir: "{app}\apps\HaarTraining\src"
Source: "apps\HaarTraining\src\Makefile.*"; DestDir: "{app}\apps\HaarTraining\src"
Source: "apps\HaarTraining\doc\*.htm*"; DestDir: "{app}\apps\HaarTraining\doc"

;Source: "apps\Tracker3dDemo\*.c*"; DestDir: "{app}\apps\Tracker3dDemo"
;Source: "apps\Tracker3dDemo\*.h*"; DestDir: "{app}\apps\Tracker3dDemo"
;Source: "apps\Tracker3dDemo\*.rc"; DestDir: "{app}\apps\Tracker3dDemo"
;Source: "apps\Tracker3dDemo\*.ds*"; DestDir: "{app}\apps\Tracker3dDemo"

; precompiled binaries
Source: "bin\cvinfo.exe"; DestDir: "{app}\bin"
Source: "bin\cvtest.exe"; DestDir: "{app}\bin"
Source: "bin\cxcoretest.exe"; DestDir: "{app}\bin"
Source: "bin\cxcore100.dll"; DestDir: "{app}\bin"
Source: "bin\cv100.dll"; DestDir: "{app}\bin"
Source: "bin\highgui100.dll"; DestDir: "{app}\bin"
Source: "bin\cvaux100.dll"; DestDir: "{app}\bin"
Source: "bin\ml100.dll"; DestDir: "{app}\bin"
Source: "bin\cxts001.dll"; DestDir: "{app}\bin"
Source: "bin\cvcam100.dll"; DestDir: "{app}\bin"
Source: "bin\haartraining.exe"; DestDir: "{app}\bin"
Source: "bin\createsamples.exe"; DestDir: "{app}\bin"
Source: "bin\performance.exe"; DestDir: "{app}\bin"
Source: "bin\proxytrans.ax"; DestDir: "{app}\bin"
Source: "bin\syncfilter.ax"; DestDir: "{app}\bin"
Source: "bin\calibfilter.ax"; DestDir: "{app}\bin"
Source: "bin\registerall.bat"; DestDir: "{app}\bin"
Source: "bin\libguide40.dll"; DestDir: "{app}\bin"

;Source: "bin\*.exe"; DestDir: "{app}\bin"
;Source: "bin\*.dll"; DestDir: "{app}\bin"
;Source: "bin\*.ax"; DestDir: "{app}\bin"
;Source: "bin\*.bat"; DestDir: "{app}\bin"

; 3D Tracking
;Source: "bin\Tracker3dDemo.exe"; DestDir: "{app}\bin"
;Source: "bin\Tracker3dFilter.ax"; DestDir: "{app}\bin"
;Source: "bin\BlobTracker.dll"; DestDir: "{app}\bin"
;Source: "bin\CamShiftTracker.dll"; DestDir: "{app}\bin"
;Source: "bin\tracking_expected_results*"; DestDir: "{app}\bin"
;Source: "bin\trackingtest*"; DestDir: "{app}\bin"
;Source: "bin\cam*.txt"; DestDir: "{app}\bin"

; import libraries
Source: "lib\cxcore.lib"; DestDir: "{app}\lib"
Source: "lib\cv.lib"; DestDir: "{app}\lib"
Source: "lib\highgui.lib"; DestDir: "{app}\lib"
Source: "lib\cvaux.lib"; DestDir: "{app}\lib"
Source: "lib\ml.lib"; DestDir: "{app}\lib"
Source: "lib\cvhaartraining.lib"; DestDir: "{app}\lib"
Source: "lib\cxts.lib"; DestDir: "{app}\lib"
Source: "lib\cvcam.lib"; DestDir: "{app}\lib"

[Icons]
Name: "{group}\OpenCV Workspace MSVC6"; Filename: "{app}\_make\opencv.dsw"
Name: "{group}\OpenCV Workspace .NET 2005"; Filename: "{app}\_make\opencv.sln"
;Name: "{group}\OpenCV Workspace Borland C++ BuilderX"; Filename: "{app}\_make\cbuilderx\opencv.bpgr"
Name: "{group}\Documentation"; Filename: "{app}\docs\index.htm"
Name: "{group}\Samples"; Filename: "{app}\samples\c\"

[Tasks]
Name: add_opencv_path; Description: "Add <...>\OpenCV\bin to the system PATH"; Flags: checkedonce

[Registry]
; Start "Software\My Company\My Program" keys under HKEY_CURRENT_USER
; and HKEY_LOCAL_MACHINE. The flags tell it to always delete the
; "My Program" keys upon uninstall, and delete the "My Company" keys
; if there is nothing left in them.
Root: HKCU; Subkey: "Environment"; ValueType: string; ValueName: "Path"; ValueData: "{app}\bin;{olddata}"; Flags: createvalueifdoesntexist; Tasks: add_opencv_path
Root: HKCU; Subkey: "Software\Intel"; Flags: uninsdeletekeyifempty
Root: HKCU; Subkey: "Software\Intel\OpenCV"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Intel\OpenCV\Settings"; ValueType: string; ValueName: "Path"; ValueData: "{app}"

[Run]
Filename: "{reg:HKLM\Software\Python\PythonCore\2.5\InstallPath,|C:\Python25\}python.exe"; Parameters: "setup-for-win.py install"; WorkingDir: "{app}\interfaces\swig\python"; Flags: skipifdoesntexist; StatusMsg: "Installing OpenCV Module for Python..."
Filename: "{app}\bin\RegisterAll.bat"; WorkingDir: "{app}\bin"; StatusMsg: "Registering DirectShow filters..."
Filename: "{app}\docs\index.htm"; Description: "View Documentation"; Flags: postinstall shellexec

[UninstallRun]
Filename: "{app}\bin\RegisterAll.bat"; Parameters: "/U"; WorkingDir: "{app}\bin"

