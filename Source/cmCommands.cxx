/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCommands.h"
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmAddCompileOptionsCommand.cxx"
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmBuildNameCommand.cxx"
#include "cmElseIfCommand.cxx"
#include "cmExportCommand.cxx"
#include "cmExportLibraryDependencies.cxx"
#include "cmFLTKWrapUICommand.cxx"
#include "cmIncludeExternalMSProjectCommand.cxx"
#include "cmInstallProgramsCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmLoadCacheCommand.cxx"
#include "cmOutputRequiredFilesCommand.cxx"
#include "cmQTWrapCPPCommand.cxx"
#include "cmQTWrapUICommand.cxx"
#include "cmRemoveCommand.cxx"
#include "cmRemoveDefinitionsCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmSubdirDependsCommand.cxx"
#include "cmTargetCompileDefinitionsCommand.cxx"
#include "cmTargetCompileOptionsCommand.cxx"
#include "cmTargetIncludeDirectoriesCommand.cxx"
#include "cmTargetPropCommandBase.cxx"
#include "cmUseMangledMesaCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmVariableRequiresCommand.cxx"
#include "cmVariableWatchCommand.cxx"

#include "cmWriteFileCommand.cxx"

// This one must be last because it includes windows.h and
// windows.h #defines GetCurrentDirectory which is a member
// of cmMakefile
#include "cmLoadCommandCommand.cxx"
#endif

void GetPredefinedCommands(std::list<cmCommand*>&
#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands
#endif
  )
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands.push_back(new cmAddCompileOptionsCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmElseIfCommand);
  commands.push_back(new cmExportCommand);
  commands.push_back(new cmExportLibraryDependenciesCommand);
  commands.push_back(new cmFLTKWrapUICommand);
  commands.push_back(new cmIncludeExternalMSProjectCommand);
  commands.push_back(new cmInstallProgramsCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmLoadCacheCommand);
  commands.push_back(new cmLoadCommandCommand);
  commands.push_back(new cmOutputRequiredFilesCommand);
  commands.push_back(new cmQTWrapCPPCommand);
  commands.push_back(new cmQTWrapUICommand);
  commands.push_back(new cmRemoveCommand);
  commands.push_back(new cmRemoveDefinitionsCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmSubdirDependsCommand);
  commands.push_back(new cmTargetIncludeDirectoriesCommand);
  commands.push_back(new cmTargetCompileDefinitionsCommand);
  commands.push_back(new cmTargetCompileOptionsCommand);
  commands.push_back(new cmUseMangledMesaCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmVariableRequiresCommand);
  commands.push_back(new cmVariableWatchCommand);
  commands.push_back(new cmWriteFileCommand);
#endif
}
