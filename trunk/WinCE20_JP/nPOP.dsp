# Microsoft Developer Studio Project File - Name="nPOP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** 編集しないでください **

# TARGTYPE "Win32 (WCE x86em) Application" 0x0b01
# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Win32 (WCE MIPS) Application" 0x0a01
# TARGTYPE "Win32 (WCE SH) Application" 0x0901

CFG=nPOP - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak" CFG="nPOP - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "nPOP - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "nPOP - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "nPOP - Win32 (WCE x86em) Release" ("Win32 (WCE x86em) Application" 用)
!MESSAGE "nPOP - Win32 (WCE x86em) Debug" ("Win32 (WCE x86em) Application" 用)
!MESSAGE "nPOP - Win32 (WCE MIPS) Release" ("Win32 (WCE MIPS) Application" 用)
!MESSAGE "nPOP - Win32 (WCE MIPS) Debug" ("Win32 (WCE MIPS) Application" 用)
!MESSAGE "nPOP - Win32 (WCE SH) Release" ("Win32 (WCE SH) Application" 用)
!MESSAGE "nPOP - Win32 (WCE SH) Debug" ("Win32 (WCE SH) Application" 用)
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "nPOP - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Gr /W3 /GX /Ox /Og /Os /Ob0 /Gf /Gy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WSAASYNC" /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WSAASYNC" /D "UNICODE" /D "_UNICODE" /U "UNICODE" /U "_UNICODE" /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x86emRel"
# PROP BASE Intermediate_Dir "x86emRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x86emRel"
# PROP Intermediate_Dir "x86emRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /O2 /D "WIN32" /D "_WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /D "_WIN32_WCE" /D "UNDER_NT" /D "_WIN32_WCE_EMULATION" /D "UNICODE" /D "_UNICODE" /D "_X86_" /YX /c
# ADD CPP /nologo /MD /W3 /D "WIN32" /D "_WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STRICT" /D "UNDER_NT" /D "_WIN32_WCE_EMULATION" /D "_UNICODE" /D "_X86_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "_WIN32_WCE" /d "UNDER_NT" /d "_WIN32_WCE_EMULATION" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /d "_WIN32_WCE" /d "UNDER_NT" /d "_WIN32_WCE_EMULATION" /d "UNICODE" /d "NDEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib coredll.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /windowsce:emulation
# ADD LINK32 comctl32.lib coredll.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo /subsystem:windows /machine:I386 /windowsce:emulation
EMPFILE=empfile.exe
# ADD BASE EMPFILE -NOSHELL -COPY
# ADD EMPFILE -NOSHELL -COPY

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x86emDbg"
# PROP BASE Intermediate_Dir "x86emDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x86emDbg"
# PROP Intermediate_Dir "x86emDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /Zi /Od /D "WIN32" /D "_WIN32" /D "_DEBUG" /D "_WINDOWS" /D "STRICT" /D "_WIN32_WCE" /D "UNDER_NT" /D "_WIN32_WCE_EMULATION" /D "UNICODE" /D "_UNICODE" /D "_X86_" /YX /c
# ADD CPP /nologo /MDd /W3 /Gm /Zi /Od /D "WIN32" /D "_WIN32" /D "_DEBUG" /D "_WINDOWS" /D "STRICT" /D "UNDER_NT" /D "_WIN32_WCE_EMULATION" /D "_UNICODE" /D "_X86_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d "_WIN32_WCE" /d "UNDER_NT" /d "_WIN32_WCE_EMULATION" /d "UNICODE" /d "_DEBUG"
# ADD RSC /l 0x411 /d "_WIN32_WCE" /d "UNDER_NT" /d "_WIN32_WCE_EMULATION" /d "UNICODE" /d "_DEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib coredll.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /windowsce:emulation
# ADD LINK32 comctl32.lib coredll.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /windowsce:emulation
# SUBTRACT LINK32 /nodefaultlib
EMPFILE=empfile.exe
# ADD BASE EMPFILE -NOSHELL -COPY
# ADD EMPFILE -NOSHELL -COPY

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WMIPSRel"
# PROP BASE Intermediate_Dir "WMIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WMIPSRel"
# PROP Intermediate_Dir "WMIPSRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D "_WIN32_WCE" /D "UNICODE" /YX /QMRWCE /c
# ADD CPP /nologo /ML /W3 /Ox /Og /Os /Ob0 /Gf /Gy /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d "_WIN32_WCE" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d "_WIN32_WCE" /d "UNICODE" /d "NDEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:MIPS /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /machine:MIPS /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSDbg"
# PROP BASE Intermediate_Dir "WMIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSDbg"
# PROP Intermediate_Dir "WMIPSDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "MIPS" /D "_MIPS_" /D "_WIN32_WCE" /D "UNICODE" /YX /QMRWCE /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "MIPS" /D "_MIPS_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /QMRWCE /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:MIPS /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /debug /machine:MIPS /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESHRel"
# PROP BASE Intermediate_Dir "WCESHRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCESHRel"
# PROP Intermediate_Dir "WCESHRel"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "NDEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /ML /W3 /Ox /Og /Os /Gf /Gy /D "NDEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "NDEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /machine:SH3 /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /machine:SH3 /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESHDbg"
# PROP BASE Intermediate_Dir "WCESHDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESHDbg"
# PROP Intermediate_Dir "WCESHDbg"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /YX /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /D "DEBUG" /D "SH3" /D "_SH3_" /D "_WIN32_WCE" /D "UNICODE" /D "_NOFILEMAP" /U "_WIN32_WCE" /YX /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "SH3" /d "_SH3_" /d "_WIN32_WCE" /d "UNICODE" /d "DEBUG" /d "_WIN32_WCE_MONO"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /debug /machine:SH3 /subsystem:windowsce
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib winsock.lib /nologo /debug /machine:SH3 /subsystem:windowsce
# SUBTRACT LINK32 /pdb:none /nodefaultlib
PFILE=pfile.exe
# ADD BASE PFILE COPY
# ADD PFILE COPY

