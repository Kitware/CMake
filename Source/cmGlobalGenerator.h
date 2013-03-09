/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmGlobalGenerator_h
#define cmGlobalGenerator_h

#include "cmStandardIncludes.h"

#include "cmTarget.h" // For cmTargets
#include "cmTargetDepend.h" // For cmTargetDependSet
#include "cmSystemTools.h" // for cmSystemTools::OutputOption
#include "cmExportSetMap.h" // For cmExportSetMap
#include "cmGeneratorTarget.h"

class cmake;
class cmGeneratorTarget;
class cmMakefile;
class cmLocalGenerator;
class cmExternalMakefileProjectGenerator;
class cmTarget;
class cmInstallTargetGenerator;
class cmInstallFilesGenerator;

/** \class cmGlobalGenerator
 * \brief Responable for overseeing the generation process for the entire tree
 *
 * Subclasses of this class generate makefiles for various
 * platforms.
 */
class cmGlobalGenerator
{
public:
  ///! Free any memory allocated with the GlobalGenerator
  cmGlobalGenerator();
  virtual ~cmGlobalGenerator();

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  ///! Get the name for this generator
  virtual const char *GetName() const { return "Generic"; };

  /** Set the generator-specific toolset name.  Returns true if toolset
      is supported and false otherwise.  */
  virtual bool SetGeneratorToolset(std::string const& ts);

  /**
   * Create LocalGenerators and process the CMakeLists files. This does not
   * actually produce any makefiles, DSPs, etc.
   */
  virtual void Configure();

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();

  /**
   * Set/Get and Clear the enabled languages.
   */
  void SetLanguageEnabled(const char*, cmMakefile* mf);
  bool GetLanguageEnabled(const char*) const;
  void ClearEnabledLanguages();
  void GetEnabledLanguages(std::vector<std::string>& lang);
  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  /**
   * Resolve the CMAKE_<lang>_COMPILER setting for the given language.
   * Intended to be called from EnableLanguage.
   */
  void ResolveLanguageCompiler(const std::string &lang, cmMakefile *mf,
                               bool optional);

