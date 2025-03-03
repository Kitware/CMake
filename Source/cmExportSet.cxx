/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportSet.h" // IWYU pragma: associated

#include <algorithm>
#include <tuple>
#include <utility>

#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetExport.h" // IWYU pragma: associated

cmExportSet::cmExportSet(std::string name)
  : Name(std::move(name))
{
}

cmExportSet::~cmExportSet() = default;

cmExportSet::PackageDependency& cmExportSet::GetPackageDependencyForSetup(
  std::string const& name)
{
  auto& dep = this->PackageDependencies[name];
  if (!dep.SpecifiedIndex) {
    dep.SpecifiedIndex = this->NextPackageDependencyIndex;
    this->NextPackageDependencyIndex++;
  }
  return dep;
}

bool cmExportSet::Compute(cmLocalGenerator* lg)
{
  for (std::unique_ptr<cmTargetExport>& tgtExport : this->TargetExports) {
    tgtExport->Target = lg->FindGeneratorTargetToUse(tgtExport->TargetName);

    auto const interfaceFileSets =
      tgtExport->Target->Target->GetAllInterfaceFileSets();
    auto const fileSetInTargetExport =
      [&tgtExport, lg](std::string const& fileSetName) -> bool {
      auto* fileSet = tgtExport->Target->Target->GetFileSet(fileSetName);

      if (!tgtExport->FileSetGenerators.count(fileSet)) {
        lg->IssueMessage(MessageType::FATAL_ERROR,
                         cmStrCat("File set \"", fileSetName,
                                  "\" is listed in interface file sets of ",
                                  tgtExport->Target->GetName(),
                                  " but has not been exported"));
        return false;
      }
      return true;
    };

    if (!std::all_of(interfaceFileSets.begin(), interfaceFileSets.end(),
                     fileSetInTargetExport)) {
      return false;
    }
  }

  return true;
}

void cmExportSet::AddTargetExport(std::unique_ptr<cmTargetExport> te)
{
  this->TargetExports.emplace_back(std::move(te));
}

void cmExportSet::AddInstallation(cmInstallExportGenerator const* installation)
{
  this->Installations.push_back(installation);
}

void cmExportSet::SetXcFrameworkLocation(std::string const& name,
                                         std::string const& location)
{
  for (auto& te : this->TargetExports) {
    if (name == te->TargetName) {
      te->XcFrameworkLocation = location;
    }
  }
}

cmExportSet& cmExportSetMap::operator[](std::string const& name)
{
  auto it = this->find(name);
  if (it == this->end()) // Export set not found
  {
    auto tup_name = std::make_tuple(name);
    it = this->emplace(std::piecewise_construct, tup_name, tup_name).first;
  }
  return it->second;
}
