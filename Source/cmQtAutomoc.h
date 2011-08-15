#ifndef cmQtAutomoc_h
#define cmQtAutomoc_h

class cmGlobalGenerator;
class cmMakefile;

class cmQtAutomoc
{
public:
  cmQtAutomoc();
  bool Run(const char* targetDirectory);

  void SetupAutomocTarget(cmTarget* target);

private:
  cmGlobalGenerator* CreateGlobalGenerator(cmake* cm,
                                           const char* targetDirectory);

  bool ReadAutomocInfoFile(cmMakefile* makefile,
                           const char* targetDirectory);
  bool ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                 const char* targetDirectory);
  void WriteOldMocDefinitionsFile(const char* targetDirectory);

  bool RunAutomocQt4();
  bool GenerateMoc(const std::string& sourceFile,
                   const std::string& mocFileName);
  void ParseCppFile(const std::string& absFilename,
                    std::map<std::string, std::string>& includedMocs,
                    std::map<std::string, std::string>& notIncludedMocs);

  void Init();

  std::string Join(const std::list<std::string>& lst, char separator);
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
  std::string MocDefinitionsStr;
  std::string MocIncludesStr;
  std::string ProjectBinaryDir;
  std::string ProjectSourceDir;
  std::string TargetName;

  std::string OldMocDefinitionsStr;

  std::string OutMocCppFilename;
  std::list<std::string> MocIncludes;
  std::list<std::string> MocDefinitions;

  bool Verbose;
  bool ColorOutput;
  bool RunMocFailed;
  bool GenerateAll;

};

#endif
