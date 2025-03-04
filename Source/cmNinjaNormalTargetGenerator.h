/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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

  void Generate(std::string const& config) override;

private:
  std::string LanguageLinkerRule(std::string const& config) const;
  std::string LanguageLinkerDeviceRule(std::string const& config) const;
  std::string LanguageLinkerCudaDeviceRule(std::string const& config) const;
  std::string LanguageLinkerCudaDeviceCompileRule(
    std::string const& config) const;
  std::string LanguageLinkerCudaFatbinaryRule(std::string const& config) const;
  std::string TextStubsGeneratorRule(std::string const& config) const;
  bool CheckUseResponseFileForLibraries(std::string const& config) const;
  char const* GetVisibleTypeName() const;
  void WriteLanguagesRules(std::string const& config);

  void WriteLinkRule(bool useResponseFile, std::string const& config);
  void WriteDeviceLinkRules(std::string const& config);
  void WriteNvidiaDeviceLinkRule(bool useResponseFile,
                                 std::string const& config);

  void WriteLinkStatement(std::string const& config,
                          std::string const& fileConfig, bool firstForConfig);
  void WriteDeviceLinkStatement(std::string const& config,
                                std::string const& fileConfig,
                                bool firstForConfig);
  void WriteDeviceLinkStatements(std::string const& config,
                                 std::vector<std::string> const& architectures,
                                 std::string const& output);
  void WriteNvidiaDeviceLinkStatement(std::string const& config,
                                      std::string const& fileConfig,
                                      std::string const& outputDir,
                                      std::string const& output);

  void WriteObjectLibStatement(std::string const& config);
  void WriteCxxModuleLibraryStatement(std::string const& config,
                                      std::string const& fileConfig,
                                      bool firstForConfig);

  std::vector<std::string> ComputeLinkCmd(std::string const& config);
  std::vector<std::string> ComputeDeviceLinkCmd();

  // Target name info.
  cmGeneratorTarget::Names TargetNames(std::string const& config) const;
  std::string TargetLinkLanguage(std::string const& config) const;
  std::string DeviceLinkObject;
};
