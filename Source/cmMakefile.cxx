/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefile.h"
#include "cmCommand.h"
#include "cmStandardIncludes.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmVariableWatch.h"
#include "cmake.h"
#include <stdio.h>  // required for sprintf
#include <stdlib.h> // required for atoi

#include <cmsys/RegularExpression.hxx>

// default is not to be building executables
cmMakefile::cmMakefile()
{
  // Setup the default include file regular expression (match everything).
  m_IncludeFileRegularExpression = "^.*$";
  // Setup the default include complaint regular expression (match nothing).
  m_ComplainFileRegularExpression = "^$";
  // Source and header file extensions that we can handle

  // The "c" extension MUST precede the "C" extension.
  m_SourceFileExtensions.push_back( "c" );
  m_SourceFileExtensions.push_back( "C" );
  
  m_SourceFileExtensions.push_back( "c++" );
  m_SourceFileExtensions.push_back( "cc" );
  m_SourceFileExtensions.push_back( "cpp" );
  m_SourceFileExtensions.push_back( "cxx" );
  m_SourceFileExtensions.push_back( "m" );
  m_SourceFileExtensions.push_back( "M" ); 
  m_SourceFileExtensions.push_back( "mm" );

  m_HeaderFileExtensions.push_back( "h" );
  m_HeaderFileExtensions.push_back( "h++" );
  m_HeaderFileExtensions.push_back( "hm" );
  m_HeaderFileExtensions.push_back( "hpp" );
  m_HeaderFileExtensions.push_back( "hxx" );
  m_HeaderFileExtensions.push_back( "in" );
  m_HeaderFileExtensions.push_back( "txx" );
  
  m_DefineFlags = " ";
  m_LocalGenerator = 0;
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup("Source Files", 
                       "\\.(C|M|c|c\\+\\+|cc|cpp|cxx|m|mm|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files", "\\.(h|h\\+\\+|hm|hpp|hxx|in|txx|inl)$");
  this->AddSourceGroup("CMake Rules", "\\.rule$");
  this->AddDefaultDefinitions();
}

const char* cmMakefile::GetReleaseVersion()
{
#if CMake_VERSION_MINOR & 1
  return "development";
#else
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH);
#endif
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
       pos != m_FunctionBlockers.end(); ++pos)
    {
    cmFunctionBlocker* b = *pos;
    delete b;
    }
  m_FunctionBlockers.clear();
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
  std::cout << " m_ProjectName; " <<  m_ProjectName.c_str() << std::endl;
  this->PrintStringVector("m_SubDirectories ", m_SubDirectories); 
  this->PrintStringVector("m_IncludeDirectories;", m_IncludeDirectories);
  this->PrintStringVector("m_LinkDirectories", m_LinkDirectories);
  for( std::vector<cmSourceGroup>::const_iterator i = m_SourceGroups.begin();
       i != m_SourceGroups.end(); ++i)
    {
    std::cout << "Source Group: " << i->GetName() << std::endl;
    }
}

bool cmMakefile::CommandExists(const char* name) const
{
  return this->GetCMakeInstance()->CommandExists(name);
}

