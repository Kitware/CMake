/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmVSSolution.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmXMLWriter.h"

namespace cm {
namespace VS {

cm::string_view const Solution::Project::TypeIdAspNetCore =
  "8BB2217D-0F2D-49D1-97BC-3654ED321F3B"_s;
cm::string_view const Solution::Project::TypeIdCSharp =
  "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC"_s;
cm::string_view const Solution::Project::TypeIdDatabase =
  "C8D11400-126E-41CD-887F-60BD40844F9E"_s;
cm::string_view const Solution::Project::TypeIdDefault =
  "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942"_s;
cm::string_view const Solution::Project::TypeIdDotNetCore =
  "9A19103F-16F7-4668-BE54-9A1E7A4F7556"_s;
cm::string_view const Solution::Project::TypeIdFSharp =
  "F2A71F9B-5D33-465A-A702-920D77279786"_s;
cm::string_view const Solution::Project::TypeIdFortran =
  "6989167D-11E4-40FE-8C1A-2192A86A7E90"_s;
cm::string_view const Solution::Project::TypeIdJScript =
  "262852C6-CD72-467D-83FE-5EEB1973A190"_s;
cm::string_view const Solution::Project::TypeIdMisc =
  "66A2671D-8FB5-11D2-AA7E-00C04F688DDE"_s;
cm::string_view const Solution::Project::TypeIdNodeJS =
  "9092AA53-FB77-4645-B42D-1CCCA6BD08BD"_s;
cm::string_view const Solution::Project::TypeIdPython =
  "888888A0-9F3D-457C-B088-3A5042F75D52"_s;
cm::string_view const Solution::Project::TypeIdSqlSrv =
  "00D1A9C2-B5F0-4AF3-8072-F6C62B433612"_s;
cm::string_view const Solution::Project::TypeIdVDProj =
  "54435603-DBB4-11D2-8724-00A0C9A8B90C"_s;
cm::string_view const Solution::Project::TypeIdVisualBasic =
  "F184B08F-C81C-45F6-A57F-5ABD9991F28F"_s;
cm::string_view const Solution::Project::TypeIdWebSite =
  "E24C65DC-7377-472B-9ABA-BC803B73C61A"_s;
cm::string_view const Solution::Project::TypeIdWinAppPkg =
  "C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5"_s;
cm::string_view const Solution::Project::TypeIdWiX =
  "930C7802-8A8C-48F9-8165-68863BCCD9DD"_s;

cm::string_view const Solution::Folder::TypeId =
  "2150E333-8FDC-42A3-9474-1A3956D46DE8"_s;

std::vector<Solution::Project const*> Solution::GetAllProjects() const
{
  std::vector<Project const*> projects;
  projects.reserve(this->ProjectMap.size());
  for (Project const* project : this->Projects) {
    projects.emplace_back(project);
  }
  for (Folder const* folder : this->Folders) {
    for (Project const* project : folder->Projects) {
      projects.emplace_back(project);
    }
  }
  return projects;
}

namespace {
template <typename T>
T* GetEntry(std::map<cm::string_view, std::unique_ptr<T>>& entryMap,
            cm::string_view name)
{
  auto i = entryMap.find(name);
  if (i == entryMap.end()) {
    auto p = cm::make_unique<T>();
    p->Name = std::string{ name };
    i = entryMap.emplace(p->Name, std::move(p)).first;
  }
  return i->second.get();
}
}

Solution::Folder* Solution::GetFolder(cm::string_view name)
{
  return GetEntry(this->FolderMap, name);
}

Solution::Project* Solution::GetProject(cm::string_view name)
{
  return GetEntry(this->ProjectMap, name);
}

Solution::PropertyGroup* Solution::GetPropertyGroup(cm::string_view name)
{
  return GetEntry(this->PropertyGroupMap, name);
}

namespace {
struct OrderByName
{
  template <typename T>
  bool operator()(T const* l, T const* r) const
  {
    return l->Name < r->Name;
  }
};
}

void Solution::CanonicalizeOrder()
{
  std::sort(this->Folders.begin(), this->Folders.end(), OrderByName());
  for (auto& fi : this->FolderMap) {
    Folder* folder = fi.second.get();
    std::sort(folder->Folders.begin(), folder->Folders.end(), OrderByName());
    std::sort(folder->Projects.begin(), folder->Projects.end(), OrderByName());
  }
  std::sort(this->Projects.begin(), this->Projects.end(), OrderByName());
  for (auto& pi : this->ProjectMap) {
    Project* project = pi.second.get();
    std::sort(project->BuildDependencies.begin(),
              project->BuildDependencies.end(), OrderByName());
  }
}

namespace {

void WriteSlnHeader(std::ostream& sln, Version version, VersionExpress express)
{
  char utf8bom[] = { char(0xEF), char(0xBB), char(0xBF) };
  sln.write(utf8bom, 3);
  sln << '\n';

  switch (version) {
    case Version::VS14:
      // Visual Studio 14 writes .sln format 12.00
      sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      if (express == VersionExpress::Yes) {
        sln << "# Visual Studio Express 14 for Windows Desktop\n";
      } else {
        sln << "# Visual Studio 14\n";
      }
      break;
    case Version::VS15:
      // Visual Studio 15 writes .sln format 12.00
      sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      sln << "# Visual Studio 15\n";
      break;
    case Version::VS16:
      // Visual Studio 16 writes .sln format 12.00
      sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      sln << "# Visual Studio Version 16\n";
      break;
    case Version::VS17:
      // Visual Studio 17 writes .sln format 12.00
      sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      sln << "# Visual Studio Version 17\n";
      break;
    case Version::VS18:
      // Visual Studio 18 writes .sln format 12.00
      sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
      sln << "# Visual Studio Version 18\n";
      break;
  }
}

void WriteSlnProject(std::ostream& sln, Solution::Project const& project)
{
  std::string projectPath = project.Path;
  std::replace(projectPath.begin(), projectPath.end(), '/', '\\');
  sln << "Project(\"{" << project.TypeId << "}\") = \"" << project.Name
      << "\", \"" << projectPath << "\", \"{" << project.Id << "}\"\n";
  sln << "\tProjectSection(ProjectDependencies) = postProject\n";
  for (Solution::Project const* d : project.BuildDependencies) {
    sln << "\t\t{" << d->Id << "} = {" << d->Id << "}\n";
  }
  sln << "\tEndProjectSection\n";
  sln << "EndProject\n";
}

void WriteSlnFolder(std::ostream& sln, Solution::Folder const& folder)
{
  std::string folderName = folder.Name;
  std::replace(folderName.begin(), folderName.end(), '/', '\\');
  std::string const fileName = cmSystemTools::GetFilenameName(folder.Name);
  sln << "Project(\"{" << Solution::Folder::TypeId << "}\") = \"" << fileName
      << "\", \"" << folderName << "\", \"{" << folder.Id << "}\"\n";
  if (!folder.Files.empty()) {
    sln << "\tProjectSection(SolutionItems) = preProject\n";
    for (std::string const& item : folder.Files) {
      sln << "\t\t" << item << " = " << item << "\n";
    }
    sln << "\tEndProjectSection\n";
  }
  sln << "EndProject\n";
}

void WriteSlnSolutionConfigurationPlatforms(std::ostream& sln,
                                            Solution const& solution)
{
  sln << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
  for (std::string const& config : solution.Configs) {
    sln << "\t\t" << config << '|' << solution.Platform << " = " << config
        << '|' << solution.Platform << '\n';
  }
  sln << "\tEndGlobalSection\n";
}

void WriteSlnProjectConfigurationPlatforms(std::ostream& sln,
                                           Solution const& solution,
                                           Solution::Project const& project)
{
  auto const writeStep = [&sln, &solution, &project](std::size_t i,
                                                     cm::string_view step) {
    sln << "\t\t{" << project.Id << "}." << solution.Configs[i] << '|'
        << solution.Platform << "." << step << " = "
        << project.Configs[i].Config << '|' << project.Platform << '\n';
  };
  assert(project.Configs.size() == solution.Configs.size());
  for (std::size_t i = 0; i < solution.Configs.size(); ++i) {
    writeStep(i, "ActiveCfg"_s);
    if (project.Configs[i].Build) {
      writeStep(i, "Build.0"_s);
    }
    if (project.Configs[i].Deploy) {
      writeStep(i, "Deploy.0"_s);
    }
  }
}

void WriteSlnProjectConfigurationPlatforms(
  std::ostream& sln, Solution const& solution,
  std::vector<Solution::Project const*> const& projects)
{
  sln << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";
  for (Solution::Project const* project : projects) {
    WriteSlnProjectConfigurationPlatforms(sln, solution, *project);
  }
  sln << "\tEndGlobalSection\n";
}

void WriteSlnNestedProjects(
  std::ostream& sln, std::vector<Solution::Folder const*> const& folders)
{
  sln << "\tGlobalSection(NestedProjects) = preSolution\n";
  for (Solution::Folder const* folder : folders) {
    for (Solution::Folder const* nestedFolder : folder->Folders) {
      sln << "\t\t{" << nestedFolder->Id << "} = {" << folder->Id << "}\n";
    }
    for (Solution::Project const* project : folder->Projects) {
      sln << "\t\t{" << project->Id << "} = {" << folder->Id << "}\n";
    }
  }
  sln << "\tEndGlobalSection\n";
}

void WriteSlnPropertyGroup(std::ostream& sln,
                           Solution::PropertyGroup const& pg)
{
  cm::string_view const order = pg.Scope == Solution::PropertyGroup::Load::Pre
    ? "preSolution"_s
    : "postSolution"_s;
  sln << "\tGlobalSection(" << pg.Name << ") = " << order << '\n';
  for (auto const& i : pg.Map) {
    sln << "\t\t" << i.first << " = " << i.second << '\n';
  }
  sln << "\tEndGlobalSection\n";
}

}

void WriteSln(std::ostream& sln, Solution const& solution)
{
  assert(solution.VSVersion);
  assert(solution.VSExpress);

  std::vector<Solution::Project const*> projects = solution.GetAllProjects();
  std::sort(projects.begin(), projects.end(),
            [&solution](Solution::Project const* l,
                        Solution::Project const* r) -> bool {
              if (r->Name == solution.StartupProject) {
                return false;
              }
              if (l->Name == solution.StartupProject) {
                return true;
              }
              return l->Name < r->Name;
            });

  WriteSlnHeader(sln, *solution.VSVersion, *solution.VSExpress);
  for (Solution::Folder const* folder : solution.Folders) {
    WriteSlnFolder(sln, *folder);
  }
  for (Solution::Project const* project : projects) {
    WriteSlnProject(sln, *project);
  }
  sln << "Global\n";
  WriteSlnSolutionConfigurationPlatforms(sln, solution);
  WriteSlnProjectConfigurationPlatforms(sln, solution, projects);
  if (!solution.Folders.empty()) {
    WriteSlnNestedProjects(sln, solution.Folders);
  }
  for (Solution::PropertyGroup const* pg : solution.PropertyGroups) {
    WriteSlnPropertyGroup(sln, *pg);
  }
  sln << "EndGlobal\n";
}

namespace {

bool NeedExplicitProjectPlatform(cm::string_view typeId)
{
  // Some projects do not build interactively in the VS IDE unless they
  // have an explicit platform, even if it matches the SLN platform.
  if (typeId == Solution::Project::TypeIdCSharp ||
      typeId == Solution::Project::TypeIdDotNetCore) {
    return true;
  }
  return false;
}

void WriteSlnxSolutionConfigurationPlatforms(cmXMLElement& xmlParent,
                                             Solution const& solution)
{
  cmXMLElement xmlConfigurations(xmlParent, "Configurations");
  for (std::string const& c : solution.Configs) {
    cmXMLElement(xmlConfigurations, "BuildType").Attribute("Name", c);
  }
  cmXMLElement(xmlConfigurations, "Platform")
    .Attribute("Name", solution.Platform);
};

void WriteSlnxProject(cmXMLElement& xmlParent, Solution const& solution,
                      Solution::Project const& project)
{
  cmXMLElement xmlProject(xmlParent, "Project");
  xmlProject.Attribute("Path", project.Path);
  xmlProject.Attribute("Type", cmSystemTools::LowerCase(project.TypeId));
  xmlProject.Attribute("Id", cmSystemTools::LowerCase(project.Id));
  if (project.Name == solution.StartupProject) {
    xmlProject.Attribute("DefaultStartup", "true");
  }
  for (Solution::Project const* d : project.BuildDependencies) {
    cmXMLElement(xmlProject, "BuildDependency").Attribute("Project", d->Path);
  }
  assert(project.Configs.size() == solution.Configs.size());
  for (std::size_t i = 0; i < solution.Configs.size(); ++i) {
    if (project.Configs[i].Config != solution.Configs[i]) {
      cmXMLElement(xmlProject, "BuildType")
        .Attribute("Solution", cmStrCat(solution.Configs[i], "|*"))
        .Attribute("Project", project.Configs[i].Config);
    }
    if (!project.Configs[i].Build) {
      cmXMLElement(xmlProject, "Build")
        .Attribute("Solution", cmStrCat(solution.Configs[i], "|*"))
        .Attribute("Project", "false");
    }
    if (project.Configs[i].Deploy) {
      cmXMLElement(xmlProject, "Deploy")
        .Attribute("Solution", cmStrCat(solution.Configs[i], "|*"));
    }
  }
  if (project.Platform != solution.Platform ||
      NeedExplicitProjectPlatform(project.TypeId)) {
    cmXMLElement(xmlProject, "Platform")
      .Attribute("Project", project.Platform);
  }
};

void WriteSlnxFolder(cmXMLElement& xmlParent, Solution const& solution,
                     Solution::Folder const& folder)
{
  cmXMLElement xmlFolder(xmlParent, "Folder");
  xmlFolder.Attribute("Name", cmStrCat('/', folder.Name, '/'));
  for (std::string const& filePath : folder.Files) {
    cmXMLElement(xmlFolder, "File").Attribute("Path", filePath);
  }
  for (Solution::Project const* project : folder.Projects) {
    WriteSlnxProject(xmlFolder, solution, *project);
  }
};

void WriteSlnxPropertyGroup(cmXMLElement& xmlParent,
                            Solution::PropertyGroup const& pg)
{
  cmXMLElement xmlProperties(xmlParent, "Properties");
  xmlProperties.Attribute("Name", pg.Name);
  if (pg.Scope == Solution::PropertyGroup::Load::Post) {
    xmlProperties.Attribute("Scope", "PostLoad");
  }
  for (auto const& i : pg.Map) {
    cmXMLElement(xmlProperties, "Properties")
      .Attribute("Name", i.first)
      .Attribute("Value", i.second);
  }
}
}

void WriteSlnx(std::ostream& slnx, Solution const& solution)
{
  cmXMLWriter xw(slnx);
  cmXMLDocument xml(xw);
  cmXMLElement xmlSolution(xml, "Solution");
  WriteSlnxSolutionConfigurationPlatforms(xmlSolution, solution);
  for (Solution::Project const* project : solution.Projects) {
    WriteSlnxProject(xmlSolution, solution, *project);
  }
  for (Solution::Folder const* folder : solution.Folders) {
    WriteSlnxFolder(xmlSolution, solution, *folder);
  }
  for (Solution::PropertyGroup const* pg : solution.PropertyGroups) {
    WriteSlnxPropertyGroup(xmlSolution, *pg);
  }
}

}
}
