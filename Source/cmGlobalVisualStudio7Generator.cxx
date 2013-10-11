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
#include <assert.h>
#include "cmGlobalVisualStudio7Generator.h"
#include "cmGeneratedFileStream.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalVisualStudio7Generator::cmGlobalVisualStudio7Generator(
  const char* platformName)
{
  this->FindMakeProgramFile = "CMakeVS7FindMake.cmake";

  if (!platformName)
    {
    platformName = "Win32";
    }
  this->PlatformName = platformName;
}


void cmGlobalVisualStudio7Generator
::EnableLanguage(std::vector<std::string>const &  lang,
                 cmMakefile *mf, bool optional)
{
  mf->AddDefinition("CMAKE_GENERATOR_RC", "rc");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->AddPlatformDefinitions(mf);
  if(!mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    mf->AddCacheDefinition(
      "CMAKE_CONFIGURATION_TYPES",
      "Debug;Release;MinSizeRel;RelWithDebInfo",
      "Semicolon separated list of supported configuration types, "
      "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
      "anything else will be ignored.",
      cmCacheManager::STRING);
    }

  // Create list of configurations requested by user's cache, if any.
  this->cmGlobalGenerator::EnableLanguage(lang, mf, optional);
  this->GenerateConfigurations(mf);

  // if this environment variable is set, then copy it to
  // a static cache entry.  It will be used by
  // cmLocalGenerator::ConstructScript, to add an extra PATH
  // to all custom commands.   This is because the VS IDE
  // does not use the environment it is run in, and this allows
  // for running commands and using dll's that the IDE environment
  // does not know about.
  const char* extraPath = cmSystemTools::GetEnv("CMAKE_MSVCIDE_RUN_PATH");
  if(extraPath)
    {
    mf->AddCacheDefinition
      ("CMAKE_MSVCIDE_RUN_PATH", extraPath,
       "Saved environment variable CMAKE_MSVCIDE_RUN_PATH",
       cmCacheManager::STATIC);
    }

}

std::string cmGlobalVisualStudio7Generator
::GenerateBuildCommand(const char* makeProgram,
                       const char *projectName, const char *projectDir,
                       const char* additionalOptions, const char *targetName,
                       const char* config, bool ignoreErrors, bool)
{
  // Visual studio 7 doesn't need project dir
  (void) projectDir;
  // Ingoring errors is not implemented in visual studio 6
  (void) ignoreErrors;

  // now build the test
  std::string makeCommand =
    cmSystemTools::ConvertToOutputPath(makeProgram);
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  // if there are spaces in the makeCommand, assume a full path
  // and convert it to a path with no spaces in it as the
  // RunSingleCommand does not like spaces
#if defined(_WIN32) && !defined(__CYGWIN__)
  if(makeCommand.find(' ') != std::string::npos)
    {
    cmSystemTools::GetShortPath(makeCommand.c_str(), makeCommand);
    }
#endif
  makeCommand += " ";
  makeCommand += projectName;
  makeCommand += ".sln ";
  bool clean = false;
  if ( targetName && strcmp(targetName, "clean") == 0 )
    {
    clean = true;
    targetName = "ALL_BUILD";
    }
  if(clean)
    {
    makeCommand += "/clean ";
    }
  else
    {
    makeCommand += "/build ";
    }

  if(config && strlen(config))
    {
    makeCommand += config;
    }
  else
    {
    makeCommand += "Debug";
    }
  makeCommand += " /project ";

  if (targetName && strlen(targetName))
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  if ( additionalOptions )
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
    }
  return makeCommand;
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio7Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg =
    new cmLocalVisualStudio7Generator(cmLocalVisualStudioGenerator::VS7);
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS7());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio7Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  cmGlobalVisualStudioGenerator::AddPlatformDefinitions(mf);
  mf->AddDefinition("CMAKE_VS_PLATFORM_NAME", this->GetPlatformName());
}

