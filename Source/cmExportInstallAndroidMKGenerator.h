/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmExportAndroidMKGenerator.h"
#include "cmExportInstallFileGenerator.h"
#include "cmStateTypes.h"

class cmGeneratorTarget;
class cmInstallExportGenerator;

/** \class cmExportInstallAndroidMKGenerator
 * \brief Generate files exporting targets from an install tree.
 *
 * cmExportInstallAndroidMKGenerator generates files exporting targets from
 * an installation tree.  The files are placed in a temporary location for
 * installation by cmInstallExportGenerator.  The file format is for the ndk
 * build system and is a makefile fragment specifying prebuilt libraries to the
 * ndk build system.
 *
 * This is used to implement the INSTALL(EXPORT_ANDROID_MK) command.
 */
class cmExportInstallAndroidMKGenerator
  : public cmExportAndroidMKGenerator
  , public cmExportInstallFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallAndroidMKGenerator(cmInstallExportGenerator* iegen);

  std::string GetConfigImportFileGlob() const override { return {}; }

protected:
  GenerateType GetGenerateType() const override { return INSTALL; }

  char GetConfigFileNameSeparator() const override { return '-'; }

  // Implement virtual methods from the superclass.
  void ReportDuplicateTarget(std::string const& targetName) const;
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportHeaderCode(std::ostream& os,
                                std::string const& config = "") override;
  void GenerateImportTargetCode(
    std::ostream& os, cmGeneratorTarget const* target,
    cmStateEnums::TargetType /*targetType*/) override;

  void ComplainAboutMissingTarget(cmGeneratorTarget const* depender,
                                  cmGeneratorTarget const* dependee,
                                  std::vector<std::string> const& namespaces);

  void GenerateImportTargetsConfig(std::ostream& os, std::string const& config,
                                   std::string const& suffix) override
  {
    this->cmExportAndroidMKGenerator::GenerateImportTargetsConfig(os, config,
                                                                  suffix);
  }

  std::string GetCxxModulesDirectory() const override { return {}; }
};
