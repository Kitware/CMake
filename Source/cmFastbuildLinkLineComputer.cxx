/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmFastbuildLinkLineComputer.h"

#include <utility>

#include "cmGlobalFastbuildGenerator.h"

class cmOutputConverter;

cmFastbuildLinkLineComputer::cmFastbuildLinkLineComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir,
  cmGlobalFastbuildGenerator const* gg)
  : cmLinkLineComputer(outputConverter, stateDir)
  , GG(gg)
{
}

std::string cmFastbuildLinkLineComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  return this->GG->ConvertToFastbuildPath(lib);
}

cmFastbuildLinkLineDeviceComputer::cmFastbuildLinkLineDeviceComputer(
  cmOutputConverter* outputConverter, cmStateDirectory const& stateDir,
  cmGlobalFastbuildGenerator const* gg, std::string language)
  : cmLinkLineDeviceComputer(outputConverter, stateDir, std::move(language))
  , GG(gg)
{
}

std::string cmFastbuildLinkLineDeviceComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  return this->GG->ConvertToFastbuildPath(lib);
}
