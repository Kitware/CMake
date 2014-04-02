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

#include "cmCacheManager.h"
#include "cmExecutionStatus.h"
#include "cmListFileCache.h"
#include "cmPolicies.h"
#include "cmPropertyMap.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmNewLineStyle.h"
#include "cmGeneratorTarget.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmSourceGroup.h"
#endif

#include <cmsys/auto_ptr.hxx>
#include <cmsys/RegularExpression.hxx>
#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cmsys/hash_map.hxx>
#endif

class cmFunctionBlocker;
class cmCommand;
class cmInstallGenerator;
class cmLocalGenerator;
class cmMakeDepend;
class cmSourceFile;
class cmTest;
class cmTestGenerator;
class cmVariableWatch;
class cmake;
class cmMakefileCall;
class cmCMakePolicyCommand;

/** \class cmMakefile
 * \brief Process the input CMakeLists.txt file.
 *
 * Process and store into memory the input CMakeLists.txt file.
 * Each CMakeLists.txt file is parsed and the commands found there
 * are added into the build process.
 */
class cmMakefile
{
  class Internals;
  cmsys::auto_ptr<Internals> Internal;
public:
  /**
   * Return the major and minor version of the cmake that
   * was used to write the currently loaded cache, note
   * this method will not work before the cache is loaded.
   */
  unsigned int GetCacheMajorVersion() const;
  unsigned int GetCacheMinorVersion() const;

  /* Check for unused variables in this scope */
  void CheckForUnusedVariables() const;
  /* Mark a variable as used */
  void MarkVariableAsUsed(const char* var);
  /* return true if a variable has been initialized */
  bool VariableInitialized(const char* ) const;
  /* return true if a variable has been used */
  bool VariableUsed(const char* ) const;
  /** Return whether compatibility features needed for a version of
      the cache or lower should be enabled.  */
  bool NeedCacheCompatibility(int major, int minor) const;

  /**
   * Construct an empty makefile.
   */
  cmMakefile();
  cmMakefile(const cmMakefile& mf);

  /**
   * Destructor.
   */
  ~cmMakefile();

  /**
   * Read and parse a CMakeLists.txt file.
   */
  bool ReadListFile(const char* listfile,
                    const char* external= 0,
                    std::string* fullPath= 0,
                    bool noPolicyScope = true);

  /**
   * Add a function blocker to this makefile
   */
  void AddFunctionBlocker(cmFunctionBlocker* fb);

  /**
   * Remove the function blocker whose scope ends with the given command.
   * This returns ownership of the function blocker object.
   */
  cmsys::auto_ptr<cmFunctionBlocker>
  RemoveFunctionBlocker(cmFunctionBlocker* fb, const cmListFileFunction& lff);

  /** Push/pop a lexical (function blocker) barrier automatically.  */
  class LexicalPushPop
  {
  public:
    LexicalPushPop(cmMakefile* mf);
    ~LexicalPushPop();
    void Quiet() { this->ReportError = false; }
  private:
    cmMakefile* Makefile;
    bool ReportError;
  };
  friend class LexicalPushPop;

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  int TryCompile(const char *srcdir, const char *bindir,
                 const char *projectName, const char *targetName,
                 bool fast,
                 const std::vector<std::string> *cmakeArgs,
                 std::string *output);

  bool GetIsSourceFileTryCompile() const;

  /**
   * Specify the makefile generator. This is platform/compiler
   * dependent, although the interface is through a generic
   * superclass.
   */
  void SetLocalGenerator(cmLocalGenerator*);

  ///! Get the current makefile generator.
  cmLocalGenerator* GetLocalGenerator() const
    { return this->LocalGenerator;}

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

  /**
   * Print the object state to std::cout.
   */
  void Print() const;

  /** Add a custom command to the build.  */
  void AddCustomCommandToTarget(const char* target,
                                const std::vector<std::string>& depends,
                                const cmCustomCommandLines& commandLines,
                                cmTarget::CustomCommandType type,
                                const char* comment, const char* workingDir,
                                bool escapeOldStyle = true) const;
  cmSourceFile* AddCustomCommandToOutput(
    const std::vector<std::string>& outputs,
    const std::vector<std::string>& depends,
    const char* main_dependency,
    const cmCustomCommandLines& commandLines,
    const char* comment, const char* workingDir,
    bool replace = false,
    bool escapeOldStyle = true);
  cmSourceFile* AddCustomCommandToOutput(
    const char* output,
    const std::vector<std::string>& depends,
    const char* main_dependency,
    const cmCustomCommandLines& commandLines,
    const char* comment, const char* workingDir,
    bool replace = false,
    bool escapeOldStyle = true);
  void AddCustomCommandOldStyle(const char* target,
                                const std::vector<std::string>& outputs,
                                const std::vector<std::string>& depends,
                                const char* source,
                                const cmCustomCommandLines& commandLines,
                                const char* comment);

