/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#include "cmBuildSbomBuilder.h"

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"

cmBuildSbomBuilder::cmBuildSbomBuilder(
  cmSbomArguments const& args, std::vector<cmExportSet*> const& exportSets)
  : cmSbomBuilder(args, exportSets)
{
}

bool cmBuildSbomBuilder::Generate(std::ostream& os, std::string const& config)
{
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

std::vector<std::string> cmBuildSbomBuilder::FindSbomPackagesFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindBuildSbomPackages(target);
}
