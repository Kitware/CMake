/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmMakefile.h"
#include "cmCommand.h"
#include "cmStandardIncludes.h"
#include "cmSourceFile.h"
#include "cmDirectory.h"
#include "cmSystemTools.h"
#include "cmMakefileGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"

// default is not to be building executables
cmMakefile::cmMakefile()
{
  // Setup the default include file regular expression.
  // Should be changed to something like "\\.(h|hh|hpp|hxx)$" or "^.*$"
  m_IncludeFileRegularExpression = "^itk|^vtk|^vnl|^vcl|^f2c";
  
  m_DefineFlags = " ";
  m_MakefileGenerator = 0;
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup("Source Files", "\\.(cpp|C|c|cxx|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files", "\\.(h|hh|hpp|hxx|hm|inl)$");
  this->AddDefaultCommands();
  this->AddDefaultDefinitions();
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
  for(unsigned int i=0; i < m_UsedCommands.size(); i++)
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

void cmMakefile::PrintStringVector(const char* s, const std::vector<std::string>& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::string>::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    std::cout << (*i).c_str() << " ";
    }
  std::cout << " )\n";
}


// call print on all the classes in the makefile
void cmMakefile::Print() const
{
  // print the class lists
  std::cout << "classes:\n";
  for(SourceMap::const_iterator l = m_Sources.begin(); 
      l != m_Sources.end(); l++)
    {
    std::cout << " Class list named: " << l->first << std::endl;
    for(std::vector<cmSourceFile>::const_iterator i = l->second.begin(); 
        i != l->second.end(); i++)
      {
      i->Print();
      }
    }
  
  std::cout << " m_Targets: ";
  for (cmTargets::const_iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    std::cout << l->first << std::endl;
    }

  std::cout << " m_CurrentOutputDirectory; " << 
    m_CurrentOutputDirectory.c_str() << std::endl;
  std::cout << " m_StartOutputDirectory; " << 
    m_StartOutputDirectory.c_str() << std::endl;
  std::cout << " m_HomeOutputDirectory; " << 
    m_HomeOutputDirectory.c_str() << std::endl;
  std::cout << " m_cmCurrentDirectory; " << 
    m_cmCurrentDirectory.c_str() << std::endl;
  std::cout << " m_cmStartDirectory; " << 
    m_cmStartDirectory.c_str() << std::endl;
  std::cout << " m_cmHomeDirectory; " << 
    m_cmHomeDirectory.c_str() << std::endl;
  std::cout << " m_ProjectName;	" <<  m_ProjectName.c_str() << std::endl;
  this->PrintStringVector("m_SubDirectories ", m_SubDirectories); 
  this->PrintStringVector("m_MakeVerbatim ", m_MakeVerbatim); 
  this->PrintStringVector("m_IncludeDirectories;", m_IncludeDirectories);
  this->PrintStringVector("m_LinkDirectories", m_LinkDirectories);
  this->PrintStringVector("m_Utilities", m_Utilities);
  this->PrintStringVector("m_UtilityDirectories", m_UtilityDirectories);
}

