# Microsoft Developer Studio Project File - Name="npopssl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x7f02
# TARGTYPE "Win32 (WCE SH3) Dynamic-Link Library" 0x8102
# TARGTYPE "Win32 (WCE ARM) Dynamic-Link Library" 0x8502
# TARGTYPE "Win32 (WCE SH4) Dynamic-Link Library" 0x8602
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x8202

CFG=npopssl - Win32 (WCE MIPS) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NPOPSSL_WCE.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NPOPSSL_WCE.MAK" CFG="npopssl - Win32 (WCE MIPS) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "npopssl - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE SH4) Release" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE SH4) Debug" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE ARM) Release" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE ARM) Debug" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "npopssl - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Ver. 2.00"
# PROP WCE_FormatVersion "6.0"

!IF  "$(CFG)" == "npopssl - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MIPSRel"
# PROP BASE Intermediate_Dir "MIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MIPSRel"
# PROP Intermediate_Dir "MIPSRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_mips" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MIPSDbg"
# PROP BASE Intermediate_Dir "MIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MIPSDbg"
# PROP Intermediate_Dir "MIPSDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_mips" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH4) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SH4Rel"
# PROP BASE Intermediate_Dir "SH4Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SH4Rel"
# PROP Intermediate_Dir "SH4Rel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
# ADD CPP /nologo /Qsh4 /M$(CECrtMT) /W3 /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_sh4" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH4) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SH4Dbg"
# PROP BASE Intermediate_Dir "SH4Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SH4Dbg"
# PROP Intermediate_Dir "SH4Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
# ADD CPP /nologo /Qsh4 /M$(CECrtMTDebug) /W3 /Zi /Od /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "DEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "DEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_sh4" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SH3Rel"
# PROP BASE Intermediate_Dir "SH3Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SH3Rel"
# PROP Intermediate_Dir "SH3Rel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_sh3" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH3) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SH3Dbg"
# PROP BASE Intermediate_Dir "SH3Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SH3Dbg"
# PROP Intermediate_Dir "SH3Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_sh3" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE ARM) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ARMRel"
# PROP BASE Intermediate_Dir "ARMRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ARMRel"
# PROP Intermediate_Dir "ARMRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_arm" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE ARM) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ARMDbg"
# PROP BASE Intermediate_Dir "ARMDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ARMDbg"
# PROP Intermediate_Dir "ARMDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "wcecompat\include" /I "C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE" /I "C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\INCLUDE" /D "DEBUG" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "NPOPSSL_EXPORTS" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
# ADD RSC /l 0x411 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /base:"0x00100000" /dll /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 wcecompat.lib wcecompatex.lib winsock.lib commctrl.lib coredll.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompat.lib C:\MYDOCU~1\VISUAL~1\WCECOM~1.2\lib\wcecompatex.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\ssleay32.lib C:\MYDOCU~1\VISUAL~1\OPENSS~1.8B\OUT32D~1\libeay32.lib /nologo /base:"0x00100000" /dll /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /libpath:"openssl\lib_arm" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "X86EMRel"
# PROP BASE Intermediate_Dir "X86EMRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "X86EMRel"
# PROP Intermediate_Dir "X86EMRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib nologo stack:0x10000,0x1000 dll nodefaultlib:"OLDNAMES.lib" nodefaultlib:$(CENoDefaultLib) windowsce:emulation MACHINE:IX86
# ADD LINK32 $(CEx86Corelibc) winsock.lib wcecompat.lib wcecompatex.lib libeay32.lib ssleay32.lib commctrl.lib coredll.lib nologo stack:0x10000,0x1000 dll nodefaultlib:"OLDNAMES.lib" nodefaultlib:$(CENoDefaultLib) libpath:"openssl\lib_x86em" windowsce:emulation MACHINE:IX86 nodefaultlib:$(CENoDefaultLib)
BSC32=bscmake.exe
# ADD BASE BSC32 nologo
# ADD BSC32 nologo
MTL=midl.exe
# ADD BASE MTL nologo D "NDEBUG" mktyplib203 o "NUL" win32
# ADD MTL nologo D "NDEBUG" mktyplib203 o "NUL" win32
CPP=cl.exe
# ADD BASE CPP nologo W3 D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "i486" D UNDER_CE=$(CEVersion) D "UNICODE" D "_UNICODE" D "_X86_" D "x86" D "NDEBUG" D "_USRDLL" D "NPOPSSL_EXPORTS" YX Gz Oxs c
# ADD CPP nologo W3 I "openssl\include" I "wcecompat\include" D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "i486" D UNDER_CE=$(CEVersion) D "UNICODE" D "_UNICODE" D "_X86_" D "x86" D "NDEBUG" D "_USRDLL" D "NPOPSSL_EXPORTS" YX Gz Oxs c
RSC=rc.exe
# ADD BASE RSC l 0x411 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "NDEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
# ADD RSC l 0x411 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "NDEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "X86EMDbg"
# PROP BASE Intermediate_Dir "X86EMDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "X86EMDbg"
# PROP Intermediate_Dir "X86EMDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib nologo stack:0x10000,0x1000 dll debug nodefaultlib:"OLDNAMES.lib" nodefaultlib:$(CENoDefaultLib) windowsce:emulation MACHINE:IX86
# ADD LINK32 $(CEx86Corelibc) winsock.lib wcecompat.lib wcecompatex.lib libeay32.lib ssleay32.lib commctrl.lib coredll.lib nologo stack:0x10000,0x1000 dll debug nodefaultlib:"OLDNAMES.lib" nodefaultlib:$(CENoDefaultLib) libpath:"openssl\lib_x86em" windowsce:emulation MACHINE:IX86 nodefaultlib:$(CENoDefaultLib)
BSC32=bscmake.exe
# ADD BASE BSC32 nologo
# ADD BSC32 nologo
MTL=midl.exe
# ADD BASE MTL nologo D "_DEBUG" mktyplib203 o "NUL" win32
# ADD MTL nologo D "_DEBUG" mktyplib203 o "NUL" win32
CPP=cl.exe
# ADD BASE CPP nologo W3 Zi Od D "DEBUG" D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "i486" D UNDER_CE=$(CEVersion) D "UNICODE" D "_UNICODE" D "_X86_" D "x86" D "_USRDLL" D "NPOPSSL_EXPORTS" YX Gz c
# ADD CPP nologo W3 Zi Od I "openssl\include" I "wcecompat\include" D "DEBUG" D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "i486" D UNDER_CE=$(CEVersion) D "UNICODE" D "_UNICODE" D "_X86_" D "x86" D "_USRDLL" D "NPOPSSL_EXPORTS" YX Gz c
RSC=rc.exe
# ADD BASE RSC l 0x411 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "DEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
# ADD RSC l 0x411 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "DEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r

