/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmMakefile_h
#define cmMakefile_h

#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmNewLineStyle.h"
#include "cmExpandedCommandArgument.h"
#include "cmake.h"
#include "cmState.h"
#include "cmAlgorithms.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmSourceGroup.h"
#endif

#include <cmsys/auto_ptr.hxx>
#include <cmsys/RegularExpression.hxx>
#if defined(CMAKE_BUILD_WITH_CMAKE)
# ifdef CMake_HAVE_CXX11_UNORDERED_MAP
#  include <unordered_map>
# else
#  include <cmsys/hash_map.hxx>
# endif
#endif

#include <stack>

class cmFunctionBlocker;
class cmCommand;
class cmInstallGenerator;
class cmSourceFile;
class cmTest;
class cmTestGenerator;
class cmVariableWatch;
class cmake;
class cmMakefileCall;
class cmCMakePolicyCommand;
class cmGeneratorExpressionEvaluationFile;
class cmExportBuildFileGenerator;

/** \class cmMakefile
 * \brief Process the input CMakeLists.txt file.
 *
 * Process and store into memory the input CMakeLists.txt file.
 * Each CMakeLists.txt file is parsed and the commands found there
 * are added into the build process.
 */
class cmMakefile
{
public:
  /* Mark a variable as used */
  void MarkVariableAsUsed(const std::string& var);
  /* return true if a variable has been initialized */
  bool VariableInitialized(const std::string& ) const;

  /**
   * Construct an empty makefile.
   */
  cmMakefile(cmGlobalGenerator* globalGenerator,
             const cmState::Snapshot& snapshot);

  /**
   * Destructor.
   */
  ~cmMakefile();

  bool ReadListFile(const char* filename);

  bool ReadDependentFile(const char* filename, bool noPolicyScope = true);

  bool ProcessBuildsystemFile(const char* filename);

  /**
   * Add a function blocker to this makefile
   */
  void AddFunctionBlocker(cmFunctionBlocker* fb);

  /// @return whether we are processing the top CMakeLists.txt file.
  bool IsRootMakefile() const;

  /**
   * Remove the function blocker whose scope ends with the given command.
   * This returns ownership of the function blocker object.
   */
  cmsys::auto_ptr<cmFunctionBlocker>
  RemoveFunctionBlocker(cmFunctionBlocker* fb, const cmListFileFunction& lff);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  int TryCompile(const std::string& srcdir, const std::string& bindir,
                 const std::string& projectName, const std::string& targetName,
                 bool fast,
                 const std::vector<std::string> *cmakeArgs,
                 std::string& output);

  bool GetIsSourceFileTryCompile() const;

  /**
   * Help enforce global target name uniqueness.
   */
  bool EnforceUniqueName(std::string const& name, std::string& msg,
                         bool isCustom = false) const;

  /**
   * Perform FinalPass, Library dependency analysis etc before output of the
   * makefile.
   */
  void ConfigureFinalPass();

  /**
   * run the final pass on all commands.
   */
  void FinalPass();

  /** Add a custom command to the build.  */
  void AddCustomCommandToTarget(const std::string& target,
                                const std::vector<std::string>& byproducts,
                                const std::vector<std::string>& depends,
                                const cmCustomCommandLines& commandLines,
                                cmTarget::CustomCommandType type,
                                const char* comment, const char* workingDir,
                                bool escapeOldStyle = true,
                                bool uses_terminal = false);
  cmSourceFile* AddCustomCommandToOutput(
    const std::vector<std::string>& outputs,
    const std::vector<std::string>& byproducts,
    const std::vector<std::string>& depends,
    const std::string& main_dependency,
    const cmCustomCommandLines& commandLines,
    const char* comment, const char* workingDir,
    bool replace = false,
    bool escapeOldStyle = true,
    bool uses_terminal = false);
  cmSourceFile* AddCustomCommandToOutput(
    const std::string& output,
    const std::vector<std::string>& depends,
    const std::string& main_dependency,
    const cmCustomCommandLines& commandLines,
    const char* comment, const char* workingDir,
    bool replace = false,
    bool escapeOldStyle = true,
    bool uses_terminal = false);
  void AddCustomCommandOldStyle(const std::string& target,
                                const std::vector<std::string>& outputs,
                                const std::vector<std::string>& depends,
                                const std::string& source,
                                const cmCustomCommandLines& commandLines,
                                const char* comment);

