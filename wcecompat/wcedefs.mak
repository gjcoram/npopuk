# Set WCEVERSION and WCELDVERSION
WCEVERSION=211
WCELDVERSION=2.11
WCEPLATFORM=MS_HPC_PRO
#
# ARM ---
!IF "$(TARGETCPU)"=="ARM"
WCETARGETCPU=ARM
WCETARGETCPUDEFS=-DARM -D_ARM_ -D_M_ARM -D$(TARGETCPU) -D_$(TARGETCPU)_
WCECC=clarm
!ENDIF
# MIPS ---
!IF "$(TARGETCPU)"=="MIPS"
WCETARGETCPU=MIPS
WCETARGETCPUDEFS=-DMIPS -D_MIPS_ -D_M_MIPS -D$(TARGETCPU) -D_$(TARGETCPU)_
WCECC=clmips
!ENDIF
# SH3 ---
!IF "$(TARGETCPU)"=="SH3"
WCETARGETCPU=SH3
WCETARGETCPUDEFS=-DSH3 -D_SH3_ -DSHx -D$(TARGETCPU) -D_$(TARGETCPU)_
WCECC=shcl
!ENDIF
# SH4 ---
!IF "$(TARGETCPU)"=="SH4"
WCETARGETCPU=SH4
WCETARGETCPUDEFS=/Qsh4 -DSH4 -D_SH4_ -DSHx -D$(TARGETCPU) -D_$(TARGETCPU)_
WCECC=shcl
!ENDIF
# X86 ---
!IF "$(TARGETCPU)"=="X86"
WCEVERSION=300
WCELDVERSION=3.0
WCETARGETCPU=X86
WCETARGETCPUDEFS=-DX86 -D_X86_ -D$(TARGETCPU) -D_$(TARGETCPU)_
WCECC=cl
!ENDIF
#
WCETARGETDEFS=$(WCETARGETDEFS) $(WCETARGETCPUDEFS) -DUNDER_CE=$(WCEVERSION) -D_WIN32_CE=$(WCEVERSION)

# Set CPU type for link
WCELDMACHINE=$(WCETARGETCPU)