!ENDIF 

# Begin Target

# Name "npopssl - Win32 (WCE MIPS) Release"
# Name "npopssl - Win32 (WCE MIPS) Debug"
# Name "npopssl - Win32 (WCE SH4) Release"
# Name "npopssl - Win32 (WCE SH4) Debug"
# Name "npopssl - Win32 (WCE SH3) Release"
# Name "npopssl - Win32 (WCE SH3) Debug"
# Name "npopssl - Win32 (WCE ARM) Release"
# Name "npopssl - Win32 (WCE ARM) Debug"
# Name "npopssl - Win32 (WCE x86em) Release"
# Name "npopssl - Win32 (WCE x86em) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\npopssl.c

!IF  "$(CFG)" == "npopssl - Win32 (WCE MIPS) Release"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE MIPS) Debug"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH4) Release"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH4) Debug"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH3) Release"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE SH3) Debug"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE ARM) Release"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE ARM) Debug"

DEP_CPP_NPOPS=\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\asn1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bio.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\bn.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\buffer.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\comp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\crypto.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\dtls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\e_os2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ebcdic.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ec.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdh.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ecdsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\err.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\evp.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\kssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\lhash.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\obj_mac.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\objects.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslconf.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\opensslv.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ossl_typ.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pem2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs12.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pkcs7.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pq_compat.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\pqueue.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rand.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\rsa.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\safestack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\sha.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl2.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl23.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\ssl3.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\stack.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\symhacks.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\tls1.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509.h"\
	"..\..\..\..\MYDOCU~1\VISUAL~1\OPENSS~1.8B\INCLUDE\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE x86em) Release"