bool cmMakefile::ExecuteCommand(const cmListFileFunction& lff)
{
  bool result = true;
  // quick return if blocked
  if(this->IsFunctionBlocked(lff))
    {
    // No error.
    return result;
    }
  std::string name = lff.m_Name;
  // execute the command
  cmCommand *rm = 
    this->GetCMakeInstance()->GetCommand(name.c_str());
  if(rm)
    {
    const char* versionValue
      = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
    int major = 0;
    int minor = 0;
    if ( versionValue )
      {
      sscanf(versionValue, "%d.%d", &major, &minor);
      }
    if ( rm->IsDeprecated(major, minor) )
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
        << lff.m_FilePath << ":" << lff.m_Line << ":\n"
        << rm->GetError();
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    cmCommand* usedCommand = rm->Clone();
    usedCommand->SetMakefile(this);
    bool keepCommand = false;
    if(usedCommand->GetEnabled() && !cmSystemTools::GetFatalErrorOccured())
      {
      // if not running in inherit mode or
      // if the command is inherited then InitialPass it.
      if(!m_Inheriting || usedCommand->IsInherited())
        {
        if(!usedCommand->InvokeInitialPass(lff.m_Arguments))
          {
          cmOStringStream error;
          error << "Error in cmake code at\n"
            << lff.m_FilePath << ":" << lff.m_Line << ":\n"
            << usedCommand->GetError();
          cmSystemTools::Error(error.str().c_str());
          result = false;
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
    if(!cmSystemTools::GetFatalErrorOccured())
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << lff.m_FilePath << ":" << lff.m_Line << ":\n"
            << "Unknown CMake command \"" << lff.m_Name.c_str() << "\".";
      cmSystemTools::Error(error.str().c_str());
      result = false;
      }
    }
  
  return result;
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
bool cmMakefile::ReadListFile(const char* filename_in, const char* external_in)
{
  // used to watch for blockers going out of scope
  // e.g. mismatched IF statement
  std::set<cmFunctionBlocker *> originalBlockers;


  const char* external = 0;
  std::string external_abs;

  const char* filename = filename_in;
  std::string filename_abs;

  if (external_in)
    {
    external_abs =
      cmSystemTools::CollapseFullPath(external_in,
                                      m_cmCurrentDirectory.c_str());
    external = external_abs.c_str();
    if (filename_in)
      {
      filename_abs =
        cmSystemTools::CollapseFullPath(filename_in,
                                        m_cmCurrentDirectory.c_str());
      filename = filename_abs.c_str();
      }
    }
  
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
          std::string srcdir = this->GetCurrentDirectory();
          std::string bindir = this->GetCurrentOutputDirectory();

          std::string::size_type pos = parentList.rfind('/');

          this->SetCurrentDirectory(parentList.substr(0, pos).c_str());
          this->SetCurrentOutputDirectory((m_HomeOutputDirectory +
                                           parentList.substr(m_cmHomeDirectory.size(),
                                                             pos - m_cmHomeDirectory.size())).c_str());

          // if not found, oops
          if(pos == std::string::npos)
            {
            cmSystemTools::Error("Trailing slash not found");
            }

          this->ReadListFile(parentList.c_str());

          // restore the current directory
          this->SetCurrentDirectory(srcdir.c_str());
          this->SetCurrentOutputDirectory(bindir.c_str());
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
  // try to see if the list file is the top most
  // list file for a project, and if it is, then it
  // must have a project command.   If there is not
  // one, then cmake will provide one via the 
  // cmListFileCache class.
  bool requireProjectCommand = false;
  if(!external && m_cmCurrentDirectory == m_cmHomeDirectory)
    {
    if(cmSystemTools::LowerCase(
         cmSystemTools::GetFilenameName(filename)) == "cmakelists.txt")
      {
      requireProjectCommand = true;
      }
    }
      
  cmListFile* lf = 
    cmListFileCache::GetInstance()->GetFileCache(filenametoread,
                                                 requireProjectCommand);
  if(!lf)
    {
    return false;
    }
  // add this list file to the list of dependencies
  m_ListFiles.push_back( filenametoread);
  const size_t numberFunctions = lf->m_Functions.size();
  for(size_t i =0; i < numberFunctions; ++i)
    {
    this->ExecuteCommand(lf->m_Functions[i]);
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
  this->GetCMakeInstance()->AddCommand(wg);
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


// this is the old style signature, we convert to new style
void cmMakefile::AddCustomCommand(const char* source,
                                  const char* command,
                                  const std::vector<std::string>& commandArgs,
                                  const std::vector<std::string>& depends,
                                  const std::vector<std::string>& outputs,
                                  const char *target,
                                  const char *comment) 
{
  if (strcmp(source,target))
    {
    // what a pain, for backwards compatibility we will try to
    // convert this to an output based rule... so for each output..
    for(std::vector<std::string>::const_iterator d = outputs.begin();
        d != outputs.end(); ++d)
      {
      // if this looks like a real file then use is as the main depend
      cmsys::RegularExpression SourceFiles("\\.(C|M|c|c\\+\\+|cc|cpp|cxx|m|mm|rc|def|r|odl|idl|hpj|bat|h|h\\+\\+|hm|hpp|hxx|in|txx|inl)$");
      if (SourceFiles.find(source))
        {
        this->AddCustomCommandToOutput(d->c_str(), command, commandArgs, 
                                       source, depends, comment);
        }
      // otherwise do not use a main depend
      else
        {
        std::vector<std::string> depends2 = depends;
        depends2.push_back(source);
        this->AddCustomCommandToOutput(d->c_str(), command, commandArgs, 
                                       0, depends2, comment);
        }
      
      // add the output to the target?
      std::string sname = *d;
      sname += ".rule";
      this->ExpandVariablesInString(sname);
      // if the rule was added to the source, 
      // then add the source to the target
      if (!this->GetSource(sname.c_str()))
        {
        if (m_Targets.find(target) != m_Targets.end())
          {
          m_Targets[target].GetSourceLists().push_back(source);
          }
        else
          {
          cmSystemTools::Error("Attempt to add a custom rule to a target that does not exist yet for target ", target);
          return;
          }
        }
      }
    }
  else
    {
    this->AddCustomCommandToTarget(target, command, commandArgs, 
                                   cmTarget::POST_BUILD, comment, depends);
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
  this->AddCustomCommand(source, command, commandArgs, depends, 
                         outputs, target);
}

void cmMakefile::
AddCustomCommandToOutput(const char* outputIn,
                         const char* inCommand,
                         const std::vector<std::string>& commandArgs,
                         const char *main_dependency,
                         const std::vector<std::string>& depends,
                         const char *comment,
                         bool replace)
{
  std::string expandC;
  std::string combinedArgs;
  std::string command = inCommand;
  
  // process the command's string
  this->ExpandVariablesInString(command);
  command = cmSystemTools::EscapeSpaces(command.c_str());  
  
  unsigned int i;
  for (i = 0; i < commandArgs.size(); ++i)
    {
    expandC = commandArgs[i].c_str();
    this->ExpandVariablesInString(expandC);
    combinedArgs += cmSystemTools::EscapeSpaces(expandC.c_str());
    combinedArgs += " ";
    }
  cmSourceFile *file = 0;

  // setup the output name and make sure we expand any variables
  std::string output = outputIn;
  this->ExpandVariablesInString(output);  
  std::string outName = output;
  outName += ".rule";

  // setup the main dependency name and expand vars of course
  std::string mainDepend;
  if (main_dependency && main_dependency[0] != '\0')
    {
    mainDepend = main_dependency;
    this->ExpandVariablesInString(mainDepend);
    }

  // OK this rule will be placed on a generated output file unless the main
  // depednency was specified.
  if (main_dependency && main_dependency[0] != '\0')
    {
    file = this->GetSource(mainDepend.c_str());
    if (file && file->GetCustomCommand() && !replace)
      {  
      cmCustomCommand* cc = file->GetCustomCommand();
      // if the command and args are the same
      // as the command already there, then silently skip
      // this add command
      if(cc->IsEquivalent(command.c_str(), combinedArgs.c_str()))
        {
        return;
        }
      // generate a source instead
      file = 0;
      }
    else
      {
      file = this->GetOrCreateSource(mainDepend.c_str());
      }
    }

  if (!file)
    {
    file = this->GetSource(outName.c_str());
    if (file && file->GetCustomCommand() && !replace)
      {
      cmCustomCommand* cc = file->GetCustomCommand();
      // if the command and args are the same
      // as the command already there, then silently skip
      // this add command
      if(cc->IsEquivalent(command.c_str(), combinedArgs.c_str()))
        {
        return;
        }
      // produce error if two different commands are given to produce
      // the same output
      cmSystemTools::Error("Attempt to add a custom rule to an output that already"
                           " has a custom rule. For output: ",  outputIn);
      return;
      }
    // create a cmSourceFile for the output
    file = this->GetOrCreateSource(outName.c_str(), true);
    // always mark as generated
    file->SetProperty("GENERATED","1");
    }
  
  // always create the output and mark it generated
  cmSourceFile *out = this->GetOrCreateSource(output.c_str(), true);
  out->SetProperty("GENERATED","1");
  
  std::vector<std::string> depends2(depends);
  if (main_dependency && main_dependency[0] != '\0')
    {
    depends2.push_back(mainDepend.c_str());
    }
  cmCustomCommand *cc = 
    new cmCustomCommand(command.c_str(),combinedArgs.c_str(),depends2, 
                        output.c_str());
  if ( comment && comment[0] )
    {
    cc->SetComment(comment);
    }
  if (file->GetCustomCommand())
    {
    delete file->GetCustomCommand();
    }
  file->SetCustomCommand(cc);
}

void cmMakefile::
AddCustomCommandToTarget(const char* target, const char* command,
                         const std::vector<std::string>& commandArgs,
                         cmTarget::CustomCommandType type,
                         const char *comment) 
{
  std::vector<std::string> empty;
  this->AddCustomCommandToTarget(target,command,commandArgs,type,
                                 comment, empty);
}

void cmMakefile::
AddCustomCommandToTarget(const char* target, const char* command,
                         const std::vector<std::string>& commandArgs,
                         cmTarget::CustomCommandType type,
                         const char *comment,  
                         const std::vector<std::string>& depends)
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
      expandC = commandArgs[i].c_str();
      this->ExpandVariablesInString(expandC);
      combinedArgs += cmSystemTools::EscapeSpaces(expandC.c_str());
      combinedArgs += " ";
      }
    
    cmCustomCommand cc(c.c_str(),combinedArgs.c_str(),depends,0);
    if ( comment && comment[0] )
      {
      cc.SetComment(comment);
      }
    switch (type)
      {
      case cmTarget::PRE_BUILD:
        m_Targets[target].GetPreBuildCommands().push_back(cc);
        break;
      case cmTarget::PRE_LINK:
        m_Targets[target].GetPreLinkCommands().push_back(cc);
        break;
      case cmTarget::POST_BUILD:
        m_Targets[target].GetPostBuildCommands().push_back(cc);
        break;
      }
    std::string cacheCommand = command;
    this->ExpandVariablesInString(cacheCommand);
    if(this->GetCacheManager()->GetCacheValue(cacheCommand.c_str()))
      {
      m_Targets[target].AddUtility(
        this->GetCacheManager()->GetCacheValue(cacheCommand.c_str()));
      }
    }
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
  // make sure it isn't already there
  if (std::find(m_SubDirectories.begin(),
                m_SubDirectories.end(), sub) == m_SubDirectories.end())  
    {
    m_SubDirectories.push_back(sub);
    }
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
  m_TemporaryDefinitionKey = name;
  m_Definitions[m_TemporaryDefinitionKey] = value;
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(m_TemporaryDefinitionKey, 
                         cmVariableWatch::VARIABLE_MODIFIED_ACCESS);
    }
}


void cmMakefile::AddCacheDefinition(const char* name, const char* value, 
                                    const char* doc,
                                    cmCacheManager::CacheEntryType type)
{
  const char* val = value;
  cmCacheManager::CacheIterator it = 
    this->GetCacheManager()->GetCacheIterator(name);
  if(!it.IsAtEnd() && (it.GetType() == cmCacheManager::UNINITIALIZED) &&
    it.Initialized())
    {
    val = it.GetValue();
    }
  this->GetCacheManager()->AddCacheEntry(name, val, doc, type);
  this->AddDefinition(name, val);
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
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_MODIFIED_ACCESS);
    }
}


