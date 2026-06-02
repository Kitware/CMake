/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#include "cmInstallSbomBuilder.h"

#include <utility>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSbomArguments.h"

cmInstallSbomBuilder::cmInstallSbomBuilder(
  cmSbomArguments args, std::vector<cmExportSet*> exportSets,
  cmLocalGenerator* lg)
  : cmSbomBuilder(std::move(args), std::move(exportSets), lg)
{
}

bool cmInstallSbomBuilder::Generate(std::ostream& os)
{
  if (!this->LocalGenerator) {
    return false;
  }
  return this->GenerateForTargets(os, cmGeneratorExpression::InstallInterface);
}

cmExportFileGenerator::ExportInfo cmInstallSbomBuilder::FindExportInfoFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindInstallExportInfo(target);
}

cmSbomBuilder::SbomInfo cmInstallSbomBuilder::FindSbomInfoFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindInstallSbomInfo(target);
}