  /**
   * Add a define flag to the build.
   */
  void AddDefineFlag(const char* definition);
  void RemoveDefineFlag(const char* definition);
  void AddCompileOption(const char* option);

  /** Create a new imported target with the name and type given.  */
  cmTarget* AddImportedTarget(const std::string& name,
                              cmState::TargetType type,
                              bool global);

  cmTarget* AddNewTarget(cmState::TargetType type, const std::string& name);

  /**
   * Add an executable to the build.
   */
  cmTarget* AddExecutable(const char *exename,
                          const std::vector<std::string> &srcs,
                          bool excludeFromAll = false);

  /**
   * Add a utility to the build.  A utiltity target is a command that
   * is run every time the target is built.
   */
  cmTarget* AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              const std::vector<std::string>& depends,
                              const char* workingDirectory,
                              const char* command,
                              const char* arg1=0,
                              const char* arg2=0,
                              const char* arg3=0,
                              const char* arg4=0);
  cmTarget* AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle = true,
                              const char* comment = 0,
                              bool uses_terminal = false);
  cmTarget* AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& byproducts,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle = true,
                              const char* comment = 0,
                              bool uses_terminal = false);

  /**
   * Add a link library to the build.
   */
  void AddLinkLibrary(const std::string&);
  void AddLinkLibrary(const std::string&, cmTargetLinkLibraryType type);
  void AddLinkLibraryForTarget(const std::string& tgt, const std::string&,
                               cmTargetLinkLibraryType type);
  void AddLinkDirectoryForTarget(const std::string& tgt, const std::string& d);

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const std::string& fullSrcDir,
                       const std::string& fullBinDir,
                       bool excludeFromAll,
                       bool immediate);

  void Configure();

  /**
   * Configure a subdirectory
   */
  void ConfigureSubDirectory(cmMakefile* mf);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectories(const std::vector<std::string> &incs,
                             bool before = false);

  /**
   * Add a variable definition to the build. This variable
   * can be used in CMake to refer to lists, directories, etc.
   */
  void AddDefinition(const std::string& name, const char* value);
  ///! Add a definition to this makefile and the global cmake cache.
  void AddCacheDefinition(const std::string& name, const char* value,
                          const char* doc,
                          cmState::CacheEntryType type,
                          bool force = false);

  /**
   * Add bool variable definition to the build.
   */
  void AddDefinition(const std::string& name, bool);

  /**
   * Remove a variable definition from the build.  This is not valid
   * for cache entries, and will only affect the current makefile.
   */
  void RemoveDefinition(const std::string& name);
  ///! Remove a definition from the cache.
  void RemoveCacheDefinition(const std::string& name);

  /**
   * Specify the name of the project for this build.
   */
  void SetProjectName(std::string const& name);

  /** Get the configurations to be generated.  */
  std::string GetConfigurations(std::vector<std::string>& configs,
                                bool single = true) const;

  /**
   * Set the name of the library.
   */
  cmTarget* AddLibrary(const std::string& libname, cmState::TargetType type,
                  const std::vector<std::string> &srcs,
                  bool excludeFromAll = false);
  void AddAlias(const std::string& libname, const std::string& tgt);

#if defined(CMAKE_BUILD_WITH_CMAKE)
  /**
   * Add a root source group for consideration when adding a new source.
   */
  void AddSourceGroup(const std::string& name, const char* regex=0);

  /**
   * Add a source group for consideration when adding a new source.
   * name is tokenized.
   */
  void AddSourceGroup(const std::vector<std::string>& name,
                      const char* regex=0);

