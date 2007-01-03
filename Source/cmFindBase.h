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
  
  virtual const char* GetFullDocumentation()
    {return this->GenericDocumentation.c_str();}

protected:
  void PrintFindStuff();
  void ExpandPaths(std::vector<std::string> userPaths);
  // add to the SearchPaths
  void AddPaths(std::vector<std::string>& paths);
  void AddFrameWorkPaths();
  void AddAppBundlePaths();
  void AddEnvironmentVairables();
  void AddCMakeVairables();
  void AddSystemEnvironmentVairables();
  void AddCMakeSystemVariables();
  void ExpandRegistryAndCleanPath(std::vector<std::string>& paths);
  // see if the VariableName is already set in the cache,
  // also copy the documentation from the cache to VariableDocumentation
  // if it has documentation in the cache
  bool CheckForVariableInCache();
  
  cmStdString GenericDocumentation;
  // use by command during find
  cmStdString VariableDocumentation;
  cmStdString VariableName;
  std::vector<std::string> Names;
  std::vector<std::string> SearchPaths;
  std::vector<std::string> SearchPathSuffixes;

  // CMAKE_*_PATH CMAKE_SYSTEM_*_PATH FRAMEWORK|LIBRARY|INCLUDE|PROGRAM
  cmStdString CMakePathName; 
  cmStdString EnvironmentPath; // LIB,INCLUDE

  bool AlreadyInCache;
  bool AlreadyInCacheWithoutMetaInfo;
  bool NoDefaultPath;
  bool NoCMakePath;
  bool NoCMakeEnvironmentPath;
  bool NoSystemEnvironmentPath;
  bool NoCMakeSystemPath;
  
  bool SearchFrameworkFirst;
  bool SearchFrameworkOnly;
  bool SearchFrameworkLast;
  
  bool SearchAppBundleFirst;
  bool SearchAppBundleOnly;
  bool SearchAppBundleLast;
  
};



#endif
