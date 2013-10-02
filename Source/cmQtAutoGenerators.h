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

class cmGlobalGenerator;
class cmMakefile;

class cmQtAutoGenerators
{
public:
  cmQtAutoGenerators();
  bool Run(const char* targetDirectory, const char *config);

  bool InitializeMocSourceFile(cmTarget* target);
  void SetupAutoGenerateTarget(cmTarget* target);

private:
  cmGlobalGenerator* CreateGlobalGenerator(cmake* cm,
                                           const char* targetDirectory);

  bool ReadAutogenInfoFile(cmMakefile* makefile,
                           const char* targetDirectory,
                           const char *config);
  bool ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                 const char* targetDirectory);
  void WriteOldMocDefinitionsFile(const char* targetDirectory);

  std::string MakeCompileSettingsString(cmMakefile* makefile);

  bool RunAutomoc(cmMakefile* makefile);
  bool GenerateMoc(const std::string& sourceFile,
                   const std::string& mocFileName);
  void ParseCppFile(const std::string& absFilename,
                    const std::vector<std::string>& headerExtensions,
                    std::map<std::string, std::string>& includedMocs);
  void StrictParseCppFile(const std::string& absFilename,
                          const std::vector<std::string>& headerExtensions,
                          std::map<std::string, std::string>& includedMocs);
  void SearchHeadersForCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::set<std::string>& absHeaders);

  void ParseHeaders(const std::set<std::string>& absHeaders,
                    const std::map<std::string, std::string>& includedMocs,
                    std::map<std::string, std::string>& notIncludedMocs);

  void Init();

  std::string Join(const std::vector<std::string>& lst, char separator);
  bool EndsWith(const std::string& str, const std::string& with);
  bool StartsWith(const std::string& str, const std::string& with);
  std::string ReadAll(const std::string& filename);

  std::string QtMajorVersion;
  std::string Sources;
  std::string Headers;
  bool IncludeProjectDirsBefore;
  std::string Srcdir;
  std::string Builddir;
  std::string MocExecutable;
  std::string MocCompileDefinitionsStr;
  std::string MocIncludesStr;
  std::string MocOptionsStr;
  std::string ProjectBinaryDir;
  std::string ProjectSourceDir;
  std::string TargetName;

  std::string CurrentCompileSettingsStr;
  std::string OldCompileSettingsStr;

  std::string OutMocCppFilename;
  std::list<std::string> MocIncludes;
  std::list<std::string> MocDefinitions;
  std::vector<std::string> MocOptions;

  bool Verbose;
  bool ColorOutput;
  bool RunMocFailed;
  bool GenerateAll;
  bool RelaxedMode;

};

#endif
