/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmState_h
#define cmState_h

#include "cmStandardIncludes.h"
#include "cmPropertyDefinitionMap.h"
#include "cmPropertyMap.h"

class cmake;
class cmCommand;

class cmState
{
public:
  cmState(cmake* cm);
  ~cmState();

  enum CacheEntryType{ BOOL=0, PATH, FILEPATH, STRING, INTERNAL,STATIC,
                       UNINITIALIZED };
  static CacheEntryType StringToCacheEntryType(const char*);
  static const char* CacheEntryTypeToString(CacheEntryType);
  static bool IsCacheEntryType(std::string const& key);

  std::vector<std::string> GetCacheEntryKeys() const;
  const char* GetCacheEntryValue(std::string const& key) const;
  const char* GetInitializedCacheValue(std::string const& key) const;
  CacheEntryType GetCacheEntryType(std::string const& key) const;
  void SetCacheEntryValue(std::string const& key, std::string const& value);
  void SetCacheValue(std::string const& key, std::string const& value);

  void AddCacheEntry(const std::string& key, const char* value,
                     const char* helpString, CacheEntryType type);
  void RemoveCacheEntry(std::string const& key);

  void SetCacheEntryProperty(std::string const& key,
                             std::string const& propertyName,
                             std::string const& value);
  void SetCacheEntryBoolProperty(std::string const& key,
                                 std::string const& propertyName,
                                 bool value);
  const char* GetCacheEntryProperty(std::string const& key,
                                    std::string const& propertyName);
  bool GetCacheEntryPropertyAsBool(std::string const& key,
                                   std::string const& propertyName);
  void AppendCacheEntryProperty(std::string const& key,
                                const std::string& property,
                                const std::string& value,
                                bool asString = false);
  void RemoveCacheEntryProperty(std::string const& key,
                                std::string const& propertyName);

  void Initialize();
  // Define a property
  void DefineProperty(const std::string& name, cmProperty::ScopeType scope,
                      const char *ShortDescription,
                      const char *FullDescription,
                      bool chain = false);

  // get property definition
  cmPropertyDefinition *GetPropertyDefinition
  (const std::string& name, cmProperty::ScopeType scope);

  // Is a property defined?
  bool IsPropertyDefined(const std::string& name, cmProperty::ScopeType scope);
  bool IsPropertyChained(const std::string& name, cmProperty::ScopeType scope);

  void SetLanguageEnabled(std::string const& l);
  bool GetLanguageEnabled(std::string const& l) const;
  std::vector<std::string> GetEnabledLanguages() const;
  void ClearEnabledLanguages();

  bool GetIsInTryCompile() const;
  void SetIsInTryCompile(bool b);

  cmCommand* GetCommand(std::string const& name) const;
  void AddCommand(cmCommand* command);
  void RemoveUnscriptableCommands();
  void RenameCommand(std::string const& oldName, std::string const& newName);
  void RemoveUserDefinedCommands();
  std::vector<std::string> GetCommandNames() const;

  void SetGlobalProperty(const std::string& prop, const char *value);
  void AppendGlobalProperty(const std::string& prop,
                      const char *value,bool asString=false);
  const char *GetGlobalProperty(const std::string& prop);
  bool GetGlobalPropertyAsBool(const std::string& prop);

  const char* GetSourceDirectory() const;
  void SetSourceDirectory(std::string const& sourceDirectory);
  const char* GetBinaryDirectory() const;
  void SetBinaryDirectory(std::string const& binaryDirectory);

private:
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap> PropertyDefinitions;
  std::vector<std::string> EnabledLanguages;
  std::map<std::string, cmCommand*> Commands;
  cmPropertyMap GlobalProperties;
  cmake* CMakeInstance;
  std::string SourceDirectory;
  std::string BinaryDirectory;
  bool IsInTryCompile;
};

#endif
