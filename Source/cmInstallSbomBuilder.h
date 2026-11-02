/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmSbomBuilder.h"

class cmExportSet;
class cmSbomArguments;

/** Install-tree SBOM (`install(SBOM ...)`).  Covers the targets in the
 *  associated export sets. */
class cmInstallSbomBuilder final : public cmSbomBuilder
{
public:
  cmInstallSbomBuilder(cmSbomArguments const& args,
                       std::vector<cmExportSet*> const& exportSets);

  bool Generate(std::ostream& os, std::string const& config) override;

protected:
  cmExportFileGenerator::ExportInfo FindExportInfoFor(
    cmGeneratorTarget const* target) const override;
  std::vector<std::string> FindSbomPackagesFor(
    cmGeneratorTarget const* target) const override;
};
