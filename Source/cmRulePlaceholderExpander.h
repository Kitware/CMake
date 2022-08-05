/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

#include "cmPlaceholderExpander.h"

class cmOutputConverter;

class cmRulePlaceholderExpander : public cmPlaceholderExpander
{
public:
  cmRulePlaceholderExpander(
    std::map<std::string, std::string> compilers,
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
    const char* CMTargetName = nullptr;
    const char* CMTargetType = nullptr;
    const char* TargetPDB = nullptr;
    const char* TargetCompilePDB = nullptr;
    const char* TargetVersionMajor = nullptr;
    const char* TargetVersionMinor = nullptr;
    const char* Language = nullptr;
    const char* AIXExports = nullptr;
    const char* Objects = nullptr;
    const char* Target = nullptr;
    const char* LinkLibraries = nullptr;
    const char* Source = nullptr;
    const char* AssemblySource = nullptr;
    const char* PreprocessedSource = nullptr;
    const char* DynDepFile = nullptr;
    const char* Output = nullptr;
    const char* Object = nullptr;
    const char* ObjectDir = nullptr;
    const char* ObjectFileDir = nullptr;
    const char* Flags = nullptr;
    const char* ObjectsQuoted = nullptr;
    const char* SONameFlag = nullptr;
    const char* TargetSOName = nullptr;
    const char* TargetInstallNameDir = nullptr;
    const char* LinkFlags = nullptr;
    const char* Manifests = nullptr;
    const char* LanguageCompileFlags = nullptr;
    const char* Defines = nullptr;
    const char* Includes = nullptr;
    const char* DependencyFile = nullptr;
    const char* DependencyTarget = nullptr;
    const char* FilterPrefix = nullptr;
    const char* SwiftLibraryName = nullptr;
    const char* SwiftModule = nullptr;
    const char* SwiftModuleName = nullptr;
    const char* SwiftOutputFileMap = nullptr;
    const char* SwiftSources = nullptr;
    const char* ISPCHeader = nullptr;
    const char* CudaCompileMode = nullptr;
    const char* Fatbinary = nullptr;
    const char* RegisterFile = nullptr;
    const char* Launcher = nullptr;
  };

  // Expand rule variables in CMake of the type found in language rules
  void ExpandRuleVariables(cmOutputConverter* outputConverter,
                           std::string& string,
                           const RuleVariables& replaceValues);

private:
  std::string ExpandVariable(std::string const& variable) override;

  std::string TargetImpLib;

  std::map<std::string, std::string> Compilers;
  std::map<std::string, std::string> VariableMappings;
  std::string CompilerSysroot;
  std::string LinkerSysroot;

  cmOutputConverter* OutputConverter = nullptr;
  RuleVariables const* ReplaceValues = nullptr;
};
