/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmLinkLineDeviceComputer_h
#define cmLinkLineDeviceComputer_h

#include "cmLinkLineComputer.h"
class cmGlobalNinjaGenerator;

class cmLinkLineDeviceComputer : public cmLinkLineComputer
{
public:
  cmLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                           cmStateDirectory stateDir);
  ~cmLinkLineDeviceComputer() CM_OVERRIDE;

  std::string ComputeLinkLibraries(cmComputeLinkInformation& cli,
                                   std::string const& stdLibString)
    CM_OVERRIDE;

  std::string GetLinkerLanguage(cmGeneratorTarget* target,
                                std::string const& config) CM_OVERRIDE;
};

class cmNinjaLinkLineDeviceComputer : public cmLinkLineDeviceComputer
{
public:
  cmNinjaLinkLineDeviceComputer(cmOutputConverter* outputConverter,
                                cmStateDirectory stateDir,
                                cmGlobalNinjaGenerator const* gg);

  std::string ConvertToLinkReference(std::string const& input) const
    CM_OVERRIDE;

private:
  cmGlobalNinjaGenerator const* GG;
};

#endif
