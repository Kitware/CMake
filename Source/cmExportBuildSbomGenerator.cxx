/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportBuildSbomGenerator.h"

#include <functional>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include "cmGeneratorExpression.h"
#include "cmSbomArguments.h"
#include "cmSbomObject.h"
#include "cmSpdx.h"
#include "cmStringAlgorithms.h"

class cmGeneratorTarget;

cmExportBuildSbomGenerator::cmExportBuildSbomGenerator(cmSbomArguments args)
  : cmExportSbomGenerator(args)
{
  this->SetNamespace(cmStrCat(this->GetPackageName(), "::"_s));
}

bool cmExportBuildSbomGenerator::GenerateMainFile(std::ostream& os)
{
  if (!this->CollectExports([&](cmGeneratorTarget const*) {})) {
    return false;
  }

  cmSbomDocument doc;
  doc.Graph.reserve(256);

  cmSpdxDocument* project = insert_back(doc.Graph, this->GenerateSbom());
  std::vector<TargetProperties> targets;

  for (auto const& exp : this->Exports) {
    cmGeneratorTarget const* target = exp.Target;

    ImportPropertyMap properties;
    this->PopulateInterfaceProperties(target, properties);
    this->PopulateInterfaceLinkLibrariesProperty(
      target, cmGeneratorExpression::BuildInterface, properties);
    this->PopulateLinkLibrariesProperty(
      target, cmGeneratorExpression::BuildInterface, properties);

    targets.push_back(TargetProperties{
      insert_back(project->RootElements, this->GenerateImportTarget(target)),
      target, std::move(properties) });
  }

  for (auto const& target : targets) {
    this->GenerateProperties(doc, project, target, targets);
  }

  this->WriteSbom(doc, os);
  return true;
}

void cmExportBuildSbomGenerator::HandleMissingTarget(
  std::string& /* link_libs */, cmGeneratorTarget const* /* depender */,
  cmGeneratorTarget* /* dependee */)
{
}

std::string cmExportBuildSbomGenerator::GetCxxModulesDirectory() const
{
  return {};
}

cm::string_view cmExportBuildSbomGenerator::GetImportPrefixWithSlash() const
{
  return "";
}

std::string cmExportBuildSbomGenerator::GetCxxModuleFile(
  std::string const& /*name*/) const
{
  return {};
}

void cmExportBuildSbomGenerator::GenerateCxxModuleConfigInformation(
  std::string const& /*name*/, std::ostream& /*os*/) const
{
  // TODO
}
