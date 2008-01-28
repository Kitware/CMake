/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio71Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio71Generator::cmGlobalVisualStudio71Generator()
{
  this->FindMakeProgramFile = "CMakeVS71FindMake.cmake";
  this->ProjectConfigurationSectionName = "ProjectConfiguration";
}



///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio71Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion71();
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS7());
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalVisualStudio71Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  mf->AddDefinition("MSVC71", "1");
}

// Write a SLN file to the stream
void cmGlobalVisualStudio71Generator
::WriteSLNFile(std::ostream& fout,
               cmLocalGenerator* root,
               std::vector<cmLocalGenerator*>& generators)
{
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);
  
  // Get the start directory with the trailing slash
  std::string rootdir = root->GetMakefile()->GetStartOutputDirectory();
  rootdir += "/";
  bool doneAllBuild = false;
  bool doneCheckBuild = false;
  bool doneRunTests = false;
  bool doneInstall  = false;
  bool doneEditCache = false;
  bool doneRebuildCache = false;
  bool donePackage = false;
  
  // For each cmMakefile, create a VCProj for it, and
  // add it to this SLN file
  unsigned int i;
  for(i = 0; i < generators.size(); ++i)
    {
    if(this->IsExcluded(root, generators[i]))
      {
      continue;
      }
    cmMakefile* mf = generators[i]->GetMakefile();

    // Get the source directory from the makefile
    std::string dir = mf->GetStartOutputDirectory();
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, rootdir.c_str(), "");

    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    cmTargets &tgts = generators[i]->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(; l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == generators[0]->GetMakefile())
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD" && !doneAllBuild)
          {
          unsigned int j;
          for(j = 0; j < generators.size(); ++j)
            {
            cmTargets &atgts = generators[j]->GetMakefile()->GetTargets();
            for(cmTargets::iterator al = atgts.begin();
                al != atgts.end(); ++al)
              {
              if (!al->second.GetPropertyAsBool("EXCLUDE_FROM_ALL"))
                {
                if (al->second.GetType() == cmTarget::UTILITY ||
                    al->second.GetType() == cmTarget::GLOBAL_TARGET)
                  {
                  l->second.AddUtility(al->first.c_str());
                  }
                else
                  {
                  l->second.AddLinkLibrary(al->first,cmTarget::GENERAL);
                  }
                }
              }
            }
          }
        }
      // Write the project into the SLN file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        cmCustomCommand cc = l->second.GetPostBuildCommands()[0];
        const cmCustomCommandLines& cmds = cc.GetCommandLines();
        std::string project = cmds[0][0];
        std::string location = cmds[0][1];
        this->WriteExternalProject(fout, project.c_str(), 
                                   location.c_str(), cc.GetDepends());
        }
      else 
        {
          bool skip = false;
          if(l->first == "ALL_BUILD" )
            {
            if(doneAllBuild)
              {
              skip = true;
              }
            else
              {
              doneAllBuild = true;
              }
            }
          if(l->first == CMAKE_CHECK_BUILD_SYSTEM_TARGET)
            {
            if(doneCheckBuild)
              {
              skip = true;
              }
            else
              {
              doneCheckBuild = true;
              }
            }
          if(l->first == "INSTALL")
            {
            if(doneInstall)
              {
              skip = true;
              }
            else
              {
              doneInstall = true;
              }
            }
          if(l->first == "RUN_TESTS")
            {
            if(doneRunTests)
              {
              skip = true;
              }
            else
              {
              doneRunTests = true;
              }
            }
          if(l->first == "EDIT_CACHE")
            {
            if(doneEditCache)
              {
              skip = true;
              }
            else
              {
              doneEditCache = true;
              }
            }
          if(l->first == "REBUILD_CACHE")
            {
            if(doneRebuildCache)
              {
              skip = true;
              }
            else
              {
              doneRebuildCache = true;
              }
            }
          if(l->first == "PACKAGE")
            {
            if(donePackage)
              {
              skip = true;
              }
            else
              {
              donePackage = true;
              }
            }
          if(!skip)
            {
            const char *dspname = 
              l->second.GetProperty("GENERATOR_FILE_NAME");
            if (dspname)
              {
              this->WriteProject(fout, dspname, dir.c_str(),l->second);
              }
            }
        }
      }
    }
  fout << "Global\n";
  this->WriteSolutionConfigurations(fout);
  fout << "\tGlobalSection(" << this->ProjectConfigurationSectionName
       << ") = postSolution\n";
  // loop over again and compute the depends
  for(i = 0; i < generators.size(); ++i)
    {
    cmMakefile* mf = generators[i]->GetMakefile();
    cmLocalVisualStudio7Generator* pg =  
      static_cast<cmLocalVisualStudio7Generator*>(generators[i]);
    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(; l != tgts.end(); ++l)
      {
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        cmCustomCommand cc = l->second.GetPostBuildCommands()[0];
        const cmCustomCommandLines& cmds = cc.GetCommandLines();
        std::string project = cmds[0][0];
        this->WriteProjectConfigurations(fout, project.c_str(),
                                         true);
        }
      else
        {
        bool partOfDefaultBuild = this->IsPartOfDefaultBuild(
          root->GetMakefile()->GetProjectName(),
          &l->second);
        const char *dspname = 
          l->second.GetProperty("GENERATOR_FILE_NAME");
        if (dspname)
          {
          this->WriteProjectConfigurations(fout, dspname,
                                           partOfDefaultBuild);
          }
        }
      }
    }
  fout << "\tEndGlobalSection\n";

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}


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

// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void
cmGlobalVisualStudio71Generator::WriteProject(std::ostream& fout,
                                              const char* dspname,
                                              const char* dir,
                                              cmTarget& t)
{
  fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" 
       << dspname << "\", \""
       << this->ConvertToSolutionPath(dir)
       << "\\" << dspname << ".vcproj\", \"{"
       << this->GetGUID(dspname) << "}\"\n";
  fout << "\tProjectSection(ProjectDependencies) = postProject\n";
  this->WriteProjectDepends(fout, dspname, dir, t);
  fout << "\tEndProjectSection\n";
  
  fout <<"EndProject\n";
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void
cmGlobalVisualStudio71Generator
::WriteProjectDepends(std::ostream& fout,
                      const char* dspname,
                      const char*, cmTarget& target)
{
#if 0
  // Create inter-target dependencies in the solution file.  For VS
  // 7.1 and below we cannot let static libraries depend directly on
  // targets to which they "link" because the librarian tool will copy
  // the targets into the static library.  See
  // cmGlobalVisualStudioGenerator::FixUtilityDependsForTarget for a
  // work-around.  VS 8 and above do not have this problem.
  if (!this->VSLinksDependencies() ||
      target.GetType() != cmTarget::STATIC_LIBRARY);
#else
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
#endif
    {
    cmTarget::LinkLibraryVectorType::const_iterator j, jend;
    j = target.GetLinkLibraries().begin();
    jend = target.GetLinkLibraries().end();
    for(;j!= jend; ++j)
      {
      if(j->first != dspname)
        {
        // is the library part of this SLN ? If so add dependency
        if(this->FindTarget(this->CurrentProject.c_str(), j->first.c_str()))
          {
          fout << "\t\t{" << this->GetGUID(j->first.c_str()) << "} = {"
               << this->GetGUID(j->first.c_str()) << "}\n";
          }
        }
      }
    }

  std::set<cmStdString>::const_iterator i, end;
  // write utility dependencies.
  i = target.GetUtilities().begin();
  end = target.GetUtilities().end();
  for(;i!= end; ++i)
    {
    if(*i != dspname)
      {
      std::string name = this->GetUtilityForTarget(target, i->c_str());
      std::string guid = this->GetGUID(name.c_str());
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
}

// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalVisualStudio71Generator
::WriteExternalProject(std::ostream& fout, 
                       const char* name,
                       const char* location,
                       const std::vector<std::string>& depends)
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
    std::vector<std::string>::const_iterator it;
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




// Standard end of dsw file
void cmGlobalVisualStudio71Generator::WriteSLNFooter(std::ostream& fout)
{
  fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "EndGlobal\n";
}

  
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
