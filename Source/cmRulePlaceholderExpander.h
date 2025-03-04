/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

#include "cmGeneratorOptions.h"
#include "cmPlaceholderExpander.h"

class cmOutputConverter;

class cmRulePlaceholderExpander : public cmPlaceholderExpander
{
public:
  cmRulePlaceholderExpander(
    cmBuildStep buildStep, std::map<std::string, std::string> compilers,
    std::map<std::string, std::string> variableMappings,
    std::string compilerSysroot, std::string linkerSysroot);

  void SetTargetImpLib(std::string const& targetImpLib)
  {
    this->TargetImpLib = targetImpLib;
  }

  // Create a struct to hold the variables passed into
  // ExpandRuleVariables
  struct RuleVariables
  {
    char const* CMTargetName = nullptr;
    char const* CMTargetType = nullptr;
    char const* CMTargetLabels = nullptr;
    char const* TargetPDB = nullptr;
    char const* TargetCompilePDB = nullptr;
    char const* TargetVersionMajor = nullptr;
    char const* TargetVersionMinor = nullptr;
    char const* Language = nullptr;
    char const* AIXExports = nullptr;
    char const* Objects = nullptr;
    char const* Target = nullptr;
    char const* LinkLibraries = nullptr;
    char const* Source = nullptr;
    char const* AssemblySource = nullptr;
    char const* PreprocessedSource = nullptr;
    char const* DynDepFile = nullptr;
    char const* Output = nullptr;
    char const* Object = nullptr;
    char const* ObjectDir = nullptr;
    char const* ObjectFileDir = nullptr;
    char const* Flags = nullptr;
    char const* ObjectsQuoted = nullptr;
    char const* SONameFlag = nullptr;
    char const* TargetSOName = nullptr;
    char const* TargetInstallNameDir = nullptr;
    char const* Linker = nullptr;
    char const* LinkFlags = nullptr;
    char const* Manifests = nullptr;
    char const* LanguageCompileFlags = nullptr;
    char const* Defines = nullptr;
    char const* Includes = nullptr;
    char const* DependencyFile = nullptr;
    char const* DependencyTarget = nullptr;
    char const* FilterPrefix = nullptr;
    char const* SwiftLibraryName = nullptr;
    char const* SwiftModule = nullptr;
    char const* SwiftModuleName = nullptr;
    char const* SwiftOutputFileMapOption = nullptr;
    char const* SwiftSources = nullptr;
    char const* ISPCHeader = nullptr;
    char const* CudaCompileMode = nullptr;
    char const* Fatbinary = nullptr;
    char const* RegisterFile = nullptr;
    char const* Launcher = nullptr;
    char const* Role = nullptr;
    char const* Config = nullptr;
  };

  // Expand rule variables in CMake of the type found in language rules
  void ExpandRuleVariables(cmOutputConverter* outputConverter,
                           std::string& string,
                           RuleVariables const& replaceValues);

private:
  std::string ExpandVariable(std::string const& variable) override;

  std::string TargetImpLib;

  cmBuildStep BuildStep = cmBuildStep::Compile;
  std::map<std::string, std::string> Compilers;
  std::map<std::string, std::string> VariableMappings;
  std::string CompilerSysroot;
  std::string LinkerSysroot;

  cmOutputConverter* OutputConverter = nullptr;
  RuleVariables const* ReplaceValues = nullptr;
};
