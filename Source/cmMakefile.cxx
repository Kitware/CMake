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
#include "cmRuleMaker.h"
#include "cmStandardIncludes.h"
#include "cmClassFile.h"
#include "cmDirectory.h"
#include "cmSystemTools.h"
#include "cmMakefileGenerator.h"

#include "cmAbstractFilesRule.h"
#include "cmAddTargetRule.h"
#include "cmAuxSourceDirectoryRule.h"
#include "cmExecutablesRule.h"
#include "cmFindIncludeRule.h"
#include "cmFindLibraryRule.h"
#include "cmFindProgramRule.h"
#include "cmIncludeDirectoryRule.h"
#include "cmLibraryRule.h"
#include "cmLinkDirectoriesRule.h"
#include "cmLinkLibrariesRule.h"
#include "cmProjectRule.h"
#include "cmSourceFilesRule.h"
#include "cmSourceFilesRequireRule.h"
#include "cmSubdirRule.h"
#include "cmUnixDefinesRule.h"
#include "cmUnixLibrariesRule.h"
#include "cmWin32DefinesRule.h"
#include "cmWin32LibrariesRule.h"
#include "cmTestsRule.h"

// default is not to be building executables
cmMakefile::cmMakefile()
{
  m_DefineFlags = " ";
  m_Executables = false;
  m_MakefileGenerator = 0;
  this->AddDefaultRules();
}

void cmMakefile::AddDefaultRules()
{
  this->AddRuleMaker(new cmAbstractFilesRule);
  this->AddRuleMaker(new cmAddTargetRule);
  this->AddRuleMaker(new cmAuxSourceDirectoryRule);
  this->AddRuleMaker(new cmExecutablesRule);
  this->AddRuleMaker(new cmFindIncludeRule);
  this->AddRuleMaker(new cmFindLibraryRule);
  this->AddRuleMaker(new cmFindProgramRule);
  this->AddRuleMaker(new cmIncludeDirectoryRule);
  this->AddRuleMaker(new cmLibraryRule);
  this->AddRuleMaker(new cmLinkDirectoriesRule);
  this->AddRuleMaker(new cmLinkLibrariesRule);
  this->AddRuleMaker(new cmProjectRule);
  this->AddRuleMaker(new cmSourceFilesRule);
  this->AddRuleMaker(new cmSourceFilesRequireRule);
  this->AddRuleMaker(new cmSubdirRule);
  this->AddRuleMaker(new cmUnixLibrariesRule);
  this->AddRuleMaker(new cmUnixDefinesRule);
  this->AddRuleMaker(new cmWin32LibrariesRule);
  this->AddRuleMaker(new cmWin32DefinesRule);
  this->AddRuleMaker(new cmTestsRule);
#ifdef _WIN32
  this->AddDefinition("WIN32", "1");
#else
  this->AddDefinition("UNIX", "1");
#endif
  // Cygwin is more like unix so enable the unix rules
#if defined(__CYGWIN__)
  this->AddDefinition("UNIX", "1");
#endif
}


cmMakefile::~cmMakefile()
{
  for(int i=0; i < m_UsedRuleMakers.size(); i++)
    {
    delete m_UsedRuleMakers[i];
    }
  for(StringRuleMakerMap::iterator j = m_RuleMakers.begin();
      j != m_RuleMakers.end(); ++j)
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
    this->ParseDirectory(m_cmCurrentDirectory.c_str());
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
      // Special rule that needs to be removed when 
      // ADD_RULE is implemented
      if(name == "VERBATIM")
        {
        if(!inheriting)
          {
          m_MakeVerbatim = arguments;
          }
        }
      else
        {
        StringRuleMakerMap::iterator pos = m_RuleMakers.find(name);
        if(pos != m_RuleMakers.end())
          {
          cmRuleMaker* rm = (*pos).second;
          cmRuleMaker* usedMaker = rm->Clone();
          usedMaker->SetMakefile(this);
          usedMaker->LoadCache();
          m_UsedRuleMakers.push_back(usedMaker);
          if(usedMaker->GetEnabled())
            {
            // if not running in inherit mode or
            // if the rule is inherited then Invoke it.
            if(!inheriting || usedMaker->IsInherited())
              {
              if(!usedMaker->Invoke(arguments))
                {
                cmSystemTools::Error(usedMaker->GetError());
                }
              }
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
  

void cmMakefile::AddRuleMaker(cmRuleMaker* wg)
{
  std::string name = wg->GetName();
  m_RuleMakers.insert( StringRuleMakerMap::value_type(name, wg));
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
  // give all the rules a chance to do something
  // after the file has been parsed before generation
  for(std::vector<cmRuleMaker*>::iterator i = m_UsedRuleMakers.begin();
      i != m_UsedRuleMakers.end(); ++i)
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



void cmMakefile::AddCustomRule(const char* source,
                               const char* result,
                               const char* command,
                               std::vector<std::string>& depends)
{
  cmMakefile::customRule rule;
  rule.m_Source = source;
  rule.m_Result = result;
  rule.m_Command = command;
  rule.m_Depends = depends;
  m_CustomRules.push_back(rule);
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
   // Now replace varibles
  std::vector<std::string>::iterator j, begin, end;
  begin = m_IncludeDirectories.begin();
  end = m_IncludeDirectories.end();
  for(j = begin; j != end; ++j)
    {
    cmSystemTools::ReplaceString(*j, "${CMAKE_BINARY_DIR}",
				 this->GetOutputHomeDirectory() );
    cmSystemTools::ReplaceString(*j, "${CMAKE_SOURCE_DIR}",
				 this->GetHomeDirectory() );
    }
  begin = m_LinkDirectories.begin();
  end = m_LinkDirectories.end();
  for(j = begin; j != end; ++j)
    {
    cmSystemTools::ReplaceString(*j, "${CMAKE_BINARY_DIR}",
				 this->GetOutputHomeDirectory() );
    cmSystemTools::ReplaceString(*j, "${CMAKE_SOURCE_DIR}",
                                 this->GetHomeDirectory() );
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
  
  // Loop over all registered rules and print out documentation
  const char *name;
  const char *terse;
  const char *full;

  for(StringRuleMakerMap::iterator j = m_RuleMakers.begin();
      j != m_RuleMakers.end(); ++j)
    {
    name = (*j).second->GetName();
    terse = (*j).second->GetTerseDocumentation();
    full = (*j).second->GetFullDocumentation();
    f << name << " - " << terse << std::endl
      << "Usage: " << full << std::endl << std::endl;
    }
  

  return 1;
}

