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
  m_FindMakeProgramFile = "CMakeVS71FindMake.cmake";
}



///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio71Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion71();
  lg->SetGlobalGenerator(this);
  return lg;
}



// Write a SLN file to the stream
void cmGlobalVisualStudio71Generator::WriteSLNFile(std::ostream& fout,
                                                   cmLocalGenerator* root,
                                                   std::vector<cmLocalGenerator*>& generators)
{
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);
  
  // Get the home directory with the trailing slash
  std::string homedir = root->GetMakefile()->GetCurrentDirectory();
  homedir += "/";
  bool doneAllBuild = false;
  bool doneRunTests = false;
  bool doneInstall  = false;
  
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
    std::string dir = mf->GetStartDirectory();
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      static_cast<cmLocalVisualStudio7Generator *>(generators[i])
      ->GetCreatedProjectNames();
    cmTargets &tgts = generators[i]->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end() && si != dspnames.end(); ++l)
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
            const cmTargets &atgts = 
              generators[j]->GetMakefile()->GetTargets();
            for(cmTargets::const_iterator al = atgts.begin();
                al != atgts.end(); ++al)
              {
              if (al->second.IsInAll())
                {
                if (al->second.GetType() == cmTarget::UTILITY)
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
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends;
        depends.push_back(cc.GetOutput());
        this->WriteExternalProject(fout, stuff[0].c_str(), 
                                   stuff[1].c_str(), depends);
        }
      else 
        {
        if ((l->second.GetType() != cmTarget::INSTALL_FILES)
            && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
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
          if(!skip)
            {
            this->WriteProject(fout, si->c_str(), dir.c_str(),l->second);
            }
          ++si;
          }
        }
      }
    }
  fout << "Global\n"
       << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\t" << *i << " = " << *i << "\n";
    }
  fout << "\tEndGlobalSection\n";
  fout << "\tGlobalSection(ProjectConfiguration) = postSolution\n";
  // loop over again and compute the depends
  for(i = 0; i < generators.size(); ++i)
    {
    cmMakefile* mf = generators[i]->GetMakefile();
    cmLocalVisualStudio7Generator* pg =  
      static_cast<cmLocalVisualStudio7Generator*>(generators[i]);
    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end() && si != dspnames.end(); ++l)
      {
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        {
        cmCustomCommand cc = l->second.GetPostBuildCommands()[0];
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        this->WriteProjectConfigurations(fout, stuff[0].c_str(), l->second.IsInAll());
        }
      else if ((l->second.GetType() != cmTarget::INSTALL_FILES)
               && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectConfigurations(fout, si->c_str(), l->second.IsInAll());
        ++si;
        }
      }
    }
  fout << "\tEndGlobalSection\n";

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteProject(std::ostream& fout, 
                               const char* dspname,
                               const char* dir,
                               const cmTarget& t)
{
  std::string d = cmSystemTools::ConvertToOutputPath(dir);
  fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" 
       << dspname << "\", \""
       << d << "\\" << dspname << ".vcproj\", \"{"
       << this->GetGUID(dspname) << "}\"\n";
  fout << "\tProjectSection(ProjectDependencies) = postProject\n";
  this->WriteProjectDepends(fout, dspname, dir, t);
  fout << "\tEndProjectSection\n";
  
  fout <<"EndProject\n";
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmGlobalVisualStudio71Generator::WriteProjectDepends(std::ostream& fout, 
                                      const char* dspname,
                                      const char* ,
                                      const cmTarget& target
  )
{
  // insert Begin Project Dependency  Project_Dep_Name project stuff here 
  if (target.GetType() != cmTarget::STATIC_LIBRARY)
    {
    cmTarget::LinkLibraries::const_iterator j, jend;
    j = target.GetLinkLibraries().begin();
    jend = target.GetLinkLibraries().end();
    for(;j!= jend; ++j)
      {
      if(j->first != dspname)
        {
        // is the library part of this SLN ? If so add dependency
        std::string libPath = j->first + "_CMAKE_PATH";
        const char* cacheValue
          = m_CMakeInstance->GetCacheDefinition(libPath.c_str());
        if(cacheValue && *cacheValue)
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
      std::string name = i->c_str();
      if(strncmp(name.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
        name.erase(name.begin(), name.begin() + 27);
      fout << "\t\t{" << this->GetGUID(name.c_str()) << "} = {"
           << this->GetGUID(name.c_str()) << "}\n";
      }
    }
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void 
cmGlobalVisualStudio71Generator::WriteProjectConfigurations(std::ostream& fout, 
                                                           const char* name, 
                                                           bool in_all_build)
{
  std::string guid = this->GetGUID(name);
  for(std::vector<std::string>::iterator i = m_Configurations.begin();
      i != m_Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i << ".ActiveCfg = " << *i << "|Win32\n";
    if (in_all_build)
      {
      fout << "\t\t{" << guid << "}." << *i << ".Build.0 = " << *i << "|Win32\n";
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
void cmGlobalVisualStudio71Generator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Visual Studio .NET 2003 project files.";
  entry.full = "";
}
