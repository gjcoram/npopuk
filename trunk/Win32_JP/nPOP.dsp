# Microsoft Developer Studio Project File - Name="nPOP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** �ҏW���Ȃ��ł������� **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nPOP - Win32 Debug
!MESSAGE ����͗L����Ҳ�̧�قł͂���܂���B ������ۼު�Ă�����ނ��邽�߂ɂ� NMAKE ���g�p���Ă��������B
!MESSAGE [Ҳ�̧�ق̴���߰�] ����ނ��g�p���Ď��s���Ă�������
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak".
!MESSAGE 
!MESSAGE NMAKE �̎��s���ɍ\�����w��ł��܂�
!MESSAGE ����� ײݏ��ϸۂ̐ݒ���`���܂��B��:
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak" CFG="nPOP - Win32 Debug"
!MESSAGE 
!MESSAGE �I���\������� Ӱ��:
!MESSAGE 
!MESSAGE "nPOP - Win32 Release" ("Win32 (x86) Application" �p)
!MESSAGE "nPOP - Win32 Debug" ("Win32 (x86) Application" �p)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gr /W3 /GX /Ox /Og /Os /Ob0 /Gf /Gy /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /machine:I386 /opt:nowin98
# SUBTRACT LINK32 /pdb:none

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D "UNICODE" /D "_UNICODE" /U "UNICODE" /U "_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "nPOP - Win32 Release"
# Name "nPOP - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Code.c
# End Source File
# Begin Source File

SOURCE=.\Edit.c
# End Source File
# Begin Source File

SOURCE=.\File.c
# End Source File
# Begin Source File

SOURCE=.\Font.c
# End Source File
# Begin Source File

SOURCE=.\Ini.c
# End Source File
# Begin Source File

SOURCE=.\Item.c
# End Source File
# Begin Source File

SOURCE=.\jp.c
# End Source File
# Begin Source File

SOURCE=.\kr.c
# End Source File
# Begin Source File

SOURCE=.\ListView.c
# End Source File
# Begin Source File

SOURCE=.\MailBox.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\md5c.c
# End Source File
# Begin Source File

SOURCE=.\MultiPart.c
# End Source File
# Begin Source File

SOURCE=.\nPOP.rc
# End Source File
# Begin Source File

SOURCE=.\Option.c
# End Source File
# Begin Source File

SOURCE=.\Pop3.c
# End Source File
# Begin Source File

SOURCE=.\Profile.c
# End Source File
# Begin Source File

SOURCE=.\Ras.c
# End Source File
# Begin Source File

SOURCE=.\Smtp.c
# End Source File
# Begin Source File

SOURCE=.\String.c
# End Source File
# Begin Source File

SOURCE=.\View.c
# End Source File
# Begin Source File

SOURCE=.\WinSock.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
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

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resrc1.h
# End Source File
# Begin Source File

SOURCE=.\Strtbl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bin1.bin
# End Source File
# Begin Source File

SOURCE=.\bitmap.bmp
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
