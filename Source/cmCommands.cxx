// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.   
#include "cmCommands.h"
#include "cmAbstractFilesCommand.cxx"
#include "cmAddExecutableCommand.cxx"
#include "cmAddLibraryCommand.cxx"
#include "cmAddTargetCommand.cxx"
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmFindIncludeCommand.cxx"
#include "cmFindLibraryCommand.cxx"
#include "cmFindProgramCommand.cxx"
#include "cmIncludeDirectoryCommand.cxx"
#include "cmLinkDirectoriesCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmTargetLinkLibrariesCommand.cxx"
#include "cmProjectCommand.cxx"
#include "cmSourceFilesCommand.cxx"
#include "cmSourceFilesRequireCommand.cxx"
#include "cmSubdirCommand.cxx"
#include "cmTestsCommand.cxx"
#include "cmUnixDefinesCommand.cxx"
#include "cmUnixLibrariesCommand.cxx"
#include "cmWin32DefinesCommand.cxx"
#include "cmWin32LibrariesCommand.cxx"
#include "cmWin32IncludeDirectoryCommand.cxx"
#include "cmConfigureFile.cxx"
#include "cmConfigureFileNoAutoconf.cxx"
#include "cmCableCommand.cxx"
#include "cmCableData.cxx"
#include "cmCableDefineSetCommand.cxx"
#include "cmCableOpenNamespaceCommand.cxx"
#include "cmCableCloseNamespaceCommand.cxx"
#include "cmCablePackageCommand.cxx"
#include "cmCablePackageEntryCommand.cxx"
#include "cmCableSourceFilesCommand.cxx"
#include "cmCableWrapCommand.cxx"
#include "cmCableInstantiateCommand.cxx"
#include "cmCableInstantiateClassCommand.cxx"
#include "cmFindFileCommand.cxx"
#include "cmFindPathCommand.cxx"
#include "cmWrapExcludeFilesCommand.cxx"
#include "cmVTKWrapPythonCommand.cxx"
#include "cmVTKWrapTclCommand.cxx"
#include "cmBuildSharedLibrariesCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmIncludeRegularExpressionCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmIfCommand.cxx"
#include "cmElseCommand.cxx"
#include "cmEndIfCommand.cxx"
#include "cmAddDefinitionsCommand.cxx"
#include "cmOptionCommand.cxx"
#include "cmIncludeCommand.cxx"

void GetPredefinedCommands(std::list<cmCommand*>& commands)
{
  commands.push_back(new cmAbstractFilesCommand);
  commands.push_back(new cmAddExecutableCommand);
  commands.push_back(new cmAddLibraryCommand);
  commands.push_back(new cmAddTargetCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmFindIncludeCommand);
  commands.push_back(new cmFindLibraryCommand);
  commands.push_back(new cmFindProgramCommand);
  commands.push_back(new cmIncludeDirectoryCommand);
  commands.push_back(new cmLinkDirectoriesCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmSourceFilesCommand);
  commands.push_back(new cmSourceFilesRequireCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmTestsCommand);
  commands.push_back(new cmUnixDefinesCommand);
  commands.push_back(new cmUnixLibrariesCommand);
  commands.push_back(new cmWin32DefinesCommand);
  commands.push_back(new cmWin32LibrariesCommand);
  commands.push_back(new cmWin32IncludeDirectoryCommand);
  commands.push_back(new cmConfigureFile);
  commands.push_back(new cmConfigureFileNoAutoconf);
  commands.push_back(new cmCableDefineSetCommand);
  commands.push_back(new cmCableOpenNamespaceCommand);
  commands.push_back(new cmCableCloseNamespaceCommand);
  commands.push_back(new cmCablePackageCommand);
  commands.push_back(new cmCableSourceFilesCommand);
  commands.push_back(new cmCableWrapCommand);
  commands.push_back(new cmCableInstantiateCommand);
  commands.push_back(new cmCableInstantiateClassCommand);
  commands.push_back(new cmFindFileCommand);
  commands.push_back(new cmFindPathCommand);
  commands.push_back(new cmWrapExcludeFilesCommand);  
  commands.push_back(new cmVTKWrapPythonCommand);
  commands.push_back(new cmVTKWrapTclCommand);
  commands.push_back(new cmBuildSharedLibrariesCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmIncludeRegularExpressionCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmIfCommand);
  commands.push_back(new cmElseCommand);
  commands.push_back(new cmEndIfCommand);
  commands.push_back(new cmAddDefinitionsCommand);
  commands.push_back(new cmOptionCommand);
  commands.push_back(new cmIncludeCommand);
}

  