void cmMakefile::AddCacheDefinition(const char* name, bool value, const char* doc)
{
  this->GetCacheManager()->AddCacheEntry(name, value, doc);
  this->AddDefinition(name, value);
}

void cmMakefile::RemoveDefinition(const char* name)
{
  m_Definitions.erase(DefinitionMap::key_type(name));
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS);
    }
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
    case cmTarget::UTILITY: 
    case cmTarget::INSTALL_FILES: 
    case cmTarget::INSTALL_PROGRAMS: 
      return;
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
  target.ClearDependencyInformation( *this, lname );
  
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
                                   bool all,
                                   const std::vector<std::string> &depends)
{
  std::vector<std::string> empty;
  this->AddUtilityCommand(utilityName,command,arguments,all,
                          depends, empty);
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
  if (out.size() > 1)
    {
    cmSystemTools::Error(
      "Utility targets can only have one output. For utilityNamed: ",
      utilityName);
    return;
    }
  if (out.size())
    {
    cmCustomCommand cc(command, arguments, dep, out[0].c_str());
    target.GetPostBuildCommands().push_back(cc);
    }
  else
    {
    cmCustomCommand cc(command, arguments, dep, (const char *)0);
    target.GetPostBuildCommands().push_back(cc);
    }
  m_Targets.insert(cmTargets::value_type(utilityName,target));
}

