/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmBinUtilsLinker.h"

#include "cmRuntimeDependencyArchive.h"

cmBinUtilsLinker::cmBinUtilsLinker(cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}

void cmBinUtilsLinker::SetError(const std::string& e)
{
  this->Archive->SetError(e);
}
