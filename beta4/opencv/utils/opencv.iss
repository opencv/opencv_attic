; -- OpenCV script for Inno Setup 2.0 (or later) Installer --

[Setup]
AppName=Intel(R) Open Source Computer Vision Library
AppVerName=Intel(R) Open Source Computer Vision Library, beta 4
AppCopyright=Copyright (C) 2000-2002 Intel Corporation
DefaultDirName={pf}\OpenCV
DefaultGroupName=OpenCV
;UninstallDisplayIcon={app}\apps\CamShiftDemo\res\CamShiftDemo.ico
SourceDir=..
Compression=bzip/9
LicenseFile="docs\license.txt"
OutputBaseFilename=OpenCV_b4a
; uncomment the following line if you want your installation to run on NT 3.51 too.
; MinVersion=4,3.51

[Dirs]

; workspaces
Name: "{app}\_make"
Name: "{app}\_make\cbuilderx"

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

; training data
Name: "{app}\data"
Name: "{app}\data\haarcascades"

; otherlibs
Name: "{app}\otherlibs"
Name: "{app}\otherlibs\_graphics"
Name: "{app}\otherlibs\_graphics\include"
Name: "{app}\otherlibs\_graphics\lib"
Name: "{app}\otherlibs\highgui"
Name: "{app}\otherlibs\cvcam"
Name: "{app}\otherlibs\cvcam\include"
Name: "{app}\otherlibs\cvcam\sample"
Name: "{app}\otherlibs\cvcam\src"
Name: "{app}\otherlibs\cvcam\src\windows"
Name: "{app}\otherlibs\cvcam\src\unix"

; interfaces
;Name: "{app}\interfaces"
;Name: "{app}\interfaces\ch"
;Name: "{app}\interfaces\ch\Devel"
;Name: "{app}\interfaces\ch\Devel\include"
;Name: "{app}\interfaces\ch\Devel\createchf"
;Name: "{app}\interfaces\ch\Devel\c"
;Name: "{app}\interfaces\ch\Devel\c\cv"
;Name: "{app}\interfaces\ch\Devel\c\highgui"
;Name: "{app}\interfaces\ch\Devel\lib.handmade"
;Name: "{app}\interfaces\ch\OpenCV"
;Name: "{app}\interfaces\ch\OpenCV\bin"
;Name: "{app}\interfaces\ch\OpenCV\include"
;Name: "{app}\interfaces\ch\OpenCV\lib"
;Name: "{app}\interfaces\ch\OpenCV\dl"
;Name: "{app}\interfaces\ch\OpenCV\demos"

;Name: "{app}\interfaces\matlab"
;Name: "{app}\interfaces\matlab\src"
;Name: "{app}\interfaces\matlab\toolbox"
;Name: "{app}\interfaces\matlab\toolbox\opencv"
;Name: "{app}\interfaces\matlab\toolbox\opencv\cvdemos"

; documentation
Name: "{app}\docs"
Name: "{app}\docs\ref"
Name: "{app}\docs\ref\pics"
Name: "{app}\docs\papers"
;Name: "{app}\docs\appPage"
;Name: "{app}\docs\appPage\3dTracker"
;Name: "{app}\docs\appPage\Calibration"
;Name: "{app}\docs\appPage\CamShift"
;Name: "{app}\docs\appPage\ConDensation"
;Name: "{app}\docs\appPage\FaceRecognition"
;Name: "{app}\docs\appPage\Kalman"
;Name: "{app}\docs\appPage\LKTracker"

; sample code
Name: "{app}\samples"
Name: "{app}\samples\c"

; batch tests
Name: "{app}\tests"
Name: "{app}\tests\trs"
Name: "{app}\tests\cv"
Name: "{app}\tests\cv\src"
Name: "{app}\tests\cv\testdata"
Name: "{app}\tests\cv\testdata\cameracalibration"
Name: "{app}\tests\cv\testdata\gesturerecognition"
Name: "{app}\tests\cv\testdata\optflow"
Name: "{app}\tests\cv\testdata\snakes"

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
Source: "_make\cbuilderx\*.cbx"; DestDir: "{app}\_make\cbuilderx"
Source: "_make\cbuilderx\*.bpgr"; DestDir: "{app}\_make\cbuilderx"

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
Source: "cvaux\src\*.h*"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\Makefile.*"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.rc"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.dsp"; DestDir: "{app}\cvaux\src"
Source: "cvaux\src\*.vcproj"; DestDir: "{app}\cvaux\src"
Source: "cvaux\Makefile.*"; DestDir: "{app}\cvaux"

Source: "data\*.txt"; DestDir: "{app}\data"
Source: "data\haarcascades\*.xml"; DestDir: "{app}\data\haarcascades"

; otherlibs
Source: "otherlibs\Makefile.*"; DestDir: "{app}\otherlibs"

