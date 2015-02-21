/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2014-2015 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmWIXShortcut_h
#define cmWIXShortcut_h

#include <string>
#include <map>
#include <set>
#include <vector>

class cmWIXFilesSourceWriter;

struct cmWIXShortcut
{
  std::string label;
  std::string workingDirectoryId;
};

class cmWIXShortcuts
{
public:
  enum Type
  {
    START_MENU,
    DESKTOP
  };

  typedef std::vector<cmWIXShortcut> shortcut_list_t;
  typedef std::map<std::string, shortcut_list_t> shortcut_id_map_t;

  void insert(Type type, std::string const& id, cmWIXShortcut const& shortcut);

  bool empty(Type type) const;

  bool EmitShortcuts(
    Type type,
    std::string const& registryKey,
    std::string const& cpackComponentName,
    cmWIXFilesSourceWriter& fileDefinitions) const;

  void AddShortcutTypes(std::set<Type>& types);

private:
  typedef std::map<Type, shortcut_id_map_t> shortcut_type_map_t;

  shortcut_type_map_t Shortcuts;
  shortcut_id_map_t EmptyIdMap;
};

#endif
