/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmDSWMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmDSPMakefile.h"
#include "cmMSProjectGenerator.h"
#include <windows.h>

// microsoft nonsense
#undef GetCurrentDirectory
#undef SetCurrentDirectory

cmDSWMakefile::cmDSWMakefile(cmMakefile* m)
{
  m_Makefile = m;
}

// output the DSW file
void cmDSWMakefile::OutputDSWFile()
{ 
  if(m_Makefile->GetStartOutputDirectory() == "")
    {
    // default to build in place
    m_Makefile->SetStartOutputDirectory(m_Makefile->GetHomeDirectory());
    }
  // If the output directory is not the m_cmHomeDirectory
  // then create it.
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      MessageBox(0, "Error creating directory ", 0, MB_OK);
      MessageBox(0, m_Makefile->GetStartOutputDirectory(), 0, MB_OK);
      }
    }
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += m_Makefile->GetProjectName();
  fname += ".dsw";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: " , fname.c_str());
    return;
    }
  this->WriteDSWFile(fout);
}

// ------------------------------------------------
// Recursive function to find all the CMakeLists.txt files
// in a project.  As each file is read in, any directories in
// the SUBDIR variable are also passed back to this function.
// The result is a vector of cmDSPMakefile objects, one for
// each directory with a CMakeLists.txt file
//
void 
cmDSWMakefile
::FindAllCMakeListsFiles(const char* subdir,
                         std::vector<cmMSProjectGenerator*>& makefiles)
{
  std::string currentDir = m_Makefile->GetCurrentDirectory();
  currentDir += "/";
  currentDir += subdir;
  currentDir += "/";
  currentDir += "CMakeLists.txt";
  // CMakeLists.txt exits in the subdirectory
  // then create a cmDSPMakefile for it
  if(cmSystemTools::FileExists(currentDir.c_str()))
    {
    // Create a new cmDSPMakefile to read the currentDir CMakeLists.txt file
    cmMSProjectGenerator* pg = new cmMSProjectGenerator;
    pg->BuildDSWOff();
    cmMakefile* mf = new cmMakefile;
    mf->SetMakefileGenerator(pg);
    // add it to the vector
    makefiles.push_back(pg);
    // Set up the file with the current context
    mf->SetHomeOutputDirectory(m_Makefile->GetStartOutputDirectory());
    mf->SetHomeDirectory(m_Makefile->GetHomeDirectory());
    // Set the output directory which may be different than the source
    std::string outdir = m_Makefile->GetStartOutputDirectory();
    outdir += "/";
    outdir += subdir;
    mf->SetStartOutputDirectory(outdir.c_str());
    // set the current directory in the Source as a full
    // path
    std::string currentDir = m_Makefile->GetStartDirectory();
    currentDir += "/";
    currentDir += subdir;
    mf->SetStartDirectory(currentDir.c_str());
    // Parse the CMakeLists.txt file
    currentDir += "/CMakeLists.txt";
    mf->MakeStartDirectoriesCurrent();
    mf->ReadListFile(currentDir.c_str());
    // Create the DSP file
    mf->GenerateMakefile();
    // Look at any sub directories parsed (SUBDIRS) and 
    // recurse into them    
    const std::vector<std::string>& subdirs = mf->GetSubDirectories();
    for(std::vector<std::string>::const_iterator i = subdirs.begin();
	i != subdirs.end(); ++i)
      {
      // append the subdirectory to the current directoy subdir
      std::string nextDir = subdir;
      nextDir += "/";
      nextDir += i->c_str();
      // recurse into nextDir
      this->FindAllCMakeListsFiles(nextDir.c_str(),
				   makefiles);
      }
    }
  else
    {
    cmSystemTools::Error("Can not find CMakeLists.txt in ",
                         currentDir.c_str());
    }
}


// Write a DSW file to the stream
void cmDSWMakefile::WriteDSWFile(std::ostream& fout)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  // Create an array of dsp files for the project
  std::vector<cmMSProjectGenerator*> dspfiles;
  // loop over all the subdirectories for the DSW file,
  // and find all sub directory projects
  const std::vector<std::string>& dirs = m_Makefile->GetSubDirectories();
  for(std::vector<std::string>::const_iterator j = dirs.begin();
      j != dirs.end(); ++j)
    {
    this->FindAllCMakeListsFiles(j->c_str(), dspfiles);
    }
  // For each DSP file created insert them into the DSW file
  for(std::vector<cmMSProjectGenerator*>::iterator k = dspfiles.begin();
      k != dspfiles.end(); ++k)
    {
    // Get the directory for the dsp file, it comes
    // from the source, so it has the source path which needs
    // to be removed as this may be built in a different directory
    // than the source
    std::string dir = (*k)->GetDSPMakefile()->
      GetMakefile()->GetStartDirectory();
    // Get the home directory with the trailing slash
    std::string homedir = m_Makefile->GetHomeDirectory();
    homedir += "/";
    // make the directory relative by removing the home directory part
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");
    // Get the list of create dsp files from the cmDSPMakefile, more
    // than one dsp could have been created per input CMakeLists.txt file
    std::vector<std::string> dspnames =
      (*k)->GetDSPMakefile()->GetCreatedProjectNames();
    for(std::vector<std::string>::iterator si = dspnames.begin();
	si != dspnames.end(); ++si)
      {
      // Write the project into the DSW file
      this->WriteProject(fout, si->c_str(), dir.c_str(), 
                         (*k)->GetDSPMakefile());
      }
    // delete the cmDSPMakefile object once done with it to avoid
    // leaks
    delete (*k)->GetDSPMakefile()->GetMakefile();
    }
  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}


void cmDSWMakefile::WriteProject(std::ostream& fout, 
				 const char* dspname,
				 const char* dir,
                                 cmDSPMakefile* project)
{
  project->GetMakefile()->ExpandVariables();
  fout << "#########################################################"
    "######################\n\n";
  fout << "Project: \"" << dspname << "\"=" 
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";
  if(project->GetMakefile()->HasExecutables())
    {
    // insert Begin Project Dependency  Project_Dep_Name project stuff here 
    std::vector<std::string>::iterator i, end;
    i = project->GetMakefile()->GetLinkLibraries().begin();
    end = project->GetMakefile()->GetLinkLibraries().end();
    for(;i!= end; ++i)
      {
		if (strcmp(i->c_str(),dspname))
			{
			fout << "Begin Project Dependency\n";
			fout << "Project_Dep_Name " << *i << "\n";
			fout << "End Project Dependency\n";
			}
      }
    }
  fout << "}}}\n\n";
}

void cmDSWMakefile::WriteDSWFooter(std::ostream& fout)
{
  fout << "######################################################"
    "#########################\n\n";
  fout << "Global:\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<3>\n{{{\n}}}\n\n";
  fout << "#####################################################"
    "##########################\n\n";
}

  
void cmDSWMakefile::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}
