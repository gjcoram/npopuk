call PATH.BAT

set TMP32DLL=tmp32dll
set OUT32DLL=out32dll
set LIB_DIR=%APP_DIR%\lib
set DLL_DIR=%APP_DIR%\lib
mkdir "%LIB_DIR%"
mkdir "%DLL_DIR%"

rem *** VC Setting
call "%ProgramFiles%\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
ECHO ON

rem *** Make OpenSSL < http://www.openssl.org/ >
del /Q %OPEN_SSL%\%TMP32DLL%\*.obj
cd %OPEN_SSL%
PATH C:\Perl\bin;%PATH%
perl Configure no-rc5 no-idea no-ec VC-WIN32
call ms\do_ms
ECHO ON
nmake -f ms\ntdll.mak

rem *** Copy Lib and DLL
copy "%OPEN_SSL%\%OUT32DLL%\libeay32.lib" "%LIB_DIR%\libeay32.lib"
copy "%OPEN_SSL%\%OUT32DLL%\ssleay32.lib" "%LIB_DIR%\ssleay32.lib"

copy "%OPEN_SSL%\%OUT32DLL%\libeay32.dll" "%DLL_DIR%\libeay32.dll"
copy "%OPEN_SSL%\%OUT32DLL%\ssleay32.dll" "%DLL_DIR%\ssleay32.dll"
