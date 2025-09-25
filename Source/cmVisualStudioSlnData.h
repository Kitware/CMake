/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include <cm/optional>

class cmSlnProjectEntry
{
public:
  cmSlnProjectEntry() = default;
  cmSlnProjectEntry(std::string guid, std::string name,
                    std::string relativePath)
    : Guid(std::move(guid))
    , Name(std::move(name))
    , RelativePath(std::move(relativePath))
  {
  }

  std::string GetGUID() const { return Guid; }
  std::string GetName() const { return Name; }
  std::string GetRelativePath() const { return RelativePath; }

  void AddProjectConfiguration(std::string const& solutionConfiguration,
                               std::string const& projectConfiguration);

  std::string GetProjectConfiguration(
    std::string const& solutionConfiguration);

private:
  std::string Guid, Name, RelativePath;
  std::map<std::string, std::string> projectConfigurationMap;
};

class cmSlnData
{
public:
  std::string GetVisualStudioVersion() const { return visualStudioVersion; }
  void SetVisualStudioVersion(std::string const& version)
  {
    visualStudioVersion = version;
  }

  std::string GetMinimumVisualStudioVersion() const
  {
    return minimumVisualStudioVersion;
  }

  void SetMinimumVisualStudioVersion(std::string const& version)
  {
    minimumVisualStudioVersion = version;
  }

  cm::optional<cmSlnProjectEntry> GetProjectByGUID(
    std::string const& projectGUID) const;

  cm::optional<cmSlnProjectEntry> GetProjectByName(
    std::string const& projectName) const;

  std::vector<cmSlnProjectEntry> GetProjects() const;

  cmSlnProjectEntry* AddProject(std::string const& projectGUID,
                                std::string const& projectName,
                                std::string const& projectRelativePath);

  void AddConfiguration(std::string const& configuration)
  {
    solutionConfigurations.push_back(configuration);
  }

  std::string GetConfigurationTarget(std::string const& projectName,
                                     std::string const& solutionConfiguration,
                                     std::string const& platformName);

private:
  std::string visualStudioVersion, minimumVisualStudioVersion;
  using ProjectStorage = std::map<std::string, cmSlnProjectEntry>;
  ProjectStorage ProjectsByGUID;
  using ProjectStringIndex = std::map<std::string, ProjectStorage::iterator>;
  ProjectStringIndex ProjectNameIndex;
  std::vector<std::string> solutionConfigurations;
};
