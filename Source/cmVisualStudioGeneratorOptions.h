/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmVisualStudioGeneratorOptions_h
#define cmVisualStudioGeneratorOptions_h

#include "cmLocalGenerator.h"

#include "cmIDEOptions.h"
typedef cmIDEFlagTable cmVS7FlagTable;

class cmVisualStudio10TargetGenerator;

//----------------------------------------------------------------------------
class cmVisualStudioGeneratorOptions: public cmIDEOptions
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
  void ParseFinish();

  // Fix the ExceptionHandling option to default to off.
  void FixExceptionHandlingDefault();

  // Store options for verbose builds.
  void SetVerboseMakefile(bool verbose);

  // Check for specific options.
  bool UsingUnicode();

  bool IsDebug();
  // Write options to output.
  void OutputPreprocessorDefinitions(std::ostream& fout,
                                     const char* prefix,
                                     const char* suffix,
                                     const char* lang);
  void OutputFlagMap(std::ostream& fout, const char* indent);
  void OutputAdditionalOptions(std::ostream& fout,
                               const char* prefix,
                               const char* suffix);
  void SetConfiguration(const char* config);
private:
  cmLocalGenerator* LocalGenerator;
  int Version;

  std::string Configuration;
  Tool CurrentTool;
  cmVisualStudio10TargetGenerator* TargetGenerator;

  bool FortranRuntimeDebug;
  bool FortranRuntimeDLL;
  bool FortranRuntimeMT;

  virtual void StoreUnknownFlag(const char* flag);
};

#endif
