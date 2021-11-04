/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmScanDepFormat.h"

#include <cctype>
#include <cstdio>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static bool ParseFilename(Json::Value const& val, std::string& result)
{
  if (val.isString()) {
    result = val.asString();
  } else {
    return false;
  }

  return true;
}

static Json::Value EncodeFilename(std::string const& path)
{
  std::string data;
  data.reserve(path.size());

  for (auto const& byte : path) {
    if (std::iscntrl(byte)) {
      // Control characters.
      data.append("\\u");
      char buf[5];
      std::snprintf(buf, sizeof(buf), "%04x", byte);
      data.append(buf);
    } else if (byte == '"' || byte == '\\') {
      // Special JSON characters.
      data.push_back('\\');
      data.push_back(byte);
    } else {
      // Other data.
      data.push_back(byte);
    }
  }

  return data;
}

#define PARSE_BLOB(val, res)                                                  \
  do {                                                                        \
    if (!ParseFilename(val, res)) {                                           \
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ", \
                                    arg_pp, ": invalid blob"));               \
      return false;                                                           \
    }                                                                         \
  } while (0)

#define PARSE_FILENAME(val, res)                                              \
  do {                                                                        \
    if (!ParseFilename(val, res)) {                                           \
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ", \
                                    arg_pp, ": invalid filename"));           \
      return false;                                                           \
    }                                                                         \
                                                                              \
    if (work_directory && !work_directory->empty() &&                         \
        !cmSystemTools::FileIsFullPath(res)) {                                \
      res = cmStrCat(*work_directory, '/', res);                              \
    }                                                                         \
  } while (0)

bool cmScanDepFormat_P1689_Parse(std::string const& arg_pp,
                                 cmScanDepInfo* info)
{
  Json::Value ppio;
  Json::Value const& ppi = ppio;
  cmsys::ifstream ppf(arg_pp.c_str(), std::ios::in | std::ios::binary);
  {
    Json::Reader reader;
    if (!reader.parse(ppf, ppio, false)) {
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                    arg_pp,
                                    reader.getFormattedErrorMessages()));
      return false;
    }
  }

  Json::Value const& version = ppi["version"];
  if (version.asUInt() > 1) {
    cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                  arg_pp, ": version ", version.asString()));
    return false;
  }

  Json::Value const& rules = ppi["rules"];
  if (rules.isArray()) {
    if (rules.size() != 1) {
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                    arg_pp, ": expected 1 source entry"));
      return false;
    }

    for (auto const& rule : rules) {
      cm::optional<std::string> work_directory;
      Json::Value const& workdir = rule["work-directory"];
      if (workdir.isString()) {
        std::string wd;
        PARSE_BLOB(workdir, wd);
        work_directory = std::move(wd);
      } else if (!workdir.isNull()) {
        cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                      arg_pp,
                                      ": work-directory is not a string"));
        return false;
      }

      if (rule.isMember("primary-output")) {
        Json::Value const& primary_output = rule["primary-output"];
        PARSE_FILENAME(primary_output, info->PrimaryOutput);
      }

      if (rule.isMember("outputs")) {
        Json::Value const& outputs = rule["outputs"];
        if (outputs.isArray()) {
          for (auto const& output : outputs) {
            std::string extra_output;
            PARSE_FILENAME(output, extra_output);

            info->ExtraOutputs.emplace_back(extra_output);
          }
        }
      }

      if (rule.isMember("provides")) {
        Json::Value const& provides = rule["provides"];
        if (!provides.isArray()) {
          cmSystemTools::Error(
            cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                     ": provides is not an array"));
          return false;
        }

        for (auto const& provide : provides) {
          cmSourceReqInfo provide_info;

          Json::Value const& logical_name = provide["logical-name"];
          PARSE_BLOB(logical_name, provide_info.LogicalName);

          if (provide.isMember("compiled-module-path")) {
            Json::Value const& compiled_module_path =
              provide["compiled-module-path"];
            PARSE_FILENAME(compiled_module_path,
                           provide_info.CompiledModulePath);
          }

          if (provide.isMember("unique-on-source-path")) {
            Json::Value const& unique_on_source_path =
              provide["unique-on-source-path"];
            if (!unique_on_source_path.isBool()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                         ": unique-on-source-path is not a boolean"));
              return false;
            }
            provide_info.UseSourcePath = unique_on_source_path.asBool();
          } else {
            provide_info.UseSourcePath = false;
          }

          if (provide.isMember("source-path")) {
            Json::Value const& source_path = provide["source-path"];
            PARSE_FILENAME(source_path, provide_info.SourcePath);
          } else if (provide_info.UseSourcePath) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                       ": source-path is missing"));
            return false;
          }

          info->Provides.push_back(provide_info);
        }
      }

      if (rule.isMember("requires")) {
        Json::Value const& reqs = rule["requires"];
        if (!reqs.isArray()) {
          cmSystemTools::Error(
            cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                     ": requires is not an array"));
          return false;
        }

        for (auto const& require : reqs) {
          cmSourceReqInfo require_info;

          Json::Value const& logical_name = require["logical-name"];
          PARSE_BLOB(logical_name, require_info.LogicalName);

          if (require.isMember("compiled-module-path")) {
            Json::Value const& compiled_module_path =
              require["compiled-module-path"];
            PARSE_FILENAME(compiled_module_path,
                           require_info.CompiledModulePath);
          }

          if (require.isMember("unique-on-source-path")) {
            Json::Value const& unique_on_source_path =
              require["unique-on-source-path"];
            if (!unique_on_source_path.isBool()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                         ": unique-on-source-path is not a boolean"));
              return false;
            }
            require_info.UseSourcePath = unique_on_source_path.asBool();
          } else {
            require_info.UseSourcePath = false;
          }

          if (require.isMember("source-path")) {
            Json::Value const& source_path = require["source-path"];
            PARSE_FILENAME(source_path, require_info.SourcePath);
          } else if (require_info.UseSourcePath) {
            cmSystemTools::Error(
              cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                       ": source-path is missing"));
            return false;
          }

          if (require.isMember("lookup-method")) {
            Json::Value const& lookup_method = require["lookup-method"];
            if (!lookup_method.isString()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                         ": lookup-method is not a string"));
              return false;
            }

            std::string lookup_method_str = lookup_method.asString();
            if (lookup_method_str == "by-name"_s) {
              require_info.Method = LookupMethod::ByName;
            } else if (lookup_method_str == "include-angle"_s) {
              require_info.Method = LookupMethod::IncludeAngle;
            } else if (lookup_method_str == "include-quote"_s) {
              require_info.Method = LookupMethod::IncludeQuote;
            } else {
              cmSystemTools::Error(cmStrCat(
                "-E cmake_ninja_dyndep failed to parse ", arg_pp,
                ": lookup-method is not a valid: ", lookup_method_str));
              return false;
            }
          } else if (require_info.UseSourcePath) {
            require_info.Method = LookupMethod::ByName;
          }

          info->Requires.push_back(require_info);
        }
      }
    }
  }

  return true;
}