cmSourceFile *cmMakefile::GetSourceFileWithOutput(const char *cname)
{
  std::string name = cname;

  // look through all the source files that have custom commands
  // and see if the custom command has the passed source file as an output
  // keep in mind the possible .rule extension that may be tacked on
  for(std::vector<cmSourceFile*>::const_iterator i = m_SourceFiles.begin(); 
      i != m_SourceFiles.end(); ++i)
    {
    // does this source file have a custom command?
    if ((*i)->GetCustomCommand())
      {
      // is the output of the custom command match the source files name
      const std::string &out = (*i)->GetCustomCommand()->GetOutput();
      if (out.rfind(name) != out.npos &&
          out.rfind(name) == out.size() - name.size())
        {
        return *i;
        }
      }
    }
  
  // otherwise return NULL
  return 0;
}


cmSourceGroup* cmMakefile::GetSourceGroup(const char* name)
{
  // First see if the group exists.  If so, replace its regular expression.
  for(std::vector<cmSourceGroup>::iterator sg = m_SourceGroups.begin();
      sg != m_SourceGroups.end(); ++sg)
    {
    std::string sgName = sg->GetName();
    if(sgName == name)
      {
      return &(*sg);
      }
    }
  return 0;
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
      if ( regex )
        {
        // We only want to set the regular expression.  If there are already
        // source files in the group, we don't want to remove them.
        sg->SetGroupRegex(regex);
        }
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
  // Now expand variables in the include and link strings
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

void cmMakefile::ExpandVariablesInCustomCommands()
{
  // do source files
  for(std::vector<cmSourceFile*>::iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    cmCustomCommand *cc = (*i)->GetCustomCommand();
    if (cc)
      {
      cc->ExpandVariables(*this);
      }
    }
  
  // now do targets
  std::vector<cmCustomCommand>::iterator ic;
  for (cmTargets::iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    for (ic = l->second.GetPreBuildCommands().begin();
         ic != l->second.GetPreBuildCommands().end(); ++ic)
      {
      ic->ExpandVariables(*this);
      }
    for (ic = l->second.GetPreLinkCommands().begin();
         ic != l->second.GetPreLinkCommands().end(); ++ic)
      {
      ic->ExpandVariables(*this);
      }
    for (ic = l->second.GetPostBuildCommands().begin();
         ic != l->second.GetPostBuildCommands().end(); ++ic)
      {
      ic->ExpandVariables(*this);
      }
    }
}

bool cmMakefile::IsOn(const char* name) const
{
  const char* value = this->GetDefinition(name);
  return cmSystemTools::IsOn(value);
}

const char* cmMakefile::GetDefinition(const char* name) const
{
  const char* def = 0;
  DefinitionMap::const_iterator pos = m_Definitions.find(name);
  if(pos != m_Definitions.end())
    {
    def = (*pos).second.c_str();
    }
  else
    {
    def = this->GetCacheManager()->GetCacheValue(name);
    }
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    if ( def )
      {
      vv->VariableAccessed(name, cmVariableWatch::VARIABLE_READ_ACCESS);
      }
    else 
      {
      // are unknown access allowed
      DefinitionMap::const_iterator pos2 = 
        m_Definitions.find("CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS");
      if (pos2 != m_Definitions.end() && 
          cmSystemTools::IsOn((*pos2).second.c_str())) 
        {
        vv->VariableAccessed(name, 
                             cmVariableWatch::ALLOWED_UNKNOWN_VARIABLE_READ_ACCESS);
        }
      else
        {
        vv->VariableAccessed(name, cmVariableWatch::
                             UNKNOWN_VARIABLE_READ_ACCESS);
        }
      }
    }
  return def;
}

