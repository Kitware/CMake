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
#include "cmDSPMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"


cmDSPMakefile::~cmDSPMakefile()
{
}


cmDSPMakefile::cmDSPMakefile(cmMakefile*mf)
{
  m_Makefile = mf;
}

void cmDSPMakefile::OutputDSPFile()
{ 
  // If not an in source build, then create the output directory
  if(strcmp(m_Makefile->GetStartOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating directory ",
                           m_Makefile->GetStartOutputDirectory());
      }
    }

  // Setup /I and /LIBPATH options for the resulting DSP file
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    m_IncludeOptions +=  "/I \"";
    m_IncludeOptions += *i;
    m_IncludeOptions += "\" ";
    }
  std::vector<std::string>& libs = m_Makefile->GetLinkLibraries();
  for(i = libs.begin(); i != libs.end(); ++i)
    {
    m_DebugLibraryOptions += " ";
    m_DebugLibraryOptions += *i;
    m_DebugLibraryOptions += ".lib ";
    }
  std::vector<std::string>& libswin32 = m_Makefile->GetLinkLibrariesWin32();
  for(i = libswin32.begin(); i != libswin32.end(); ++i)
    {
    m_DebugLibraryOptions += " ";
    m_DebugLibraryOptions += *i;
    m_DebugLibraryOptions += ".lib ";
    }
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(i = libdirs.begin(); i != libdirs.end(); ++i)
    {
    m_DebugLibraryOptions += " /LIBPATH:\"";
    m_DebugLibraryOptions += *i;
    if(i->find("Debug") == std::string::npos)
      {
      if(i->find("Release") == std::string::npos)
	{
	m_DebugLibraryOptions += "/Debug\" ";
	}
      }
    }
  m_DebugLibraryOptions += "/STACK:10000000 ";
  // add any extra define flags 
  m_ReleaseLibraryOptions = m_DebugLibraryOptions;
  cmSystemTools::ReplaceString(m_ReleaseLibraryOptions, "Debug", "Release");
  

  
  // Create the DSP or set of DSP's for libraries and executables
  if(strlen(m_Makefile->GetLibraryName()) != 0)
    {
    this->SetBuildType(STATIC_LIBRARY);
    this->CreateSingleDSP();
    }
  // if there are executables build them
  if (m_Makefile->HasExecutables())
    {
    this->CreateExecutableDSPFiles();
    }
}
void cmDSPMakefile::CreateExecutableDSPFiles()
{
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(int i = 0; i < Classes.size(); ++i)
    {
    cmClassFile& classfile = Classes[i];
    if (classfile.m_IsExecutable)
      {
      std::string fname = m_Makefile->GetStartOutputDirectory();
      fname += "/";
      fname += classfile.m_ClassName;
      fname += ".dsp";
      std::ofstream fout(fname.c_str());
      if(!fout)
        {
        cmSystemTools::Error("Error Writing ",
                             fname.c_str());
        }
      else
        {
        m_Makefile->SetLibraryName(classfile.m_ClassName.c_str());
        this->SetBuildType(EXECUTABLE);
        std::string pname = m_Makefile->GetLibraryName();
        m_CreatedProjectNames.push_back(pname);
        
        this->WriteDSPHeader(fout);
        this->WriteDSPBeginGroup(fout, "Source Files", "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat");
        this->WriteDSPBuildRule(fout, classfile.m_FullPath.c_str());
        this->WriteDSPEndGroup(fout);
        this->WriteDSPBuildRule(fout);
        this->WriteDSPFooter(fout);
        }
      }
    }
}


void cmDSPMakefile::CreateSingleDSP()
{
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += m_Makefile->GetLibraryName();
  fname += ".dsp";
  m_CreatedProjectNames.clear();
  std::string pname = m_Makefile->GetLibraryName();
  m_CreatedProjectNames.push_back(pname);
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error Writing ",
                         fname.c_str());
    }
  this->WriteDSPFile(fout);
}

void cmDSPMakefile::WriteDSPBuildRule(std::ostream& fout)
{
  std::string dspname = *(m_CreatedProjectNames.end()-1);
  dspname += ".dsp";
  std::string makefileIn = m_Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  std::string dsprule = m_Makefile->GetHomeDirectory();
  dsprule += "/CMake/Source/CMakeSetupCMD ";
  dsprule += makefileIn;
  dsprule += " -DSP -H";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += " -S";
  dsprule += m_Makefile->GetStartDirectory();
  dsprule += " -O";
  dsprule += m_Makefile->GetStartOutputDirectory();
  dsprule += " -B";
  dsprule += m_Makefile->GetHomeOutputDirectory();
  this->WriteCustomRule(fout, makefileIn.c_str(), 
			dspname.c_str(),
			dsprule.c_str());
}

