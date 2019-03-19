/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmLinkLineDeviceComputer_h
#define cmLinkLineDeviceComputer_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLinkLineComputer.h"

class cmComputeLinkInformation;
class cmGeneratorTarget;
class cmGlobalNinjaGenerator;
class cmOutputConverter;
class cmStateDirectory;

class cmLinkLineDeviceComputer : public cmLinkLineComputer
{
public:
  cmLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                           cmStateDirectory const& stateDir);
  ~cmLinkLineDeviceComputer() override;

  cmLinkLineDeviceComputer(cmLinkLineDeviceComputer const&) = delete;
  cmLinkLineDeviceComputer& operator=(cmLinkLineDeviceComputer const&) =
    delete;

  std::string ComputeLinkLibraries(cmComputeLinkInformation& cli,
                                   std::string const& stdLibString) override;

  std::string GetLinkerLanguage(cmGeneratorTarget* target,
                                std::string const& config) override;
};

class cmNinjaLinkLineDeviceComputer : public cmLinkLineDeviceComputer
{
public:
  cmNinjaLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                                cmStateDirectory const& stateDir,
                                cmGlobalNinjaGenerator const* gg);

  cmNinjaLinkLineDeviceComputer(cmNinjaLinkLineDeviceComputer const&) = delete;
  cmNinjaLinkLineDeviceComputer& operator=(
    cmNinjaLinkLineDeviceComputer const&) = delete;

  std::string ConvertToLinkReference(std::string const& input) const override;

private:
  cmGlobalNinjaGenerator const* GG;
};

#endif
