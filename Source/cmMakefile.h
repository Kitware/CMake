/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cm_sys_stat.h"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandTypes.h"
#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmNewLineStyle.h"
#include "cmPolicies.h"
#include "cmSourceFileLocationKind.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmValue.h"

// IWYU does not see that 'std::unordered_map<std::string, cmTarget>'
// will not compile without the complete type.
#include "cmTarget.h" // IWYU pragma: keep

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmSourceGroup.h"
#endif

class cmCompiledGeneratorExpression;
class cmCustomCommandLines;
class cmExecutionStatus;
class cmExpandedCommandArgument;
class cmExportBuildFileGenerator;
class cmFunctionBlocker;
class cmGeneratorExpressionEvaluationFile;
class cmGlobalGenerator;
class cmInstallGenerator;
class cmLocalGenerator;
class cmMessenger;
class cmSourceFile;
class cmState;
class cmTest;
class cmTestGenerator;
class cmVariableWatch;
class cmake;

/** A type-safe wrapper for a string representing a directory id.  */
class cmDirectoryId
{
public:
  cmDirectoryId(std::string s);
  std::string String;
};

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
  bool VariableInitialized(const std::string&) const;

  /**
   * Construct an empty makefile.
   */
  cmMakefile(cmGlobalGenerator* globalGenerator,
             const cmStateSnapshot& snapshot);

  /**
   * Destructor.
   */
  ~cmMakefile();

  cmMakefile(cmMakefile const&) = delete;
  cmMakefile& operator=(cmMakefile const&) = delete;

  cmDirectoryId GetDirectoryId() const;

  bool ReadListFile(const std::string& filename);

  bool ReadListFileAsString(const std::string& content,
                            const std::string& virtualFileName);

  bool ReadDependentFile(const std::string& filename,
                         bool noPolicyScope = true);

  /**
   * Add a function blocker to this makefile
   */
  void AddFunctionBlocker(std::unique_ptr<cmFunctionBlocker> fb);

  /// @return whether we are processing the top CMakeLists.txt file.
  bool IsRootMakefile() const;

  /**
   * Remove the function blocker whose scope ends with the given command.
   * This returns ownership of the function blocker object.
   */
  std::unique_ptr<cmFunctionBlocker> RemoveFunctionBlocker();

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
  int TryCompile(const std::string& srcdir, const std::string& bindir,
                 const std::string& projectName, const std::string& targetName,
                 bool fast, int jobs,
                 const std::vector<std::string>* cmakeArgs,
                 std::string& output);

  bool GetIsSourceFileTryCompile() const;

  /**
   * Help enforce global target name uniqueness.
   */
  bool EnforceUniqueName(std::string const& name, std::string& msg,
                         bool isCustom = false) const;

  class GeneratorAction
  {
    using ActionT =
      std::function<void(cmLocalGenerator&, const cmListFileBacktrace&)>;
    using CCActionT =
      std::function<void(cmLocalGenerator&, const cmListFileBacktrace&,
                         std::unique_ptr<cmCustomCommand> cc)>;

  public:
    GeneratorAction(ActionT&& action)
      : Action(std::move(action))
    {
    }

    GeneratorAction(std::unique_ptr<cmCustomCommand> tcc, CCActionT&& action)
      : CCAction(std::move(action))
      , cc(std::move(tcc))
    {
    }

    void operator()(cmLocalGenerator& lg, const cmListFileBacktrace& lfbt);

  private:
    ActionT Action;

    // FIXME: Use std::variant
    CCActionT CCAction;
    std::unique_ptr<cmCustomCommand> cc;
  };

  /**
   * Register an action that is executed during Generate
   */
  void AddGeneratorAction(GeneratorAction&& action);

  /// Helper to insert the constructor GeneratorAction(args...)
  template <class... Args>
  void AddGeneratorAction(Args&&... args)
  {
    AddGeneratorAction(GeneratorAction(std::move(args)...));
  }

  /**
   * Perform generate actions, Library dependency analysis etc before output of
   * the makefile.
   */
  void Generate(cmLocalGenerator& lg);

  /**
   * Get the target for PRE_BUILD, PRE_LINK, or POST_BUILD commands.
   */
  cmTarget* GetCustomCommandTarget(const std::string& target,
                                   cmObjectLibraryCommands objLibCommands,
                                   const cmListFileBacktrace& lfbt) const;

  /**
   * Dispatch adding a custom PRE_BUILD, PRE_LINK, or POST_BUILD command to a
   * target.
   */
  cmTarget* AddCustomCommandToTarget(const std::string& target,
                                     cmCustomCommandType type,
                                     std::unique_ptr<cmCustomCommand> cc);

  /**
   * Called for each file with custom command.
   */
  using CommandSourceCallback = std::function<void(cmSourceFile*)>;

  /**
   * Dispatch adding a custom command to a source file.
   */
  void AddCustomCommandToOutput(
    std::unique_ptr<cmCustomCommand> cc,
    const CommandSourceCallback& callback = nullptr, bool replace = false);
  void AddCustomCommandOldStyle(const std::string& target,
                                const std::vector<std::string>& outputs,
                                const std::vector<std::string>& depends,
                                const std::string& source,
                                const cmCustomCommandLines& commandLines,
                                const char* comment);
  void AppendCustomCommandToOutput(
    const std::string& output, const std::vector<std::string>& depends,
    const cmImplicitDependsList& implicit_depends,
    const cmCustomCommandLines& commandLines);

  /**
   * Add a define flag to the build.
   */
  void AddDefineFlag(std::string const& definition);
  void RemoveDefineFlag(std::string const& definition);
  void AddCompileDefinition(std::string const& definition);
  void AddCompileOption(std::string const& option);
  void AddLinkOption(std::string const& option);
  void AddLinkDirectory(std::string const& directory, bool before = false);

  /** Create a new imported target with the name and type given.  */
  cmTarget* AddImportedTarget(const std::string& name,
                              cmStateEnums::TargetType type, bool global);

  std::pair<cmTarget&, bool> CreateNewTarget(
    const std::string& name, cmStateEnums::TargetType type,
    cmTarget::PerConfig perConfig = cmTarget::PerConfig::Yes);

  cmTarget* AddNewTarget(cmStateEnums::TargetType type,
                         const std::string& name);

  /** Create a target instance for the utility.  */
  cmTarget* AddNewUtilityTarget(const std::string& utilityName,
                                bool excludeFromAll);

  /**
   * Add an executable to the build.
   */
  cmTarget* AddExecutable(const std::string& exename,
                          const std::vector<std::string>& srcs,
                          bool excludeFromAll = false);

  /**
   * Dispatch adding a utility to the build.  A utility target is a command
   * that is run every time the target is built.
   */
  cmTarget* AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              std::unique_ptr<cmCustomCommand> cc);

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const std::string& fullSrcDir,
                       const std::string& fullBinDir, bool excludeFromAll,
                       bool immediate);

  void Configure();

  /**
   * Configure a subdirectory
   */
  void ConfigureSubDirectory(cmMakefile* mf);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectories(const std::vector<std::string>& incs,
                             bool before = false);

  /**
   * Add a variable definition to the build. This variable
   * can be used in CMake to refer to lists, directories, etc.
   */
  void AddDefinition(const std::string& name, cm::string_view value);
  void AddDefinition(const std::string& name, cmValue value)
  {
    this->AddDefinition(name, *value);
  }
  /**
   * Add bool variable definition to the build.
   */
  void AddDefinitionBool(const std::string& name, bool);
  //! Add a definition to this makefile and the global cmake cache.
  void AddCacheDefinition(const std::string& name, const char* value,
                          const char* doc, cmStateEnums::CacheEntryType type,
                          bool force = false);
  void AddCacheDefinition(const std::string& name, const std::string& value,
                          const char* doc, cmStateEnums::CacheEntryType type,
                          bool force = false)
  {
    this->AddCacheDefinition(name, value.c_str(), doc, type, force);
  }

  /**
   * Remove a variable definition from the build.  This is not valid
   * for cache entries, and will only affect the current makefile.
   */
  void RemoveDefinition(const std::string& name);
  //! Remove a definition from the cache.
  void RemoveCacheDefinition(const std::string& name) const;

  /**
   * Specify the name of the project for this build.
   */
  void SetProjectName(std::string const& name);

  void InitCMAKE_CONFIGURATION_TYPES(std::string const& genDefault);

  /* Get the default configuration */
  std::string GetDefaultConfiguration() const;

  enum GeneratorConfigQuery
  {
    IncludeEmptyConfig, // Include "" aka noconfig
    ExcludeEmptyConfig, // Exclude "" aka noconfig
    OnlyMultiConfig,
  };

  /** Get the configurations for dependency checking.  */
  std::vector<std::string> GetGeneratorConfigs(
    GeneratorConfigQuery mode) const;

  /**
   * Set the name of the library.
   */
  cmTarget* AddLibrary(const std::string& libname,
                       cmStateEnums::TargetType type,
                       const std::vector<std::string>& srcs,
                       bool excludeFromAll = false);
  void AddAlias(const std::string& libname, const std::string& tgt,
                bool globallyVisible = true);

  //@{
  /**
   * Set, Push, Pop policy values for CMake.
   */
  bool SetPolicy(cmPolicies::PolicyID id, cmPolicies::PolicyStatus status);
  bool SetPolicy(const char* id, cmPolicies::PolicyStatus status);
  cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID id,
                                           bool parent_scope = false) const;
  bool SetPolicyVersion(std::string const& version_min,
                        std::string const& version_max);
  void RecordPolicies(cmPolicies::PolicyMap& pm) const;
  //@}

  /** Helper class to push and pop policies automatically.  */
  class PolicyPushPop
  {
  public:
    PolicyPushPop(cmMakefile* m);
    ~PolicyPushPop();

    PolicyPushPop(const PolicyPushPop&) = delete;
    PolicyPushPop& operator=(const PolicyPushPop&) = delete;

  private:
    cmMakefile* Makefile;
  };
  friend class PolicyPushPop;

  /**
   * Determine if the given context, name pair has already been reported
   * in context of CMP0054.
   */
  bool HasCMP0054AlreadyBeenReported(const cmListFileContext& context) const;

  bool IgnoreErrorsCMP0061() const;

  std::string const& GetHomeDirectory() const;
  std::string const& GetHomeOutputDirectory() const;

  /**
   * Set CMAKE_SCRIPT_MODE_FILE variable when running a -P script.
   */
  void SetScriptModeFile(std::string const& scriptfile);

  /**
   * Set CMAKE_ARGC, CMAKE_ARGV0 ... variables.
   */
  void SetArgcArgv(const std::vector<std::string>& args);

  std::string const& GetCurrentSourceDirectory() const;
  std::string const& GetCurrentBinaryDirectory() const;

  //@}

  /**
   * Set a regular expression that include files must match
   * in order to be considered as part of the depend information.
   */
  void SetIncludeRegularExpression(const std::string& regex)
  {
    this->SetProperty("INCLUDE_REGULAR_EXPRESSION", regex.c_str());
  }
  const std::string& GetIncludeRegularExpression() const
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
  const std::string& GetComplainRegularExpression() const
  {
    return this->ComplainFileRegularExpression;
  }

  // -- List of targets
  using cmTargetMap = std::unordered_map<std::string, cmTarget>;
  /** Get the target map */
  cmTargetMap& GetTargets() { return this->Targets; }
  /** Get the target map - const version */
  cmTargetMap const& GetTargets() const { return this->Targets; }

  const std::vector<std::unique_ptr<cmTarget>>& GetOwnedImportedTargets() const
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
  void AddSystemIncludeDirectories(const std::set<std::string>& incs);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmSourceFile* GetSource(
    const std::string& sourceName,
    cmSourceFileLocationKind kind = cmSourceFileLocationKind::Ambiguous) const;

  /** Create the source file and return it. generated
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* CreateSource(
    const std::string& sourceName, bool generated = false,
    cmSourceFileLocationKind kind = cmSourceFileLocationKind::Ambiguous);

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then create the source file and return it. generated
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* GetOrCreateSource(
    const std::string& sourceName, bool generated = false,
    cmSourceFileLocationKind kind = cmSourceFileLocationKind::Ambiguous);

  /** Get a cmSourceFile pointer for a given source name and always mark the
   * file as generated, if the name is not found, then create the source file
   * and return it.
   */
  cmSourceFile* GetOrCreateGeneratedSource(const std::string& sourceName);

  void AddTargetObject(std::string const& tgtName, std::string const& objFile);

  /**
   * Given a variable name, return its value (as a string).
   * If the variable is not found in this makefile instance, the
   * cache is then queried.
   */
  cmValue GetDefinition(const std::string&) const;
  const std::string& GetSafeDefinition(const std::string&) const;
  const std::string& GetRequiredDefinition(const std::string& name) const;
  bool IsDefinitionSet(const std::string&) const;
  bool IsNormalDefinitionSet(const std::string&) const;
  bool GetDefExpandList(const std::string& name, std::vector<std::string>& out,
                        bool emptyArgs = false) const;
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

  /** Return whether the target platform is 32-bit. */
  bool PlatformIs32Bit() const;

  /** Return whether the target platform is 64-bit.  */
  bool PlatformIs64Bit() const;
  /** Return whether the target platform is x32.  */
  bool PlatformIsx32() const;

  /** Apple SDK Type */
  enum class AppleSDK
  {
    MacOS,
    IPhoneOS,
    IPhoneSimulator,
    AppleTVOS,
    AppleTVSimulator,
    WatchOS,
    WatchSimulator,
  };

  /** What SDK type points CMAKE_OSX_SYSROOT to? */
  AppleSDK GetAppleSDKType() const;

  /** Return whether the target platform is Apple iOS.  */
  bool PlatformIsAppleEmbedded() const;

  /** Retrieve soname flag for the specified language if supported */
  const char* GetSONameFlag(const std::string& language) const;

  /**
   * Get a list of preprocessor define flags.
   */
  std::string GetDefineFlags() const { return this->DefineFlags; }

  /**
   * Make sure CMake can write this file
   */
  bool CanIWriteThisFile(std::string const& fileName) const;

