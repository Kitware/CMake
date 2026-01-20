/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>

#include "cmVSVersion.h"

namespace cm {
namespace VS {

/** Represent a Visual Studio Solution.
    In VS terminology, a "project" corresponds to a CMake "target".  */
struct Solution final
{
  Solution() = default;
  Solution(Solution&&) = default;
  Solution& operator=(Solution&&) = default;

  /** Represent how a project behaves under one solution config.  */
  struct ProjectConfig final
  {
    /** Project-specific config corresponding to this solution config.
        This is usually the same as the solution config, but it can map
        to another config in some cases.  */
    std::string Config;

    /** Does the project build under this solution config?  */
    bool Build = false;

    /** Does the project deploy under this solution config?  */
    bool Deploy = false;
  };

  /** Represent one project in a Solution.
      This corresponds to one CMake "target".  */
  struct Project final
  {
    /** Project name.  This corresponds to the CMake "target" name.  */
    std::string Name;

    /** Project GUID.  */
    std::string Id;

    /** Project type GUID.  */
    std::string TypeId;

    /** Path to the project file on disk.
        This is either absolute or relative to the Solution file.  */
    std::string Path;

    /** Project-specific platform.  This is usually the same as the
        Solution::Platform, but it can be different in some cases.  */
    std::string Platform;

    /** Project-specific configuration corresponding to each solution config.
        This vector has the same length as the Solution::Configs vector.  */
    std::vector<ProjectConfig> Configs;

    /** Solution-level dependencies of the project on other projects.  */
    std::vector<Project const*> BuildDependencies;

    // Project type GUIDs used during creation.
    static cm::string_view const TypeIdAspNetCore;
    static cm::string_view const TypeIdCSharp;
    static cm::string_view const TypeIdDatabase;
    static cm::string_view const TypeIdDefault;
    static cm::string_view const TypeIdDotNetCore;
    static cm::string_view const TypeIdFSharp;
    static cm::string_view const TypeIdFortran;
    static cm::string_view const TypeIdJScript;
    static cm::string_view const TypeIdMisc;
    static cm::string_view const TypeIdNodeJS;
    static cm::string_view const TypeIdPython;
    static cm::string_view const TypeIdSqlSrv;
    static cm::string_view const TypeIdVDProj;
    static cm::string_view const TypeIdVisualBasic;
    static cm::string_view const TypeIdWebSite;
    static cm::string_view const TypeIdWinAppPkg;
    static cm::string_view const TypeIdWiX;
  };

  /** Represent one folder in a Solution.  */
  struct Folder final
  {
    /** Canonical folder name.  This includes parent folders separated by
        forward slashes.  */
    std::string Name;

    /** Folder GUID.  */
    std::string Id;

    /** List of folders contained inside this folder.  */
    std::vector<Folder const*> Folders;

    /** List of projects contained inside this folder.  */
    std::vector<Project const*> Projects;

    /** Solution-level files contained inside this folder.  */
    std::set<std::string> Files;

    // Folder type GUID.
    static cm::string_view const TypeId;
  };

  /** Represent a group of solution-level Properties.  */
  struct PropertyGroup final
  {
    enum class Load
    {
      Pre,
      Post,
    };

    /** Properties group name.  */
    std::string Name;

    /** Properties group load behavior.  */
    Load Scope = Load::Post;

    /** Property key-value pairs in the group.  */
    std::map<std::string, std::string> Map;
  };

  /** Visual Studio major version number, if known.  */
  cm::optional<Version> VSVersion;

  /** Whether this is a VS Express edition, if known.  */
  cm::optional<VersionExpress> VSExpress;

  /** Solution-wide target platform.  This is a Windows architecture.  */
  std::string Platform;

  /** Solution-wide build configurations.
      This corresponds to CMAKE_CONFIGURATION_TYPES.  */
  std::vector<std::string> Configs;

  /** List of all folders in the solution.  */
  std::vector<Folder const*> Folders;

  /** List of projects in the solution that are not in folders.  */
  std::vector<Project const*> Projects;

  /** List of solution-level property groups.  */
  std::vector<PropertyGroup const*> PropertyGroups;

  /** Name of the default startup project.  */
  std::string StartupProject;

  /** Get all projects in the solution, including all folders.  */
  std::vector<Project const*> GetAllProjects() const;

  // Non-const methods used during creation.
  Folder* GetFolder(cm::string_view name);
  Project* GetProject(cm::string_view name);
  PropertyGroup* GetPropertyGroup(cm::string_view name);
  void CanonicalizeOrder();

private:
  Solution(Solution const&) = delete;
  Solution& operator=(Solution const&) = delete;

  // Own and index named entities.
  // The string_view keys point at the Name members.
  std::map<cm::string_view, std::unique_ptr<Folder>> FolderMap;
  std::map<cm::string_view, std::unique_ptr<Project>> ProjectMap;
  std::map<cm::string_view, std::unique_ptr<PropertyGroup>> PropertyGroupMap;
};

/** Write the .sln-format representation.  */
void WriteSln(std::ostream& sln, Solution const& solution);

/** Write the .slnx-format representation.  */
void WriteSlnx(std::ostream& slnx, Solution const& solution);

}
}
