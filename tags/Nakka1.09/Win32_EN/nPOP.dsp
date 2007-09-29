# Microsoft Developer Studio Project File - Name="nPOP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nPOP - Win32 Debug_Unicode
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "nPOP.mak" CFG="nPOP - Win32 Debug_Unicode"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "nPOP - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "nPOP - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE "nPOP - Win32 Release_Unicode" ("Win32 (x86) Application" 用)
!MESSAGE "nPOP - Win32 Debug_Unicode" ("Win32 (x86) Application" 用)
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
# ADD CPP /nologo /W3 /GX /Ox /Og /Os /Ob0 /Gf /Gy /I "." /I "..\EN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /machine:I386 /libpath:"G:\openssl\slib" /opt:nowin98
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
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\EN" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /YX /FD /GZ /c
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

!ELSEIF  "$(CFG)" == "nPOP - Win32 Release_Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nPOP___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "nPOP___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Ox /Og /Os /Ob0 /Gf /Gy /I "." /I "..\EN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /YX /FD /c
# ADD CPP /nologo /W3 /GX /Ox /Og /Os /Ob0 /Gf /Gy /I "." /I "..\EN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /machine:I386 /libpath:"G:\openssl\slib" /opt:nowin98
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /machine:I386 /libpath:"G:\openssl\slib" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "nPOP - Win32 Debug_Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nPOP___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "nPOP___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\EN" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D "UNICODE" /D "_UNICODE" /U "UNICODE" /U "_UNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\EN" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "WSAASYNC" /D _WIN32_IE=0x0400 /D "UNICODE" /D "_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib Winmm.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "nPOP - Win32 Release"
# Name "nPOP - Win32 Debug"
# Name "nPOP - Win32 Release_Unicode"
# Name "nPOP - Win32 Debug_Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\Charset.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Code.c
# End Source File
# Begin Source File

SOURCE=..\src\Edit.c
# End Source File
# Begin Source File

SOURCE=..\src\File.c
# End Source File
# Begin Source File

SOURCE=..\src\Font.c
# End Source File
# Begin Source File

SOURCE=..\src\Ini.c
# End Source File
# Begin Source File

SOURCE=..\src\Item.c
# End Source File
# Begin Source File

SOURCE=..\src\jp.c
# End Source File
# Begin Source File

SOURCE=..\src\ListView.c
# End Source File
# Begin Source File

SOURCE=..\src\MailBox.c
# End Source File
# Begin Source File

SOURCE=..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\src\md5c.c
# End Source File
# Begin Source File

SOURCE=..\src\Memory.c
# End Source File
# Begin Source File

SOURCE=..\src\mime.c
# End Source File
# Begin Source File

SOURCE=..\src\MultiPart.c
# End Source File
# Begin Source File

SOURCE=.\nPOP.rc
# End Source File
# Begin Source File

SOURCE=..\src\Option.c
# End Source File
# Begin Source File

SOURCE=..\src\Pop3.c
# End Source File
# Begin Source File

SOURCE=..\src\Profile.c
# End Source File
# Begin Source File

SOURCE=..\src\Ras.c
# End Source File
# Begin Source File

SOURCE=..\src\Smtp.c
# End Source File
# Begin Source File

SOURCE=..\src\String.c
# End Source File
# Begin Source File

SOURCE=..\src\util.c
# End Source File
# Begin Source File

SOURCE=..\src\View.c
# End Source File
# Begin Source File

SOURCE=..\src\WinSock.c
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

SOURCE=..\src\Profile.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resrc1.h
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

SOURCE=..\res\bitmap.bmp
# End Source File
# Begin Source File

SOURCE=..\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=..\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=..\res\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=..\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00005.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00006.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00007.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00008.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00009.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00010.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00011.ico
# End Source File
# Begin Source File

SOURCE=..\res\ico00012.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_che.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_cli.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_del.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_dow.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_new.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_non.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_rea.ico
# End Source File
# Begin Source File

SOURCE=..\res\icon_sen.ico
# End Source File
# Begin Source File

SOURCE=..\res\manifest.xml
# End Source File
# Begin Source File

SOURCE=..\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=..\res\toolbar_.bmp
# End Source File
# Begin Source File

SOURCE=..\res\toolbar_win32.bmp
# End Source File
# End Group
# End Target
# End Project
