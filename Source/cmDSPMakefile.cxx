#include "cmDSPMakefile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef GetCurrentDirectory

static void Die(const char* message)
{
  MessageBox(0, message, 0, MB_OK);
  exit(-1);
}
cmDSPMakefile::~cmDSPMakefile()
{
}


cmDSPMakefile::cmDSPMakefile(cmMakefile*mf)
{
  m_Makefile = mf;
}

void cmDSPMakefile::OutputDSPFile()
{ 
  // Setup /I and /LIBPATH options
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
  m_DebugLibraryOptions += m_Makefile->GetDefineFlags();
  m_ReleaseLibraryOptions = m_DebugLibraryOptions;
  cmSystemTools::ReplaceString(m_ReleaseLibraryOptions, "Debug", "Release");
  
  // If the output directory is not the m_cmHomeDirectory
  // then create it.
  if(strcmp(m_Makefile->GetOutputDirectory(),
            m_Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_Makefile->GetOutputDirectory()))
      {
      std::string message = "Error creating directory ";
      message += m_Makefile->GetOutputDirectory();
      Die(message.c_str());
      }
    }
  
  if(!m_Makefile->HasExecutables())
    {
    if(strlen(m_Makefile->GetLibraryName()) == 0)
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
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(int i = 0; i < Classes.size(); ++i)
    {
    cmClassFile& classfile = Classes[i];
    std::string fname = m_Makefile->GetOutputDirectory();
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


void cmDSPMakefile::CreateSingleDSP()
{
  std::string fname;
  fname = m_Makefile->GetOutputDirectory();
  fname += "/";
  fname += m_Makefile->GetLibraryName();
  fname += ".dsp";
  m_CreatedProjectNames.clear();
  std::string pname = m_Makefile->GetLibraryName();
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
  std::string makefileIn = m_Makefile->GetCurrentDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  std::string dsprule = m_Makefile->GetHomeDirectory();
  dsprule += "/CMake/Source/CMakeSetupCMD ";
  dsprule += makefileIn;
  dsprule += " -DSP -H";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += " -D";
  dsprule += m_Makefile->GetCurrentDirectory();
  dsprule += " -O";
  dsprule += m_Makefile->GetOutputDirectory();
  dsprule += " -B";
  dsprule += m_Makefile->GetOutputHomeDirectory();
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
                                    m_Makefile->GetLibraryName());
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
  std::vector<cmClassFile>& Classes = m_Makefile->GetClasses();
  for(int i = 0; i < Classes.size(); ++i)
    {
    if(!Classes[i].m_AbstractClass && !Classes[i].m_HeaderFileOnly)
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
