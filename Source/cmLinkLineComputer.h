/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#ifndef cmLinkLineComputer_h
#define cmLinkLineComputer_h

#include "cmState.h"

class cmLinkLineComputer
{
public:
  cmLinkLineComputer(cmState::Directory stateDir);
  virtual ~cmLinkLineComputer();

  virtual std::string ConvertToLinkReference(std::string const& input) const;

private:
  cmState::Directory StateDir;
};

#endif
