/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmDocumentationEntry.h" // IWYU pragma: keep
#include "cmGeneratedFileStream.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmInstalledFile.h"
#include "cmListFileCache.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmValue.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm/optional>

#  include <cm3p/json/value.h>

#  include "cmCMakePresetsGraph.h"
#  include "cmMakefileProfilingData.h"
#endif

class cmConfigureLog;

#ifdef CMake_ENABLE_DEBUGGER
namespace cmDebugger {
class cmDebuggerAdapter;
}
#endif

class cmExternalMakefileProjectGeneratorFactory;
class cmFileAPI;
class cmFileTimeCache;
class cmGlobalGenerator;
class cmMakefile;
class cmMessenger;
class cmVariableWatch;
struct cmBuildOptions;

/** \brief Represents a cmake invocation.
 *
 * This class represents a cmake invocation. It is the top level class when
 * running cmake. Most cmake based GUIs should primarily create an instance
 * of this class and communicate with it.
 *
 * The basic process for a GUI is as follows:
 *
 * -# Create a cmake instance
 * -# Set the Home directories, generator, and cmake command. this
 *    can be done using the Set methods or by using SetArgs and passing in
 *    command line arguments.
 * -# Load the cache by calling LoadCache (duh)
 * -# if you are using command line arguments with -D or -C flags then
 *    call SetCacheArgs (or if for some other reason you want to modify the
 *    cache), do it now.
 * -# Finally call Configure
 * -# Let the user change values and go back to step 5
 * -# call Generate

 * If your GUI allows the user to change the home directories then
 * you must at a minimum redo steps 2 through 7.
 */

class cmake
{
public:
  enum Role
  {
    RoleInternal, // no commands
    RoleScript,   // script commands
    RoleProject   // all commands
  };

  enum DiagLevel
  {
    DIAG_IGNORE,
    DIAG_WARN,
    DIAG_ERROR
  };

  /** \brief Describes the working modes of cmake */
  enum WorkingMode
  {
    NORMAL_MODE, ///< Cmake runs to create project files

    /** \brief Script mode (started by using -P).
     *
     * In script mode there is no generator and no cache. Also,
     * languages are not enabled, so add_executable and things do
     * nothing.
     */
    SCRIPT_MODE,

    /** \brief Help mode
     *
     * Used to print help for things that can only be determined after finding
     * the source directory, for example, the list of presets.
     */
    HELP_MODE,

    /** \brief A pkg-config like mode
     *
     * In this mode cmake just searches for a package and prints the results to
     * stdout. This is similar to SCRIPT_MODE, but commands like add_library()
     * work too, since they may be used e.g. in exported target files. Started
     * via --find-package.
     */
    FIND_PACKAGE_MODE
  };

  using TraceFormat = cmTraceEnums::TraceOutputFormat;

  struct GeneratorInfo
  {
    std::string name;
    std::string baseName;
    std::string extraName;
    bool supportsToolset;
    bool supportsPlatform;
    std::vector<std::string> supportedPlatforms;
    std::string defaultPlatform;
    bool isAlias;
  };

  struct FileExtensions
  {
    bool Test(cm::string_view ext) const
    {
      return (this->unordered.find(ext) != this->unordered.end());
    }

    std::vector<std::string> ordered;
    std::unordered_set<cm::string_view> unordered;
  };

  using InstalledFilesMap = std::map<std::string, cmInstalledFile>;

  static const int NO_BUILD_PARALLEL_LEVEL = -1;
  static const int DEFAULT_BUILD_PARALLEL_LEVEL = 0;

  /// Default constructor
  cmake(Role role, cmState::Mode mode,
        cmState::ProjectKind projectKind = cmState::ProjectKind::Normal);
  /// Destructor
  ~cmake();

  cmake(cmake const&) = delete;
  cmake& operator=(cmake const&) = delete;

#if !defined(CMAKE_BOOTSTRAP)
  Json::Value ReportVersionJson() const;
  Json::Value ReportCapabilitiesJson() const;
#endif
  std::string ReportCapabilities() const;

