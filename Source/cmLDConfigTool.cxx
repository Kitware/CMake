/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmLDConfigTool.h"

cmLDConfigTool::cmLDConfigTool(cmRuntimeDependencyArchive* archive)
  : Archive(archive)
{
}
