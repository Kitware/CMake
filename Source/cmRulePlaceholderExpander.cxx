/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmRulePlaceholderExpander.h"

#include <cctype>
#include <utility>

#include "cmOutputConverter.h"
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

std::string cmRulePlaceholderExpander::ExpandRuleVariable(
  cmOutputConverter* outputConverter, std::string const& variable,
  const RuleVariables& replaceValues)
{
  if (replaceValues.LinkFlags) {
    if (variable == "LINK_FLAGS") {
      return replaceValues.LinkFlags;
    }
  }
  if (replaceValues.Manifests) {
    if (variable == "MANIFESTS") {
      return replaceValues.Manifests;
    }
  }
  if (replaceValues.Flags) {
    if (variable == "FLAGS") {
      return replaceValues.Flags;
    }
  }

  if (replaceValues.Source) {
    if (variable == "SOURCE") {
      return replaceValues.Source;
    }
  }
  if (replaceValues.DynDepFile) {
    if (variable == "DYNDEP_FILE") {
      return replaceValues.DynDepFile;
    }
  }
  if (replaceValues.PreprocessedSource) {
    if (variable == "PREPROCESSED_SOURCE") {
      return replaceValues.PreprocessedSource;
    }
  }
  if (replaceValues.AssemblySource) {
    if (variable == "ASSEMBLY_SOURCE") {
      return replaceValues.AssemblySource;
    }
  }
  if (replaceValues.Object) {
    if (variable == "OBJECT") {
      return replaceValues.Object;
    }
  }
  if (replaceValues.ObjectDir) {
    if (variable == "OBJECT_DIR") {
      return replaceValues.ObjectDir;
    }
  }
  if (replaceValues.ObjectFileDir) {
    if (variable == "OBJECT_FILE_DIR") {
      return replaceValues.ObjectFileDir;
    }
  }
  if (replaceValues.Objects) {
    if (variable == "OBJECTS") {
      return replaceValues.Objects;
    }
  }
  if (replaceValues.ObjectsQuoted) {
    if (variable == "OBJECTS_QUOTED") {
      return replaceValues.ObjectsQuoted;
    }
  }
  if (replaceValues.AIXExports) {
    if (variable == "AIX_EXPORTS") {
      return replaceValues.AIXExports;
    }
  }
  if (replaceValues.ISPCHeader) {
    if (variable == "ISPC_HEADER") {
      return replaceValues.ISPCHeader;
    }
  }
  if (replaceValues.Defines && variable == "DEFINES") {
    return replaceValues.Defines;
  }
  if (replaceValues.Includes && variable == "INCLUDES") {
    return replaceValues.Includes;
  }
  if (replaceValues.SwiftLibraryName) {
    if (variable == "SWIFT_LIBRARY_NAME") {
      return replaceValues.SwiftLibraryName;
    }
  }
  if (replaceValues.SwiftModule) {
    if (variable == "SWIFT_MODULE") {
      return replaceValues.SwiftModule;
    }
  }
  if (replaceValues.SwiftModuleName) {
    if (variable == "SWIFT_MODULE_NAME") {
      return replaceValues.SwiftModuleName;
    }
  }
  if (replaceValues.SwiftOutputFileMap) {
    if (variable == "SWIFT_OUTPUT_FILE_MAP") {
      return replaceValues.SwiftOutputFileMap;
    }
  }
  if (replaceValues.SwiftSources) {
    if (variable == "SWIFT_SOURCES") {
      return replaceValues.SwiftSources;
    }
  }
  if (replaceValues.TargetPDB) {
    if (variable == "TARGET_PDB") {
      return replaceValues.TargetPDB;
    }
  }
  if (replaceValues.TargetCompilePDB) {
    if (variable == "TARGET_COMPILE_PDB") {
      return replaceValues.TargetCompilePDB;
    }
  }
  if (replaceValues.DependencyFile) {
    if (variable == "DEP_FILE") {
      return replaceValues.DependencyFile;
    }
  }
  if (replaceValues.DependencyTarget) {
    if (variable == "DEP_TARGET") {
      return replaceValues.DependencyTarget;
    }
  }
  if (replaceValues.Fatbinary) {
    if (variable == "FATBINARY") {
      return replaceValues.Fatbinary;
    }
  }
  if (replaceValues.RegisterFile) {
    if (variable == "REGISTER_FILE") {
      return replaceValues.RegisterFile;
    }
  }

  if (replaceValues.Target) {
    if (variable == "TARGET_QUOTED") {
      std::string targetQuoted = replaceValues.Target;
      if (!targetQuoted.empty() && targetQuoted.front() != '\"') {
        targetQuoted = '\"';
        targetQuoted += replaceValues.Target;
        targetQuoted += '\"';
      }
      return targetQuoted;
    }
    if (variable == "TARGET_UNQUOTED") {
      std::string unquoted = replaceValues.Target;
      std::string::size_type sz = unquoted.size();
      if (sz > 2 && unquoted.front() == '\"' && unquoted.back() == '\"') {
        unquoted = unquoted.substr(1, sz - 2);
      }
      return unquoted;
    }
    if (replaceValues.LanguageCompileFlags) {
      if (variable == "LANGUAGE_COMPILE_FLAGS") {
        return replaceValues.LanguageCompileFlags;
      }
    }
    if (replaceValues.Target) {
      if (variable == "TARGET") {
        return replaceValues.Target;
      }
    }
    if (variable == "TARGET_IMPLIB") {
      return this->TargetImpLib;
    }
    if (variable == "TARGET_VERSION_MAJOR") {
      if (replaceValues.TargetVersionMajor) {
        return replaceValues.TargetVersionMajor;
      }
      return "0";
    }
    if (variable == "TARGET_VERSION_MINOR") {
      if (replaceValues.TargetVersionMinor) {
        return replaceValues.TargetVersionMinor;
      }
      return "0";
    }
    if (replaceValues.Target) {
      if (variable == "TARGET_BASE") {
        // Strip the last extension off the target name.
        std::string targetBase = replaceValues.Target;
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
    if (replaceValues.TargetSOName) {
      if (variable == "TARGET_SONAME") {
        return replaceValues.TargetSOName;
      }
      if (variable == "SONAME_FLAG" && replaceValues.SONameFlag) {
        return replaceValues.SONameFlag;
      }
      if (replaceValues.TargetInstallNameDir &&
          variable == "TARGET_INSTALLNAME_DIR") {
        return replaceValues.TargetInstallNameDir;
      }
    }
    return "";
  }
  if (replaceValues.LinkLibraries) {
    if (variable == "LINK_LIBRARIES") {
      return replaceValues.LinkLibraries;
    }
  }
  if (replaceValues.Language) {
    if (variable == "LANGUAGE") {
      return replaceValues.Language;
    }
  }
  if (replaceValues.CMTargetName) {
    if (variable == "TARGET_NAME") {
      return replaceValues.CMTargetName;
    }
  }
  if (replaceValues.CMTargetType) {
    if (variable == "TARGET_TYPE") {
      return replaceValues.CMTargetType;
    }
  }
  if (replaceValues.Output) {
    if (variable == "OUTPUT") {
      return replaceValues.Output;
    }
  }
  if (variable == "CMAKE_COMMAND") {
    return outputConverter->ConvertToOutputFormat(
      cmSystemTools::GetCMakeCommand(), cmOutputConverter::SHELL);
  }

  auto compIt = this->Compilers.find(variable);

  if (compIt != this->Compilers.end()) {
    std::string ret = outputConverter->ConvertToOutputForExisting(
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
      ret += outputConverter->EscapeForShell(compilerExternalToolchain, true);
    }
    std::string sysroot;
    // Some platforms may use separate sysroots for compiling and linking.
    // If we detect link flags, then we pass the link sysroot instead.
    // FIXME: Use a more robust way to detect link line expansion.
    if (replaceValues.LinkFlags) {
      sysroot = this->LinkerSysroot;
    } else {
      sysroot = this->CompilerSysroot;
    }
    if (!sysroot.empty() && !compilerOptionSysroot.empty()) {
      ret += " ";
      ret += compilerOptionSysroot;
      ret += outputConverter->EscapeForShell(sysroot, true);
    }
    return ret;
  }

  auto mapIt = this->VariableMappings.find(variable);
  if (mapIt != this->VariableMappings.end()) {
    if (variable.find("_FLAG") == std::string::npos) {
      return outputConverter->ConvertToOutputForExisting(mapIt->second);
    }
    return mapIt->second;
  }
  return variable;
}

void cmRulePlaceholderExpander::ExpandRuleVariables(
  cmOutputConverter* outputConverter, std::string& s,
  const RuleVariables& replaceValues)
{
  std::string::size_type start = s.find('<');
  // no variables to expand
  if (start == std::string::npos) {
    return;
  }
  std::string::size_type pos = 0;
  std::string expandedInput;
  while (start != std::string::npos && start < s.size() - 2) {
    std::string::size_type end = s.find('>', start);
    // if we find a < with no > we are done
    if (end == std::string::npos) {
      return;
    }
    char c = s[start + 1];
    // if the next char after the < is not A-Za-z then
    // skip it and try to find the next < in the string
    if (!isalpha(c)) {
      start = s.find('<', start + 1);
    } else {
      // extract the var
      std::string var = s.substr(start + 1, end - start - 1);
      std::string replace =
        this->ExpandRuleVariable(outputConverter, var, replaceValues);
      expandedInput += s.substr(pos, start - pos);

      // Prevent consecutive whitespace in the output if the rule variable
      // expands to an empty string.
      bool consecutive = replace.empty() && start > 0 && s[start - 1] == ' ' &&
        end + 1 < s.size() && s[end + 1] == ' ';
      if (consecutive) {
        expandedInput.pop_back();
      }

      expandedInput += replace;

      // move to next one
      start = s.find('<', start + var.size() + 2);
      pos = end + 1;
    }
  }
  // add the rest of the input
  expandedInput += s.substr(pos, s.size() - pos);
  s = expandedInput;
}
