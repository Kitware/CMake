/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportSet.h"

#include <utility>

#include "cmLocalGenerator.h"
#include "cmTargetExport.h"

cmExportSet::cmExportSet(std::string name)
  : Name(std::move(name))
{
}

cmExportSet::~cmExportSet() = default;

void cmExportSet::Compute(cmLocalGenerator* lg)
{
  for (std::unique_ptr<cmTargetExport>& tgtExport : this->TargetExports) {
    tgtExport->Target = lg->FindGeneratorTargetToUse(tgtExport->TargetName);
  }
}

void cmExportSet::AddTargetExport(std::unique_ptr<cmTargetExport> te)
{
  this->TargetExports.emplace_back(std::move(te));
}

void cmExportSet::AddInstallation(cmInstallExportGenerator const* installation)
{
  this->Installations.push_back(installation);
}
