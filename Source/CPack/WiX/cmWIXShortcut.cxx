/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmWIXShortcut.h"

#include "cmWIXFilesSourceWriter.h"

void cmWIXShortcuts::insert(
  Type type, std::string const& id, cmWIXShortcut const& shortcut)
{
  this->Shortcuts[type][id].push_back(shortcut);
}

bool cmWIXShortcuts::empty(Type type) const
{
  return this->Shortcuts.find(type) == this->Shortcuts.end();
}

bool cmWIXShortcuts::EmitShortcuts(
  Type type,
  std::string const& registryKey,
  std::string const& cpackComponentName,
  cmWIXFilesSourceWriter& fileDefinitions) const
{
  shortcut_type_map_t::const_iterator i = this->Shortcuts.find(type);

  if(i == this->Shortcuts.end())
    {
    return false;
    }

  shortcut_id_map_t const& id_map = i->second;

  std::string shortcutPrefix;
  std::string registrySuffix;

  switch(type)
    {
    case START_MENU:
      shortcutPrefix = "CM_S";
      break;
    case DESKTOP:
      shortcutPrefix = "CM_DS";
      registrySuffix = "_desktop";
      break;
    default:
      return false;
    }

  for(shortcut_id_map_t::const_iterator j = id_map.begin();
    j != id_map.end(); ++j)
    {
    std::string const& id = j->first;
    shortcut_list_t const& shortcutList = j->second;

    for(shortcut_list_t::const_iterator k = shortcutList.begin();
      k != shortcutList.end(); ++k)
      {
      cmWIXShortcut const& shortcut = *k;
      fileDefinitions.EmitShortcut(id, shortcut, shortcutPrefix);
      }
    }

  fileDefinitions.EmitInstallRegistryValue(
    registryKey, cpackComponentName, registrySuffix);

  return true;
}

void cmWIXShortcuts::AddShortcutTypes(std::set<Type>& types)
{
  for(shortcut_type_map_t::const_iterator i = this->Shortcuts.begin();
    i != this->Shortcuts.end(); ++i)
    {
    types.insert(i->first);
    }
}
