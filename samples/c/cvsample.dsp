# Microsoft Developer Studio Project File - Name="cvsample" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=cvsample - Win32 Debug64 Itanium
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cvsample.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cvsample.mak" CFG="cvsample - Win32 Debug64 Itanium"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cvsample - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "cvsample - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "cvsample - Win32 Release64" (based on "Win32 (x86) Console Application")
!MESSAGE "cvsample - Win32 Debug64" (based on "Win32 (x86) Console Application")
!MESSAGE "cvsample - Win32 Release64 Itanium" (based on "Win32 (x86) Console Application")
!MESSAGE "cvsample - Win32 Debug64 Itanium" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
RSC=rc.exe

!IF  "$(CFG)" == "cvsample - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cvsample_Rls"
# PROP Intermediate_Dir "..\..\_temp\cvsample_Rls"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /Gm /GX /Zi /O2 /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib cv.lib highgui.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libmmd.lib" /out:".\cvsample.exe" /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "cvsample - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cvsample_Dbg"
# PROP Intermediate_Dir "..\..\_temp\cvsample_Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib cvd.lib highguid.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libmmdd.lib" /out:".\cvsampled.exe" /pdbtype:sept /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "cvsample - Win32 Release64"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cvsample___Win32_Release64"
# PROP BASE Intermediate_Dir "cvsample___Win32_Release64"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cvsample_Rls64"
# PROP Intermediate_Dir "..\..\_temp\cvsample_Rls64"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /Zi /O2 /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /Gm /GX /Zi /O2 /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "NDEBUG" /D "WIN32" /D "WIN64" /D "EM64T" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore.lib cv.lib highgui.lib /nologo /subsystem:console /debug /machine:IX86 /nodefaultlib:"libmmd.lib" /out:".\cvsample.exe" /libpath:"../../lib" /machine:AMD64
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore_64.lib cv_64.lib highgui_64.lib /nologo /subsystem:console /debug /machine:IX86 /nodefaultlib:"libmmd.lib" /out:".\cvsample_64.exe" /libpath:"../../lib" /machine:AMD64
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "cvsample - Win32 Debug64"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cvsample___Win32_Debug64"
# PROP BASE Intermediate_Dir "cvsample___Win32_Debug64"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cvsample_Dbg64"
# PROP Intermediate_Dir "..\..\_temp\cvsample_Dbg64"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /Zi /Od /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "WIN64" /D "EM64T" /YX /FD /Wp64 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored.lib cvd.lib highguid.lib /nologo /subsystem:console /debug /machine:IX86 /nodefaultlib:"libmmdd.lib" /out:".\cvsampled.exe" /pdbtype:sept /libpath:"../../lib" /machine:AMD64
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored_64.lib cvd_64.lib highguid_64.lib /nologo /subsystem:console /pdb:".\cvsampled_64.pdb" /debug /machine:IX86 /nodefaultlib:"libmmdd.lib" /out:".\cvsampled_64.exe" /libpath:"../../lib" /machine:AMD64
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "cvsample - Win32 Release64 Itanium"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cvsample___Win32_Release64_Itanium"
# PROP BASE Intermediate_Dir "cvsample___Win32_Release64_Itanium"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\_temp\cvsample_RlsI7"
# PROP Intermediate_Dir "..\..\_temp\cvsample_RlsI7"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /Zi /O2 /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "NDEBUG" /D "WIN32" /D "EM64T" /YX /FD /c
# ADD CPP /nologo /MD /w /W0 /Gm /GX /Zi /O2 /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "NDEBUG" /D "WIN32" /D "WIN64" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore_i7.lib cv_i7.lib highgui_i7.lib /nologo /subsystem:console /debug /machine:IX86 /nodefaultlib:"libmmd.lib" /out:".\cvsample_i7.exe" /libpath:"../../lib" /machine:IA64
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcore_i7.lib cv_i7.lib highgui_i7.lib /nologo /subsystem:console /debug /machine:IX86 /out:".\cvsample_i7.exe" /libpath:"../../lib" /machine:IA64
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "cvsample - Win32 Debug64 Itanium"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cvsample___Win32_Debug64_Itanium"
# PROP BASE Intermediate_Dir "cvsample___Win32_Debug64_Itanium"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\_temp\cvsample_DbgI7"
# PROP Intermediate_Dir "..\..\_temp\cvsample_DbgI7"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /Zi /Od /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "EM64T" /YX /FD /Wp64 /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../cxcore/include" /I "../../cv/include" /I "../../otherlibs/highgui" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "WIN64" /YX /FD /Qwd167 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored_i7.lib cvd_i7.lib highguid_i7.lib /nologo /subsystem:console /pdb:".\cvsampled_i7.pdb" /debug /machine:IX86 /nodefaultlib:"libmmdd.lib" /out:".\cvsampled_i7.exe" /libpath:"../../lib" /machine:IA64
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cxcored_i7.lib cvd_i7.lib highguid_i7.lib /nologo /subsystem:console /pdb:".\cvsampled_i7.pdb" /debug /machine:IX86 /out:".\cvsampled_i7.exe" /libpath:"../../lib" /machine:IA64
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "cvsample - Win32 Release"
# Name "cvsample - Win32 Debug"
# Name "cvsample - Win32 Release64"
# Name "cvsample - Win32 Debug64"
# Name "cvsample - Win32 Release64 Itanium"
# Name "cvsample - Win32 Debug64 Itanium"
# Begin Source File

SOURCE=.\facedetect.c
# End Source File
# End Target
# End Project
