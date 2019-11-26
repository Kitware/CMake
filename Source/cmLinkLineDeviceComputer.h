/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmLinkLineDeviceComputer_h
#define cmLinkLineDeviceComputer_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmLinkLineComputer.h"

class cmComputeLinkInformation;
class cmGeneratorTarget;
class cmLocalGenerator;
class cmOutputConverter;
class cmStateDirectory;
template <typename T>
class BT;

class cmLinkLineDeviceComputer : public cmLinkLineComputer
{
public:
  cmLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                           cmStateDirectory const& stateDir);
  ~cmLinkLineDeviceComputer() override;

  cmLinkLineDeviceComputer(cmLinkLineDeviceComputer const&) = delete;
  cmLinkLineDeviceComputer& operator=(cmLinkLineDeviceComputer const&) =
    delete;

  bool ComputeRequiresDeviceLinking(cmComputeLinkInformation& cli);

  void ComputeLinkLibraries(
    cmComputeLinkInformation& cli, std::string const& stdLibString,
    std::vector<BT<std::string>>& linkLibraries) override;

  std::string GetLinkerLanguage(cmGeneratorTarget* target,
                                std::string const& config) override;
};

bool requireDeviceLinking(cmGeneratorTarget& target, cmLocalGenerator& lg,
                          const std::string& config);

#endif
