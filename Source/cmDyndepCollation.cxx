/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmDyndepCollation.h"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmExportBuildFileGenerator.h"
#include "cmExportSet.h"
#include "cmFileSet.h"
#include "cmGeneratorExpression.h" // IWYU pragma: keep
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallCxxModuleBmiGenerator.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmInstallGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

namespace {

Json::Value CollationInformationCxxModules(
  cmGeneratorTarget const* gt, std::string const& config,
  cmDyndepGeneratorCallbacks const& cb)
{
  cmTarget const* tgt = gt->Target;
  auto all_file_sets = tgt->GetAllFileSetNames();
  Json::Value tdi_cxx_module_info = Json::objectValue;
  for (auto const& file_set_name : all_file_sets) {
    auto const* file_set = tgt->GetFileSet(file_set_name);
    if (!file_set) {
      gt->Makefile->IssueMessage(MessageType::INTERNAL_ERROR,
                                 cmStrCat("Target \"", tgt->GetName(),
                                          "\" is tracked to have file set \"",
                                          file_set_name,
                                          "\", but it was not found."));
      continue;
    }
    auto fs_type = file_set->GetType();
    // We only care about C++ module sources here.
    if (fs_type != "CXX_MODULES"_s) {
      continue;
    }

    auto fileEntries = file_set->CompileFileEntries();
    auto directoryEntries = file_set->CompileDirectoryEntries();

    auto directories = file_set->EvaluateDirectoryEntries(
      directoryEntries, gt->LocalGenerator, config, gt);
    std::map<std::string, std::vector<std::string>> files_per_dirs;
    for (auto const& entry : fileEntries) {
      file_set->EvaluateFileEntry(directories, files_per_dirs, entry,
                                  gt->LocalGenerator, config, gt);
    }

    std::map<std::string, cmSourceFile const*> sf_map;
    {
      std::vector<cmSourceFile const*> objectSources;
      gt->GetObjectSources(objectSources, config);
      for (auto const* sf : objectSources) {
        auto full_path = sf->GetFullPath();
        if (full_path.empty()) {
          gt->Makefile->IssueMessage(
            MessageType::INTERNAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(),
                     "\" has a full path-less source file."));
          continue;
        }
        sf_map[full_path] = sf;
      }
    }

    Json::Value fs_dest = Json::nullValue;
    for (auto const& ig : gt->Makefile->GetInstallGenerators()) {
      if (auto const* fsg =
            dynamic_cast<cmInstallFileSetGenerator const*>(ig.get())) {
        if (fsg->GetTarget() == gt && fsg->GetFileSet() == file_set) {
          fs_dest = fsg->GetDestination(config);
          continue;
        }
      }
    }

    for (auto const& files_per_dir : files_per_dirs) {
      for (auto const& file : files_per_dir.second) {
        auto lookup = sf_map.find(file);
        if (lookup == sf_map.end()) {
          gt->Makefile->IssueMessage(
            MessageType::INTERNAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(), "\" has source file \"",
                     file,
                     R"(" which is not in any of its "FILE_SET BASE_DIRS".)"));
          continue;
        }

        auto const* sf = lookup->second;

        if (!sf) {
          gt->Makefile->IssueMessage(
            MessageType::INTERNAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(), "\" has source file \"",
                     file, "\" which has not been tracked properly."));
          continue;
        }

        auto obj_path = cb.ObjectFilePath(sf, config);
        Json::Value& tdi_module_info = tdi_cxx_module_info[obj_path] =
          Json::objectValue;

        tdi_module_info["source"] = file;
        tdi_module_info["relative-directory"] = files_per_dir.first;
        tdi_module_info["name"] = file_set->GetName();
        tdi_module_info["type"] = file_set->GetType();
        tdi_module_info["visibility"] =
          std::string(cmFileSetVisibilityToName(file_set->GetVisibility()));
        tdi_module_info["destination"] = fs_dest;
      }
    }
  }

  return tdi_cxx_module_info;
}

