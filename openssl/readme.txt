readme.txt for openssl (version 0.9.8k)

Please review LICENSE.txt

 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"

1) Download the latest OpenSSL source code from
   http://www.openssl.org/source

2) Uncompress and extract the files

3) Copy the "crypto" and "ssl" directories into this folder

4) Set up your compiler environment (eg, vcvars32.bat for MS VC++)

5) nmake -f ntlib.mak

   You may need to replace crypto\cversion.c with the version here;
   there may be other minor tweaks necessary.

6) The .lib files should now be in "out32"


For Windows CE, you will need to obtain wcecompat.lib; you should
be able to download it from http://www.essemer.com.au/windowsce
If you have already built the Win32 version of the OpenSSL libs,
please delete inc32\openssl\opensslconf.h

You may need to run another batch file to set up for the specific
processor.  Then "nmake -f celib.mak" and the .lib files should be
made in "out32_PROC" where PROC stands for the processor  (ARM,
MIPS, SH3, SH4, X86).