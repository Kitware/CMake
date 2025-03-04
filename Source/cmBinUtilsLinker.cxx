/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmBinUtilsLinker.h"

#include "cmRuntimeDependencyArchive.h"

cmBinUtilsLinker::cmBinUtilsLinker(cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}

void cmBinUtilsLinker::SetError(std::string const& e)
{
  this->Archive->SetError(e);
}