  /**
   * Set the home directory from `-S` or from a known location
   * that contains a CMakeLists.txt. Will generate warnings
   * when overriding an existing source directory.
   *
   *  |    args           | src dir| warning        |
   *  | ----------------- | ------ | -------------- |
   *  | `dirA dirA`       | dirA   | N/A            |
   *  | `-S dirA -S dirA` | dirA   | N/A            |
   *  | `-S dirA -S dirB` | dirB   | Ignoring dirA  |
   *  | `-S dirA dirB`    | dirB   | Ignoring dirA  |
   *  | `dirA -S dirB`    | dirB   | Ignoring dirA  |
   *  | `dirA dirB`       | dirB   | Ignoring dirA  |
   */
  void SetHomeDirectoryViaCommandLine(std::string const& path);

  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is the
   * path-to-source cmake was run with.
   */
  void SetHomeDirectory(const std::string& dir);
  std::string const& GetHomeDirectory() const;
  void SetHomeOutputDirectory(const std::string& dir);
  std::string const& GetHomeOutputDirectory() const;
  //@}

  /**
   * Working directory at CMake launch
   */
  std::string const& GetCMakeWorkingDirectory() const
  {
    return this->CMakeWorkingDirectory;
  }

  /**
   * Handle a command line invocation of cmake.
   */
  int Run(const std::vector<std::string>& args)
  {
    return this->Run(args, false);
  }
  int Run(const std::vector<std::string>& args, bool noconfigure);

  /**
   * Run the global generator Generate step.
   */
  int Generate();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int Configure();
  int ActualConfigure();

  //! Break up a line like VAR:type="value" into var, type and value
  static bool ParseCacheEntry(const std::string& entry, std::string& var,
                              std::string& value,
                              cmStateEnums::CacheEntryType& type);

  int LoadCache();
  bool LoadCache(const std::string& path);
  bool LoadCache(const std::string& path, bool internal,
                 std::set<std::string>& excludes,
                 std::set<std::string>& includes);
  bool SaveCache(const std::string& path);
  bool DeleteCache(const std::string& path);
  void PreLoadCMakeFiles();

  //! Create a GlobalGenerator
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, bool allowArch = true);

  //! Create a GlobalGenerator and set it as our own
  bool CreateAndSetGlobalGenerator(const std::string& name, bool allowArch);

#ifndef CMAKE_BOOTSTRAP
  //! Print list of configure presets
  void PrintPresetList(const cmCMakePresetsGraph& graph) const;
