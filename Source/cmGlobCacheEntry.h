/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

struct cmGlobCacheEntry
{
  const bool Recurse;
  const bool ListDirectories;
  const bool FollowSymlinks;
  const std::string Relative;
  const std::string Expression;
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
