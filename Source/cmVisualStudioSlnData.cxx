/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmVisualStudioSlnData.h"

#include <cstddef>
#include <utility>

#include "cmSystemTools.h"

void cmSlnProjectEntry::AddProjectConfiguration(
  const std::string& solutionConfiguration,
  const std::string& projectConfiguration)
{
  projectConfigurationMap[solutionConfiguration] = projectConfiguration;
}

std::string cmSlnProjectEntry::GetProjectConfiguration(
  const std::string& solutionConfiguration)
{
  return projectConfigurationMap[solutionConfiguration];
}

const cm::optional<cmSlnProjectEntry> cmSlnData::GetProjectByGUID(
  const std::string& projectGUID) const
{
  ProjectStorage::const_iterator it(ProjectsByGUID.find(projectGUID));
  if (it != ProjectsByGUID.end())
    return it->second;
  else
    return cm::nullopt;
}

const cm::optional<cmSlnProjectEntry> cmSlnData::GetProjectByName(
  const std::string& projectName) const
{
  ProjectStringIndex::const_iterator it(ProjectNameIndex.find(projectName));
  if (it != ProjectNameIndex.end())
    return it->second->second;
  else
    return cm::nullopt;
}

std::vector<cmSlnProjectEntry> cmSlnData::GetProjects() const
{
  ProjectStringIndex::const_iterator it(this->ProjectNameIndex.begin()),
    itEnd(this->ProjectNameIndex.end());
  std::vector<cmSlnProjectEntry> result;
  for (; it != itEnd; ++it)
    result.push_back(it->second->second);
  return result;
}

cmSlnProjectEntry* cmSlnData::AddProject(
  const std::string& projectGUID, const std::string& projectName,
  const std::string& projectRelativePath)
{
  ProjectStorage::iterator it(ProjectsByGUID.find(projectGUID));
  if (it != ProjectsByGUID.end())
    return NULL;
  it = ProjectsByGUID
         .insert(ProjectStorage::value_type(
           projectGUID,
           cmSlnProjectEntry(projectGUID, projectName, projectRelativePath)))
         .first;
  ProjectNameIndex[projectName] = it;
  return &it->second;
}

std::string cmSlnData::GetConfigurationTarget(
  const std::string& projectName, const std::string& solutionConfiguration,
  const std::string& platformName)
{
  std::string solutionTarget = solutionConfiguration + "|" + platformName;
  cm::optional<cmSlnProjectEntry> project = GetProjectByName(projectName);
  if (!project)
    return platformName;

  std::string projectTarget = project->GetProjectConfiguration(solutionTarget);
  if (projectTarget.empty())
    return platformName;

  std::vector<std::string> targetElements =
    cmSystemTools::SplitString(projectTarget, '|');
  if (targetElements.size() != 2)
    return platformName;

  return targetElements[1];
}
