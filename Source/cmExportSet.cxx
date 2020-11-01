/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportSet.h"

#include <tuple>
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

cmExportSet& cmExportSetMap::operator[](const std::string& name)
{
  auto it = this->find(name);
  if (it == this->end()) // Export set not found
  {
    auto tup_name = std::make_tuple(name);
    it = this->emplace(std::piecewise_construct, tup_name, tup_name).first;
  }
  return it->second;
}