void cmGlobalVisualStudio7Generator::GenerateConfigurations(cmMakefile* mf)
{
  // process the configurations
  const char* ct
    = this->CMakeInstance->GetCacheDefinition("CMAKE_CONFIGURATION_TYPES");
  if ( ct )
    {
    std::vector<std::string> argsOut;
    cmSystemTools::ExpandListArgument(ct, argsOut);
    for(std::vector<std::string>::iterator i = argsOut.begin();
        i != argsOut.end(); ++i)
      {
      if(std::find(this->Configurations.begin(),
                   this->Configurations.end(),
                   *i) == this->Configurations.end())
        {
        this->Configurations.push_back(*i);
        }
      }
    }
  // default to at least Debug and Release
  if(this->Configurations.size() == 0)
    {
    this->Configurations.push_back("Debug");
    this->Configurations.push_back("Release");
    }

  // Reset the entry to have a semi-colon separated list.
  std::string configs = this->Configurations[0];
  for(unsigned int i=1; i < this->Configurations.size(); ++i)
    {
    configs += ";";
    configs += this->Configurations[i];
    }

  mf->AddCacheDefinition(
    "CMAKE_CONFIGURATION_TYPES",
    configs.c_str(),
    "Semicolon separated list of supported configuration types, "
    "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
    "anything else will be ignored.",
    cmCacheManager::STRING);
}

void cmGlobalVisualStudio7Generator::Generate()
{
  // first do the superclass method
  this->cmGlobalVisualStudioGenerator::Generate();

  // Now write out the DSW
  this->OutputSLNFile();
  // If any solution or project files changed during the generation,
  // tell Visual Studio to reload them...
  if(!cmSystemTools::GetErrorOccuredFlag())
    {
    this->CallVisualStudioMacro(MacroReload);
    }
}

void cmGlobalVisualStudio7Generator
::OutputSLNFile(cmLocalGenerator* root,
                std::vector<cmLocalGenerator*>& generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  this->CurrentProject = root->GetMakefile()->GetProjectName();
  std::string fname = root->GetMakefile()->GetStartOutputDirectory();
  fname += "/";
  fname += root->GetMakefile()->GetProjectName();
  fname += ".sln";
  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);
  if(!fout)
    {
    return;
    }
  this->WriteSLNFile(fout, root, generators);
  if (fout.Close())
    {
    this->FileReplacedDuringGenerate(fname);
    }
}

// output the SLN file
void cmGlobalVisualStudio7Generator::OutputSLNFile()
{
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    this->OutputSLNFile(it->second[0], it->second);
    }
}


void cmGlobalVisualStudio7Generator::WriteTargetConfigurations(
  std::ostream& fout,
  cmLocalGenerator* root,
  OrderedTargetDependSet const& projectTargets)
{
  // loop over again and write out configurations for each target
  // in the solution
  for(OrderedTargetDependSet::const_iterator tt =
        projectTargets.begin(); tt != projectTargets.end(); ++tt)
    {
    cmTarget* target = *tt;
    const char* expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if(expath)
      {
      std::set<std::string> allConfigurations(this->Configurations.begin(),
                                              this->Configurations.end());
      this->WriteProjectConfigurations(
        fout, target->GetName(), target->GetType(),
        allConfigurations, target->GetProperty("VS_PLATFORM_MAPPING"));
      }
    else
      {
      const std::set<std::string>& configsPartOfDefaultBuild =
        this->IsPartOfDefaultBuild(root->GetMakefile()->GetProjectName(),
                                   target);
      const char *vcprojName =
        target->GetProperty("GENERATOR_FILE_NAME");
      if (vcprojName)
        {
        this->WriteProjectConfigurations(fout, vcprojName, target->GetType(),
                                         configsPartOfDefaultBuild);
        }
      }
    }
}