; graphic libraries
Source: "otherlibs\_graphics\include\*.h"; DestDir: "{app}\otherlibs\_graphics\include"
Source: "otherlibs\_graphics\lib\*.a"; DestDir: "{app}\otherlibs\_graphics\lib"
Source: "otherlibs\_graphics\lib\*.lib"; DestDir: "{app}\otherlibs\_graphics\lib"
Source: "otherlibs\_graphics\readme.txt"; DestDir: "{app}\otherlibs\_graphics"

; otherlibs: highgui
Source: "otherlibs\highgui\*.c*"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.h*"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.rc"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.dsp"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\*.vcproj"; DestDir: "{app}\otherlibs\highgui"
Source: "otherlibs\highgui\Makefile.*"; DestDir: "{app}\otherlibs\highgui"

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
;Source: "interfaces\ch\ReadMe.txt"; DestDir: "{app}\interfaces\ch"
;Source: "interfaces\ch\Devel\*.ch"; DestDir: "{app}\interfaces\ch\Devel"
;Source: "interfaces\ch\Devel\include\*.h"; DestDir: "{app}\interfaces\ch\Devel\include"
;Source: "interfaces\ch\Devel\createchf\*.h"; DestDir: "{app}\interfaces\ch\Devel\createchf"
;Source: "interfaces\ch\Devel\c\cv\*.c"; DestDir: "{app}\interfaces\ch\Devel\c\cv"
;Source: "interfaces\ch\Devel\c\cv\Makefile"; DestDir: "{app}\interfaces\ch\Devel\c\cv"
;Source: "interfaces\ch\Devel\c\highgui\*.c"; DestDir: "{app}\interfaces\ch\Devel\c\highgui"
;Source: "interfaces\ch\Devel\c\highgui\Makefile"; DestDir: "{app}\interfaces\ch\Devel\c\highgui"
;Source: "interfaces\ch\Devel\c\cvcam\*.c"; DestDir: "{app}\interfaces\ch\Devel\c\cvcam"
;Source: "interfaces\ch\Devel\c\cvcam\Makefile"; DestDir: "{app}\interfaces\ch\Devel\c\cvcam"
;Source: "interfaces\ch\Devel\lib.handmade\*.chf"; DestDir: "{app}\interfaces\ch\Devel\lib.handmade"
;Source: "interfaces\ch\OpenCV\lib\*.chf"; DestDir: "{app}\interfaces\ch\OpenCV\lib"
;Source: "interfaces\ch\OpenCV\dl\*.dl"; DestDir: "{app}\interfaces\ch\OpenCV\dl"
; add files from CV (because Inno handles duplicated files, this won't increase install size)
;Source: "cv\include\*.h*"; DestDir: "{app}\interfaces\ch\OpenCV\include"
;Source: "otherlibs\highgui\highgui.h"; DestDir: "{app}\interfaces\ch\OpenCV\include"
;Source: "otherlibs\cvcam\include\cvcam.h"; DestDir: "{app}\interfaces\ch\OpenCV\include"
;Source: "bin\cv.dll"; DestDir: "{app}\interfaces\ch\OpenCV\bin"
;Source: "bin\highgui.dll"; DestDir: "{app}\interfaces\ch\OpenCV\bin"
;Source: "bin\cvcam.dll"; DestDir: "{app}\interfaces\ch\OpenCV\bin"

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
;Source: "docs\appPage\appindex.htm"; DestDir: "{app}\docs\appPage"
;Source: "docs\appPage\*.png"; DestDir: "{app}\docs\appPage"
;Source: "docs\appPage\3dTracker\*.htm*"; DestDir: "{app}\docs\appPage\3dTracker"
;Source: "docs\appPage\3dTracker\*.png"; DestDir: "{app}\docs\appPage\3dTracker"
;Source: "docs\appPage\Calibration\*.htm*"; DestDir: "{app}\docs\appPage\Calibration"
;Source: "docs\appPage\Calibration\*.png"; DestDir: "{app}\docs\appPage\Calibration"
;Source: "docs\appPage\Calibration\*.jp*"; DestDir: "{app}\docs\appPage\Calibration"
;Source: "docs\appPage\CamShift\*.htm*"; DestDir: "{app}\docs\appPage\CamShift"
;Source: "docs\appPage\CamShift\*.png"; DestDir: "{app}\docs\appPage\CamShift"
;Source: "docs\appPage\CamShift\*.jp*"; DestDir: "{app}\docs\appPage\CamShift"
;Source: "docs\appPage\ConDensation\*.htm*"; DestDir: "{app}\docs\appPage\ConDensation"
;Source: "docs\appPage\ConDensation\*.png"; DestDir: "{app}\docs\appPage\ConDensation"
;Source: "docs\appPage\ConDensation\*.jp*"; DestDir: "{app}\docs\appPage\ConDensation"
;Source: "docs\appPage\FaceRecognition\*.htm*"; DestDir: "{app}\docs\appPage\FaceRecognition"
;Source: "docs\appPage\FaceRecognition\*.png"; DestDir: "{app}\docs\appPage\FaceRecognition"
;Source: "docs\appPage\FaceRecognition\*.jp*"; DestDir: "{app}\docs\appPage\FaceRecognition"
;Source: "docs\appPage\Kalman\*.htm*"; DestDir: "{app}\docs\appPage\Kalman"
;Source: "docs\appPage\Kalman\*.png"; DestDir: "{app}\docs\appPage\Kalman"
;Source: "docs\appPage\Kalman\*.jp*"; DestDir: "{app}\docs\appPage\Kalman"
;Source: "docs\appPage\LKTracker\*.htm*"; DestDir: "{app}\docs\appPage\LKTracker"
;Source: "docs\appPage\LKTracker\*.png"; DestDir: "{app}\docs\appPage\LKTracker"
;Source: "docs\appPage\LKTracker\*.jp*"; DestDir: "{app}\docs\appPage\LKTracker"

; sample code
Source: "samples\c\*.c*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.sh"; DestDir: "{app}\samples\c"
Source: "samples\c\*.jp*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.png"; DestDir: "{app}\samples\c"
Source: "samples\c\*.dsp"; DestDir: "{app}\samples\c"
Source: "samples\c\*.vcproj"; DestDir: "{app}\samples\c"
Source: "samples\c\Makefile.*"; DestDir: "{app}\samples\c"
Source: "samples\c\*.exe"; DestDir: "{app}\samples\c"

; batch tests
Source: "tests\Makefile.*"; DestDir: "{app}\tests"
Source: "tests\trs\Makefile.*"; DestDir: "{app}\tests\trs"
Source: "tests\trs\*.h*"; DestDir: "{app}\tests\trs"
Source: "tests\trs\*.c*"; DestDir: "{app}\tests\trs"
Source: "tests\trs\*.dsp"; DestDir: "{app}\tests\trs"
Source: "tests\trs\*.vcproj"; DestDir: "{app}\tests\trs"
Source: "tests\cv\Makefile.*"; DestDir: "{app}\tests\cv"
Source: "tests\cv\src\*.c*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.h*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\Makefile.*"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.dsp"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.vcproj"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\src\*.inc"; DestDir: "{app}\tests\cv\src"
Source: "tests\cv\testdata\cameracalibration\*.*"; DestDir: "{app}\tests\cv\testdata\cameracalibration"
Source: "tests\cv\testdata\gesturerecognition\*.*"; DestDir: "{app}\tests\cv\testdata\gesturerecognition"
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

; utilities
Source: "utils\*.cmd"; DestDir: "{app}\utils"
Source: "utils\*.py"; DestDir: "{app}\utils"
Source: "utils\*.iss"; DestDir: "{app}\utils"
Source: "utils\Makefile.*"; DestDir: "{app}\utils"
Source: "utils\cvinfo\*.c*"; DestDir: "{app}\utils\cvinfo"
Source: "utils\cvinfo\*.dsp"; DestDir: "{app}\utils\cvinfo"
Source: "utils\cvinfo\*.vcproj"; DestDir: "{app}\utils\cvinfo"
Source: "utils\cvinfo\Makefile.*"; DestDir: "{app}\utils\cvinfo"

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
Source: "bin\cxcore096.dll"; DestDir: "{app}\bin"
Source: "bin\cv096.dll"; DestDir: "{app}\bin"
Source: "bin\highgui096.dll"; DestDir: "{app}\bin"
Source: "bin\cvaux096.dll"; DestDir: "{app}\bin"
Source: "bin\cxts001.dll"; DestDir: "{app}\bin"
Source: "bin\trs.dll"; DestDir: "{app}\bin"
Source: "bin\cvcam096.dll"; DestDir: "{app}\bin"
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
Source: "lib\cvhaartraining.lib"; DestDir: "{app}\lib"
Source: "lib\cxts.lib"; DestDir: "{app}\lib"
Source: "lib\cvcam.lib"; DestDir: "{app}\lib"
Source: "lib\trs.lib"; DestDir: "{app}\lib"

[Icons]
Name: "{group}\OpenCV Workspace MSVC6"; Filename: "{app}\_make\opencv.dsw"
Name: "{group}\OpenCV Workspace .NET"; Filename: "{app}\_make\opencv.sln"
Name: "{group}\OpenCV Workspace Borland C++ BuilderX"; Filename: "{app}\_make\cbuilderx\opencv.bpgr"
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
Filename: "{app}\bin\RegisterAll.bat"; WorkingDir: "{app}\bin"
Filename: "{app}\docs\index.htm"; Description: "View Documentation"; Flags: postinstall shellexec

[UninstallRun]
Filename: "{app}\bin\RegisterAll.bat"; Parameters: "/U"; WorkingDir: "{app}\bin"

