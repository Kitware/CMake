/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmQtAutoGenerators_h
#define cmQtAutoGenerators_h

#include "cmStandardIncludes.h"

#include <list>
#include <map>
#include <string>
#include <vector>

class cmMakefile;

class cmQtAutoGenerators
{
public:
  cmQtAutoGenerators();
  bool Run(const std::string& targetDirectory, const std::string& config);

private:
  bool ReadAutogenInfoFile(cmMakefile* makefile,
                           const std::string& targetDirectory,
                           const std::string& config);
  bool ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                 const std::string& targetDirectory);
  void WriteOldMocDefinitionsFile(const std::string& targetDirectory);

  std::string MakeCompileSettingsString(cmMakefile* makefile);

  bool RunAutogen(cmMakefile* makefile);
  bool GenerateMocFiles(
    const std::map<std::string, std::string>& includedMocs,
    const std::map<std::string, std::string>& notIncludedMocs);
  bool GenerateMoc(const std::string& sourceFile,
                   const std::string& mocFileName);
  bool GenerateUiFiles(
    const std::map<std::string, std::vector<std::string> >& includedUis);
  bool GenerateUi(const std::string& realName, const std::string& uiInputFile,
                  const std::string& uiOutputFile);
  bool GenerateQrcFiles();
  bool GenerateQrc(const std::string& qrcInputFile,
                   const std::string& qrcOutputFile);
  void ParseCppFile(
    const std::string& absFilename,
    const std::vector<std::string>& headerExtensions,
    std::map<std::string, std::string>& includedMocs,
    std::map<std::string, std::vector<std::string> >& includedUis);
  void StrictParseCppFile(
    const std::string& absFilename,
    const std::vector<std::string>& headerExtensions,
    std::map<std::string, std::string>& includedMocs,
    std::map<std::string, std::vector<std::string> >& includedUis);
  void SearchHeadersForCppFile(
    const std::string& absFilename,
    const std::vector<std::string>& headerExtensions,
    std::set<std::string>& absHeaders);

  void ParseHeaders(
    const std::set<std::string>& absHeaders,
    const std::map<std::string, std::string>& includedMocs,
    std::map<std::string, std::string>& notIncludedMocs,
    std::map<std::string, std::vector<std::string> >& includedUis);

  void ParseForUic(
    const std::string& fileName, const std::string& contentsString,
    std::map<std::string, std::vector<std::string> >& includedUis);

  void ParseForUic(
    const std::string& fileName,
    std::map<std::string, std::vector<std::string> >& includedUis);

  void Init();

  std::string SourceRelativePath(const std::string& filename);

  bool NameCollisionTest(const std::map<std::string, std::string>& genFiles,
                         std::multimap<std::string, std::string>& collisions);
  void NameCollisionLog(
    const std::string& message,
    const std::multimap<std::string, std::string>& collisions);

  void LogInfo(const std::string& message);
  void LogError(const std::string& message);
  void LogCommand(const std::vector<std::string>& command);
  std::string JoinExts(const std::vector<std::string>& lst);

  static void MergeUicOptions(std::vector<std::string>& opts,
                              const std::vector<std::string>& fileOpts,
                              bool isQt5);

  bool InputFilesNewerThanQrc(const std::string& qrcFile,
                              const std::string& rccOutput);

  std::string QtMajorVersion;
  std::string Sources;
  std::vector<std::string> RccSources;
  std::string SkipMoc;
  std::string SkipUic;
  std::string Headers;
  std::string Srcdir;
  std::string Builddir;
  std::string MocExecutable;
  std::string UicExecutable;
  std::string RccExecutable;
  std::string MocCompileDefinitionsStr;
  std::string MocIncludesStr;
  std::string MocOptionsStr;
  std::string ProjectBinaryDir;
  std::string ProjectSourceDir;
  std::string TargetName;
  std::string OriginTargetName;

  std::string CurrentCompileSettingsStr;
  std::string OldCompileSettingsStr;

  std::string TargetBuildSubDir;
  std::string OutMocCppFilenameRel;
  std::string OutMocCppFilenameAbs;
  std::list<std::string> MocIncludes;
  std::list<std::string> MocDefinitions;
  std::vector<std::string> MocOptions;
  std::vector<std::string> UicTargetOptions;
  std::map<std::string, std::string> UicOptions;
  std::map<std::string, std::string> RccOptions;
  std::map<std::string, std::vector<std::string> > RccInputs;

  bool IncludeProjectDirsBefore;
  bool Verbose;
  bool ColorOutput;
  bool RunMocFailed;
  bool RunUicFailed;
  bool RunRccFailed;
  bool GenerateAll;
  bool RelaxedMode;
};

#endif
