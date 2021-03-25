/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmGeneratorTarget.h"
#include "cmNinjaTargetGenerator.h"

class cmNinjaNormalTargetGenerator : public cmNinjaTargetGenerator
{
public:
  cmNinjaNormalTargetGenerator(cmGeneratorTarget* target);
  ~cmNinjaNormalTargetGenerator() override;

  void Generate(const std::string& config) override;

private:
  std::string LanguageLinkerRule(const std::string& config) const;
  std::string LanguageLinkerDeviceRule(const std::string& config) const;
  std::string LanguageLinkerCudaDeviceRule(const std::string& config) const;
  std::string LanguageLinkerCudaDeviceCompileRule(
    const std::string& config) const;
  std::string LanguageLinkerCudaFatbinaryRule(const std::string& config) const;

  const char* GetVisibleTypeName() const;
  void WriteLanguagesRules(const std::string& config);

  void WriteLinkRule(bool useResponseFile, const std::string& config);
  void WriteDeviceLinkRules(const std::string& config);
  void WriteNvidiaDeviceLinkRule(bool useResponseFile,
                                 const std::string& config);

  void WriteLinkStatement(const std::string& config,
                          const std::string& fileConfig, bool firstForConfig);
  void WriteDeviceLinkStatement(const std::string& config,
                                const std::string& fileConfig,
                                bool firstForConfig);
  void WriteDeviceLinkStatements(const std::string& config,
                                 const std::vector<std::string>& architectures,
                                 const std::string& output);
  void WriteNvidiaDeviceLinkStatement(const std::string& config,
                                      const std::string& fileConfig,
                                      const std::string& outputDir,
                                      const std::string& output);

  void WriteObjectLibStatement(const std::string& config);

  std::vector<std::string> ComputeLinkCmd(const std::string& config);
  std::vector<std::string> ComputeDeviceLinkCmd();

  // Target name info.
  cmGeneratorTarget::Names TargetNames(const std::string& config) const;
  std::string TargetLinkLanguage(const std::string& config) const;
  std::string DeviceLinkObject;
};
