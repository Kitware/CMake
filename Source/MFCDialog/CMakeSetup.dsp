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

SOURCE=..\cmAbstractFilesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmAddTargetCommand.cxx
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

SOURCE=..\cmAuxSourceDirectoryCommand.cxx
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

SOURCE=..\cmExecutablesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindIncludeCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindLibraryCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmFindProgramCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmIncludeDirectoryCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLibraryCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLinkDirectoriesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmLinkLibrariesCommand.cxx
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

SOURCE=..\cmProjectCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmRegularExpression.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRequireCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSubdirCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmSystemTools.cxx
# End Source File
# Begin Source File

SOURCE=..\cmTestsCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmUnixDefinesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmUnixLibrariesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmWin32DefinesCommand.cxx
# End Source File
# Begin Source File

SOURCE=..\cmWin32LibrariesCommand.cxx
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

SOURCE=..\cmAbstractFilesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmAddTargetCommand.h
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

SOURCE=..\cmAuxSourceDirectoryCommand.h
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

SOURCE=..\cmExecutablesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmFindIncludeCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmFindLibraryCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmFindProgramCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmIncludeDirectoryCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmLibraryCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmLinkDirectoriesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmLinkLibrariesCommand.h
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

SOURCE=..\cmProjectCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmRegularExpression.h
# End Source File
# Begin Source File

SOURCE=..\cmCommandMaker.h
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRequireCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmStandardIncludes.h
# End Source File
# Begin Source File

SOURCE=..\cmSubdirCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmSystemTools.h
# End Source File
# Begin Source File

SOURCE=..\cmTestsCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmUnixDefinesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmUnixLibrariesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32DefinesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32LibrariesCommand.h
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