void cmGlobalVisualStudio7Generator::WriteTargetsToSolution(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets)
{
  VisualStudioFolders.clear();

  for(OrderedTargetDependSet::const_iterator tt =
        projectTargets.begin(); tt != projectTargets.end(); ++tt)
    {
    cmTarget* target = *tt;
    bool written = false;

    // handle external vc project files
    const char* expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if(expath)
      {
      std::string project = target->GetName();
      std::string location = expath;

      this->WriteExternalProject(fout,
                                 project.c_str(),
                                 location.c_str(),
                                 target->GetProperty("VS_PROJECT_TYPE"),
                                 target->GetUtilities());
      written = true;
      }
    else
      {
      const char *vcprojName =
        target->GetProperty("GENERATOR_FILE_NAME");
      if(vcprojName)
        {
        cmMakefile* tmf = target->GetMakefile();
        std::string dir = tmf->GetStartOutputDirectory();
        dir = root->Convert(dir.c_str(),
                            cmLocalGenerator::START_OUTPUT);
        if(dir == ".")
          {
          dir = ""; // msbuild cannot handle ".\" prefix
          }
        this->WriteProject(fout, vcprojName, dir.c_str(),
                           *target);
        written = true;
        }
      }

    // Create "solution folder" information from FOLDER target property
    //
    if (written && this->UseFolderProperty())
      {
      const char *targetFolder = target->GetProperty("FOLDER");
      if (targetFolder)
        {
        std::vector<cmsys::String> tokens =
          cmSystemTools::SplitString(targetFolder, '/', false);

        std::string cumulativePath = "";

        for(std::vector<cmsys::String>::iterator iter = tokens.begin();
            iter != tokens.end(); ++iter)
          {
          if(!iter->size())
            {
            continue;
            }

          if (cumulativePath.empty())
            {
            cumulativePath = "CMAKE_FOLDER_GUID_" + *iter;
            }
          else
            {
            VisualStudioFolders[cumulativePath].insert(
              cumulativePath + "/" + *iter);

            cumulativePath = cumulativePath + "/" + *iter;
            }

          this->CreateGUID(cumulativePath.c_str());
          }

        if (!cumulativePath.empty())
          {
          VisualStudioFolders[cumulativePath].insert(target->GetName());
          }
        }
      }
    }
}


void cmGlobalVisualStudio7Generator::WriteTargetDepends(
    std::ostream& fout,
    OrderedTargetDependSet const& projectTargets
    )
{
  for(OrderedTargetDependSet::const_iterator tt =
        projectTargets.begin(); tt != projectTargets.end(); ++tt)
    {
    cmTarget* target = *tt;
    if(target->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }
    cmMakefile* mf = target->GetMakefile();
    const char *vcprojName =
      target->GetProperty("GENERATOR_FILE_NAME");
    if (vcprojName)
      {
      std::string dir = mf->GetStartDirectory();
      this->WriteProjectDepends(fout, vcprojName,
                                dir.c_str(), *target);
      }
    }
}

