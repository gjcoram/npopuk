# Microsoft Developer Studio Project File - Name="zlib_wce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86) Static Library" 0x8304
# TARGTYPE "Win32 (WCE SH4) Static Library" 0x8604
# TARGTYPE "Win32 (WCE MIPS) Static Library" 0x8204
# TARGTYPE "Win32 (WCE ARM) Static Library" 0x8504
# TARGTYPE "Win32 (WCE SH3) Static Library" 0x8104

CFG=zlib_wce - Win32 (WCE MIPS) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib_wce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib_wce.mak" CFG="zlib_wce - Win32 (WCE MIPS) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib_wce - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE SH4) Release" (based on "Win32 (WCE SH4) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE SH4) Debug" (based on "Win32 (WCE SH4) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE ARM) Release" (based on "Win32 (WCE ARM) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE ARM) Debug" (based on "Win32 (WCE ARM) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE x86) Release" (based on "Win32 (WCE x86) Static Library")
!MESSAGE "zlib_wce - Win32 (WCE x86) Debug" (based on "Win32 (WCE x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Ver. 2.00"
# PROP WCE_FormatVersion "6.0"

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib_mips"
# PROP BASE Intermediate_Dir "lib_mips"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib_mips"
# PROP Intermediate_Dir "lib_mips"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /QMRWCE /c
# ADD CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /QMRWCE /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSDbg"
# PROP BASE Intermediate_Dir "WMIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSDbg"
# PROP Intermediate_Dir "WMIPSDbg"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /QMRWCE /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /QMRWCE /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib_sh4"
# PROP BASE Intermediate_Dir "lib_sh4"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib_sh4"
# PROP Intermediate_Dir "lib_sh4"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /Qsh4 /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESH4Dbg"
# PROP BASE Intermediate_Dir "WCESH4Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESH4Dbg"
# PROP Intermediate_Dir "WCESH4Dbg"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /Qsh4 /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib_sh3"
# PROP BASE Intermediate_Dir "lib_sh3"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib_sh3"
# PROP Intermediate_Dir "lib_sh3"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESH3Dbg"
# PROP BASE Intermediate_Dir "WCESH3Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESH3Dbg"
# PROP Intermediate_Dir "WCESH3Dbg"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib_arm"
# PROP BASE Intermediate_Dir "lib_arm"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "lib_arm"
# PROP Intermediate_Dir "lib_arm"
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCEARMDbg"
# PROP BASE Intermediate_Dir "WCEARMDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCEARMDbg"
# PROP Intermediate_Dir "WCEARMDbg"
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
# ADD CPP /nologo /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_LIB" /YX /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCEX86Rel"
# PROP BASE Intermediate_Dir "WCEX86Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCEX86Rel"
# PROP Intermediate_Dir "WCEX86Rel"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Gs8192 /GF /c
# ADD CPP /nologo /ML /W3 /O2 /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Gs8192 /GF /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCEX86Dbg"
# PROP BASE Intermediate_Dir "WCEX86Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCEX86Dbg"
# PROP Intermediate_Dir "WCEX86Dbg"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "DEBUG" /D "_MBCS" /D "_LIB" /Gs8192 /GF /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "DEBUG" /D "_MBCS" /D "_LIB" /Gs8192 /GF /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "zlib_wce - Win32 (WCE MIPS) Release"
# Name "zlib_wce - Win32 (WCE MIPS) Debug"
# Name "zlib_wce - Win32 (WCE SH4) Release"
# Name "zlib_wce - Win32 (WCE SH4) Debug"
# Name "zlib_wce - Win32 (WCE SH3) Release"
# Name "zlib_wce - Win32 (WCE SH3) Debug"
# Name "zlib_wce - Win32 (WCE ARM) Release"
# Name "zlib_wce - Win32 (WCE ARM) Debug"
# Name "zlib_wce - Win32 (WCE x86) Release"
# Name "zlib_wce - Win32 (WCE x86) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\zlib123\adler32.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_ADLER=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\compress.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_COMPR=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\crc32.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_CRC32=\
	"..\zlib123\crc32.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\deflate.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_DEFLA=\
	"..\zlib123\deflate.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\infback.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_INFBA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\inffast.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_INFFA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\inflate.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_INFLA=\
	"..\zlib123\inffast.h"\
	"..\zlib123\inffixed.h"\
	"..\zlib123\inflate.h"\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\inftrees.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_INFTR=\
	"..\zlib123\inftrees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\trees.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_TREES=\
	"..\zlib123\deflate.h"\
	"..\zlib123\trees.h"\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\uncompr.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_UNCOM=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\zlib123\zutil.c

!IF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Release"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE MIPS) Debug"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Release"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH4) Debug"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Release"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE SH3) Debug"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Release"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE ARM) Debug"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Release"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ELSEIF  "$(CFG)" == "zlib_wce - Win32 (WCE x86) Debug"

DEP_CPP_ZUTIL=\
	"..\zlib123\zconf.h"\
	"..\zlib123\zlib.h"\
	"..\zlib123\zutil.h"\
	

!ENDIF 

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

SOURCE=..\zlib123\zlib.h
# End Source File
# Begin Source File

SOURCE=..\zlib123\zutil.h
# End Source File
# End Group
# End Target
# End Project
