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
#include "cmAbstractFilesCommand.cxx"
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmEndWhileCommand.cxx"
#include "cmExportLibraryDependencies.cxx"
#include "cmEnableLanguageCommand.cxx"
#include "cmFLTKWrapUICommand.cxx"
#include "cmGetCMakePropertyCommand.cxx"
#include "cmGetDirectoryPropertyCommand.cxx"
#include "cmGetTargetPropertyCommand.cxx"
#include "cmITKWrapTclCommand.cxx"
#include "cmIncludeExternalMSProjectCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmLoadCacheCommand.cxx"
#include "cmOutputRequiredFilesCommand.cxx"
#include "cmRemoveCommand.cxx"
#include "cmSetDirectoryPropertiesCommand.cxx"
#include "cmSetTargetPropertiesCommand.cxx"
#include "cmSourceFilesCommand.cxx"
#include "cmSourceFilesRemoveCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmVTKMakeInstantiatorCommand.cxx"
#include "cmVTKWrapJavaCommand.cxx"
#include "cmVTKWrapPythonCommand.cxx"
#include "cmVTKWrapTclCommand.cxx"
#include "cmQTWrapCPPCommand.cxx"
#include "cmQTWrapUICommand.cxx"
#include "cmUseMangledMesaCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmWhileCommand.cxx"
#include "cmWrapExcludeFilesCommand.cxx"

// This one must be last because it includes windows.h and
// windows.h #defines GetCurrentDirectory which is a member
// of cmMakefile
#include "cmLoadCommandCommand.cxx"
#endif

void GetPredefinedCommands(std::list<cmCommand*>& commands)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands.push_back(new cmAbstractFilesCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEndWhileCommand);
  commands.push_back(new cmExportLibraryDependenciesCommand);
  commands.push_back(new cmFLTKWrapUICommand);
  commands.push_back(new cmGetCMakePropertyCommand);
  commands.push_back(new cmGetDirectoryPropertyCommand);
  commands.push_back(new cmGetTargetPropertyCommand);
  commands.push_back(new cmITKWrapTclCommand);
  commands.push_back(new cmIncludeExternalMSProjectCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmLoadCacheCommand);
  commands.push_back(new cmLoadCommandCommand);
  commands.push_back(new cmOutputRequiredFilesCommand);
  commands.push_back(new cmRemoveCommand);
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSetTargetPropertiesCommand);
  commands.push_back(new cmSourceFilesCommand);
  commands.push_back(new cmSourceFilesRemoveCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmVTKMakeInstantiatorCommand);
  commands.push_back(new cmVTKWrapJavaCommand);
  commands.push_back(new cmVTKWrapPythonCommand);
  commands.push_back(new cmVTKWrapTclCommand);
  commands.push_back(new cmQTWrapCPPCommand);
  commands.push_back(new cmQTWrapUICommand);
  commands.push_back(new cmUseMangledMesaCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmWhileCommand);
  commands.push_back(new cmWrapExcludeFilesCommand);
#endif
}
