/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPIToolchains.h"

#include <memory>
#include <string>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

struct ToolchainVariable
{
  std::string ObjectKey;
  std::string VariableSuffix;
  bool IsList;
};

class Toolchains
{
  cmFileAPI& FileAPI;
  unsigned long Version;

  Json::Value DumpToolchains();
  Json::Value DumpToolchain(std::string const& lang);
  Json::Value DumpToolchainVariables(
    cmMakefile const* mf, std::string const& lang,
    std::vector<ToolchainVariable> const& variables);
  void DumpToolchainVariable(cmMakefile const* mf, Json::Value& object,
                             std::string const& lang,
                             ToolchainVariable const& variable);

public:
  Toolchains(cmFileAPI& fileAPI, unsigned long version);
  Json::Value Dump();
};

Toolchains::Toolchains(cmFileAPI& fileAPI, unsigned long version)
  : FileAPI(fileAPI)
  , Version(version)
{
  static_cast<void>(this->Version);
}

Json::Value Toolchains::Dump()
{
  Json::Value toolchains = Json::objectValue;
  toolchains["toolchains"] = this->DumpToolchains();
  return toolchains;
}

Json::Value Toolchains::DumpToolchains()
{
  Json::Value toolchains = Json::arrayValue;

  for (std::string const& lang :
       this->FileAPI.GetCMakeInstance()->GetState()->GetEnabledLanguages()) {
    toolchains.append(this->DumpToolchain(lang));
  }

  return toolchains;
}

Json::Value Toolchains::DumpToolchain(std::string const& lang)
{
  static const std::vector<ToolchainVariable> CompilerVariables{
    { "path", "COMPILER", false },
    { "id", "COMPILER_ID", false },
    { "version", "COMPILER_VERSION", false },
    { "target", "COMPILER_TARGET", false },
  };

  static const std::vector<ToolchainVariable> CompilerImplicitVariables{
    { "includeDirectories", "IMPLICIT_INCLUDE_DIRECTORIES", true },
    { "linkDirectories", "IMPLICIT_LINK_DIRECTORIES", true },
    { "linkFrameworkDirectories", "IMPLICIT_LINK_FRAMEWORK_DIRECTORIES",
      true },
    { "linkLibraries", "IMPLICIT_LINK_LIBRARIES", true },
  };

  static const ToolchainVariable SourceFileExtensionsVariable{
    "sourceFileExtensions", "SOURCE_FILE_EXTENSIONS", true
  };

  const auto& mf =
    this->FileAPI.GetCMakeInstance()->GetGlobalGenerator()->GetMakefiles()[0];
  Json::Value toolchain = Json::objectValue;
  toolchain["language"] = lang;
  toolchain["compiler"] =
    this->DumpToolchainVariables(mf.get(), lang, CompilerVariables);
  toolchain["compiler"]["implicit"] =
    this->DumpToolchainVariables(mf.get(), lang, CompilerImplicitVariables);
  this->DumpToolchainVariable(mf.get(), toolchain, lang,
                              SourceFileExtensionsVariable);
  return toolchain;
}

Json::Value Toolchains::DumpToolchainVariables(
  cmMakefile const* mf, std::string const& lang,
  std::vector<ToolchainVariable> const& variables)
{
  Json::Value object = Json::objectValue;
  for (const auto& variable : variables) {
    this->DumpToolchainVariable(mf, object, lang, variable);
  }
  return object;
}

void Toolchains::DumpToolchainVariable(cmMakefile const* mf,
                                       Json::Value& object,
                                       std::string const& lang,
                                       ToolchainVariable const& variable)
{
  std::string const variableName =
    cmStrCat("CMAKE_", lang, "_", variable.VariableSuffix);

  if (variable.IsList) {
    std::vector<std::string> values;
    if (mf->GetDefExpandList(variableName, values)) {
      Json::Value jsonArray = Json::arrayValue;
      for (std::string const& value : values) {
        jsonArray.append(value);
      }
      object[variable.ObjectKey] = jsonArray;
    }
  } else {
    cmValue def = mf->GetDefinition(variableName);
    if (def) {
      object[variable.ObjectKey] = *def;
    }
  }
}
}

Json::Value cmFileAPIToolchainsDump(cmFileAPI& fileAPI, unsigned long version)
{
  Toolchains toolchains(fileAPI, version);
  return toolchains.Dump();
}
