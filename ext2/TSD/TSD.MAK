# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=tsd - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to tsd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tsd - Win32 Release" && "$(CFG)" != "tsd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "tsd.mak" CFG="tsd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tsd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tsd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "tsd - Win32 Debug"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "tsd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\tsd.exe"

CLEAN : 
	-@erase ".\Release\tsd.exe"
	-@erase ".\Release\vxd.obj"
	-@erase ".\Release\tsd.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "e:\vtd95\include" /I ".." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "e:\vtd95\include" /I ".." /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/tsd.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/tsd.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/tsd.pdb" /machine:I386\
 /out:"$(OUTDIR)/tsd.exe" 
LINK32_OBJS= \
	"$(INTDIR)/vxd.obj" \
	"$(INTDIR)/tsd.obj"

"$(OUTDIR)\tsd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tsd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\tsd.exe"

CLEAN : 
	-@erase ".\Debug\tsd.exe"
	-@erase ".\Debug\vxd.obj"
	-@erase ".\Debug\tsd.obj"
	-@erase ".\Debug\tsd.ilk"
	-@erase ".\Debug\tsd.pdb"
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "e:\vtd95\include" /I ".." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "e:\vtd95\include" /I ".." /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/tsd.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/tsd.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/tsd.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/tsd.exe" 
LINK32_OBJS= \
	"$(INTDIR)/vxd.obj" \
	"$(INTDIR)/tsd.obj"

"$(OUTDIR)\tsd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "tsd - Win32 Release"
# Name "tsd - Win32 Debug"

!IF  "$(CFG)" == "tsd - Win32 Release"

!ELSEIF  "$(CFG)" == "tsd - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\vxd.c

!IF  "$(CFG)" == "tsd - Win32 Release"


"$(INTDIR)\vxd.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tsd - Win32 Debug"

DEP_CPP_VXD_C=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\e2version.h"\
	"e:\vtd95\include\vtoolsc.h"\
	".\..\shared\iosmgr.h"\
	"e:\vtd95\include\vsvctab.h"\
	"e:\vtd95\include\vtoolsd.h"\
	"e:\vtd95\include\bcdefseg.h"\
	"e:\vtd95\include\ddkinc.h"\
	"e:\vtd95\include\vthunks.h"\
	"e:\vtd95\include\vxdsvc.h"\
	"e:\vtd95\include\crtl.h"\
	"e:\vtd95\include\bcintrin.h"\
	"e:\vtd95\include\msintrin.h"\
	"e:\vtd95\include\ndis.h"\
	"e:\vtd95\include\scsi.h"\
	"e:\vtd95\include\scsiport.h"\
	"e:\vtd95\include\vmm.h"\
	"e:\vtd95\include\blockdev.h"\
	"e:\vtd95\include\debug.h"\
	"e:\vtd95\include\pagefile.h"\
	"e:\vtd95\include\shell.h"\
	"e:\vtd95\include\v86mmgr.h"\
	"e:\vtd95\include\vcd.h"\
	"e:\vtd95\include\power.h"\
	"e:\vtd95\include\vddgrb.h"\
	"e:\vtd95\include\vdmad.h"\
	"e:\vtd95\include\vkd.h"\
	"e:\vtd95\include\vmd.h"\
	"e:\vtd95\include\vmpoll.h"\
	"e:\vtd95\include\vpicd.h"\
	"e:\vtd95\include\vtd.h"\
	"e:\vtd95\include\vsd.h"\
	"e:\vtd95\include\vcomm.h"\
	"e:\vtd95\include\vxdldr.h"\
	"e:\vtd95\include\vpowerd.h"\
	"e:\vtd95\include\configmg.h"\
	"e:\vtd95\include\ifs.h"\
	"e:\vtd95\include\pccard.h"\
	"e:\vtd95\include\drp.h"\
	"e:\vtd95\include\irs.h"\
	"e:\vtd95\include\ior.h"\
	"e:\vtd95\include\ilb.h"\
	"e:\vtd95\include\isp.h"\
	"e:\vtd95\include\aep.h"\
	"e:\vtd95\include\iop.h"\
	"e:\vtd95\include\dcb.h"\
	"e:\vtd95\include\vwin32.h"\
	"e:\vtd95\include\vcache.h"\
	"e:\vtd95\include\perf.h"\
	"e:\vtd95\include\vrp.h"\
	"e:\vtd95\include\sgd.h"\
	"e:\vtd95\include\vfbackup.h"\
	"e:\vtd95\include\VKDpaste.h"\
	"e:\vtd95\include\vmmreg.h"\
	"e:\vtd95\include\netvxd.h"\
	"e:\vtd95\include\srb.h"\
	"e:\vtd95\include\miniport.h"\
	

