@echo off
REM
REM Sample batch file used to compile the CabWiz INF file
REM
REM
REM Replace <CabWiz-Path> with the full path to the CabWiz.exe file
REM Replace <INF-Path> with the full path to the Sample.INF file
REM

"C:\Program Files\Windows CE Tools\wce300\hpc2000\support\activesyncsdk\windows ce application installation\cabwiz\CabWiz.exe" nPOPuk.inf /err err.log /cpu HPC_SH3 HPC_MIPS HPCPro_ARM HPCPro_MIPS HPCPro_SH3 HPCPro_SH4
