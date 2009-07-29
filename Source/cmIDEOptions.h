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
#ifndef cmIDEOptions_h
#define cmIDEOptions_h

#include "cmStandardIncludes.h"
#include "cmIDEFlagTable.h"

/** \class cmIDEOptions
 * \brief Superclass for IDE option processing
 */
class cmIDEOptions
{
public:
  cmIDEOptions();
  virtual ~cmIDEOptions();

  // Store definitions and flags.
  void AddDefine(const std::string& define);
  void AddDefines(const char* defines);
  void AddFlag(const char* flag, const char* value);

protected:
  // create a map of xml tags to the values they should have in the output
  // for example, "BufferSecurityCheck" = "TRUE"
  // first fill this table with the values for the configuration
  // Debug, Release, etc,
  // Then parse the command line flags specified in CMAKE_CXX_FLAGS
  // and CMAKE_C_FLAGS
  // and overwrite or add new values to this map
  std::map<cmStdString, cmStdString> FlagMap;

  // Preprocessor definitions.
  std::vector<std::string> Defines;

  // Unrecognized flags that get no special handling.
  cmStdString FlagString;

  bool DoingDefine;
  bool AllowDefine;
  bool AllowSlash;
  enum { FlagTableCount = 16 };
  cmIDEFlagTable const* FlagTable[FlagTableCount];
  void HandleFlag(const char* flag);
  bool CheckFlagTable(cmIDEFlagTable const* table, const char* flag,
                      bool& flag_handled);
  virtual void StoreUnknownFlag(const char* flag) = 0;
};

#endif
