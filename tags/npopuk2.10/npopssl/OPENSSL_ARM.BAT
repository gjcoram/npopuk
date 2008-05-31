call PATH.BAT

set TMP32DLL=tmp32dll_ARM
set OUT32DLL=out32dll_ARM
set LIB_DIR=%APP_DIR%\lib_arm
set DLL_DIR=%APP_DIR%\lib_arm
mkdir "%LIB_DIR%"
mkdir "%DLL_DIR%"

rem *** Windows CE Setting
set PATH=%WCEROOT%\COMMON\EVC\bin;%WCEROOT%\EVC\%OSVERSION%\bin;%path%
set INCLUDE=%SDKROOT%\%OSVERSION%\%PLATFORM%\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\include;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\include;
set LIB=%SDKROOT%\%OSVERSION%\%PLATFORM%\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\MFC\lib\arm;%SDKROOT%\%OSVERSION%\%PLATFORM%\ATL\lib\arm;

set CC=clarm.exe

set TARGETCPU=ARM
set CFG=none

rem *** Make wcecompat < http://www.essemer.com.au/windowsce/ >
del /Q %WCECOMPAT%\lib\*.lib
del /Q %WCECOMPAT%\obj\*.obj
cd %WCECOMPAT%
nmake
copy "%WCECOMPAT%\lib\wcecompat.lib" "%LIB_DIR%\wcecompat.lib"
copy "%WCECOMPAT%\lib\wcecompatex.lib" "%LIB_DIR%\wcecompatex.lib"

rem *** Make OpenSSL < http://www.openssl.org/ >
del /Q %OPEN_SSL%\%TMP32DLL%\*.obj
cd %OPEN_SSL%
PATH C:\Perl\bin;%PATH%
perl Configure no-rc5 no-idea no-ec VC-CE
call ms\do_ms
ECHO ON
nmake -f ms\cedll.mak

rem *** Copy Lib and DLL
copy "%OPEN_SSL%\%OUT32DLL%\libeay32.lib" "%LIB_DIR%\libeay32.lib"
copy "%OPEN_SSL%\%OUT32DLL%\ssleay32.lib" "%LIB_DIR%\ssleay32.lib"

copy "%OPEN_SSL%\%OUT32DLL%\libeay32.dll" "%DLL_DIR%\libeay32.dll"
copy "%OPEN_SSL%\%OUT32DLL%\ssleay32.dll" "%DLL_DIR%\ssleay32.dll"
