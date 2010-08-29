# Microsoft Developer Studio Project File - Name="zlib_win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib_win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib_win32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib_win32.mak" CFG="zlib_win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib_win32 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib_win32 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
# PROP WCE_FormatVersion "6.0"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib_win32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "out32"
# PROP BASE Intermediate_Dir "out32"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "out32"
# PROP Intermediate_Dir "out32"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_win32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "zlib_win32 - Win32 Release"
# Name "zlib_win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\zlib123\adler32.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\compress.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\crc32.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\deflate.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\infback.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\inffast.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\inflate.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\trees.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\zlib123\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\zlib123\crc32.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\deflate.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\inffast.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\inflate.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\trees.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\zconf.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\zconf.in.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\zlib.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\zutil.h
# End Source File
# End Group
# End Target
# End Project
