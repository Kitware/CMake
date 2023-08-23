/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmValue.h"

class cmMakefile;
class cmGeneratorTarget;
class cmTarget;

class cmStandardLevelResolver
{

public:
  explicit cmStandardLevelResolver(cmMakefile* makefile)
    : Makefile(makefile)
  {
  }

  std::string GetCompileOptionDef(cmGeneratorTarget const* target,
                                  std::string const& lang,
                                  std::string const& config) const;
  std::string GetEffectiveStandard(cmGeneratorTarget const* target,
                                   std::string const& lang,
                                   std::string const& config) const;

  bool AddRequiredTargetFeature(cmTarget* target, const std::string& feature,
                                std::string* error = nullptr) const;

  bool CompileFeatureKnown(const std::string& targetName,
                           const std::string& feature, std::string& lang,
                           std::string* error) const;

  cmValue CompileFeaturesAvailable(const std::string& lang,
                                   std::string* error) const;

  bool GetNewRequiredStandard(const std::string& targetName,
                              const std::string& feature,
                              cmValue currentLangStandardValue,
                              std::string& newRequiredStandard,
                              std::string* error = nullptr) const;

  bool HaveStandardAvailable(cmGeneratorTarget const* target,
                             std::string const& lang,
                             std::string const& config,
                             const std::string& feature) const;

  bool IsLaterStandard(std::string const& lang, std::string const& lhs,
                       std::string const& rhs) const;

private:
  bool CheckCompileFeaturesAvailable(const std::string& targetName,
                                     const std::string& feature,
                                     std::string& lang,
                                     std::string* error) const;

  cmMakefile* Makefile;
};
