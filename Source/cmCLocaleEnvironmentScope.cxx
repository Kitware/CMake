/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCLocaleEnvironmentScope.h"

#include <utility>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmCLocaleEnvironmentScope::cmCLocaleEnvironmentScope()
{
  this->SetEnv("LANGUAGE", "");
  this->SetEnv("LC_MESSAGES", "C");

  std::string lcAll = this->GetEnv("LC_ALL");

  if (!lcAll.empty()) {
    this->SetEnv("LC_ALL", "");
    this->SetEnv("LC_CTYPE", lcAll);
  }
}

std::string cmCLocaleEnvironmentScope::GetEnv(std::string const& key)
{
  std::string value;
  cmSystemTools::GetEnv(key, value);
  return value;
}

void cmCLocaleEnvironmentScope::SetEnv(std::string const& key,
                                       std::string const& value)
{
  std::string oldValue = this->GetEnv(key);

  this->EnvironmentBackup.insert(std::make_pair(key, oldValue));

  if (value.empty()) {
    cmSystemTools::UnsetEnv(key.c_str());
  } else {
    cmSystemTools::PutEnv(cmStrCat(key, '=', value));
  }
}

cmCLocaleEnvironmentScope::~cmCLocaleEnvironmentScope()
{
  for (auto const& envb : this->EnvironmentBackup) {
    cmSystemTools::PutEnv(cmStrCat(envb.first, '=', envb.second));
  }
}
