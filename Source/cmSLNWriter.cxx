/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSLNWriter.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmVCProjWriter.h"
#include "cmMSDotNETGenerator.h"
#include "cmCacheManager.h"
#include "windows.h"

cmSLNWriter::cmSLNWriter(cmMakefile* m)
{
  m_Makefile = m;
}

// output the SLN file
void cmSLNWriter::OutputSLNFile()
{ 
  // if this is an out of source build, create the output directory
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating output directory for SLN file",
                           m_Makefile->GetStartOutputDirectory());
      }
    }
  // create the dsw file name
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_Makefile->GetProjectName()) == 0)
    {
    m_Makefile->SetProjectName("Project");
    }
  fname += m_Makefile->GetProjectName();
  fname += ".sln";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open SLN file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteSLNFile(fout);
}


// Write a SLN file to the stream
void cmSLNWriter::WriteSLNFile(std::ostream& fout)
{
  // Write out the header for a SLN file
  this->WriteSLNHeader(fout);
  
  // Create a list of cmMakefile created from all the
  // CMakeLists.txt files that are in sub directories of
  // this one.
  std::vector<cmMakefile*> allListFiles;
  // add this makefile to the list
  allListFiles.push_back(m_Makefile);
  // add a special target that depends on ALL projects for easy build
  // of Debug only
  m_Makefile->AddUtilityCommand("ALL_BUILD", "echo", "\"Build all projects\"",
                                false);
  m_Makefile->FindSubDirectoryCMakeListsFiles(allListFiles);
  // For each cmMakefile, create a VCProj for it, and
  // add it to this SLN file
  std::vector<cmMakefile*>::iterator k;
  for(k = allListFiles.begin();
      k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg = 0;
    // if not this makefile, then create a new generator
    if(m_Makefile != mf)
      {
      // Create an MS generator with SLN off, so it only creates dsp files
      pg = new cmMSDotNETGenerator;
      }
    else
      {
      pg = static_cast<cmMSDotNETGenerator*>(m_Makefile->GetMakefileGenerator());
      }
    // make sure the generator is building dsp files
    pg->BuildSLNOff();
    mf->SetMakefileGenerator(pg);
    mf->GenerateMakefile();
    // Get the source directory from the makefile
    std::string dir = mf->GetStartDirectory();
    // Get the home directory with the trailing slash
    std::string homedir = m_Makefile->GetHomeDirectory();
    homedir += "/";
    // remove the home directory and / from the source directory
    // this gives a relative path 
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");

    // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetVCProjWriter()->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetVCProjWriter()->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      // special handling for the current makefile
      if(mf == m_Makefile)
        {
        dir = "."; // no subdirectory for project generated
        // if this is the special ALL_BUILD utility, then
        // make it depend on every other non UTILITY project.
        // This is done by adding the names to the GetUtilities
        // vector on the makefile
        if(l->first == "ALL_BUILD")
          {
          for(std::vector<cmMakefile*>::iterator a = allListFiles.begin();
              a != allListFiles.end(); ++a)
            {
            const cmTargets &atgts = (*a)->GetTargets();
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
                  l->second.GetLinkLibraries().push_back(
                    cmTarget::LinkLibraries::value_type(al->first,
                                                        cmTarget::GENERAL));
                  }
                }
              }
            }
          }
        }
      // Write the project into the SLN file
      if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
      {
        cmCustomCommand cc = l->second.GetCustomCommands()[0];
        
        // dodgy use of the cmCustomCommand's members to store the 
        // arguments from the INCLUDE_EXTERNAL_MSPROJECT command
        std::vector<std::string> stuff = cc.GetDepends();
        std::vector<std::string> depends = cc.GetOutputs();
        this->WriteExternalProject(fout, stuff[0].c_str(), stuff[1].c_str(), depends);
        ++si;
      }
      else if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProject(fout, si->c_str(), dir.c_str(), 
                           pg->GetVCProjWriter(),l->second);
        ++si;
        }
      }
    }
  fout << "Global\n"
       << "\tGlobalSection(SolutionConfiguration) = preSolution\n"
       << "\t\tConfigName.0 = Debug\n"
       << "\t\tConfigName.1 = MinSizeRel\n"
       << "\t\tConfigName.2 = Release\n"
       << "\t\tConfigName.3 = RelWithDebInfo\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ProjectDependencies) = postSolution\n";
  // loop over again and compute the depends
  for(k = allListFiles.begin(); k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg =  
      static_cast<cmMSDotNETGenerator*>(mf->GetMakefileGenerator());
        // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetVCProjWriter()->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetVCProjWriter()->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectDepends(fout, si->c_str(), dir.c_str(), 
                                  pg->GetVCProjWriter(),l->second);
        ++si;
        }
      }
    }
  fout << "\tEndGlobalSection\n";
  fout << "\tGlobalSection(ProjectConfiguration) = postSolution\n";
    // loop over again and compute the depends
  for(k = allListFiles.begin(); k != allListFiles.end(); ++k)
    {
    cmMakefile* mf = *k;
    cmMSDotNETGenerator* pg =  
      static_cast<cmMSDotNETGenerator*>(mf->GetMakefileGenerator());
        // Get the list of create dsp files names from the cmVCProjWriter, more
    // than one dsp could have been created per input CMakeLists.txt file
    // for each target
    std::vector<std::string> dspnames = 
      pg->GetVCProjWriter()->GetCreatedProjectNames();
    cmTargets &tgts = pg->GetVCProjWriter()->GetMakefile()->GetTargets();
    cmTargets::iterator l = tgts.begin();
    std::string dir = mf->GetStartDirectory();
    for(std::vector<std::string>::iterator si = dspnames.begin(); 
        l != tgts.end(); ++l)
      {
      if ((l->second.GetType() != cmTarget::INSTALL_FILES)
          && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS))
        {
        this->WriteProjectConfigurations(fout, si->c_str());
        ++si;
        }
      }
    // delete the cmMakefile which also deletes the cmMSProjectGenerator
    if(mf != m_Makefile)
      {
      delete mf;
      }
    }
  fout << "\tEndGlobalSection\n";

  // Write the footer for the SLN file
  this->WriteSLNFooter(fout);
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmSLNWriter::WriteProject(std::ostream& fout, 
                               const char* dspname,
                               const char* dir,
                               cmVCProjWriter*,
                               const cmTarget& target
  )
{
  std::string d = dir;
  cmSystemTools::ConvertToWindowsSlashes(d);
  fout << "Project(\"{" << this->CreateGUID(m_Makefile->GetProjectName()) 
       << "}\") = \"" << dspname << "\", \""
       << d << "\\" << dspname << ".vcproj\", \"{"
       << this->CreateGUID(dspname) << "}\"\nEndProject\n";
}



// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmSLNWriter::WriteProjectDepends(std::ostream& fout, 
                                      const char* dspname,
                                      const char* dir,
                                      cmVCProjWriter*,
                                      const cmTarget& target
  )
{
  int depcount = 0;
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
        const char* cacheValue
          = m_Makefile->GetDefinition(j->first.c_str());
        if(cacheValue)
          {
          fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
               << this->CreateGUID(j->first.c_str()) << "}\n";
          depcount++;
          }
        }
      }
    }

  std::set<std::string>::const_iterator i, end;
  // write utility dependencies.
  i = target.GetUtilities().begin();
  end = target.GetUtilities().end();
  for(;i!= end; ++i)
    {
    if(*i != dspname)
      {
      fout << "\t\t{" << this->CreateGUID(dspname) << "}." << depcount << " = {"
           << this->CreateGUID(i->c_str()) << "}\n";
      depcount++;
      }
    }
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmSLNWriter::WriteProjectConfigurations(std::ostream& fout, const char* name)
{
  std::string guid = this->CreateGUID(name);
  fout << "\t\t{" << guid << "}.Debug.ActiveCfg = Debug|Win32\n"
       << "\t\t{" << guid << "}.Debug.Build.0 = Debug|Win32\n"
       << "\t\t{" << guid << "}.MinSizeRel.ActiveCfg = Debug|Win32\n"
       << "\t\t{" << guid << "}.MinSizeRel.Build.0 = Debug|Win32\n"
       << "\t\t{" << guid << "}.Release.ActiveCfg = Debug|Win32\n"
       << "\t\t{" << guid << "}.Release.Build.0 = Debug|Win32\n"
       << "\t\t{" << guid << "}.RelWithDebInfo.ActiveCfg = Debug|Win32\n"
       << "\t\t{" << guid << "}.RelWithDebInfo.Build.0 = Debug|Win32\n";
}


// Write a dsp file into the SLN file,
// Note, that dependencies from executables to 
// the libraries it uses are also done here
void cmSLNWriter::WriteExternalProject(std::ostream& fout, 
			       const char* name,
			       const char* location,
                               const std::vector<std::string>& dependencies)
{
  cmSystemTools::Error("WriteExternalProject not implemented");
//  fout << "#########################################################"
//     "######################\n\n";
//   fout << "Project: \"" << name << "\"=" 
//        << location << " - Package Owner=<4>\n\n";
//   fout << "Package=<5>\n{{{\n}}}\n\n";
//   fout << "Package=<4>\n";
//   fout << "{{{\n";

  
//   std::vector<std::string>::const_iterator i, end;
//   // write dependencies.
//   i = dependencies.begin();
//   end = dependencies.end();
//   for(;i!= end; ++i)
//   {
//     fout << "Begin Project Dependency\n";
//     fout << "Project_Dep_Name " << *i << "\n";
//     fout << "End Project Dependency\n";
//   }
//   fout << "}}}\n\n";
}



// Standard end of dsw file
void cmSLNWriter::WriteSLNFooter(std::ostream& fout)
{
  fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
       << "\tEndGlobalSection\n"
       << "EndGlobal\n";
}

  
// ouput standard header for dsw file
void cmSLNWriter::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 7.00\n";
}


std::string cmSLNWriter::CreateGUID(const char* name)
{
  std::map<cmStdString, cmStdString>::iterator i = m_GUIDMap.find(name);
  if(i != m_GUIDMap.end())
    {
    return i->second;
    }
  std::string ret;
  UUID uid;
  unsigned char *uidstr;
  UuidCreate(&uid);
  UuidToString(&uid,&uidstr);
  ret = reinterpret_cast<char*>(uidstr);
  RpcStringFree(&uidstr);
  m_GUIDMap[name] = ret;
  return ret;
}
