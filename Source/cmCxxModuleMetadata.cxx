/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmCxxModuleMetadata.h"

#include <algorithm>
#include <set>
#include <string>
#include <utility>

#include <cmext/string_view>

#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

#include "cmFileSet.h"
#include "cmJSONState.h"
#include "cmListFileCache.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

namespace {

bool JsonIsStringArray(Json::Value const& v)
{
  return v.isArray() &&
    std::all_of(v.begin(), v.end(),
                [](Json::Value const& it) { return it.isString(); });
}

bool ParsePreprocessorDefine(Json::Value& dval,
                             cmCxxModuleMetadata::PreprocessorDefineData& out,
                             cmJSONState* state)
{
  if (!dval.isObject()) {
    state->AddErrorAtValue("each entry in 'definitions' must be an object",
                           &dval);
    return false;
  }

  if (!dval.isMember("name") || !dval["name"].isString() ||
      dval["name"].asString().empty()) {
    state->AddErrorAtValue(
      "preprocessor definition requires a non-empty 'name'", &dval["name"]);
    return false;
  }
  out.Name = dval["name"].asString();

  if (dval.isMember("value")) {
    if (dval["value"].isString()) {
      out.Value = dval["value"].asString();
    } else if (!dval["value"].isNull()) {
      state->AddErrorAtValue(
        "'value' in preprocessor definition must be string or null",
        &dval["value"]);
      return false;
    }
  }

  if (dval.isMember("undef")) {
    if (!dval["undef"].isBool()) {
      state->AddErrorAtValue(
        "'undef' in preprocessor definition must be boolean", &dval["undef"]);
      return false;
    }
    out.Undef = dval["undef"].asBool();
  }

  if (dval.isMember("vendor")) {
    out.Vendor = std::move(dval["vendor"]);
  }

  return true;
}

bool ParseLocalArguments(Json::Value& lav,
                         cmCxxModuleMetadata::LocalArgumentsData& out,
                         cmJSONState* state)
{
  if (!lav.isObject()) {
    state->AddErrorAtValue("'local-arguments' must be an object", &lav);
    return false;
  }

  if (lav.isMember("include-directories")) {
    if (!JsonIsStringArray(lav["include-directories"])) {
      state->AddErrorAtValue(
        "'include-directories' must be an array of strings",
        &lav["include-directories"]);
      return false;
    }
    for (auto const& s : lav["include-directories"]) {
      out.IncludeDirectories.push_back(s.asString());
    }
  }

  if (lav.isMember("system-include-directories")) {
    if (!JsonIsStringArray(lav["system-include-directories"])) {
      state->AddErrorAtValue(
        "'system-include-directories' must be an array of strings",
        &lav["system-include-directories"]);
      return false;
    }
    for (auto const& s : lav["system-include-directories"]) {
      out.SystemIncludeDirectories.push_back(s.asString());
    }
  }

  if (lav.isMember("definitions")) {
    if (!lav["definitions"].isArray()) {
      state->AddErrorAtValue("'definitions' must be an array",
                             &lav["definitions"]);
      return false;
    }
    for (Json::Value& dval : lav["definitions"]) {
      out.Definitions.emplace_back();
      if (!ParsePreprocessorDefine(dval, out.Definitions.back(), state)) {
        return false;
      }
    }
  }

  if (lav.isMember("vendor")) {
    out.Vendor = std::move(lav["vendor"]);
  }

  return true;
}

bool ParseModule(Json::Value& mval, cmCxxModuleMetadata::ModuleData& mod,
                 cmJSONState* state)
{
  if (!mval.isObject()) {
    state->AddErrorAtValue("each entry in 'modules' must be an object", &mval);
    return false;
  }

  if (!mval.isMember("logical-name") || !mval["logical-name"].isString() ||
      mval["logical-name"].asString().empty()) {
    state->AddErrorAtValue(
      "module entries require a non-empty 'logical-name' string",
      &mval["logical-name"]);
    return false;
  }
  mod.LogicalName = mval["logical-name"].asString();

  if (!mval.isMember("source-path") || !mval["source-path"].isString() ||
      mval["source-path"].asString().empty()) {
    state->AddErrorAtValue(
      "module entries require a non-empty 'source-path' string",
      &mval["source-path"]);
    return false;
  }
  mod.SourcePath = mval["source-path"].asString();

  if (mval.isMember("is-interface")) {
    if (!mval["is-interface"].isBool()) {
      state->AddErrorAtValue("'is-interface' must be boolean",
                             &mval["is-interface"]);
      return false;
    }
    mod.IsInterface = mval["is-interface"].asBool();
  } else {
    mod.IsInterface = true;
  }

  if (mval.isMember("is-std-library")) {
    if (!mval["is-std-library"].isBool()) {
      state->AddErrorAtValue("'is-std-library' must be boolean",
                             &mval["is-std-library"]);
      return false;
    }
    mod.IsStdLibrary = mval["is-std-library"].asBool();
  } else {
    mod.IsStdLibrary = false;
  }

  if (mval.isMember("local-arguments")) {
    mod.LocalArguments.emplace();
    if (!ParseLocalArguments(mval["local-arguments"], *mod.LocalArguments,
                             state)) {
      return false;
    }
  }

  if (mval.isMember("vendor")) {
    mod.Vendor = std::move(mval["vendor"]);
  }

  return true;
}

bool ParseRoot(Json::Value& root, cmCxxModuleMetadata& meta,
               cmJSONState* state)
{
  if (!root.isMember("version") || !root["version"].isInt()) {
    state->AddErrorAtValue(
      "Top-level member 'version' is required and must be an integer", &root);
    return false;
  }
  meta.Version = root["version"].asInt();

  if (root.isMember("revision")) {
    if (!root["revision"].isInt()) {
      state->AddErrorAtValue("'revision' must be an integer",
                             &root["revision"]);
      return false;
    }
    meta.Revision = root["revision"].asInt();
  }

  if (root.isMember("modules")) {
    if (!root["modules"].isArray()) {
      state->AddErrorAtValue("'modules' must be an array", &root["modules"]);
      return false;
    }
    for (Json::Value& mval : root["modules"]) {
      meta.Modules.emplace_back();
      if (!ParseModule(mval, meta.Modules.back(), state)) {
        return false;
      }
    }
  }

  for (std::string& key : root.getMemberNames()) {
    if (key == "version" || key == "revision" || key == "modules") {
      continue;
    }
    meta.Extensions.emplace(std::move(key), std::move(root[key]));
  }

  return true;
}

} // namespace

