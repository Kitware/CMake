/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// This class represents a cmake invocation. It is the top level class when
// running cmake. Most cmake based GUIS should primarily create an instance
// of this class and communicate with it.
//
// The basic process for a GUI is as follows:
//
// 1) Create a cmake instance
// 2) Set the Home & Start directories, generator, and cmake command. this
//    can be done using the Set methods or by using SetArgs and passing in
//    command line arguments.
// 3) Load the cache by calling LoadCache (duh)
// 4) if you are using command line arguments with -D or -C flags then
//    call SetCacheArgs (or if for some other reason you want to modify the
//    cache, do it now.
// 5) Finally call Configure
// 6) Let the user change values and go back to step 5
// 7) call Generate
//
// If your GUI allows the user to change the start & home directories then
// you must at a minimum redo steps 2 through 7.
//


#ifndef cmake_h
#define cmake_h

#include "cmSystemTools.h"
#include "cmPropertyDefinitionMap.h"
#include "cmPropertyMap.h"

class cmGlobalGenerator;
class cmLocalGenerator;
class cmCacheManager;
class cmMakefile;
class cmCommand;
class cmVariableWatch;
class cmFileTimeComparison;
class cmExternalMakefileProjectGenerator;
class cmDocumentationSection;
class cmPolicies;
class cmListFileBacktrace;
class cmTarget;
class cmGeneratedFileStream;

class cmake
{
 public:
  enum MessageType
  { AUTHOR_WARNING,
    FATAL_ERROR,
    INTERNAL_ERROR,
    MESSAGE,
    WARNING,
    LOG
  };
  typedef std::map<cmStdString, cmCommand*> RegisteredCommandsMap;

  ///! construct an instance of cmake
  cmake();
  ///! destruct an instance of cmake
  ~cmake();

  ///! construct an instance of cmake
  static const char *GetCMakeFilesDirectory() {return "/CMakeFiles";};
  static const char *GetCMakeFilesDirectoryPostSlash() {
    return "CMakeFiles/";};

  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is where
   * cmake was run. Remember that CMake processes
   * CMakeLists files by recursing up the tree starting at the StartDirectory
   * and going up until it reaches the HomeDirectory.
   */
  void SetHomeDirectory(const char* dir);
  const char* GetHomeDirectory() const
    {
    return this->cmHomeDirectory.c_str();
    }
  void SetHomeOutputDirectory(const char* lib);
  const char* GetHomeOutputDirectory() const
    {
    return this->HomeOutputDirectory.c_str();
    }
  //@}

  //@{
  /**
   * Set/Get the start directory (or output directory). The start directory
   * is the directory of the CMakeLists.txt file that started the current
   * round of processing. Remember that CMake processes CMakeLists files by
   * recursing up the tree starting at the StartDirectory and going up until
   * it reaches the HomeDirectory.
   */
  void SetStartDirectory(const char* dir)
    {
      this->cmStartDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(this->cmStartDirectory);
    }
  const char* GetStartDirectory() const
    {
      return this->cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      this->StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(this->StartOutputDirectory);
    }
  const char* GetStartOutputDirectory() const
    {
      return this->StartOutputDirectory.c_str();
    }
  //@}

  /**
   * Handle a command line invocation of cmake.
   */
  int Run(const std::vector<std::string>&args)
    { return this->Run(args, false); }
  int Run(const std::vector<std::string>&args, bool noconfigure);

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

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int LoadCache();
  void PreLoadCMakeFiles();

  ///! Create a GlobalGenerator
  cmGlobalGenerator* CreateGlobalGenerator(const char* name);

  ///! Return the global generator assigned to this instance of cmake
  cmGlobalGenerator* GetGlobalGenerator()     { return this->GlobalGenerator; }
  ///! Return the global generator assigned to this instance of cmake, const
  const cmGlobalGenerator* GetGlobalGenerator() const
                                              { return this->GlobalGenerator; }

  ///! Return the global generator assigned to this instance of cmake
  void SetGlobalGenerator(cmGlobalGenerator *);

  ///! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<std::string>& names);

  ///! get the cmCachemManager used by this invocation of cmake
  cmCacheManager *GetCacheManager() { return this->CacheManager; }

  ///! set the cmake command this instance of cmake should use
  void SetCMakeCommand(const char* cmd) { this->CMakeCommand = cmd; }

