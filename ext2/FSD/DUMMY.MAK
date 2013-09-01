# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=dummy - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to dummy - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dummy - Win32 Release" && "$(CFG)" != "dummy - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "dummy.mak" CFG="dummy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dummy - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "dummy - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "dummy - Win32 Debug"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "dummy - Win32 Release"

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

ALL : "$(OUTDIR)\dummy.exe"

CLEAN : 
	-@erase ".\Release\dummy.exe"
	-@erase ".\Release\super.obj"
	-@erase ".\Release\blkdev.obj"
	-@erase ".\Release\fsdfile.obj"
	-@erase ".\Release\vxddebug.obj"
	-@erase ".\Release\util.obj"
	-@erase ".\Release\parttbl.obj"
	-@erase ".\Release\ext2fs.obj"
	-@erase ".\Release\fsdmount.obj"
	-@erase ".\Release\fsdvol.obj"
	-@erase ".\Release\cache.obj"
	-@erase ".\Release\inode.obj"
	-@erase ".\Release\partwalk.obj"
	-@erase ".\Release\file.obj"
	-@erase ".\Release\dir.obj"
	-@erase ".\Release\block.obj"
	-@erase ".\Release\vxd.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "e:\vtd95\include" /I ".." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "e:\vtd95\include" /I ".." /D "WIN32" /D\
 "NDEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/dummy.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dummy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:no /pdb:"$(OUTDIR)/dummy.pdb" /machine:I386\
 /out:"$(OUTDIR)/dummy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/super.obj" \
	"$(INTDIR)/blkdev.obj" \
	"$(INTDIR)/fsdfile.obj" \
	"$(INTDIR)/vxddebug.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/parttbl.obj" \
	"$(INTDIR)/ext2fs.obj" \
	"$(INTDIR)/fsdmount.obj" \
	"$(INTDIR)/fsdvol.obj" \
	"$(INTDIR)/cache.obj" \
	"$(INTDIR)/inode.obj" \
	"$(INTDIR)/partwalk.obj" \
	"$(INTDIR)/file.obj" \
	"$(INTDIR)/dir.obj" \
	"$(INTDIR)/block.obj" \
	"$(INTDIR)/vxd.obj"

"$(OUTDIR)\dummy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

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

ALL : "$(OUTDIR)\dummy.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\dummy.exe"
	-@erase ".\Debug\cache.obj"
	-@erase ".\Debug\blkdev.obj"
	-@erase ".\Debug\inode.obj"
	-@erase ".\Debug\file.obj"
	-@erase ".\Debug\ext2fs.obj"
	-@erase ".\Debug\block.obj"
	-@erase ".\Debug\fsdmount.obj"
	-@erase ".\Debug\dir.obj"
	-@erase ".\Debug\partwalk.obj"
	-@erase ".\Debug\super.obj"
	-@erase ".\Debug\fsdfile.obj"
	-@erase ".\Debug\util.obj"
	-@erase ".\Debug\vxd.obj"
	-@erase ".\Debug\fsdvol.obj"
	-@erase ".\Debug\vxddebug.obj"
	-@erase ".\Debug\parttbl.obj"
	-@erase ".\Debug\dummy.ilk"
	-@erase ".\Debug\dummy.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "e:\vtd95\include" /I ".." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "e:\vtd95\include" /I ".." /D\
 "WIN32" /D "_DEBUG" /D "_CONSOLE" /Fp"$(INTDIR)/dummy.pch" /YX /Fo"$(INTDIR)/"\
 /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/dummy.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /subsystem:console /incremental:yes /pdb:"$(OUTDIR)/dummy.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/dummy.exe" 
LINK32_OBJS= \
	"$(INTDIR)/cache.obj" \
	"$(INTDIR)/blkdev.obj" \
	"$(INTDIR)/inode.obj" \
	"$(INTDIR)/file.obj" \
	"$(INTDIR)/ext2fs.obj" \
	"$(INTDIR)/block.obj" \
	"$(INTDIR)/fsdmount.obj" \
	"$(INTDIR)/dir.obj" \
	"$(INTDIR)/partwalk.obj" \
	"$(INTDIR)/super.obj" \
	"$(INTDIR)/fsdfile.obj" \
	"$(INTDIR)/util.obj" \
	"$(INTDIR)/vxd.obj" \
	"$(INTDIR)/fsdvol.obj" \
	"$(INTDIR)/vxddebug.obj" \
	"$(INTDIR)/parttbl.obj"

"$(OUTDIR)\dummy.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "dummy - Win32 Release"
# Name "dummy - Win32 Debug"

!IF  "$(CFG)" == "dummy - Win32 Release"