cmCxxModuleMetadata::ParseResult cmCxxModuleMetadata::LoadFromFile(
  std::string const& path)
{
  ParseResult res;

  Json::Value root;
  cmJSONState parseState(path, &root);
  if (!parseState.errors.empty()) {
    res.Error = parseState.GetErrorMessage();
    return res;
  }

  cmCxxModuleMetadata meta;
  if (!ParseRoot(root, meta, &parseState)) {
    res.Error = parseState.GetErrorMessage();
    return res;
  }

  meta.MetadataFilePath = path;
  res.Meta = std::move(meta);
  return res;
}

namespace {

Json::Value SerializePreprocessorDefine(
  cmCxxModuleMetadata::PreprocessorDefineData const& d)
{
  Json::Value dv(Json::objectValue);
  dv["name"] = d.Name;
  if (d.Value) {
    dv["value"] = *d.Value;
  } else {
    dv["value"] = Json::Value::null;
  }
  dv["undef"] = d.Undef;
  if (d.Vendor) {
    dv["vendor"] = *d.Vendor;
  }
  return dv;
}

Json::Value SerializeLocalArguments(
  cmCxxModuleMetadata::LocalArgumentsData const& la)
{
  Json::Value lav(Json::objectValue);

  if (!la.IncludeDirectories.empty()) {
    Json::Value& inc = lav["include-directories"] = Json::arrayValue;
    for (auto const& s : la.IncludeDirectories) {
      inc.append(s);
    }
  }

  if (!la.SystemIncludeDirectories.empty()) {
    Json::Value& sinc = lav["system-include-directories"] = Json::arrayValue;
    for (auto const& s : la.SystemIncludeDirectories) {
      sinc.append(s);
    }
  }

  if (!la.Definitions.empty()) {
    Json::Value& defs = lav["definitions"] = Json::arrayValue;
    for (auto const& d : la.Definitions) {
      defs.append(SerializePreprocessorDefine(d));
    }
  }

  if (la.Vendor) {
    lav["vendor"] = *la.Vendor;
  }

  return lav;
}

Json::Value SerializeModule(cmCxxModuleMetadata::ModuleData const& m)
{
  Json::Value mv(Json::objectValue);
  mv["logical-name"] = m.LogicalName;
  mv["source-path"] = m.SourcePath;
  mv["is-interface"] = m.IsInterface;
  mv["is-std-library"] = m.IsStdLibrary;

  if (m.LocalArguments) {
    mv["local-arguments"] = SerializeLocalArguments(*m.LocalArguments);
  }

  if (m.Vendor) {
    mv["vendor"] = *m.Vendor;
  }

  return mv;
}

} // namespace

