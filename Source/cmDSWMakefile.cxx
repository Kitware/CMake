#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmDSWMakefile.h"
#include "cmDSPBuilder.h"
#include "cmSystemTools.h"
#include <iostream>
#include <fstream>
#include <windows.h>


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
    std::cerr  << "Error can not open " << fname.c_str() << " for write" << std::endl;
    return;
    }
  this->WriteDSWFile(fout);
}


void cmDSWMakefile::WriteDSWFile(std::ostream& fout)
{
  this->WriteDSWHeader(fout);
  for(std::vector<std::string>::iterator i = m_SubDirectories.begin();
      i != m_SubDirectories.end(); ++i)
    {
    const char* dir = (*i).c_str();
    std::vector<std::string> dspnames = this->CreateDSPFile(dir);
    std::cerr << "Create dsp for " << dspnames.size() << " number of dsp files in " << dir << std::endl;
    for(std::vector<std::string>::iterator si = dspnames.begin();
	si != dspnames.end(); ++si)
      {
      std::string dspname = *si;
      std::cerr << "Create dsp for " << (*si).c_str() << std::endl;
      if(dspname == "")
	{
	std::cerr << "Project name not found in " << dir << "/Makefile.in" << std::endl;
	std::cerr << "Skipping Project " << std::endl;
	}
      else
	{
	std::string subdir = "./";
	subdir += dir;
	this->WriteProject(fout, dspname.c_str(), subdir.c_str());
	}
      }
    }
  this->WriteDSWFooter(fout);
}

std::vector<std::string> cmDSWMakefile::CreateDSPFile(const char* subdir)
{
#undef GetCurrentDirectory
  std::string currentDir = this->GetCurrentDirectory();
  currentDir += "/";
  currentDir += subdir;
  cmDSPBuilder dsp;
  dsp.SetOutputHomeDirectory(this->GetOutputDirectory());
  dsp.SetHomeDirectory(this->GetHomeDirectory());
  dsp.SetMakefileDirectory(currentDir.c_str());
  std::string outdir = m_OutputDirectory;
  outdir += "/";
  outdir += subdir;
  dsp.SetOutputDirectory(outdir.c_str());
  currentDir += "/";
  currentDir += "Makefile.in";
  dsp.SetInputMakefilePath(currentDir.c_str());
  dsp.CreateDSPFile();
  return dsp.GetCreatedProjectNames();
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
