/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmDyndepCollation.h"

#include <algorithm>
#include <map>
#include <ostream>
#include <set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmExportBuildFileGenerator.h"
#include "cmExportSet.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h" // IWYU pragma: keep
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallCxxModuleBmiGenerator.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmInstallGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmScanDepFormat.h"
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

    enum class CompileType
    {
      ObjectAndBmi,
      BmiOnly,
    };
    std::map<std::string, std::pair<cmSourceFile const*, CompileType>> sf_map;
    {
      auto fill_sf_map = [gt, tgt, &sf_map](cmSourceFile const* sf,
                                            CompileType type) {
        auto full_path = sf->GetFullPath();
        if (full_path.empty()) {
          gt->Makefile->IssueMessage(
            MessageType::INTERNAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(),
                     "\" has a full path-less source file."));
          return;
        }
        sf_map[full_path] = std::make_pair(sf, type);
      };

      std::vector<cmSourceFile const*> objectSources;
      gt->GetObjectSources(objectSources, config);
      for (auto const* sf : objectSources) {
        fill_sf_map(sf, CompileType::ObjectAndBmi);
      }

      std::vector<cmSourceFile const*> cxxModuleSources;
      gt->GetCxxModuleSources(cxxModuleSources, config);
      for (auto const* sf : cxxModuleSources) {
        fill_sf_map(sf, CompileType::BmiOnly);
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
        auto const full_file = cmSystemTools::CollapseFullPath(file);
        auto lookup = sf_map.find(full_file);
        if (lookup == sf_map.end()) {
          gt->Makefile->IssueMessage(
            MessageType::FATAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(), "\" has source file\n  ",
                     file,
                     "\nin a \"FILE_SET TYPE CXX_MODULES\" but it is not "
                     "scheduled for compilation."));
          continue;
        }

        auto const* sf = lookup->second.first;
        CompileType const ct = lookup->second.second;

        if (!sf) {
          gt->Makefile->IssueMessage(
            MessageType::INTERNAL_ERROR,
            cmStrCat("Target \"", tgt->GetName(), "\" has source file \"",
                     file, "\" which has not been tracked properly."));
          continue;
        }

        auto obj_path = ct == CompileType::ObjectAndBmi
          ? cb.ObjectFilePath(sf, config)
          : cb.BmiFilePath(sf, config);
        Json::Value& tdi_module_info = tdi_cxx_module_info[obj_path] =
          Json::objectValue;

        tdi_module_info["source"] = full_file;
        tdi_module_info["bmi-only"] = ct == CompileType::BmiOnly;
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
    std::vector<cmExportBuildFileGenerator::TargetExport> targets;
    exp->GetTargets(targets);

    // Ignore exports sets which are not for this target.
    auto const& name = gt->GetName();
    bool has_current_target =
      std::any_of(targets.begin(), targets.end(),
                  [name](cmExportBuildFileGenerator::TargetExport const& te) {
                    return te.Name == name;
                  });
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

struct CxxModuleFileSet
{
  std::string Name;
  bool BmiOnly = false;
  std::string RelativeDirectory;
  std::string SourcePath;
  std::string Type;
  cmFileSetVisibility Visibility = cmFileSetVisibility::Private;
  cm::optional<std::string> Destination;
};

struct CxxModuleBmiInstall
{
  std::string Component;
  std::string Destination;
  bool ExcludeFromAll;
  bool Optional;
  std::string Permissions;
  std::string MessageLevel;
  std::string ScriptLocation;
};

struct CxxModuleExport
{
  std::string Name;
  std::string Destination;
  std::string Prefix;
  std::string CxxModuleInfoDir;
  std::string Namespace;
  bool Install;
};

struct cmCxxModuleExportInfo
{
  std::map<std::string, CxxModuleFileSet> ObjectToFileSet;
  cm::optional<CxxModuleBmiInstall> BmiInstallation;
  std::vector<CxxModuleExport> Exports;
  std::string Config;
};

void cmCxxModuleExportInfoDeleter::operator()(cmCxxModuleExportInfo* ei) const
{
  delete ei;
}

