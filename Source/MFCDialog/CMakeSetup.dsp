# Microsoft Developer Studio Project File - Name="CMakeSetup" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CMakeSetup - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CMakeSetup.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CMakeSetup.mak" CFG="CMakeSetup - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CMakeSetup - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CMakeSetup - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CMakeSetup - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "CMakeSetup - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CMakeSetup - Win32 Release"
# Name "CMakeSetup - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\cmAbstractFilesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmAddTargetRule.cxx
# End Source File
# Begin Source File

SOURCE=.\CMakeSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\CMakeSetup.rc
# End Source File
# Begin Source File

SOURCE=..\CMakeSetupCMD.cxx
# End Source File
# Begin Source File

SOURCE=.\CMakeSetupDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\cmAuxSourceDirectoryRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmClassFile.cxx
# End Source File
# Begin Source File

SOURCE=..\cmDirectory.cxx
# End Source File
# Begin Source File

SOURCE=..\cmDSPMakefile.cxx
# End Source File
# Begin Source File

SOURCE=..\cmDSWMakefile.cxx
# End Source File
# Begin Source File

SOURCE=..\cmExecutablesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindIncludeRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindLibraryRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindProgramRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmIncludeDirectoryRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLibraryRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLinkDirectoriesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLinkLibrariesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmMakeDepend.cxx
# End Source File
# Begin Source File

SOURCE=..\cmMakefile.cxx
# End Source File
# Begin Source File

SOURCE=..\cmMakefileGenerator.cxx
# End Source File
# Begin Source File

SOURCE=..\cmMSProjectGenerator.cxx
# End Source File
# Begin Source File

SOURCE=..\cmProjectRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmRegularExpression.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRequireRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSubdirRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSystemTools.cxx
# End Source File
# Begin Source File

SOURCE=..\cmTestsRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmUnixDefinesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmUnixLibrariesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmWin32DefinesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmWin32LibrariesRule.cxx
# End Source File
# Begin Source File

SOURCE=..\cmWindowsConfigure.cxx
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\cmAbstractFilesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmAddTargetRule.h
# End Source File
# Begin Source File

SOURCE=..\CMakeSetup.h
# End Source File
# Begin Source File

SOURCE=.\CMakeSetup.h
# End Source File
# Begin Source File

SOURCE=.\CMakeSetupDialog.h
# End Source File
# Begin Source File

SOURCE=..\cmAuxSourceDirectoryRule.h
# End Source File
# Begin Source File

SOURCE=..\cmClassFile.h
# End Source File
# Begin Source File

SOURCE=..\cmDirectory.h
# End Source File
# Begin Source File

SOURCE=..\cmDSPBuilder.h
# End Source File
# Begin Source File

SOURCE=..\cmDSPMakefile.h
# End Source File
# Begin Source File

SOURCE=..\cmDSWBuilder.h
# End Source File
# Begin Source File

SOURCE=..\cmDSWMakefile.h
# End Source File
# Begin Source File

SOURCE=..\cmExecutablesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmFindIncludeRule.h
# End Source File
# Begin Source File

SOURCE=..\cmFindLibraryRule.h
# End Source File
# Begin Source File

SOURCE=..\cmFindProgramRule.h
# End Source File
# Begin Source File

SOURCE=..\cmIncludeDirectoryRule.h
# End Source File
# Begin Source File

SOURCE=..\cmLibraryRule.h
# End Source File
# Begin Source File

SOURCE=..\cmLinkDirectoriesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmLinkLibrariesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmMakeDepend.h
# End Source File
# Begin Source File

SOURCE=..\cmMakefile.h
# End Source File
# Begin Source File

SOURCE=..\cmMakefile2.h
# End Source File
# Begin Source File

SOURCE=..\cmMakefileGenerator.h
# End Source File
# Begin Source File

SOURCE=..\cmMSProjectGenerator.h
# End Source File
# Begin Source File

SOURCE=..\cmProjectRule.h
# End Source File
# Begin Source File

SOURCE=..\cmRegularExpression.h
# End Source File
# Begin Source File

SOURCE=..\cmRuleMaker.h
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRequireRule.h
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmStandardIncludes.h
# End Source File
# Begin Source File

SOURCE=..\cmSubdirRule.h
# End Source File
# Begin Source File

SOURCE=..\cmSystemTools.h
# End Source File
# Begin Source File

SOURCE=..\cmTestsRule.h
# End Source File
# Begin Source File

SOURCE=..\cmUnixDefinesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmUnixLibrariesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32DefinesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32LibrariesRule.h
# End Source File
# Begin Source File

SOURCE=..\cmWindowsConfigure.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CMakeSetup.ico
# End Source File
# Begin Source File

SOURCE=.\res\CMakeSetupDialog.ico
# End Source File
# Begin Source File

SOURCE=.\res\CMakeSetupDialog.rc2
# End Source File
# End Group
# End Target
# End Project