  /**
   * Given a variable name, return its value (as a string).
   */
  const char* GetCacheDefinition(const char*) const;
  ///! Add an entry into the cache
  void AddCacheEntry(const char* key, const char* value,
                     const char* helpString,
                     int type);
  /**
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int ExecuteCMakeCommand(std::vector<std::string>&);

  /**
   * Get the system information and write it to the file specified
   */
  int GetSystemInformation(std::vector<std::string>&);

  /**
   * Add a command to this cmake instance
   */
  void AddCommand(cmCommand* );
  void RenameCommand(const char* oldName, const char* newName);
  void RemoveCommand(const char* name);
  void RemoveUnscriptableCommands();

  /**
   * Get a command by its name
   */
  cmCommand *GetCommand(const char *name);

  /** Get list of all commands */
  RegisteredCommandsMap* GetCommands() { return &this->Commands; }

  /** Check if a command exists. */
  bool CommandExists(const char* name) const;

  ///! Parse command line arguments
  void SetArgs(const std::vector<std::string>&,
               bool directoriesSetBefore = false);

  ///! Is this cmake running as a result of a TRY_COMPILE command
  bool GetIsInTryCompile() { return this->InTryCompile; }

  ///! Is this cmake running as a result of a TRY_COMPILE command
  void SetIsInTryCompile(bool i) { this->InTryCompile = i; }

  ///! Parse command line arguments that might set cache values
  bool SetCacheArgs(const std::vector<std::string>&);

  typedef  void (*ProgressCallbackType)
    (const char*msg, float progress, void *);
  /**
   *  Set the function used by GUI's to receive progress updates
   *  Function gets passed: message as a const char*, a progress
   *  amount ranging from 0 to 1.0 and client data. The progress
   *  number provided may be negative in cases where a message is
   *  to be displayed without any progress percentage.
   */
  void SetProgressCallback(ProgressCallbackType f, void* clientData=0);

  ///! this is called by generators to update the progress
  void UpdateProgress(const char *msg, float prog);

  ///!  get the cmake policies instance
  cmPolicies *GetPolicies() {return this->Policies;} ;

  ///! Get the variable watch object
  cmVariableWatch* GetVariableWatch() { return this->VariableWatch; }

  /** Get the documentation entries for the supported commands.
   *  If withCurrentCommands is true, the documentation for the
   *  recommended set of commands is included.
   *  If withCompatCommands is true, the documentation for discouraged
   *  (compatibility) commands is included.
   *  You probably don't want to set both to false.
   */
  void GetCommandDocumentation(std::vector<cmDocumentationEntry>& entries,
                               bool withCurrentCommands = true,
                               bool withCompatCommands = true) const;
  void GetPropertiesDocumentation(std::map<std::string,
                                  cmDocumentationSection *>&);
  void GetGeneratorDocumentation(std::vector<cmDocumentationEntry>&);
  void GetPolicyDocumentation(std::vector<cmDocumentationEntry>& entries);