void cmDSPMakefile::WriteDSPFile(std::ostream& fout)
{
  this->WriteDSPHeader(fout);
  this->WriteDSPBeginGroup(fout, "Source Files", "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat");
  this->WriteDSPBuildRules(fout);
  this->WriteDSPEndGroup(fout);
  this->WriteDSPBuildRule(fout);
  this->WriteDSPFooter(fout);
}


void cmDSPMakefile::WriteCustomRule(std::ostream& fout,
                                    const char* source,
                                    const char* result,
                                    const char* command)
{
  fout << "# Begin Source File\n\n";
  fout << "SOURCE=" << source << "\n\n";
  fout << "# Begin Custom Build\n\n";
  fout << '\"' << result << "\" : $(SOURCE) \"$(INTDIR)\" \"$(OUTDIR)\"\n";
  fout << "  " << command << "\n\n";
  fout << "# End Custom Build\n\n";
  fout << "# End Source File\n";
}


void cmDSPMakefile::WriteDSPBeginGroup(std::ostream& fout, 
					const char* group,
					const char* filter)
{
  fout << "# Begin Group \"" << group << "\"\n"
    "# PROP Default_Filter \"" << filter << "\"\n";
}


void cmDSPMakefile::WriteDSPEndGroup(std::ostream& fout)
{
  fout << "# End Group\n";
}




void cmDSPMakefile::SetBuildType(BuildType b)
{
  switch(b)
    {
    case STATIC_LIBRARY:
      m_DSPHeaderTemplate = m_Makefile->GetHomeDirectory();
      m_DSPHeaderTemplate += "/CMake/Source/staticLibHeader.dsptemplate";
      m_DSPFooterTemplate = m_Makefile->GetHomeDirectory();
      m_DSPFooterTemplate += "/CMake/Source/staticLibFooter.dsptemplate";
      break;
    case DLL:
      m_DSPHeaderTemplate =  m_Makefile->GetHomeDirectory();
      m_DSPHeaderTemplate += "/CMake/Source/DLLHeader.dsptemplate";
      m_DSPFooterTemplate =  m_Makefile->GetHomeDirectory();
      m_DSPFooterTemplate += "/CMake/Source/DLLFooter.dsptemplate";
      break;
    case EXECUTABLE:
      m_DSPHeaderTemplate = m_Makefile->GetHomeDirectory();
      m_DSPHeaderTemplate += "/CMake/Source/EXEHeader.dsptemplate";
      m_DSPFooterTemplate = m_Makefile->GetHomeDirectory();
      m_DSPFooterTemplate += "/CMake/Source/EXEFooter.dsptemplate";
      break;
    }
}

  
void cmDSPMakefile::WriteDSPHeader(std::ostream& fout)
{
  std::ifstream fin(m_DSPHeaderTemplate.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ", m_DSPHeaderTemplate.c_str());
    }
  char buffer[2048];

  while(fin)
    {
      fin.getline(buffer, 2048);
      std::string line = buffer;
      cmSystemTools::ReplaceString(line, "CM_RELEASE_LIBRARIES",
                                    m_ReleaseLibraryOptions.c_str());
      cmSystemTools::ReplaceString(line, "CM_DEBUG_LIBRARIES",
                                    m_DebugLibraryOptions.c_str());
      cmSystemTools::ReplaceString(line, "BUILD_INCLUDES",
                                    m_IncludeOptions.c_str());
      cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME", 
                                    m_Makefile->GetLibraryName());
      cmSystemTools::ReplaceString(line, 
                                    "EXTRA_DEFINES", 
				   m_Makefile->GetDefineFlags());
      fout << line.c_str() << std::endl;
    }
}


void cmDSPMakefile::WriteDSPFooter(std::ostream& fout)
{  
  std::ifstream fin(m_DSPFooterTemplate.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ",
                         m_DSPFooterTemplate.c_str());
    }
  char buffer[2048];
  while(fin)
    {
      fin.getline(buffer, 2048);
      fout << buffer << std::endl;
    }
}

					
void cmDSPMakefile::WriteDSPBuildRules(std::ostream& fout)
{
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(int i = 0; i < Classes.size(); ++i)
    {
    if(!Classes[i].m_IsExecutable && !Classes[i].m_AbstractClass && 
       !Classes[i].m_HeaderFileOnly)
      {
      this->WriteDSPBuildRule(fout, Classes[i].m_FullPath.c_str());
      }
    }
}

void cmDSPMakefile::WriteDSPBuildRule(std::ostream& fout, const char* path)
{
  fout << "# Begin Source File\n\n";
  fout << "SOURCE=" 
       << path << "\n";
  fout << "# End Source File\n";
}
