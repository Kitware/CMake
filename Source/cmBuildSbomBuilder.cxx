/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#include "cmBuildSbomBuilder.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSbomArguments.h"

cmBuildSbomBuilder::cmBuildSbomBuilder(cmSbomArguments args,
                                       std::vector<cmExportSet*> exportSets,
                                       cmLocalGenerator* lg)
  : cmSbomBuilder(std::move(args), std::move(exportSets), lg)
{
}

bool cmBuildSbomBuilder::Generate(std::ostream& os, std::string const& config)
{
  if (!this->LocalGenerator) {
    return false;
  }
  return this->GenerateForTargets(os, config,
                                  cmGeneratorExpression::BuildInterface);
}

cmExportFileGenerator::ExportInfo cmBuildSbomBuilder::FindExportInfoFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindBuildExportInfo(target);
}

cmSbomBuilder::SbomInfo cmBuildSbomBuilder::FindSbomInfoFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()->GetGlobalGenerator()->FindBuildSbomInfo(
    target);
}
