/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLinkLineDeviceComputer.h"

class cmGlobalNinjaGenerator;
class cmOutputConverter;
class cmStateDirectory;

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