"$(INTDIR)\vxd.obj" : $(SOURCE) $(DEP_CPP_VXD_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tsd.c

!IF  "$(CFG)" == "tsd - Win32 Release"

DEP_CPP_TSD_C=\
	".\vxd.h"\
	"e:\vtd95\include\iop.h"\
	"e:\vtd95\include\ddb.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\partwalk.h"\
	".\..\shared\parttype.h"\
	"e:\vtd95\include\vtoolsc.h"\
	".\..\shared\iosmgr.h"\
	"e:\vtd95\include\vsvctab.h"\
	"e:\vtd95\include\vtoolsd.h"\
	"e:\vtd95\include\dcb.h"\
	

"$(INTDIR)\tsd.obj" : $(SOURCE) $(DEP_CPP_TSD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "tsd - Win32 Debug"

DEP_CPP_TSD_C=\
	".\vxd.h"\
	"e:\vtd95\include\iop.h"\
	"e:\vtd95\include\ddb.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\partwalk.h"\
	".\..\shared\parttype.h"\
	"e:\vtd95\include\vtoolsc.h"\
	".\..\shared\iosmgr.h"\
	"e:\vtd95\include\vsvctab.h"\
	"e:\vtd95\include\vtoolsd.h"\
	"e:\vtd95\include\bcdefseg.h"\
	"e:\vtd95\include\ddkinc.h"\
	"e:\vtd95\include\vthunks.h"\
	"e:\vtd95\include\vxdsvc.h"\
	"e:\vtd95\include\crtl.h"\
	"e:\vtd95\include\bcintrin.h"\
	"e:\vtd95\include\msintrin.h"\
	"e:\vtd95\include\ndis.h"\
	"e:\vtd95\include\scsi.h"\
	"e:\vtd95\include\scsiport.h"\
	"e:\vtd95\include\vmm.h"\
	"e:\vtd95\include\blockdev.h"\
	"e:\vtd95\include\debug.h"\
	"e:\vtd95\include\pagefile.h"\
	"e:\vtd95\include\shell.h"\
	"e:\vtd95\include\v86mmgr.h"\
	"e:\vtd95\include\vcd.h"\
	"e:\vtd95\include\power.h"\
	"e:\vtd95\include\vddgrb.h"\
	"e:\vtd95\include\vdmad.h"\
	"e:\vtd95\include\vkd.h"\
	"e:\vtd95\include\vmd.h"\
	"e:\vtd95\include\vmpoll.h"\
	"e:\vtd95\include\vpicd.h"\
	"e:\vtd95\include\vtd.h"\
	"e:\vtd95\include\vsd.h"\
	"e:\vtd95\include\vcomm.h"\
	"e:\vtd95\include\vxdldr.h"\
	"e:\vtd95\include\vpowerd.h"\
	"e:\vtd95\include\configmg.h"\
	"e:\vtd95\include\ifs.h"\
	"e:\vtd95\include\pccard.h"\
	"e:\vtd95\include\drp.h"\
	"e:\vtd95\include\irs.h"\
	"e:\vtd95\include\ior.h"\
	"e:\vtd95\include\ilb.h"\
	"e:\vtd95\include\isp.h"\
	"e:\vtd95\include\aep.h"\
	"e:\vtd95\include\dcb.h"\
	"e:\vtd95\include\vwin32.h"\
	"e:\vtd95\include\vcache.h"\
	"e:\vtd95\include\perf.h"\
	"e:\vtd95\include\vrp.h"\
	"e:\vtd95\include\sgd.h"\
	"e:\vtd95\include\vfbackup.h"\
	"e:\vtd95\include\VKDpaste.h"\
	"e:\vtd95\include\vmmreg.h"\
	"e:\vtd95\include\netvxd.h"\
	"e:\vtd95\include\srb.h"\
	"e:\vtd95\include\miniport.h"\
	

"$(INTDIR)\tsd.obj" : $(SOURCE) $(DEP_CPP_TSD_C) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