  ///! Set/Get a property of this target file
  void SetProperty(const char *prop, const char *value);
  void AppendProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop);
  const char *GetProperty(const char *prop, cmProperty::ScopeType scope);
  bool GetPropertyAsBool(const char *prop);

  // Get the properties
  cmPropertyMap &GetProperties() { return this->Properties; };

  ///! Do all the checks before running configure
  int DoPreConfigureChecks();

  /**
   * Set and get the script mode option. In script mode there is no
   * generator and no cache. Also, language are not enabled, so
   * add_executable and things do not do anything.
   */
  void SetScriptMode(bool mode) { this->ScriptMode = mode; }
  bool GetScriptMode() { return this->ScriptMode; }

  ///! Debug the try compile stuff by not delelting the files
  bool GetDebugTryCompile(){return this->DebugTryCompile;}
  void DebugTryCompileOn(){this->DebugTryCompile = true;}

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths();

  /**
   * Get the file comparison class
   */
  cmFileTimeComparison* GetFileComparison() { return this->FileComparison; }

  /**
   * Get the path to ctest
   */
  const char* GetCTestCommand();
  const char* GetCPackCommand();

  // Do we want debug output during the cmake run.
  bool GetDebugOutput() { return this->DebugOutput; }
  void SetDebugOutputOn(bool b) { this->DebugOutput = b;}

  // Do we want trace output during the cmake run.
  bool GetTrace() { return this->Trace;}
  void SetTrace(bool b) {  this->Trace = b;}
  bool GetWarnUninitialized() { return this->WarnUninitialized;}
  void SetWarnUninitialized(bool b) {  this->WarnUninitialized = b;}
  bool GetWarnUnused() { return this->WarnUnused;}
  void SetWarnUnused(bool b) {  this->WarnUnused = b;}
  bool GetWarnUnusedCli() { return this->WarnUnusedCli;}
  void SetWarnUnusedCli(bool b) {  this->WarnUnusedCli = b;}
  bool GetCheckSystemVars() { return this->CheckSystemVars;}
  void SetCheckSystemVars(bool b) {  this->CheckSystemVars = b;}

  void MarkCliAsUsed(const std::string& variable);

  // Define a property
  void DefineProperty(const char *name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chain = false,
                      const char *variableGroup = 0);

  // get property definition
  cmPropertyDefinition *GetPropertyDefinition
  (const char *name, cmProperty::ScopeType scope);

  // Is a property defined?
  bool IsPropertyDefined(const char *name, cmProperty::ScopeType scope);
  bool IsPropertyChained(const char *name, cmProperty::ScopeType scope);

  /** Get the list of configurations (in upper case) considered to be
      debugging configurations.*/
  std::vector<std::string> const& GetDebugConfigs();

  // record accesses of properties and variables
  void RecordPropertyAccess(const char *name, cmProperty::ScopeType scope);
  void ReportUndefinedPropertyAccesses(const char *filename);

  // Define the properties
  static void DefineProperties(cmake *cm);

  void SetCMakeEditCommand(const char* s)
    {
      this->CMakeEditCommand = s;
    }
  void SetSuppressDevWarnings(bool v)
    {
      this->SuppressDevWarnings = v;
      this->DoSuppressDevWarnings = true;
    }

  /** Display a message to the user.  */
  void IssueMessage(cmake::MessageType t, std::string const& text,
                    cmListFileBacktrace const& backtrace);
  //  * run the --build option
  int Build(const std::string& dir,
            const std::string& target,
            const std::string& config,
            const std::vector<std::string>& nativeOptions,
            bool clean);

  void UnwatchUnusedCli(const char* var);
  void WatchUnusedCli(const char* var);
protected:
  void RunCheckForUnusedVariables();
  void InitializeProperties();
  int HandleDeleteCacheVariables(const char* var);
  cmPropertyMap Properties;
  std::set<std::pair<cmStdString,cmProperty::ScopeType> > AccessedProperties;

  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap>
  PropertyDefinitions;

  typedef
     cmExternalMakefileProjectGenerator* (*CreateExtraGeneratorFunctionType)();
  typedef std::map<cmStdString,
                CreateExtraGeneratorFunctionType> RegisteredExtraGeneratorsMap;

  typedef cmGlobalGenerator* (*CreateGeneratorFunctionType)();
  typedef std::map<cmStdString,
                   CreateGeneratorFunctionType> RegisteredGeneratorsMap;
  RegisteredCommandsMap Commands;
  RegisteredGeneratorsMap Generators;
  RegisteredExtraGeneratorsMap ExtraGenerators;
  void AddDefaultCommands();
  void AddDefaultGenerators();
  void AddDefaultExtraGenerators();
  void AddExtraGenerator(const char* name,
                         CreateExtraGeneratorFunctionType newFunction);

  cmPolicies *Policies;
  cmGlobalGenerator *GlobalGenerator;
  cmCacheManager *CacheManager;
  std::string cmHomeDirectory;
  std::string HomeOutputDirectory;
  std::string cmStartDirectory;
  std::string StartOutputDirectory;
  bool SuppressDevWarnings;
  bool DoSuppressDevWarnings;

  ///! read in a cmake list file to initialize the cache
  void ReadListFile(const std::vector<std::string>& args, const char *path);

  ///! Check if CMAKE_CACHEFILE_DIR is set. If it is not, delete the log file.
  ///  If it is set, truncate it to 50kb
  void TruncateOutputLog(const char* fname);

  /**
   * Method called to check build system integrity at build time.
   * Returns 1 if CMake should rerun and 0 otherwise.
   */
  int CheckBuildSystem();

  void SetDirectoriesFromFile(const char* arg);

  //! Make sure all commands are what they say they are and there is no
  //macros.
  void CleanupCommandsAndMacros();

  void GenerateGraphViz(const char* fileName) const;

  static int SymlinkLibrary(std::vector<std::string>& args);
  static int SymlinkExecutable(std::vector<std::string>& args);
  static bool SymlinkInternal(std::string const& file,
                              std::string const& link);
  static int ExecuteEchoColor(std::vector<std::string>& args);
  static int ExecuteLinkScript(std::vector<std::string>& args);
  static int VisualStudioLink(std::vector<std::string>& args, int type);
  static int VisualStudioLinkIncremental(std::vector<std::string>& args,
                                         int type,
                                         bool verbose);
  static int VisualStudioLinkNonIncremental(std::vector<std::string>& args,
                                            int type,
                                            bool hasManifest,
                                            bool verbose);
  static int ParseVisualStudioLinkCommand(std::vector<std::string>& args,
                                          std::vector<cmStdString>& command,
                                          std::string& targetName);
  static bool RunCommand(const char* comment,
                         std::vector<cmStdString>& command,
                         bool verbose,
                         int* retCodeOut = 0);
  cmVariableWatch* VariableWatch;

  ///! Find the full path to one of the cmake programs like ctest, cpack, etc.
  std::string FindCMakeProgram(const char* name) const;

  bool RejectUnsupportedPaths(const char* desc, std::string const& path);