std::vector<std::string> cmMakefile::GetDefinitions(int cacheonly /* = 0 */) const
{
  std::map<std::string, int> definitions;
  if ( !cacheonly )
    {
    DefinitionMap::const_iterator it;
    for ( it = m_Definitions.begin(); it != m_Definitions.end(); it ++ )
      {
      definitions[it->first] = 1;
      }
    }
  cmCacheManager::CacheIterator cit = this->GetCacheManager()->GetCacheIterator();
  for ( cit.Begin(); !cit.IsAtEnd(); cit.Next() )
    {
    definitions[cit.GetName()] = 1;
    }
  
  std::vector<std::string> res;

  std::map<std::string, int>::iterator fit;
  for ( fit = definitions.begin(); fit != definitions.end(); fit ++ )
    {
    res.push_back(fit->first);
    }
  return res;
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
    cmsys::RegularExpression var("(\\${[A-Za-z_0-9]*})");
    while (var.find(source))
      {
      source.erase(var.start(),var.end() - var.start());
      }
    }
  
  if(!atOnly)
    {
    cmsys::RegularExpression varb("(\\$ENV{[A-Za-z_0-9]*})");
    while (varb.find(source))
      {
      source.erase(varb.start(),varb.end() - varb.start());
      }
    }
  cmsys::RegularExpression var2("(@[A-Za-z_0-9]*@)");
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
  // First search for a group that lists the file explicitly.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    if(sg->MatchesFiles(source))
      {
      return *sg;
      }
    }
  
  // Now search for a group whose regex matches the file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    if(sg->MatchesRegex(source))
      {
      return *sg;
      }
    }
  
  // Shouldn't get here, but just in case, return the default group.
  return groups.front();
}