  /**
   * Add a define flag to the build.
   */
  void AddDefineFlag(const char* definition);
  void RemoveDefineFlag(const char* definition);
  void AddCompileOption(const char* option);

  /** Create a new imported target with the name and type given.  */
  cmTarget* AddImportedTarget(const char* name, cmTarget::TargetType type,
                              bool global);

  cmTarget* AddNewTarget(cmTarget::TargetType type, const char* name);

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
  void AddUtilityCommand(const char* utilityName, bool excludeFromAll,
                         const std::vector<std::string>& depends,
                         const char* workingDirectory,
                         const char* command,
                         const char* arg1=0,
                         const char* arg2=0,
                         const char* arg3=0,
                         const char* arg4=0);
  cmTarget* AddUtilityCommand(const char* utilityName, bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle = true,
                              const char* comment = 0);

  /**
   * Add a link library to the build.
   */
  void AddLinkLibrary(const char*);
  void AddLinkLibrary(const char*, cmTarget::LinkLibraryType type);
  void AddLinkLibraryForTarget(const char *tgt, const char*,
                               cmTarget::LinkLibraryType type);
  void AddLinkDirectoryForTarget(const char *tgt, const char* d);

  /**
   * Add a link directory to the build.
   */
  void AddLinkDirectory(const char*);

  const std::vector<std::string>& GetLinkDirectories() const
    {
      return this->LinkDirectories;
    }
  void SetLinkDirectories(const std::vector<std::string>& vec)
    {
      this->LinkDirectories = vec;
    }

  /**
   * Add a subdirectory to the build.
   */
  void AddSubDirectory(const char*, bool excludeFromAll=false,
                       bool preorder = false);
  void AddSubDirectory(const char* fullSrcDir,const char *fullBinDir,
                       bool excludeFromAll, bool preorder,
                       bool immediate);

  /**
   * Configure a subdirectory
   */
  void ConfigureSubDirectory(cmLocalGenerator *);

  /**
   * Add an include directory to the build.
   */
  void AddIncludeDirectories(const std::vector<std::string> &incs,
                             bool before = false);

  /**
   * Add a variable definition to the build. This variable
   * can be used in CMake to refer to lists, directories, etc.
   */
  void AddDefinition(const char* name, const char* value);
  ///! Add a definition to this makefile and the global cmake cache.
  void AddCacheDefinition(const char* name, const char* value,
                          const char* doc,
                          cmCacheManager::CacheEntryType type,
                          bool force = false);

  /**
   * Add bool variable definition to the build.
   */
  void AddDefinition(const char* name, bool);

  /**
   * Remove a variable definition from the build.  This is not valid
   * for cache entries, and will only affect the current makefile.
   */
  void RemoveDefinition(const char* name);
  ///! Remove a definition from the cache.
  void RemoveCacheDefinition(const char* name);

  /**
   * Specify the name of the project for this build.
   */
  void SetProjectName(const char*);

  /**
   * Get the name of the project for this build.
   */
  const char* GetProjectName() const
    {
      return this->ProjectName.c_str();
    }

  /** Get the configurations to be generated.  */
  const char* GetConfigurations(std::vector<std::string>& configs,
                                bool single = true) const;

