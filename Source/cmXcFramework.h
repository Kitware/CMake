/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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

  cmXcFrameworkPlistLibrary const* SelectSuitableLibrary(
    cmMakefile const& mf,
    cmListFileBacktrace const& bt = cmListFileBacktrace{}) const;
};

cm::optional<cmXcFrameworkPlist> cmParseXcFrameworkPlist(
  std::string const& xcframeworkPath, cmMakefile const& mf,
  cmListFileBacktrace const& bt = cmListFileBacktrace{});
