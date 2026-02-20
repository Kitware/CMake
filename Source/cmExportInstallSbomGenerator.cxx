/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportInstallSbomGenerator.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include "cmExportSet.h"
#include "cmFileSetMetadata.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorFileSet.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmSbomArguments.h"
#include "cmSbomObject.h"
#include "cmSpdx.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

cmExportInstallSbomGenerator::cmExportInstallSbomGenerator(
  cmInstallExportGenerator* iegen, cmSbomArguments args)
  : cmExportSbomGenerator(std::move(args))
  , cmExportInstallFileGenerator(iegen)
{
  this->SetNamespace(cmStrCat(this->GetPackageName(), "::"_s));
}

std::string cmExportInstallSbomGenerator::GetConfigImportFileGlob() const
{
  std::string glob = cmStrCat(this->FileBase, "@*", this->FileExt);
  return glob;
}

std::string const& cmExportInstallSbomGenerator::GetExportName() const
{
  return this->GetPackageName();
}

cm::string_view cmExportInstallSbomGenerator::GetImportPrefixWithSlash() const
{
  return "@prefix@/"_s;
}

bool cmExportInstallSbomGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport const*> allTargets;
  {
    auto visitor = [&](cmTargetExport const* te) { allTargets.push_back(te); };

    if (!this->CollectExports(visitor)) {
      return false;
    }
  }
  cmSbomDocument doc;
  doc.Graph.reserve(256);

  cmSpdxCreationInfo const* ci =
    insert_back(doc.Graph, this->GenerateCreationInfo());
  cmSpdxDocument* project = insert_back(doc.Graph, this->GenerateSbom(ci));
  std::vector<TargetProperties> targets;
  targets.reserve(allTargets.size());

  for (cmTargetExport const* te : allTargets) {
    cmGeneratorTarget const* gt = te->Target;
    ImportPropertyMap properties;
    if (!this->PopulateInterfaceProperties(te, properties)) {
      return false;
    }
    this->PopulateLinkLibrariesProperty(
      gt, cmGeneratorExpression::InstallInterface, properties);
    this->PopulateInterfaceLinkLibrariesProperty(
      gt, cmGeneratorExpression::InstallInterface, properties);

    targets.push_back(TargetProperties{
      insert_back(project->RootElements,
                  this->GenerateImportTarget(ci, te->Target)),
      te->Target, std::move(properties) });
  }

  for (auto const& target : targets) {
    this->GenerateProperties(doc, project, ci, target, targets);
  }

  this->WriteSbom(doc, os);
  return true;
}

void cmExportInstallSbomGenerator::GenerateImportTargetsConfig(
  std::ostream& os, std::string const& config, std::string const& suffix)
{
  cmSbomDocument doc;
  doc.Graph.reserve(256);

  cmSpdxCreationInfo const* ci =
    insert_back(doc.Graph, this->GenerateCreationInfo());
  cmSpdxDocument* project = insert_back(doc.Graph, this->GenerateSbom(ci));

  std::vector<TargetProperties> targets;
  std::string cfg = (config.empty() ? "noconfig" : config);

  for (auto const& te : this->GetExportSet()->GetTargetExports()) {
    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    if (this->GetExportTargetType(te.get()) !=
        cmStateEnums::INTERFACE_LIBRARY) {
      this->PopulateImportProperties(config, suffix, te.get(), properties,
                                     importedLocations);
    }
    this->PopulateInterfaceProperties(te.get(), properties);
    this->PopulateInterfaceLinkLibrariesProperty(
      te->Target, cmGeneratorExpression::InstallInterface, properties);
    this->PopulateLinkLibrariesProperty(
      te->Target, cmGeneratorExpression::InstallInterface, properties);

    targets.push_back(TargetProperties{
      insert_back(project->RootElements,
                  this->GenerateImportTarget(ci, te->Target)),
      te->Target, std::move(properties) });
  }

  for (auto const& target : targets) {
    this->GenerateProperties(doc, project, ci, target, targets);
  }

  this->WriteSbom(doc, os);
}

std::string cmExportInstallSbomGenerator::GenerateImportPrefix() const
{
  std::string expDest = this->IEGen->GetDestination();
  if (cmSystemTools::FileIsFullPath(expDest)) {
    std::string const& installPrefix =
      this->IEGen->GetLocalGenerator()->GetMakefile()->GetSafeDefinition(
        "CMAKE_INSTALL_PREFIX");
    if (cmHasPrefix(expDest, installPrefix)) {
      auto n = installPrefix.length();
      while (n < expDest.length() && expDest[n] == '/') {
        ++n;
      }
      expDest = expDest.substr(n);
    } else {
      this->ReportError(
        cmStrCat("install(SBOM \"", this->GetExportName(),
                 "\" ...) specifies DESTINATION \"", expDest,
                 "\" which is not a subdirectory of the install prefix."));
      return {};
    }
  }

  if (expDest.empty()) {
    return this->GetInstallPrefix();
  }
  return cmStrCat(this->GetImportPrefixWithSlash(), expDest);
}

void cmExportInstallSbomGenerator::HandleMissingTarget(
  std::string& /* link_libs */, cmGeneratorTarget const* /* depender */,
  cmGeneratorTarget* /* dependee */)
{
}

bool cmExportInstallSbomGenerator::CheckInterfaceDirs(
  std::string const& /* prepro */, cmGeneratorTarget const* /* target */,
  std::string const& /* prop */) const
{
  return true;
}

std::string cmExportInstallSbomGenerator::InstallNameDir(
  cmGeneratorTarget const* target, std::string const& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir =
      target->GetInstallNameDirForInstallTree(config, "@prefix@");
  }

  return install_name_dir;
}

std::string cmExportInstallSbomGenerator::GetCxxModulesDirectory() const
{
  return {};
}

void cmExportInstallSbomGenerator::GenerateCxxModuleConfigInformation(
  std::string const&, std::ostream&) const
{
}

std::string cmExportInstallSbomGenerator::GetCxxModuleFile(
  std::string const& /* name */) const
{
  return {};
}

cm::optional<std::string> cmExportInstallSbomGenerator::GetFileSetDirectory(
  cmGeneratorTarget* gte, cmTargetExport const* te,
  cmGeneratorFileSet const* fileSet, cm::optional<std::string> const& config)
{
  cmGeneratorExpression ge(*gte->Makefile->GetCMakeInstance());
  auto cge =
    ge.Parse(te->FileSetGenerators.at(fileSet->GetName())->GetDestination());

  std::string const unescapedDest =
    cge->Evaluate(gte->LocalGenerator, config.value_or(""), gte);
  bool const isConfigDependent = cge->GetHadContextSensitiveCondition();

  if (config && !isConfigDependent) {
    return {};
  }

  std::string const& type = fileSet->GetType();
  if (config && (type == cm::FileSetMetadata::CXX_MODULES)) {
    cmMakefile* mf = gte->LocalGenerator->GetMakefile();
    std::ostringstream e;
    e << "The \"" << gte->GetName() << "\" target's interface file set \""
      << fileSet->GetName() << "\" of type \"" << type
      << "\" contains context-sensitive base file entries which is not "
         "supported.";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return {};
  }

  cm::optional<std::string> dest = cmOutputConverter::EscapeForCMake(
    unescapedDest, cmOutputConverter::WrapQuotes::NoWrap);

  if (!cmSystemTools::FileIsFullPath(unescapedDest)) {
    dest = cmStrCat("@prefix@/"_s, *dest);
  }

  return dest;
}
