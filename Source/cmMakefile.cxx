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
#include "cmMakefile.h"
#include "cmCommand.h"
#include "cmStandardIncludes.h"
#include "cmClassFile.h"
#include "cmDirectory.h"
#include "cmSystemTools.h"
#include "cmMakefileGenerator.h"
#include "cmCommands.h"

// default is not to be building executables
cmMakefile::cmMakefile()
{
  m_DefineFlags = " ";
  m_Executables = false;
  m_MakefileGenerator = 0;
  this->AddDefaultCommands();
}

void cmMakefile::AddDefaultCommands()
{
  std::list<cmCommand*> commands;
  GetPredefinedCommands(commands);
  for(std::list<cmCommand*>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->AddCommand(*i);
    }
#ifdef _WIN32
  this->AddDefinition("WIN32", "1");
#else
  this->AddDefinition("UNIX", "1");
#endif
  // Cygwin is more like unix so enable the unix commands
#if defined(__CYGWIN__)
  this->AddDefinition("UNIX", "1");
#endif
}


cmMakefile::~cmMakefile()
{
  for(int i=0; i < m_UsedCommands.size(); i++)
    {
    delete m_UsedCommands[i];
    }
  for(RegisteredCommandsMap::iterator j = m_Commands.begin();
      j != m_Commands.end(); ++j)
    {
    delete (*j).second;
    }
  delete m_MakefileGenerator;
}

void cmMakefile::PrintStringVector(const char* s, std::vector<std::string>& v)
{
  std::cout << s << ": ( \n";
  for(std::vector<std::string>::iterator i = v.begin();
      i != v.end(); ++i)
    {
    std::cout << (*i).c_str() << " ";
    }
  std::cout << " )\n";
}


// call print on all the classes in the makefile
void cmMakefile::Print()
{
  std::cout << "classes:\n";
  for(unsigned int i = 0; i < m_Classes.size(); i++)
    m_Classes[i].Print();
  std::cout << " m_OutputDirectory; " << 
    m_OutputDirectory.c_str() << std::endl;
  std::cout << " m_OutputHomeDirectory; " << 
    m_OutputHomeDirectory.c_str() << std::endl;
  std::cout << " m_cmHomeDirectory; " << 
    m_cmHomeDirectory.c_str() << std::endl;
  std::cout << " m_cmCurrentDirectory; " << 
    m_cmCurrentDirectory.c_str() << std::endl;
  std::cout << " m_LibraryName;	" <<  m_LibraryName.c_str() << std::endl;
  std::cout << " m_ProjectName;	" <<  m_ProjectName.c_str() << std::endl;
  this->PrintStringVector("m_SubDirectories ", m_SubDirectories); 
  this->PrintStringVector("m_MakeVerbatim ", m_MakeVerbatim); 
  this->PrintStringVector("m_IncludeDirectories;", m_IncludeDirectories);
  this->PrintStringVector("m_LinkDirectories", m_LinkDirectories);
  this->PrintStringVector("m_LinkLibraries", m_LinkLibraries);
  this->PrintStringVector("m_LinkLibrariesWin32", m_LinkLibrariesWin32);
  this->PrintStringVector("m_LinkLibrariesUnix", m_LinkLibrariesUnix);
}

// Parse the given CMakeLists.txt file into a list of classes.
bool cmMakefile::ReadMakefile(const char* filename, bool inheriting)
{
  // If not being called from ParseDirectory which
  // sets the inheriting flag, then parse up the
  // tree and collect inherited parameters
  if(!inheriting)
    {
    cmSystemTools::ConvertToUnixSlashes(m_cmCurrentDirectory);
    m_SourceHomeDirectory = m_cmHomeDirectory;
    cmSystemTools::ConvertToUnixSlashes(m_SourceHomeDirectory);
    // if this is already the top level directory then 
    if(m_SourceHomeDirectory != m_cmCurrentDirectory)
      {
      this->ParseDirectory(m_cmCurrentDirectory.c_str());
      }
    }
  // Now read the input file
  std::ifstream fin(filename);
  if(!fin)
    {
    cmSystemTools::Error("error can not open file ", filename);
    return false;
    }
  std::string name;
  std::vector<std::string> arguments;
  while ( fin )
    {
    if(cmSystemTools::ParseFunction(fin, name, arguments) )
      {
      // Special command that needs to be removed when 
      // ADD_COMMAND is implemented
      if(name == "VERBATIM")
        {
        if(!inheriting)
          {
          m_MakeVerbatim = arguments;
          }
        }
      else
        {
        RegisteredCommandsMap::iterator pos = m_Commands.find(name);
        if(pos != m_Commands.end())
          {
          cmCommand* rm = (*pos).second;
          cmCommand* usedCommand = rm->Clone();
          usedCommand->SetMakefile(this);
          usedCommand->LoadCache();
          bool keepCommand = false;
          if(usedCommand->GetEnabled())
            {
            // if not running in inherit mode or
            // if the command is inherited then Invoke it.
            if(!inheriting || usedCommand->IsInherited())
              {
              if(!usedCommand->Invoke(arguments))
                {
                cmSystemTools::Error(usedCommand->GetError());
                }
              else
                {
                // use the command
                keepCommand = true;
                m_UsedCommands.push_back(usedCommand);
                }
              }
            }
          // if the Cloned command was not used 
          // then delete it
          if(!keepCommand)
            {
            delete usedCommand;
            }
          }
        else
          {
          cmSystemTools::Error("unknown CMake function", name.c_str());
          }
        }
      }
    }
  return true;
}

  

