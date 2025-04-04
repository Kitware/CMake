/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <unordered_set>

/** \class cmPackageState
 * \brief Information about the state of an imported package.
 */
class cmPackageState
{
public:
  std::unordered_set<std::string> ImportedFiles;
};
