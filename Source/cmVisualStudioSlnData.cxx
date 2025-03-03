/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmVisualStudioSlnData.h"

#include <cstddef>
#include <utility>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

void cmSlnProjectEntry::AddProjectConfiguration(
  std::string const& solutionConfiguration,
  std::string const& projectConfiguration)
{
  projectConfigurationMap[solutionConfiguration] = projectConfiguration;
}

std::string cmSlnProjectEntry::GetProjectConfiguration(
  std::string const& solutionConfiguration)
{
  return projectConfigurationMap[solutionConfiguration];
}

cm::optional<cmSlnProjectEntry> cmSlnData::GetProjectByGUID(
  std::string const& projectGUID) const
{
  auto it(ProjectsByGUID.find(projectGUID));
  if (it != ProjectsByGUID.end()) {
    return it->second;
  }
  return cm::nullopt;
}

cm::optional<cmSlnProjectEntry> cmSlnData::GetProjectByName(
  std::string const& projectName) const
{
  auto it(ProjectNameIndex.find(projectName));
  if (it != ProjectNameIndex.end()) {
    return it->second->second;
  }
  return cm::nullopt;
}

std::vector<cmSlnProjectEntry> cmSlnData::GetProjects() const
{
  auto it(this->ProjectNameIndex.begin());
  auto itEnd(this->ProjectNameIndex.end());
  std::vector<cmSlnProjectEntry> result;
  for (; it != itEnd; ++it) {
    result.push_back(it->second->second);
  }
  return result;
}

cmSlnProjectEntry* cmSlnData::AddProject(
  std::string const& projectGUID, std::string const& projectName,
  std::string const& projectRelativePath)
{
  auto it(ProjectsByGUID.find(projectGUID));
  if (it != ProjectsByGUID.end()) {
    return nullptr;
  }
  it = ProjectsByGUID
         .insert(ProjectStorage::value_type(
           projectGUID,
           cmSlnProjectEntry(projectGUID, projectName, projectRelativePath)))
         .first;
  ProjectNameIndex[projectName] = it;
  return &it->second;
}

std::string cmSlnData::GetConfigurationTarget(
  std::string const& projectName, std::string const& solutionConfiguration,
  std::string const& platformName)
{
  std::string solutionTarget =
    cmStrCat(solutionConfiguration, '|', platformName);
  cm::optional<cmSlnProjectEntry> project = GetProjectByName(projectName);
  if (!project) {
    return platformName;
  }

  std::string projectTarget = project->GetProjectConfiguration(solutionTarget);
  if (projectTarget.empty()) {
    return platformName;
  }

  std::vector<std::string> targetElements =
    cmSystemTools::SplitString(projectTarget, '|');
  if (targetElements.size() != 2) {
    return platformName;
  }

  return targetElements[1];
}
