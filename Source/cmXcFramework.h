/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include <cm/optional>

#include "cmListFileCache.h"

class cmMakefile;

enum class cmXcFrameworkPlistSupportedPlatform
{
  macOS,
  iOS,
  tvOS,
  watchOS,
  visionOS,
};

enum class cmXcFrameworkPlistSupportedPlatformVariant
{
  maccatalyst,
  simulator,
};

struct cmXcFrameworkPlistLibrary
{
  std::string LibraryIdentifier;
  std::string LibraryPath;
  std::string HeadersPath;
  std::vector<std::string> SupportedArchitectures;
  cmXcFrameworkPlistSupportedPlatform SupportedPlatform;
  cm::optional<cmXcFrameworkPlistSupportedPlatformVariant>
    SupportedPlatformVariant;
};

struct cmXcFrameworkPlist
{
  std::string Path;
  std::vector<cmXcFrameworkPlistLibrary> AvailableLibraries;

  const cmXcFrameworkPlistLibrary* SelectSuitableLibrary(
    const cmMakefile& mf,
    const cmListFileBacktrace& bt = cmListFileBacktrace{}) const;
};

cm::optional<cmXcFrameworkPlist> cmParseXcFrameworkPlist(
  const std::string& xcframeworkPath, const cmMakefile& mf,
  const cmListFileBacktrace& bt = cmListFileBacktrace{});