#endif

  //! Return the global generator assigned to this instance of cmake
  cmGlobalGenerator* GetGlobalGenerator()
  {
    return this->GlobalGenerator.get();
  }
  //! Return the global generator assigned to this instance of cmake, const
  const cmGlobalGenerator* GetGlobalGenerator() const
  {
    return this->GlobalGenerator.get();
  }

  //! Return the full path to where the CMakeCache.txt file should be.
  static std::string FindCacheFile(const std::string& binaryDir);

  //! Return the global generator assigned to this instance of cmake
  void SetGlobalGenerator(std::unique_ptr<cmGlobalGenerator>);

  //! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<GeneratorInfo>& generators,
                               bool includeNamesWithPlatform = true) const;

  //! Set the name of the selected generator-specific instance.
  void SetGeneratorInstance(std::string const& instance)
  {
    this->GeneratorInstance = instance;
    this->GeneratorInstanceSet = true;
  }

  //! Set the name of the selected generator-specific platform.
  void SetGeneratorPlatform(std::string const& ts)
  {
    this->GeneratorPlatform = ts;
    this->GeneratorPlatformSet = true;
  }

  //! Set the name of the selected generator-specific toolset.
  void SetGeneratorToolset(std::string const& ts)
  {
    this->GeneratorToolset = ts;
    this->GeneratorToolsetSet = true;
  }

  bool IsAKnownSourceExtension(cm::string_view ext) const
  {
    return this->CLikeSourceFileExtensions.Test(ext) ||
      this->CudaFileExtensions.Test(ext) ||
      this->FortranFileExtensions.Test(ext) ||
      this->HipFileExtensions.Test(ext) || this->ISPCFileExtensions.Test(ext);
  }

  bool IsACLikeSourceExtension(cm::string_view ext) const
  {
    return this->CLikeSourceFileExtensions.Test(ext);
  }

  bool IsAKnownExtension(cm::string_view ext) const
  {
    return this->IsAKnownSourceExtension(ext) || this->IsAHeaderExtension(ext);
  }

  std::vector<std::string> GetAllExtensions() const;

  const std::vector<std::string>& GetHeaderExtensions() const
  {
    return this->HeaderFileExtensions.ordered;
  }

  bool IsAHeaderExtension(cm::string_view ext) const
  {
    return this->HeaderFileExtensions.Test(ext);
  }

  // Strips the extension (if present and known) from a filename
  std::string StripExtension(const std::string& file) const;

  /**
   * Given a variable name, return its value (as a string).
   */
  cmValue GetCacheDefinition(const std::string&) const;
  //! Add an entry into the cache
  void AddCacheEntry(const std::string& key, const std::string& value,
                     const std::string& helpString, int type)
  {
    this->AddCacheEntry(key, cmValue{ value }, cmValue{ helpString }, type);
  }
  void AddCacheEntry(const std::string& key, cmValue value,
                     const std::string& helpString, int type)
  {
    this->AddCacheEntry(key, value, cmValue{ helpString }, type);
  }
  void AddCacheEntry(const std::string& key, cmValue value, cmValue helpString,
                     int type);

  bool DoWriteGlobVerifyTarget() const;
  std::string const& GetGlobVerifyScript() const;
  std::string const& GetGlobVerifyStamp() const;
  void AddGlobCacheEntry(bool recurse, bool listDirectories,
                         bool followSymlinks, const std::string& relative,
                         const std::string& expression,
                         const std::vector<std::string>& files,
                         const std::string& variable,
                         cmListFileBacktrace const& bt);

  /**
   * Get the system information and write it to the file specified
   */
  int GetSystemInformation(std::vector<std::string>&);

  //! Parse environment variables
  void LoadEnvironmentPresets();

  //! Parse command line arguments
  void SetArgs(const std::vector<std::string>& args);

  //! Is this cmake running as a result of a TRY_COMPILE command
  bool GetIsInTryCompile() const;

#ifndef CMAKE_BOOTSTRAP
  void SetWarningFromPreset(const std::string& name,
                            const cm::optional<bool>& warning,
                            const cm::optional<bool>& error);
  void ProcessPresetVariables();
  void PrintPresetVariables();
  void ProcessPresetEnvironment();
  void PrintPresetEnvironment();
#endif

  //! Parse command line arguments that might set cache values
  bool SetCacheArgs(const std::vector<std::string>&);

  void ProcessCacheArg(const std::string& var, const std::string& value,
                       cmStateEnums::CacheEntryType type);

  using ProgressCallbackType = std::function<void(const std::string&, float)>;
  /**
   *  Set the function used by GUIs to receive progress updates
   *  Function gets passed: message as a const char*, a progress
   *  amount ranging from 0 to 1.0 and client data. The progress
   *  number provided may be negative in cases where a message is
   *  to be displayed without any progress percentage.
   */
  void SetProgressCallback(ProgressCallbackType f);

  //! this is called by generators to update the progress
  void UpdateProgress(const std::string& msg, float prog);

#if !defined(CMAKE_BOOTSTRAP)
  //! Get the variable watch object
  cmVariableWatch* GetVariableWatch() { return this->VariableWatch.get(); }
