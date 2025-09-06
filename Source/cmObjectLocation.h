/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <utility>

#include <cm/optional>
#include <cm/string_view>

struct cmObjectLocation
{
public:
  cmObjectLocation() = default;
  cmObjectLocation(std::string path)
    : Path(std::move(path))
  {
  }

  void Update(std::string path);

  // Get the path to the object under the common directory for the target's
  // objects.
  std::string const& GetPath() const;
  // Get the directory of the object file under the common directory.
  cm::string_view GetDirectory() const;
  // Get the name of the object file.
  cm::string_view GetName() const;

private:
  std::string Path;
};

struct cmObjectLocations
{
  cmObjectLocations() = default;

  cm::optional<cmObjectLocation> ShortLoc;
  cmObjectLocation LongLoc;
  std::map<std::string, cmObjectLocation> InstallLongLoc;

  enum class UseShortPath
  {
    Yes,
    No,
  };
  cmObjectLocation const& GetLocation(UseShortPath use) const;
  std::string const& GetPath(UseShortPath use) const;

  cmObjectLocation const& GetInstallLocation(UseShortPath use,
                                             std::string const& config) const;
  std::string const& GetInstallPath(UseShortPath use,
                                    std::string const& config) const;
};
