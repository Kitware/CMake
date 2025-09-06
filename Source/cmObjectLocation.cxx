/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmObjectLocation.h"

void cmObjectLocation::Update(std::string path)
{
  this->Path = std::move(path);
}

std::string const& cmObjectLocation::GetPath() const
{
  return this->Path;
}

cm::string_view cmObjectLocation::GetDirectory() const
{
  auto const pos = this->Path.rfind('/');
  if (pos == std::string::npos) {
    return {};
  }
  return cm::string_view(this->Path.c_str(), pos);
}

cm::string_view cmObjectLocation::GetName() const
{
  auto const pos = this->Path.rfind('/');
  if (pos == std::string::npos) {
    return this->Path;
  }
  auto const nameStart = pos + 1;
  return cm::string_view(this->Path.c_str() + nameStart,
                         this->Path.size() - nameStart);
}

cmObjectLocation const& cmObjectLocations::GetLocation(UseShortPath use) const
{
  if (use == UseShortPath::Yes && this->ShortLoc) {
    return *this->ShortLoc;
  }
  return this->LongLoc;
}

std::string const& cmObjectLocations::GetPath(UseShortPath use) const
{
  return this->GetLocation(use).GetPath();
}

cmObjectLocation const& cmObjectLocations::GetInstallLocation(
  UseShortPath use, std::string const& config) const
{
  if (use == UseShortPath::Yes && this->ShortLoc) {
    return *this->ShortLoc;
  }
  auto it = this->InstallLongLoc.find(config);
  if (it != this->InstallLongLoc.end()) {
    return it->second;
  }
  return this->LongLoc;
}

std::string const& cmObjectLocations::GetInstallPath(
  UseShortPath use, std::string const& config) const
{
  return this->GetInstallLocation(use, config).GetPath();
}