void cmMakefile::AddCommand(cmCommand* wg)
{
  std::string name = wg->GetName();
  m_Commands.insert( RegisteredCommandsMap::value_type(name, wg));
}

  // Set the make file 
void cmMakefile::SetMakefileGenerator(cmMakefileGenerator* mf)
{
  delete m_MakefileGenerator;
  m_MakefileGenerator = mf;
}

  // Generate the output file
void cmMakefile::GenerateMakefile()
{
  // do all the variable expansions here
  this->ExpandVaribles();
  // set the makefile on the generator
  m_MakefileGenerator->SetMakefile(this);
  // give all the commands a chance to do something
  // after the file has been parsed before generation
  for(std::vector<cmCommand*>::iterator i = m_UsedCommands.begin();
      i != m_UsedCommands.end(); ++i)
    {
    (*i)->FinalPass();
    }
  // now do the generation
  m_MakefileGenerator->GenerateMakefile();
}

void cmMakefile::AddClass(cmClassFile& cmfile)
{
  m_Classes.push_back(cmfile);
}



void cmMakefile::AddCustomCommand(const char* source,
                               const char* result,
                               const char* command,
                               std::vector<std::string>& depends)
{
  cmMakefile::customCommand customCommand;
  customCommand.m_Source = source;
  customCommand.m_Result = result;
  customCommand.m_Command = command;
  customCommand.m_Depends = depends;
  m_CustomCommands.push_back(customCommand);
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  m_DefineFlags += " ";
  m_DefineFlags += flag;
}

void cmMakefile::AddExecutable(cmClassFile& cf)
{
  m_Classes.push_back(cf);
  m_Executables = true;
}

void cmMakefile::AddLinkLibrary(const char* lib)
{
  m_LinkLibraries.push_back(lib);
}

void cmMakefile::AddLinkDirectory(const char* dir)
{
  m_LinkDirectories.push_back(dir);
}

void cmMakefile::AddSubDirectory(const char* sub)
{
  m_SubDirectories.push_back(sub);
}

void cmMakefile::AddIncludeDirectory(const char* inc)
{
  m_IncludeDirectories.push_back(inc);
}

void cmMakefile::AddDefinition(const char* name, const char* value)
{
  m_Definitions.insert(DefinitionMap::value_type(name, value));
}

void cmMakefile::SetProjectName(const char* p)
{
  m_ProjectName = p;
}

void cmMakefile::SetLibraryName(const char* l)
{
  m_LibraryName = l;
}


void cmMakefile::AddExtraDirectory(const char* dir)
{
  m_AuxSourceDirectories.push_back(dir);
}


// Go until directory == m_cmHomeDirectory 
// 1. fix slashes
// 2. peal off /dir until home found, go no higher
void cmMakefile::ParseDirectory(const char* dir)
{
  std::string listsFile = dir;
  listsFile += "/CMakeLists.txt";
  if(cmSystemTools::FileExists(listsFile.c_str()))
    {
    this->ReadMakefile(listsFile.c_str(), true);
    }
  if(m_SourceHomeDirectory == dir)
    {
    return;
    }


  std::string dotdotDir = dir;
  std::string::size_type pos = dotdotDir.rfind('/');
  if(pos != std::string::npos)
    {
    dotdotDir = dotdotDir.substr(0, pos);
    this->ParseDirectory(dotdotDir.c_str());
    }
}


// expance CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR in the
// include and library directories.

void cmMakefile::ExpandVaribles()
{
  // make sure binary and source dir are defined
  this->AddDefinition("CMAKE_BINARY_DIR", this->GetOutputHomeDirectory());
  this->AddDefinition("CMAKE_SOURCE_DIR", this->GetHomeDirectory());

   // Now expand varibles in the include and link strings
  std::vector<std::string>::iterator j, begin, end;
  begin = m_IncludeDirectories.begin();
  end = m_IncludeDirectories.end();
  for(j = begin; j != end; ++j)
    {
    this->ExpandVariblesInString(*j);
    }
  begin = m_LinkDirectories.begin();
  end = m_LinkDirectories.end();
  for(j = begin; j != end; ++j)
    {
    this->ExpandVariblesInString(*j);
    }
}

const char* cmMakefile::GetDefinition(const char* name)
{
  DefinitionMap::iterator pos = m_Definitions.find(name);
  if(pos != m_Definitions.end())
    {
    return (*pos).second.c_str();
    }
  return 0;
}

int cmMakefile::DumpDocumentationToFile(const char *fileName)
{
  // Open the supplied filename
  std::ofstream f;
  f.open(fileName, std::ios::out);
  
  if ( f.fail() )
    {
    return 0;
    }
  
  // Loop over all registered commands and print out documentation
  const char *name;
  const char *terse;
  const char *full;

  for(RegisteredCommandsMap::iterator j = m_Commands.begin();
      j != m_Commands.end(); ++j)
    {
    name = (*j).second->GetName();
    terse = (*j).second->GetTerseDocumentation();
    full = (*j).second->GetFullDocumentation();
    f << name << " - " << terse << std::endl
      << "Usage: " << full << std::endl << std::endl;
    }
  

  return 1;
}


void cmMakefile::ExpandVariblesInString(std::string& source)
{
  for(DefinitionMap::iterator i = m_Definitions.begin();
      i != m_Definitions.end(); ++i)
    {
    std::string variable = "${";
    variable += (*i).first;
    variable += "}";
    cmSystemTools::ReplaceString(source, variable.c_str(),
                                 (*i).second.c_str());
    variable = "@";
    variable += (*i).first;
    variable += "@";
    cmSystemTools::ReplaceString(source, variable.c_str(),
                                 (*i).second.c_str());
    }
}