Json::Value CollationInformationBmiInstallation(cmGeneratorTarget const* gt,
                                                std::string const& config)
{
  cmInstallCxxModuleBmiGenerator const* bmi_gen = nullptr;
  for (auto const& ig : gt->Makefile->GetInstallGenerators()) {
    if (auto const* bmig =
          dynamic_cast<cmInstallCxxModuleBmiGenerator const*>(ig.get())) {
      if (bmig->GetTarget() == gt) {
        bmi_gen = bmig;
        continue;
      }
    }
  }
  if (bmi_gen) {
    Json::Value tdi_bmi_info = Json::objectValue;

    tdi_bmi_info["permissions"] = bmi_gen->GetFilePermissions();
    tdi_bmi_info["destination"] = bmi_gen->GetDestination(config);
    const char* msg_level = "";
    switch (bmi_gen->GetMessageLevel()) {
      case cmInstallGenerator::MessageDefault:
        break;
      case cmInstallGenerator::MessageAlways:
        msg_level = "MESSAGE_ALWAYS";
        break;
      case cmInstallGenerator::MessageLazy:
        msg_level = "MESSAGE_LAZY";
        break;
      case cmInstallGenerator::MessageNever:
        msg_level = "MESSAGE_NEVER";
        break;
    }
    tdi_bmi_info["message-level"] = msg_level;
    tdi_bmi_info["script-location"] = bmi_gen->GetScriptLocation(config);

    return tdi_bmi_info;
  }
  return Json::nullValue;
}

Json::Value CollationInformationExports(cmGeneratorTarget const* gt)
{
  Json::Value tdi_exports = Json::arrayValue;
  std::string export_name = gt->GetExportName();

  auto const& all_install_exports = gt->GetGlobalGenerator()->GetExportSets();
  for (auto const& exp : all_install_exports) {
    // Ignore exports sets which are not for this target.
    auto const& targets = exp.second.GetTargetExports();
    auto tgt_export =
      std::find_if(targets.begin(), targets.end(),
                   [gt](std::unique_ptr<cmTargetExport> const& te) {
                     return te->Target == gt;
                   });
    if (tgt_export == targets.end()) {
      continue;
    }

    auto const* installs = exp.second.GetInstallations();
    for (auto const* install : *installs) {
      Json::Value tdi_export_info = Json::objectValue;

      auto const& ns = install->GetNamespace();
      auto const& dest = install->GetDestination();
      auto const& cxxm_dir = install->GetCxxModuleDirectory();
      auto const& export_prefix = install->GetTempDir();

      tdi_export_info["namespace"] = ns;
      tdi_export_info["export-name"] = export_name;
      tdi_export_info["destination"] = dest;
      tdi_export_info["cxx-module-info-dir"] = cxxm_dir;
      tdi_export_info["export-prefix"] = export_prefix;
      tdi_export_info["install"] = true;

      tdi_exports.append(tdi_export_info);
    }
  }

  auto const& all_build_exports = gt->Makefile->GetExportBuildFileGenerators();
  for (auto const& exp : all_build_exports) {
    std::vector<std::string> targets;
    exp->GetTargets(targets);

    // Ignore exports sets which are not for this target.
    auto const& name = gt->GetName();
    bool has_current_target =
      std::any_of(targets.begin(), targets.end(),
                  [name](std::string const& tname) { return tname == name; });
    if (!has_current_target) {
      continue;
    }

    Json::Value tdi_export_info = Json::objectValue;

    auto const& ns = exp->GetNamespace();
    auto const& main_fn = exp->GetMainExportFileName();
    auto const& cxxm_dir = exp->GetCxxModuleDirectory();
    auto dest = cmsys::SystemTools::GetParentDirectory(main_fn);
    auto const& export_prefix =
      cmSystemTools::GetFilenamePath(exp->GetMainExportFileName());

    tdi_export_info["namespace"] = ns;
    tdi_export_info["export-name"] = export_name;
    tdi_export_info["destination"] = dest;
    tdi_export_info["cxx-module-info-dir"] = cxxm_dir;
    tdi_export_info["export-prefix"] = export_prefix;
    tdi_export_info["install"] = false;

    tdi_exports.append(tdi_export_info);
  }

  return tdi_exports;
}
}

void cmDyndepCollation::AddCollationInformation(
  Json::Value& tdi, cmGeneratorTarget const* gt, std::string const& config,
  cmDyndepGeneratorCallbacks const& cb)
{
  tdi["cxx-modules"] = CollationInformationCxxModules(gt, config, cb);
  tdi["bmi-installation"] = CollationInformationBmiInstallation(gt, config);
  tdi["exports"] = CollationInformationExports(gt);
  tdi["config"] = config;
}
