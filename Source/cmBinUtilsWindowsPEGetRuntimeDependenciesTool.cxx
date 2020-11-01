/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsWindowsPEGetRuntimeDependenciesTool.h"

#include "cmRuntimeDependencyArchive.h"

cmBinUtilsWindowsPEGetRuntimeDependenciesTool::
  cmBinUtilsWindowsPEGetRuntimeDependenciesTool(
    cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}

void cmBinUtilsWindowsPEGetRuntimeDependenciesTool::SetError(
  const std::string& error)
{
  this->Archive->SetError(error);
}
