/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmRulePlaceholderExpander.h"

#include <utility>

#include "cmOutputConverter.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmRulePlaceholderExpander::cmRulePlaceholderExpander(
  std::map<std::string, std::string> compilers,
  std::map<std::string, std::string> variableMappings,
  std::string compilerSysroot, std::string linkerSysroot)
  : Compilers(std::move(compilers))
  , VariableMappings(std::move(variableMappings))
  , CompilerSysroot(std::move(compilerSysroot))
  , LinkerSysroot(std::move(linkerSysroot))
{
}

std::string cmRulePlaceholderExpander::ExpandVariable(
  std::string const& variable)
{
  if (this->ReplaceValues->LinkFlags) {
    if (variable == "LINK_FLAGS") {
      return this->ReplaceValues->LinkFlags;
    }
  }
  if (this->ReplaceValues->Manifests) {
    if (variable == "MANIFESTS") {
      return this->ReplaceValues->Manifests;
    }
  }
  if (this->ReplaceValues->Flags) {
    if (variable == "FLAGS") {
      return this->ReplaceValues->Flags;
    }
  }

  if (this->ReplaceValues->Source) {
    if (variable == "SOURCE") {
      return this->ReplaceValues->Source;
    }
  }
  if (this->ReplaceValues->DynDepFile) {
    if (variable == "DYNDEP_FILE") {
      return this->ReplaceValues->DynDepFile;
    }
  }
  if (this->ReplaceValues->PreprocessedSource) {
    if (variable == "PREPROCESSED_SOURCE") {
      return this->ReplaceValues->PreprocessedSource;
    }
  }
  if (this->ReplaceValues->AssemblySource) {
    if (variable == "ASSEMBLY_SOURCE") {
      return this->ReplaceValues->AssemblySource;
    }
  }
  if (this->ReplaceValues->Object) {
    if (variable == "OBJECT") {
      return this->ReplaceValues->Object;
    }
  }
  if (this->ReplaceValues->ObjectDir) {
    if (variable == "OBJECT_DIR") {
      return this->ReplaceValues->ObjectDir;
    }
  }
  if (this->ReplaceValues->ObjectFileDir) {
    if (variable == "OBJECT_FILE_DIR") {
      return this->ReplaceValues->ObjectFileDir;
    }
  }
  if (this->ReplaceValues->Objects) {
    if (variable == "OBJECTS") {
      return this->ReplaceValues->Objects;
    }
  }
  if (this->ReplaceValues->ObjectsQuoted) {
    if (variable == "OBJECTS_QUOTED") {
      return this->ReplaceValues->ObjectsQuoted;
    }
  }
  if (this->ReplaceValues->CudaCompileMode) {
    if (variable == "CUDA_COMPILE_MODE") {
      return this->ReplaceValues->CudaCompileMode;
    }
  }
  if (this->ReplaceValues->AIXExports) {
    if (variable == "AIX_EXPORTS") {
      return this->ReplaceValues->AIXExports;
    }
  }
  if (this->ReplaceValues->ISPCHeader) {
    if (variable == "ISPC_HEADER") {
      return this->ReplaceValues->ISPCHeader;
    }
  }
  if (this->ReplaceValues->Defines && variable == "DEFINES") {
    return this->ReplaceValues->Defines;
  }
  if (this->ReplaceValues->Includes && variable == "INCLUDES") {
    return this->ReplaceValues->Includes;
  }
  if (this->ReplaceValues->SwiftLibraryName) {
    if (variable == "SWIFT_LIBRARY_NAME") {
      return this->ReplaceValues->SwiftLibraryName;
    }
  }
  if (this->ReplaceValues->SwiftModule) {
    if (variable == "SWIFT_MODULE") {
      return this->ReplaceValues->SwiftModule;
    }
  }
  if (this->ReplaceValues->SwiftModuleName) {
    if (variable == "SWIFT_MODULE_NAME") {
      return this->ReplaceValues->SwiftModuleName;
    }
  }
  if (this->ReplaceValues->SwiftOutputFileMap) {
    if (variable == "SWIFT_OUTPUT_FILE_MAP") {
      return this->ReplaceValues->SwiftOutputFileMap;
    }
  }
  if (this->ReplaceValues->SwiftSources) {
    if (variable == "SWIFT_SOURCES") {
      return this->ReplaceValues->SwiftSources;
    }
  }
  if (this->ReplaceValues->TargetPDB) {
    if (variable == "TARGET_PDB") {
      return this->ReplaceValues->TargetPDB;
    }
  }
  if (this->ReplaceValues->TargetCompilePDB) {
    if (variable == "TARGET_COMPILE_PDB") {
      return this->ReplaceValues->TargetCompilePDB;
    }
  }
  if (this->ReplaceValues->DependencyFile) {
    if (variable == "DEP_FILE") {
      return this->ReplaceValues->DependencyFile;
    }
  }
  if (this->ReplaceValues->DependencyTarget) {
    if (variable == "DEP_TARGET") {
      return this->ReplaceValues->DependencyTarget;
    }
  }
  if (this->ReplaceValues->Fatbinary) {
    if (variable == "FATBINARY") {
      return this->ReplaceValues->Fatbinary;
    }
  }
  if (this->ReplaceValues->RegisterFile) {
    if (variable == "REGISTER_FILE") {
      return this->ReplaceValues->RegisterFile;
    }
  }

  if (this->ReplaceValues->Target) {
    if (variable == "TARGET_QUOTED") {
      std::string targetQuoted = this->ReplaceValues->Target;
      if (!targetQuoted.empty() && targetQuoted.front() != '\"') {
        targetQuoted = '\"';
        targetQuoted += this->ReplaceValues->Target;
        targetQuoted += '\"';
      }
      return targetQuoted;
    }
    if (variable == "TARGET_UNQUOTED") {
      std::string unquoted = this->ReplaceValues->Target;
      std::string::size_type sz = unquoted.size();
      if (sz > 2 && unquoted.front() == '\"' && unquoted.back() == '\"') {
        unquoted = unquoted.substr(1, sz - 2);
      }
      return unquoted;
    }
    if (this->ReplaceValues->LanguageCompileFlags) {
      if (variable == "LANGUAGE_COMPILE_FLAGS") {
        return this->ReplaceValues->LanguageCompileFlags;
      }
    }
    if (this->ReplaceValues->Target) {
      if (variable == "TARGET") {
        return this->ReplaceValues->Target;
      }
    }
    if (variable == "TARGET_IMPLIB") {
      return this->TargetImpLib;
    }
    if (variable == "TARGET_VERSION_MAJOR") {
      if (this->ReplaceValues->TargetVersionMajor) {
        return this->ReplaceValues->TargetVersionMajor;
      }
      return "0";
    }
    if (variable == "TARGET_VERSION_MINOR") {
      if (this->ReplaceValues->TargetVersionMinor) {
        return this->ReplaceValues->TargetVersionMinor;
      }
      return "0";
    }
    if (this->ReplaceValues->Target) {
      if (variable == "TARGET_BASE") {
        // Strip the last extension off the target name.
        std::string targetBase = this->ReplaceValues->Target;
        std::string::size_type pos = targetBase.rfind('.');
        if (pos != std::string::npos) {
          return targetBase.substr(0, pos);
        }
        return targetBase;
      }
    }
  }
  if (variable == "TARGET_SONAME" || variable == "SONAME_FLAG" ||
      variable == "TARGET_INSTALLNAME_DIR") {
    // All these variables depend on TargetSOName
    if (this->ReplaceValues->TargetSOName) {
      if (variable == "TARGET_SONAME") {
        return this->ReplaceValues->TargetSOName;
      }
      if (variable == "SONAME_FLAG" && this->ReplaceValues->SONameFlag) {
        return this->ReplaceValues->SONameFlag;
      }
      if (this->ReplaceValues->TargetInstallNameDir &&
          variable == "TARGET_INSTALLNAME_DIR") {
        return this->ReplaceValues->TargetInstallNameDir;
      }
    }
    return "";
  }
  if (this->ReplaceValues->LinkLibraries) {
    if (variable == "LINK_LIBRARIES") {
      return this->ReplaceValues->LinkLibraries;
    }
  }
  if (this->ReplaceValues->Language) {
    if (variable == "LANGUAGE") {
      return this->ReplaceValues->Language;
    }
  }
  if (this->ReplaceValues->CMTargetName) {
    if (variable == "TARGET_NAME") {
      return this->ReplaceValues->CMTargetName;
    }
  }
  if (this->ReplaceValues->CMTargetType) {
    if (variable == "TARGET_TYPE") {
      return this->ReplaceValues->CMTargetType;
    }
  }
  if (this->ReplaceValues->Output) {
    if (variable == "OUTPUT") {
      return this->ReplaceValues->Output;
    }
  }
  if (variable == "CMAKE_COMMAND") {
    return this->OutputConverter->ConvertToOutputFormat(
      cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
  }

  auto compIt = this->Compilers.find(variable);

  if (compIt != this->Compilers.end()) {
    std::string ret = this->OutputConverter->ConvertToOutputForExisting(
      this->VariableMappings["CMAKE_" + compIt->second + "_COMPILER"]);
    std::string const& compilerArg1 =
      this->VariableMappings["CMAKE_" + compIt->second + "_COMPILER_ARG1"];
    std::string const& compilerTarget =
      this->VariableMappings["CMAKE_" + compIt->second + "_COMPILER_TARGET"];
    std::string const& compilerOptionTarget =
      this->VariableMappings["CMAKE_" + compIt->second +
                             "_COMPILE_OPTIONS_TARGET"];
    std::string const& compilerExternalToolchain =
      this->VariableMappings["CMAKE_" + compIt->second +
                             "_COMPILER_EXTERNAL_TOOLCHAIN"];
    std::string const& compilerOptionExternalToolchain =
      this->VariableMappings["CMAKE_" + compIt->second +
                             "_COMPILE_OPTIONS_EXTERNAL_TOOLCHAIN"];
    std::string const& compilerOptionSysroot =
      this->VariableMappings["CMAKE_" + compIt->second +
                             "_COMPILE_OPTIONS_SYSROOT"];

    if (compIt->second == this->ReplaceValues->Language &&
        this->ReplaceValues->Launcher) {
      // Add launcher as part of expansion so that it always appears
      // immediately before the command itself, regardless of whether the
      // overall rule template contains other content at the front.
      ret = cmStrCat(this->ReplaceValues->Launcher, " ", ret);
    }

    // if there are required arguments to the compiler add it
    // to the compiler string
    if (!compilerArg1.empty()) {
      ret += " ";
      ret += compilerArg1;
    }
    if (!compilerTarget.empty() && !compilerOptionTarget.empty()) {
      ret += " ";
      ret += compilerOptionTarget;
      ret += compilerTarget;
    }
    if (!compilerExternalToolchain.empty() &&
        !compilerOptionExternalToolchain.empty()) {
      ret += " ";
      ret += compilerOptionExternalToolchain;
      ret +=
        this->OutputConverter->EscapeForShell(compilerExternalToolchain, true);
    }
    std::string sysroot;
    // Some platforms may use separate sysroots for compiling and linking.
    // If we detect link flags, then we pass the link sysroot instead.
    // FIXME: Use a more robust way to detect link line expansion.
    if (this->ReplaceValues->LinkFlags) {
      sysroot = this->LinkerSysroot;
    } else {
      sysroot = this->CompilerSysroot;
    }
    if (!sysroot.empty() && !compilerOptionSysroot.empty()) {
      ret += " ";
      ret += compilerOptionSysroot;
      ret += this->OutputConverter->EscapeForShell(sysroot, true);
    }
    return ret;
  }

  auto mapIt = this->VariableMappings.find(variable);
  if (mapIt != this->VariableMappings.end()) {
    if (variable.find("_FLAG") == std::string::npos) {
      std::string ret =
        this->OutputConverter->ConvertToOutputForExisting(mapIt->second);

      if (this->ReplaceValues->Launcher && variable == "CMAKE_LINKER") {
        // Add launcher as part of expansion so that it always appears
        // immediately before the command itself, regardless of whether the
        // overall rule template contains other content at the front.
        ret = cmStrCat(this->ReplaceValues->Launcher, " ", ret);
      }

      return ret;
    }
    return mapIt->second;
  }
  return variable;
}

void cmRulePlaceholderExpander::ExpandRuleVariables(
  cmOutputConverter* outputConverter, std::string& s,
  const RuleVariables& replaceValues)
{
  this->OutputConverter = outputConverter;
  this->ReplaceValues = &replaceValues;

  this->ExpandVariables(s);
}