  /**
   * Try to determine system infomation, get it from another generator
   */
  virtual void EnableLanguagesFromGenerator(cmGlobalGenerator *gen,
                                            cmMakefile* mf);

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         bool fast, std::string *output, cmMakefile* mf);


  /**
   * Build a file given the following information. This is a more direct call
   * that is used by both CTest and TryCompile. If target name is NULL or
   * empty then all is assumed. clean indicates if a "make clean" should be
   * done first.
   */
  int Build(const char *srcdir, const char *bindir,
            const char *projectName, const char *targetName,
            std::string *output,
            const char *makeProgram, const char *config,
            bool clean, bool fast,
            double timeout,
            cmSystemTools::OutputOption outputflag=cmSystemTools::OUTPUT_NONE,
            const char* extraOptions = 0,
            std::vector<std::string> const& nativeOptions =
            std::vector<std::string>());

  virtual std::string GenerateBuildCommand(
    const char* makeProgram,
    const char *projectName, const char* additionalOptions,
    const char *targetName,
    const char* config, bool ignoreErrors, bool fast);


  ///! Set the CMake instance
  void SetCMakeInstance(cmake *cm);

  ///! Get the CMake instance
  cmake *GetCMakeInstance() { return this->CMakeInstance; }
  const cmake *GetCMakeInstance() const { return this->CMakeInstance; }

  void SetConfiguredFilesPath(cmGlobalGenerator* gen);
  const std::vector<cmLocalGenerator *>& GetLocalGenerators() const {
    return this->LocalGenerators;}

  cmLocalGenerator* GetCurrentLocalGenerator()
                                          {return this->CurrentLocalGenerator;}

  void SetCurrentLocalGenerator(cmLocalGenerator* lg)
                                            {this->CurrentLocalGenerator = lg;}

  void AddLocalGenerator(cmLocalGenerator *lg);

  ///! Set an generator for an "external makefile based project"
  void SetExternalMakefileProjectGenerator(
                           cmExternalMakefileProjectGenerator *extraGenerator);

  const char* GetExtraGeneratorName() const;

  void AddInstallComponent(const char* component);

  const std::set<cmStdString>* GetInstallComponents() const
    { return &this->InstallComponents; }

  cmExportSetMap& GetExportSets() {return this->ExportSets;}

  /** Add a file to the manifest of generated targets for a configuration.  */
  void AddToManifest(const char* config, std::string const& f);

  void EnableInstallTarget();

  int TryCompileTimeout;

  bool GetForceUnixPaths() const { return this->ForceUnixPaths; }
  bool GetToolSupportsColor() const { return this->ToolSupportsColor; }

  ///! return the language for the given extension
  const char* GetLanguageFromExtension(const char* ext);
  ///! is an extension to be ignored
  bool IgnoreFile(const char* ext);
  ///! What is the preference for linkers and this language (None or Prefered)
  int GetLinkerPreference(const char* lang);
  ///! What is the object file extension for a given source file?
  const char* GetLanguageOutputExtension(cmSourceFile const&);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGIntDir() const { return "."; }

  /** Get whether the generator should use a script for link commands.  */
  bool GetUseLinkScript() const { return this->UseLinkScript; }

  /** Get whether the generator should produce special marks on rules
      producing symbolic (non-file) outputs.  */
  bool GetNeedSymbolicMark() const { return this->NeedSymbolicMark; }

  /*
   * Determine what program to use for building the project.
   */
  void FindMakeProgram(cmMakefile*);

  ///! Find a target by name by searching the local generators.
  cmTarget* FindTarget(const char* project, const char* name);

  /** Determine if a name resolves to a framework on disk or a built target
      that is a framework. */
  bool NameResolvesToFramework(const std::string& libname);

  /** If check to see if the target is linked to by any other
      target in the project */
  bool IsDependedOn(const char* project, cmTarget* target);
  ///! Find a local generator by its startdirectory
  cmLocalGenerator* FindLocalGenerator(const char* start_dir);

  /** Append the subdirectory for the given configuration.  If anything is
      appended the given prefix and suffix will be appended around it, which
      is useful for leading or trailing slashes.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  /** Get the manifest of all targets that will be built for each
      configuration.  This is valid during generation only.  */
  cmTargetManifest const& GetTargetManifest() const
    { return this->TargetManifest; }

  /** Get the content of a directory.  Directory listings are loaded
      from disk at most once and cached.  During the generation step
      the content will include the target files to be built even if
      they do not yet exist.  */
  std::set<cmStdString> const& GetDirectoryContent(std::string const& dir,
                                                   bool needDisk = true);

  void AddTarget(cmTarget* t);

  virtual const char* GetAllTargetName()         const { return "ALL_BUILD"; }
  virtual const char* GetInstallTargetName()       const { return "INSTALL"; }
  virtual const char* GetInstallLocalTargetName()  const { return 0; }
  virtual const char* GetInstallStripTargetName()  const { return 0; }
  virtual const char* GetPreinstallTargetName()    const { return 0; }
  virtual const char* GetTestTargetName()        const { return "RUN_TESTS"; }
  virtual const char* GetPackageTargetName()       const { return "PACKAGE"; }
  virtual const char* GetPackageSourceTargetName() const { return 0; }
  virtual const char* GetEditCacheTargetName()     const { return 0; }
  virtual const char* GetRebuildCacheTargetName()  const { return 0; }
  virtual const char* GetCleanTargetName()         const { return 0; }

  // Class to track a set of dependencies.
  typedef cmTargetDependSet TargetDependSet;

  // what targets does the specified target depend on directly
  // via a target_link_libraries or add_dependencies
  TargetDependSet const& GetTargetDirectDepends(cmTarget & target);

  /** Get per-target generator information.  */
  cmGeneratorTarget* GetGeneratorTarget(cmTarget*) const;

  const std::map<cmStdString, std::vector<cmLocalGenerator*> >& GetProjectMap()
                                               const {return this->ProjectMap;}

  // track files replaced during a Generate
  void FileReplacedDuringGenerate(const std::string& filename);
  void GetFilesReplacedDuringGenerate(std::vector<std::string>& filenames);

  void AddRuleHash(const std::vector<std::string>& outputs,
                   std::string const& content);

  /** Return whether the given binary directory is unused.  */
  bool BinaryDirectoryIsNew(const char* dir)
    {
    return this->BinaryDirectories.insert(dir).second;
    }
  /** Supported systems creates a GUID for the given name */
  virtual void CreateGUID(const char*) {}

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  virtual bool IsMultiConfig() { return false; }

  std::string GetSharedLibFlagsForLanguage(std::string const& lang);

  /** Generate an <output>.rule file path for a given command output.  */
  virtual std::string GenerateRuleFile(std::string const& output) const;

  static std::string EscapeJSON(const std::string& s);

