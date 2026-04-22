/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmValue.h"

class cmMakefile;

enum class cmGetDirectoryPropertyResult
{
  Success,
  DirectoryNotFound,
};

// Resolve a DIRECTORY scope name to its makefile.  Returns nullptr if
// no such directory has been processed.  An empty dirName returns
// &callerMf (i.e. the current directory's makefile).
cmMakefile* cmResolveDirectoryMakefile(cmMakefile& callerMf,
                                       std::string const& dirName);

cmGetDirectoryPropertyResult cmGetDirectoryProperty(
  std::string const& dirName, std::string const& propertyName,
  cmMakefile& callerMf, cmValue& propertyValue);

// Overload for callers that have already resolved the target directory.
cmGetDirectoryPropertyResult cmGetDirectoryProperty(
  cmMakefile& mf, std::string const& propertyName, cmValue& propertyValue);