#endif

  std::vector<cmDocumentationEntry> GetGeneratorsDocumentation();

  //! Set/Get a property of this target file
  void SetProperty(const std::string& prop, cmValue value);
  void SetProperty(const std::string& prop, std::nullptr_t)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void SetProperty(const std::string& prop, const std::string& value)
  {
    this->SetProperty(prop, cmValue(value));
  }
  void AppendProperty(const std::string& prop, const std::string& value,
                      bool asString = false);
  cmValue GetProperty(const std::string& prop);
  bool GetPropertyAsBool(const std::string& prop);

  //! Get or create an cmInstalledFile instance and return a pointer to it
  cmInstalledFile* GetOrCreateInstalledFile(cmMakefile* mf,
                                            const std::string& name);

  cmInstalledFile const* GetInstalledFile(const std::string& name) const;

  InstalledFilesMap const& GetInstalledFiles() const
  {
    return this->InstalledFiles;
  }

  //! Do all the checks before running configure
  int DoPreConfigureChecks();

  void SetWorkingMode(WorkingMode mode) { this->CurrentWorkingMode = mode; }
  WorkingMode GetWorkingMode() { return this->CurrentWorkingMode; }

  //! Debug the try compile stuff by not deleting the files
  bool GetDebugTryCompile() const { return this->DebugTryCompile; }
  void DebugTryCompileOn() { this->DebugTryCompile = true; }

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths();

  /**
   * Get the file comparison class
   */
  cmFileTimeCache* GetFileTimeCache() { return this->FileTimeCache.get(); }

  bool WasLogLevelSetViaCLI() const { return this->LogLevelWasSetViaCLI; }

  //! Get the selected log level for `message()` commands during the cmake run.
  Message::LogLevel GetLogLevel() const { return this->MessageLogLevel; }
  void SetLogLevel(Message::LogLevel level) { this->MessageLogLevel = level; }
  static Message::LogLevel StringToLogLevel(cm::string_view levelStr);
  static std::string LogLevelToString(Message::LogLevel level);
  static TraceFormat StringToTraceFormat(const std::string& levelStr);

  bool HasCheckInProgress() const
  {
    return !this->CheckInProgressMessages.empty();
  }
  std::size_t GetCheckInProgressSize() const
  {
    return this->CheckInProgressMessages.size();
  }
  std::string GetTopCheckInProgressMessage()
  {
    auto message = this->CheckInProgressMessages.back();
    this->CheckInProgressMessages.pop_back();
    return message;
  }
  void PushCheckInProgressMessage(std::string message)
  {
    this->CheckInProgressMessages.emplace_back(std::move(message));
  }
  std::vector<std::string> const& GetCheckInProgressMessages() const
  {
    return this->CheckInProgressMessages;
  }

  //! Should `message` command display context.
  bool GetShowLogContext() const { return this->LogContext; }
  void SetShowLogContext(bool b) { this->LogContext = b; }

  //! Do we want debug output during the cmake run.
  bool GetDebugOutput() const { return this->DebugOutput; }
  void SetDebugOutputOn(bool b) { this->DebugOutput = b; }

  //! Do we want debug output from the find commands during the cmake run.
  bool GetDebugFindOutput() const { return this->DebugFindOutput; }
  bool GetDebugFindOutput(std::string const& var) const;
  bool GetDebugFindPkgOutput(std::string const& pkg) const;
  void SetDebugFindOutput(bool b) { this->DebugFindOutput = b; }
  void SetDebugFindOutputPkgs(std::string const& args);
  void SetDebugFindOutputVars(std::string const& args);

  //! Do we want trace output during the cmake run.
  bool GetTrace() const { return this->Trace; }
  void SetTrace(bool b) { this->Trace = b; }
  bool GetTraceExpand() const { return this->TraceExpand; }
  void SetTraceExpand(bool b) { this->TraceExpand = b; }
  TraceFormat GetTraceFormat() const { return this->TraceFormatVar; }
  void SetTraceFormat(TraceFormat f) { this->TraceFormatVar = f; }
  void AddTraceSource(std::string const& file)
  {
    this->TraceOnlyThisSources.push_back(file);
  }
  std::vector<std::string> const& GetTraceSources() const
  {
    return this->TraceOnlyThisSources;
  }
  cmGeneratedFileStream& GetTraceFile()
  {
    if (this->TraceRedirect) {
      return this->TraceRedirect->GetTraceFile();
    }
    return this->TraceFile;
  }
  void SetTraceFile(std::string const& file);
  void PrintTraceFormatVersion();

#ifndef CMAKE_BOOTSTRAP
  cmConfigureLog* GetConfigureLog() const { return this->ConfigureLog.get(); }
