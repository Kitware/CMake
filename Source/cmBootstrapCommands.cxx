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
// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.   
#include "cmCommands.h"
#include "cmAddCustomCommandCommand.cxx"
#include "cmAddCustomTargetCommand.cxx"
#include "cmAddDefinitionsCommand.cxx"
#include "cmAddDependenciesCommand.cxx"
#include "cmAddExecutableCommand.cxx"
#include "cmAddLibraryCommand.cxx"
#include "cmAddTestCommand.cxx"
#include "cmBuildCommand.cxx"
#include "cmBuildNameCommand.cxx"
#include "cmCMakeMinimumRequired.cxx"
#include "cmConfigureFileCommand.cxx"
#include "cmCreateTestSourceList.cxx"
#include "cmElseCommand.cxx"
#include "cmEnableTestingCommand.cxx"
#include "cmEndForEachCommand.cxx"
#include "cmEndIfCommand.cxx"
#include "cmExecProgramCommand.cxx"
#include "cmFileCommand.cxx"
#include "cmFindFileCommand.cxx"
#include "cmFindLibraryCommand.cxx"
#include "cmFindPackageCommand.cxx"
#include "cmFindPathCommand.cxx"
#include "cmFindProgramCommand.cxx"
#include "cmForEachCommand.cxx"
#include "cmGetFilenameComponentCommand.cxx"
#include "cmGetSourceFilePropertyCommand.cxx"
#include "cmIfCommand.cxx"
#include "cmIncludeCommand.cxx"
#include "cmIncludeDirectoryCommand.cxx"
#include "cmIncludeRegularExpressionCommand.cxx"
#include "cmInstallFilesCommand.cxx"
#include "cmInstallProgramsCommand.cxx"
#include "cmInstallTargetsCommand.cxx"
#include "cmLinkDirectoriesCommand.cxx"
#include "cmMacroCommand.cxx"
#include "cmMakeDirectoryCommand.cxx"
#include "cmMarkAsAdvancedCommand.cxx"
#include "cmMessageCommand.cxx"
#include "cmOptionCommand.cxx"
#include "cmProjectCommand.cxx"
#include "cmRemoveDefinitionsCommand.cxx"
#include "cmSeparateArgumentsCommand.cxx"
#include "cmSetCommand.cxx"
#include "cmSetSourceFilesPropertiesCommand.cxx"
#include "cmSiteNameCommand.cxx"
#include "cmStringCommand.cxx"
#include "cmSubdirCommand.cxx"
#include "cmSubdirDependsCommand.cxx"
#include "cmTargetLinkLibrariesCommand.cxx"
#include "cmTryCompileCommand.cxx"
#include "cmTryRunCommand.cxx"
#include "cmVariableRequiresCommand.cxx"
#include "cmWriteFileCommand.cxx"

void GetBootstrapCommands(std::list<cmCommand*>& commands)
{
  commands.push_back(new cmAddCustomCommandCommand);
  commands.push_back(new cmAddCustomTargetCommand);
  commands.push_back(new cmAddDefinitionsCommand);
  commands.push_back(new cmAddDependenciesCommand);
  commands.push_back(new cmAddExecutableCommand);
  commands.push_back(new cmAddLibraryCommand);
  commands.push_back(new cmAddTestCommand);
  commands.push_back(new cmBuildCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmCMakeMinimumRequired);
  commands.push_back(new cmConfigureFileCommand);
  commands.push_back(new cmCreateTestSourceList);
  commands.push_back(new cmElseCommand);
  commands.push_back(new cmEnableTestingCommand);  
  commands.push_back(new cmEndForEachCommand);
  commands.push_back(new cmEndIfCommand);
  commands.push_back(new cmExecProgramCommand);
  commands.push_back(new cmFileCommand);
  commands.push_back(new cmFindFileCommand);
  commands.push_back(new cmFindLibraryCommand);
  commands.push_back(new cmFindPackageCommand);
  commands.push_back(new cmFindPathCommand);
  commands.push_back(new cmFindProgramCommand);
  commands.push_back(new cmForEachCommand);
  commands.push_back(new cmGetFilenameComponentCommand);
  commands.push_back(new cmGetSourceFilePropertyCommand);
  commands.push_back(new cmIfCommand);
  commands.push_back(new cmIncludeCommand);
  commands.push_back(new cmIncludeDirectoryCommand);
  commands.push_back(new cmIncludeRegularExpressionCommand);
  commands.push_back(new cmInstallFilesCommand);
  commands.push_back(new cmInstallProgramsCommand);
  commands.push_back(new cmInstallTargetsCommand);
  commands.push_back(new cmLinkDirectoriesCommand);
  commands.push_back(new cmMacroCommand);
  commands.push_back(new cmMakeDirectoryCommand);
  commands.push_back(new cmMarkAsAdvancedCommand);
  commands.push_back(new cmMessageCommand);
  commands.push_back(new cmOptionCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmRemoveDefinitionsCommand);
  commands.push_back(new cmSeparateArgumentsCommand);
  commands.push_back(new cmSetCommand);
  commands.push_back(new cmSetSourceFilesPropertiesCommand);
  commands.push_back(new cmSiteNameCommand);
  commands.push_back(new cmStringCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmSubdirDependsCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmTryCompileCommand);
  commands.push_back(new cmTryRunCommand);
  commands.push_back(new cmVariableRequiresCommand);
  commands.push_back(new cmWriteFileCommand);
}
