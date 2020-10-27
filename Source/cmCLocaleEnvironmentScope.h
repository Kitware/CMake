/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

class cmCLocaleEnvironmentScope
{
public:
  cmCLocaleEnvironmentScope();
  ~cmCLocaleEnvironmentScope();

  cmCLocaleEnvironmentScope(cmCLocaleEnvironmentScope const&) = delete;
  cmCLocaleEnvironmentScope& operator=(cmCLocaleEnvironmentScope const&) =
    delete;

private:
  std::string GetEnv(std::string const& key);
  void SetEnv(std::string const& key, std::string const& value);

  using backup_map_t = std::map<std::string, std::string>;
  backup_map_t EnvironmentBackup;
};
