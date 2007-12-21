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

#ifndef cmGlobalGenerator_h
#define cmGlobalGenerator_h

#include "cmStandardIncludes.h"

#include "cmTarget.h" // For cmTargets

class cmake;
class cmMakefile;
class cmLocalGenerator;
class cmExternalMakefileProjectGenerator;
class cmTarget;
class cmTargetExport;
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
  
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
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
   * Try to determine system infomation, get it from another generator
   */
  virtual void EnableLanguagesFromGenerator(cmGlobalGenerator *gen);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output, cmMakefile* mf);

  
  /**
   * Build a file given the following information. This is a more direct call
   * that is used by both CTest and TryCompile. If target name is NULL or
   * empty then all is assumed. clean indicates if a "make clean" should be
   * done first.
   */
  virtual int Build(const char *srcdir, const char *bindir,
                    const char *projectName, const char *targetName,
                    std::string *output, 
                    const char *makeProgram, const char *config,
                    bool clean, bool fast,
                    double timeout);
  virtual std::string GenerateBuildCommand
  (const char* makeProgram,
   const char *projectName, const char* additionalOptions, 
   const char *targetName,
   const char* config, bool ignoreErrors, bool fast);


  ///! Set the CMake instance
  void SetCMakeInstance(cmake *cm);
  
  ///! Get the CMake instance
  cmake *GetCMakeInstance() { return this->CMakeInstance; };

  void SetConfiguredFilesPath(const char* s){this->ConfiguredFilesPath = s;}
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

  ///! Add one installed target to the sets of the exports
  void AddTargetToExports(const char* exportSet, cmTarget* target, 
                          cmInstallTargetGenerator* archive,
                          cmInstallTargetGenerator* runTime,
                          cmInstallTargetGenerator* library,
                          cmInstallTargetGenerator* framework,
                          cmInstallTargetGenerator* bundle,
                          cmInstallFilesGenerator* publicHeaders);
  ///! Get the export target set with the   given name
  const std::vector<cmTargetExport*>* GetExportSet(const char* name) const;

  void EnableInstallTarget();

  int TryCompileTimeout;
  
  bool GetForceUnixPaths() {return this->ForceUnixPaths;}
  bool GetToolSupportsColor() { return this->ToolSupportsColor; }
  void SetToolSupportsColor(bool enable) { this->ToolSupportsColor = enable; }

  ///! return the language for the given extension
  const char* GetLanguageFromExtension(const char* ext);
  ///! is an extension to be ignored
  bool IgnoreFile(const char* ext);
  ///! What is the preference for linkers and this language (None or Prefered)
  int GetLinkerPreference(const char* lang);
  ///! What is the object file extension for a given source file?
  const char* GetLanguageOutputExtension(cmSourceFile const&);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGInitDirectory()  { return "."; }

  /** Get whether the generator should use a script for link commands.  */
  bool GetUseLinkScript() { return this->UseLinkScript; }

  /** Get whether the generator should produce special marks on rules
      producing symbolic (non-file) outputs.  */
  bool GetNeedSymbolicMark() { return this->NeedSymbolicMark; }

  /*
   * Determine what program to use for building the project.
   */
  void FindMakeProgram(cmMakefile*);

  ///! Find a target by name by searching the local generators.
  cmTarget* FindTarget(const char* project, 
                       const char* name, 
                       bool useImportedTargets);

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
  cmTargetManifest const& GetTargetManifest() { return this->TargetManifest; }

  void AddTarget(cmTargets::value_type &v);

  virtual const char* GetAllTargetName()          { return "ALL_BUILD"; }
  virtual const char* GetInstallTargetName()      { return "INSTALL"; }
  virtual const char* GetInstallLocalTargetName() { return 0; }
  virtual const char* GetInstallStripTargetName() { return 0; }
  virtual const char* GetPreinstallTargetName()   { return 0; }
  virtual const char* GetTestTargetName()         { return "RUN_TESTS"; }
  virtual const char* GetPackageTargetName()      { return "PACKAGE"; }
  virtual const char* GetPackageSourceTargetName(){ return 0; }
  virtual const char* GetEditCacheTargetName()    { return 0; }
  virtual const char* GetRebuildCacheTargetName() { return 0; }
  virtual const char* GetCleanTargetName()        { return 0; }

  // what targets does the specified target depend on
  std::vector<cmTarget *>& GetTargetDepends(cmTarget& target);

  const std::map<cmStdString, std::vector<cmLocalGenerator*> >& GetProjectMap()
                                               const {return this->ProjectMap;}

  // track files replaced during a Generate
  void FileReplacedDuringGenerate(const std::string& filename);
  void GetFilesReplacedDuringGenerate(std::vector<std::string>& filenames);

protected:
  void SetLanguageEnabledFlag(const char* l, cmMakefile* mf);
  void SetLanguageEnabledMaps(const char* l, cmMakefile* mf);

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
    std::vector<std::string> depends, bool depends_on_all = false);

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
  std::map<cmLocalGenerator*, std::set<cmTarget*> > LocalGeneratorToTargetMap;

  // Set of named installation components requested by the project.
  std::set<cmStdString> InstallComponents;
  bool InstallTargetEnabled;
  // Sets of named target exports
  std::map<cmStdString, std::vector<cmTargetExport*> > ExportSets;

  // Manifest of all targets that will be built for each configuration.
  // This is computed just before local generators generate.
  cmTargetManifest TargetManifest;

private:
  float FirstTimeProgress;
  // If you add a new map here, make sure it is copied
  // in EnableLanguagesFromGenerator 
  std::map<cmStdString, bool> IgnoreExtensions;
  std::map<cmStdString, bool> LanguageEnabled;
  std::map<cmStdString, cmStdString> OutputExtensions;
  std::map<cmStdString, cmStdString> LanguageToOutputExtension;
  std::map<cmStdString, cmStdString> ExtensionToLanguage;
  std::map<cmStdString, int> LanguageToLinkerPreference; 

  // this is used to improve performance 
  std::map<cmStdString,cmTarget *> TotalTargets;
  std::map<cmStdString,cmTarget *> ImportedTotalTargets;
  
  std::map<cmStdString, std::vector<cmTarget *> > TargetDependencies;

  cmExternalMakefileProjectGenerator* ExtraGenerator;

  // track files replaced during a Generate
  std::vector<std::string> FilesReplacedDuringGenerate;
};

#endif