NODEP_CPP_NPOPS=\
	".\openssl\include\openssl\asn1.h"\
	".\openssl\include\openssl\bio.h"\
	".\openssl\include\openssl\bn.h"\
	".\openssl\include\openssl\buffer.h"\
	".\openssl\include\openssl\comp.h"\
	".\openssl\include\openssl\crypto.h"\
	".\openssl\include\openssl\dh.h"\
	".\openssl\include\openssl\dsa.h"\
	".\openssl\include\openssl\dtls1.h"\
	".\openssl\include\openssl\e_os2.h"\
	".\openssl\include\openssl\ebcdic.h"\
	".\openssl\include\openssl\ec.h"\
	".\openssl\include\openssl\ecdh.h"\
	".\openssl\include\openssl\ecdsa.h"\
	".\openssl\include\openssl\err.h"\
	".\openssl\include\openssl\evp.h"\
	".\openssl\include\openssl\kssl.h"\
	".\openssl\include\openssl\lhash.h"\
	".\openssl\include\openssl\obj_mac.h"\
	".\openssl\include\openssl\objects.h"\
	".\openssl\include\openssl\opensslconf.h"\
	".\openssl\include\openssl\opensslv.h"\
	".\openssl\include\openssl\ossl_typ.h"\
	".\openssl\include\openssl\pem.h"\
	".\openssl\include\openssl\pem2.h"\
	".\openssl\include\openssl\pkcs12.h"\
	".\openssl\include\openssl\pkcs7.h"\
	".\openssl\include\openssl\pq_compat.h"\
	".\openssl\include\openssl\pqueue.h"\
	".\openssl\include\openssl\rand.h"\
	".\openssl\include\openssl\rsa.h"\
	".\openssl\include\openssl\safestack.h"\
	".\openssl\include\openssl\sha.h"\
	".\openssl\include\openssl\ssl.h"\
	".\openssl\include\openssl\ssl2.h"\
	".\openssl\include\openssl\ssl23.h"\
	".\openssl\include\openssl\ssl3.h"\
	".\openssl\include\openssl\stack.h"\
	".\openssl\include\openssl\symhacks.h"\
	".\openssl\include\openssl\tls1.h"\
	".\openssl\include\openssl\x509.h"\
	".\openssl\include\openssl\x509_vfy.h"\
	

!ELSEIF  "$(CFG)" == "npopssl - Win32 (WCE x86em) Debug"

NODEP_CPP_NPOPS=\
	".\openssl\include\openssl\asn1.h"\
	".\openssl\include\openssl\bio.h"\
	".\openssl\include\openssl\bn.h"\
	".\openssl\include\openssl\buffer.h"\
	".\openssl\include\openssl\comp.h"\
	".\openssl\include\openssl\crypto.h"\
	".\openssl\include\openssl\dh.h"\
	".\openssl\include\openssl\dsa.h"\
	".\openssl\include\openssl\dtls1.h"\
	".\openssl\include\openssl\e_os2.h"\
	".\openssl\include\openssl\ebcdic.h"\
	".\openssl\include\openssl\ec.h"\
	".\openssl\include\openssl\ecdh.h"\
	".\openssl\include\openssl\ecdsa.h"\
	".\openssl\include\openssl\err.h"\
	".\openssl\include\openssl\evp.h"\
	".\openssl\include\openssl\kssl.h"\
	".\openssl\include\openssl\lhash.h"\
	".\openssl\include\openssl\obj_mac.h"\
	".\openssl\include\openssl\objects.h"\
	".\openssl\include\openssl\opensslconf.h"\
	".\openssl\include\openssl\opensslv.h"\
	".\openssl\include\openssl\ossl_typ.h"\
	".\openssl\include\openssl\pem.h"\
	".\openssl\include\openssl\pem2.h"\
	".\openssl\include\openssl\pkcs12.h"\
	".\openssl\include\openssl\pkcs7.h"\
	".\openssl\include\openssl\pq_compat.h"\
	".\openssl\include\openssl\pqueue.h"\
	".\openssl\include\openssl\rand.h"\
	".\openssl\include\openssl\rsa.h"\
	".\openssl\include\openssl\safestack.h"\
	".\openssl\include\openssl\sha.h"\
	".\openssl\include\openssl\ssl.h"\
	".\openssl\include\openssl\ssl2.h"\
	".\openssl\include\openssl\ssl23.h"\
	".\openssl\include\openssl\ssl3.h"\
	".\openssl\include\openssl\stack.h"\
	".\openssl\include\openssl\symhacks.h"\
	".\openssl\include\openssl\tls1.h"\
	".\openssl\include\openssl\x509.h"\
	".\openssl\include\openssl\x509_vfy.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
