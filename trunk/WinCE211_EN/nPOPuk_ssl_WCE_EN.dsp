# Microsoft Developer Studio Project File - Name="nPOPuk_WCE_EN" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86) Application" 0x8301
# TARGTYPE "Win32 (WCE ARM) Application" 0x8501
# TARGTYPE "Win32 (WCE x86em) Application" 0x7f01
# TARGTYPE "Win32 (WCE SH3) Application" 0x8101
# TARGTYPE "Win32 (WCE SH4) Application" 0x8601
# TARGTYPE "Win32 (WCE MIPS) Application" 0x8201

CFG=nPOPuk_WCE_EN - Win32 (WCE ARM) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nPOPuk_WCE_EN.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nPOPuk_WCE_EN.mak" CFG="nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE SH4) Release" (based on "Win32 (WCE SH4) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug" (based on "Win32 (WCE SH4) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE ARM) Release" (based on "Win32 (WCE ARM) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug" (based on "Win32 (WCE ARM) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE x86) Release" (based on "Win32 (WCE x86) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE x86) Debug" (based on "Win32 (WCE x86) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Application")
!MESSAGE "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Pro 2.11"
# PROP WCE_FormatVersion "6.0"

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

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
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "." /I "..\EN" /I "..\inc32" /D "MIPS" /D "_MIPS_" /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"MIPSRel/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

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
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "." /I "..\EN" /I "..\inc32" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "MIPS" /d "_MIPS_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /out:"MIPSDbg/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

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
# ADD BASE CPP /nologo /Qsh4 /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /YX /Oxs /c
# ADD CPP /nologo /Qsh4 /M$(CECrtMT) /W3 /I "." /I "..\EN" /I "..\inc32" /D "SHx" /D "SH4" /D "_SH4_" /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "NDEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /out:"SH4Rel/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

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
# ADD BASE CPP /nologo /Qsh4 /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /YX /c
# ADD CPP /nologo /Qsh4 /M$(CECrtMTDebug) /W3 /Zi /Od /I "." /I "..\EN" /I "..\inc32" /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "DEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "DEBUG" /d "UNICODE" /d "_UNICODE" /d "$(CEConfigName)" /d "SHx" /d "SH4" /d "_SH4_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /out:"SH4Dbg/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

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
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "." /I "..\EN" /I "..\inc32" /D "SHx" /D "SH3" /D "_SH3_" /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"SH3Rel/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

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
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "." /I "..\EN" /I "..\inc32" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "SHx" /d "SH3" /d "_SH3_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /out:"SH3Dbg/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

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
# ADD BASE CPP /nologo /M$(CECrtMT) /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /YX /Oxs /c
# ADD CPP /nologo /M$(CECrtMT) /W3 /I "." /I "..\EN" /I "..\inc32" /D "ARM" /D "_ARM_" /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /out:"ARMRel/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

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
# ADD BASE CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /D "DEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /YX /c
# ADD CPP /nologo /M$(CECrtMTDebug) /W3 /Zi /Od /I "." /I "..\EN" /I "..\inc32" /D "DEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "ARM" /d "_ARM_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /out:"ARMDbg/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "X86Rel"
# PROP BASE Intermediate_Dir "X86Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "X86Rel"
# PROP Intermediate_Dir "X86Rel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_i386_" /D UNDER_CE=$(CEVersion) /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /YX /Gs8192 /GF /Oxs /c
# ADD CPP /nologo /W3 /I "." /I "..\EN" /I "..\inc32" /D "_i386_" /D "i_386_" /D "_X86_" /D "x86" /D "NDEBUG" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Gs8192 /GF /Oxs /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "_X86_" /d "x86" /d "_i386_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "$(CEConfigName)" /d "_X86_" /d "x86" /d "_i386_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /machine:IX86 /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 $(CEx86Corelibc) $(CEx86Corelibc) commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /machine:IX86 /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:"$(CENoDefaultLib)" /out:"X86Rel/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "X86Dbg"
# PROP BASE Intermediate_Dir "X86Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "X86Dbg"
# PROP Intermediate_Dir "X86Dbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Zi /Od /D "DEBUG" /D "_i386_" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /YX /Gs8192 /GF /c
# ADD CPP /nologo /W3 /Zi /Od /I "." /I "..\EN" /I "..\inc32" /D "DEBUG" /D "_i386_" /D "i_386_" /D "_X86_" /D "x86" /D UNDER_CE=$(CEVersion) /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "UNICODE" /D "_UNICODE" /D "_NOFILEMAP" /YX /Gs8192 /GF /c
RSC=rc.exe
# ADD BASE RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "_X86_" /d "x86" /d "_i386_"
# ADD RSC /l 0x409 /r /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "DEBUG" /d "$(CEConfigName)" /d "_X86_" /d "x86" /d "_i386_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /debug /machine:IX86 /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:"$(CENoDefaultLib)" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)
# ADD LINK32 $(CEx86Corelibc) $(CEx86Corelibc) commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib /nologo /debug /machine:IX86 /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:"$(CENoDefaultLib)" /out:"X86Dbg/nPOPuk.exe" /stack:0x10000,0x1000 /subsystem:$(CESubsystem)

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

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
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib nologo stack:0x10000,0x1000 subsystem:windows machine:IX86 nodefaultlib:"OLDNAMES.lib" nodefaultlib:"$(CENoDefaultLib)" windowsce:emulation
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib nologo stack:0x10000,0x1000 subsystem:windows machine:IX86 nodefaultlib:"OLDNAMES.lib" nodefaultlib:"$(CENoDefaultLib)" out:"X86EMRel/nPOPuk.exe" windowsce:emulation
BSC32=bscmake.exe
# ADD BASE BSC32 nologo
# ADD BSC32 nologo
MTL=midl.exe
# ADD BASE MTL nologo D "NDEBUG" mktyplib203 o "NUL" win32
# ADD MTL nologo D "NDEBUG" mktyplib203 o "NUL" win32
RSC=rc.exe
# ADD BASE RSC l 0x409 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "NDEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
# ADD RSC l 0x409 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "NDEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
CPP=cl.exe
# ADD BASE CPP nologo Gz W3 D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "i486" D UNDER_CE=$(CEVersion) D "UNICODE" D "_UNICODE" D "_X86_" D "x86" D "NDEBUG" YX Oxs c
# ADD CPP nologo Gz W3 I "." I "..\EN" /I "..\inc32" D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "i486" D "_X86_" D "x86" D "NDEBUG" D UNDER_CE=$(CEVersion) D _WIN32_WCE=$(CEVersion) D "$(CEConfigName)" D "UNICODE" D "_UNICODE" D "_NOFILEMAP" YX Oxs c

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

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
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib nologo stack:0x10000,0x1000 subsystem:windows debug machine:IX86 nodefaultlib:"OLDNAMES.lib" nodefaultlib:"$(CENoDefaultLib)" windowsce:emulation
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib winsock.lib ole32.lib oleaut32.lib nologo stack:0x10000,0x1000 subsystem:windows debug machine:IX86 nodefaultlib:"OLDNAMES.lib" nodefaultlib:"$(CENoDefaultLib)" out:"X86EMDbg/nPOPuk.exe" windowsce:emulation
BSC32=bscmake.exe
# ADD BASE BSC32 nologo
# ADD BSC32 nologo
MTL=midl.exe
# ADD BASE MTL nologo D "_DEBUG" mktyplib203 o "NUL" win32
# ADD MTL nologo D "_DEBUG" mktyplib203 o "NUL" win32
RSC=rc.exe
# ADD BASE RSC l 0x409 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "DEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
# ADD RSC l 0x409 d UNDER_CE=$(CEVersion) d _WIN32_WCE=$(CEVersion) d "UNICODE" d "_UNICODE" d "DEBUG" d "$(CEConfigName)" d "_X86_" d "x86" d "i486" r
CPP=cl.exe
# ADD BASE CPP nologo Gz W3 Zi Od D "DEBUG" D "i486" D UNDER_CE=$(CEVersion) D _WIN32_WCE=$(CEVersion) D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "$(CEConfigName)" D "UNICODE" D "_UNICODE" D "_X86_" D "x86" YX c
# ADD CPP nologo Gz W3 Zi Od I "." I "..\EN" /I "..\inc32" D "DEBUG" D "i486" D "WIN32" D "STRICT" D "_WIN32_WCE_EMULATION" D "INTERNATIONAL" D "USA" D "INTLMSG_CODEPAGE" D "_X86_" D "x86" D UNDER_CE=$(CEVersion) D _WIN32_WCE=$(CEVersion) D "$(CEConfigName)" D "UNICODE" D "_UNICODE" D "_NOFILEMAP" YX c

