/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmNinjaLinkLineDeviceComputer.h"

#include <utility>

#include "cmGlobalNinjaGenerator.h"

cmNinjaLinkLineDeviceComputer::cmNinjaLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir,
  cmGlobalNinjaGenerator const* gg, std::string language)
  : cmLinkLineDeviceComputer(outputConverter, stateDir, std::move(language))
  , GG(gg)
{
}

std::string cmNinjaLinkLineDeviceComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  return this->GG->ConvertToNinjaPath(lib);
}