#endif

  //! Use trace from another ::cmake instance.
  void SetTraceRedirect(cmake* other);

  bool GetWarnUninitialized() const { return this->WarnUninitialized; }
  void SetWarnUninitialized(bool b) { this->WarnUninitialized = b; }
  bool GetWarnUnusedCli() const { return this->WarnUnusedCli; }
  void SetWarnUnusedCli(bool b) { this->WarnUnusedCli = b; }
  bool GetCheckSystemVars() const { return this->CheckSystemVars; }
  void SetCheckSystemVars(bool b) { this->CheckSystemVars = b; }
  bool GetIgnoreWarningAsError() const { return this->IgnoreWarningAsError; }
  void SetIgnoreWarningAsError(bool b) { this->IgnoreWarningAsError = b; }

  void MarkCliAsUsed(const std::string& variable);

  /** Get the list of configurations (in upper case) considered to be
      debugging configurations.*/
  std::vector<std::string> GetDebugConfigs();

  void SetCMakeEditCommand(std::string const& s)
  {
    this->CMakeEditCommand = s;
  }
  std::string const& GetCMakeEditCommand() const
  {
    return this->CMakeEditCommand;
  }

  cmMessenger* GetMessenger() const { return this->Messenger.get(); }

  /**
   * Get the state of the suppression of developer (author) warnings.
   * Returns false, by default, if developer warnings should be shown, true
   * otherwise.
   */
  bool GetSuppressDevWarnings() const;
  /**
   * Set the state of the suppression of developer (author) warnings.
   */
  void SetSuppressDevWarnings(bool v);

  /**
   * Get the state of the suppression of deprecated warnings.
   * Returns false, by default, if deprecated warnings should be shown, true
   * otherwise.
   */
  bool GetSuppressDeprecatedWarnings() const;
  /**
   * Set the state of the suppression of deprecated warnings.
   */
  void SetSuppressDeprecatedWarnings(bool v);

  /**
   * Get the state of treating developer (author) warnings as errors.
   * Returns false, by default, if warnings should not be treated as errors,
   * true otherwise.
   */
  bool GetDevWarningsAsErrors() const;
  /**
   * Set the state of treating developer (author) warnings as errors.
   */
  void SetDevWarningsAsErrors(bool v);

  /**
   * Get the state of treating deprecated warnings as errors.
   * Returns false, by default, if warnings should not be treated as errors,
   * true otherwise.
   */
  bool GetDeprecatedWarningsAsErrors() const;
  /**
   * Set the state of treating developer (author) warnings as errors.
   */
  void SetDeprecatedWarningsAsErrors(bool v);

  /** Display a message to the user.  */
  void IssueMessage(
    MessageType t, std::string const& text,
    cmListFileBacktrace const& backtrace = cmListFileBacktrace()) const;

  //! run the --build option
  int Build(int jobs, std::string dir, std::vector<std::string> targets,
            std::string config, std::vector<std::string> nativeOptions,
            cmBuildOptions& buildOptions, bool verbose,
            const std::string& presetName, bool listPresets);

  //! run the --open option
  bool Open(const std::string& dir, bool dryRun);

  //! run the --workflow option
  enum class WorkflowListPresets
  {
    No,
    Yes,
  };
  enum class WorkflowFresh
  {
    No,
    Yes,
  };
  int Workflow(const std::string& presetName, WorkflowListPresets listPresets,
               WorkflowFresh fresh);

  void UnwatchUnusedCli(const std::string& var);
  void WatchUnusedCli(const std::string& var);

#if !defined(CMAKE_BOOTSTRAP)
  cmFileAPI* GetFileAPI() const { return this->FileAPI.get(); }
#endif

  cmState* GetState() const { return this->State.get(); }
  void SetCurrentSnapshot(cmStateSnapshot const& snapshot)
  {
    this->CurrentSnapshot = snapshot;
  }
  cmStateSnapshot GetCurrentSnapshot() const { return this->CurrentSnapshot; }

  bool GetRegenerateDuringBuild() const { return this->RegenerateDuringBuild; }