bool cmMakefile::IsFunctionBlocked(const cmListFileFunction& lff)
{
  // if there are no blockers get out of here
  if (m_FunctionBlockers.begin() == m_FunctionBlockers.end())
    {
    return false;
    }

  // loop over all function blockers to see if any block this command
  // evaluate in reverse, this is critical for balanced IF statements etc
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = m_FunctionBlockers.rbegin(); 
       pos != m_FunctionBlockers.rend(); ++pos)
    {
    if((*pos)->IsFunctionBlocked(lff, *this))
      {
      return true;
      }
    }
  
  return false;
}

void cmMakefile::ExpandArguments(
  std::vector<cmListFileArgument> const& inArgs,
  std::vector<std::string>& outArgs)
{
  std::vector<cmListFileArgument>::const_iterator i;
  std::string value;
  outArgs.reserve(inArgs.size());
  for(i = inArgs.begin(); i != inArgs.end(); ++i)
    {
    // Expand the variables in the argument.
    value = i->Value;
    this->ExpandVariablesInString(value);
    
    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if(i->Quoted)
      {
      outArgs.push_back(value);
      }
    else
      {
      cmSystemTools::ExpandListArgument(value, outArgs);
      }
    }
}

void cmMakefile::RemoveFunctionBlocker(const cmListFileFunction& lff)
{
  // loop over all function blockers to see if any block this command
  std::list<cmFunctionBlocker *>::reverse_iterator pos;
  for (pos = m_FunctionBlockers.rbegin(); 
       pos != m_FunctionBlockers.rend(); ++pos)
    {
    if ((*pos)->ShouldRemove(lff, *this))
      {
      cmFunctionBlocker* b = *pos;
      m_FunctionBlockers.remove(b);
      delete b;
      break;
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
  // if the source is provided with a full path use it, otherwise
  // by default it is in the current source dir
  std::string path = cmSystemTools::GetFilenamePath(sourceName);
  if (path.empty())
    {
    path = this->GetCurrentDirectory();
    }

  std::string sname = 
    cmSystemTools::GetFilenameWithoutLastExtension(sourceName);

  // compute the extension
  std::string ext
    = cmSystemTools::GetFilenameLastExtension(sourceName);
  if ( ext.length() && ext[0] == '.' )
    {
    ext = ext.substr(1);
    }

  for(std::vector<cmSourceFile*>::const_iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    if ((*i)->GetSourceName() == sname &&
        cmSystemTools::GetFilenamePath((*i)->GetFullPath()) == path &&
        (ext.size() == 0 || (ext == (*i)->GetSourceExtension())))
      {
      return *i;
      }
    }

  // geeze, if it wasn't found maybe it is listed under the output dir
  if (!cmSystemTools::GetFilenamePath(sourceName).empty())
    {
    return 0;
    }
    
  path = this->GetCurrentOutputDirectory();
  for(std::vector<cmSourceFile*>::const_iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    if ((*i)->GetSourceName() == sname &&
        cmSystemTools::GetFilenamePath((*i)->GetFullPath()) == path &&
        (ext.size() == 0 || (ext == (*i)->GetSourceExtension())))
      {
      return *i;
      }
    }  
  
  return 0;
}

cmSourceFile* cmMakefile::GetOrCreateSource(const char* sourceName, 
                                            bool generated)
{
  // make it a full path first
  std::string src = sourceName;
  bool relative = !cmSystemTools::FileIsFullPath(sourceName);
  std::string srcTreeFile = this->GetCurrentDirectory();
  srcTreeFile += "/";
  srcTreeFile += sourceName;

  if(relative)
    {
    src = srcTreeFile;
    }
  
  // check to see if it exists
  cmSourceFile* ret = this->GetSource(src.c_str());
  if (ret)
    {
    return ret;
    }

  // OK a source file object doesn't exist for the source
  // maybe we made a bad call on assuming it was in the src tree
  std::string buildTreeFile = this->GetCurrentOutputDirectory();
  buildTreeFile += "/";
  buildTreeFile += sourceName;

  if (relative)
    {
    src = buildTreeFile;
    ret = this->GetSource(src.c_str());
    if (ret)
      {
      return ret;
      }
    // if it has not been marked generated check to see if it exists in the
    // src tree
    if(!generated)
      {
      // see if the file is in the source tree, otherwise assume it 
      // is in the binary tree
      if (cmSystemTools::FileExists(srcTreeFile.c_str()) &&
          !cmSystemTools::FileIsDirectory(srcTreeFile.c_str()))
        {
        src = srcTreeFile;
        }
      else
        {
        if ( cmSystemTools::GetFilenameLastExtension(srcTreeFile.c_str()).size() == 0)
          {
          if (cmSystemTools::DoesFileExistWithExtensions(
                srcTreeFile.c_str(), this->GetSourceExtensions()))
            {
            src = srcTreeFile;
            }
          else if (cmSystemTools::DoesFileExistWithExtensions(
                     srcTreeFile.c_str(), this->GetHeaderExtensions()))
            {
            src = srcTreeFile;
            }
          }
        }
      }
    }

  // a cmSourceFile instance does not exist yet so we must create one
  // go back to looking in the source directory for it

  // we must create one
  cmSourceFile file; 
  std::string path = cmSystemTools::GetFilenamePath(src);
  if(generated)
    {
    std::string ext = cmSystemTools::GetFilenameLastExtension(src);
    std::string name_no_ext = cmSystemTools::GetFilenameName(src.c_str());
    name_no_ext = name_no_ext.substr(0, name_no_ext.length()-ext.length());
    if ( ext.length() && ext[0] == '.' )
      {
      ext = ext.substr(1);
      }
    file.SetName(name_no_ext.c_str(), path.c_str(), ext.c_str(), false);
    }
  else
    {
    file.SetName(cmSystemTools::GetFilenameName(src.c_str()).c_str(), 
                 path.c_str(),
                 this->GetSourceExtensions(),
                 this->GetHeaderExtensions());
    }
  // add the source file to the makefile
  this->AddSource(file);
  src = file.GetFullPath();
  ret = this->GetSource(src.c_str());
  if (!ret)
    {
    cmSystemTools::Error(
      "CMake failed to properly look up cmSourceFile: ", sourceName);
    }
  return ret;
}

cmSourceFile* cmMakefile::AddSource(cmSourceFile const&sf)
{
  // check to see if it exists
  cmSourceFile* ret = this->GetSource(sf.GetFullPath().c_str());
  if(ret)
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
  unsigned int i;
  for(i = 0; i < arguments.size(); ++i)
    {
    // is the arg defined ?, if so use the def
    const char *def = this->GetDefinition(arguments[i].c_str());
    if (def && oldVersion && i >= start)
      {
      // Definition lookup could result in a list that needs to be
      // expanded.
      cmSystemTools::ExpandListArgument(def, newargs);
      }
    else
      {
      // List expansion will have been done already.
      newargs.push_back(arguments[i]);
      }
    }
}

int cmMakefile::TryCompile(const char *srcdir, const char *bindir, 
                           const char *projectName, const char *targetName,
                           const std::vector<std::string> *cmakeArgs,
                           std::string *output)
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
  cm.SetCMakeCommand(cmakeCommand.c_str());
  cm.LoadCache();
  // if cmake args were provided then pass them in
  if (cmakeArgs)
    {
    cm.SetCacheArgs(*cmakeArgs);
    }
  // to save time we pass the EnableLanguage info directly
  gg->EnableLanguagesFromGenerator(m_LocalGenerator->GetGlobalGenerator());
  
  if (cm.Configure() != 0)
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
                                                       targetName,
                                                       output);

  cmSystemTools::ChangeDirectory(cwd.c_str());
  return ret;
}

