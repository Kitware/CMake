/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCommands.h"
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmBuildNameCommand.cxx"
#include "cmElseIfCommand.cxx"
#include "cmEnableLanguageCommand.cxx"
#include "cmEndWhileCommand.cxx"
#include "cmExportCommand.cxx"
#include "cmExportLibraryDependencies.cxx"
#include "cmFLTKWrapUICommand.cxx"
#include "cmGetTestPropertyCommand.cxx"
#include "cmIncludeExternalMSProjectCommand.cxx"
#include "cmInstallProgramsCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmLoadCacheCommand.cxx"
#include "cmOutputRequiredFilesCommand.cxx"
#include "cmQTWrapCPPCommand.cxx"
#include "cmQTWrapUICommand.cxx"
#include "cmRemoveCommand.cxx"
#include "cmRemoveDefinitionsCommand.cxx"
#include "cmSetDirectoryPropertiesCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmSubdirDependsCommand.cxx"
#include "cmUseMangledMesaCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmVariableRequiresCommand.cxx"
#include "cmVariableWatchCommand.cxx"

#include "cmWhileCommand.cxx"
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
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmElseIfCommand);
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEndWhileCommand);
  commands.push_back(new cmExportCommand);
  commands.push_back(new cmExportLibraryDependenciesCommand);
  commands.push_back(new cmFLTKWrapUICommand);
  commands.push_back(new cmGetTestPropertyCommand);
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
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmSubdirDependsCommand);
  commands.push_back(new cmUseMangledMesaCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmVariableRequiresCommand);
  commands.push_back(new cmVariableWatchCommand);
  commands.push_back(new cmWhileCommand);
  commands.push_back(new cmWriteFileCommand);
#endif
}
