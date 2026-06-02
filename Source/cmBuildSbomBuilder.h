/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <vector>

#include "cmExportFileGenerator.h"
#include "cmSbomBuilder.h"

class cmExportSet;
class cmLocalGenerator;
class cmSbomArguments;

/** Build-tree SBOM (`export(SBOM ...)`).  Covers the targets in the
 *  associated export sets. */
class cmBuildSbomBuilder final : public cmSbomBuilder
{
public:
  cmBuildSbomBuilder(cmSbomArguments args,
                     std::vector<cmExportSet*> exportSets,
                     cmLocalGenerator* lg = nullptr);

  bool Generate(std::ostream& os) override;

protected:
  cmExportFileGenerator::ExportInfo FindExportInfoFor(
    cmGeneratorTarget const* target) const override;
  SbomInfo FindSbomInfoFor(cmGeneratorTarget const* target) const override;
};