cmake *cmMakefile::GetCMakeInstance() const
{
  if ( m_LocalGenerator && m_LocalGenerator->GetGlobalGenerator() )
    {
    return m_LocalGenerator->GetGlobalGenerator()->GetCMakeInstance();
    }
  return 0;
}

cmVariableWatch *cmMakefile::GetVariableWatch() const
{
  if ( this->GetCMakeInstance() &&
       this->GetCMakeInstance()->GetVariableWatch() )
    {
    return this->GetCMakeInstance()->GetVariableWatch();
    }
  return 0;
}

cmCacheManager *cmMakefile::GetCacheManager() const
{
  return this->GetCMakeInstance()->GetCacheManager();
}

bool cmMakefile::GetLocal() const
{
  return this->GetCMakeInstance()->GetLocal();
}
void cmMakefile::DisplayStatus(const char* message, float s)
{
  this->GetLocalGenerator()->GetGlobalGenerator()
    ->GetCMakeInstance()->UpdateProgress(message, s);
}

/**
 * Find the library with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the library if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string cmMakefile::FindLibrary(const char* name,
                                    const std::vector<std::string>& userPaths)
{
  // See if the executable exists as written.
  if(cmSystemTools::FileExists(name))
    {
    return cmSystemTools::CollapseFullPath(name);
    }
  
  // Add the system search path to our path.
  std::vector<std::string> path = userPaths;
  cmSystemTools::GetPath(path);

  // Add some lib directories specific to compilers, depending on the
  // current generator, so that library that might have been stored here
  // can be found too.
  // i.e. Microsoft Visual Studio or .Net: path to compiler/../Lib
  //      Borland: path to compiler/../Lib
  const char* genName = this->GetDefinition("CMAKE_GENERATOR");
  if (genName)
    {
    if (!strcmp(genName, "NMake Makefiles") ||
        !strcmp(genName, "Visual Studio 6"))
      {
      const char* compiler = this->GetDefinition("CMAKE_CXX_COMPILER");
      if (compiler)
        {
        std::string compiler_path = cmSystemTools::FindProgram(compiler);
        if (compiler_path.size())
          {
          std::string lib_path = 
            cmSystemTools::GetFilenamePath(
              cmSystemTools::GetFilenamePath(compiler_path)) + "/Lib";
          path.push_back(lib_path);
          }
        }
      }
    else if (!strcmp(genName, "Visual Studio 7"))
      {
      // It is likely that the compiler won't be in the path for .Net, but
      // we know where devenv is.
      const char* devenv = this->GetDefinition("MICROSOFT_DEVENV");
      if (devenv)
        {
        std::string devenv_path = cmSystemTools::FindProgram(devenv);
        if (devenv_path.size())
          {
          std::string vc7_path = 
            cmSystemTools::GetFilenamePath(
              cmSystemTools::GetFilenamePath(
                cmSystemTools::GetFilenamePath(devenv_path))) + "/Vc7";
          path.push_back(vc7_path + "/lib");
          path.push_back(vc7_path + "/PlatformSDK/lib");
          }
        }
      }
    else if (!strcmp(genName, "Borland Makefiles"))
      {
      const char* bcb_bin_path = this->GetDefinition("BCB_BIN_PATH");
      if (bcb_bin_path)
        {
        std::string lib_path = 
          cmSystemTools::GetFilenamePath(bcb_bin_path) + "/Lib";
        path.push_back(lib_path);
        }
      }
    }
  
  return cmSystemTools::FindLibrary(name, path);
}
