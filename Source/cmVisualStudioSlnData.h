/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include <cm/optional>

class cmSlnProjectEntry
{
public:
  cmSlnProjectEntry() {}
  cmSlnProjectEntry(const std::string& guid, const std::string& name,
                    const std::string& relativePath)
    : Guid(guid)
    , Name(name)
    , RelativePath(relativePath)
  {
  }

  std::string GetGUID() const { return Guid; }
  std::string GetName() const { return Name; }
  std::string GetRelativePath() const { return RelativePath; }

  void AddProjectConfiguration(const std::string& solutionConfiguration,
                               const std::string& projectConfiguration);

  std::string GetProjectConfiguration(
    const std::string& solutionConfiguration);

private:
  std::string Guid, Name, RelativePath;
  std::map<std::string, std::string> projectConfigurationMap;
};

class cmSlnData
{
public:
  std::string GetVisualStudioVersion() const { return visualStudioVersion; }
  void SetVisualStudioVersion(const std::string& version)
  {
    visualStudioVersion = version;
  }

  std::string GetMinimumVisualStudioVersion() const
  {
    return minimumVisualStudioVersion;
  }

  void SetMinimumVisualStudioVersion(const std::string& version)
  {
    minimumVisualStudioVersion = version;
  }

  const cm::optional<cmSlnProjectEntry> GetProjectByGUID(
    const std::string& projectGUID) const;

  const cm::optional<cmSlnProjectEntry> GetProjectByName(
    const std::string& projectName) const;

  std::vector<cmSlnProjectEntry> GetProjects() const;

  cmSlnProjectEntry* AddProject(const std::string& projectGUID,
                                const std::string& projectName,
                                const std::string& projectRelativePath);

  void AddConfiguration(const std::string& configuration)
  {
    solutionConfigurations.push_back(configuration);
  }

  std::string GetConfigurationTarget(const std::string& projectName,
                                     const std::string& solutionConfiguration,
                                     const std::string& platformName);

private:
  std::string visualStudioVersion, minimumVisualStudioVersion;
  using ProjectStorage = std::map<std::string, cmSlnProjectEntry>;
  ProjectStorage ProjectsByGUID;
  using ProjectStringIndex = std::map<std::string, ProjectStorage::iterator>;
  ProjectStringIndex ProjectNameIndex;
  std::vector<std::string> solutionConfigurations;
};
