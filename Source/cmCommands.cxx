// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.   
#include "cmCommands.h"
#include "cmAbstractFilesCommand.cxx"
#include "cmAddCustomTargetCommand.cxx"
#include "cmAddDefinitionsCommand.cxx"
#include "cmAddExecutableCommand.cxx"
#include "cmAddLibraryCommand.cxx"
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmBuildCommand.cxx"
#include "cmBuildNameCommand.cxx"
#include "cmBuildSharedLibrariesCommand.cxx"
#include "cmCableCloseNamespaceCommand.cxx"
#include "cmCableCommand.cxx"
#include "cmCableData.cxx"
#include "cmCableDefineSetCommand.cxx"
#include "cmCableInstantiateClassCommand.cxx"
#include "cmCableInstantiateCommand.cxx"
#include "cmCableOpenNamespaceCommand.cxx"
#include "cmCablePackageCommand.cxx"
#include "cmCablePackageEntryCommand.cxx"
#include "cmCableSourceFilesCommand.cxx"
#include "cmCableWrapCommand.cxx"
#include "cmConfigureFile.cxx"
#include "cmConfigureFileNoAutoconf.cxx"
#include "cmElseCommand.cxx"
#include "cmEndIfCommand.cxx"
#include "cmExecProgram.cxx"
#include "cmFindFileCommand.cxx"
#include "cmFindIncludeCommand.cxx"
#include "cmFindLibraryCommand.cxx"
#include "cmFindPathCommand.cxx"
#include "cmFindProgramCommand.cxx"
#include "cmIfCommand.cxx"
#include "cmIncludeCommand.cxx"
#include "cmIncludeDirectoryCommand.cxx"
#include "cmIncludeRegularExpressionCommand.cxx"
#include "cmLinkDirectoriesCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmOptionCommand.cxx"
#include "cmProjectCommand.cxx"
#include "cmSetCommand.cxx"
#include "cmSiteNameCommand.cxx"
#include "cmSourceFilesCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmSubdirCommand.cxx"
#include "cmTargetLinkLibrariesCommand.cxx"
#include "cmTestsCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmVTKWrapJavaCommand.cxx"
#include "cmVTKWrapPythonCommand.cxx"
#include "cmVTKWrapTclCommand.cxx"
#include "cmWrapExcludeFilesCommand.cxx"


void GetPredefinedCommands(std::list<cmCommand*>& commands)
{
  commands.push_back(new cmAbstractFilesCommand);
  commands.push_back(new cmAddCustomTargetCommand);
  commands.push_back(new cmAddDefinitionsCommand);
  commands.push_back(new cmAddExecutableCommand);
  commands.push_back(new cmAddLibraryCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmBuildCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmBuildSharedLibrariesCommand);
  commands.push_back(new cmCableCloseNamespaceCommand);
  commands.push_back(new cmCableDefineSetCommand);
  commands.push_back(new cmCableInstantiateCommand);
  commands.push_back(new cmCableInstantiateClassCommand);
  commands.push_back(new cmCableOpenNamespaceCommand);
  commands.push_back(new cmCablePackageCommand);
  commands.push_back(new cmCableSourceFilesCommand);
  commands.push_back(new cmCableWrapCommand);
  commands.push_back(new cmConfigureFile);
  commands.push_back(new cmConfigureFileNoAutoconf);
  commands.push_back(new cmElseCommand);
  commands.push_back(new cmEndIfCommand);
  commands.push_back(new cmExecProgram);
  commands.push_back(new cmFindFileCommand);
  commands.push_back(new cmFindIncludeCommand);
  commands.push_back(new cmFindLibraryCommand);
  commands.push_back(new cmFindPathCommand);
  commands.push_back(new cmFindProgramCommand);
  commands.push_back(new cmIfCommand);
  commands.push_back(new cmIncludeCommand);
  commands.push_back(new cmIncludeDirectoryCommand);
  commands.push_back(new cmIncludeRegularExpressionCommand);
  commands.push_back(new cmLinkDirectoriesCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmOptionCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmSetCommand);
  commands.push_back(new cmSiteNameCommand);
  commands.push_back(new cmSourceFilesCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmTestsCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmVTKWrapJavaCommand);
  commands.push_back(new cmVTKWrapPythonCommand);
  commands.push_back(new cmVTKWrapTclCommand);
  commands.push_back(new cmWrapExcludeFilesCommand);  
}

  