#if !defined(CMAKE_BOOTSTRAP)
  cmMakefileProfilingData& GetProfilingOutput();
  bool IsProfilingEnabled() const;

  cm::optional<cmMakefileProfilingData::RAII> CreateProfilingEntry(
    const std::string& category, const std::string& name)
  {
    return this->CreateProfilingEntry(
      category, name, []() -> cm::nullopt_t { return cm::nullopt; });
  }

  template <typename ArgsFunc>
  cm::optional<cmMakefileProfilingData::RAII> CreateProfilingEntry(
    const std::string& category, const std::string& name, ArgsFunc&& argsFunc)
  {
    if (this->IsProfilingEnabled()) {
      return cm::make_optional<cmMakefileProfilingData::RAII>(
        this->GetProfilingOutput(), category, name, argsFunc());
    }
    return cm::nullopt;
  }
#endif

#ifdef CMake_ENABLE_DEBUGGER
  bool GetDebuggerOn() const { return this->DebuggerOn; }
  std::string GetDebuggerPipe() const { return this->DebuggerPipe; }
  std::string GetDebuggerDapLogFile() const
  {
    return this->DebuggerDapLogFile;
  }
  void SetDebuggerOn(bool b) { this->DebuggerOn = b; }
  bool StartDebuggerIfEnabled();
  void StopDebuggerIfNeeded(int exitCode);
  std::shared_ptr<cmDebugger::cmDebuggerAdapter> GetDebugAdapter()
    const noexcept
  {
    return this->DebugAdapter;
  }
#endif

protected:
  void RunCheckForUnusedVariables();
  int HandleDeleteCacheVariables(const std::string& var);

  using RegisteredGeneratorsVector =
    std::vector<std::unique_ptr<cmGlobalGeneratorFactory>>;
  RegisteredGeneratorsVector Generators;
  using RegisteredExtraGeneratorsVector =
    std::vector<cmExternalMakefileProjectGeneratorFactory*>;
  RegisteredExtraGeneratorsVector ExtraGenerators;
  void AddScriptingCommands() const;
  void AddProjectCommands() const;
  void AddDefaultGenerators();
  void AddDefaultExtraGenerators();

  std::map<std::string, DiagLevel> DiagLevels;
  std::string GeneratorInstance;
  std::string GeneratorPlatform;
  std::string GeneratorToolset;
  bool GeneratorInstanceSet = false;
  bool GeneratorPlatformSet = false;
  bool GeneratorToolsetSet = false;

  //! read in a cmake list file to initialize the cache
  void ReadListFile(const std::vector<std::string>& args,
                    const std::string& path);
  bool FindPackage(const std::vector<std::string>& args);

  //! Check if CMAKE_CACHEFILE_DIR is set. If it is not, delete the log file.
  ///  If it is set, truncate it to 50kb
  void TruncateOutputLog(const char* fname);

  /**
   * Method called to check build system integrity at build time.
   * Returns 1 if CMake should rerun and 0 otherwise.
   */
  int CheckBuildSystem();

  bool SetDirectoriesFromFile(const std::string& arg);

  //! Make sure all commands are what they say they are and there is no
  /// macros.
  void CleanupCommandsAndMacros();

  void GenerateGraphViz(const std::string& fileName) const;

private:
  std::string CMakeWorkingDirectory;
  ProgressCallbackType ProgressCallback;
  WorkingMode CurrentWorkingMode = NORMAL_MODE;
  bool DebugOutput = false;
  bool DebugFindOutput = false;
  bool Trace = false;
  bool TraceExpand = false;
  TraceFormat TraceFormatVar = TraceFormat::Human;
  cmGeneratedFileStream TraceFile;
  cmake* TraceRedirect = nullptr;
#ifndef CMAKE_BOOTSTRAP
  std::unique_ptr<cmConfigureLog> ConfigureLog;
#endif
  bool WarnUninitialized = false;
  bool WarnUnusedCli = true;
  bool CheckSystemVars = false;
  bool IgnoreWarningAsError = false;
  std::map<std::string, bool> UsedCliVariables;
  std::string CMakeEditCommand;
  std::string CXXEnvironment;
  std::string CCEnvironment;
  std::string CheckBuildSystemArgument;
  std::string CheckStampFile;
  std::string CheckStampList;
  std::string VSSolutionFile;
  std::string EnvironmentGenerator;
  FileExtensions CLikeSourceFileExtensions;
  FileExtensions HeaderFileExtensions;
  FileExtensions CudaFileExtensions;
  FileExtensions ISPCFileExtensions;
  FileExtensions FortranFileExtensions;
  FileExtensions HipFileExtensions;
  bool ClearBuildSystem = false;
  bool DebugTryCompile = false;
  bool FreshCache = false;
  bool RegenerateDuringBuild = false;
  std::unique_ptr<cmFileTimeCache> FileTimeCache;
  std::string GraphVizFile;
  InstalledFilesMap InstalledFiles;
