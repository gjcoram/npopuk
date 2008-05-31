rem PATH
set OPEN_SSL=G:\openssl\openssl-0.9.8a
set APP_DIR=G:\VC Source\nPOP\npopssl\openssl

rem Windows CE
set WCECOMPAT=G:\openssl\wcecompat
if "%OSVERSION%"=="" set OSVERSION=WCE300
if "%PLATFORM%"=="" set PLATFORM=ms pocket pc
if "%WCEROOT%"=="" set WCEROOT=%ProgramFiles%\Microsoft eMbedded Tools
if "%SDKROOT%"=="" set SDKROOT=C:\Windows CE Tools
