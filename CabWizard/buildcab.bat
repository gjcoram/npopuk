@echo off
REM
REM Batch file used to compile the CabWiz INF file
REM
REM
REM Must use the full path to the CabWiz.exe file
REM .INF file expected to be in the current directory
REM List all targets after the /cpu switch
REM

"C:\Program Files\Windows CE Tools\wce300\hpc2000\support\activesyncsdk\windows ce application installation\cabwiz\CabWiz.exe" nPOPuk.inf /err err.log /cpu HPC_SH3 HPC_MIPS HPCPro_ARM HPCPro_MIPS HPCPro_SH3 HPCPro_SH4 HPC2000_ARM HPC2000_MIPS HPC2000_X86
