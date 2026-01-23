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
#include "cmGeneratedFileStream.h"
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

  return true;
}

bool ParseCMakeLocalArgumentsVendor(
  Json::Value& cmlav, cmCxxModuleMetadata::LocalArgumentsData& out,
  cmJSONState* state)
{

  if (!cmlav.isObject()) {
    state->AddErrorAtValue("'vendor' must be an object", &cmlav);
    return false;
  }

  if (cmlav.isMember("compile-options")) {
    if (!JsonIsStringArray(cmlav["compile-options"])) {
      state->AddErrorAtValue("'compile-options' must be an array of strings",
                             &cmlav["compile-options"]);
      return false;
    }
    for (auto const& s : cmlav["compile-options"]) {
      out.CompileOptions.push_back(s.asString());
    }
  }

  if (cmlav.isMember("compile-features")) {
    if (!JsonIsStringArray(cmlav["compile-features"])) {
      state->AddErrorAtValue("'compile-features' must be an array of strings",
                             &cmlav["compile-features"]);
      return false;
    }
    for (auto const& s : cmlav["compile-features"]) {
      out.CompileFeatures.push_back(s.asString());
    }
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
    if (!ParseCMakeLocalArgumentsVendor(lav["vendor"], out, state)) {
      return false;
    }
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

  if (meta.Version != 1) {
    state->AddErrorAtValue(cmStrCat("Module manifest version number, '",
                                    meta.Version, '.', meta.Revision,
                                    "' is newer than max supported (1.1)"),
                           &root);
    return false;
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
  }
  if (d.Undef) {
    dv["undef"] = d.Undef;
  }
  return dv;
}

Json::Value SerializeCMakeLocalArgumentsVendor(
  cmCxxModuleMetadata::LocalArgumentsData const& la)
{
  Json::Value vend(Json::objectValue);

  if (!la.CompileOptions.empty()) {
    Json::Value& opts = vend["compile-options"] = Json::arrayValue;
    for (auto const& s : la.CompileOptions) {
      opts.append(s);
    }
  }

  if (!la.CompileFeatures.empty()) {
    Json::Value& feats = vend["compile-features"] = Json::arrayValue;
    for (auto const& s : la.CompileFeatures) {
      feats.append(s);
    }
  }

  return vend;
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

  Json::Value vend = SerializeCMakeLocalArgumentsVendor(la);
  if (!vend.empty()) {
    Json::Value& cmvend = lav["vendor"] = Json::objectValue;
    cmvend["cmake"] = std::move(vend);
  }

  return lav;
}

Json::Value SerializeModule(std::string& manifestRoot,
                            cmCxxModuleMetadata::ModuleData const& m)
{
  Json::Value mv(Json::objectValue);
  mv["logical-name"] = m.LogicalName;
  if (cmSystemTools::FileIsFullPath(m.SourcePath)) {
    mv["source-path"] = m.SourcePath;
  } else {
    mv["source-path"] = cmSystemTools::ForceToRelativePath(
      manifestRoot, cmStrCat('/', m.SourcePath));
  }
  mv["is-interface"] = m.IsInterface;
  mv["is-std-library"] = m.IsStdLibrary;

  if (m.LocalArguments) {
    mv["local-arguments"] = SerializeLocalArguments(*m.LocalArguments);
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
  std::string manifestRoot =
    cmSystemTools::GetFilenamePath(meta.MetadataFilePath);

  if (!cmSystemTools::FileIsFullPath(meta.MetadataFilePath)) {
    manifestRoot = cmStrCat('/', manifestRoot);
  }

  for (auto const& m : meta.Modules) {
    modules.append(SerializeModule(manifestRoot, m));
  }

  return root;
}

cmCxxModuleMetadata::SaveResult cmCxxModuleMetadata::SaveToFile(
  std::string const& path, cmCxxModuleMetadata const& meta)
{
  SaveResult st;

  cmGeneratedFileStream ofs(path);
  if (!ofs.is_open()) {
    st.Error = "Unable to open temp file for writing";
    return st;
  }

  Json::StreamWriterBuilder wbuilder;
  wbuilder["indentation"] = "  ";
  ofs << Json::writeString(wbuilder, ToJsonValue(meta));

  ofs.Close();

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
  std::vector<cm::string_view> allCompileOptions;
  std::vector<cm::string_view> allCompileFeatures;
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
      for (auto const& opt : module.LocalArguments->CompileOptions) {
        allCompileOptions.push_back(opt);
      }
      for (auto const& opt : module.LocalArguments->CompileFeatures) {
        allCompileFeatures.push_back(opt);
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

  if (!allCompileOptions.empty()) {
    target.SetProperty("IMPORTED_CXX_MODULES_COMPILE_OPTIONS",
                       cmJoin(allCompileOptions, ";"));
  }

  if (!allCompileFeatures.empty()) {
    target.SetProperty("IMPORTED_CXX_MODULES_COMPILE_FEATURES",
                       cmJoin(allCompileFeatures, ";"));
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
