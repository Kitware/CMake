/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include <cm/string_view>

#include "cmExportBuildFileGenerator.h"
#include "cmExportSbomGenerator.h"

class cmSbomArguments;

class cmExportBuildSbomGenerator
  : public cmExportBuildFileGenerator
  , public cmExportSbomGenerator
{
public:
  cmExportBuildSbomGenerator(cmSbomArguments args);

protected:
  void HandleMissingTarget(std::string& link_libs,
                           cmGeneratorTarget const* depender,
                           cmGeneratorTarget* dependee) override;
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportTargetsConfig(std::ostream&, std::string const&,
                                   std::string const&) override
  {
  }
  std::string GetCxxModulesDirectory() const override;

  cm::string_view GetImportPrefixWithSlash() const override;

  std::string GetCxxModuleFile(std::string const& /*name*/) const override;

  void GenerateCxxModuleConfigInformation(std::string const& /*name*/,
                                          std::ostream& /*os*/) const override;
};