!ENDIF 

# Begin Target

# Name "nPOP - Win32 Release"
# Name "nPOP - Win32 Debug"
# Name "nPOP - Win32 (WCE x86em) Release"
# Name "nPOP - Win32 (WCE x86em) Debug"
# Name "nPOP - Win32 (WCE MIPS) Release"
# Name "nPOP - Win32 (WCE MIPS) Debug"
# Name "nPOP - Win32 (WCE SH) Release"
# Name "nPOP - Win32 (WCE SH) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Code.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_CODE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_CODE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_CODE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Edit.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_EDIT_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_EDIT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_EDIT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_EDIT_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_EDIT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_EDIT_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\File.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_FILE_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_FILE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_FILE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_FILE_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_FILE_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_FILE_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Font.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_FONT_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_FONT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_FONT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_FONT_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_FONT_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_FONT_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ini.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_INI_C=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_INI_C=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_INI_C=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_INI_C=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_INI_C=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_INI_C=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Item.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_ITEM_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_ITEM_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_ITEM_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_ITEM_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_ITEM_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_ITEM_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jp.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\kr.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ListView.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_LISTV=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_LISTV=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_LISTV=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_LISTV=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_LISTV=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_LISTV=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MailBox.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_MAILB=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_MAILB=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_MAILB=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_MAILB=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_MAILB=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_MAILB=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_MAIN_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_MAIN_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_MAIN_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_MAIN_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_MAIN_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_MAIN_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\md5c.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_MD5C_=\
	".\global.h"\
	".\md5.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MultiPart.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_MULTI=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_MULTI=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_MULTI=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nPOP.rc

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Option.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_OPTIO=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_OPTIO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_OPTIO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_OPTIO=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_OPTIO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_OPTIO=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Pop3.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_POP3_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Profile.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_PROFI=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_PROFI=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_PROFI=\
	".\General.h"\
	".\Profile.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ras.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_RAS_C=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_RAS_C=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_RAS_C=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_RAS_C=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_RAS_C=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_RAS_C=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Smtp.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_SMTP_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_SMTP_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_SMTP_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_SMTP_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_SMTP_=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_SMTP_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\String.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_STRIN=\
	".\General.h"\
	".\global.h"\
	".\md5.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\View.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_VIEW_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_VIEW_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_VIEW_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_VIEW_=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_VIEW_=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_VIEW_=\
	".\General.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinSock.c

!IF  "$(CFG)" == "nPOP - Win32 Release"

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug"

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Release"

DEP_CPP_WINSO=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE x86em) Debug"

DEP_CPP_WINSO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Release"

DEP_CPP_WINSO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE MIPS) Debug"

DEP_CPP_WINSO=\
	".\General.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Release"

DEP_CPP_WINSO=\
	".\General.h"\
	".\stdafx.h"\
	".\strtbl.h"\
	

!ELSEIF  "$(CFG)" == "nPOP - Win32 (WCE SH) Debug"

DEP_CPP_WINSO=\
	".\General.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\General.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\jp.h
# End Source File
# Begin Source File

SOURCE=.\kr.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\Profile.h
# End Source File
# Begin Source File

SOURCE=.\strtbl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\ico00006.ico
# End Source File
# Begin Source File

SOURCE=.\ico00007.ico
# End Source File
# Begin Source File

SOURCE=.\ico00008.ico
# End Source File
# Begin Source File

SOURCE=.\ico00009.ico
# End Source File
# Begin Source File

SOURCE=.\ico00010.ico
# End Source File
# Begin Source File

SOURCE=.\ico00011.ico
# End Source File
# Begin Source File

SOURCE=.\ico00012.ico
# End Source File
# Begin Source File

SOURCE=.\icon.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon_che.ico
# End Source File
# Begin Source File

SOURCE=.\icon_cli.ico
# End Source File
# Begin Source File

SOURCE=.\icon_del.ico
# End Source File
# Begin Source File

SOURCE=.\icon_dow.ico
# End Source File
# Begin Source File

SOURCE=.\icon_new.ico
# End Source File
# Begin Source File

SOURCE=.\icon_non.ico
# End Source File
# Begin Source File

SOURCE=.\icon_rea.ico
# End Source File
# Begin Source File

SOURCE=.\icon_sen.ico
# End Source File
# Begin Source File

SOURCE=.\manifest.xml
# End Source File
# Begin Source File

SOURCE=.\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\toolbar_.bmp
# End Source File
# End Group
# End Target
# End Project
