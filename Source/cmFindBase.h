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

#include "cmFindCommon.h"

/** \class cmFindBase
 * \brief Base class for most FIND_XXX commands.
 *
 * cmFindBase is a parent class for cmFindProgramCommand, cmFindPathCommand,
 * and cmFindLibraryCommand, cmFindFileCommand
 */
class cmFindBase : public cmFindCommon
{
public:
  cmFindBase();
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool ParseArguments(std::vector<std::string> const& args);
  cmTypeMacro(cmFindBase, cmFindCommon);
  
  virtual const char* GetFullDocumentation()
    {return this->GenericDocumentation.c_str();}

protected:
  void PrintFindStuff();
  void ExpandPaths(std::vector<std::string> userPaths);

  // add to the SearchPaths
  void AddPaths(std::vector<std::string>& paths);
  void AddFrameWorkPaths();
  void AddAppBundlePaths();
  void AddEnvironmentVariables();
  void AddFindPrefix(std::vector<std::string>& dest, 
                     const std::vector<std::string>& src);
  void AddCMakeVariables();
  void AddSystemEnvironmentVariables();
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

  // CMAKE_*_PATH CMAKE_SYSTEM_*_PATH FRAMEWORK|LIBRARY|INCLUDE|PROGRAM
  cmStdString EnvironmentPath; // LIB,INCLUDE

  bool AlreadyInCache;
  bool AlreadyInCacheWithoutMetaInfo;
};



#endif