private:
  cmake(const cmake&);  // Not implemented.
  void operator=(const cmake&);  // Not implemented.
  ProgressCallbackType ProgressCallback;
  void* ProgressCallbackClientData;
  bool Verbose;
  bool InTryCompile;
  bool ScriptMode;
  bool DebugOutput;
  bool Trace;
  bool WarnUninitialized;
  bool WarnUnused;
  bool WarnUnusedCli;
  bool CheckSystemVars;
  std::map<cmStdString, bool> UsedCliVariables;
  std::string CMakeEditCommand;
  std::string CMakeCommand;
  std::string CXXEnvironment;
  std::string CCEnvironment;
  std::string CheckBuildSystemArgument;
  std::string CheckStampFile;
  std::string CheckStampList;
  std::string VSSolutionFile;
  std::string CTestCommand;
  std::string CPackCommand;
  bool ClearBuildSystem;
  bool DebugTryCompile;
  cmFileTimeComparison* FileComparison;
  std::string GraphVizFile;
  std::vector<std::string> DebugConfigs;

  void UpdateConversionPathTable();
};

#define CMAKE_STANDARD_OPTIONS_TABLE \
  {"-C <initial-cache>", "Pre-load a script to populate the cache.", \
   "When cmake is first run in an empty build tree, it creates a " \
   "CMakeCache.txt file and populates it with customizable settings " \
   "for the project.  This option may be used to specify a file from " \
   "which to load cache entries before the first pass through " \
   "the project's cmake listfiles.  The loaded entries take priority " \
   "over the project's default values.  The given file should be a CMake " \
   "script containing SET commands that use the CACHE option, " \
   "not a cache-format file."}, \
  {"-D <var>:<type>=<value>", "Create a cmake cache entry.", \
   "When cmake is first run in an empty build tree, it creates a " \
   "CMakeCache.txt file and populates it with customizable settings " \
   "for the project.  This option may be used to specify a setting " \
   "that takes priority over the project's default value.  The option " \
   "may be repeated for as many cache entries as desired."}, \
  {"-U <globbing_expr>", "Remove matching entries from CMake cache.", \
   "This option may be used to remove one or more variables from the " \
   "CMakeCache.txt file, globbing expressions using * and ? are supported. "\
   "The option may be repeated for as many cache entries as desired.\n" \
   "Use with care, you can make your CMakeCache.txt non-working."}, \
  {"-G <generator-name>", "Specify a makefile generator.", \
   "CMake may support multiple native build systems on certain platforms.  " \
   "A makefile generator is responsible for generating a particular build " \
   "system.  Possible generator names are specified in the Generators " \
   "section."},\
  {"-Wno-dev", "Suppress developer warnings.",\
   "Suppress warnings that are meant for the author"\
   " of the CMakeLists.txt files."},\
  {"-Wdev", "Enable developer warnings.",\
   "Enable warnings that are meant for the author"\
   " of the CMakeLists.txt files."}


#define CMAKE_STANDARD_INTRODUCTION \
  {0, \
   "CMake is a cross-platform build system generator.  Projects " \
   "specify their build process with platform-independent CMake listfiles " \
   "included in each directory of a source tree with the name " \
   "CMakeLists.txt. " \
   "Users build a project by using CMake to generate a build system " \
   "for a native tool on their platform.", 0}
#endif
