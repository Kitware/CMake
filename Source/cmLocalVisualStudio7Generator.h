/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cmLocalVisualStudioGenerator.h"
#include "cmVisualStudioGeneratorOptions.h"

class cmCustomCommand;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmLocalVisualStudio7GeneratorFCInfo;
class cmLocalVisualStudio7GeneratorInternals;
class cmMakefile;
class cmSourceFile;
class cmSourceGroup;

class cmVS7GeneratorOptions : public cmVisualStudioGeneratorOptions
{
public:
  cmVS7GeneratorOptions(cmLocalVisualStudioGenerator* lg, Tool tool,
                        cmVS7FlagTable const* table = nullptr,
                        cmVS7FlagTable const* extraTable = nullptr)
    : cmVisualStudioGeneratorOptions(lg, tool, table, extraTable)
  {
  }
  void OutputFlag(std::ostream& fout, int indent, std::string const& tag,
                  std::string const& content) override;
};

/** \class cmLocalVisualStudio7Generator
 * \brief Write Visual Studio .NET project files.
 *
 * cmLocalVisualStudio7Generator produces a Visual Studio .NET project
 * file for each target in its directory.
 */
class cmLocalVisualStudio7Generator : public cmLocalVisualStudioGenerator
{
public:
  //! Set cache only and recurse to false by default.
  cmLocalVisualStudio7Generator(cmGlobalGenerator* gg, cmMakefile* mf);

  ~cmLocalVisualStudio7Generator() override;

  cmLocalVisualStudio7Generator(cmLocalVisualStudio7Generator const&) = delete;
  cmLocalVisualStudio7Generator const& operator=(
    cmLocalVisualStudio7Generator const&) = delete;

  void AddHelperCommands() override;

  /**
   * Generate the makefile for this directory.
   */
  void Generate() override;

  enum BuildType
  {
    STATIC_LIBRARY,
    DLL,
    EXECUTABLE,
    WIN32_EXECUTABLE,
    UTILITY
  };

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType, std::string const& name);

  std::string GetTargetDirectory(
    cmGeneratorTarget const* target) const override;
  cmSourceFile* CreateVCProjBuildRule();
  void WriteStampFiles();
  std::string ComputeLongestObjectDirectory(
    cmGeneratorTarget const*) const override;

  virtual void ReadAndStoreExternalGUID(std::string const& name,
                                        char const* path);

  std::set<cmSourceFile const*>& GetSourcesVisited(
    cmGeneratorTarget const* target)
  {
    return this->SourcesVisited[target];
  };

  bool IsVFProj() const override { return this->FortranProject; }

protected:
  virtual void GenerateTarget(cmGeneratorTarget* target);

private:
  using Options = cmVS7GeneratorOptions;
  using FCInfo = cmLocalVisualStudio7GeneratorFCInfo;
  void FixGlobalTargets();
  void WriteVCProjHeader(std::ostream& fout, std::string const& libName,
                         cmGeneratorTarget* tgt,
                         std::vector<cmSourceGroup>& sgs);
  void WriteVCProjFooter(std::ostream& fout, cmGeneratorTarget* target);
  void WriteVCProjFile(std::ostream& fout, std::string const& libName,
                       cmGeneratorTarget* tgt);
  void WriteConfigurations(std::ostream& fout,
                           std::vector<std::string> const& configs,
                           std::string const& libName, cmGeneratorTarget* tgt);
  void WriteConfiguration(std::ostream& fout, std::string const& configName,
                          std::string const& libName, cmGeneratorTarget* tgt);
  std::string EscapeForXML(std::string const& s);
  std::string ConvertToXMLOutputPath(std::string const& path);
  std::string ConvertToXMLOutputPathSingle(std::string const& path);
  void OutputTargetRules(std::ostream& fout, std::string const& configName,
                         cmGeneratorTarget* target,
                         std::string const& libName);
  void OutputBuildTool(std::ostream& fout, std::string const& linkLanguage,
                       std::string const& configName, cmGeneratorTarget* t,
                       Options const& targetOptions);
  void OutputDeploymentDebuggerTool(std::ostream& fout,
                                    std::string const& config,
                                    cmGeneratorTarget* target);
  void OutputLibraryDirectories(std::ostream& fout,
                                std::vector<std::string> const& stdlink,
                                std::vector<std::string> const& dirs);
  void WriteProjectSCC(std::ostream& fout, cmGeneratorTarget* target);
  void WriteProjectStart(std::ostream& fout, std::string const& libName,
                         cmGeneratorTarget* tgt,
                         std::vector<cmSourceGroup>& sgs);
  void WriteProjectStartFortran(std::ostream& fout, std::string const& libName,
                                cmGeneratorTarget* tgt);
  void WriteVCProjBeginGroup(std::ostream& fout, char const* group,
                             char const* filter);
  void WriteVCProjEndGroup(std::ostream& fout);

  void WriteCustomRule(std::ostream& fout,
                       std::vector<std::string> const& configs,
                       char const* source, cmCustomCommand const& command,
                       FCInfo& fcinfo);
  void WriteTargetVersionAttribute(std::ostream& fout, cmGeneratorTarget* gt);

  class AllConfigSources;
  bool WriteGroup(cmSourceGroup const* sg, cmGeneratorTarget* target,
                  std::ostream& fout, std::string const& libName,
                  std::vector<std::string> const& configs,
                  AllConfigSources const& sources);

  friend class cmLocalVisualStudio7GeneratorFCInfo;
  friend class cmLocalVisualStudio7GeneratorInternals;

  class EventWriter;

  friend class EventWriter;

  bool FortranProject = false;
  bool WindowsCEProject = false;
  std::unique_ptr<cmLocalVisualStudio7GeneratorInternals> Internal;

  std::map<cmGeneratorTarget const*, std::set<cmSourceFile const*>>
    SourcesVisited;
};