!ENDIF 

# Begin Target

# Name "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"
# Name "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"
# Name "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"
# Name "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"
# Name "nPOPuk_WCE_EN - Win32 (WCE x86) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"
# Name "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"
# Name "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\Charset.cpp

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_CHARS=\
	"..\src\Charset.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Code.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_CODE_=\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\kr.h"\
	".\stdafx.h"\
	
NODEP_CPP_CODE_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Edit.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_EDIT_=\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_EDIT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_EDIT_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\File.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_FILE_=\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\ppcpoom.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_FILE_=\
	"..\src\General.h"\
	"..\src\SelectFile.h"\
	".\stdafx.h"\
	
NODEP_CPP_FILE_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Font.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_FONT_=\
	"..\src\Font.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_FONT_=\
	"..\src\Font.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_FONT_=\
	"..\src\Font.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_FONT_=\
	"..\src\Font.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_FONT_=\
	"..\src\Font.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_FONT_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_FONT_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Ini.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_INI_C=\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_INI_C=\
	"..\src\General.h"\
	"..\src\Profile.h"\
	".\stdafx.h"\
	
NODEP_CPP_INI_C=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Item.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_ITEM_=\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_ITEM_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_ITEM_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\jp.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_JP_Ce=\
	"..\src\Font.h"\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_JP_Ce=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_JP_Ce=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\ListView.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_LISTV=\
	"..\src\Font.h"\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_LISTV=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_LISTV=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\MailBox.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_MAILB=\
	"..\src\Font.h"\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_MAILB=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAILB=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\main.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_MAIN_=\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\nEdit.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_MAIN_=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MAIN_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\md5c.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_MD5C_=\
	"..\src\global.h"\
	"..\src\md5.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Memory.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\mime.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_MIME_=\
	"..\src\Charset.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MIME_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\MultiPart.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_MULTI=\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_MULTI=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_MULTI=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nPOP.rc

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Option.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	"..\src\ppcpoom.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_OPTIO=\
	"..\src\Charset.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\jp.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_OPTIO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_OPTIO=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Pop3.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_POP3_=\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_POP3_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_POP3_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\ppcpoom.cpp

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_PPCPO=\
	"..\src\ppcpoom.h"\
	".\Pimstore.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Profile.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_PROFI=\
	"..\src\Profile.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Ras.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_RAS_C=\
	"..\src\Font.h"\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_RAS_C=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_RAS_C=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Smtp.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_SMTP_=\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_SMTP_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_SMTP_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\String.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_STRIN=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_STRIN=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\util.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_UTIL_=\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_UTIL_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_UTIL_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\View.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_VIEW_=\
	"..\src\code.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	"..\src\mime.h"\
	"..\src\multipart.h"\
	"..\src\nEdit.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_VIEW_=\
	"..\src\General.h"\
	"..\src\global.h"\
	"..\src\md5.h"\
	".\stdafx.h"\
	