//----------------------------------------------------------------------------
// Write a SLN file to the stream
void cmGlobalVisualStudio7Generator
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
  fout << "Global\n"
       << "\tGlobalSection(SolutionConfiguration) = preSolution\n";

  int c = 0;
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\tConfigName." << c << " = " << *i << "\n";
    c++;
    }
  fout << "\tEndGlobalSection\n";
  // Write out project(target) depends
  fout << "\tGlobalSection(ProjectDependencies) = postSolution\n";
  this->WriteTargetDepends(fout, orderedProjectTargets);
  fout << "\tEndGlobalSection\n";

  if (useFolderProperty)
    {
    // Write out project folders
    fout << "\tGlobalSection(NestedProjects) = preSolution\n";
    this->WriteFoldersContent(fout);
    fout << "\tEndGlobalSection\n";
    }

  // Write out the configurations for all the targets in the project
  fout << "\tGlobalSection(ProjectConfiguration) = postSolution\n";
  this->WriteTargetConfigurations(fout, root, orderedProjectTargets);
  fout << "\tEndGlobalSection\n";

  // Write out global sections
  this->WriteSLNGlobalSections(fout, root);

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio7Generator::WriteFolders(std::ostream& fout)
{
  const char *prefix = "CMAKE_FOLDER_GUID_";
  const std::string::size_type skip_prefix = strlen(prefix);
  std::string guidProjectTypeFolder = "2150E333-8FDC-42A3-9474-1A3956D46DE8";
  for(std::map<std::string,std::set<std::string> >::iterator iter =
    VisualStudioFolders.begin(); iter != VisualStudioFolders.end(); ++iter)
    {
    std::string fullName = iter->first;
    std::string guid = this->GetGUID(fullName.c_str());

    cmSystemTools::ReplaceString(fullName, "/", "\\");
    if (cmSystemTools::StringStartsWith(fullName.c_str(), prefix))
      {
      fullName = fullName.substr(skip_prefix);
      }

    std::string nameOnly = cmSystemTools::GetFilenameName(fullName);

    fout << "Project(\"{" <<
      guidProjectTypeFolder << "}\") = \"" <<
      nameOnly << "\", \"" <<
      fullName << "\", \"{" <<
      guid <<
      "}\"\nEndProject\n";
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio7Generator::WriteFoldersContent(std::ostream& fout)
{
  for(std::map<std::string,std::set<std::string> >::iterator iter =
    VisualStudioFolders.begin(); iter != VisualStudioFolders.end(); ++iter)
    {
    std::string key(iter->first);
    std::string guidParent(this->GetGUID(key.c_str()));

    for(std::set<std::string>::iterator it = iter->second.begin();
        it != iter->second.end(); ++it)
      {
      std::string value(*it);
      std::string guid(this->GetGUID(value.c_str()));

      fout << "\t\t{" << guid << "} = {" << guidParent << "}\n";
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalVisualStudio7Generator::ConvertToSolutionPath(const char* path)
{
  // Convert to backslashes.  Do not use ConvertToOutputPath because
  // we will add quoting ourselves, and we know these projects always
  // use windows slashes.
  std::string d = path;
  std::string::size_type pos = 0;
  while((pos = d.find('/', pos)) != d.npos)
    {
    d[pos++] = '\\';
    }
  return d;
}

// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio7Generator::WriteProject(std::ostream& fout,
                               const char* dspname,
                               const char* dir, cmTarget& target)
{
   // check to see if this is a fortran build
  const char* ext = ".vcproj";
  const char* project =
    "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"";
  if(this->TargetIsFortranOnly(target))
    {
    ext = ".vfproj";
    project = "Project(\"{6989167D-11E4-40FE-8C1A-2192A86A7E90}\") = \"";
    }

  fout << project
       << dspname << "\", \""
       << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
       << dspname << ext << "\", \"{"
       << this->GetGUID(dspname) << "}\"\nEndProject\n";

  UtilityDependsMap::iterator ui = this->UtilityDepends.find(&target);
  if(ui != this->UtilityDepends.end())
    {
    const char* uname = ui->second.c_str();
    fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \""
         << uname << "\", \""
         << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
         << uname << ".vcproj" << "\", \"{"
         << this->GetGUID(uname) << "}\"\n"
         << "EndProject\n";
    }
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void
cmGlobalVisualStudio7Generator
::WriteProjectDepends(std::ostream& fout,
                      const char* dspname,
                      const char*, cmTarget& target)
{
  int depcount = 0;
  std::string dspguid = this->GetGUID(dspname);
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
    fout << "\t\t{" << dspguid << "}." << depcount << " = {" << guid << "}\n";
    depcount++;
    }

  UtilityDependsMap::iterator ui = this->UtilityDepends.find(&target);
  if(ui != this->UtilityDepends.end())
    {
    const char* uname = ui->second.c_str();
    fout << "\t\t{" << this->GetGUID(uname) << "}.0 = {" << dspguid << "}\n";
    }
}


// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio7Generator
::WriteProjectConfigurations(
  std::ostream& fout, const char* name, cmTarget::TargetType,
  const std::set<std::string>& configsPartOfDefaultBuild,
  const char* platformMapping)
{
  const char* platformName =
    platformMapping ? platformMapping : this->GetPlatformName();
  std::string guid = this->GetGUID(name);
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i
         << ".ActiveCfg = " << *i << "|" << platformName << "\n";
      std::set<std::string>::const_iterator
        ci = configsPartOfDefaultBuild.find(*i);
      if(!(ci == configsPartOfDefaultBuild.end()))
      {
      fout << "\t\t{" << guid << "}." << *i
           << ".Build.0 = " << *i << "|" << platformName << "\n";
      }
    }
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalVisualStudio7Generator::WriteExternalProject(std::ostream& fout,
                               const char* name,
                               const char* location,
                               const char* typeGuid,
                               const std::set<cmStdString>&)
{
  std::string d = cmSystemTools::ConvertToOutputPath(location);
  fout << "Project("
       << "\"{"
       << (typeGuid ? typeGuid : "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942")
       << "}\") = \""
       << name << "\", \""
       << this->ConvertToSolutionPath(location) << "\", \"{"
       << this->GetGUID(name)
       << "}\"\n";
  fout << "EndProject\n";
}



void cmGlobalVisualStudio7Generator
::WriteSLNGlobalSections(std::ostream& fout,
                         cmLocalGenerator* root)
{
  bool extensibilityGlobalsOverridden = false;
  bool extensibilityAddInsOverridden = false;
  const cmPropertyMap& props = root->GetMakefile()->GetProperties();
  for(cmPropertyMap::const_iterator itProp = props.begin();
      itProp != props.end(); ++itProp)
    {
    if(itProp->first.find("VS_GLOBAL_SECTION_") == 0)
      {
      std::string sectionType;
      std::string name = itProp->first.substr(18);
      if(name.find("PRE_") == 0)
        {
        name = name.substr(4);
        sectionType = "preSolution";
        }
      else if(name.find("POST_") == 0)
        {
        name = name.substr(5);
        sectionType = "postSolution";
        }
      else
        continue;
      if(!name.empty())
        {
        if(name == "ExtensibilityGlobals" && sectionType == "postSolution")
          extensibilityGlobalsOverridden = true;
        else if(name == "ExtensibilityAddIns" && sectionType == "postSolution")
          extensibilityAddInsOverridden = true;
        fout << "\tGlobalSection(" << name << ") = " << sectionType << "\n";
        std::vector<std::string> keyValuePairs;
        cmSystemTools::ExpandListArgument(itProp->second.GetValue(),
                                          keyValuePairs);
        for(std::vector<std::string>::const_iterator itPair =
            keyValuePairs.begin(); itPair != keyValuePairs.end(); ++itPair)
          {
          const std::string::size_type posEqual = itPair->find('=');
          if(posEqual != std::string::npos)
            {
            const std::string key =
              cmSystemTools::TrimWhitespace(itPair->substr(0, posEqual));
            const std::string value =
              cmSystemTools::TrimWhitespace(itPair->substr(posEqual + 1));
            fout << "\t\t" << key << " = " << value << "\n";
            }
          }
        fout << "\tEndGlobalSection\n";
        }
      }
    }
  if(!extensibilityGlobalsOverridden)
    fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
         << "\tEndGlobalSection\n";
  if(!extensibilityAddInsOverridden)
    fout << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
         << "\tEndGlobalSection\n";
}



// Standard end of dsw file
void cmGlobalVisualStudio7Generator::WriteSLNFooter(std::ostream& fout)
{
  fout << "EndGlobal\n";
}


// ouput standard header for dsw file
void cmGlobalVisualStudio7Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 7.00\n";
}

//----------------------------------------------------------------------------
std::string
cmGlobalVisualStudio7Generator::WriteUtilityDepend(cmTarget* target)
{
  std::string pname = target->GetName();
  pname += "_UTILITY";
  std::string fname = target->GetMakefile()->GetStartOutputDirectory();
  fname += "/";
  fname += pname;
  fname += ".vcproj";
  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);
  this->CreateGUID(pname.c_str());
  std::string guid = this->GetGUID(pname.c_str());

  fout <<
    "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n"
    "<VisualStudioProject\n"
    "\tProjectType=\"Visual C++\"\n"
    "\tVersion=\"" << this->GetIDEVersion() << "0\"\n"
    "\tName=\"" << pname << "\"\n"
    "\tProjectGUID=\"{" << guid << "}\"\n"
    "\tKeyword=\"Win32Proj\">\n"
    "\t<Platforms><Platform Name=\"Win32\"/></Platforms>\n"
    "\t<Configurations>\n"
    ;
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout <<
      "\t\t<Configuration\n"
      "\t\t\tName=\"" << *i << "|Win32\"\n"
      "\t\t\tOutputDirectory=\"" << *i << "\"\n"
      "\t\t\tIntermediateDirectory=\"" << pname << ".dir\\" << *i << "\"\n"
      "\t\t\tConfigurationType=\"10\"\n"
      "\t\t\tUseOfMFC=\"0\"\n"
      "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"\n"
      "\t\t\tCharacterSet=\"2\">\n"
      "\t\t</Configuration>\n"
      ;
    }
  fout <<
    "\t</Configurations>\n"
    "\t<Files></Files>\n"
    "\t<Globals></Globals>\n"
    "</VisualStudioProject>\n"
    ;

  if(fout.Close())
    {
    this->FileReplacedDuringGenerate(fname);
    }
  return pname;
}

std::string cmGlobalVisualStudio7Generator::GetGUID(const char* name)
{
  std::string guidStoreName = name;
  guidStoreName += "_GUID_CMAKE";
  const char* storedGUID =
    this->CMakeInstance->GetCacheDefinition(guidStoreName.c_str());
  if(storedGUID)
    {
    return std::string(storedGUID);
    }
  cmSystemTools::Error("Unknown Target referenced : ",
                       name);
  return "";
}


void cmGlobalVisualStudio7Generator::CreateGUID(const char* name)
{
  std::string guidStoreName = name;
  guidStoreName += "_GUID_CMAKE";
  if(this->CMakeInstance->GetCacheDefinition(guidStoreName.c_str()))
    {
    return;
    }
  std::string ret;
  UUID uid;
  unsigned char *uidstr;
  UuidCreate(&uid);
  UuidToString(&uid,&uidstr);
  ret = reinterpret_cast<char*>(uidstr);
  RpcStringFree(&uidstr);
  ret = cmSystemTools::UpperCase(ret);
  this->CMakeInstance->AddCacheEntry(guidStoreName.c_str(),
                                     ret.c_str(), "Stored GUID",
                                     cmCacheManager::INTERNAL);
}

std::vector<std::string> *cmGlobalVisualStudio7Generator::GetConfigurations()
{
  return &this->Configurations;
};

//----------------------------------------------------------------------------
void cmGlobalVisualStudio7Generator
::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalVisualStudio7Generator::GetActualName();
  entry.Brief = "Generates Visual Studio .NET 2002 project files.";
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio7Generator
::AppendDirectoryForConfig(const char* prefix,
                           const char* config,
                           const char* suffix,
                           std::string& dir)
{
  if(config)
    {
    dir += prefix;
    dir += config;
    dir += suffix;
    }
}

std::set<std::string>
cmGlobalVisualStudio7Generator::IsPartOfDefaultBuild(const char* project,
                                                     cmTarget* target)
{
  std::set<std::string> activeConfigs;
  // if it is a utilitiy target then only make it part of the
  // default build if another target depends on it
  int type = target->GetType();
  if (type == cmTarget::GLOBAL_TARGET)
    {
    return activeConfigs;
    }
  if(type == cmTarget::UTILITY && !this->IsDependedOn(project, target))
    {
    return activeConfigs;
    }
  // inspect EXCLUDE_FROM_DEFAULT_BUILD[_<CONFIG>] properties
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    const char* propertyValue =
      target->GetFeature("EXCLUDE_FROM_DEFAULT_BUILD", i->c_str());
    if(cmSystemTools::IsOff(propertyValue))
      {
      activeConfigs.insert(*i);
      }
    }
  return activeConfigs;
}

