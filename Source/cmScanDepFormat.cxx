/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmScanDepFormat.h"

#include <cctype>
#include <cstdio>

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
    if (!cmSystemTools::FileIsFullPath(res)) {                                \
      res = cmStrCat(work_directory, '/', res);                               \
    }                                                                         \
  } while (0)

bool cmScanDepFormat_P1689_Parse(std::string const& arg_pp, cmSourceInfo* info)
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
  if (version.asUInt() != 0) {
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
      Json::Value const& workdir = rule["work-directory"];
      if (!workdir.isString()) {
        cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                      arg_pp,
                                      ": work-directory is not a string"));
        return false;
      }
      std::string work_directory;
      PARSE_BLOB(workdir, work_directory);

      Json::Value const& depends = rule["depends"];
      if (depends.isArray()) {
        std::string depend_filename;
        for (auto const& depend : depends) {
          PARSE_FILENAME(depend, depend_filename);
          info->Includes.push_back(depend_filename);
        }
      }

      if (rule.isMember("future-compile")) {
        Json::Value const& future_compile = rule["future-compile"];

        if (future_compile.isMember("outputs")) {
          Json::Value const& outputs = future_compile["outputs"];
          if (outputs.isArray()) {
            if (outputs.empty()) {
              cmSystemTools::Error(
                cmStrCat("-E cmake_ninja_dyndep failed to parse ", arg_pp,
                         ": expected at least one 1 output"));
              return false;
            }

            PARSE_FILENAME(outputs[0], info->PrimaryOutput);
          }
        }

        if (future_compile.isMember("provides")) {
          Json::Value const& provides = future_compile["provides"];
          if (provides.isArray()) {
            for (auto const& provide : provides) {
              cmSourceReqInfo provide_info;

              Json::Value const& logical_name = provide["logical-name"];
              PARSE_BLOB(logical_name, provide_info.LogicalName);

              if (provide.isMember("compiled-module-path")) {
                Json::Value const& compiled_module_path =
                  provide["compiled-module-path"];
                PARSE_FILENAME(compiled_module_path,
                               provide_info.CompiledModulePath);
              } else {
                provide_info.CompiledModulePath =
                  cmStrCat(provide_info.LogicalName, ".mod");
              }

              info->Provides.push_back(provide_info);
            }
          }
        }

        if (future_compile.isMember("requires")) {
          Json::Value const& reqs = future_compile["requires"];
          if (reqs.isArray()) {
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

              info->Requires.push_back(require_info);
            }
          }
        }
      }
    }
  }

  return true;
}

bool cmScanDepFormat_P1689_Write(std::string const& path,
                                 std::string const& input,
                                 cmSourceInfo const& info)
{
  Json::Value ddi(Json::objectValue);
  ddi["version"] = 0;
  ddi["revision"] = 0;

  Json::Value& rules = ddi["rules"] = Json::arrayValue;

  Json::Value rule(Json::objectValue);
  rule["work-directory"] =
    EncodeFilename(cmSystemTools::GetCurrentWorkingDirectory());
  Json::Value& inputs = rule["inputs"] = Json::arrayValue;
  inputs.append(EncodeFilename(input));

  Json::Value& rule_outputs = rule["outputs"] = Json::arrayValue;
  rule_outputs.append(EncodeFilename(path));

  Json::Value& depends = rule["depends"] = Json::arrayValue;
  for (auto const& include : info.Includes) {
    depends.append(EncodeFilename(include));
  }

  Json::Value& future_compile = rule["future-compile"] = Json::objectValue;

  Json::Value& outputs = future_compile["outputs"] = Json::arrayValue;
  outputs.append(info.PrimaryOutput);

  Json::Value& provides = future_compile["provides"] = Json::arrayValue;
  for (auto const& provide : info.Provides) {
    Json::Value provide_obj(Json::objectValue);
    auto const encoded = EncodeFilename(provide.LogicalName);
    provide_obj["logical-name"] = encoded;
    if (provide.CompiledModulePath.empty()) {
      provide_obj["compiled-module-path"] = encoded;
    } else {
      provide_obj["compiled-module-path"] =
        EncodeFilename(provide.CompiledModulePath);
    }

    // TODO: Source file tracking. See below.

    provides.append(provide_obj);
  }

  Json::Value& reqs = future_compile["requires"] = Json::arrayValue;
  for (auto const& require : info.Requires) {
    Json::Value require_obj(Json::objectValue);
    auto const encoded = EncodeFilename(require.LogicalName);
    require_obj["logical-name"] = encoded;
    if (require.CompiledModulePath.empty()) {
      require_obj["compiled-module-path"] = encoded;
    } else {
      require_obj["compiled-module-path"] =
        EncodeFilename(require.CompiledModulePath);
    }

    // TODO: Source filename inclusion. Requires collating with the provides
    // filenames (as a sanity check if available on both sides).

    reqs.append(require_obj);
  }

  rules.append(rule);

  cmGeneratedFileStream ddif(path);
  ddif << ddi;

  return !!ddif;
}
