/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLinkLineComputer.h"

class cmGlobalFastbuildGenerator;
class cmOutputConverter;
class cmStateDirectory;

class cmFastbuildLinkLineComputer : public cmLinkLineComputer
{
public:
  cmFastbuildLinkLineComputer(cmOutputConverter* outputConverter,
                              cmStateDirectory const& stateDir,
                              cmGlobalFastbuildGenerator const* gg);

  cmFastbuildLinkLineComputer(cmFastbuildLinkLineComputer const&) = delete;
  cmFastbuildLinkLineComputer& operator=(cmFastbuildLinkLineComputer const&) =
    delete;

  std::string ConvertToLinkReference(std::string const& input) const override;

private:
  cmGlobalFastbuildGenerator const* GG;
};
