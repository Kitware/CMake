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
#include "cmRegularExpression.h"
#include "cmCacheManager.h"

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
    m_LibraryOptions += " ";
    m_LibraryOptions += *i;
    m_LibraryOptions += ".lib ";
    }
  std::vector<std::string>& libswin32 = m_Makefile->GetLinkLibrariesWin32();
  for(i = libswin32.begin(); i != libswin32.end(); ++i)
    {
    m_LibraryOptions += " ";
    m_LibraryOptions += *i;
    m_LibraryOptions += ".lib ";
    }
  std::vector<std::string>& libdirs = m_Makefile->GetLinkDirectories();
  for(i = libdirs.begin(); i != libdirs.end(); ++i)
    {
    m_LibraryOptions += " /LIBPATH:\"";
    m_LibraryOptions += *i;
    m_LibraryOptions += "/$(OUTDIR)\" ";
    }
  m_LibraryOptions += "/STACK:10000000 ";
  m_OutputLibName = m_Makefile->GetLibraryName();
  
  // Create the DSP or set of DSP's for libraries and executables
  if(strlen(m_Makefile->GetLibraryName()) != 0)
    {
    const char* cacheValue
      = cmCacheManager::GetInstance()->GetCacheValue("BUILD_SHARED_LIBS");
    if(cacheValue && strcmp(cacheValue,"0"))
      {
      this->SetBuildType(DLL);
      }
    else
      {
      this->SetBuildType(STATIC_LIBRARY);
      }
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
//        m_Makefile->SetLibraryName(classfile.m_ClassName.c_str());
        this->SetBuildType(EXECUTABLE);
        m_OutputLibName = classfile.m_ClassName;
        std::string pname = classfile.m_ClassName.c_str(); //m_Makefile->GetLibraryName();
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
  std::string makefileIn = "\"";
  makefileIn += m_Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt\"";
  std::string dsprule = "\"";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += "/CMake/Source/CMakeSetupCMD\" ";
  dsprule += makefileIn;
  dsprule += " -DSP -H\"";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += "\" -S\"";
  dsprule += m_Makefile->GetStartDirectory();
  dsprule += "\" -O\"";
  dsprule += m_Makefile->GetStartOutputDirectory();
  dsprule += "\" -B\"";
  dsprule += m_Makefile->GetHomeOutputDirectory();
  dsprule += "\"";

  std::set<std::string> depends;
  std::set<std::string> outputs;
  outputs.insert(outputs.begin(), dspname);
  fout << "# Begin Source File\n\n";
  fout << "SOURCE=" << makefileIn.c_str() << "\n\n";
  this->WriteCustomRule(fout, dsprule.c_str(), depends, outputs);
  fout << "# End Source File\n";
}


void cmDSPMakefile::AddDSPBuildRule(cmSourceGroup& sourceGroup)
{
  std::string dspname = *(m_CreatedProjectNames.end()-1);
  dspname += ".dsp";
  std::string makefileIn = "\"";
  makefileIn += m_Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt\"";
  std::string dsprule = "\"";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += "/CMake/Source/CMakeSetupCMD\" ";
  dsprule += makefileIn;
  dsprule += " -DSP -H\"";
  dsprule += m_Makefile->GetHomeDirectory();
  dsprule += "\" -S\"";
  dsprule += m_Makefile->GetStartDirectory();
  dsprule += "\" -O\"";
  dsprule += m_Makefile->GetStartOutputDirectory();
  dsprule += "\" -B\"";
  dsprule += m_Makefile->GetHomeOutputDirectory();
  dsprule += "\"";

  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  outputs.push_back(dspname);
  sourceGroup.AddCustomCommand(makefileIn.c_str(), dsprule.c_str(),
                               depends, outputs);
}