protected:
  typedef std::vector<cmLocalGenerator*> GeneratorVector;
  // for a project collect all its targets by following depend
  // information, and also collect all the targets
  virtual void GetTargetSets(TargetDependSet& projectTargets,
                             TargetDependSet& originalTargets,
                             cmLocalGenerator* root, GeneratorVector const&);
  virtual bool IsRootOnlyTarget(cmTarget* target);
  void AddTargetDepends(cmTarget* target, TargetDependSet& projectTargets);
  void SetLanguageEnabledFlag(const char* l, cmMakefile* mf);
  void SetLanguageEnabledMaps(const char* l, cmMakefile* mf);
  void FillExtensionToLanguageMap(const char* l, cmMakefile* mf);

  virtual bool ComputeTargetDepends();

  virtual bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS();

  bool CheckTargets();
  void CreateAutomocTargets();


  // Fill the ProjectMap, this must be called after LocalGenerators
  // has been populated.
  void FillProjectMap();
  void CheckLocalGenerators();
  bool IsExcluded(cmLocalGenerator* root, cmLocalGenerator* gen);
  bool IsExcluded(cmLocalGenerator* root, cmTarget& target);
  void FillLocalGeneratorToTargetMap();
  void CreateDefaultGlobalTargets(cmTargets* targets);
  cmTarget CreateGlobalTarget(const char* name, const char* message,
    const cmCustomCommandLines* commandLines,
    std::vector<std::string> depends, const char* workingDir);

  bool NeedSymbolicMark;
  bool UseLinkScript;
  bool ForceUnixPaths;
  bool ToolSupportsColor;
  cmStdString FindMakeProgramFile;
  cmStdString ConfiguredFilesPath;
  cmake *CMakeInstance;
  std::vector<cmLocalGenerator *> LocalGenerators;
  cmLocalGenerator* CurrentLocalGenerator;
  // map from project name to vector of local generators in that project
  std::map<cmStdString, std::vector<cmLocalGenerator*> > ProjectMap;
  std::map<cmLocalGenerator*, std::set<cmTarget *> > LocalGeneratorToTargetMap;

  // Set of named installation components requested by the project.
  std::set<cmStdString> InstallComponents;
  bool InstallTargetEnabled;
  // Sets of named target exports
  cmExportSetMap ExportSets;

  // Manifest of all targets that will be built for each configuration.
  // This is computed just before local generators generate.
  cmTargetManifest TargetManifest;

  // All targets in the entire project.
  std::map<cmStdString,cmTarget *> TotalTargets;
  std::map<cmStdString,cmTarget *> ImportedTargets;

  virtual const char* GetPredefinedTargetsFolder();
  virtual bool UseFolderProperty();
  void EnableMinGWLanguage(cmMakefile *mf);

private:
  cmMakefile* TryCompileOuterMakefile;
  float FirstTimeProgress;
  // If you add a new map here, make sure it is copied
  // in EnableLanguagesFromGenerator
  std::map<cmStdString, bool> IgnoreExtensions;
  std::map<cmStdString, bool> LanguageEnabled;
  std::set<cmStdString> LanguagesReady; // Ready for try_compile
  std::map<cmStdString, cmStdString> OutputExtensions;
  std::map<cmStdString, cmStdString> LanguageToOutputExtension;
  std::map<cmStdString, cmStdString> ExtensionToLanguage;
  std::map<cmStdString, int> LanguageToLinkerPreference;
  std::map<cmStdString, cmStdString> LanguageToOriginalSharedLibFlags;

  // Record hashes for rules and outputs.
  struct RuleHash { char Data[32]; };
  std::map<cmStdString, RuleHash> RuleHashes;
  void CheckRuleHashes();
  void CheckRuleHashes(std::string const& pfile, std::string const& home);
  void WriteRuleHashes(std::string const& pfile);

  void WriteSummary();
  void WriteSummary(cmTarget* target);

  cmExternalMakefileProjectGenerator* ExtraGenerator;

  // track files replaced during a Generate
  std::vector<std::string> FilesReplacedDuringGenerate;

  // Store computed inter-target dependencies.
  typedef std::map<cmTarget *, TargetDependSet> TargetDependMap;
  TargetDependMap TargetDependencies;

  // Per-target generator information.
  cmGeneratorTargetsType GeneratorTargets;
  void CreateGeneratorTargets();
  void ClearGeneratorTargets();
  virtual void ComputeTargetObjects(cmGeneratorTarget* gt) const;

  // Cache directory content and target files to be built.
  struct DirectoryContent: public std::set<cmStdString>
  {
    typedef std::set<cmStdString> derived;
    bool LoadedFromDisk;
    DirectoryContent(): LoadedFromDisk(false) {}
    DirectoryContent(DirectoryContent const& dc):
      derived(dc), LoadedFromDisk(dc.LoadedFromDisk) {}
  };
  std::map<cmStdString, DirectoryContent> DirectoryContentMap;

  // Set of binary directories on disk.
  std::set<cmStdString> BinaryDirectories;
};

#endif
