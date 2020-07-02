/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmStandardLevelResolver_h
#define cmStandardLevelResolver_h

#include <string>

#include "cmProperty.h"

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

  bool AddRequiredTargetFeature(cmTarget* target, const std::string& feature,
                                std::string* error = nullptr) const;

  bool CompileFeatureKnown(const std::string& targetName,
                           const std::string& feature, std::string& lang,
                           std::string* error) const;

  const char* CompileFeaturesAvailable(const std::string& lang,
                                       std::string* error) const;

  bool GetNewRequiredStandard(const std::string& targetName,
                              const std::string& feature,
                              cmProp currentLangStandardValue,
                              std::string& newRequiredStandard,
                              std::string* error = nullptr) const;

  bool HaveStandardAvailable(cmGeneratorTarget const* target,
                             std::string const& lang,
                             std::string const& config,
                             const std::string& feature) const;

  bool IsLaterStandard(std::string const& lang, std::string const& lhs,
                       std::string const& rhs) const;

private:
  bool AddRequiredTargetCFeature(cmTarget* target, const std::string& feature,
                                 std::string const& lang,
                                 std::string* error = nullptr) const;
  bool AddRequiredTargetCxxFeature(cmTarget* target,
                                   const std::string& feature,
                                   std::string const& lang,
                                   std::string* error = nullptr) const;
  bool AddRequiredTargetCudaFeature(cmTarget* target,
                                    const std::string& feature,
                                    std::string const& lang,
                                    std::string* error = nullptr) const;

  bool CheckCompileFeaturesAvailable(const std::string& targetName,
                                     const std::string& feature,
                                     std::string& lang,
                                     std::string* error) const;

  void CheckNeededCLanguage(const std::string& feature,
                            std::string const& lang, bool& needC90,
                            bool& needC99, bool& needC11) const;
  void CheckNeededCxxLanguage(const std::string& feature,
                              std::string const& lang, bool& needCxx98,
                              bool& needCxx11, bool& needCxx14,
                              bool& needCxx17, bool& needCxx20) const;
  void CheckNeededCudaLanguage(const std::string& feature,
                               std::string const& lang, bool& needCuda03,
                               bool& needCuda11, bool& needCuda14,
                               bool& needCuda17, bool& needCuda20) const;

  bool GetNewRequiredCStandard(const std::string& targetName,
                               const std::string& feature,
                               std::string const& lang,
                               cmProp currentLangStandardValue,
                               std::string& newRequiredStandard,
                               std::string* error = nullptr) const;
  bool GetNewRequiredCxxStandard(const std::string& targetName,
                                 const std::string& feature,
                                 std::string const& lang,
                                 cmProp currentLangStandardValue,
                                 std::string& newRequiredStandard,
                                 std::string* error = nullptr) const;
  bool GetNewRequiredCudaStandard(const std::string& targetName,
                                  const std::string& feature,
                                  std::string const& lang,
                                  cmProp currentLangStandardValue,
                                  std::string& newRequiredStandard,
                                  std::string* error = nullptr) const;

  bool HaveCStandardAvailable(cmGeneratorTarget const* target,
                              std::string const& lang,
                              std::string const& config,
                              const std::string& feature) const;
  bool HaveCxxStandardAvailable(cmGeneratorTarget const* target,
                                std::string const& lang,
                                std::string const& config,
                                const std::string& feature) const;
  bool HaveCudaStandardAvailable(cmGeneratorTarget const* target,
                                 std::string const& lang,
                                 std::string const& config,
                                 const std::string& feature) const;

  cmMakefile* Makefile;
};
#endif
