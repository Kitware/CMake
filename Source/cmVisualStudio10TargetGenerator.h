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
#ifndef cmVisualStudioTargetGenerator_h
#define cmVisualStudioTargetGenerator_h
#include "cmStandardIncludes.h"

class cmTarget;
class cmMakefile;
class cmGeneratedFileStream;
class cmGlobalVisualStudio7Generator;
class cmSourceFile;
class cmCustomCommand;
class cmLocalVisualStudio7Generator;
class cmComputeLinkInformation;

class cmVisualStudio10TargetGenerator
{
public:
  cmVisualStudio10TargetGenerator(cmTarget* target, 
                                  cmGlobalVisualStudio7Generator* gg);
  ~cmVisualStudio10TargetGenerator();
  void Generate();
  // used by cmVisualStudioGeneratorOptions 
  void WritePlatformConfigTag( 
    const char* tag,
    const char* config, 
    int indentLevel,
    const char* attribute = 0,
    const char* end = 0,
    std::ostream* strm = 0
    );
  
private:
  void ConvertToWindowsSlash(std::string& s);
  void WriteString(const char* line, int indentLevel);
  void WriteProjectConfigurations();
  void WriteProjectConfigurationValues();
  void WriteCLSources();
  void WriteObjSources();
  void WritePathAndIncrementalLinkOptions();
  void WriteItemDefinitionGroups();
  void WriteClOptions(std::string const& config,
                      std::vector<std::string> const & includes);
  void WriteRCOptions(std::string const& config,
                      std::vector<std::string> const & includes);
  void WriteLinkOptions(std::string const& config);
  void WriteMidlOptions(std::string const& config,
                        std::vector<std::string> const & includes);
  void OutputIncludes(std::vector<std::string> const & includes);
  void OutputLinkIncremental(std::string const& configName);
  void WriteCustomRule(cmSourceFile* source,
                       cmCustomCommand const & command);
  void WriteCustomCommands();
  void WriteGroups();
  void WriteProjectReferences();
  bool OutputSourceSpecificFlags(cmSourceFile* source);
  void AddLibraries(cmComputeLinkInformation& cli, std::string& libstring);
  void WriteLibOptions(std::string const& config);
private:
  cmTarget* Target;
  cmMakefile* Makefile;
  std::string Platform;
  std::string GUID;
  cmGlobalVisualStudio7Generator* GlobalGenerator;
  cmGeneratedFileStream* BuildFileStream;
  cmLocalVisualStudio7Generator* LocalGenerator;
};

#endif