NODEP_CPP_VIEW_=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\WinSockSSL.c

!IF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Release"

DEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sslsock.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE MIPS) Debug"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Release"

DEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sslsock.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH4) Debug"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Release"

DEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sslsock.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE SH3) Debug"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Release"

DEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	"..\src\Font.h"\
	"..\src\General.h"\
	"..\src\npop_ver.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"sslsock.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE ARM) Debug"

DEP_CPP_WINSO=\
	"..\src\Font.h"\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	".\slsock.h"\
	".\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Release"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86) Debug"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Release"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOPuk_WCE_EN - Win32 (WCE x86em) Debug"

DEP_CPP_WINSO=\
	"..\src\General.h"\
	".\stdafx.h"\
	
NODEP_CPP_WINSO=\
	"..\EN\Strtbl.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\Charset.h
# End Source File
# Begin Source File

SOURCE=..\src\code.h
# End Source File
# Begin Source File

SOURCE=..\src\Font.h
# End Source File
# Begin Source File

SOURCE=..\src\General.h
# End Source File
# Begin Source File

SOURCE=..\src\global.h
# End Source File
# Begin Source File

SOURCE=..\src\jp.h
# End Source File
# Begin Source File

SOURCE=..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\Memory.h
# End Source File
# Begin Source File

SOURCE=..\src\mime.h
# End Source File
# Begin Source File

SOURCE=..\src\multipart.h
# End Source File
# Begin Source File

SOURCE=..\src\npop_ver.h
# End Source File
# Begin Source File

SOURCE=.\Pimstore.h
# End Source File
# Begin Source File

SOURCE=..\src\ppcpoom.h
# End Source File
# Begin Source File

SOURCE=..\src\Profile.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=..\src\String.h
# End Source File
# Begin Source File

SOURCE=..\EN\Strtbl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\Res\Ico_clhi.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_cllo.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_down.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_flag.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_high.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_main.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_nchk.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_none.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_read.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_repl.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_send.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Ico_sent.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Icon_chk.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_cli.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Icon_del.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Icon_fwd.ico
# End Source File
# Begin Source File

SOURCE=..\Res\Icon_low.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_new.ico
# End Source File
# Begin Source File

SOURCE=..\Res\tbar_edit.bmp
# End Source File
# Begin Source File

SOURCE=..\Res\tbar_view.bmp
# End Source File
# Begin Source File

SOURCE=..\Res\Toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