  /**
   * Set the name of the library.
   */
  cmTarget* AddLibrary(const char *libname, cmTarget::TargetType type,
                  const std::vector<std::string> &srcs,
                  bool excludeFromAll = false);
  void AddAlias(const char *libname, cmTarget *tgt);

#if defined(CMAKE_BUILD_WITH_CMAKE)
  /**
   * Add a root source group for consideration when adding a new source.
   */
  void AddSourceGroup(const char* name, const char* regex=0);

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
    PolicyPushPop(cmMakefile* m,
                  bool weak = false,
                  cmPolicies::PolicyMap const& pm = cmPolicies::PolicyMap());
    ~PolicyPushPop();
    void Quiet() { this->ReportError = false; }
  private:
    cmMakefile* Makefile;
    bool ReportError;
  };
  friend class PolicyPushPop;

  /**
    * Get the Policies Instance
    */
 cmPolicies *GetPolicies() const;

  /**
   * Add an auxiliary directory to the build.
   */
  void AddExtraDirectory(const char* dir);


  /**
   * Add an auxiliary directory to the build.
   */
  void MakeStartDirectoriesCurrent()
    {
      this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR",
                          this->cmStartDirectory.c_str());
      this->AddDefinition("CMAKE_CURRENT_BINARY_DIR",
                          this->StartOutputDirectory.c_str());
    }

  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is where
   * CMakeSetup or configure was run. Remember that CMake processes
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

  /**
   * Set CMAKE_SCRIPT_MODE_FILE variable when running a -P script.
   */
  void SetScriptModeFile(const char* scriptfile);

  /**
   * Set CMAKE_ARGC, CMAKE_ARGV0 ... variables.
   */
  void SetArgcArgv(const std::vector<std::string>& args);

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
      this->cmStartDirectory =
        cmSystemTools::CollapseFullPath(this->cmStartDirectory.c_str());
      this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR",
                          this->cmStartDirectory.c_str());
    }
  const char* GetStartDirectory() const
    {
      return this->cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      this->StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(this->StartOutputDirectory);
      this->StartOutputDirectory =
        cmSystemTools::CollapseFullPath(this->StartOutputDirectory.c_str());
      cmSystemTools::MakeDirectory(this->StartOutputDirectory.c_str());
      this->AddDefinition("CMAKE_CURRENT_BINARY_DIR",
                          this->StartOutputDirectory.c_str());
    }
  const char* GetStartOutputDirectory() const
    {
      return this->StartOutputDirectory.c_str();
    }
  //@}

  const char* GetCurrentDirectory() const
    {
      return this->cmStartDirectory.c_str();
    }
  const char* GetCurrentOutputDirectory() const
    {
      return this->StartOutputDirectory.c_str();
    }

  /* Get the current CMakeLists.txt file that is being processed.  This
   * is just used in order to be able to 'branch' from one file to a second
   * transparently */
  const char* GetCurrentListFile() const
    {
      return this->cmCurrentListFile.c_str();
    }

  //@}

  /**
   * Set a regular expression that include files must match
   * in order to be considered as part of the depend information.
   */
  void SetIncludeRegularExpression(const char* regex)
    {
      this->IncludeFileRegularExpression = regex;
    }
  const char* GetIncludeRegularExpression() const
    {
      return this->IncludeFileRegularExpression.c_str();
    }

  /**
   * Set a regular expression that include files that are not found
   * must match in order to be considered a problem.
   */
  void SetComplainRegularExpression(const char* regex)
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

  const cmGeneratorTargetsType &GetGeneratorTargets() const
    {
      return this->GeneratorTargets;
    }

  void SetGeneratorTargets(const cmGeneratorTargetsType &targets)
    {
      this->GeneratorTargets = targets;
    }

  cmTarget* FindTarget(const std::string& name,
                       bool excludeAliases = false) const;

  /** Find a target to use in place of the given name.  The target
      returned may be imported or built within the project.  */
  cmTarget* FindTargetToUse(const std::string& name,
                            bool excludeAliases = false) const;
  bool IsAlias(const std::string& name) const;
  cmGeneratorTarget* FindGeneratorTargetToUse(const char* name) const;

  /**
   * Mark include directories as system directories.
   */
  void AddSystemIncludeDirectories(const std::set<cmStdString> &incs);

  /** Expand out any arguements in the vector that have ; separated
   *  strings into multiple arguements.  A new vector is created
   *  containing the expanded versions of all arguments in argsIn.
   * This method differes from the one in cmSystemTools in that if
   * the CmakeLists file is version 1.2 or earlier it will check for
   * source lists being used without ${} around them
   */
  void ExpandSourceListArguments(std::vector<std::string> const& argsIn,
                                 std::vector<std::string>& argsOut,
                                 unsigned int startArgumentIndex) const;

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmSourceFile* GetSource(const char* sourceName) const;

  /** Get a cmSourceFile pointer for a given source name, if the name is
   *  not found, then create the source file and return it. generated
   * indicates if it is a generated file, this is used in determining
   * how to create the source file instance e.g. name
   */
  cmSourceFile* GetOrCreateSource(const char* sourceName,
                                  bool generated = false);

  /**
   * Obtain a list of auxiliary source directories.
   */
  const std::vector<std::string>& GetAuxSourceDirectories() const
    {return this->AuxSourceDirectories;}

  //@{
  /**
   * Return a list of extensions associated with source and header
   * files
   */
  const std::vector<std::string>& GetSourceExtensions() const
    {return this->SourceFileExtensions;}
  const std::vector<std::string>& GetHeaderExtensions() const
    {return this->HeaderFileExtensions;}
  //@}

  /**
   * Given a variable name, return its value (as a string).
   * If the variable is not found in this makefile instance, the
   * cache is then queried.
   */
  const char* GetDefinition(const char*) const;
  const char* GetSafeDefinition(const char*) const;
  const char* GetRequiredDefinition(const char* name) const;
  bool IsDefinitionSet(const char*) const;
  /**
   * Get the list of all variables in the current space. If argument
   * cacheonly is specified and is greater than 0, then only cache
   * variables will be listed.
   */
  std::vector<std::string> GetDefinitions(int cacheonly=0) const;

  /** Test a boolean cache entry to see if it is true or false,
   *  returns false if no entry defined.
   */
  bool IsOn(const char* name) const;
  bool IsSet(const char* name) const;

  /** Return whether the target platform is 64-bit.  */
  bool PlatformIs64Bit() const;

  /** Retrieve soname flag for the specified language if supported */
  const char* GetSONameFlag(const char* language) const;

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

  std::string GetListFileStack() const;

  /**
   * Get the current context backtrace.
   */
  bool GetBacktrace(cmListFileBacktrace& backtrace) const;

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
                                      bool replaceAt = true) const;

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

  /** Check if a command exists. */
  bool CommandExists(const char* name) const;

  /**
   * Add a command to this cmake instance
   */
  void AddCommand(cmCommand* );

  ///! Enable support for named language, if nil then all languages are
  ///enabled.
  void EnableLanguage(std::vector<std::string>const& languages, bool optional);

  /**
   * Set/Get the name of the parent directories CMakeLists file
   * given a current CMakeLists file name
   */
  cmCacheManager *GetCacheManager() const;

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
                       std::vector<std::string>& outArgs) const;
  /**
   * Get the instance
   */
  cmake *GetCMakeInstance() const;

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
  cmSourceFile *GetSourceFileWithOutput(const char *outName) const;

  /**
   * Add a macro to the list of macros. The arguments should be name of the
   * macro and a documentation signature of it
   */
  void AddMacro(const char* name, const char* signature);

  ///! Add a new cmTest to the list of tests for this makefile.
  cmTest* CreateTest(const char* testName);

  /** Get a cmTest pointer for a given test name, if the name is
   *  not found, then a null pointer is returned.
   */
  cmTest* GetTest(const char* testName) const;

  /**
   * Get a list of macros as a ; separated string
   */
  void GetListOfMacros(std::string& macros) const;

  /**
   * Return a location of a file in cmake or custom modules directory
   */
  std::string GetModulesFile(const char* name) const;

  ///! Set/Get a property of this directory
  void SetProperty(const char *prop, const char *value);
  void AppendProperty(const char *prop, const char *value,bool asString=false);
  const char *GetProperty(const char *prop) const;
  const char *GetPropertyOrDefinition(const char *prop) const;
  const char *GetProperty(const char *prop, cmProperty::ScopeType scope) const;
  bool GetPropertyAsBool(const char *prop) const;

  const char* GetFeature(const char* feature, const char* config);

  // Get the properties
  cmPropertyMap &GetProperties() { return this->Properties; };

  ///! Initialize a makefile from its parent
  void InitializeFromParent();

  ///! Set/Get the preorder flag
  void SetPreOrder(bool p) { this->PreOrder = p; }
  bool GetPreOrder() const { return this->PreOrder; }

  void AddInstallGenerator(cmInstallGenerator* g)
    { if(g) this->InstallGenerators.push_back(g); }
  std::vector<cmInstallGenerator*>& GetInstallGenerators()
    { return this->InstallGenerators; }

  void AddTestGenerator(cmTestGenerator* g)
    { if(g) this->TestGenerators.push_back(g); }
  const std::vector<cmTestGenerator*>& GetTestGenerators() const
    { return this->TestGenerators; }

  // Define the properties
  static void DefineProperties(cmake *cm);

  // push and pop variable scopes
  void PushScope();
  void PopScope();
  void RaiseScope(const char *var, const char *value);

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
                    std::string const& text) const;

  /** Set whether or not to report a CMP0000 violation.  */
  void SetCheckCMP0000(bool b) { this->CheckCMP0000 = b; }

  const std::vector<cmValueWithOrigin>& GetIncludeDirectoriesEntries() const
  {
    return this->IncludeDirectoriesEntries;
  }
  const std::vector<cmValueWithOrigin>& GetCompileOptionsEntries() const
  {
    return this->CompileOptionsEntries;
  }
  const std::vector<cmValueWithOrigin>& GetCompileDefinitionsEntries() const
  {
    return this->CompileDefinitionsEntries;
  }

  bool IsGeneratingBuildSystem() const { return this->GeneratingBuildSystem; }
  void SetGeneratingBuildSystem(){ this->GeneratingBuildSystem = true; }

  void AddQtUiFileWithOptions(cmSourceFile *sf);
  std::vector<cmSourceFile*> GetQtUiFilesWithOptions() const;

  std::set<cmStdString> const & GetSystemIncludeDirectories() const
    { return this->SystemIncludeDirectories; }

  bool PolicyOptionalWarningEnabled(std::string const& var);

