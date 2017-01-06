/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmMSVC60LinkLineComputer_h
#define cmMSVC60LinkLineComputer_h

#include <cmConfigure.h>

#include <string>

#include "cmLinkLineComputer.h"

class cmOutputConverter;
class cmStateDirectory;

class cmMSVC60LinkLineComputer : public cmLinkLineComputer
{
public:
  cmMSVC60LinkLineComputer(cmOutputConverter* outputConverter,
                           cmStateDirectory stateDir);

  std::string ConvertToLinkReference(std::string const& input) const
    CM_OVERRIDE;
};

#endif
