# Microsoft Developer Studio Project File - Name="CMakeLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CMakeLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CMakeLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CMakeLib.mak" CFG="CMakeLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CMakeLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CMakeLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CMakeLib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CMakeLib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "CMakeLib - Win32 Release"
# Name "CMakeLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmake.cxx
# End Source File
# Begin Source File

SOURCE=.\cmNMakeMakefileGenerator.cxx
# End Source File
# Begin Source File

SOURCE=.\cmBorlandMakefileGenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\cmCableClassSet.cxx
# End Source File
# Begin Source File

SOURCE=.\cmCacheManager.cxx
# End Source File
# Begin Source File

SOURCE=.\cmCommands.cxx
# End Source File
# Begin Source File

SOURCE=.\cmCustomCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDirectory.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDSPWriter.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDSWWriter.cxx
# End Source File
# Begin Source File

SOURCE=.\cmListFileCache.cxx
# End Source File
# Begin Source File

SOURCE=.\cmMakeDepend.cxx
# End Source File
# Begin Source File

SOURCE=.\cmMakefile.cxx
# End Source File
# Begin Source File

SOURCE=.\cmMakefileGenerator.cxx
# End Source File
# Begin Source File

SOURCE=.\cmMSProjectGenerator.cxx
# End Source File
# Begin Source File

SOURCE=.\cmRegularExpression.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSourceFile.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSourceGroup.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSystemTools.cxx
# End Source File
# Begin Source File

SOURCE=.\cmTarget.cxx
# End Source File
# Begin Source File

SOURCE=.\cmUnixMakefileGenerator.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cmAbstractFilesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmAddTargetCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmAuxSourceDirectoryCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmCacheManager.h
# End Source File
# Begin Source File

SOURCE=.\cmCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmDirectory.h
# End Source File
# Begin Source File

SOURCE=.\cmDSPMakefile.h
# End Source File
# Begin Source File

SOURCE=.\cmDSWMakefile.h
# End Source File
# Begin Source File

SOURCE=.\cmExecutablesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmFindIncludeCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmFindLibraryCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmFindProgramCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmGetFilenameComponentCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmIncludeDirectoryCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmLibraryCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmLinkDirectoriesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmLinkLibrariesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmMakeDepend.h
# End Source File
# Begin Source File

SOURCE=.\cmMakefile.h
# End Source File
# Begin Source File

SOURCE=.\cmMakefileGenerator.h
# End Source File
# Begin Source File

SOURCE=.\cmMSProjectGenerator.h
# End Source File
# Begin Source File

SOURCE=.\cmProjectCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmRegularExpression.h
# End Source File
# Begin Source File

SOURCE=..\cmSourceFilesRequireCommand.h
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

SOURCE=..\cmUnixMakefileGenerator.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32DefinesCommand.h
# End Source File
# Begin Source File

SOURCE=..\cmWin32LibrariesCommand.h
# End Source File
# End Group
# End Target
# End Project