#if !defined(CMAKE_BOOTSTRAP)
  /**
   * Get the vector source groups.
   */
  const std::vector<cmSourceGroup>& GetSourceGroups() const
  {
    return this->SourceGroups;
  }

  /**
   * Get the source group
   */
  cmSourceGroup* GetSourceGroup(const std::vector<std::string>& name) const;

  /**
   * Add a root source group for consideration when adding a new source.
   */
  void AddSourceGroup(const std::string& name, const char* regex = nullptr);

  /**
   * Add a source group for consideration when adding a new source.
   * name is tokenized.
   */
  void AddSourceGroup(const std::vector<std::string>& name,
                      const char* regex = nullptr);

  /**
   * Get and existing or create a new source group.
   */
  cmSourceGroup* GetOrCreateSourceGroup(
    const std::vector<std::string>& folders);

  /**
   * Get and existing or create a new source group.
   * The name will be tokenized.
   */
  cmSourceGroup* GetOrCreateSourceGroup(const std::string& name);

  /**
   * find what source group this source is in
   */
  cmSourceGroup* FindSourceGroup(const std::string& source,
                                 std::vector<cmSourceGroup>& groups) const;
#endif

  /**
   * Get the vector of list files on which this makefile depends
   */
  const std::vector<std::string>& GetListFiles() const
  {
    return this->ListFiles;
  }
  //! When the file changes cmake will be re-run from the build system.
  void AddCMakeDependFile(const std::string& file)
  {
    this->ListFiles.push_back(file);
  }
  void AddCMakeDependFilesFromUser();

  std::string FormatListFileStack() const;

  /**
   * Get the current context backtrace.
   */
  cmListFileBacktrace GetBacktrace() const;

  /**
   * Get the vector of  files created by this makefile
   */
  const std::vector<std::string>& GetOutputFiles() const
  {
    return this->OutputFiles;
  }
  void AddCMakeOutputFile(const std::string& file)
  {
    this->OutputFiles.push_back(file);
  }

  /**
   * Expand all defined variables in the string.
   * Defined variables come from the this->Definitions map.
   * They are expanded with ${var} where var is the
   * entry in the this->Definitions map.  Also \@var\@ is
   * expanded to match autoconf style expansions.
   */
  const std::string& ExpandVariablesInString(std::string& source) const;
  const std::string& ExpandVariablesInString(
    std::string& source, bool escapeQuotes, bool noEscapes,
    bool atOnly = false, const char* filename = nullptr, long line = -1,
    bool removeEmpty = false, bool replaceAt = false) const;

  /**
   * Remove any remaining variables in the string. Anything with ${var} or
   * \@var\@ will be removed.
   */
  void RemoveVariablesInString(std::string& source, bool atOnly = false) const;

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
   * Copy file but change lines according to ConfigureString
   */
  int ConfigureFile(const std::string& infile, const std::string& outfile,
                    bool copyonly, bool atOnly, bool escapeQuotes,
                    mode_t permissions = 0, cmNewLineStyle = cmNewLineStyle());

  /**
   * Print a command's invocation
   */
  void PrintCommandTrace(cmListFileFunction const& lff,
                         cm::optional<std::string> const& deferId = {}) const;

  /**
   * Set a callback that is invoked whenever ExecuteCommand is called.
   */
  void OnExecuteCommand(std::function<void()> callback);

  /**
   * Execute a single CMake command.  Returns true if the command
   * succeeded or false if it failed.
   */
  bool ExecuteCommand(const cmListFileFunction& lff, cmExecutionStatus& status,
                      cm::optional<std::string> deferId = {});

  //! Enable support for named language, if nil then all languages are
  /// enabled.
  void EnableLanguage(std::vector<std::string> const& languages,
                      bool optional);

  cmState* GetState() const;

