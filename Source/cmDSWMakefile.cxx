#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmDSWMakefile.h"
#include "cmSystemTools.h"
#include "cmDSPMakefile.h"
#include <iostream>
#include <fstream>
#include <windows.h>

// microsoft nonsense
#undef GetCurrentDirectory
#undef SetCurrentDirectory

// virtual override, ouput the makefile 
void cmDSWMakefile::OutputDSWFile()
{ 
  if(m_OutputDirectory == "")
    {
    // default to build in place
    m_OutputDirectory = m_cmHomeDirectory;
    }
  // If the output directory is not the m_cmHomeDirectory
  // then create it.
  if(m_OutputDirectory != m_cmHomeDirectory)
    {
    if(!cmSystemTools::MakeDirectory(m_OutputDirectory.c_str()))
      {
      MessageBox(0, "Error creating directory ", 0, MB_OK);
      MessageBox(0, m_OutputDirectory.c_str(), 0, MB_OK);
      }
    }
  std::string fname;
  fname = m_OutputDirectory;
  fname += "/";
  fname += this->m_LibraryName;
  fname += ".dsw";
  std::cerr << "writting dsw file " << fname.c_str() << std::endl;
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    std::cerr  << "Error can not open " 
	       << fname.c_str() << " for write" << std::endl;
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
void cmDSWMakefile::FindAllCMakeListsFiles(const char* subdir,
					   std::vector<cmDSPMakefile*>& makefiles)
{
  std::string currentDir = this->GetCurrentDirectory();
  currentDir += "/";
  currentDir += subdir;
  currentDir += "/";
  currentDir += "CMakeLists.txt";
  // CMakeLists.txt exits in the subdirectory
  // then create a cmDSPMakefile for it
  if(cmSystemTools::FileExists(currentDir.c_str()))
    {
    // Create a new cmDSPMakefile to read the currentDir CMakeLists.txt file
    cmDSPMakefile* dsp = new cmDSPMakefile;
    // add it to the vector
    makefiles.push_back(dsp);
    // Set up the file with the current context
    dsp->SetOutputHomeDirectory(this->GetOutputDirectory());
    dsp->SetHomeDirectory(this->GetHomeDirectory());
    // set the current directory in the Source as a full
    // path
    std::string currentDir = this->GetCurrentDirectory();
    currentDir += "/";
    currentDir += subdir;
    dsp->SetCurrentDirectory(currentDir.c_str());
    // Parse the CMakeLists.txt file
    currentDir += "/CMakeLists.txt";
    dsp->ReadMakefile(currentDir.c_str());
    // Set the output directory which may be different than the source
    std::string outdir = m_OutputDirectory;
    outdir += "/";
    outdir += subdir;
    dsp->SetOutputDirectory(outdir.c_str());
    // Create the DSP file
    dsp->OutputDSPFile();
    // Look at any sub directories parsed (SUBDIRS) and 
    // recurse into them    
    const std::vector<std::string>& subdirs = dsp->GetSubDirectories();
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
    std::cerr << "Can not find CMakeLists.txt in " << currentDir.c_str() 
	      << std::endl;
    }
}


// Write a DSW file to the stream
void cmDSWMakefile::WriteDSWFile(std::ostream& fout)
{
  // Write out the header for a DSW file
  this->WriteDSWHeader(fout);
  // Create an array of dsp files for the project
  std::vector<cmDSPMakefile*> dspfiles;
  // loop over all the subdirectories for the DSW file,
  // and find all sub directory projects
  for(std::vector<std::string>::iterator j = m_SubDirectories.begin();
      j != m_SubDirectories.end(); ++j)
    {
    this->FindAllCMakeListsFiles(j->c_str(), dspfiles);
    }
  // For each DSP file created insert them into the DSW file
  for(std::vector<cmDSPMakefile*>::iterator k = dspfiles.begin();
      k != dspfiles.end(); ++k)
    {
    // Get the directory for the dsp file, it comes
    // from the source, so it has the source path which needs
    // to be removed as this may be built in a different directory
    // than the source
    std::string dir = (*k)->GetCurrentDirectory();
    // Get the home directory with the trailing slash
    std::string homedir = this->GetHomeDirectory();
    homedir += "/";
    // make the directory relative by removing the home directory part
    cmSystemTools::ReplaceString(dir, homedir.c_str(), "");
    // Get the list of create dsp files from the cmDSPMakefile, more
    // than one dsp could have been created per input CMakeLists.txt file
    std::vector<std::string> dspnames = (*k)->GetCreatedProjectNames();
    std::cerr << "Create dsp for " 
	      << dspnames.size()
	      << " number of dsp files in " << dir << std::endl;
    for(std::vector<std::string>::iterator si = dspnames.begin();
	si != dspnames.end(); ++si)
      {
      // Write the project into the DSW file
      this->WriteProject(fout, si->c_str(), dir.c_str());
      }
    // delete the cmDSPMakefile object once done with it to avoid
    // leaks
    delete *k;
    }
  // Write the footer for the DSW file
  this->WriteDSWFooter(fout);
}


void cmDSWMakefile::WriteProject(std::ostream& fout, 
				 const char* dspname,
				 const char* dir)
{
  fout << "###############################################################################\n\n";
  fout << "Project: \"" << dspname << "\"=" 
       << dir << "\\" << dspname << ".dsp - Package Owner=<4>\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<4>\n";
  fout << "{{{\n";
  // insert Begin Project Dependency  Project_Dep_Name project stuff here
  fout << "}}}\n\n";
}

void cmDSWMakefile::WriteDSWFooter(std::ostream& fout)
{
  fout << "###############################################################################\n\n";
  fout << "Global:\n\n";
  fout << "Package=<5>\n{{{\n}}}\n\n";
  fout << "Package=<3>\n{{{\n}}}\n\n";
  fout << "###############################################################################\n\n";
}

  
void cmDSWMakefile::WriteDSWHeader(std::ostream& fout)
{
  fout << "Microsoft Developer Studio Workspace File, Format Version 6.00\n";
  fout << "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n\n";
}
