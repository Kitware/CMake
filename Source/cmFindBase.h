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
#ifndef cmFindBase_h
#define cmFindBase_h

#include "cmCommand.h"

/** \class cmFindBase
 * \brief Define a command to search for an executable program.
 *
 * cmFindBase is a parent class for cmFindProgramCommand, cmFindPathCommand,
 * and cmFindLibraryCommand, cmFindFile
 */
class cmFindBase : public cmCommand
{
public:
  cmFindBase();
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool ParseArguments(std::vector<std::string> const& args);
  cmTypeMacro(cmFindBase, cmCommand);
protected:
  void PrintFindStuff();
  void ExpandPaths(std::vector<std::string> userPaths);
  void AddEnvironmentVairables();
  void AddCMakeVairables();
  void AddSystemEnvironmentVairables();
  void AddCMakeSystemVariables();
  void ExpandRegistryAndCleanPath();
  // see if the VariableName is already set in the cache,
  // also copy the documentation from the cache to VariableDocumentation
  // if it has documentation in the cache
  bool CheckForVariableInCache();
  
  // use by command during find
  cmStdString VariableDocumentation;
  cmStdString VariableName;
  std::vector<std::string> Names;
  std::vector<std::string> SearchPaths;
  std::vector<std::string> SearchPathSuffixes;
  
  cmStdString CMakePathName; // CMAKE_*_PATH CMAKE_SYSTEM_*_PATH FRAMEWORK|LIBRARY|INCLUDE|PROGRAM
  cmStdString EnvironmentPath; // LIB,INCLUDE

  bool AlreadyInCache;
  bool NoSystemPath;
  bool NoCMakeEnvironmentPath;
  bool NoCMakePath;
  bool NoCMakeSystemPath;
  
  bool SearchFrameworkFirst;
  bool SearchFrameworkOnly;
  bool SearchFrameworkLast;
  
};



#endif
