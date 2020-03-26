/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmExportBuildFileGenerator_h
#define cmExportBuildFileGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cmExportFileGenerator.h"
#include "cmStateTypes.h"

class cmExportSet;
class cmGeneratorTarget;
class cmGlobalGenerator;
class cmLocalGenerator;

/** \class cmExportBuildFileGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildFileGenerator generates a file exporting targets from
 * a build tree.  A single file exports information for all
 * configurations built.
 *
 * This is used to implement the EXPORT() command.
 */
class cmExportBuildFileGenerator : public cmExportFileGenerator
{
public:
  cmExportBuildFileGenerator();

  /** Set the list of targets to export.  */
  void SetTargets(std::vector<std::string> const& targets)
  {
    this->Targets = targets;
  }
  void GetTargets(std::vector<std::string>& targets) const;
  void AppendTargets(std::vector<std::string> const& targets)
  {
    cm::append(this->Targets, targets);
  }
  void SetExportSet(cmExportSet*);

  /** Set whether to append generated code to the output file.  */
  void SetAppendMode(bool append) { this->AppendMode = append; }

  void Compute(cmLocalGenerator* lg);

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(
    std::ostream& os, const std::string& config, std::string const& suffix,
    std::vector<std::string>& missingTargets) override;
  cmStateEnums::TargetType GetExportTargetType(
    cmGeneratorTarget const* target) const;
  void HandleMissingTarget(std::string& link_libs,
                           std::vector<std::string>& missingTargets,
                           cmGeneratorTarget* depender,
                           cmGeneratorTarget* dependee) override;

  void ComplainAboutMissingTarget(cmGeneratorTarget* depender,
                                  cmGeneratorTarget* dependee,
                                  std::vector<std::string> const& namespaces);

  /** Fill in properties indicating built file locations.  */
  void SetImportLocationProperty(const std::string& config,
                                 std::string const& suffix,
                                 cmGeneratorTarget* target,
                                 ImportPropertyMap& properties);

  std::string InstallNameDir(cmGeneratorTarget* target,
                             const std::string& config) override;

  std::pair<std::vector<std::string>, std::string> FindBuildExportInfo(
    cmGlobalGenerator* gg, const std::string& name);

  std::vector<std::string> Targets;
  cmExportSet* ExportSet;
  std::vector<cmGeneratorTarget*> Exports;
  cmLocalGenerator* LG;
};

#endif
