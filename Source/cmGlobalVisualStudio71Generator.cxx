/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalVisualStudio71Generator.h"

#include <map>
#include <sstream>

#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmake;

cmGlobalVisualStudio71Generator::cmGlobalVisualStudio71Generator(
  cmake* cm, const std::string& platformName)
  : cmGlobalVisualStudio7Generator(cm, platformName)
{
  this->ProjectConfigurationSectionName = "ProjectConfiguration";
}

void cmGlobalVisualStudio71Generator::WriteSLNFile(
  std::ostream& fout, cmLocalGenerator* root,
  std::vector<cmLocalGenerator*>& generators)
{
  std::vector<std::string> configs =
    root->GetMakefile()->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);

  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet orderedProjectTargets(
    projectTargets, this->GetStartupProjectName(root));

  // Generate the targets specification to a string.  We will put this in
  // the actual .sln file later.  As a side effect, this method also
  // populates the set of folders.
  std::ostringstream targetsSlnString;
  this->WriteTargetsToSolution(targetsSlnString, root, orderedProjectTargets);

  // Generate folder specification.
  bool useFolderProperty = this->UseFolderProperty();
  if (useFolderProperty) {
    this->WriteFolders(fout);
  }

  // Now write the actual target specification content.
  fout << targetsSlnString.str();

  // Write out the configurations information for the solution
  fout << "Global\n";
  // Write out the configurations for the solution
  this->WriteSolutionConfigurations(fout, configs);
  fout << "\tGlobalSection(" << this->ProjectConfigurationSectionName
       << ") = postSolution\n";
  // Write out the configurations for all the targets in the project
  this->WriteTargetConfigurations(fout, configs, orderedProjectTargets);
  fout << "\tEndGlobalSection\n";

  if (useFolderProperty) {
    // Write out project folders
    fout << "\tGlobalSection(NestedProjects) = preSolution\n";
    this->WriteFoldersContent(fout);
    fout << "\tEndGlobalSection\n";
  }

  // Write out global sections
  this->WriteSLNGlobalSections(fout, root);

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}

void cmGlobalVisualStudio71Generator::WriteSolutionConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs)
{
  fout << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  for (std::string const& i : configs) {
    fout << "\t\t" << i << " = " << i << "\n";
  }
  fout << "\tEndGlobalSection\n";
}

// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteProject(std::ostream& fout,
                                                   const std::string& dspname,
                                                   const std::string& dir,
                                                   cmGeneratorTarget const* t)
{
  // check to see if this is a fortran build
  std::string ext = ".vcproj";
  const char* project =
    R"(Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = ")";
  if (this->TargetIsFortranOnly(t)) {
    ext = ".vfproj";
    project = R"(Project("{6989167D-11E4-40FE-8C1A-2192A86A7E90}") = ")";
  }
  if (t->IsCSharpOnly()) {
    ext = ".csproj";
    project = R"(Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = ")";
  }
  cmValue targetExt = t->GetProperty("GENERATOR_FILE_NAME_EXT");
  if (targetExt) {
    ext = *targetExt;
  }

  std::string guid = this->GetGUID(dspname);
  fout << project << dspname << "\", \"" << this->ConvertToSolutionPath(dir)
       << (!dir.empty() ? "\\" : "") << dspname << ext << "\", \"{" << guid
       << "}\"\n";
  fout << "\tProjectSection(ProjectDependencies) = postProject\n";
  this->WriteProjectDepends(fout, dspname, dir, t);
  fout << "\tEndProjectSection\n";

  fout << "EndProject\n";

  auto ui = this->UtilityDepends.find(t);
  if (ui != this->UtilityDepends.end()) {
    const char* uname = ui->second.c_str();
    /* clang-format off */
    fout << R"(Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = ")"
         << uname << "\", \""
         << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
         << uname << ".vcproj" << "\", \"{"
         << this->GetGUID(uname) << "}\"\n"
         << "\tProjectSection(ProjectDependencies) = postProject\n"
         << "\t\t{" << guid << "} = {" << guid << "}\n"
         << "\tEndProjectSection\n"
         << "EndProject\n";
    /* clang-format on */
  }
}

// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteProjectDepends(
  std::ostream& fout, const std::string&, const std::string&,
  cmGeneratorTarget const* target)
{
  VSDependSet const& depends = this->VSTargetDepends[target];
  for (std::string const& name : depends) {
    std::string guid = this->GetGUID(name);
    if (guid.empty()) {
      std::string m = cmStrCat("Target: ", target->GetName(),
                               " depends on unknown target: ", name);
      cmSystemTools::Error(m);
    }
    fout << "\t\t{" << guid << "} = {" << guid << "}\n";
  }
}

// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteExternalProject(
  std::ostream& fout, const std::string& name, const std::string& location,
  cmValue typeGuid, const std::set<BT<std::pair<std::string, bool>>>& depends)
{
  fout << "Project(\"{"
       << (typeGuid ? *typeGuid
                    : std::string(
                        cmGlobalVisualStudio71Generator::ExternalProjectType(
                          location)))
       << "}\") = \"" << name << "\", \""
       << this->ConvertToSolutionPath(location) << "\", \"{"
       << this->GetGUID(name) << "}\"\n";

  // write out the dependencies here VS 7.1 includes dependencies with the
  // project instead of in the global section
  if (!depends.empty()) {
    fout << "\tProjectSection(ProjectDependencies) = postProject\n";
    for (BT<std::pair<std::string, bool>> const& it : depends) {
      std::string const& dep = it.Value.first;
      if (this->IsDepInSolution(dep)) {
        fout << "\t\t{" << this->GetGUID(dep) << "} = {" << this->GetGUID(dep)
             << "}\n";
      }
    }
    fout << "\tEndProjectSection\n";
  }

  fout << "EndProject\n";
}

// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteProjectConfigurations(
  std::ostream& fout, const std::string& name, cmGeneratorTarget const& target,
  std::vector<std::string> const& configs,
  const std::set<std::string>& configsPartOfDefaultBuild,
  std::string const& platformMapping)
{
  const std::string& platformName =
    !platformMapping.empty() ? platformMapping : this->GetPlatformName();
  std::string guid = this->GetGUID(name);
  for (std::string const& i : configs) {
    cmList mapConfig;
    const char* dstConfig = i.c_str();
    if (target.GetProperty("EXTERNAL_MSPROJECT")) {
      if (cmValue m = target.GetProperty("MAP_IMPORTED_CONFIG_" +
                                         cmSystemTools::UpperCase(i))) {
        mapConfig.assign(*m);
        if (!mapConfig.empty()) {
          dstConfig = mapConfig[0].c_str();
        }
      }
    }
    fout << "\t\t{" << guid << "}." << i << ".ActiveCfg = " << dstConfig << "|"
         << platformName << std::endl;
    auto ci = configsPartOfDefaultBuild.find(i);
    if (!(ci == configsPartOfDefaultBuild.end())) {
      fout << "\t\t{" << guid << "}." << i << ".Build.0 = " << dstConfig << "|"
           << platformName << std::endl;
    }
  }
}
