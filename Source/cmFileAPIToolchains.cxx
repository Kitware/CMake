/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileAPIToolchains.h"

#include <memory>
#include <string>
#include <vector>

#include <cm3p/json/value.h>

#include "cmFileAPI.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
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
  bool OmitEmpty;
};

class Toolchains
{
  cmFileAPI& FileAPI;
  unsigned int Version;

  Json::Value DumpToolchains();
  Json::Value DumpToolchain(std::string const& lang);
  Json::Value DumpToolchainVariables(
    cmMakefile const* mf, std::string const& lang,
    std::vector<ToolchainVariable> const& variables);
  void DumpToolchainVariable(cmMakefile const* mf, Json::Value& object,
                             std::string const& lang,
                             ToolchainVariable const& variable);

public:
  Toolchains(cmFileAPI& fileAPI, unsigned int version);
  Json::Value Dump();
};

Toolchains::Toolchains(cmFileAPI& fileAPI, unsigned int version)
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
  static std::vector<ToolchainVariable> const CompilerVariables{
    { "path", "COMPILER", false, false },
    { "commandFragment", "COMPILER_ARG1", false, true },
    { "id", "COMPILER_ID", false, false },
    { "version", "COMPILER_VERSION", false, false },
    { "target", "COMPILER_TARGET", false, false },
  };

  static std::vector<ToolchainVariable> const CompilerImplicitVariables{
    { "includeDirectories", "IMPLICIT_INCLUDE_DIRECTORIES", true, false },
    { "linkDirectories", "IMPLICIT_LINK_DIRECTORIES", true, false },
    { "linkFrameworkDirectories", "IMPLICIT_LINK_FRAMEWORK_DIRECTORIES", true,
      false },
    { "linkLibraries", "IMPLICIT_LINK_LIBRARIES", true, false },
  };

  static ToolchainVariable const SourceFileExtensionsVariable{
    "sourceFileExtensions", "SOURCE_FILE_EXTENSIONS", true, false
  };

  auto const& mf =
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
  for (auto const& variable : variables) {
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
    cmStrCat("CMAKE_", lang, '_', variable.VariableSuffix);

  if (variable.IsList) {
    cmValue data = mf->GetDefinition(variableName);
    if (data) {
      cmList values(data);
      if (!variable.OmitEmpty || !values.empty()) {
        Json::Value jsonArray = Json::arrayValue;
        for (auto const& value : values) {
          jsonArray.append(value);
        }
        object[variable.ObjectKey] = jsonArray;
      }
    }
  } else {
    cmValue def = mf->GetDefinition(variableName);
    if (def && (!variable.OmitEmpty || !def.IsEmpty())) {
      object[variable.ObjectKey] = *def;
    }
  }
}
}

Json::Value cmFileAPIToolchainsDump(cmFileAPI& fileAPI, unsigned int version)
{
  Toolchains toolchains(fileAPI, version);
  return toolchains.Dump();
}
