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
#include "cmListFileCache.h"
#include <stdio.h>  // required for sprintf

// default is not to be building executables
cmMakefile::cmMakefile()
{
  // Setup the default include file regular expression (match everything).
  m_IncludeFileRegularExpression = "^.*$";
  // Setup the default include complaint regular expression (match nothing).
  m_ComplainFileRegularExpression = "^$";
  // Source and header file extensions that we can handle
  m_SourceFileExtensions.push_back( "cxx" );
  m_SourceFileExtensions.push_back( "cpp" );
  m_SourceFileExtensions.push_back( "c" );
  m_SourceFileExtensions.push_back( "M" );
  m_SourceFileExtensions.push_back( "m" );
  m_SourceFileExtensions.push_back( "mm" );

  m_HeaderFileExtensions.push_back( "h" );
  m_HeaderFileExtensions.push_back( "txx" );
  
  m_DefineFlags = " ";
  m_MakefileGenerator = 0;
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup("Source Files", "\\.(cpp|C|c|cxx|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files", "\\.(h|hh|hpp|hxx|hm|inl)$");
  this->AddDefaultCommands();
  this->AddDefaultDefinitions();
  cmCacheManager::GetInstance()->DefineCache(this);
}

unsigned int cmMakefile::GetCacheMajorVersion()
{
  if(!cmCacheManager::GetInstance()->
     GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"))
    {
    return 0;
    }
  return atoi(cmCacheManager::GetInstance()->
              GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"));
}

unsigned int cmMakefile::GetCacheMinorVersion()
{
  if(!cmCacheManager::GetInstance()->
     GetCacheValue("Cmake_Cache_MINOR_VERSION"))
    {
    return 0;
    }
  return atoi(cmCacheManager::GetInstance()->
              GetCacheValue("CMAKE_CACHE_MINOR_VERSION"));
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
#if defined(_WIN32) || defined(__CYGWIN__)
  this->AddDefinition("WIN32", "1");
#else
  this->AddDefinition("UNIX", "1");
#endif
  // Cygwin is more like unix so enable the unix commands
#if defined(__CYGWIN__)
  this->AddDefinition("UNIX", "1");
  this->AddDefinition("CYGWIN", "1");
#endif
#if defined(__APPLE__)
  this->AddDefinition("APPLE", "1");
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
  for(DataMap::const_iterator d = m_DataMap.begin();
      d != m_DataMap.end(); ++d)
    {
    if(d->second)
      {
      delete d->second;
      }
    }
  std::set<cmFunctionBlocker *>::const_iterator pos;
  for (pos = m_FunctionBlockers.begin(); 
       pos != m_FunctionBlockers.end(); pos = m_FunctionBlockers.begin())
    {
    cmFunctionBlocker* b = *pos;
    m_FunctionBlockers.erase(*pos);
    delete b;
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
  this->PrintStringVector("m_IncludeDirectories;", m_IncludeDirectories);
  this->PrintStringVector("m_LinkDirectories", m_LinkDirectories);
  for( std::vector<cmSourceGroup>::const_iterator i = m_SourceGroups.begin();
       i != m_SourceGroups.end(); ++i)
    {
    i->Print();
    }
}

bool cmMakefile::CommandExists(const char* name) const
{
  return (m_Commands.find(name) != m_Commands.end());
}
      
void cmMakefile::ExecuteCommand(std::string &name,
                                std::vector<std::string> const& arguments)
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
      // if the command is inherited then InitialPass it.
      if(!m_Inheriting || usedCommand->IsInherited())
        {
        if(!usedCommand->InitialPass(arguments))
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
    cmSystemTools::Error("unknown CMake command:", name.c_str(), 
                         "\nReading cmake file in directory:" , 
                         m_cmCurrentDirectory.c_str());
    }
}

// Parse the given CMakeLists.txt file into a list of classes.
// Reads in current CMakeLists file and all parent CMakeLists files
// executing all inherited commands in the parents
//
//   if external is non-zero, this means that we have branched to grab some
//   commands from a remote list-file (that is, the equivalent of a
//   #include has been called).  We DO NOT look at the parents of this
//   list-file, and for all other purposes, the name of this list-file
//   is "filename" and not "external".
bool cmMakefile::ReadListFile(const char* filename, const char* external)
{
  // keep track of the current file being read
  if (filename)
    {
    if(m_cmCurrentListFile != filename)
      {
      m_cmCurrentListFile = filename;
      }
    }

  // if this is not a remote makefile
  //  (if it were, this would be called from the "filename" call,
  //   rather than the "external" call)
  if (!external)
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
    }

  // are we at the start CMakeLists file or are we processing a parent 
  // lists file
  //
  //   this might, or might not be true, irrespective if we are
  //   off looking at an external makefile.
  m_Inheriting = (m_cmCurrentDirectory != m_cmStartDirectory);
                    
  // Now read the input file
  const char *filenametoread= filename;

  if( external)
    {
    filenametoread= external;
    }

  cmListFile* lf = 
    cmListFileCache::GetInstance()->GetFileCache(filenametoread);
  if(!lf)
    {
    cmSystemTools::Error("error can not open file ", filenametoread);
    return false;
    }
  // add this list file to the list of dependencies
  m_ListFiles.push_back( filenametoread);
  const int numberFunctions = lf->m_Functions.size();
  for(int i =0; i < numberFunctions; ++i)
    {
    cmListFileFunction& curFunction = lf->m_Functions[i];
    if(!this->IsFunctionBlocked(curFunction.m_Name.c_str(),
                                curFunction.m_Arguments))
      {
      this->ExecuteCommand(curFunction.m_Name,
                           curFunction.m_Arguments);
      }
    }

  // send scope ended to and funciton blockers
  if (filename)
    {
    // loop over all function blockers to see if any block this command
    std::set<cmFunctionBlocker *>::const_iterator pos;
    for (pos = m_FunctionBlockers.begin(); 
         pos != m_FunctionBlockers.end(); ++pos)
      {
      (*pos)->ScopeEnded(*this);
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
  if(mf == m_MakefileGenerator)
  {
    return;
  }
  delete m_MakefileGenerator;
  m_MakefileGenerator = mf;
  mf->SetMakefile(this);
}

void cmMakefile::FinalPass()
{
  // do all the variable expansions here
  this->ExpandVariables();

  // give all the commands a chance to do something
  // after the file has been parsed before generation
  for(std::vector<cmCommand*>::iterator i = m_UsedCommands.begin();
      i != m_UsedCommands.end(); ++i)
    {
    (*i)->FinalPass();
    }

}

  // Generate the output file
void cmMakefile::GenerateMakefile()
{
  this->FinalPass();
  // merge libraries
  for (cmTargets::iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    l->second.GenerateSourceFilesFromSourceLists(*this);
    l->second.MergeLibraries(m_LinkLibraries);
    }
  // now do the generation
  m_MakefileGenerator->GenerateMakefile();
}


void cmMakefile::AddSource(cmSourceFile& cmfile, const char *srclist)
{
  m_Sources[srclist].push_back(cmfile);
}

struct FindSrcByName : std::binary_function<cmSourceFile, cmSourceFile, bool>
{
  public:
    bool operator () (const cmSourceFile &f, const cmSourceFile &test) const
    {
      return !strcmp(f.GetSourceName().c_str(),test.GetSourceName().c_str());
    }
};

void cmMakefile::RemoveSource(cmSourceFile& cmfile,const char *srclist)
{
  std::vector<cmSourceFile> &maplist = m_Sources[srclist];
  std::vector<cmSourceFile>::iterator f =
    std::find_if(maplist.begin(), maplist.end(), std::bind2nd(FindSrcByName(),cmfile));
//  std::vector<cmSourceFile>::iterator f = find_if(maplist.begin(), maplist.end(), matches(srclist);
  if (f!=maplist.end())
    {
      maplist.erase(f);
    }
}

void cmMakefile::AddCustomCommand(const char* source,
                                  const char* command,
                                  const std::vector<std::string>& commandArgs,
                                  const std::vector<std::string>& depends,
                                  const std::vector<std::string>& outputs,
                                  const char *target) 
{
  // find the target, 
  if (m_Targets.find(target) != m_Targets.end())
    {
    std::string expandC = command;
    this->ExpandVariablesInString(expandC);
    std::string c = cmSystemTools::EscapeSpaces(expandC.c_str());

    std::string combinedArgs;
    unsigned int i;
    
    for (i = 0; i < commandArgs.size(); ++i)
      {
      combinedArgs += cmSystemTools::EscapeSpaces(commandArgs[i].c_str());
      combinedArgs += " ";
      }
    
    cmCustomCommand cc(source,c.c_str(),combinedArgs.c_str(),depends,outputs);
    m_Targets[target].GetCustomCommands().push_back(cc);
    std::string cacheCommand = command;
    this->ExpandVariablesInString(cacheCommand);
    if(cmCacheManager::GetInstance()->GetCacheValue(cacheCommand.c_str()))
      {
      m_Targets[target].AddUtility(
        cmCacheManager::GetInstance()->GetCacheValue(cacheCommand.c_str()));
      }
    }
}

void cmMakefile::AddCustomCommand(const char* source,
                                  const char* command,
                                  const std::vector<std::string>& commandArgs,
                                  const std::vector<std::string>& depends,
                                  const char* output, 
                                  const char *target) 
{
  std::vector<std::string> outputs;
  outputs.push_back(output);
  this->AddCustomCommand(source, command, commandArgs, depends, outputs, target);
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  m_DefineFlags += " ";
  m_DefineFlags += flag;
}


void cmMakefile::AddLinkLibrary(const char* lib, cmTarget::LinkLibraryType llt)
{
  m_LinkLibraries.push_back(
	  std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt));
}

void cmMakefile::AddLinkLibraryForTarget(const char *target,
                                         const char* lib, 
                                         cmTarget::LinkLibraryType llt)
{
  if (m_Targets.find(target) != m_Targets.end())
    {
    m_Targets[target].GetLinkLibraries().
      push_back(
        std::pair<std::string, cmTarget::LinkLibraryType>(lib,llt));
    }
  else
    {
    cmSystemTools::Error("Attempt to add link libraries to non-existant target: ",  target, " for lib ", lib);
    }
}

void cmMakefile::AddLinkLibrary(const char* lib)
{
  this->AddLinkLibrary(lib,cmTarget::GENERAL);
}

void cmMakefile::AddLinkDirectory(const char* dir)
{
  // Don't add a link directory that is already present.  Yes, this
  // linear search results in n^2 behavior, but n won't be getting
  // much bigger than 20.  We cannot use a set because of order
  // dependency of the link search path.
  if(std::find(m_LinkDirectories.begin(),
               m_LinkDirectories.end(), dir) == m_LinkDirectories.end())
    {
    m_LinkDirectories.push_back(dir);
    }
}

void cmMakefile::AddSubDirectory(const char* sub)
{
  m_SubDirectories.push_back(sub);
}

void cmMakefile::AddIncludeDirectory(const char* inc, bool before)
{
  // Don't add an include directory that is already present.  Yes,
  // this linear search results in n^2 behavior, but n won't be
  // getting much bigger than 20.  We cannot use a set because of
  // order dependency of the include path.
  if(std::find(m_IncludeDirectories.begin(),
               m_IncludeDirectories.end(), inc) == m_IncludeDirectories.end())
    {
    if (before)
      {
      // WARNING: this *is* expensive (linear time) since it's a vector
      m_IncludeDirectories.insert(m_IncludeDirectories.begin(), inc);
      }
    else
      {
      m_IncludeDirectories.push_back(inc);
      }
    }
}


void cmMakefile::AddDefinition(const char* name, const char* value)
{
  m_Definitions.erase( DefinitionMap::key_type(name));
  m_Definitions.insert(DefinitionMap::value_type(name, value));
}


void cmMakefile::AddCacheDefinition(const char* name, const char* value, 
                                    const char* doc,
                                    cmCacheManager::CacheEntryType type)
{
  cmCacheManager::GetInstance()->AddCacheEntry(name, value, doc, type);
  this->AddDefinition(name, value);
}


void cmMakefile::AddDefinition(const char* name, bool value)
{
  if(value)
    {
    m_Definitions.erase( DefinitionMap::key_type(name));
    m_Definitions.insert(DefinitionMap::value_type(name, "ON"));
    }
  else
    {
    m_Definitions.erase( DefinitionMap::key_type(name));
    m_Definitions.insert(DefinitionMap::value_type(name, "OFF"));
    }
}


void cmMakefile::AddCacheDefinition(const char* name, bool value, const char* doc)
{
  cmCacheManager::GetInstance()->AddCacheEntry(name, value, doc);
  this->AddDefinition(name, value);
}

void cmMakefile::SetProjectName(const char* p)
{
  m_ProjectName = p;
}

void cmMakefile::AddLibrary(const char* lname, int shared,
                            const std::vector<std::string> &srcs)
{
  cmTarget target;
  switch (shared)
    {
    case 0:
      target.SetType(cmTarget::STATIC_LIBRARY);
      break;
    case 1:
      target.SetType(cmTarget::SHARED_LIBRARY);
      break;
    case 2:
      target.SetType(cmTarget::MODULE_LIBRARY);
      break;
    default:
      target.SetType(cmTarget::STATIC_LIBRARY);
    }

  target.SetInAll(true);
  target.GetSourceLists() = srcs;
  m_Targets.insert(cmTargets::value_type(lname,target));

  // Add an entry into the cache 
  cmCacheManager::GetInstance()->
    AddCacheEntry(lname,
                  this->GetCurrentOutputDirectory(),
                  "Path to a library", cmCacheManager::INTERNAL);

  // Add an entry into the cache
  std::string ltname = lname;
  ltname += "_LIBRARY_TYPE";
  switch (shared)
    {
    case 0:
      cmCacheManager::GetInstance()->
	AddCacheEntry(ltname.c_str(),
		      "STATIC",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    case 1:
      cmCacheManager::GetInstance()->
	AddCacheEntry(ltname.c_str(),
		      "SHARED",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    case 2:
      cmCacheManager::GetInstance()->
	AddCacheEntry(ltname.c_str(),
		      "MODULE",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    default:
      cmCacheManager::GetInstance()->
	AddCacheEntry(ltname.c_str(),
		      "STATIC",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
    }
}

void cmMakefile::AddExecutable(const char *exeName, 
                               const std::vector<std::string> &srcs)
{
  this->AddExecutable(exeName,srcs,false);
}

void cmMakefile::AddExecutable(const char *exeName, 
                               const std::vector<std::string> &srcs,
                               bool win32)
{
  cmTarget target;
  if (win32)
    {
    target.SetType(cmTarget::WIN32_EXECUTABLE);
    }
  else
    {
    target.SetType(cmTarget::EXECUTABLE);
    }
  target.SetInAll(true);
  target.GetSourceLists() = srcs;
  m_Targets.insert(cmTargets::value_type(exeName,target));
  
  // Add an entry into the cache 
  cmCacheManager::GetInstance()->
    AddCacheEntry(exeName,
                  this->GetCurrentOutputDirectory(),
                  "Path to an executable", cmCacheManager::INTERNAL);
}


void cmMakefile::AddUtilityCommand(const char* utilityName,
                                   const char* command,
                                   const char* arguments,
                                   bool all)
{
  std::vector<std::string> empty;
  this->AddUtilityCommand(utilityName,command,arguments,all,
                          empty,empty);
}

void cmMakefile::AddUtilityCommand(const char* utilityName,
                                   const char* command,
                                   const char* arguments,
                                   bool all,
                                   const std::vector<std::string> &dep,
                                   const std::vector<std::string> &out)
{
  cmTarget target;
  target.SetType(cmTarget::UTILITY);
  target.SetInAll(all);
  cmCustomCommand cc(utilityName, command, arguments, dep, out);
  target.GetCustomCommands().push_back(cc);
  m_Targets.insert(cmTargets::value_type(utilityName,target));
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
  while(!cmSystemTools::FileExists(parentFile.c_str()))
    {
    // There is no CMakeLists.txt file in the parent directory.  This
    // can occur when coming out of a subdirectory resulting from a
    // SUBDIRS(Foo/Bar) command (coming out of Bar into Foo).  Try
    // walking up until a CMakeLists.txt is found or the home
    // directory is hit.
    
    // if we are in the home directory then stop, return 0
    if(m_cmHomeDirectory == listsDir) { return ""; }
    
    // is there a parent directory we can check
    pos = listsDir.rfind('/');
    // if we could not find the directory return 0
    if(pos == std::string::npos) { return ""; }
    listsDir = listsDir.substr(0, pos);
    parentFile = listsDir;
    parentFile += "/CMakeLists.txt";
    }
    
  return parentFile;
}

// expance CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR in the
// include and library directories.

void cmMakefile::ExpandVariables()
{
  // Now expand varibles in the include and link strings
  for(std::vector<std::string>::iterator d = m_IncludeDirectories.begin();
        d != m_IncludeDirectories.end(); ++d)
    {
    this->ExpandVariablesInString(*d);
    }
  for(std::vector<std::string>::iterator d = m_LinkDirectories.begin();
        d != m_LinkDirectories.end(); ++d)
    {
    this->ExpandVariablesInString(*d);
    }
  for(cmTarget::LinkLibraries::iterator l = m_LinkLibraries.begin();
      l != m_LinkLibraries.end(); ++l)
    {
    this->ExpandVariablesInString(l->first);
    }
}

bool cmMakefile::IsOn(const char* name)
{
  const char* value = this->GetDefinition(name);
  return cmSystemTools::IsOn(value);
}

const char* cmMakefile::GetDefinition(const char* name) const
{
  DefinitionMap::const_iterator pos = m_Definitions.find(name);
  if(pos != m_Definitions.end())
    {
    return (*pos).second.c_str();
    }
  return cmCacheManager::GetInstance()->GetCacheValue(name);
}

int cmMakefile::DumpDocumentationToFile(std::ostream& f)
{
  // Open the supplied filename
  
  // Loop over all registered commands and print out documentation
  const char *name;
  const char *terse;
  const char *full;
  char tmp[1024];
  sprintf(tmp,"Version %d.%d", cmMakefile::GetMajorVersion(),
          cmMakefile::GetMinorVersion());
  f << "<html>\n";
  f << "<h1>Documentation for commands of CMake " << tmp << "</h1>\n";
  f << "<ul>\n";
  for(RegisteredCommandsMap::iterator j = m_Commands.begin();
      j != m_Commands.end(); ++j)
    {
    name = (*j).second->GetName();
    terse = (*j).second->GetTerseDocumentation();
    full = (*j).second->GetFullDocumentation();
    f << "<li><b>" << name << "</b> - " << terse << std::endl
      << "<br><i>Usage:</i> " << full << "</li>" << std::endl << std::endl;
    }
  f << "</ul></html>\n";
  return 1;
}


const char *cmMakefile::ExpandVariablesInString(std::string& source) const
{
  return this->ExpandVariablesInString(source, false);
}

const char *cmMakefile::ExpandVariablesInString(std::string& source,
                                                bool escapeQuotes,
                                                bool atOnly) const
{
  // This method replaces ${VAR} and @VAR@ where VAR is looked up
  // in the m_Definitions map, if not found in the map, nothing is expanded.
  // It also supports the $ENV{VAR} syntax where VAR is looked up in
  // the current environment variables.

  // start by look for $ or @ in the string
  std::string::size_type markerPos;
  if(atOnly)
    {
    markerPos = source.find_first_of("@");
    }
  else
    {
    markerPos = source.find_first_of("$@");
    }
  // if not found, or found as the last character, then leave quickly as
  // nothing needs to be expanded
  if((markerPos == std::string::npos) || (markerPos >= source.size()-1))
    {
    return source.c_str();
    }
  // current position
  std::string::size_type currentPos =0; // start at 0
  std::string result; // string with replacements
  // go until the the end of the string
  while((markerPos != std::string::npos) && (markerPos < source.size()-1))
    {
    // grab string from currentPos to the start of the variable
    // and add it to the result
    result += source.substr(currentPos, markerPos - currentPos);
    char endVariableMarker;     // what is the end of the variable @ or }
    int markerStartSize = 1;    // size of the start marker 1 or 2 or 5
    if(!atOnly && source[markerPos] == '$')
      {
      // ${var} case
      if(source[markerPos+1] == '{')
        {
        endVariableMarker = '}';
        markerStartSize = 2;
        }
      // $ENV{var} case
      else if(markerPos+4 < source.size() &&
              source[markerPos+4] == '{' &&
              !source.substr(markerPos+1, 3).compare("ENV"))
        {
        endVariableMarker = '}';
        markerStartSize = 5;
        }
      else
        {
        // bogus $ with no { so add $ to result and move on
        result += '$'; // add bogus $ back into string
        currentPos = markerPos+1; // move on
        endVariableMarker = ' '; // set end var to space so we can tell bogus
        }
      }
    else
      {
      // @VAR case
      endVariableMarker = '@';
      }
    // if it was a valid variable (started with @ or ${ or $ENV{ )
    if(endVariableMarker != ' ')
      {
      markerPos += markerStartSize; // move past marker
      // find the end variable marker starting at the markerPos
      std::string::size_type endVariablePos =
        source.find(endVariableMarker, markerPos);
      if(endVariablePos == std::string::npos)
        {
        // no end marker found so add the bogus start
        if(endVariableMarker == '@')
          {
          result += '@';
          }
        else
          {
          result += (markerStartSize == 5 ? "$ENV{" : "${");
          }
        currentPos = markerPos;
        }
      else
        {
        // good variable remove it
        std::string var = source.substr(markerPos, endVariablePos - markerPos);
        bool found = false;
        if (markerStartSize == 5) // $ENV{
          {
          char *ptr = getenv(var.c_str());
          if (ptr)
            {
            if (escapeQuotes)
              {
              result += cmSystemTools::EscapeQuotes(ptr);
              }
            else
              {
              result += ptr;
              }
            found = true;
            }
          }
        else
          {
          const char* lookup = this->GetDefinition(var.c_str());
          if(lookup)
            {
            if (escapeQuotes)
              {
              result += cmSystemTools::EscapeQuotes(lookup);
              }
            else
              {
              result += lookup;
              }
            found = true;
            }
          }
        // if found add to result, if not, then it gets blanked
        if (!found)
          {
          // if no definition is found then add the var back
          if(endVariableMarker == '@')
            {
            result += "@";
            result += var;
            result += "@";
            }
          else
            {
            result += (markerStartSize == 5 ? "$ENV{" : "${");
            result += var;
            result += "}";
            }
          }
        // lookup var, and replace it
        currentPos = endVariablePos+1;
        }
      }
    if(atOnly)
      {
      markerPos = source.find_first_of("@", currentPos);
      }
    else
      {
      markerPos = source.find_first_of("$@", currentPos);
      }
    }
  result += source.substr(currentPos); // pick up the rest of the string
  source = result;
  return source.c_str();
}

void cmMakefile::RemoveVariablesInString(std::string& source,
                                         bool atOnly) const
{
  if(!atOnly)
    {
    cmRegularExpression var("(\\${[A-Za-z_0-9]*})");
    while (var.find(source))
      {
      source.erase(var.start(),var.end() - var.start());
      }
    }
  
  if(!atOnly)
    {
    cmRegularExpression varb("(\\$ENV{[A-Za-z_0-9]*})");
    while (varb.find(source))
      {
      source.erase(varb.start(),varb.end() - varb.start());
      }
    }
  cmRegularExpression var2("(@[A-Za-z_0-9]*@)");
  while (var2.find(source))
    {
    source.erase(var2.start(),var2.end() - var2.start());
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


/**
 * Add the default definitions to the makefile.  These values must not
 * be dependent on anything that isn't known when this cmMakefile instance
 * is constructed.
 */
void cmMakefile::AddDefaultDefinitions()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->AddDefinition("CMAKE_CFG_INTDIR","$(IntDir)");
#else
  this->AddDefinition("CMAKE_CFG_INTDIR",".");
#endif
  char temp[1024];
  sprintf(temp, "%d", cmMakefile::GetMinorVersion());
  this->AddDefinition("CMAKE_MINOR_VERSION", temp);
  sprintf(temp, "%d", cmMakefile::GetMajorVersion());
  this->AddDefinition("CMAKE_MAJOR_VERSION", temp);
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
                                   std::vector<std::string> const&args)
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
      cmFunctionBlocker* b = *pos;
      m_FunctionBlockers.erase(*pos);
      delete b;
      return;
      }
    }
  
  return;
}

void cmMakefile::SetHomeDirectory(const char* dir) 
{
  m_cmHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(m_cmHomeDirectory);
  this->AddDefinition("CMAKE_SOURCE_DIR", this->GetHomeDirectory());
}

void cmMakefile::SetHomeOutputDirectory(const char* lib)
{
  m_HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(m_HomeOutputDirectory);
  this->AddDefinition("CMAKE_BINARY_DIR", this->GetHomeOutputDirectory());
}


/**
 * Register the given cmData instance with its own name.
 */
void cmMakefile::RegisterData(cmData* data)
{
  std::string name = data->GetName();
  DataMap::const_iterator d = m_DataMap.find(name);
  if((d != m_DataMap.end()) && (d->second != NULL) && (d->second != data))
    {
    delete d->second;
    }
  m_DataMap[name] = data;
}


/**
 * Register the given cmData instance with the given name.  This can be used
 * to register a NULL pointer.
 */
void cmMakefile::RegisterData(const char* name, cmData* data)
{
  DataMap::const_iterator d = m_DataMap.find(name);
  if((d != m_DataMap.end()) && (d->second != NULL) && (d->second != data))
    {
    delete d->second;
    }
  m_DataMap[name] = data;
}


/**
 * Lookup a cmData instance previously registered with the given name.  If
 * the instance cannot be found, return NULL.
 */
cmData* cmMakefile::LookupData(const char* name) const
{
  DataMap::const_iterator d = m_DataMap.find(name);
  if(d != m_DataMap.end())
    {
    return d->second;
    }
  else
    {
    return NULL;
    }
}

