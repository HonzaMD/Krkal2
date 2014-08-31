# Microsoft Developer Studio Generated NMAKE File, Based on renelem.dsp
!IF "$(CFG)" == ""
CFG=renelem - Win32 Release
!MESSAGE No configuration specified. Defaulting to renelem - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "renelem - Win32 Release" && "$(CFG)" != "renelem - Win32 Debug" && "$(CFG)" != "renelem - Win32 Hybrid"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "renelem.mak" CFG="renelem - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "renelem - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "renelem - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "renelem - Win32 Hybrid" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "renelem - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\..\maxsdk\plugin\renelem.dln"


CLEAN :
	-@erase "$(INTDIR)\renelem.pch"
	-@erase "$(INTDIR)\renElem.res"
	-@erase "$(INTDIR)\renElemMain.obj"
	-@erase "$(INTDIR)\renElemPch.obj"
	-@erase "$(INTDIR)\sampRenElem.obj"
	-@erase "$(INTDIR)\stdRenElem.obj"
	-@erase "$(OUTDIR)\renelem.exp"
	-@erase "$(OUTDIR)\renelem.lib"
	-@erase "..\..\..\..\maxsdk\plugin\renelem.dln"
	-@erase ".\renelem.idb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

renelem=midl.exe
CPP_PROJ=/nologo /G6 /MD /W3 /O2 /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yu"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\renElem.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\renelem.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x084E0000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\renelem.pdb" /machine:I386 /def:".\renElem.def" /out:"..\..\..\..\maxsdk\plugin\renelem.dln" /implib:"$(OUTDIR)\renelem.lib" /release 
DEF_FILE= \
	".\renElem.def"
LINK32_OBJS= \
	"$(INTDIR)\renElemMain.obj" \
	"$(INTDIR)\renElemPch.obj" \
	"$(INTDIR)\sampRenElem.obj" \
	"$(INTDIR)\stdRenElem.obj" \
	"$(INTDIR)\renElem.res" \
	"..\..\..\lib\core.lib" \
	"..\..\..\lib\maxutil.lib" \
	"..\..\..\lib\Paramblk2.lib"

"..\..\..\..\maxsdk\plugin\renelem.dln" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "renelem - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\..\..\maxsdk\plugin\renelem.dln"


CLEAN :
	-@erase "$(INTDIR)\renelem.pch"
	-@erase "$(INTDIR)\renElem.res"
	-@erase "$(INTDIR)\renElemMain.obj"
	-@erase "$(INTDIR)\renElemPch.obj"
	-@erase "$(INTDIR)\sampRenElem.obj"
	-@erase "$(INTDIR)\stdRenElem.obj"
	-@erase "$(OUTDIR)\renelem.exp"
	-@erase "$(OUTDIR)\renelem.lib"
	-@erase "$(OUTDIR)\renelem.pdb"
	-@erase "..\..\..\..\maxsdk\plugin\renelem.dln"
	-@erase "..\..\..\..\maxsdk\plugin\renelem.ilk"
	-@erase ".\renelem.idb"
	-@erase ".\renelem.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

renelem=midl.exe
CPP_PROJ=/nologo /G6 /MDd /W3 /Gm /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yu"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\renElem.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\renelem.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x084E0000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\renelem.pdb" /debug /machine:I386 /def:".\renElem.def" /out:"..\..\..\..\maxsdk\plugin\renelem.dln" /implib:"$(OUTDIR)\renelem.lib" 
DEF_FILE= \
	".\renElem.def"
LINK32_OBJS= \
	"$(INTDIR)\renElemMain.obj" \
	"$(INTDIR)\renElemPch.obj" \
	"$(INTDIR)\sampRenElem.obj" \
	"$(INTDIR)\stdRenElem.obj" \
	"$(INTDIR)\renElem.res" \
	"..\..\..\lib\core.lib" \
	"..\..\..\lib\maxutil.lib" \
	"..\..\..\lib\Paramblk2.lib"

"..\..\..\..\maxsdk\plugin\renelem.dln" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "renelem - Win32 Hybrid"

OUTDIR=.\Hybrid
INTDIR=.\Hybrid

ALL : "..\..\..\..\maxsdk\plugin\renelem.dln"


CLEAN :
	-@erase "$(INTDIR)\renelem.pch"
	-@erase "$(INTDIR)\renElem.res"
	-@erase "$(INTDIR)\renElemMain.obj"
	-@erase "$(INTDIR)\renElemPch.obj"
	-@erase "$(INTDIR)\sampRenElem.obj"
	-@erase "$(INTDIR)\stdRenElem.obj"
	-@erase "$(OUTDIR)\renelem.exp"
	-@erase "$(OUTDIR)\renelem.lib"
	-@erase "$(OUTDIR)\renelem.pdb"
	-@erase "..\..\..\..\maxsdk\plugin\renelem.dln"
	-@erase "..\..\..\..\maxsdk\plugin\renelem.ilk"
	-@erase ".\renelem.idb"
	-@erase ".\renelem.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

renelem=midl.exe
CPP_PROJ=/nologo /G6 /MD /W3 /Gm /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yu"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\renElem.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\renelem.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x084E0000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\renelem.pdb" /debug /machine:I386 /def:".\renElem.def" /out:"..\..\..\..\maxsdk\plugin\renelem.dln" /implib:"$(OUTDIR)\renelem.lib" 
DEF_FILE= \
	".\renElem.def"
LINK32_OBJS= \
	"$(INTDIR)\renElemMain.obj" \
	"$(INTDIR)\renElemPch.obj" \
	"$(INTDIR)\sampRenElem.obj" \
	"$(INTDIR)\stdRenElem.obj" \
	"$(INTDIR)\renElem.res" \
	"..\..\..\lib\core.lib" \
	"..\..\..\lib\maxutil.lib" \
	"..\..\..\lib\Paramblk2.lib"

"..\..\..\..\maxsdk\plugin\renelem.dln" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("renelem.dep")
!INCLUDE "renelem.dep"
!ELSE 
!MESSAGE Warning: cannot find "renelem.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "renelem - Win32 Release" || "$(CFG)" == "renelem - Win32 Debug" || "$(CFG)" == "renelem - Win32 Hybrid"
SOURCE=.\renElemMain.cpp

"$(INTDIR)\renElemMain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\renelem.pch"


SOURCE=.\renElemPch.cpp

!IF  "$(CFG)" == "renelem - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MD /W3 /O2 /I "..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yc"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 

"$(INTDIR)\renElemPch.obj"	"$(INTDIR)\renelem.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "renelem - Win32 Debug"

CPP_SWITCHES=/nologo /G6 /MDd /W3 /Gm /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yc"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 

"$(INTDIR)\renElemPch.obj"	"$(INTDIR)\renelem.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "renelem - Win32 Hybrid"

CPP_SWITCHES=/nologo /G6 /MD /W3 /Gm /ZI /Od /I "..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\renelem.pch" /Yc"renelemPch.h" /Fo"$(INTDIR)\\" /Fd"renelem.pdb" /FD /c 

"$(INTDIR)\renElemPch.obj"	"$(INTDIR)\renelem.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\sampRenElem.cpp

"$(INTDIR)\sampRenElem.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\renelem.pch"


SOURCE=.\stdRenElem.cpp

"$(INTDIR)\stdRenElem.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\renelem.pch"


SOURCE=.\renElem.rc

"$(INTDIR)\renElem.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

