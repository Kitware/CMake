/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmValue.h"

class cmGeneratorTarget;
class cmGlobalCommonGenerator;
class cmLocalCommonGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmCommonTargetGenerator
 * \brief Common infrastructure for Makefile and Ninja per-target generators
 */
class cmCommonTargetGenerator
{
public:
  cmCommonTargetGenerator(cmGeneratorTarget* gt);
  virtual ~cmCommonTargetGenerator();

  std::vector<std::string> const& GetConfigNames() const;

protected:
  // Feature query methods.
  cmValue GetFeature(std::string const& feature, std::string const& config);

  cmGeneratorTarget* GeneratorTarget;
  cmMakefile* Makefile;
  cmLocalCommonGenerator* LocalCommonGenerator;
  cmGlobalCommonGenerator* GlobalCommonGenerator;
  std::vector<std::string> ConfigNames;
  bool UseLWYU = false;

  void AppendFortranFormatFlags(std::string& flags,
                                cmSourceFile const& source);

  enum class PreprocessFlagsRequired
  {
    YES,
    NO
  };
  void AppendFortranPreprocessFlags(
    std::string& flags, cmSourceFile const& source,
    PreprocessFlagsRequired requires_pp = PreprocessFlagsRequired::YES);

  virtual void AddIncludeFlags(std::string& flags, std::string const& lang,
                               std::string const& config) = 0;
  virtual std::string GetClangTidyReplacementsFilePath(
    std::string const& directory, cmSourceFile const& source,
    std::string const& config) const = 0;

  void AppendOSXVerFlag(std::string& flags, std::string const& lang,
                        char const* name, bool so);

  std::string GetFlags(std::string const& l, std::string const& config,
                       std::string const& arch = std::string());
  std::string GetDefines(std::string const& l, std::string const& config);
  std::string GetIncludes(std::string const& l, std::string const& config);
  std::string GetManifests(std::string const& config);
  std::string GetAIXExports(std::string const& config);
  std::string GenerateCodeCheckRules(
    cmSourceFile const& source, std::string& compilerLauncher,
    std::string const& cmakeCmd, std::string const& config,
    std::function<std::string(std::string const&)> const& pathConverter);

  std::string GetCompilerLauncher(std::string const& lang,
                                  std::string const& config);

  struct LinkedTargetDirs
  {
    std::vector<std::string> Direct;
    std::vector<std::string> Forward;
  };

  LinkedTargetDirs GetLinkedTargetDirectories(std::string const& lang,
                                              std::string const& config) const;
  std::string ComputeTargetCompilePDB(std::string const& config) const;

  std::string GetLinkerLauncher(std::string const& config);

  bool HaveRequiredLanguages(std::vector<cmSourceFile const*> const& sources,
                             std::set<std::string>& languagesNeeded) const;

private:
  using ByLanguageMap = std::map<std::string, std::string>;
  struct ByConfig
  {
    ByLanguageMap FlagsByLanguage;
    ByLanguageMap DefinesByLanguage;
    ByLanguageMap IncludesByLanguage;
  };
  std::map<std::string, ByConfig> Configs;
};
