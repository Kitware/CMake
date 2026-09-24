/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLinkLineComputer.h"

class cmComputeLinkInformation;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmOutputConverter;
class cmStateDirectory;

class cmLinkLineDeviceComputer : public cmLinkLineComputer
{
public:
  cmLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                           cmStateDirectory const& stateDir,
                           std::string language = "CUDA");
  ~cmLinkLineDeviceComputer() override;

  cmLinkLineDeviceComputer(cmLinkLineDeviceComputer const&) = delete;
  cmLinkLineDeviceComputer& operator=(cmLinkLineDeviceComputer const&) =
    delete;

  bool ComputeRequiresDeviceLinking(cmComputeLinkInformation& cli);
  bool ComputeRequiresDeviceLinkingIPOFlag(cmComputeLinkInformation& cli);

  void ComputeLinkLibraries(
    cmComputeLinkInformation& cli, std::string const& stdLibString,
    std::vector<BT<std::string>>& linkLibraries) override;

  std::string GetLinkerLanguage(cmGeneratorTarget* target,
                                std::string const& config) override;

private:
  std::string Language;
};

bool requireDeviceLinking(cmGeneratorTarget const& target,
                          cmLocalGenerator& lg, std::string const& config,
                          std::string const& language = "CUDA");

std::string deviceLinkLanguage(cmGeneratorTarget& target, cmLocalGenerator& lg,
                               std::string const& config);