protected:
  // add link libraries and directories to the target
  void AddGlobalLinkInformation(const char* name, cmTarget& target);

  // Check for a an unused variable
  void CheckForUnused(const char* reason, const char* name) const;

  std::string Prefix;
  std::vector<std::string> AuxSourceDirectories; //

  std::string cmStartDirectory;
  std::string StartOutputDirectory;
  std::string cmHomeDirectory;
  std::string HomeOutputDirectory;
  std::string cmCurrentListFile;

  std::string ProjectName;    // project name

  // libraries, classes, and executables
  mutable cmTargets Targets;
  std::map<std::string, cmTarget*> AliasTargets;
  cmGeneratorTargetsType GeneratorTargets;
  std::vector<cmSourceFile*> SourceFiles;

  // Tests
  std::map<cmStdString, cmTest*> Tests;

  // The link-library paths.  Order matters, use std::vector (not std::set).
  std::vector<std::string> LinkDirectories;

  // The set of include directories that are marked as system include
  // directories.
  std::set<cmStdString> SystemIncludeDirectories;

  std::vector<std::string> ListFiles; // list of command files loaded
  std::vector<std::string> OutputFiles; // list of command files loaded


  cmTarget::LinkLibraryVectorType LinkLibraries;

  std::vector<cmInstallGenerator*> InstallGenerators;
  std::vector<cmTestGenerator*> TestGenerators;

  std::string IncludeFileRegularExpression;
  std::string ComplainFileRegularExpression;
  std::vector<std::string> SourceFileExtensions;
  std::vector<std::string> HeaderFileExtensions;
  std::string DefineFlags;

  std::vector<cmValueWithOrigin> IncludeDirectoriesEntries;
  std::vector<cmValueWithOrigin> CompileOptionsEntries;
  std::vector<cmValueWithOrigin> CompileDefinitionsEntries;

  // Track the value of the computed DEFINITIONS property.
  void AddDefineFlag(const char*, std::string&);
  void RemoveDefineFlag(const char*, std::string::size_type, std::string&);
  std::string DefineFlagsOrig;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  std::vector<cmSourceGroup> SourceGroups;
