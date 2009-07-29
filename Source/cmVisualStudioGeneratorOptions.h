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
#ifndef cmVisualStudioGeneratorOptions_h
#define cmVisualStudioGeneratorOptions_h

#include "cmLocalGenerator.h"

#include "cmIDEFlagTable.h"
typedef cmIDEFlagTable cmVS7FlagTable;

class cmVisualStudio10TargetGenerator;

//----------------------------------------------------------------------------
class cmVisualStudioGeneratorOptions
{
public:
  // Construct an options table for a given tool.
  enum Tool
  {
    Compiler,
    Linker,
    FortranCompiler
  };
  cmVisualStudioGeneratorOptions(cmLocalGenerator* lg,
                                 int version,
                                 Tool tool,
                                 cmVS7FlagTable const* table,
                                 cmVS7FlagTable const* extraTable = 0,
                                 cmVisualStudio10TargetGenerator* g = 0);

  // Store options from command line flags.
  void Parse(const char* flags);

  // Fix the ExceptionHandling option to default to off.
  void FixExceptionHandlingDefault();

  // Store options for verbose builds.
  void SetVerboseMakefile(bool verbose);

  // Store definitions and flags.
  void AddDefine(const std::string& define);
  void AddDefines(const char* defines);
  void AddFlag(const char* flag, const char* value);

  // Check for specific options.
  bool UsingUnicode();

  bool IsDebug();
  // Write options to output.
  void OutputPreprocessorDefinitions(std::ostream& fout,
                                     const char* prefix,
                                     const char* suffix);
  void OutputFlagMap(std::ostream& fout, const char* indent);
  void OutputAdditionalOptions(std::ostream& fout,
                               const char* prefix,
                               const char* suffix);
  void SetConfiguration(const char* config);
private:
  cmLocalGenerator* LocalGenerator;
  int Version;

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
  std::string Configuration;
  cmVisualStudio10TargetGenerator* TargetGenerator;
  Tool CurrentTool;
  bool DoingDefine;
  cmVS7FlagTable const* FlagTable;
  cmVS7FlagTable const* ExtraFlagTable;
  void HandleFlag(const char* flag);
  bool CheckFlagTable(cmVS7FlagTable const* table, const char* flag,
                      bool& flag_handled);
};

#endif