#endif

  //@{
  /**
     * Set, Push, Pop policy values for CMake.
     */
  bool SetPolicy(cmPolicies::PolicyID id, cmPolicies::PolicyStatus status);
  bool SetPolicy(const char *id, cmPolicies::PolicyStatus status);
  cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID id) const;
  bool SetPolicyVersion(const char *version);
  void RecordPolicies(cmPolicies::PolicyMap& pm);
  //@}

  /** Helper class to push and pop policies automatically.  */
  class PolicyPushPop
  {
  public:
    PolicyPushPop(cmMakefile* m);
    ~PolicyPushPop();
  private:
    cmMakefile* Makefile;
  };
  friend class PolicyPushPop;

  /**
   * Determine if the given context, name pair has already been reported
   * in context of CMP0054.
   */
  bool HasCMP0054AlreadyBeenReported(const cmListFileContext &context) const;

  bool IgnoreErrorsCMP0061() const;

  const char* GetHomeDirectory() const;
  const char* GetHomeOutputDirectory() const;

  /**
   * Set CMAKE_SCRIPT_MODE_FILE variable when running a -P script.
   */
  void SetScriptModeFile(const char* scriptfile);

  /**
   * Set CMAKE_ARGC, CMAKE_ARGV0 ... variables.
   */
  void SetArgcArgv(const std::vector<std::string>& args);

  const char* GetCurrentSourceDirectory() const;
  const char* GetCurrentBinaryDirectory() const;

  //@}

  /**
   * Set a regular expression that include files must match
   * in order to be considered as part of the depend information.
   */
  void SetIncludeRegularExpression(const char* regex)
    {
      this->SetProperty("INCLUDE_REGULAR_EXPRESSION", regex);
    }
  const char* GetIncludeRegularExpression() const
    {
      return this->GetProperty("INCLUDE_REGULAR_EXPRESSION");
    }

  /**
   * Set a regular expression that include files that are not found
   * must match in order to be considered a problem.
   */
  void SetComplainRegularExpression(const std::string& regex)
    {
      this->ComplainFileRegularExpression = regex;
    }
  const char* GetComplainRegularExpression() const
    {
      return this->ComplainFileRegularExpression.c_str();
    }

  /**
   * Get the list of targets
   */
  cmTargets &GetTargets() { return this->Targets; }
  /**
   * Get the list of targets, const version
   */
  const cmTargets &GetTargets() const { return this->Targets; }
  const std::vector<cmTarget*> &GetOwnedImportedTargets() const
    {
      return this->ImportedTargetsOwned;
    }
  std::vector<cmTarget*> GetImportedTargets() const;

  cmTarget* FindLocalNonAliasTarget(const std::string& name) const;

  /** Find a target to use in place of the given name.  The target
      returned may be imported or built within the project.  */
  cmTarget* FindTargetToUse(const std::string& name,
                            bool excludeAliases = false) const;
  bool IsAlias(const std::string& name) const;

  std::map<std::string, std::string> GetAliasTargets() const
  {
    return this->AliasTargets;
  }

  /**
   * Mark include directories as system directories.
   */
  void AddSystemIncludeDirectories(const std::set<std::string> &incs);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmSourceFile* GetSource(const std::string& sourceName) const;

  /** Create the source file and return it. generated
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* CreateSource(const std::string& sourceName,
                             bool generated = false);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then create the source file and return it. generated
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* GetOrCreateSource(const std::string& sourceName,
                                  bool generated = false);

  /**
   * Given a variable name, return its value (as a string).
   * If the variable is not found in this makefile instance, the
   * cache is then queried.
   */
  const char* GetDefinition(const std::string&) const;
  const char* GetSafeDefinition(const std::string&) const;
  const char* GetRequiredDefinition(const std::string& name) const;
  bool IsDefinitionSet(const std::string&) const;
  /**
   * Get the list of all variables in the current space. If argument
   * cacheonly is specified and is greater than 0, then only cache
   * variables will be listed.
   */
  std::vector<std::string> GetDefinitions() const;

  /**
   * Test a boolean variable to see if it is true or false.
   * If the variable is not found in this makefile instance, the
   * cache is then queried.
   * Returns false if no entry defined.
   */
  bool IsOn(const std::string& name) const;
  bool IsSet(const std::string& name) const;

  /** Return whether the target platform is 64-bit.  */
  bool PlatformIs64Bit() const;

  /** Return whether the target platform is Apple iOS.  */
  bool PlatformIsAppleIos() const;

  /** Retrieve soname flag for the specified language if supported */
  const char* GetSONameFlag(const std::string& language) const;

  /**
   * Get a list of preprocessor define flags.
   */
  const char* GetDefineFlags() const
    {return this->DefineFlags.c_str();}

  /**
   * Make sure CMake can write this file
   */
  bool CanIWriteThisFile(const char* fileName) const;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  /**
   * Get the vector source groups.
   */
  const std::vector<cmSourceGroup>& GetSourceGroups() const
    { return this->SourceGroups; }

  /**
   * Get the source group
   */
  cmSourceGroup* GetSourceGroup(const std::vector<std::string>&name) const;
