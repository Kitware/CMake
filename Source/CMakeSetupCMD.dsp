# Microsoft Developer Studio Project File - Name="CMakeSetupCMD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CMakeSetupCMD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CMakeSetupCMD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CMakeSetupCMD.mak" CFG="CMakeSetupCMD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CMakeSetupCMD - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "CMakeSetupCMD - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CMakeSetupCMD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "CMakeSetupCMD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "CMakeSetupCMD - Win32 Release"
# Name "CMakeSetupCMD - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmAbstractFilesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmAddTargetCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\CMakeSetupCMD.cxx
# End Source File
# Begin Source File

SOURCE=.\cmAuxSourceDirectoryCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmClassFile.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDirectory.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDSPMakefile.cxx
# End Source File
# Begin Source File

SOURCE=.\cmDSWMakefile.cxx
# End Source File
# Begin Source File

SOURCE=.\cmExecutablesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmFindIncludeCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmFindLibraryCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmFindProgramCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmIncludeDirectoryCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmLibraryCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmLinkDirectoriesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmLinkLibrariesCommand.cxx
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

SOURCE=.\cmProjectCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmRegularExpression.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSourceFilesRequireCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSourceFilesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSubdirCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmSystemTools.cxx
# End Source File
# Begin Source File

SOURCE=.\cmTestsCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmUnixDefinesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmUnixLibrariesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmWin32DefinesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmWin32LibrariesCommand.cxx
# End Source File
# Begin Source File

SOURCE=.\cmWindowsConfigure.cxx
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

SOURCE=.\cmClassFile.h
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

SOURCE=.\cmMakefile2.h
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

SOURCE=.\cmCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmSourceFilesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmStandardIncludes.h
# End Source File
# Begin Source File

SOURCE=.\cmSubdirCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmSystemTools.h
# End Source File
# Begin Source File

SOURCE=.\cmTestsCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmUnixDefinesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmUnixLibrariesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmWin32DefinesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmWin32LibrariesCommand.h
# End Source File
# Begin Source File

SOURCE=.\cmWindowsConfigure.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