std::unique_ptr<cmCxxModuleExportInfo, cmCxxModuleExportInfoDeleter>
cmDyndepCollation::ParseExportInfo(Json::Value const& tdi)
{
  auto export_info =
    std::unique_ptr<cmCxxModuleExportInfo, cmCxxModuleExportInfoDeleter>(
      new cmCxxModuleExportInfo);

  export_info->Config = tdi["config"].asString();
  if (export_info->Config.empty()) {
    export_info->Config = "noconfig";
  }
  Json::Value const& tdi_exports = tdi["exports"];
  if (tdi_exports.isArray()) {
    for (auto const& tdi_export : tdi_exports) {
      CxxModuleExport exp;
      exp.Install = tdi_export["install"].asBool();
      exp.Name = tdi_export["export-name"].asString();
      exp.Destination = tdi_export["destination"].asString();
      exp.Prefix = tdi_export["export-prefix"].asString();
      exp.CxxModuleInfoDir = tdi_export["cxx-module-info-dir"].asString();
      exp.Namespace = tdi_export["namespace"].asString();

      export_info->Exports.push_back(exp);
    }
  }
  auto const& bmi_installation = tdi["bmi-installation"];
  if (bmi_installation.isObject()) {
    CxxModuleBmiInstall bmi_install;

    bmi_install.Component = bmi_installation["component"].asString();
    bmi_install.Destination = bmi_installation["destination"].asString();
    bmi_install.ExcludeFromAll = bmi_installation["exclude-from-all"].asBool();
    bmi_install.Optional = bmi_installation["optional"].asBool();
    bmi_install.Permissions = bmi_installation["permissions"].asString();
    bmi_install.MessageLevel = bmi_installation["message-level"].asString();
    bmi_install.ScriptLocation =
      bmi_installation["script-location"].asString();

    export_info->BmiInstallation = bmi_install;
  }
  Json::Value const& tdi_cxx_modules = tdi["cxx-modules"];
  if (tdi_cxx_modules.isObject()) {
    for (auto i = tdi_cxx_modules.begin(); i != tdi_cxx_modules.end(); ++i) {
      CxxModuleFileSet& fsi = export_info->ObjectToFileSet[i.key().asString()];
      auto const& tdi_cxx_module_info = *i;
      fsi.Name = tdi_cxx_module_info["name"].asString();
      fsi.BmiOnly = tdi_cxx_module_info["bmi-only"].asBool();
      fsi.RelativeDirectory =
        tdi_cxx_module_info["relative-directory"].asString();
      if (!fsi.RelativeDirectory.empty() &&
          fsi.RelativeDirectory.back() != '/') {
        fsi.RelativeDirectory = cmStrCat(fsi.RelativeDirectory, '/');
      }
      fsi.SourcePath = tdi_cxx_module_info["source"].asString();
      fsi.Type = tdi_cxx_module_info["type"].asString();
      fsi.Visibility = cmFileSetVisibilityFromName(
        tdi_cxx_module_info["visibility"].asString(), nullptr);
      auto const& tdi_fs_dest = tdi_cxx_module_info["destination"];
      if (tdi_fs_dest.isString()) {
        fsi.Destination = tdi_fs_dest.asString();
      }
    }
  }

  return export_info;
}