#endif

  /**
   * Get the vector of list files on which this makefile depends
   */
  const std::vector<std::string>& GetListFiles() const
    { return this->ListFiles; }
  ///! When the file changes cmake will be re-run from the build system.
  void AddCMakeDependFile(const std::string& file)
    { this->ListFiles.push_back(file);}
  void AddCMakeDependFilesFromUser();

  std::string FormatListFileStack() const;

  /**
   * Get the current context backtrace.
   */
  cmListFileBacktrace GetBacktrace() const;
  cmListFileBacktrace GetBacktrace(cmCommandContext const& lfc) const;
  cmListFileContext GetExecutionContext() const;

  /**
   * Get the vector of  files created by this makefile
   */
  const std::vector<std::string>& GetOutputFiles() const
    { return this->OutputFiles; }
  void AddCMakeOutputFile(const std::string& file)
    { this->OutputFiles.push_back(file);}

  /**
   * Expand all defined variables in the string.
   * Defined variables come from the this->Definitions map.
   * They are expanded with ${var} where var is the
   * entry in the this->Definitions map.  Also \@var\@ is
   * expanded to match autoconf style expansions.
   */
  const char *ExpandVariablesInString(std::string& source) const;
  const char *ExpandVariablesInString(std::string& source, bool escapeQuotes,
                                      bool noEscapes,
                                      bool atOnly = false,
                                      const char* filename = 0,
                                      long line = -1,
                                      bool removeEmpty = false,
                                      bool replaceAt = false) const;

  /**
   * Remove any remaining variables in the string. Anything with ${var} or
   * \@var\@ will be removed.
   */
  void RemoveVariablesInString(std::string& source,
                               bool atOnly = false) const;

  /**
   * Expand variables in the makefiles ivars such as link directories etc
   */
  void ExpandVariablesCMP0019();

  /**
   * Replace variables and #cmakedefine lines in the given string.
   * See cmConfigureFileCommand for details.
   */
  void ConfigureString(const std::string& input, std::string& output,
                       bool atOnly, bool escapeQuotes) const;

  /**
   * Copy file but change lines acording to ConfigureString
   */
  int ConfigureFile(const char* infile, const char* outfile,
                    bool copyonly, bool atOnly, bool escapeQuotes,
                    const cmNewLineStyle& = cmNewLineStyle());


#if defined(CMAKE_BUILD_WITH_CMAKE)
  /**
   * find what source group this source is in
   */
  cmSourceGroup* FindSourceGroup(const char* source,
                                 std::vector<cmSourceGroup> &groups) const;
