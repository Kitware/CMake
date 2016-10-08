/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLinkLineComputer.h"
#include "cmOutputConverter.h"

cmLinkLineComputer::cmLinkLineComputer(cmState::Directory stateDir)
  : StateDir(stateDir)
{
}

cmLinkLineComputer::~cmLinkLineComputer()
{
}

std::string cmLinkLineComputer::ConvertToLinkReference(
  std::string const& lib) const
{
  std::string relLib = lib;

  if (cmOutputConverter::ContainedInDirectory(
        this->StateDir.GetCurrentBinary(), lib, this->StateDir)) {
    relLib = cmOutputConverter::ForceToRelativePath(
      this->StateDir.GetCurrentBinary(), lib);
  }
  return relLib;
}
