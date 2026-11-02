/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details. */
#include "cmInstallSbomBuilder.h"

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"

cmInstallSbomBuilder::cmInstallSbomBuilder(
  cmSbomArguments const& args, std::vector<cmExportSet*> const& exportSets)
  : cmSbomBuilder(args, exportSets)
{
}

bool cmInstallSbomBuilder::Generate(std::ostream& os,
                                    std::string const& config)
{
  return this->GenerateForTargets(os, config,
                                  cmGeneratorExpression::InstallInterface);
}

cmExportFileGenerator::ExportInfo cmInstallSbomBuilder::FindExportInfoFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindInstallExportInfo(target);
}

std::vector<std::string> cmInstallSbomBuilder::FindSbomPackagesFor(
  cmGeneratorTarget const* target) const
{
  return target->GetLocalGenerator()
    ->GetGlobalGenerator()
    ->FindInstallSbomPackages(target);
}