#endif

  /**
   * Print a command's invocation
   */
  void PrintCommandTrace(const cmListFileFunction& lff) const;

  /**
   * Execute a single CMake command.  Returns true if the command
   * succeeded or false if it failed.
   */
  bool ExecuteCommand(const cmListFileFunction& lff,
                      cmExecutionStatus &status);

  ///! Enable support for named language, if nil then all languages are
  ///enabled.
  void EnableLanguage(std::vector<std::string>const& languages, bool optional);

  cmState *GetState() const;

  /**
   * Get the variable watch. This is used to determine when certain variables
   * are accessed.
   */
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* GetVariableWatch() const;
#endif

  ///! Display progress or status message.
  void DisplayStatus(const char*, float) const;

  /**
   * Expand the given list file arguments into the full set after
   * variable replacement and list expansion.
   */
  bool ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                       std::vector<std::string>& outArgs,
                       const char* filename = 0) const;

  bool ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                       std::vector<cmExpandedCommandArgument>& outArgs,
                       const char* filename = 0) const;

  /**
   * Get the instance
   */
  cmake *GetCMakeInstance() const;
  cmGlobalGenerator* GetGlobalGenerator() const;

  /**
   * Get all the source files this makefile knows about
   */
  const std::vector<cmSourceFile*> &GetSourceFiles() const
    {return this->SourceFiles;}
  std::vector<cmSourceFile*> &GetSourceFiles() {return this->SourceFiles;}

  /**
   * Is there a source file that has the provided source file as an output?
   * if so then return it
   */
  cmSourceFile *GetSourceFileWithOutput(const std::string& outName) const;

  ///! Add a new cmTest to the list of tests for this makefile.
  cmTest* CreateTest(const std::string& testName);

  /** Get a cmTest pointer for a given test name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmTest* GetTest(const std::string& testName) const;

  /**
   * Return a location of a file in cmake or custom modules directory
   */
  std::string GetModulesFile(const char* name) const;

  ///! Set/Get a property of this directory
  void SetProperty(const std::string& prop, const char *value);
  void AppendProperty(const std::string& prop, const char *value,
                      bool asString=false);
  const char *GetProperty(const std::string& prop) const;
  const char *GetProperty(const std::string& prop, bool chain) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  std::vector<std::string> GetPropertyKeys() const;

  ///! Initialize a makefile from its parent
  void InitializeFromParent(cmMakefile* parent);

  void AddInstallGenerator(cmInstallGenerator* g)
    { if(g) this->InstallGenerators.push_back(g); }
  std::vector<cmInstallGenerator*>& GetInstallGenerators()
    { return this->InstallGenerators; }

  void AddTestGenerator(cmTestGenerator* g)
    { if(g) this->TestGenerators.push_back(g); }
  const std::vector<cmTestGenerator*>& GetTestGenerators() const
    { return this->TestGenerators; }

  class FunctionPushPop
  {
  public:
    FunctionPushPop(cmMakefile* mf, std::string const& fileName,
                    cmPolicies::PolicyMap const& pm);
    ~FunctionPushPop();

    void Quiet() { this->ReportError = false; }
  private:
    cmMakefile* Makefile;
    bool ReportError;
  };

  class MacroPushPop
  {
  public:
    MacroPushPop(cmMakefile* mf, std::string const& fileName,
                 cmPolicies::PolicyMap const& pm);
    ~MacroPushPop();

    void Quiet() { this->ReportError = false; }
  private:
    cmMakefile* Makefile;
    bool ReportError;
  };

  void PushFunctionScope(std::string const& fileName,
                         cmPolicies::PolicyMap const& pm);
  void PopFunctionScope(bool reportError);
  void PushMacroScope(std::string const& fileName,
                      cmPolicies::PolicyMap const& pm);
  void PopMacroScope(bool reportError);
  void PushScope();
  void PopScope();
  void RaiseScope(const std::string& var, const char *value);

  // push and pop loop scopes
  void PushLoopBlockBarrier();
  void PopLoopBlockBarrier();

  /** Helper class to push and pop scopes automatically.  */
  class ScopePushPop
  {
  public:
    ScopePushPop(cmMakefile* m): Makefile(m) { this->Makefile->PushScope(); }
    ~ScopePushPop() { this->Makefile->PopScope(); }
  private:
    cmMakefile* Makefile;
  };

  void IssueMessage(cmake::MessageType t,
                    std::string const& text,
                    bool force = false) const;

  /** Set whether or not to report a CMP0000 violation.  */
  void SetCheckCMP0000(bool b) { this->CheckCMP0000 = b; }

  cmStringRange GetIncludeDirectoriesEntries() const;
  cmBacktraceRange GetIncludeDirectoriesBacktraces() const;
  cmStringRange GetCompileOptionsEntries() const;
  cmBacktraceRange GetCompileOptionsBacktraces() const;
  cmStringRange GetCompileDefinitionsEntries() const;
  cmBacktraceRange GetCompileDefinitionsBacktraces() const;

  void AddQtUiFileWithOptions(cmSourceFile *sf);
  std::vector<cmSourceFile*> GetQtUiFilesWithOptions() const;

  std::set<std::string> const & GetSystemIncludeDirectories() const
    { return this->SystemIncludeDirectories; }

  bool PolicyOptionalWarningEnabled(std::string const& var);

  bool AddRequiredTargetFeature(cmTarget *target,
                                const std::string& feature,
                                std::string *error = 0) const;

  bool CompileFeatureKnown(cmTarget const* target, const std::string& feature,
                           std::string& lang, std::string *error) const;

  const char* CompileFeaturesAvailable(const std::string& lang,
                                       std::string *error) const;

  bool HaveStandardAvailable(cmTarget const* target, std::string const& lang,
                            const std::string& feature) const;

  bool IsLaterStandard(std::string const& lang,
                       std::string const& lhs,
                       std::string const& rhs);

  void PushLoopBlock();
  void PopLoopBlock();
  bool IsLoopBlock() const;

  void ClearMatches();
  void StoreMatches(cmsys::RegularExpression& re);

  cmState::Snapshot GetStateSnapshot() const;

  const char* GetDefineFlagsCMP0059() const;

  std::string GetExecutionFilePath() const;

  void EnforceDirectoryLevelRules() const;

  void AddEvaluationFile(const std::string &inputFile,
                  cmsys::auto_ptr<cmCompiledGeneratorExpression> outputName,
                  cmsys::auto_ptr<cmCompiledGeneratorExpression> condition,
                  bool inputIsContent);
  std::vector<cmGeneratorExpressionEvaluationFile*> GetEvaluationFiles() const;

  std::vector<cmExportBuildFileGenerator*>
  GetExportBuildFileGenerators() const;
  void RemoveExportBuildFileGeneratorCMP0024(cmExportBuildFileGenerator* gen);
  void AddExportBuildFileGenerator(cmExportBuildFileGenerator* gen);

