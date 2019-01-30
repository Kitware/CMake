/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmNinjaLinkLineComputer_h
#define cmNinjaLinkLineComputer_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLinkLineComputer.h"

class cmGlobalNinjaGenerator;
class cmOutputConverter;
class cmStateDirectory;

class cmNinjaLinkLineComputer : public cmLinkLineComputer
{
public:
  cmNinjaLinkLineComputer(cmOutputConverter* outputConverter,
                          cmStateDirectory const& stateDir,
                          cmGlobalNinjaGenerator const* gg);

  cmNinjaLinkLineComputer(cmNinjaLinkLineComputer const&) = delete;
  cmNinjaLinkLineComputer& operator=(cmNinjaLinkLineComputer const&) = delete;

  std::string ConvertToLinkReference(std::string const& input) const override;

private:
  cmGlobalNinjaGenerator const* GG;
};

#endif
