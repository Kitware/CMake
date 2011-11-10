/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio71Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudio71Generator::cmGlobalVisualStudio71Generator()
{
  this->FindMakeProgramFile = "CMakeVS71FindMake.cmake";
  this->ProjectConfigurationSectionName = "ProjectConfiguration";
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio71Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg =
    new cmLocalVisualStudio7Generator(cmLocalVisualStudioGenerator::VS71);
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS7());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio71Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  this->cmGlobalVisualStudio7Generator::AddPlatformDefinitions(mf);
  mf->RemoveDefinition("MSVC70");
  mf->AddDefinition("MSVC71", "1");
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio71Generator::GetUserMacrosDirectory()
{
  // Macros not supported on Visual Studio 7.1 and earlier because
  // they do not appear to work *during* a build when called by an
  // outside agent...
  //
  return "";

#if 0
  //
  // The COM result from calling a Visual Studio macro with 7.1 indicates
  // that the call succeeds, but the macro does not appear to execute...
  //
  // So, I am leaving this code here to show how to do it, but have not
  // yet figured out what the issue is in terms of why the macro does not
  // appear to execute...
  //
  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
    "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\7.1;"
    "VisualStudioProjectsLocation",
    base))
    {
    cmSystemTools::ConvertToUnixSlashes(base);

    // 7.1 macros folder:
    path = base + "/VSMacros71";
    }

  // path is (correctly) still empty if we did not read the base value from
  // the Registry value
  return path;
#endif
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio71Generator::GetUserMacrosRegKeyBase()
{
  // Macros not supported on Visual Studio 7.1 and earlier because
  // they do not appear to work *during* a build when called by an
  // outside agent...
  //
  return "";

#if 0
  return "Software\\Microsoft\\VisualStudio\\7.1\\vsmacros";
#endif
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio71Generator
::WriteSLNFile(std::ostream& fout,
               cmLocalGenerator* root,
               std::vector<cmLocalGenerator*>& generators)
{ 
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet orderedProjectTargets(projectTargets);

  this->WriteTargetsToSolution(fout, root, orderedProjectTargets);

  bool useFolderProperty = this->UseFolderProperty();
  if (useFolderProperty)
    {
    this->WriteFolders(fout);
    }

  // Write out the configurations information for the solution
  fout << "Global\n";
  // Write out the configurations for the solution
  this->WriteSolutionConfigurations(fout);
  fout << "\tGlobalSection(" << this->ProjectConfigurationSectionName
       << ") = postSolution\n";
  // Write out the configurations for all the targets in the project
  this->WriteTargetConfigurations(fout, root, orderedProjectTargets);
  fout << "\tEndGlobalSection\n";

  if (useFolderProperty)
    {
    // Write out project folders
    fout << "\tGlobalSection(NestedProjects) = preSolution\n";
    this->WriteFoldersContent(fout);
    fout << "\tEndGlobalSection\n";
    }

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio71Generator
::WriteSolutionConfigurations(std::ostream& fout)
{
  fout << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t" << *i << " = " << *i << "\n";
    }
  fout << "\tEndGlobalSection\n";
}

//----------------------------------------------------------------------------
// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void
cmGlobalVisualStudio71Generator::WriteProject(std::ostream& fout,
                                              const char* dspname,
                                              const char* dir,
                                              cmTarget& t)
{
  // check to see if this is a fortran build
  const char* ext = ".vcproj";
  const char* project = 
    "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"";
  if(this->TargetIsFortranOnly(t))
    {
    ext = ".vfproj"; 
    project = "Project(\"{6989167D-11E4-40FE-8C1A-2192A86A7E90}\") = \"";
    }
  const char* targetExt = t.GetProperty("GENERATOR_FILE_NAME_EXT");
  if(targetExt)
    {
    ext = targetExt;
    }

  std::string guid = this->GetGUID(dspname);
  fout << project
       << dspname << "\", \""
       << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
       << dspname << ext << "\", \"{" << guid << "}\"\n";
  fout << "\tProjectSection(ProjectDependencies) = postProject\n";
  this->WriteProjectDepends(fout, dspname, dir, t);
  fout << "\tEndProjectSection\n";
  
  fout <<"EndProject\n";

  UtilityDependsMap::iterator ui = this->UtilityDepends.find(&t);
  if(ui != this->UtilityDepends.end())
    {
    const char* uname = ui->second.c_str();
    fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \""
         << uname << "\", \""
         << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
         << uname << ".vcproj" << "\", \"{"
         << this->GetGUID(uname) << "}\"\n"
         << "\tProjectSection(ProjectDependencies) = postProject\n"
         << "\t\t{" << guid << "} = {" << guid << "}\n"
         << "\tEndProjectSection\n"
         << "EndProject\n";
    }
}

//----------------------------------------------------------------------------
// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void
cmGlobalVisualStudio71Generator
::WriteProjectDepends(std::ostream& fout,
                      const char*,
                      const char*, cmTarget& target)
{
  VSDependSet const& depends = this->VSTargetDepends[&target];
  for(VSDependSet::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    const char* name = di->c_str();
    std::string guid = this->GetGUID(name);
    if(guid.size() == 0)
      {
      std::string m = "Target: ";
      m += target.GetName();
      m += " depends on unknown target: ";
      m += name;
      cmSystemTools::Error(m.c_str());
      }
    fout << "\t\t{" << guid << "} = {" << guid << "}\n";
    }
}

//----------------------------------------------------------------------------
// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio71Generator
::WriteExternalProject(std::ostream& fout, 
                       const char* name,
                       const char* location,
                       const std::set<cmStdString>& depends)
{ 
  fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" 
       << name << "\", \""
       << this->ConvertToSolutionPath(location) << "\", \"{"
       << this->GetGUID(name)
       << "}\"\n";
  
  // write out the dependencies here VS 7.1 includes dependencies with the
  // project instead of in the global section
  if(!depends.empty())
    {
    fout << "\tProjectSection(ProjectDependencies) = postProject\n";
    std::set<cmStdString>::const_iterator it;
    for(it = depends.begin(); it != depends.end(); ++it)
      {
      if(it->size() > 0)
        {
        fout << "\t\t{" 
             << this->GetGUID(it->c_str()) 
             << "} = {" 
             << this->GetGUID(it->c_str()) 
             << "}\n";
        }
      }
    fout << "\tEndProjectSection\n";
    }  

  fout << "EndProject\n";
  

}

//----------------------------------------------------------------------------
// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio71Generator
::WriteProjectConfigurations(std::ostream& fout, const char* name,
                             bool partOfDefaultBuild)
{
  std::string guid = this->GetGUID(name);
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i 
         << ".ActiveCfg = " << *i << "|Win32\n";
    if(partOfDefaultBuild)
      {
      fout << "\t\t{" << guid << "}." << *i
           << ".Build.0 = " << *i << "|Win32\n";
      }
    }
}

//----------------------------------------------------------------------------
// Standard end of dsw file
void cmGlobalVisualStudio71Generator::WriteSLNFooter(std::ostream& fout)
{
  fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "EndGlobal\n";
}

//----------------------------------------------------------------------------
// ouput standard header for dsw file
void cmGlobalVisualStudio71Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 8.00\n";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio71Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio .NET 2003 project files.";
  entry.Full = "";
}
