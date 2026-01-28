/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include <cm/optional>

#include "cmExportInstallFileGenerator.h"
#include "cmExportPackageInfoGenerator.h"

class cmFileSet;
class cmGeneratorTarget;
class cmInstallExportGenerator;
class cmPackageInfoArguments;
class cmTargetExport;

namespace Json {
class Value;
}

/** \class cmExportInstallPackageInfoGenerator
 * \brief Generate files exporting targets from an install tree.
 *
 * cmExportInstallPackageInfoGenerator generates files exporting targets from
 * an installation tree.  The files are placed in a temporary location for
 * installation by cmInstallExportGenerator.  The file format is the Common
 * Package Specification (https://cps-org.github.io/cps/).
 *
 * One main file is generated that describes the imported targets.  Additional,
 * per-configuration files describe target locations and settings for each
 * configuration.
 *
 * This is used to implement the INSTALL(PACKAGE_INFO) command.
 */
class cmExportInstallPackageInfoGenerator
  : public cmExportPackageInfoGenerator
  , public cmExportInstallFileGenerator
{
public:
  /** Construct with the export installer that will install the
      files.  */
  cmExportInstallPackageInfoGenerator(cmInstallExportGenerator* iegen,
                                      cmPackageInfoArguments arguments);

  /** Compute the globbing expression used to load per-config import
      files from the main file.  */
  std::string GetConfigImportFileGlob() const override;

protected:
  bool RequiresConfigFiles = false;

  std::string const& GetExportName() const override;

  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream& os, std::string const& config,
                                   std::string const& suffix) override;

  char GetConfigFileNameSeparator() const override { return '@'; }

  /** Generate the cps_path, which determines the import prefix.  */
  std::string GenerateImportPrefix() const;

  std::string InstallNameDir(cmGeneratorTarget const* target,
                             std::string const& config) override;

  std::string GetCxxModulesDirectory() const override;

  cm::optional<std::string> GetFileSetDirectory(
    cmGeneratorTarget* gte, cmTargetExport const* te, cmFileSet* fileSet,
    cm::optional<std::string> const& config = {});

  bool GenerateFileSetProperties(Json::Value& component,
                                 cmGeneratorTarget* gte,
                                 cmTargetExport const* te,
                                 std::string const& packagePath,
                                 cm::optional<std::string> config = {});
};