/**
 * Get the variable watch. This is used to determine when certain variables
 * are accessed.
 */
#ifndef CMAKE_BOOTSTRAP
  cmVariableWatch* GetVariableWatch() const;
#endif

  //! Display progress or status message.
  void DisplayStatus(const std::string&, float) const;

  /**
   * Expand the given list file arguments into the full set after
   * variable replacement and list expansion.
   */
  bool ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                       std::vector<std::string>& outArgs) const;
  bool ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                       std::vector<cmExpandedCommandArgument>& outArgs) const;

  /**
   * Get the instance
   */
  cmake* GetCMakeInstance() const;
  cmMessenger* GetMessenger() const;
  cmGlobalGenerator* GetGlobalGenerator() const;

  /**
   * Get all the source files this makefile knows about
   */
  const std::vector<std::unique_ptr<cmSourceFile>>& GetSourceFiles() const
  {
    return this->SourceFiles;
  }

  std::vector<cmTarget*> const& GetOrderedTargets() const
  {
    return this->OrderedTargets;
  }

  //! Add a new cmTest to the list of tests for this makefile.
  cmTest* CreateTest(const std::string& testName);

  /** Get a cmTest pointer for a given test name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmTest* GetTest(const std::string& testName) const;

  /**
   * Get all tests that run under the given configuration.
   */
  void GetTests(const std::string& config, std::vector<cmTest*>& tests) const;

  /**
   * Return a location of a file in cmake or custom modules directory
   */
  std::string GetModulesFile(const std::string& name) const
  {
    bool system;
    std::string debugBuffer;
    return this->GetModulesFile(name, system, false, debugBuffer);
  }

  /**
   * Return a location of a file in cmake or custom modules directory
   */
  std::string GetModulesFile(const std::string& name, bool& system) const
  {
    std::string debugBuffer;
    return this->GetModulesFile(name, system, false, debugBuffer);
  }

  std::string GetModulesFile(const std::string& name, bool& system, bool debug,
                             std::string& debugBuffer) const;

  //! Set/Get a property of this directory
  void SetProperty(const std::string& prop, const char* value);
  void SetProperty(const std::string& prop, cmValue value);
  void SetProperty(const std::string& prop, const std::string& value)
  {
    this->SetProperty(prop, cmValue(value));
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  cmValue GetProperty(const std::string& prop) const;
  cmValue GetProperty(const std::string& prop, bool chain) const;
  bool GetPropertyAsBool(const std::string& prop) const;
  std::vector<std::string> GetPropertyKeys() const;

  //! Initialize a makefile from its parent
  void InitializeFromParent(cmMakefile* parent);

  void AddInstallGenerator(std::unique_ptr<cmInstallGenerator> g);

  std::vector<std::unique_ptr<cmInstallGenerator>>& GetInstallGenerators()
  {
    return this->InstallGenerators;
  }
  const std::vector<std::unique_ptr<cmInstallGenerator>>&
  GetInstallGenerators() const
  {
    return this->InstallGenerators;
  }

  void AddTestGenerator(std::unique_ptr<cmTestGenerator> g);

  const std::vector<std::unique_ptr<cmTestGenerator>>& GetTestGenerators()
    const
  {
    return this->TestGenerators;
  }

  class FunctionPushPop
  {
  public:
    FunctionPushPop(cmMakefile* mf, std::string const& fileName,
                    cmPolicies::PolicyMap const& pm);
    ~FunctionPushPop();

    FunctionPushPop(const FunctionPushPop&) = delete;
    FunctionPushPop& operator=(const FunctionPushPop&) = delete;

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

    MacroPushPop(const MacroPushPop&) = delete;
    MacroPushPop& operator=(const MacroPushPop&) = delete;

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
  void RaiseScope(const std::string& var, const char* value);

  // push and pop loop scopes
  void PushLoopBlockBarrier();
  void PopLoopBlockBarrier();

  /** Helper class to push and pop scopes automatically.  */
  class ScopePushPop
  {
  public:
    ScopePushPop(cmMakefile* m)
      : Makefile(m)
    {
      this->Makefile->PushScope();
    }

    ~ScopePushPop() { this->Makefile->PopScope(); }

    ScopePushPop(ScopePushPop const&) = delete;
    ScopePushPop& operator=(ScopePushPop const&) = delete;

  private:
    cmMakefile* Makefile;
  };

  void IssueMessage(MessageType t, std::string const& text) const;

  /** Set whether or not to report a CMP0000 violation.  */
  void SetCheckCMP0000(bool b) { this->CheckCMP0000 = b; }

  bool CheckCMP0037(std::string const& targetName,
                    cmStateEnums::TargetType targetType) const;

  cmBTStringRange GetIncludeDirectoriesEntries() const;
  cmBTStringRange GetCompileOptionsEntries() const;
  cmBTStringRange GetCompileDefinitionsEntries() const;
  cmBTStringRange GetLinkOptionsEntries() const;
  cmBTStringRange GetLinkDirectoriesEntries() const;

  std::set<std::string> const& GetSystemIncludeDirectories() const
  {
    return this->SystemIncludeDirectories;
  }

  bool PolicyOptionalWarningEnabled(std::string const& var) const;

  void PushLoopBlock();
  void PopLoopBlock();
  bool IsLoopBlock() const;

  void ClearMatches();
  void StoreMatches(cmsys::RegularExpression& re);

  cmStateSnapshot GetStateSnapshot() const;

  const char* GetDefineFlagsCMP0059() const;

  void EnforceDirectoryLevelRules() const;

  void AddEvaluationFile(
    const std::string& inputFile, const std::string& targetName,
    std::unique_ptr<cmCompiledGeneratorExpression> outputName,
    std::unique_ptr<cmCompiledGeneratorExpression> condition,
    const std::string& newLineCharacter, mode_t permissions,
    bool inputIsContent);
  const std::vector<std::unique_ptr<cmGeneratorExpressionEvaluationFile>>&
  GetEvaluationFiles() const;

  std::vector<std::unique_ptr<cmExportBuildFileGenerator>> const&
  GetExportBuildFileGenerators() const;
  void RemoveExportBuildFileGeneratorCMP0024(cmExportBuildFileGenerator* gen);
  void AddExportBuildFileGenerator(
    std::unique_ptr<cmExportBuildFileGenerator> gen);

  // Maintain a stack of package roots to allow nested PACKAGE_ROOT_PATH
  // searches
  std::deque<std::vector<std::string>> FindPackageRootPathStack;

  class DebugFindPkgRAII
  {
    cmMakefile* Makefile;
    bool OldValue;

  public:
    DebugFindPkgRAII(cmMakefile* mf, std::string const& pkg);
    ~DebugFindPkgRAII();
  };

  bool GetDebugFindPkgMode() const;

  void MaybeWarnCMP0074(std::string const& pkg);
  void MaybeWarnUninitialized(std::string const& variable,
                              const char* sourceFilename) const;
  bool IsProjectFile(const char* filename) const;

  int GetRecursionDepth() const;
  void SetRecursionDepth(int recursionDepth);

  std::string NewDeferId() const;
  bool DeferCall(std::string id, std::string fileName, cmListFileFunction lff);
  bool DeferCancelCall(std::string const& id);
  cm::optional<std::string> DeferGetCallIds() const;
  cm::optional<std::string> DeferGetCall(std::string const& id) const;

protected:
  // add link libraries and directories to the target
  void AddGlobalLinkInformation(cmTarget& target);

  mutable std::set<cmListFileContext> CMP0054ReportedIds;

  // libraries, classes, and executables
  mutable cmTargetMap Targets;
  std::map<std::string, std::string> AliasTargets;

  std::vector<cmTarget*> OrderedTargets;

  std::vector<std::unique_ptr<cmSourceFile>> SourceFiles;

  // Because cmSourceFile names are compared in a fuzzy way (see
  // cmSourceFileLocation::Match()) we can't have a straight mapping from
  // filename to cmSourceFile.  To make lookups more efficient we store the
  // Name portion of the cmSourceFileLocation and then compare on the list of
  // cmSourceFiles that might match that name.  Note that on platforms which
  // have a case-insensitive filesystem we store the key in all lowercase.
  using SourceFileMap =
    std::unordered_map<std::string, std::vector<cmSourceFile*>>;
  SourceFileMap SourceFileSearchIndex;

  // For "Known" paths we can store a direct filename to cmSourceFile map
  std::unordered_map<std::string, cmSourceFile*> KnownFileSearchIndex;

  // Tests
  std::map<std::string, std::unique_ptr<cmTest>> Tests;

  // The set of include directories that are marked as system include
  // directories.
  std::set<std::string> SystemIncludeDirectories;

  std::vector<std::string> ListFiles;
  std::vector<std::string> OutputFiles;

  std::vector<std::unique_ptr<cmInstallGenerator>> InstallGenerators;
  std::vector<std::unique_ptr<cmTestGenerator>> TestGenerators;

  std::string ComplainFileRegularExpression;
  std::string DefineFlags;

  // Track the value of the computed DEFINITIONS property.
  std::string DefineFlagsOrig;

#if !defined(CMAKE_BOOTSTRAP)
  std::vector<cmSourceGroup> SourceGroups;
  size_t ObjectLibrariesSourceGroupIndex;
#endif

  cmGlobalGenerator* GlobalGenerator;
  bool IsFunctionBlocked(const cmListFileFunction& lff,
                         cmExecutionStatus& status);

private:
  cmStateSnapshot StateSnapshot;
  cmListFileBacktrace Backtrace;
  int RecursionDepth;

  struct DeferCommand
  {
    // Id is empty for an already-executed or canceled operation.
    std::string Id;
    std::string FilePath;
    cmListFileFunction Command;
  };
  struct DeferCommands
  {
    std::vector<DeferCommand> Commands;
  };
  std::unique_ptr<DeferCommands> Defer;
  bool DeferRunning = false;

  void DoGenerate(cmLocalGenerator& lg);

  void RunListFile(cmListFile const& listFile,
                   const std::string& filenametoread,
                   DeferCommands* defer = nullptr);

  bool ParseDefineFlag(std::string const& definition, bool remove);

  bool EnforceUniqueDir(const std::string& srcPath,
                        const std::string& binPath) const;

  std::function<void()> ExecuteCommandCallback;
  using FunctionBlockerPtr = std::unique_ptr<cmFunctionBlocker>;
  using FunctionBlockersType =
    std::stack<FunctionBlockerPtr, std::vector<FunctionBlockerPtr>>;
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
  std::vector<std::unique_ptr<cmExportBuildFileGenerator>>
    ExportBuildFileGenerators;

  std::vector<std::unique_ptr<cmGeneratorExpressionEvaluationFile>>
    EvaluationFiles;

  std::vector<cmExecutionStatus*> ExecutionStatusStack;
  friend class cmMakefileCall;
  friend class cmParseFileScope;

  std::vector<std::unique_ptr<cmTarget>> ImportedTargetsOwned;
  using TargetMap = std::unordered_map<std::string, cmTarget*>;
  TargetMap ImportedTargets;

  // Internal policy stack management.
  void PushPolicy(bool weak = false,
                  cmPolicies::PolicyMap const& pm = cmPolicies::PolicyMap());
  void PopPolicy();
  void PopSnapshot(bool reportError = true);
  friend bool cmCMakePolicyCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status);
  class IncludeScope;
  friend class IncludeScope;

  class ListFileScope;
  friend class ListFileScope;

  class DeferScope;
  friend class DeferScope;

  class DeferCallScope;
  friend class DeferCallScope;

  class BuildsystemFileScope;
  friend class BuildsystemFileScope;

  // CMP0053 == old
  MessageType ExpandVariablesInStringOld(std::string& errorstr,
                                         std::string& source,
                                         bool escapeQuotes, bool noEscapes,
                                         bool atOnly, const char* filename,
                                         long line, bool removeEmpty,
                                         bool replaceAt) const;
  // CMP0053 == new
  MessageType ExpandVariablesInStringNew(std::string& errorstr,
                                         std::string& source,
                                         bool escapeQuotes, bool noEscapes,
                                         bool atOnly, const char* filename,
                                         long line, bool replaceAt) const;

  bool ValidateCustomCommand(const cmCustomCommandLines& commandLines) const;

  void CreateGeneratedOutputs(const std::vector<std::string>& outputs);

  std::vector<BT<GeneratorAction>> GeneratorActions;
  bool GeneratorActionsInvoked = false;

  bool DebugFindPkg = false;

  bool CheckSystemVars;
  bool CheckCMP0000;
  std::set<std::string> WarnedCMP0074;
  bool IsSourceFileTryCompile;
  mutable bool SuppressSideEffects;
};
