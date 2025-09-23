/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

struct cmGlobCacheEntry
{
  bool const Recurse;
  bool const ListDirectories;
  bool const FollowSymlinks;
  std::string const Relative;
  std::string const Expression;
  std::vector<std::string> Files;

  cmGlobCacheEntry(bool recurse, bool listDirectories, bool followSymlinks,
                   std::string relative, std::string expression,
                   std::vector<std::string> files)
    : Recurse(recurse)
    , ListDirectories(listDirectories)
    , FollowSymlinks(followSymlinks)
    , Relative(std::move(relative))
    , Expression(std::move(expression))
    , Files(std::move(files))
  {
  }
};