bool cmScanDepFormat_P1689_Write(std::string const& path,
                                 cmScanDepInfo const& info)
{
  Json::Value ddi(Json::objectValue);
  ddi["version"] = 0;
  ddi["revision"] = 0;

  Json::Value& rules = ddi["rules"] = Json::arrayValue;

  Json::Value rule(Json::objectValue);

  rule["primary-output"] = EncodeFilename(info.PrimaryOutput);

  Json::Value& rule_outputs = rule["outputs"] = Json::arrayValue;
  for (auto const& output : info.ExtraOutputs) {
    rule_outputs.append(EncodeFilename(output));
  }

  Json::Value& provides = rule["provides"] = Json::arrayValue;
  for (auto const& provide : info.Provides) {
    Json::Value provide_obj(Json::objectValue);
    auto const encoded = EncodeFilename(provide.LogicalName);
    provide_obj["logical-name"] = encoded;
    if (!provide.CompiledModulePath.empty()) {
      provide_obj["compiled-module-path"] =
        EncodeFilename(provide.CompiledModulePath);
    }

    if (provide.UseSourcePath) {
      provide_obj["unique-on-source-path"] = true;
      provide_obj["source-path"] = EncodeFilename(provide.SourcePath);
    } else if (!provide.SourcePath.empty()) {
      provide_obj["source-path"] = EncodeFilename(provide.SourcePath);
    }

    provides.append(provide_obj);
  }

  Json::Value& reqs = rule["requires"] = Json::arrayValue;
  for (auto const& require : info.Requires) {
    Json::Value require_obj(Json::objectValue);
    auto const encoded = EncodeFilename(require.LogicalName);
    require_obj["logical-name"] = encoded;
    if (!require.CompiledModulePath.empty()) {
      require_obj["compiled-module-path"] =
        EncodeFilename(require.CompiledModulePath);
    }

    if (require.UseSourcePath) {
      require_obj["unique-on-source-path"] = true;
      require_obj["source-path"] = EncodeFilename(require.SourcePath);
    } else if (!require.SourcePath.empty()) {
      require_obj["source-path"] = EncodeFilename(require.SourcePath);
    }

    const char* lookup_method = nullptr;
    switch (require.Method) {
      case LookupMethod::ByName:
        // No explicit value needed for the default.
        break;
      case LookupMethod::IncludeAngle:
        lookup_method = "include-angle";
        break;
      case LookupMethod::IncludeQuote:
        lookup_method = "include-quote";
        break;
    }
    if (lookup_method) {
      require_obj["lookup-method"] = lookup_method;
    }

    reqs.append(require_obj);
  }

  rules.append(rule);

  cmGeneratedFileStream ddif(path);
  ddif << ddi;

  return !!ddif;
}
