#include "cmDSPMakefile.h"
#include "cmSystemTools.h"
#include "cmCollectFlags.h"
#include <iostream>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef GetCurrentDirectory

static void Die(const char* message)
{
  MessageBox(0, message, 0, MB_OK);
  exit(-1);
}


void cmDSPMakefile::OutputDSPFile()
{ 
  std::vector<std::string>& includes = m_BuildFlags.GetIncludeDirectories();
  std::vector<std::string>::iterator i;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    cmSystemTools::ReplaceString(include, "${CMAKE_CONFIG_DIR}",
				 this->GetOutputHomeDirectory() );
    cmSystemTools::ReplaceString(include, "${srcdir}",
				 this->GetHomeDirectory() );
    m_IncludeOptions +=  "/I \"";
    m_IncludeOptions += include;
    m_IncludeOptions += "\" ";
    }
  std::vector<std::string>& libs = m_BuildFlags.GetLinkLibraries();
  for(i = libs.begin(); i != libs.end(); ++i)
    {
    m_DebugLibraryOptions += " ";
    m_DebugLibraryOptions += *i;
    m_DebugLibraryOptions += ".lib ";
    }
  std::vector<std::string>& libswin32 = m_BuildFlags.GetLinkLibrariesWin32();
  for(i = libswin32.begin(); i != libswin32.end(); ++i)
    {
    m_DebugLibraryOptions += " ";
    m_DebugLibraryOptions += *i;
    m_DebugLibraryOptions += ".lib ";
    }
  std::vector<std::string>& libdirs = m_BuildFlags.GetLinkDirectories();
  for(i = libdirs.begin(); i != libdirs.end(); ++i)
    {
    m_DebugLibraryOptions += " /LIBPATH:\"";
    m_DebugLibraryOptions += *i;
    cmSystemTools::ReplaceString(m_DebugLibraryOptions, "${CMAKE_CONFIG_DIR}",
				 this->GetOutputHomeDirectory() );
    if(i->find("Debug") == std::string::npos)
      {
      if(i->find("Release") == std::string::npos)
	{
	m_DebugLibraryOptions += "/Debug\" ";
	}
      }
    }
  m_DebugLibraryOptions += "/STACK:10000000 ";
  m_ReleaseLibraryOptions = m_DebugLibraryOptions;
  cmSystemTools::ReplaceString(m_ReleaseLibraryOptions, "Debug", "Release");
  // If the output directory is not the m_cmHomeDirectory
  // then create it.
  if(m_OutputDirectory != m_cmHomeDirectory)
    {
    if(!cmSystemTools::MakeDirectory(m_OutputDirectory.c_str()))
      {
      std::string message = "Error creating directory ";
      message += m_OutputDirectory;
      Die(message.c_str());
      }
    }
  
  if(!m_Executables)
    {
    if(this->m_LibraryName == "")
      {
      // if no library silently give up
      return;
      }
    this->SetBuildType(STATIC_LIBRARY);
    this->CreateSingleDSP();
    }
  else
    {
    this->CreateExecutableDSPFiles();
    }
}
void cmDSPMakefile::CreateExecutableDSPFiles()
{
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
      std::string message = "Error Writing ";
      message += fname;
      Die(message.c_str());
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
  fname = m_OutputDirectory;
  fname += "/";
  fname += this->m_LibraryName;
  fname += ".dsp";
  m_CreatedProjectNames.clear();
  std::string pname = m_LibraryName;
  m_CreatedProjectNames.push_back(pname);
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    std::string message = "Error Writing ";
    message += fname;
    Die(message.c_str());
    }
  this->WriteDSPFile(fout);
}

void cmDSPMakefile::WriteDSPBuildRule(std::ostream& fout)
{
  std::string dspname = *(m_CreatedProjectNames.end()-1);
  dspname += ".dsp";
  std::string makefileIn = this->GetCurrentDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  std::string dsprule = GetHomeDirectory();
  dsprule += "/CMake/Source/CMakeSetupCMD ";
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
      m_DSPHeaderTemplate += "/CMake/Source/staticLibHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/Source/staticLibFooter.dsptemplate";
      break;
    case DLL:
      m_DSPHeaderTemplate = m_cmHomeDirectory;
      m_DSPHeaderTemplate += "/CMake/Source/DLLHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/Source/DLLFooter.dsptemplate";
      break;
    case EXECUTABLE:
      m_DSPHeaderTemplate = m_cmHomeDirectory;
      m_DSPHeaderTemplate += "/CMake/Source/EXEHeader.dsptemplate";
      m_DSPFooterTemplate = m_cmHomeDirectory;
      m_DSPFooterTemplate += "/CMake/Source/EXEFooter.dsptemplate";
      break;
    }
}

  
void cmDSPMakefile::WriteDSPHeader(std::ostream& fout)
{
  std::ifstream fin(m_DSPHeaderTemplate.c_str());
  if(!fin)
    {
    std::string message = "Error Reading ";
    message += m_DSPHeaderTemplate;
    Die(message.c_str());
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
    std::string message = "Error Reading ";
    message += m_DSPFooterTemplate;
    Die(message.c_str());
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