//----------------------------------------------------------------------------
static cmVS7FlagTable cmVS7ExtraFlagTable[] =
{
  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  {"UsePrecompiledHeader", "YX", "Automatically Generate", "2",
   cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue},
  {"PrecompiledHeaderThrough", "YX", "Precompiled Header Name", "",
   cmVS7FlagTable::UserValueRequired},
  {"UsePrecompiledHeader", "Yu", "Use Precompiled Header", "3",
   cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue},
  {"PrecompiledHeaderThrough", "Yu", "Precompiled Header Name", "",
   cmVS7FlagTable::UserValueRequired},
  {"WholeProgramOptimization", "LTCG", "WholeProgramOptimization", "TRUE", 0},

  // Exception handling mode.  If no entries match, it will be FALSE.
  {"ExceptionHandling", "GX", "enable c++ exceptions", "TRUE", 0},
  {"ExceptionHandling", "EHsc", "enable c++ exceptions", "TRUE", 0},
  // The EHa option does not have an IDE setting.  Let it go to false,
  // and have EHa passed on the command line by leaving out the table
  // entry.

  {0,0,0,0,0}
};
cmIDEFlagTable const* cmGlobalVisualStudio7Generator::GetExtraFlagTableVS7()
{
  return cmVS7ExtraFlagTable;
}
