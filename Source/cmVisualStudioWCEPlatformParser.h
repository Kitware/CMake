/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "cmXMLParser.h"

// This class is used to parse XML with configuration
// of installed SDKs in system
class cmVisualStudioWCEPlatformParser : public cmXMLParser
{
public:
  cmVisualStudioWCEPlatformParser(char const* name = nullptr)
    : RequiredName(name)
  {
  }

  int ParseVersion(char const* version);

  bool Found() const { return this->FoundRequiredName; }
  char const* GetArchitectureFamily() const;
  std::string GetOSVersion() const;
  std::string GetIncludeDirectories() const
  {
    return this->FixPaths(this->Include);
  }
  std::string GetLibraryDirectories() const
  {
    return this->FixPaths(this->Library);
  }
  std::string GetPathDirectories() const { return this->FixPaths(this->Path); }
  std::vector<std::string> const& GetAvailablePlatforms() const
  {
    return this->AvailablePlatforms;
  }

protected:
  void StartElement(std::string const& name, char const** attributes) override;
  void EndElement(std::string const& name) override;
  void CharacterDataHandler(char const* data, int length) override;

private:
  std::string FixPaths(std::string const& paths) const;

  std::string CharacterData;

  std::string Include;
  std::string Library;
  std::string Path;
  std::string PlatformName;
  std::string OSMajorVersion;
  std::string OSMinorVersion;
  std::map<std::string, std::string> Macros;
  std::vector<std::string> AvailablePlatforms;

  char const* RequiredName;
  bool FoundRequiredName = false;
  std::string VcInstallDir;
  std::string VsInstallDir;
};