protected:
  // add link libraries and directories to the target
  void AddGlobalLinkInformation(const std::string& name, cmTarget& target);

  // Check for a an unused variable
  void LogUnused(const char* reason, const std::string& name) const;

  mutable std::set<cmListFileContext> CMP0054ReportedIds;

  // libraries, classes, and executables
  mutable cmTargets Targets;
#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
  typedef std::unordered_map<std::string, cmTarget*> TargetMap;
#else
  typedef cmsys::hash_map<std::string, cmTarget*> TargetMap;
#endif
#else
  typedef std::map<std::string, cmTarget*> TargetMap;
#endif
  std::map<std::string, std::string> AliasTargets;
  std::vector<cmSourceFile*> SourceFiles;

  // Tests
  std::map<std::string, cmTest*> Tests;

  // The link-library paths.  Order matters, use std::vector (not std::set).
  std::vector<std::string> LinkDirectories;

  // The set of include directories that are marked as system include
  // directories.
  std::set<std::string> SystemIncludeDirectories;

  std::vector<std::string> ListFiles;
  std::vector<std::string> OutputFiles;

  cmTarget::LinkLibraryVectorType LinkLibraries;

  std::vector<cmInstallGenerator*> InstallGenerators;
  std::vector<cmTestGenerator*> TestGenerators;

  std::string ComplainFileRegularExpression;
  std::string DefineFlags;

  // Track the value of the computed DEFINITIONS property.
  void AddDefineFlag(const char*, std::string&);
  void RemoveDefineFlag(const char*, std::string::size_type, std::string&);
  std::string DefineFlagsOrig;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  std::vector<cmSourceGroup> SourceGroups;
#endif

  std::vector<cmCommand*> FinalPassCommands;
  cmGlobalGenerator* GlobalGenerator;
  bool IsFunctionBlocked(const cmListFileFunction& lff,
                         cmExecutionStatus &status);

