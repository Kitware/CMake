/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmExportBuildFileGenerator.h"
#include "cmExportPackageInfoGenerator.h"

namespace Json {
class Value;
}

class cmGeneratorTarget;
class cmPackageInfoArguments;

/** \class cmExportBuildPackageInfoGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildCMakeConfigGenerator generates a file exporting targets from
 * a build tree.  This exports the targets to the Common Package Specification
 * (https://cps-org.github.io/cps/).
 *
 * This is used to implement the export() command.
 */
class cmExportBuildPackageInfoGenerator
  : public cmExportBuildFileGenerator
  , public cmExportPackageInfoGenerator
{
public:
  cmExportBuildPackageInfoGenerator(cmPackageInfoArguments arguments);

protected:
  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream&, std::string const&,
                                   std::string const&) override
  {
  }

  void GenerateInterfacePropertiesConfig(Json::Value& configurations,
                                         cmGeneratorTarget* target,
                                         std::string const& config);

  std::string GetCxxModulesDirectory() const override;
  // TODO: Generate C++ module info in a not-CMake-specific format.
};
