readme.txt for openssl (version 1.0.2j)

Please review LICENSE.txt

 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"

1) Download the latest OpenSSL source code from
   http://www.openssl.org/source

2) Uncompress and extract the files

3) Copy the "crypto" and "ssl" directories into this folder

4) Set up your compiler environment (eg, vcvars32.bat for MS VC++)

5) If you are changing versions, you may need to re-create ntlib.mak
   This requires perl.

   perl Configure no-rc5 no-idea no-ec no-asm VC-WIN32

6) nmake -f ntlib.mak

   You may need to replace crypto\cversion.c with the version here;
   there may be other minor tweaks necessary.  I found that they 
   can all be handled by changing things like 
   #include <openssl/asn1.h> to #include "asn1.h"

5) The .lib files should now be in "out32"


For Windows CE, you will need to obtain wcecompat.lib; you should
be able to download it from http://www.essemer.com.au/windowsce
If you have already built the Win32 version of the OpenSSL libs,
please delete inc32\openssl\opensslconf.h

On my compiler (MS Visual Studio 6.0), I had to change:
- comment out "#include <strings.h>" from crypto\o_str.c
- comment out an unused declaration of "len" in crypto\dso\dso_win32.c
- change _timeb to timeb in crypto\bio\bss_dgram.c and ssl\d1_lib.c


You may need to run another batch file to set up for the specific
processor.  Then "nmake -f cearmlib.mak" and the .lib files should be
made in "out32_ARM" (similarly for cesh3lib.mak, cesh4lib.mak, and
cempslib.mak).

These files now include support for ZLIB compression using code from
http://www.zlib.net/