bool cmDyndepCollation::WriteDyndepMetadata(
  std::string const& lang, std::vector<cmScanDepInfo> const& objects,
  cmCxxModuleExportInfo const& export_info,
  cmDyndepMetadataCallbacks const& cb)
{
  // Only C++ supports any of the file-set or BMI installation considered
  // below.
  if (lang != "CXX"_s) {
    return true;
  }

  bool result = true;

  // Prepare the export information blocks.
  std::string const config_upper =
    cmSystemTools::UpperCase(export_info.Config);
  std::vector<
    std::pair<std::unique_ptr<cmGeneratedFileStream>, CxxModuleExport const*>>
    exports;
  for (auto const& exp : export_info.Exports) {
    std::unique_ptr<cmGeneratedFileStream> properties;

    std::string const export_dir =
      cmStrCat(exp.Prefix, '/', exp.CxxModuleInfoDir, '/');
    std::string const property_file_path = cmStrCat(
      export_dir, "target-", exp.Name, '-', export_info.Config, ".cmake");
    properties = cm::make_unique<cmGeneratedFileStream>(property_file_path);

    // Set up the preamble.
    *properties << "set_property(TARGET \"" << exp.Namespace << exp.Name
                << "\"\n"
                << "  PROPERTY IMPORTED_CXX_MODULES_" << config_upper << '\n';

    exports.emplace_back(std::move(properties), &exp);
  }

  std::unique_ptr<cmGeneratedFileStream> bmi_install_script;
  if (export_info.BmiInstallation) {
    bmi_install_script = cm::make_unique<cmGeneratedFileStream>(
      export_info.BmiInstallation->ScriptLocation);
  }

  auto cmEscape = [](cm::string_view str) {
    return cmOutputConverter::EscapeForCMake(
      str, cmOutputConverter::WrapQuotes::NoWrap);
  };
  auto install_destination =
    [&cmEscape](std::string const& dest) -> std::pair<bool, std::string> {
    if (cmSystemTools::FileIsFullPath(dest)) {
      return std::make_pair(true, cmEscape(dest));
    }
    return std::make_pair(false,
                          cmStrCat("${_IMPORT_PREFIX}/", cmEscape(dest)));
  };

  // public/private requirement tracking.
  std::set<std::string> private_modules;
  std::map<std::string, std::set<std::string>> public_source_requires;

  for (cmScanDepInfo const& object : objects) {
    // Convert to forward slashes.
    auto output_path = object.PrimaryOutput;
#ifdef _WIN32
    cmSystemTools::ConvertToUnixSlashes(output_path);
#endif
    // Find the fileset for this object.
    auto fileset_info_itr = export_info.ObjectToFileSet.find(output_path);
    bool const has_provides = !object.Provides.empty();
    if (fileset_info_itr == export_info.ObjectToFileSet.end()) {
      // If it provides anything, it should have type `CXX_MODULES`
      // and be present.
      if (has_provides) {
        // Take the first module provided to provide context.
        auto const& provides = object.Provides[0];
        cmSystemTools::Error(
          cmStrCat("Output ", object.PrimaryOutput, " provides the `",
                   provides.LogicalName,
                   "` module but it is not found in a `FILE_SET` of type "
                   "`CXX_MODULES`"));
        result = false;
      }

      // This object file does not provide anything, so nothing more needs to
      // be done.
      continue;
    }

    auto const& file_set = fileset_info_itr->second;

    // Verify the fileset type for the object.
    if (file_set.Type == "CXX_MODULES"_s) {
      if (!has_provides) {
        cmSystemTools::Error(
          cmStrCat("Output ", object.PrimaryOutput,
                   " is of type `CXX_MODULES` but does not provide a module "
                   "interface unit or partition"));
        result = false;
        continue;
      }
    } else if (file_set.Type == "CXX_MODULE_HEADERS"_s) {
      // TODO.
    } else {
      if (has_provides) {
        auto const& provides = object.Provides[0];
        cmSystemTools::Error(cmStrCat(
          "Source ", file_set.SourcePath, " provides the `",
          provides.LogicalName, "` C++ module but is of type `", file_set.Type,
          "` module but must be of type `CXX_MODULES`"));
        result = false;
      }

      // Not a C++ module; ignore.
      continue;
    }

    if (!cmFileSetVisibilityIsForInterface(file_set.Visibility)) {
      // Nothing needs to be conveyed about non-`PUBLIC` modules.
      for (auto const& p : object.Provides) {
        private_modules.insert(p.LogicalName);
      }
      continue;
    }

    // The module is public. Record what it directly requires.
    {
      auto& reqs = public_source_requires[file_set.SourcePath];
      for (auto const& r : object.Requires) {
        reqs.insert(r.LogicalName);
      }
    }

    // Write out properties and install rules for any exports.
    for (auto const& p : object.Provides) {
      bool bmi_dest_is_abs = false;
      std::string bmi_destination;
      if (export_info.BmiInstallation) {
        auto dest =
          install_destination(export_info.BmiInstallation->Destination);
        bmi_dest_is_abs = dest.first;
        bmi_destination = cmStrCat(dest.second, '/');
      }

      std::string install_bmi_path;
      std::string build_bmi_path;
      auto m = cb.ModuleFile(p.LogicalName);
      if (m) {
        install_bmi_path = cmStrCat(
          bmi_destination, cmEscape(cmSystemTools::GetFilenameName(*m)));
        build_bmi_path = cmEscape(*m);
      }

      for (auto const& exp : exports) {
        std::string iface_source;
        if (exp.second->Install && file_set.Destination) {
          auto dest = install_destination(*file_set.Destination);
          iface_source = cmStrCat(
            dest.second, '/', cmEscape(file_set.RelativeDirectory),
            cmEscape(cmSystemTools::GetFilenameName(file_set.SourcePath)));
        } else {
          iface_source = cmEscape(file_set.SourcePath);
        }

        std::string bmi_path;
        if (exp.second->Install && export_info.BmiInstallation) {
          bmi_path = install_bmi_path;
        } else if (!exp.second->Install) {
          bmi_path = build_bmi_path;
        }

        if (iface_source.empty()) {
          // No destination for the C++ module source; ignore this property
          // value.
          continue;
        }

        *exp.first << "    \"" << cmEscape(p.LogicalName) << '='
                   << iface_source;
        if (!bmi_path.empty()) {
          *exp.first << ',' << bmi_path;
        }
        *exp.first << "\"\n";
      }

      if (bmi_install_script) {
        auto const& bmi_install = *export_info.BmiInstallation;

        *bmi_install_script << "if (CMAKE_INSTALL_COMPONENT STREQUAL \""
                            << cmEscape(bmi_install.Component) << '\"';
        if (!bmi_install.ExcludeFromAll) {
          *bmi_install_script << " OR NOT CMAKE_INSTALL_COMPONENT";
        }
        *bmi_install_script << ")\n";
        *bmi_install_script << "  file(INSTALL\n"
                               "    DESTINATION \"";
        if (!bmi_dest_is_abs) {
          *bmi_install_script << "${CMAKE_INSTALL_PREFIX}/";
        }
        *bmi_install_script << cmEscape(bmi_install.Destination)
                            << "\"\n"
                               "    TYPE FILE\n";
        if (bmi_install.Optional) {
          *bmi_install_script << "    OPTIONAL\n";
        }
        if (!bmi_install.MessageLevel.empty()) {
          *bmi_install_script << "    " << bmi_install.MessageLevel << "\n";
        }
        if (!bmi_install.Permissions.empty()) {
          *bmi_install_script << "    PERMISSIONS" << bmi_install.Permissions
                              << "\n";
        }
        *bmi_install_script << "    FILES \"" << *m << "\")\n";
        if (bmi_dest_is_abs) {
          *bmi_install_script
            << "  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES\n"
               "    \""
            << cmEscape(cmSystemTools::GetFilenameName(*m))
            << "\")\n"
               "  if (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)\n"
               "    message(WARNING\n"
               "      \"ABSOLUTE path INSTALL DESTINATION : "
               "${CMAKE_ABSOLUTE_DESTINATION_FILES}\")\n"
               "  endif ()\n"
               "  if (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)\n"
               "    message(FATAL_ERROR\n"
               "      \"ABSOLUTE path INSTALL DESTINATION forbidden (by "
               "caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}\")\n"
               "  endif ()\n";
        }
        *bmi_install_script << "endif ()\n";
      }
    }
  }

  // Add trailing parenthesis for the `set_property` call.
  for (auto const& exp : exports) {
    *exp.first << ")\n";
  }

  // Check that public sources only require public modules.
  for (auto const& pub_reqs : public_source_requires) {
    for (auto const& req : pub_reqs.second) {
      if (private_modules.count(req)) {
        cmSystemTools::Error(cmStrCat(
          "Public C++ module source `", pub_reqs.first, "` requires the `",
          req, "` C++ module which is provided by a private source"));
        result = false;
      }
    }
  }

  return result;
}

bool cmDyndepCollation::IsObjectPrivate(
  std::string const& object, cmCxxModuleExportInfo const& export_info)
{
#ifdef _WIN32
  std::string output_path = object;
  cmSystemTools::ConvertToUnixSlashes(output_path);
#else
  std::string const& output_path = object;
#endif
  auto fileset_info_itr = export_info.ObjectToFileSet.find(output_path);
  if (fileset_info_itr == export_info.ObjectToFileSet.end()) {
    return false;
  }
  auto const& file_set = fileset_info_itr->second;
  return !cmFileSetVisibilityIsForInterface(file_set.Visibility);
}

bool cmDyndepCollation::IsBmiOnly(cmCxxModuleExportInfo const& exportInfo,
                                  std::string const& object)
{
#ifdef _WIN32
  auto object_path = object;
  cmSystemTools::ConvertToUnixSlashes(object_path);
#else
  auto const& object_path = object;
#endif
  auto fs = exportInfo.ObjectToFileSet.find(object_path);
  return (fs != exportInfo.ObjectToFileSet.end()) && fs->second.BmiOnly;
}