// Parse the given CMakeLists.txt file into a list of classes.
// Reads in current CMakeLists file and all parent CMakeLists files
// executing all inherited commands in the parents
bool cmMakefile::ReadListFile(const char* filename)
{
  // is there a parent CMakeLists file that does not go beyond the
  // Home directory? if so recurse and read in that List file 
  std::string parentList = this->GetParentListFileName(filename);
  if (parentList != "")
    {
    // save the current directory
    std::string srcdir = m_cmCurrentDirectory;
    std::string bindir = m_CurrentOutputDirectory;    
    // compute the new current directories
    std::string::size_type pos = m_cmCurrentDirectory.rfind('/');
    if(pos != std::string::npos)
      {
      m_cmCurrentDirectory = m_cmCurrentDirectory.substr(0, pos);
      }
    pos = m_CurrentOutputDirectory.rfind('/');
    if(pos != std::string::npos)
      {
      m_CurrentOutputDirectory = m_CurrentOutputDirectory.substr(0, pos);
      }
    this->ReadListFile(parentList.c_str());
    // restore the current directory
    m_cmCurrentDirectory = srcdir;
    m_CurrentOutputDirectory = bindir;    
    }

  // are we at the start CMakeLists file or are we processing a parent 
  // lists file
  bool inheriting = (m_cmCurrentDirectory != m_cmStartDirectory);
                    
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
    if(cmSystemTools::ParseFunction(fin, name, arguments) &&
       !this->IsFunctionBlocked(name.c_str(),arguments))
      {
      // Special command that needs to be removed when 
      // ADD_COMMAND is implemented
      if(name == "VERBATIM")
        {
        if (!inheriting)
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
          bool keepCommand = false;
          if(usedCommand->GetEnabled())
            {
            // if not running in inherit mode or
            // if the command is inherited then Invoke it.
            if(!inheriting || usedCommand->IsInherited())
              {
              if(!usedCommand->Invoke(arguments))
                {
                cmSystemTools::Error(usedCommand->GetName(),
                                     ": Error : \n",
                                     usedCommand->GetError(),
                                     m_cmCurrentDirectory.c_str());
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
          cmSystemTools::Error("unknown CMake command ", name.c_str());
          }
        }
      }
    }
  return true;
}

  

cmSourceFile *cmMakefile::GetSource(const char *srclist, const char *cname)
{
  SourceMap::iterator sl = m_Sources.find(srclist);
  // find the src list
  if (sl == m_Sources.end())
    {
    return 0;
    }
  // find the class
  for (std::vector<cmSourceFile>::iterator i = sl->second.begin();
       i != sl->second.end(); ++i)
    {
    if (i->GetSourceName() == cname)
      {
      return &(*i);
      }
    }
  return 0;
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
  this->ExpandVariables();
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

void cmMakefile::AddSource(cmSourceFile& cmfile, const char *srclist)
{
  m_Sources[srclist].push_back(cmfile);
}

void cmMakefile::AddCustomCommand(const char* source,
                                  const char* command,
                                  const std::vector<std::string>& depends,
                                  const std::vector<std::string>& outputs,
                                  const char *target) 
{
  // find the target, 
  if (m_Targets.find(target) != m_Targets.end())
    {
    cmCustomCommand cc(source,command,depends,outputs);
    m_Targets[target].GetCustomCommands().push_back(cc);
    }
}

void cmMakefile::AddCustomCommand(const char* source,
                                  const char* command,
                                  const std::vector<std::string>& depends,
                                  const char* output, 
                                  const char *target) 
{
  std::vector<std::string> outputs;
  outputs.push_back(output);
  this->AddCustomCommand(source, command, depends, outputs, target);
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  m_DefineFlags += " ";
  m_DefineFlags += flag;
}

void cmMakefile::AddUtility(const char* util)
{
  m_Utilities.push_back(util);
}

void cmMakefile::AddUtilityDirectory(const char* dir)
{
  m_UtilityDirectories.push_back(dir);
}

void cmMakefile::AddLinkLibrary(const char* lib, LinkLibraryType llt)
{
  m_LinkLibraries.push_back(
    std::pair<std::string, LinkLibraryType>(lib,llt));
}

void cmMakefile::AddLinkLibrary(const char* lib)
{
  this->AddLinkLibrary(lib,GENERAL);
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
void cmMakefile::AddDefinition(const char* name, bool value)
{
  if(value)
    {
    m_Definitions.insert(DefinitionMap::value_type(name, "ON"));
    }
  else
    {
    m_Definitions.insert(DefinitionMap::value_type(name, "OFF"));
    }
}

void cmMakefile::SetProjectName(const char* p)
{
  m_ProjectName = p;
}

void cmMakefile::AddLibrary(const char* lname, const std::vector<std::string> &srcs)
{
  cmTarget target;
  target.SetIsALibrary(1);
  target.GetSourceLists() = srcs;
  m_Targets.insert(cmTargets::value_type(lname,target));
}

void cmMakefile::AddExecutable(const char *exeName, 
                               const std::vector<std::string> &srcs)
{
  cmTarget target;
  target.SetIsALibrary(0);
  target.GetSourceLists() = srcs;
  m_Targets.insert(cmTargets::value_type(exeName,target));
}


void cmMakefile::AddSourceGroup(const char* name, const char* regex)
{
  // First see if the group exists.  If so, replace its regular expression.
  for(std::vector<cmSourceGroup>::iterator sg = m_SourceGroups.begin();
      sg != m_SourceGroups.end(); ++sg)
    {
    std::string sgName = sg->GetName();
    if(sgName == name)
      {
      // We only want to set the regular expression.  If there are already
      // source files in the group, we don't want to remove them.
      sg->SetGroupRegex(regex);
      return;
      }
    }
  
  // The group doesn't exist.  Add it.
  m_SourceGroups.push_back(cmSourceGroup(name, regex));
}

void cmMakefile::AddExtraDirectory(const char* dir)
{
  m_AuxSourceDirectories.push_back(dir);
}


// return the file name for the parent CMakeLists file to the 
// one passed in. Zero is returned if the CMakeLists file is the
// one in the home directory or if for some reason a parent cmake lists 
// file cannot be found.
std::string cmMakefile::GetParentListFileName(const char *currentFileName)
{
  // extract the directory name
  std::string parentFile;
  std::string listsDir = currentFileName;
  std::string::size_type pos = listsDir.rfind('/');
  // if we could not find the directory return 0
  if(pos == std::string::npos)
    {
    return parentFile;
    }
  listsDir = listsDir.substr(0, pos);
  
  // if we are in the home directory then stop, return 0
  if(m_cmHomeDirectory == listsDir)
    {
    return parentFile;
    }
  
  // is there a parent directory we can check
  pos = listsDir.rfind('/');
  // if we could not find the directory return 0
  if(pos == std::string::npos)
    {
    return parentFile;
    }
  listsDir = listsDir.substr(0, pos);
  
  // is there a CMakeLists.txt file in the parent directory ?
  parentFile = listsDir;
  parentFile += "/CMakeLists.txt";
  if(!cmSystemTools::FileExists(parentFile.c_str()))
    {
    parentFile = "";
    return parentFile;
    }
  
  return parentFile;
}

// expance CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR in the
// include and library directories.

void cmMakefile::ExpandVariables()
{
  // Now expand varibles in the include and link strings
  std::vector<std::string>::iterator j, begin, end;
  begin = m_IncludeDirectories.begin();
  end = m_IncludeDirectories.end();
  for(j = begin; j != end; ++j)
    {
    this->ExpandVariablesInString(*j);
    }
  begin = m_LinkDirectories.begin();
  end = m_LinkDirectories.end();
  for(j = begin; j != end; ++j)
    {
    this->ExpandVariablesInString(*j);
    }
  LinkLibraries::iterator j2, end2;
  j2 = m_LinkLibraries.begin();
  end2 = m_LinkLibraries.end();
  for(; j2 != end2; ++j2)
    {
    this->ExpandVariablesInString(j2->first);
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


void cmMakefile::ExpandVariablesInString(std::string& source) const
{
  for(DefinitionMap::const_iterator i = m_Definitions.begin();
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


// recursive function to create a vector of cmMakefile objects
// This is done by reading the sub directory CMakeLists.txt files,
// then calling this function with the new cmMakefile object
void 
cmMakefile::FindSubDirectoryCMakeListsFiles(std::vector<cmMakefile*>&
                                            makefiles)
{ 
  // loop over all the sub directories of this makefile
  const std::vector<std::string>& subdirs = this->GetSubDirectories();
  for(std::vector<std::string>::const_iterator i = subdirs.begin();
      i != subdirs.end(); ++i)
    {
    std::string subdir = *i;
    // Create a path to the list file in the sub directory
    std::string listFile = this->GetCurrentDirectory();
    listFile += "/";
    listFile += subdir;
    listFile += "/CMakeLists.txt";
    // if there is a CMakeLists.txt file read it
    if(!cmSystemTools::FileExists(listFile.c_str()))
      {
      cmSystemTools::Error("CMakeLists.txt file missing from sub directory:",
                           listFile.c_str());
      }
    else
      {
      cmMakefile* mf = new cmMakefile;
      makefiles.push_back(mf);
      // initialize new makefile
      mf->SetHomeOutputDirectory(this->GetHomeOutputDirectory());
      mf->SetHomeDirectory(this->GetHomeDirectory());
      // add the subdir to the start output directory
      std::string outdir = this->GetStartOutputDirectory();
      outdir += "/";
      outdir += subdir;
      mf->SetStartOutputDirectory(outdir.c_str());
      // add the subdir to the start source directory
      std::string currentDir = this->GetStartDirectory();
      currentDir += "/";
      currentDir += subdir;
      mf->SetStartDirectory(currentDir.c_str());
      // Parse the CMakeLists.txt file
      currentDir += "/CMakeLists.txt";
      mf->MakeStartDirectoriesCurrent();
      mf->ReadListFile(currentDir.c_str());
      // recurse into nextDir
      mf->FindSubDirectoryCMakeListsFiles(makefiles);
      }
    }
}

      
void cmMakefile::GenerateCacheOnly()
{
  std::vector<cmMakefile*> makefiles;
  this->FindSubDirectoryCMakeListsFiles(makefiles);
  for(std::vector<cmMakefile*>::iterator i = makefiles.begin();
      i != makefiles.end(); ++i)
    {
    cmMakefile* mf = *i;
    std::string source = mf->GetHomeDirectory();
    source += "/CMake/CMakeMakefileTemplate.in";
    cmSystemTools::MakeDirectory(mf->GetStartOutputDirectory());
    std::string dest = mf->GetStartOutputDirectory();
    dest += "/Makefile";
    std::ofstream fout(dest.c_str());
    std::cout << "cmake: creating : " << dest.c_str() << "\n";
    if(!fout)
      {
      cmSystemTools::Error("Failed to open file for write " , dest.c_str());
      }
    else
      {
      if(strcmp(mf->GetHomeDirectory(), 
                mf->GetHomeOutputDirectory()) == 0)
        {
        fout << "srcdir        = .\n\n";
        }
      else
        {
        fout << "srcdir        = " <<  mf->GetStartDirectory() << "\n";
        fout << "VPATH         = " <<  mf->GetStartDirectory() << "\n";
        }
      }
    fout << "include "
         << mf->GetHomeOutputDirectory() << "/CMake/CMakeMaster.make\n";
    }
  
  for(unsigned int i =0; i < makefiles.size(); ++i)
    {
    delete makefiles[i];
    }
}


/**
 * Add the default definitions to the makefile.  These values must not
 * be dependent on anything that isn't known when this cmMakefile instance
 * is constructed.
 */
void cmMakefile::AddDefaultDefinitions()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->AddDefinition("CMAKE_CFG_OUTDIR","$(OUTDIR)");
#else
  this->AddDefinition("CMAKE_CFG_OUTDIR",".");
#endif
}

/**
 * Find a source group whose regular expression matches the filename
 * part of the given source name.  Search backward through the list of
 * source groups, and take the first matching group found.  This way
 * non-inherited SOURCE_GROUP commands will have precedence over
 * inherited ones.
 */
cmSourceGroup& 
cmMakefile::FindSourceGroup(const char* source,
                            std::vector<cmSourceGroup> &groups)
{
  std::string file = source;
  std::string::size_type pos = file.rfind('/');
  if(pos != std::string::npos)
    {
    file = file.substr(pos, file.length()-pos);
    }

  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    if(sg->Matches(file.c_str()))
      {
      return *sg;
      }
    }
  
  // Shouldn't get here, but just in case, return the default group.
  return groups.front();
}


bool cmMakefile::IsFunctionBlocked(const char *name,
                                   std::vector<std::string> &args) const
{
  // loop over all function blockers to see if any block this command
  std::set<cmFunctionBlocker *>::const_iterator pos;
  for (pos = m_FunctionBlockers.begin(); 
       pos != m_FunctionBlockers.end(); ++pos)
    {
    if ((*pos)->IsFunctionBlocked(name, args, *this))
      {
      return true;
      }
    }
  
  return false;
}

void cmMakefile::RemoveFunctionBlocker(const char *name,
								       const std::vector<std::string> &args)
{
  // loop over all function blockers to see if any block this command
  std::set<cmFunctionBlocker *>::const_iterator pos;
  for (pos = m_FunctionBlockers.begin(); 
       pos != m_FunctionBlockers.end(); ++pos)
    {
    if ((*pos)->ShouldRemove(name, args, *this))
      {
      m_FunctionBlockers.erase(*pos);
      return;
      }
    }
  
  return;
}