Json::Value cmCxxModuleMetadata::ToJsonValue(cmCxxModuleMetadata const& meta)
{
  Json::Value root(Json::objectValue);

  root["version"] = meta.Version;
  root["revision"] = meta.Revision;

  Json::Value& modules = root["modules"] = Json::arrayValue;
  for (auto const& m : meta.Modules) {
    modules.append(SerializeModule(m));
  }

  for (auto const& kv : meta.Extensions) {
    root[kv.first] = kv.second;
  }

  return root;
}

cmCxxModuleMetadata::SaveResult cmCxxModuleMetadata::SaveToFile(
  std::string const& path, cmCxxModuleMetadata const& meta)
{
  SaveResult st;

  cmsys::ofstream ofs(path.c_str());
  if (!ofs.is_open()) {
    st.Error = cmStrCat("Unable to open file for writing: "_s, path);
    return st;
  }

  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "  ";
  ofs << Json::writeString(wbuilder, ToJsonValue(meta));

  if (!ofs.good()) {
    st.Error = cmStrCat("Write failed for file: "_s, path);
    return st;
  }

  st.Ok = true;
  return st;
}

void cmCxxModuleMetadata::PopulateTarget(
  cmTarget& target, cmCxxModuleMetadata const& meta,
  std::vector<std::string> const& configs)
{
  std::vector<cm::string_view> allIncludeDirectories;
  std::vector<std::string> allCompileDefinitions;
  std::set<std::string> baseDirs;

  std::string metadataDir =
    cmSystemTools::GetFilenamePath(meta.MetadataFilePath);

  auto fileSet = target.GetOrCreateFileSet("CXX_MODULES", "CXX_MODULES",
                                           cmFileSetVisibility::Interface);

  for (auto const& module : meta.Modules) {
    std::string sourcePath = module.SourcePath;
    if (!cmSystemTools::FileIsFullPath(sourcePath)) {
      sourcePath = cmStrCat(metadataDir, '/', sourcePath);
    }

    // Module metadata files can reference files in different roots,
    // just use the immediate parent directory as a base directory
    baseDirs.insert(cmSystemTools::GetFilenamePath(sourcePath));

    fileSet.first->AddFileEntry(sourcePath);

    if (module.LocalArguments) {
      for (auto const& incDir : module.LocalArguments->IncludeDirectories) {
        allIncludeDirectories.push_back(incDir);
      }
      for (auto const& sysIncDir :
           module.LocalArguments->SystemIncludeDirectories) {
        allIncludeDirectories.push_back(sysIncDir);
      }

      for (auto const& def : module.LocalArguments->Definitions) {
        if (!def.Undef) {
          if (def.Value) {
            allCompileDefinitions.push_back(
              cmStrCat(def.Name, "="_s, *def.Value));
          } else {
            allCompileDefinitions.push_back(def.Name);
          }
        }
      }
    }
  }

  for (auto const& baseDir : baseDirs) {
    fileSet.first->AddDirectoryEntry(baseDir);
  }

  if (!allIncludeDirectories.empty()) {
    target.SetProperty("IMPORTED_CXX_MODULES_INCLUDE_DIRECTORIES",
                       cmJoin(allIncludeDirectories, ";"));
  }

  if (!allCompileDefinitions.empty()) {
    target.SetProperty("IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS",
                       cmJoin(allCompileDefinitions, ";"));
  }

  for (auto const& config : configs) {
    std::vector<std::string> moduleList;
    for (auto const& module : meta.Modules) {
      if (module.IsInterface) {
        std::string sourcePath = module.SourcePath;
        if (!cmSystemTools::FileIsFullPath(sourcePath)) {
          sourcePath = cmStrCat(metadataDir, '/', sourcePath);
        }
        moduleList.push_back(cmStrCat(module.LogicalName, "="_s, sourcePath));
      }
    }

    if (!moduleList.empty()) {
      std::string upperConfig = cmSystemTools::UpperCase(config);
      std::string propertyName =
        cmStrCat("IMPORTED_CXX_MODULES_"_s, upperConfig);
      target.SetProperty(propertyName, cmJoin(moduleList, ";"));
    }
  }
}