#endif

  std::vector<cmCommand*> FinalPassCommands;
  cmLocalGenerator* LocalGenerator;
  bool IsFunctionBlocked(const cmListFileFunction& lff,
                         cmExecutionStatus &status);

private:
  void Initialize();

  bool ParseDefineFlag(std::string const& definition, bool remove);

  bool EnforceUniqueDir(const char* srcPath, const char* binPath) const;

  friend class cmMakeDepend;    // make depend needs direct access
                                // to the Sources array
  void PrintStringVector(const char* s, const
                         std::vector<std::pair<cmStdString, bool> >& v) const;
  void PrintStringVector(const char* s,
                         const std::vector<std::string>& v) const;

  void AddDefaultDefinitions();
  typedef std::vector<cmFunctionBlocker*> FunctionBlockersType;
  FunctionBlockersType FunctionBlockers;
  std::vector<FunctionBlockersType::size_type> FunctionBlockerBarriers;
  void PushFunctionBlockerBarrier();
  void PopFunctionBlockerBarrier(bool reportError = true);

  typedef std::map<cmStdString, cmStdString> StringStringMap;
  StringStringMap MacrosMap;

  std::map<cmStdString, bool> SubDirectoryOrder;

  mutable cmsys::RegularExpression cmDefineRegex;
  mutable cmsys::RegularExpression cmDefine01Regex;
  mutable cmsys::RegularExpression cmAtVarRegex;

  cmPropertyMap Properties;

  // should this makefile be processed before or after processing the parent
  bool PreOrder;

  // Unused variable flags
  bool WarnUnused;
  bool CheckSystemVars;

  // stack of list files being read
  std::deque<cmStdString> ListFileStack;

  // stack of commands being invoked.
  struct CallStackEntry
  {
    cmListFileContext const* Context;
    cmExecutionStatus* Status;
  };
  typedef std::deque<CallStackEntry> CallStackType;
  CallStackType CallStack;
  friend class cmMakefileCall;

  std::vector<cmTarget*> ImportedTargetsOwned;
  std::map<cmStdString, cmTarget*> ImportedTargets;

  // Internal policy stack management.
  void PushPolicy(bool weak = false,
                  cmPolicies::PolicyMap const& pm = cmPolicies::PolicyMap());
  void PopPolicy();
  void PushPolicyBarrier();
  void PopPolicyBarrier(bool reportError = true);
  friend class cmCMakePolicyCommand;
  class IncludeScope;
  friend class IncludeScope;

  // stack of policy settings
  struct PolicyStackEntry: public cmPolicies::PolicyMap
  {
    typedef cmPolicies::PolicyMap derived;
    PolicyStackEntry(bool w = false): derived(), Weak(w) {}
    PolicyStackEntry(derived const& d, bool w = false): derived(d), Weak(w) {}
    PolicyStackEntry(PolicyStackEntry const& r): derived(r), Weak(r.Weak) {}
    bool Weak;
  };
  typedef std::vector<PolicyStackEntry> PolicyStackType;
  PolicyStackType PolicyStack;
  std::vector<PolicyStackType::size_type> PolicyBarriers;
  cmPolicies::PolicyStatus
  GetPolicyStatusInternal(cmPolicies::PolicyID id) const;

  bool CheckCMP0000;

  // Enforce rules about CMakeLists.txt files.
  void EnforceDirectoryLevelRules() const;

  bool GeneratingBuildSystem;
  /**
   * Old version of GetSourceFileWithOutput(const char*) kept for
   * backward-compatibility. It implements a linear search and support
   * relative file paths. It is used as a fall back by
   * GetSourceFileWithOutput(const char*).
   */
  cmSourceFile *LinearGetSourceFileWithOutput(const char *cname) const;

  // A map for fast output to input look up.
#if defined(CMAKE_BUILD_WITH_CMAKE)
  typedef cmsys::hash_map<std::string, cmSourceFile*> OutputToSourceMap;
#else
  typedef std::map<std::string, cmSourceFile*> OutputToSourceMap;
#endif
  OutputToSourceMap OutputToSource;

  void UpdateOutputToSourceMap(std::vector<std::string> const& outputs,
                               cmSourceFile* source);
  void UpdateOutputToSourceMap(std::string const& output,
                               cmSourceFile* source);

  std::vector<cmSourceFile*> QtUiFilesWithOptions;
};

//----------------------------------------------------------------------------
// Helper class to make sure the call stack is valid.
class cmMakefileCall
{
public:
  cmMakefileCall(cmMakefile* mf,
                 cmListFileContext const& lfc,
                 cmExecutionStatus& status): Makefile(mf)
    {
    cmMakefile::CallStackEntry entry = {&lfc, &status};
    this->Makefile->CallStack.push_back(entry);
    }
  ~cmMakefileCall()
    {
    this->Makefile->CallStack.pop_back();
    }
private:
  cmMakefile* Makefile;
};

#endif