private:
  cmMakefile(const cmMakefile& mf);
  cmMakefile& operator=(const cmMakefile& mf);

  cmState::Snapshot StateSnapshot;

  void ReadListFile(cmListFile const& listFile,
                    const std::string& filenametoread);

  bool ParseDefineFlag(std::string const& definition, bool remove);

  bool EnforceUniqueDir(const std::string& srcPath,
                        const std::string& binPath) const;

  typedef std::vector<cmFunctionBlocker*> FunctionBlockersType;
  FunctionBlockersType FunctionBlockers;
  std::vector<FunctionBlockersType::size_type> FunctionBlockerBarriers;
  void PushFunctionBlockerBarrier();
  void PopFunctionBlockerBarrier(bool reportError = true);

  std::stack<int> LoopBlockCounter;

  mutable cmsys::RegularExpression cmDefineRegex;
  mutable cmsys::RegularExpression cmDefine01Regex;
  mutable cmsys::RegularExpression cmAtVarRegex;
  mutable cmsys::RegularExpression cmNamedCurly;

  std::vector<cmMakefile*> UnConfiguredDirectories;
  std::vector<cmExportBuildFileGenerator*> ExportBuildFileGenerators;

  std::vector<cmGeneratorExpressionEvaluationFile*> EvaluationFiles;

  std::vector<cmCommandContext const*> ContextStack;
  std::vector<cmExecutionStatus*> ExecutionStatusStack;
  friend class cmMakefileCall;
  friend class cmParseFileScope;

  std::vector<cmTarget*> ImportedTargetsOwned;
  TargetMap ImportedTargets;

  // Internal policy stack management.
  void PushPolicy(bool weak = false,
                  cmPolicies::PolicyMap const& pm = cmPolicies::PolicyMap());
  void PopPolicy();
  void PopSnapshot(bool reportError = true);
  friend class cmCMakePolicyCommand;
  class IncludeScope;
  friend class IncludeScope;
  class ListFileScope;
  friend class ListFileScope;
  class BuildsystemFileScope;
  friend class BuildsystemFileScope;


  // CMP0053 == old
  cmake::MessageType ExpandVariablesInStringOld(
                                  std::string& errorstr,
                                  std::string& source,
                                  bool escapeQuotes,
                                  bool noEscapes,
                                  bool atOnly,
                                  const char* filename,
                                  long line,
                                  bool removeEmpty,
                                  bool replaceAt) const;
  // CMP0053 == new
  cmake::MessageType ExpandVariablesInStringNew(
                                  std::string& errorstr,
                                  std::string& source,
                                  bool escapeQuotes,
                                  bool noEscapes,
                                  bool atOnly,
                                  const char* filename,
                                  long line,
                                  bool removeEmpty,
                                  bool replaceAt) const;
  /**
   * Old version of GetSourceFileWithOutput(const std::string&) kept for
   * backward-compatibility. It implements a linear search and support
   * relative file paths. It is used as a fall back by
   * GetSourceFileWithOutput(const std::string&).
   */
  cmSourceFile *LinearGetSourceFileWithOutput(const std::string& cname) const;

  // A map for fast output to input look up.
#if defined(CMAKE_BUILD_WITH_CMAKE)
#ifdef CMake_HAVE_CXX11_UNORDERED_MAP
  typedef std::unordered_map<std::string, cmSourceFile*> OutputToSourceMap;
#else
  typedef cmsys::hash_map<std::string, cmSourceFile*> OutputToSourceMap;
#endif
#else
  typedef std::map<std::string, cmSourceFile*> OutputToSourceMap;
#endif
  OutputToSourceMap OutputToSource;

  void UpdateOutputToSourceMap(std::vector<std::string> const& outputs,
                               cmSourceFile* source);
  void UpdateOutputToSourceMap(std::string const& output,
                               cmSourceFile* source);

  std::vector<cmSourceFile*> QtUiFilesWithOptions;

  bool AddRequiredTargetCFeature(cmTarget *target,
                                 const std::string& feature) const;

  bool AddRequiredTargetCxxFeature(cmTarget *target,
                                   const std::string& feature) const;

  void CheckNeededCLanguage(const std::string& feature, bool& needC90,
                            bool& needC99, bool& needC11) const;
  void CheckNeededCxxLanguage(const std::string& feature, bool& needCxx98,
                              bool& needCxx11, bool& needCxx14) const;

  bool HaveCStandardAvailable(cmTarget const* target,
                             const std::string& feature) const;
  bool HaveCxxStandardAvailable(cmTarget const* target,
                               const std::string& feature) const;

  void CheckForUnusedVariables() const;

  // Unused variable flags
  bool WarnUnused;
  bool CheckSystemVars;
  bool CheckCMP0000;
  bool IsSourceFileTryCompile;
  mutable bool SuppressWatches;
};

//----------------------------------------------------------------------------
// Helper class to make sure the call stack is valid.
class cmMakefileCall
{
public:
  cmMakefileCall(cmMakefile* mf,
                 cmCommandContext const& lfc,
                 cmExecutionStatus& status);
  ~cmMakefileCall();
private:
  cmMakefile* Makefile;
};

#endif
