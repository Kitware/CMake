/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportTryCompileFileGenerator.h"

#include <map>
#include <utility>

#include <cm/memory>

#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"

class cmTargetExport;

cmExportTryCompileFileGenerator::cmExportTryCompileFileGenerator(
  cmGlobalGenerator* gg, const std::vector<std::string>& targets,
  cmMakefile* mf, std::set<std::string> const& langs)
  : Languages(langs.begin(), langs.end())
{
  gg->CreateImportedGenerationObjects(mf, targets, this->Exports);
}

bool cmExportTryCompileFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::set<cmGeneratorTarget const*> emitted;
  std::set<cmGeneratorTarget const*> emittedDeps;
  while (!this->Exports.empty()) {
    cmGeneratorTarget const* te = this->Exports.back();
    this->Exports.pop_back();
    if (emitted.insert(te).second) {
      emittedDeps.insert(te);
      this->GenerateImportTargetCode(os, te, te->GetType());

      ImportPropertyMap properties;

      for (std::string const& lang : this->Languages) {
#define FIND_TARGETS(PROPERTY)                                                \
  this->FindTargets("INTERFACE_" #PROPERTY, te, lang, emittedDeps);

        CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(FIND_TARGETS)

#undef FIND_TARGETS
      }

      this->PopulateProperties(te, properties, emittedDeps);

      this->GenerateInterfaceProperties(te, os, properties);
    }
  }
  return true;
}

std::string cmExportTryCompileFileGenerator::FindTargets(
  const std::string& propName, cmGeneratorTarget const* tgt,
  std::string const& language, std::set<cmGeneratorTarget const*>& emitted)
{
  cmValue prop = tgt->GetProperty(propName);
  if (!prop) {
    return std::string();
  }

  cmGeneratorExpression ge;

  std::unique_ptr<cmGeneratorExpressionDAGChecker> parentDagChecker;
  if (propName == "INTERFACE_LINK_OPTIONS") {
    // To please constraint checks of DAGChecker, this property must have
    // LINK_OPTIONS property as parent
    parentDagChecker = cm::make_unique<cmGeneratorExpressionDAGChecker>(
      tgt, "LINK_OPTIONS", nullptr, nullptr);
  }
  cmGeneratorExpressionDAGChecker dagChecker(tgt, propName, nullptr,
                                             parentDagChecker.get());

  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*prop);

  cmTarget dummyHead("try_compile_dummy_exe", cmStateEnums::EXECUTABLE,
                     cmTarget::VisibilityNormal, tgt->Target->GetMakefile(),
                     cmTarget::PerConfig::Yes);

  cmGeneratorTarget gDummyHead(&dummyHead, tgt->GetLocalGenerator());

  std::string result = cge->Evaluate(tgt->GetLocalGenerator(), this->Config,
                                     &gDummyHead, &dagChecker, tgt, language);

  const std::set<cmGeneratorTarget const*>& allTargets =
    cge->GetAllTargetsSeen();
  for (cmGeneratorTarget const* target : allTargets) {
    if (emitted.insert(target).second) {
      this->Exports.push_back(target);
    }
  }
  return result;
}

void cmExportTryCompileFileGenerator::PopulateProperties(
  const cmGeneratorTarget* target, ImportPropertyMap& properties,
  std::set<cmGeneratorTarget const*>& emitted)
{
  // Look through all non-special properties.
  std::vector<std::string> props = target->GetPropertyKeys();
  // Include special properties that might be relevant here.
  props.emplace_back("INTERFACE_LINK_LIBRARIES");
  props.emplace_back("INTERFACE_LINK_LIBRARIES_DIRECT");
  props.emplace_back("INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE");
  for (std::string const& p : props) {
    cmValue v = target->GetProperty(p);
    if (!v) {
      continue;
    }
    properties[p] = *v;

    if (cmHasLiteralPrefix(p, "IMPORTED_LINK_INTERFACE_LIBRARIES") ||
        cmHasLiteralPrefix(p, "IMPORTED_LINK_DEPENDENT_LIBRARIES") ||
        cmHasLiteralPrefix(p, "INTERFACE_LINK_LIBRARIES")) {
      std::string evalResult =
        this->FindTargets(p, target, std::string(), emitted);

      std::vector<std::string> depends = cmExpandedList(evalResult);
      for (std::string const& li : depends) {
        cmGeneratorTarget* tgt =
          target->GetLocalGenerator()->FindGeneratorTargetToUse(li);
        if (tgt && emitted.insert(tgt).second) {
          this->Exports.push_back(tgt);
        }
      }
    }
  }
}

std::string cmExportTryCompileFileGenerator::InstallNameDir(
  cmGeneratorTarget const* target, const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir = target->GetInstallNameDirForBuildTree(config);
  }

  return install_name_dir;
}

std::string cmExportTryCompileFileGenerator::GetFileSetDirectories(
  cmGeneratorTarget* /*gte*/, cmFileSet* fileSet, cmTargetExport* /*te*/)
{
  return cmOutputConverter::EscapeForCMake(
    cmJoin(fileSet->GetDirectoryEntries(), ";"));
}

std::string cmExportTryCompileFileGenerator::GetFileSetFiles(
  cmGeneratorTarget* /*gte*/, cmFileSet* fileSet, cmTargetExport* /*te*/)
{
  return cmOutputConverter::EscapeForCMake(
    cmJoin(fileSet->GetFileEntries(), ";"));
}
