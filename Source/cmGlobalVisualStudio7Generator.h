/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include <cm3p/json/value.h>

#include "cmGlobalVisualStudioGenerator.h"
#include "cmValue.h"

class cmGeneratorTarget;
struct cmIDEFlagTable;
class cmLocalGenerator;
class cmMakefile;
class cmake;
template <typename T>
class BT;

struct cmVisualStudioFolder
{
  std::set<std::string> Projects;
  std::set<std::string> SolutionItems;
};

/** \class cmGlobalVisualStudio7Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio7Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio7Generator : public cmGlobalVisualStudioGenerator
{
public:
  ~cmGlobalVisualStudio7Generator() override;

  //! Create a local generator appropriate to this Global Generator
  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf) override;

#if !defined(CMAKE_BOOTSTRAP)
  Json::Value GetJson() const override;
#endif

  bool SetSystemName(std::string const& s, cmMakefile* mf) override;

  /**
   * Utilized by the generator factory to determine if this generator
   * supports toolsets.
   */
  static bool SupportsToolset() { return false; }

  /**
   * Utilized by the generator factory to determine if this generator
   * supports platforms.
   */
  static bool SupportsPlatform() { return false; }

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  /**
   * Try running cmake and building a file. This is used for dynamically
   * loaded commands, not as part of the usual build process.
   */
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions const& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  //! Lookup a stored GUID or compute one deterministically.
  std::string GetGUID(std::string const& name) const;

  /** Append the subdirectory for the given configuration.  */
  void AppendDirectoryForConfig(std::string const& prefix,
                                std::string const& config,
                                std::string const& suffix,
                                std::string& dir) override;

  //! What is the configurations directory variable called?
  char const* GetCMakeCFGIntDir() const override
  {
    return "$(ConfigurationName)";
  }

  /** Return true if the target project file should have the option
      LinkLibraryDependencies and link to .sln dependencies. */
  virtual bool NeedLinkLibraryDependencies(cmGeneratorTarget*)
  {
    return false;
  }

  std::string const& GetIntelProjectVersion();
  virtual cm::optional<std::string> GetPlatformToolsetFortran() const
  {
    return cm::nullopt;
  }

  bool FindMakeProgram(cmMakefile* mf) override;

  /** Is the Microsoft Assembler enabled?  */
  bool IsMarmasmEnabled() const { return this->MarmasmEnabled; }
  bool IsMasmEnabled() const { return this->MasmEnabled; }
  bool IsNasmEnabled() const { return this->NasmEnabled; }

  // Encoding for Visual Studio files
  virtual std::string Encoding();

  cmIDEFlagTable const* ExtraFlagTable;

  virtual bool SupportsCxxModuleDyndep() const { return false; }

protected:
  cmGlobalVisualStudio7Generator(cmake* cm);

  void Generate() override;

  struct VSFolders
  {
    std::map<std::string, cmVisualStudioFolder> Folders;
    cmVisualStudioFolder* Create(std::string const& path);
  };

  std::string const& GetDevEnvCommand();
  virtual std::string FindDevEnvCommand();

  static char const* ExternalProjectType(std::string const& location);

  virtual void OutputSLNFile(cmLocalGenerator* root,
                             std::vector<cmLocalGenerator*>& generators);
  virtual void WriteSLNFile(
    std::ostream& fout, cmLocalGenerator* root,
    OrderedTargetDependSet const& orderedProjectTargets,
    VSFolders const& vsFolders) const = 0;
  virtual void WriteProject(std::ostream& fout, std::string const& name,
                            std::string const& path,
                            cmGeneratorTarget const* t) const = 0;
  virtual void WriteProjectDepends(std::ostream& fout, std::string const& name,
                                   std::string const& path,
                                   cmGeneratorTarget const* t) const = 0;
  virtual void WriteProjectConfigurations(
    std::ostream& fout, std::string const& name,
    cmGeneratorTarget const& target, std::vector<std::string> const& configs,
    std::set<std::string> const& configsPartOfDefaultBuild,
    std::string const& platformMapping = "") const = 0;
  virtual void WriteSLNGlobalSections(std::ostream& fout,
                                      cmLocalGenerator* root) const;
  virtual void WriteSLNFooter(std::ostream& fout) const;
  std::string WriteUtilityDepend(cmGeneratorTarget const* target) override;

  VSFolders CreateSolutionFolders(
    OrderedTargetDependSet const& orderedProjectTargets);

  virtual void WriteTargetsToSolution(
    std::ostream& fout, cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets) const;
  virtual void WriteTargetConfigurations(
    std::ostream& fout, std::vector<std::string> const& configs,
    OrderedTargetDependSet const& projectTargets) const;

  virtual void WriteExternalProject(
    std::ostream& fout, std::string const& name, std::string const& path,
    cmValue typeGuid,
    std::set<BT<std::pair<std::string, bool>>> const& dependencies) const = 0;

  std::string ConvertToSolutionPath(std::string const& path) const;

  std::set<std::string> IsPartOfDefaultBuild(
    std::vector<std::string> const& configs,
    OrderedTargetDependSet const& projectTargets,
    cmGeneratorTarget const* target) const;
  bool IsDependedOn(OrderedTargetDependSet const& projectTargets,
                    cmGeneratorTarget const* target) const;
  std::map<std::string, std::string> GUIDMap;

  virtual void WriteFolders(std::ostream& fout,
                            VSFolders const& vsFolders) const;
  virtual void WriteFoldersContent(std::ostream& fout,
                                   VSFolders const& vsFolders) const;

  virtual void AddSolutionItems(cmLocalGenerator* root,
                                VSFolders& vsFolders) = 0;
  virtual void WriteFolderSolutionItems(
    std::ostream& fout, cmVisualStudioFolder const& folder) const = 0;

  bool MarmasmEnabled;
  bool MasmEnabled;
  bool NasmEnabled;

private:
  std::string IntelProjectVersion;
  std::string DevEnvCommand;
  bool DevEnvCommandInitialized;
  std::string GetVSMakeProgram() override { return this->GetDevEnvCommand(); }
};

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"