void cmDSPMakefile::WriteDSPFile(std::ostream& fout)
{
  // Write the DSP file's header.
  this->WriteDSPHeader(fout);
  
  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = m_Makefile->GetSourceGroups();
  
  // Find the group in which the CMakeLists.txt source belongs, and add
  // the rule to generate this DSP file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = sourceGroups.rbegin();
      sg != sourceGroups.rend(); ++sg)
    {
    if(sg->Matches("CMakeLists.txt"))
      {
      this->AddDSPBuildRule(*sg);
      break;
      }    
    }
  
  // Loop through every source group.
  for(std::vector<cmSourceGroup>::const_iterator sg = sourceGroups.begin();
      sg != sourceGroups.end(); ++sg)
    {
    const std::vector<std::string>& sources = sg->GetSources();
    const cmSourceGroup::CustomCommands& customCommands = sg->GetCustomCommands();
    // If the group is empty, don't write it at all.
    if(sources.empty() && customCommands.empty())
      { continue; }
    
    // If the group has a name, write the header.
    std::string name = sg->GetName();
    if(name != "")
      {
      this->WriteDSPBeginGroup(fout, name.c_str(), "");
      }
    
    // Loop through each source in the source group.
    for(std::vector<std::string>::const_iterator s = sources.begin();
        s != sources.end(); ++s)
      {
      this->WriteDSPBuildRule(fout, s->c_str());
      }    
    
    // Loop through each custom command in the source group.
    for(cmSourceGroup::CustomCommands::const_iterator cc =
          customCommands.begin(); cc != customCommands.end(); ++ cc)
      {
      std::string source = cc->first;
      const cmSourceGroup::Commands& commands = cc->second;

      fout << "# Begin Source File\n\n";
      fout << "SOURCE=" << source << "\n\n";
      
      // Loop through every command generating code from the current source.
      for(cmSourceGroup::Commands::const_iterator c = commands.begin();
          c != commands.end(); ++c)
        {
        std::string command = c->first;
        const cmSourceGroup::CommandFiles& commandFiles = c->second;
        this->WriteCustomRule(fout, command.c_str(), commandFiles.m_Depends,
                              commandFiles.m_Outputs);
        }      
      
      fout << "# End Source File\n";
      }
    
    // If the group has a name, write the footer.
    if(name != "")
      {
      this->WriteDSPEndGroup(fout);
      }
    }  

  // Write the DSP file's footer.
  this->WriteDSPFooter(fout);
}


void cmDSPMakefile::WriteCustomRule(std::ostream& fout,
                                    const char* command,
                                    const std::set<std::string>& depends,
                                    const std::set<std::string>& outputs)
{
  std::vector<std::string>::iterator i;
  for(i = m_Configurations.begin(); i != m_Configurations.end(); ++i)
    {
    if (i == m_Configurations.begin())
      {
      fout << "!IF  \"$(CFG)\" == " << i->c_str() << std::endl;
      }
    else 
      {
      fout << "!ELSEIF  \"$(CFG)\" == " << i->c_str() << std::endl;
      }
    fout << "# Begin Custom Build\n\n";
    
    // Write a rule for every output generated by this command.
    for(std::set<std::string>::const_iterator output = outputs.begin();
        output != outputs.end(); ++output)
      {
      fout << "\"" << output->c_str()
           << "\" :  \"$(SOURCE)\" \"$(INTDIR)\" \"$(OUTDIR)\"";
      // Write out all the dependencies for this rule.
      for(std::set<std::string>::const_iterator d = depends.begin();
          d != depends.end(); ++d)
        {
        fout << " \"" << d->c_str() << "\"";
        }
      fout << "\n  " << command << "\n\n";
      }
    
    fout << "# End Custom Build\n\n";
    }
  
  fout << "!ENDIF\n\n";
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
  m_BuildType = b;
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

  // once the build type is set, determine what configurations are
  // possible
  std::ifstream fin(m_DSPHeaderTemplate.c_str());
  cmRegularExpression reg("# Name ");
  if(!fin)
    {
    cmSystemTools::Error("Error Reading ", m_DSPHeaderTemplate.c_str());
    }

  // reset m_Configurations
  m_Configurations.erase(m_Configurations.begin(), m_Configurations.end());
  // now add all the configurations possible
  char buffer[2048];
  while(fin)
    {
    fin.getline(buffer, 2048);
    std::string line = buffer;
    cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME", 
                                 m_Makefile->GetLibraryName());
    if (reg.find(line))
      {
      m_Configurations.push_back(line.substr(reg.end()));
      }
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
      cmSystemTools::ReplaceString(line, "CM_LIBRARIES",
                                    m_LibraryOptions.c_str());
      cmSystemTools::ReplaceString(line, "BUILD_INCLUDES",
                                    m_IncludeOptions.c_str());
      cmSystemTools::ReplaceString(line, "OUTPUT_LIBNAME", 
                                    m_OutputLibName.c_str());
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


void cmDSPMakefile::WriteDSPBuildRule(std::ostream& fout, const char* path)
{
  fout << "# Begin Source File\n\n";
  fout << "SOURCE=" 
       << path << "\n";
  fout << "# End Source File\n";
}

bool cmDSPMakefile::NeedsDependencies(const char* dspname)
{
  if(strcmp(m_Makefile->GetLibraryName(), dspname) == 0)
    {
    // only shared libs need depend info
    const char* cacheValue
      = cmCacheManager::GetInstance()->GetCacheValue("BUILD_SHARED_LIBS");
    if(cacheValue && strcmp(cacheValue,"0"))
      {
      return true;
      }
    else
      {
      return false;
      }
    }
  // must be an executable so it needs depends
  return true;
}

  
  