#ifndef CMAKE_BOOTSTRAP
  std::map<std::string, cm::optional<cmCMakePresetsGraph::CacheVariable>>
    UnprocessedPresetVariables;
  std::map<std::string, cm::optional<std::string>>
    UnprocessedPresetEnvironment;
#endif

#if !defined(CMAKE_BOOTSTRAP)
  std::unique_ptr<cmVariableWatch> VariableWatch;
  std::unique_ptr<cmFileAPI> FileAPI;
#endif

  std::unique_ptr<cmState> State;
  cmStateSnapshot CurrentSnapshot;
  std::unique_ptr<cmMessenger> Messenger;

  std::vector<std::string> TraceOnlyThisSources;

  std::set<std::string> DebugFindPkgs;
  std::set<std::string> DebugFindVars;

  Message::LogLevel MessageLogLevel = Message::LogLevel::LOG_STATUS;
  bool LogLevelWasSetViaCLI = false;
  bool LogContext = false;

  std::vector<std::string> CheckInProgressMessages;

  std::unique_ptr<cmGlobalGenerator> GlobalGenerator;

  void UpdateConversionPathTable();

  //! Print a list of valid generators to stderr.
  void PrintGeneratorList();

  std::unique_ptr<cmGlobalGenerator> EvaluateDefaultGlobalGenerator();
  void CreateDefaultGlobalGenerator();

  void AppendGlobalGeneratorsDocumentation(std::vector<cmDocumentationEntry>&);
  void AppendExtraGeneratorsDocumentation(std::vector<cmDocumentationEntry>&);

#if !defined(CMAKE_BOOTSTRAP)
  template <typename T>
  const T* FindPresetForWorkflow(
    cm::static_string_view type,
    const std::map<std::string, cmCMakePresetsGraph::PresetPair<T>>& presets,
    const cmCMakePresetsGraph::WorkflowPreset::WorkflowStep& step);

  std::function<int()> BuildWorkflowStep(const std::vector<std::string>& args);
#endif

#if !defined(CMAKE_BOOTSTRAP)
  std::unique_ptr<cmMakefileProfilingData> ProfilingOutput;
#endif

#ifdef CMake_ENABLE_DEBUGGER
  std::shared_ptr<cmDebugger::cmDebuggerAdapter> DebugAdapter;
  bool DebuggerOn = false;
  std::string DebuggerPipe;
  std::string DebuggerDapLogFile;
#endif

  cm::optional<int> ScriptModeExitCode;

public:
  bool HasScriptModeExitCode() const { return ScriptModeExitCode.has_value(); }
  void SetScriptModeExitCode(int code) { ScriptModeExitCode = code; }
  int GetScriptModeExitCode() const { return ScriptModeExitCode.value_or(-1); }

  static cmDocumentationEntry CMAKE_STANDARD_OPTIONS_TABLE[18];
};

#define FOR_EACH_C90_FEATURE(F) F(c_function_prototypes)

#define FOR_EACH_C99_FEATURE(F)                                               \
  F(c_restrict)                                                               \
  F(c_variadic_macros)

#define FOR_EACH_C11_FEATURE(F) F(c_static_assert)

#define FOR_EACH_C_FEATURE(F)                                                 \
  F(c_std_90)                                                                 \
  F(c_std_99)                                                                 \
  F(c_std_11)                                                                 \
  F(c_std_17)                                                                 \
  F(c_std_23)                                                                 \
  FOR_EACH_C90_FEATURE(F)                                                     \
  FOR_EACH_C99_FEATURE(F)                                                     \
  FOR_EACH_C11_FEATURE(F)

#define FOR_EACH_CXX98_FEATURE(F) F(cxx_template_template_parameters)

