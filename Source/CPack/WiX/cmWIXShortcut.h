/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cmInstalledFile.h"

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
    DESKTOP,
    STARTUP
  };

  using shortcut_list_t = std::vector<cmWIXShortcut>;
  using shortcut_id_map_t = std::map<std::string, shortcut_list_t>;

  void insert(Type type, std::string const& id, cmWIXShortcut const& shortcut);

  bool empty(Type type) const;

  bool EmitShortcuts(Type type, std::string const& registryKey,
                     std::string const& cpackComponentName,
                     cmWIXFilesSourceWriter& fileDefinitions) const;

  void AddShortcutTypes(std::set<Type>& types);

  void CreateFromProperties(std::string const& id,
                            std::string const& directoryId,
                            cmInstalledFile const& installedFile);

private:
  using shortcut_type_map_t = std::map<Type, shortcut_id_map_t>;

  void CreateFromProperty(std::string const& propertyName, Type type,
                          std::string const& id,
                          std::string const& directoryId,
                          cmInstalledFile const& installedFile);

  shortcut_type_map_t Shortcuts;
  shortcut_id_map_t EmptyIdMap;
};
