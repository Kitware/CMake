#include "cmDSPMakefile.h"
#include "cmSystemTools.h"
#include <iostream>
#include <fstream>
#include <windows.h>

void cmDSPMakefile::OutputDSPFile()
{ 
  m_IncludeOptions = "/STACK:10000000 ";
  m_IncludeOptions = "/I \"";
  m_IncludeOptions += this->GetHomeDirectory();
  m_IncludeOptions += "/Code/Common\" ";
  m_IncludeOptions += "/I \"";
  m_IncludeOptions += this->GetHomeDirectory();
  m_IncludeOptions += "/Code/Insight3DParty/vxl\" ";
  // Add the Build directory vcl to the -I path for config.h type stuff
  m_IncludeOptions += "/I \"";
  m_IncludeOptions += this->GetOutputHomeDirectory();
  m_IncludeOptions += "/Code/Insight3DParty/vxl\" ";
  // Add the Build directory to the -I path for config.h type stuff
  m_IncludeOptions += "/I \"";
  m_IncludeOptions += this->GetOutputHomeDirectory();
  m_IncludeOptions += "\" ";
  m_DebugLibraryOptions = " ITKCommon.lib ITKNumerics.lib ";
  m_DebugLibraryOptions += " /LIBPATH:\"";
  m_DebugLibraryOptions += this->GetOutputHomeDirectory();
  m_DebugLibraryOptions += "/Code/Common/Debug\" ";
  m_DebugLibraryOptions += " /LIBPATH:\"";
  m_DebugLibraryOptions += this->GetOutputHomeDirectory();
  m_DebugLibraryOptions += "/Code/Insight3DParty/vxl/Debug\" ";
  m_ReleaseLibraryOptions = m_DebugLibraryOptions;
  cmSystemTools::ReplaceString(m_ReleaseLibraryOptions, "Debug", "Release");
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
  
  if(!m_Executables)
    {
    if(this->m_LibraryName == "")
      {
      std::cerr  << "No library name in Makefile.in dsp not created" << std::endl;
      return;
      }
    std::cerr << "building library " << this->m_LibraryName.c_str() << std::endl;
    this->SetBuildType(STATIC_LIBRARY);
    this->CreateSingleDSP();
    }
  else
    {
    std::cerr << "Build Executables " << std::endl;
    this->CreateExecutableDSPFiles();
    }
}
void cmDSPMakefile::CreateExecutableDSPFiles()
{
  std::cerr << "Create executables for ";
  for(int i = 0; i < m_Classes.size(); ++i)
    {
    cmClassFile& classfile = m_Classes[i];
    std::string fname = m_OutputDirectory;
    fname += "/";
    fname += classfile.m_ClassName;
    fname += ".dsp";
    std::ofstream fout(fname.c_str());
    if(!fout)
      {
      MessageBox(0, "Error writing ", 0, MB_OK);
      MessageBox(0, fname.c_str(), 0, MB_OK);
      std::cerr  << "Error can not open " << fname.c_str() << " for write" << std::endl;
      }
    else
      {
      m_LibraryName = classfile.m_ClassName;
      this->SetBuildType(EXECUTABLE);
      std::string pname = m_LibraryName;
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


void cmDSPMakefile::CreateSingleDSP()
{
  std::string fname;
  std::cerr << "writting dsp file " << m_cmCurrentDirectory.c_str()
	    << std::endl;
  fname = m_OutputDirectory;
  fname += "/";
  fname += this->m_LibraryName;
  fname += ".dsp";
  m_CreatedProjectNames.clear();
  std::string pname = m_LibraryName;
  m_CreatedProjectNames.push_back(pname);
  std::cerr << "writting dsp file " << fname.c_str() << std::endl;
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    MessageBox(0, "Error writing ", 0, MB_OK);
    MessageBox(0, fname.c_str(), 0, MB_OK);
    std::cerr  << "Error can not open " << fname.c_str() << " for write" << std::endl;
    return;
  }
  this->WriteDSPFile(fout);
}

void cmDSPMakefile::WriteDSPBuildRule(std::ostream& fout)
{
  std::string dspname = *(m_CreatedProjectNames.end()-1);
  dspname += ".dsp";
#undef GetCurrentDirectory
  std::string makefileIn = this->GetCurrentDirectory();
  makefileIn += "/";
  makefileIn += "Makefile.in";
  std::string dsprule = GetHomeDirectory();
  dsprule += "/CMake/pcbuilderCMD ";
  dsprule += makefileIn;
  dsprule += " -DSP -H";
  dsprule += this->GetHomeDirectory();
  dsprule += " -D";
  dsprule += this->GetCurrentDirectory();
  dsprule += " -O";
  dsprule += this->GetOutputDirectory();
  dsprule += " -B";
  dsprule += this->GetOutputHomeDirectory();
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
      m_DSPHeaderTemplate = m_cmHomeDirectory;
      m_DSPHeaderTemplate += "/CMake/staticLibHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/staticLibFooter.dsptemplate";
      break;
    case DLL:
      m_DSPHeaderTemplate = m_cmHomeDirectory;
      m_DSPHeaderTemplate += "/CMake/DLLHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/DLLFooter.dsptemplate";
      break;
    case EXECUTABLE:
      m_DSPHeaderTemplate = m_cmHomeDirectory;
      m_DSPHeaderTemplate += "/CMake/EXEHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/EXEFooter.dsptemplate";
      break;
    }
}

  
void cmDSPMakefile::WriteDSPHeader(std::ostream& fout)
{
  std::ifstream fin(m_DSPHeaderTemplate.c_str());
  if(!fin)
    {
    std::cerr << "failed to open " << m_DSPHeaderTemplate.c_str() 
	      << " for read" << std::endl;
    return;
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
                                    m_LibraryName.c_str());
      cmSystemTools::ReplaceString(line, 
                                    "EXTRA_DEFINES", "");
      fout << line.c_str() << std::endl;
    }
}


void cmDSPMakefile::WriteDSPFooter(std::ostream& fout)
{  
  std::ifstream fin(m_DSPFooterTemplate.c_str());
  if(!fin)
    {
    std::cerr << "can not open " << m_DSPFooterTemplate.c_str() <<
      " for read" << std::endl;
    return;
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
  for(int i = 0; i < m_Classes.size(); ++i)
    {
    if(!m_Classes[i].m_AbstractClass && !m_Classes[i].m_HeaderFileOnly)
      {
      this->WriteDSPBuildRule(fout, m_Classes[i].m_FullPath.c_str());
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