#define FOR_EACH_CXX11_FEATURE(F)                                             \
  F(cxx_alias_templates)                                                      \
  F(cxx_alignas)                                                              \
  F(cxx_alignof)                                                              \
  F(cxx_attributes)                                                           \
  F(cxx_auto_type)                                                            \
  F(cxx_constexpr)                                                            \
  F(cxx_decltype)                                                             \
  F(cxx_decltype_incomplete_return_types)                                     \
  F(cxx_default_function_template_args)                                       \
  F(cxx_defaulted_functions)                                                  \
  F(cxx_defaulted_move_initializers)                                          \
  F(cxx_delegating_constructors)                                              \
  F(cxx_deleted_functions)                                                    \
  F(cxx_enum_forward_declarations)                                            \
  F(cxx_explicit_conversions)                                                 \
  F(cxx_extended_friend_declarations)                                         \
  F(cxx_extern_templates)                                                     \
  F(cxx_final)                                                                \
  F(cxx_func_identifier)                                                      \
  F(cxx_generalized_initializers)                                             \
  F(cxx_inheriting_constructors)                                              \
  F(cxx_inline_namespaces)                                                    \
  F(cxx_lambdas)                                                              \
  F(cxx_local_type_template_args)                                             \
  F(cxx_long_long_type)                                                       \
  F(cxx_noexcept)                                                             \
  F(cxx_nonstatic_member_init)                                                \
  F(cxx_nullptr)                                                              \
  F(cxx_override)                                                             \
  F(cxx_range_for)                                                            \
  F(cxx_raw_string_literals)                                                  \
  F(cxx_reference_qualified_functions)                                        \
  F(cxx_right_angle_brackets)                                                 \
  F(cxx_rvalue_references)                                                    \
  F(cxx_sizeof_member)                                                        \
  F(cxx_static_assert)                                                        \
  F(cxx_strong_enums)                                                         \
  F(cxx_thread_local)                                                         \
  F(cxx_trailing_return_types)                                                \
  F(cxx_unicode_literals)                                                     \
  F(cxx_uniform_initialization)                                               \
  F(cxx_unrestricted_unions)                                                  \
  F(cxx_user_literals)                                                        \
  F(cxx_variadic_macros)                                                      \
  F(cxx_variadic_templates)

#define FOR_EACH_CXX14_FEATURE(F)                                             \
  F(cxx_aggregate_default_initializers)                                       \
  F(cxx_attribute_deprecated)                                                 \
  F(cxx_binary_literals)                                                      \
  F(cxx_contextual_conversions)                                               \
  F(cxx_decltype_auto)                                                        \
  F(cxx_digit_separators)                                                     \
  F(cxx_generic_lambdas)                                                      \
  F(cxx_lambda_init_captures)                                                 \
  F(cxx_relaxed_constexpr)                                                    \
  F(cxx_return_type_deduction)                                                \
  F(cxx_variable_templates)

#define FOR_EACH_CXX_FEATURE(F)                                               \
  F(cxx_std_98)                                                               \
  F(cxx_std_11)                                                               \
  F(cxx_std_14)                                                               \
  F(cxx_std_17)                                                               \
  F(cxx_std_20)                                                               \
  F(cxx_std_23)                                                               \
  F(cxx_std_26)                                                               \
  FOR_EACH_CXX98_FEATURE(F)                                                   \
  FOR_EACH_CXX11_FEATURE(F)                                                   \
  FOR_EACH_CXX14_FEATURE(F)

#define FOR_EACH_CUDA_FEATURE(F)                                              \
  F(cuda_std_03)                                                              \
  F(cuda_std_11)                                                              \
  F(cuda_std_14)                                                              \
  F(cuda_std_17)                                                              \
  F(cuda_std_20)                                                              \
  F(cuda_std_23)                                                              \
  F(cuda_std_26)

#define FOR_EACH_HIP_FEATURE(F)                                               \
  F(hip_std_98)                                                               \
  F(hip_std_11)                                                               \
  F(hip_std_14)                                                               \
  F(hip_std_17)                                                               \
  F(hip_std_20)                                                               \
  F(hip_std_23)                                                               \
  F(hip_std_26)
