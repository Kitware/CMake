/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefile.h"
#include "cmCommand.h"
#include "cmStandardIncludes.h"
#include "cmSourceFile.h"
#include "cmDirectory.h"
#include "cmSystemTools.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmake.h"
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
  m_HeaderFileExtensions.push_back( "in" );
  
  m_DefineFlags = " ";
  m_LocalGenerator = 0;
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup("Source Files", "\\.(cpp|C|c|cxx|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files", "\\.(h|hh|hpp|hxx|hm|inl)$");
  this->AddDefaultDefinitions();
}

unsigned int cmMakefile::GetCacheMajorVersion()
{
  if(!this->GetCacheManager()->GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"))
    {
    return 0;
    }
  return atoi(this->GetCacheManager()->GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"));
}

unsigned int cmMakefile::GetCacheMinorVersion()
{
  if(!this->GetCacheManager()->GetCacheValue("Cmake_Cache_MINOR_VERSION"))
    {
    return 0;
    }
  return atoi(this->GetCacheManager()->GetCacheValue("CMAKE_CACHE_MINOR_VERSION"));
}


cmMakefile::~cmMakefile()
{
  for(std::vector<cmSourceFile*>::iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    delete *i;
    }
  for(unsigned int i=0; i < m_UsedCommands.size(); i++)
    {
    delete m_UsedCommands[i];
    }
  for(DataMap::const_iterator d = m_DataMap.begin();
      d != m_DataMap.end(); ++d)
    {
    if(d->second)
      {
      delete d->second;
      }
    }
  std::list<cmFunctionBlocker *>::iterator pos;
  for (pos = m_FunctionBlockers.begin(); 
       pos != m_FunctionBlockers.end(); pos = m_FunctionBlockers.begin())
    {
    cmFunctionBlocker* b = *pos;
    m_FunctionBlockers.remove(*pos);
    delete b;
    }
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
  return m_LocalGenerator->GetGlobalGenerator()->GetCMakeInstance()->CommandExists(name);
}
      
void cmMakefile::ExecuteCommand(std::string const &name,
                                std::vector<std::string> const& arguments)
{
  // quick return if blocked
  if(this->IsFunctionBlocked(name.c_str(), arguments))
    {
    return;
    }
  // execute the command
  cmCommand *rm = 
    m_LocalGenerator->GetGlobalGenerator()->GetCMakeInstance()->GetCommand(name.c_str());
  if(rm)
    {
    cmCommand* usedCommand = rm->Clone();
    usedCommand->SetMakefile(this);
    bool keepCommand = false;
    if(usedCommand->GetEnabled())
      {
      // if not running in inherit mode or
      // if the command is inherited then InitialPass it.
      if(!m_Inheriting || usedCommand->IsInherited())
        {
        std::vector<std::string> expandedArguments;
        for(std::vector<std::string>::const_iterator i = arguments.begin();
            i != arguments.end(); ++i)
          {
          std::string tmps = *i;
          this->ExpandVariablesInString(tmps);
          if (tmps.find_first_not_of(" ") != std::string::npos)
            {
            // we found something in the args
            expandedArguments.push_back(tmps);
            }
          }
        if(!usedCommand->InitialPass(expandedArguments))
          {
	  std::string error;
	  error = usedCommand->GetName();
	  error += ": Error : \n";
	  error += usedCommand->GetError();
	  error += " from CMakeLists.txt file in directory: ";
	  error += m_cmCurrentDirectory;
          cmSystemTools::Error(error.c_str());
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
  else if((name == "CABLE_WRAP_TCL") || (name == "CABLE_CLASS_SET") ||
          (name == "CONFIGURE_GCCXML"))
    {
    cmSystemTools::Error("The command ", name.c_str(),
                         " is not implemented in this version of CMake.\n"
                         "Contact cable@public.kitware.com for more information.");
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
  // used to watch for blockers going out of scope
  // e.g. mismatched IF statement
  std::set<cmFunctionBlocker *> originalBlockers;

      // keep track of the current file being read
  if (filename)
    {
    if(m_cmCurrentListFile != filename)
      {
      m_cmCurrentListFile = filename;
      }
    // loop over current function blockers and record them
    std::list<cmFunctionBlocker *>::iterator pos;
    for (pos = m_FunctionBlockers.begin(); 
         pos != m_FunctionBlockers.end(); ++pos)
      {
      originalBlockers.insert(*pos);
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
	  std::string srcdir = m_cmCurrentDirectory;
          std::string bindir = m_CurrentOutputDirectory;

	  std::string::size_type pos = parentList.rfind('/');

          m_cmCurrentDirectory = parentList.substr(0, pos);
	  m_CurrentOutputDirectory = m_HomeOutputDirectory + parentList.substr(m_cmHomeDirectory.size(), pos - m_cmHomeDirectory.size());

	  // if not found, oops
	  if(pos == std::string::npos)
	    {
            cmSystemTools::Error("Trailing slash not found");
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
    return false;
    }
  // add this list file to the list of dependencies
  m_ListFiles.push_back( filenametoread);
  const size_t numberFunctions = lf->m_Functions.size();
  for(size_t i =0; i < numberFunctions; ++i)
    {
    cmListFileFunction& curFunction = lf->m_Functions[i];
    this->ExecuteCommand(curFunction.m_Name,
                         curFunction.m_Arguments);
    }

  // send scope ended to and funciton blockers
  if (filename)
    {
    // loop over all function blockers to see if any block this command
    std::list<cmFunctionBlocker *>::iterator pos;
    for (pos = m_FunctionBlockers.begin(); 
         pos != m_FunctionBlockers.end(); ++pos)
      {
      // if this blocker was not in the original then send a 
      // scope ended message
      if (originalBlockers.find(*pos) == originalBlockers.end())
        {
        (*pos)->ScopeEnded(*this);
        }
      }
    }
  
  return true;
}


void cmMakefile::AddCommand(cmCommand* wg)
{
  m_LocalGenerator->GetGlobalGenerator()->GetCMakeInstance()->AddCommand(wg);
}

  // Set the make file 
void cmMakefile::SetLocalGenerator(cmLocalGenerator* lg)
{
  m_LocalGenerator = lg;
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
void cmMakefile::ConfigureFinalPass()
{
  this->FinalPass();
  const char* versionValue
    = this->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  bool oldVersion = (!versionValue || atof(versionValue) < 1.4);
  // merge libraries
  
  for (cmTargets::iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    l->second.GenerateSourceFilesFromSourceLists(*this);
    // pick up any LINK_LIBRARIES that were added after the target
    if(oldVersion)
      {
      this->AddGlobalLinkInformation(l->first.c_str(), l->second);
      }
    l->second.AnalyzeLibDependencies(*this);
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
    if(this->GetCacheManager()->GetCacheValue(cacheCommand.c_str()))
      {
      m_Targets[target].AddUtility(
        this->GetCacheManager()->GetCacheValue(cacheCommand.c_str()));
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
  cmTargets::iterator i = m_Targets.find(target);
  if ( i != m_Targets.end())
    {
    i->second.AddLinkLibrary( *this, target, lib, llt );
    }
  else
    {
    cmSystemTools::Error("Attempt to add link libraries to non-existant target: ",  target, " for lib ", lib);
    }
}

void cmMakefile::AddLinkDirectoryForTarget(const char *target,
                                           const char* d)
{
  cmTargets::iterator i = m_Targets.find(target);
  if ( i != m_Targets.end())
    {
    i->second.AddLinkDirectory( d );
    }
  else
    {
    cmSystemTools::Error("Attempt to add link directories to non-existant target: ", 
                         target, " for directory ", d);
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
  
  // remove trailing slashes
  if(dir && dir[strlen(dir)-1] == '/')
    {
    std::string newdir = dir;
    newdir = newdir.substr(0, newdir.size()-1);
    if(std::find(m_LinkDirectories.begin(),
                 m_LinkDirectories.end(), newdir.c_str()) == m_LinkDirectories.end())
      {
      m_LinkDirectories.push_back(newdir);
      }
    }
  else
    {
    if(std::find(m_LinkDirectories.begin(),
                 m_LinkDirectories.end(), dir) == m_LinkDirectories.end())
      {
      m_LinkDirectories.push_back(dir);
      }
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
  if (!value )
    {
    return;
    }
  m_Definitions.erase( DefinitionMap::key_type(name));
  m_Definitions.insert(DefinitionMap::value_type(name, value));
}


void cmMakefile::AddCacheDefinition(const char* name, const char* value, 
                                    const char* doc,
                                    cmCacheManager::CacheEntryType type)
{
  this->GetCacheManager()->AddCacheEntry(name, value, doc, type);
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
  this->GetCacheManager()->AddCacheEntry(name, value, doc);
  this->AddDefinition(name, value);
}

void cmMakefile::SetProjectName(const char* p)
{
  m_ProjectName = p;
}


void cmMakefile::AddGlobalLinkInformation(const char* name, cmTarget& target)
{
  // for these targets do not add anything
  switch(target.GetType())
    {
    case cmTarget::UTILITY: return;
    case cmTarget::INSTALL_FILES: return;
    case cmTarget::INSTALL_PROGRAMS: return;
    default:;
    }
  std::vector<std::string>::iterator j;
  for(j = m_LinkDirectories.begin();
      j != m_LinkDirectories.end(); ++j)
    {
    target.AddLinkDirectory(j->c_str());
    }
  target.MergeLinkLibraries( *this, name, m_LinkLibraries );
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
  // Clear its dependencies. Otherwise, dependencies might persist
  // over changes in CMakeLists.txt, making the information stale and
  // hence useless.
  std::string depname = lname;
  depname += "_LIB_DEPENDS";
  this->GetCacheManager()->
    AddCacheEntry(depname.c_str(), "",
                  "Dependencies for target", cmCacheManager::STATIC);

  
  target.SetInAll(true);
  target.GetSourceLists() = srcs;
  this->AddGlobalLinkInformation(lname, target);
  m_Targets.insert(cmTargets::value_type(lname,target));

  // Add an entry into the cache 
  std::string libPath = lname;
  libPath += "_CMAKE_PATH";
  this->GetCacheManager()->
    AddCacheEntry(libPath.c_str(),
                  this->GetCurrentOutputDirectory(),
                  "Path to a library", cmCacheManager::INTERNAL);

  // Add an entry into the cache
  std::string ltname = lname;
  ltname += "_LIBRARY_TYPE";
  switch (shared)
    {
    case 0:
      this->GetCacheManager()->AddCacheEntry(ltname.c_str(),"STATIC",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    case 1:
      this->GetCacheManager()->
	AddCacheEntry(ltname.c_str(),
		      "SHARED",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    case 2:
      this->GetCacheManager()->
	AddCacheEntry(ltname.c_str(),
		      "MODULE",
		      "Whether a library is static, shared or module.",
		      cmCacheManager::INTERNAL);
      break;
    default:
      this->GetCacheManager()->
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
  this->AddGlobalLinkInformation(exeName, target);
  m_Targets.insert(cmTargets::value_type(exeName,target));
  
  
  // Add an entry into the cache 
  std::string exePath = exeName;
  exePath += "_CMAKE_PATH";
  this->GetCacheManager()->
    AddCacheEntry(exePath.c_str(),
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

bool cmMakefile::IsOn(const char* name) const
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
  return this->GetCacheManager()->GetCacheValue(name);
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
  // with GetDefinition(), if not found in the map, nothing is expanded.
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
          // do nothing, we remove the variable
/*          else
            {
            result += (markerStartSize == 5 ? "$ENV{" : "${");
            result += var;
            result += "}";
            }
*/
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

/**
 * Add the default definitions to the makefile.  These values must not
 * be dependent on anything that isn't known when this cmMakefile instance
 * is constructed.
 */
void cmMakefile::AddDefaultDefinitions()
{
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
  // if there are no blockers get out of here
  if (m_FunctionBlockers.begin() == m_FunctionBlockers.end())
    {
    return false;
    }

  // loop over all function blockers to see if any block this command
  std::vector<std::string> expandedArguments;
  for(std::vector<std::string>::const_iterator i = args.begin();
      i != args.end(); ++i)
    {
    std::string tmps = *i;
    this->ExpandVariablesInString(tmps);
    if (tmps.find_first_not_of(" ") != std::string::npos)
      {
      // we found something in the args
      expandedArguments.push_back(tmps);
      }
    }
  // evaluate in reverse, this is critical for balanced IF statements etc
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = m_FunctionBlockers.rbegin(); 
       pos != m_FunctionBlockers.rend(); ++pos)
    {
    if ((*pos)->NeedExpandedVariables()) 
      {
      if ((*pos)->IsFunctionBlocked(name, expandedArguments, *this))
        {
        return true;
        }
      }
    else
      {
      if ((*pos)->IsFunctionBlocked(name, args, *this))
        {
        return true;
        }
      }
    }
  
  return false;
}

void cmMakefile::RemoveFunctionBlocker(const char *name,
				       const std::vector<std::string> &args)
{
  // loop over all function blockers to see if any block this command
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = m_FunctionBlockers.rbegin(); 
       pos != m_FunctionBlockers.rend(); ++pos)
    {
    if ((*pos)->ShouldRemove(name, args, *this))
      {
      cmFunctionBlocker* b = *pos;
      m_FunctionBlockers.remove(*pos);
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
  if((d != m_DataMap.end()) && (d->second != 0) && (d->second != data))
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
  if((d != m_DataMap.end()) && (d->second != 0) && (d->second != data))
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
    return 0;
    }
}

cmSourceFile* cmMakefile::GetSource(const char* sourceName) const
{
  std::string s = cmSystemTools::GetFilenameName(sourceName);
  std::string ext;
  std::string::size_type pos = s.rfind('.');
  if(pos != std::string::npos)
    {
    ext = s.substr(pos+1, s.size() - pos-1);
    s = s.substr(0, pos);
    }
  for(std::vector<cmSourceFile*>::const_iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    if ((*i)->GetSourceName() == s)
      {
      if ((ext.size() == 0 || (ext == (*i)->GetSourceExtension())))
        {
        return *i;
        }
      }
    }
  return 0;
}



cmSourceFile* cmMakefile::AddSource(cmSourceFile const&sf)
{
  // check to see if it exists
  cmSourceFile* ret = this->GetSource(sf.GetSourceName().c_str());
  if(ret && ret->GetSourceExtension() == sf.GetSourceExtension())
    {
    return ret;
    }
  ret = new cmSourceFile(sf);
  m_SourceFiles.push_back(ret);
  return ret;
}

  
void cmMakefile::EnableLanguage(const char* lang)
{
  m_LocalGenerator->GetGlobalGenerator()->EnableLanguage(lang, this);
}

void cmMakefile::ExpandSourceListArguments(
  std::vector<std::string> const& arguments, 
  std::vector<std::string>& newargs, unsigned int start)
{
  // first figure out if we need to handle version 1.2 style source lists
  int oldVersion = 1;
  const char* versionValue
    = this->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    oldVersion = 0;
    }
  
  // now expand the args
  std::vector<std::string> tmpArgs;
  unsigned int i;
  for(i = 0; i < arguments.size(); ++i)
    {
    // is the arg defined ?, if so use the def
    const char *def = this->GetDefinition(arguments[i].c_str());
    if (def && oldVersion && i >= start)
      {
      tmpArgs.push_back(def);
      }
    else
      {
      tmpArgs.push_back(arguments[i]);
      }
    }
  cmSystemTools::ExpandListArguments(tmpArgs, newargs);
}

int cmMakefile::TryCompile(const char *srcdir, const char *bindir, 
                           const char *projectName, const char *targetName)
{
  // does the binary directory exist ? If not create it...
  if (!cmSystemTools::FileIsDirectory(bindir))
    {
    cmSystemTools::MakeDirectory(bindir);
    }
  
  // change to the tests directory and run cmake
  // use the cmake object instead of calling cmake
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  
  // make sure the same generator is used
  // use this program as the cmake to be run, it should not
  // be run that way but the cmake object requires a vailid path
  std::string cmakeCommand = this->GetDefinition("CMAKE_COMMAND");
  cmake cm;
  cm.AddCMakePaths(cmakeCommand.c_str());
  cm.SetIsInTryCompile(true);
  cmGlobalGenerator *gg = 
    cm.CreateGlobalGenerator(m_LocalGenerator->GetGlobalGenerator()->GetName());
  if (!gg)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile bad GlobalGenerator");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cm.SetGlobalGenerator(gg);
  
  // do a configure
  cm.SetHomeDirectory(srcdir);
  cm.SetHomeOutputDirectory(bindir);
  cm.SetStartDirectory(srcdir);
  cm.SetStartOutputDirectory(bindir);
  
  // to save time we pass the EnableLanguage info directly
  gg->EnableLanguagesFromGenerator(m_LocalGenerator->GetGlobalGenerator(),
                                   this);
  
  if (cm.Configure(cmakeCommand.c_str()) != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile configure of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  if (cm.Generate() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile generation of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }

  // finally call the generator to actually build the resulting project
  int ret = 
    m_LocalGenerator->GetGlobalGenerator()->TryCompile(srcdir,bindir,
                                                       projectName, 
                                                       targetName);

  cmSystemTools::ChangeDirectory(cwd.c_str());
  return ret;
}

cmCacheManager *cmMakefile::GetCacheManager() const
{
  return m_LocalGenerator->GetGlobalGenerator()->GetCMakeInstance()->GetCacheManager();
}
