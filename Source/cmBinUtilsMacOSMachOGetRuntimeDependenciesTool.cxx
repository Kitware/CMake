/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmBinUtilsMacOSMachOGetRuntimeDependenciesTool.h"

#include "cmRuntimeDependencyArchive.h"

cmBinUtilsMacOSMachOGetRuntimeDependenciesTool::
  cmBinUtilsMacOSMachOGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}

void cmBinUtilsMacOSMachOGetRuntimeDependenciesTool::SetError(
  std::string const& error)
{
  this->Archive->SetError(error);
}