!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ext2fs.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_EXT2F=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\ext2fs.h"\
	".\inode.h"\
	".\super.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	".\..\shared\blkdev.h"\
	

"$(INTDIR)\ext2fs.obj" : $(SOURCE) $(DEP_CPP_EXT2F) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_EXT2F=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\ext2fs.h"\
	".\inode.h"\
	".\super.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\ext2fs.obj" : $(SOURCE) $(DEP_CPP_EXT2F) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fsdvol.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_FSDVO=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\fsdfile.h"\
	".\fsdvol.h"\
	".\ext2fs.h"\
	".\super.h"\
	".\inode.h"\
	".\unixstat.h"\
	".\util.h"\
	".\dir.h"\
	".\file.h"\
	".\Crs.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	".\..\shared\blkdev.h"\
	

"$(INTDIR)\fsdvol.obj" : $(SOURCE) $(DEP_CPP_FSDVO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_FSDVO=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\fsdfile.h"\
	".\fsdvol.h"\
	".\ext2fs.h"\
	".\super.h"\
	".\inode.h"\
	".\unixstat.h"\
	".\util.h"\
	".\dir.h"\
	".\file.h"\
	".\Crs.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\fsdvol.obj" : $(SOURCE) $(DEP_CPP_FSDVO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cache.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_CACHE=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\cache.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\cache.obj" : $(SOURCE) $(DEP_CPP_CACHE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_CACHE=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\cache.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\cache.obj" : $(SOURCE) $(DEP_CPP_CACHE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\makefile

!IF  "$(CFG)" == "dummy - Win32 Release"

!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\util.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_UTIL_=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\util.h"\
	".\dir.h"\
	".\unixstat.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\ext2fs.h"\
	".\inode.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_UTIL_=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\util.h"\
	".\dir.h"\
	".\unixstat.h"\
	".\..\shared\debug.h"\
	".\ext2fs.h"\
	".\inode.h"\
	

"$(INTDIR)\util.obj" : $(SOURCE) $(DEP_CPP_UTIL_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\inode.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_INODE=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\inode.h"\
	".\super.h"\
	".\ext2fs.h"\
	".\unixstat.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	".\..\shared\blkdev.h"\
	

"$(INTDIR)\inode.obj" : $(SOURCE) $(DEP_CPP_INODE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_INODE=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\inode.h"\
	".\super.h"\
	".\ext2fs.h"\
	".\unixstat.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\inode.obj" : $(SOURCE) $(DEP_CPP_INODE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\super.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_SUPER=\
	".\vxd.h"\
	".\..\shared\parttype.h"\
	".\..\shared\vxddebug.h"\
	".\super.h"\
	".\inode.h"\
	".\parttbl.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	".\ext2fs.h"\
	".\..\shared\blkdev.h"\
	

"$(INTDIR)\super.obj" : $(SOURCE) $(DEP_CPP_SUPER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_SUPER=\
	".\vxd.h"\
	".\..\shared\parttype.h"\
	".\..\shared\vxddebug.h"\
	".\super.h"\
	".\inode.h"\
	".\parttbl.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\super.obj" : $(SOURCE) $(DEP_CPP_SUPER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\block.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_BLOCK=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\cache.h"\
	".\block.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\block.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_BLOCK=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\cache.h"\
	".\block.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\block.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\notes.txt

!IF  "$(CFG)" == "dummy - Win32 Release"

!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dir.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_DIR_C=\
	".\vxd.h"\
	".\dir.h"\
	".\cache.h"\
	".\super.h"\
	".\inode.h"\
	".\block.h"\
	".\util.h"\
	".\unixstat.h"\
	".\..\shared\vxddebug.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\ext2fs.h"\
	".\..\shared\blkdev.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\dir.obj" : $(SOURCE) $(DEP_CPP_DIR_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_DIR_C=\
	".\vxd.h"\
	".\dir.h"\
	".\cache.h"\
	".\super.h"\
	".\inode.h"\
	".\block.h"\
	".\util.h"\
	".\unixstat.h"\
	".\..\shared\vxddebug.h"\
	".\ext2fs.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\dir.obj" : $(SOURCE) $(DEP_CPP_DIR_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\file.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_FILE_=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\file.h"\
	".\ext2fs.h"\
	".\super.h"\
	".\inode.h"\
	".\unixstat.h"\
	".\util.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	".\..\shared\blkdev.h"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_FILE_=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\file.h"\
	".\ext2fs.h"\
	".\super.h"\
	".\inode.h"\
	".\unixstat.h"\
	".\util.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fsdfile.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_FSDFI=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\fsdfile.h"\
	".\file.h"\
	".\ext2fs.h"\
	".\dir.h"\
	".\inode.h"\
	".\util.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\fsdfile.obj" : $(SOURCE) $(DEP_CPP_FSDFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_FSDFI=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\fsdfile.h"\
	".\file.h"\
	".\ext2fs.h"\
	".\dir.h"\
	".\inode.h"\
	".\util.h"\
	".\..\shared\debug.h"\
	

"$(INTDIR)\fsdfile.obj" : $(SOURCE) $(DEP_CPP_FSDFI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\parttbl.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_PARTT=\
	".\vxd.h"\
	".\..\shared\partwalk.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\..\shared\fsdioctl.h"\
	".\parttbl.h"\
	"e:\vtd95\include\dcb.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\..\shared\parttype.h"\
	

"$(INTDIR)\parttbl.obj" : $(SOURCE) $(DEP_CPP_PARTT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_PARTT=\
	".\vxd.h"\
	".\..\shared\partwalk.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\..\shared\fsdioctl.h"\
	".\parttbl.h"\
	"e:\vtd95\include\dcb.h"\
	".\..\shared\debug.h"\
	".\..\shared\parttype.h"\
	

"$(INTDIR)\parttbl.obj" : $(SOURCE) $(DEP_CPP_PARTT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vxd.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_VXD_C=\
	".\vxd.h"\
	".\..\shared\e2version.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\..\shared\fsdioctl.h"\
	".\ext2fs.h"\
	".\fsdmount.h"\
	".\fsdvol.h"\
	".\super.h"\
	".\inode.h"\
	".\block.h"\
	".\dir.h"\
	".\parttbl.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\..\shared\parttype.h"\
	

"$(INTDIR)\vxd.obj" : $(SOURCE) $(DEP_CPP_VXD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_VXD_C=\
	".\vxd.h"\
	".\..\shared\e2version.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\..\shared\fsdioctl.h"\
	".\ext2fs.h"\
	".\fsdmount.h"\
	".\fsdvol.h"\
	".\super.h"\
	".\inode.h"\
	".\block.h"\
	".\dir.h"\
	".\parttbl.h"\
	".\..\shared\debug.h"\
	".\..\shared\parttype.h"\
	

"$(INTDIR)\vxd.obj" : $(SOURCE) $(DEP_CPP_VXD_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fsdmount.c

!IF  "$(CFG)" == "dummy - Win32 Release"

DEP_CPP_FSDMO=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\fsdvol.h"\
	".\fsdmount.h"\
	".\ext2fs.h"\
	".\super.h"\
	"e:\vtd95\include\vtoolsc.h"\
	"E:\Vtd95\Include\Vsvctab.h"\
	"E:\Vtd95\Include\Vtoolsd.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\fsdmount.obj" : $(SOURCE) $(DEP_CPP_FSDMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

DEP_CPP_FSDMO=\
	".\vxd.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	".\fsdvol.h"\
	".\fsdmount.h"\
	".\ext2fs.h"\
	".\super.h"\
	".\..\shared\debug.h"\
	".\block.h"\
	

"$(INTDIR)\fsdmount.obj" : $(SOURCE) $(DEP_CPP_FSDMO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\fsdext2\shared\vxddebug.c
DEP_CPP_VXDDE=\
	".\..\shared\vxddebug.h"\
	".\..\shared\debug.h"\
	
NODEP_CPP_VXDDE=\
	".\..\shared\vxd.h"\
	

"$(INTDIR)\vxddebug.obj" : $(SOURCE) $(DEP_CPP_VXDDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\fsdext2\shared\partwalk.c
DEP_CPP_PARTW=\
	".\..\shared\parttype.h"\
	".\..\shared\partwalk.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	"e:\vtd95\include\dcb.h"\
	".\..\shared\debug.h"\
	
NODEP_CPP_PARTW=\
	".\..\shared\vxd.h"\
	

"$(INTDIR)\partwalk.obj" : $(SOURCE) $(DEP_CPP_PARTW) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\fsdext2\shared\blkdev.c
DEP_CPP_BLKDE=\
	".\..\shared\iosmgr.h"\
	"e:\vtd95\include\iop.h"\
	"e:\vtd95\include\ddb.h"\
	".\..\shared\vxddebug.h"\
	".\..\shared\blkdev.h"\
	"e:\vtd95\include\dcb.h"\
	".\..\shared\debug.h"\
	
NODEP_CPP_BLKDE=\
	".\..\shared\vxd.h"\
	

"$(INTDIR)\blkdev.obj" : $(SOURCE) $(DEP_CPP_BLKDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\fsdext2\release\history.txt

!IF  "$(CFG)" == "dummy - Win32 Release"

!ELSEIF  "$(CFG)" == "dummy - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
