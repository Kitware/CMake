/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmState.h"

#include "cmake.h"
#include "cmCacheManager.h"

cmState::cmState(cmake* cm)
  : CMakeInstance(cm)
{
}

const char* cmCacheEntryTypes[] =
{ "BOOL",
  "PATH",
  "FILEPATH",
  "STRING",
  "INTERNAL",
  "STATIC",
  "UNINITIALIZED",
  0
};

const char*
cmState::CacheEntryTypeToString(cmState::CacheEntryType type)
{
  if ( type > 6 )
    {
    return cmCacheEntryTypes[6];
    }
  return cmCacheEntryTypes[type];
}

cmState::CacheEntryType
cmState::StringToCacheEntryType(const char* s)
{
  int i = 0;
  while(cmCacheEntryTypes[i])
    {
    if(strcmp(s, cmCacheEntryTypes[i]) == 0)
      {
      return static_cast<cmState::CacheEntryType>(i);
      }
    ++i;
    }
  return STRING;
}

bool cmState::IsCacheEntryType(std::string const& key)
{
  for(int i=0; cmCacheEntryTypes[i]; ++i)
    {
    if(strcmp(key.c_str(), cmCacheEntryTypes[i]) == 0)
      {
      return true;
      }
    }
  return false;
}

std::vector<std::string> cmState::GetCacheEntryKeys() const
{
  std::vector<std::string> definitions;
  definitions.reserve(this->CMakeInstance->GetCacheManager()->GetSize());
  cmCacheManager::CacheIterator cit =
    this->CMakeInstance->GetCacheManager()->GetCacheIterator();
  for ( cit.Begin(); !cit.IsAtEnd(); cit.Next() )
    {
    definitions.push_back(cit.GetName());
    }
  return definitions;
}

const char* cmState::GetCacheEntryValue(std::string const& key) const
{
  cmCacheManager::CacheEntry* e = this->CMakeInstance->GetCacheManager()
             ->GetCacheEntry(key);
  if (!e)
    {
    return 0;
    }
  return e->Value.c_str();
}

const char*
cmState::GetInitializedCacheValue(std::string const& key) const
{
  return this->CMakeInstance->GetCacheManager()->GetInitializedCacheValue(key);
}

cmState::CacheEntryType
cmState::GetCacheEntryType(std::string const& key) const
{
  cmCacheManager::CacheIterator it =
      this->CMakeInstance->GetCacheManager()->GetCacheIterator(key.c_str());
  return it.GetType();
}

void cmState::SetCacheEntryValue(std::string const& key,
                                         std::string const& value)
{
  this->CMakeInstance->GetCacheManager()->SetCacheEntryValue(key, value);
}

void cmState::SetCacheEntryProperty(std::string const& key,
                            std::string const& propertyName,
                            std::string const& value)
{
  cmCacheManager::CacheIterator it =
      this->CMakeInstance->GetCacheManager()->GetCacheIterator(key.c_str());
  it.SetProperty(propertyName, value.c_str());
}

void cmState::SetCacheEntryBoolProperty(std::string const& key,
                            std::string const& propertyName,
                            bool value)
{
  cmCacheManager::CacheIterator it =
      this->CMakeInstance->GetCacheManager()->GetCacheIterator(key.c_str());
  it.SetProperty(propertyName, value);
}

const char* cmState::GetCacheEntryProperty(std::string const& key,
                                              std::string const& propertyName)
{
  cmCacheManager::CacheIterator it = this->CMakeInstance->GetCacheManager()
             ->GetCacheIterator(key.c_str());
  if (!it.PropertyExists(propertyName))
    {
    return 0;
    }
  return it.GetProperty(propertyName);
}

bool cmState::GetCacheEntryPropertyAsBool(std::string const& key,
                                              std::string const& propertyName)
{
  return this->CMakeInstance->GetCacheManager()
             ->GetCacheIterator(key.c_str()).GetPropertyAsBool(propertyName);
}

void cmState::AddCacheEntry(const std::string& key, const char* value,
                                    const char* helpString,
                                    cmState::CacheEntryType type)
{
  this->CMakeInstance->GetCacheManager()->AddCacheEntry(key, value,
                                                        helpString, type);
}

void cmState::RemoveCacheEntry(std::string const& key)
{
  this->CMakeInstance->GetCacheManager()->RemoveCacheEntry(key);
}

void cmState::AppendCacheEntryProperty(const std::string& key,
                                               const std::string& property,
                                               const std::string& value,
                                               bool asString)
{
  this->CMakeInstance->GetCacheManager()
       ->GetCacheIterator(key.c_str()).AppendProperty(property,
                                                       value.c_str(),
                                                       asString);
}

void cmState::RemoveCacheEntryProperty(std::string const& key,
                                              std::string const& propertyName)
{
  this->CMakeInstance->GetCacheManager()
       ->GetCacheIterator(key.c_str()).SetProperty(propertyName, (void*)0);
}

void cmState::Initialize()
{
  this->PropertyDefinitions.clear();
  this->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::DIRECTORY,
     "", "", true);
  this->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::DIRECTORY,
     "", "", true);
  this->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::DIRECTORY,
     "", "", true);

  this->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::TARGET,
     "", "", true);
  this->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::TARGET,
     "", "", true);
  this->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::TARGET,
     "", "", true);
}

void cmState::DefineProperty(const std::string& name,
                           cmProperty::ScopeType scope,
                           const char *ShortDescription,
                           const char *FullDescription,
                           bool chained)
{
  this->PropertyDefinitions[scope].DefineProperty(name,scope,ShortDescription,
                                                  FullDescription,
                                                  chained);
}

cmPropertyDefinition *cmState
::GetPropertyDefinition(const std::string& name,
                        cmProperty::ScopeType scope)
{
  if (this->IsPropertyDefined(name,scope))
    {
    return &(this->PropertyDefinitions[scope][name]);
    }
  return 0;
}

bool cmState::IsPropertyDefined(const std::string& name,
                              cmProperty::ScopeType scope)
{
  return this->PropertyDefinitions[scope].IsPropertyDefined(name);
}

bool cmState::IsPropertyChained(const std::string& name,
                              cmProperty::ScopeType scope)
{
  return this->PropertyDefinitions[scope].IsPropertyChained(name);
}
