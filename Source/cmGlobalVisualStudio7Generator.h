/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio7Generator_h
#define cmGlobalVisualStudio7Generator_h

#include "cmGlobalVisualStudioGenerator.h"
#include "cmGlobalGeneratorFactory.h"

class cmTarget;
struct cmIDEFlagTable;

/** \class cmGlobalVisualStudio7Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio7Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio7Generator : public cmGlobalVisualStudioGenerator
{
public:
  cmGlobalVisualStudio7Generator(const char* platformName = NULL);
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalVisualStudio7Generator>(); }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio7Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 7";}

  ///! Get the name for the platform.
  const char* GetPlatformName() const { return this->PlatformName.c_str(); }

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  virtual void AddPlatformDefinitions(cmMakefile* mf);

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
  virtual std::string GenerateBuildCommand(const char* makeProgram,
                                           const char *projectName,
                                           const char *projectDir,
                                           const char* additionalOptions,
                                           const char *targetName,
                                           const char* config,
                                           bool ignoreErrors,
                                           bool fast);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.
   */
  virtual void Generate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputSLNFile();

  /**
   * Get the list of configurations
   */
  std::vector<std::string> *GetConfigurations();

  ///! Create a GUID or get an existing one.
  void CreateGUID(const char* name);
  std::string GetGUID(const char* name);

  /** Append the subdirectory for the given configuration.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGIntDir() const { return "$(OutDir)"; }

  /** Return true if the target project file should have the option
      LinkLibraryDependencies and link to .sln dependencies. */
  virtual bool NeedLinkLibraryDependencies(cmTarget&) { return false; }

  std::string const& GetIntelProjectVersion() const
    { return this->IntelProjectVersion; }
protected:
  virtual const char* GetIDEVersion() { return "7.0"; }

  static cmIDEFlagTable const* GetExtraFlagTableVS7();
  virtual void OutputSLNFile(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  virtual void WriteSLNFile(std::ostream& fout, cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  virtual void WriteProject(std::ostream& fout,
                            const char* name, const char* path, cmTarget &t);
  virtual void WriteProjectDepends(std::ostream& fout,
                           const char* name, const char* path, cmTarget &t);
  virtual void WriteProjectConfigurations(
    std::ostream& fout, const char* name, cmTarget::TargetType type,
    const std::set<std::string>& configsPartOfDefaultBuild,
    const char* platformMapping = NULL);
  virtual void WriteSLNGlobalSections(std::ostream& fout,
                                      cmLocalGenerator* root);
  virtual void WriteSLNFooter(std::ostream& fout);
  virtual void WriteSLNHeader(std::ostream& fout);
  virtual std::string WriteUtilityDepend(cmTarget* target);

  virtual void WriteTargetsToSolution(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets);
  virtual void WriteTargetDepends(
    std::ostream& fout,
    OrderedTargetDependSet const& projectTargets);
  virtual void WriteTargetConfigurations(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets);

  void GenerateConfigurations(cmMakefile* mf);

  virtual void WriteExternalProject(std::ostream& fout,
                                    const char* name,
                                    const char* path,
                                    const char* typeGuid,
                                    const std::set<cmStdString>&
                                    dependencies);

  std::string ConvertToSolutionPath(const char* path);

  std::set<std::string> IsPartOfDefaultBuild(const char* project,
                                             cmTarget* target);
  std::vector<std::string> Configurations;
  std::map<cmStdString, cmStdString> GUIDMap;

  virtual void WriteFolders(std::ostream& fout);
  virtual void WriteFoldersContent(std::ostream& fout);
  std::map<std::string,std::set<std::string> > VisualStudioFolders;

  // Set during OutputSLNFile with the name of the current project.
  // There is one SLN file per project.
  std::string CurrentProject;
  std::string PlatformName;

private:
  void InitIntelProjectVersion();
  std::string IntelProjectVersion;
};

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

#endif
