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
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmCommandArgumentParserHelper.h"
#include "cmTest.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
#  include "cmVariableWatch.h"
#endif
#include "cmake.h"
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

  // Set up a list of source and header extensions
  // these are used to find files when the extension 
  // is not given
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
  m_cmDefineRegex.compile("#cmakedefine[ \t]*([A-Za-z_0-9]*)");

  this->PreOrder = false;
}

const char* cmMakefile::GetReleaseVersion()
{
#if CMake_VERSION_MINOR & 1
  return "development";
#else
# if CMake_VERSION_PATCH == 0
  return "beta";
# else
  return "patch " CMAKE_TO_STRING(CMake_VERSION_PATCH);
# endif  
#endif
}

unsigned int cmMakefile::GetCacheMajorVersion()
{
  if(const char* vstr =
     this->GetCacheManager()->GetCacheValue("CMAKE_CACHE_MAJOR_VERSION"))
    {
    unsigned int v=0;
    if(sscanf(vstr, "%u", &v) == 1)
      {
      return v;
      }
    }
  return 0;
}

unsigned int cmMakefile::GetCacheMinorVersion()
{
  if(const char* vstr =
     this->GetCacheManager()->GetCacheValue("CMAKE_CACHE_MINOR_VERSION"))
    {
    unsigned int v=0;
    if(sscanf(vstr, "%u", &v) == 1)
      {
      return v;
      }
    }
  return 0;
}


cmMakefile::~cmMakefile()
{
  for(std::vector<cmSourceFile*>::iterator i = m_SourceFiles.begin();
      i != m_SourceFiles.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmTest*>::iterator i = m_Tests.begin();
      i != m_Tests.end(); ++i)
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

void cmMakefile::PrintStringVector(const char* s, const std::vector<std::pair<cmStdString, bool> >& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::pair<cmStdString, bool> >::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    std::cout << i->first.c_str() << " " << i->second;
    }
  std::cout << " )\n";
}


// call print on all the classes in the makefile
void cmMakefile::Print()
{
  // print the class lists
  std::cout << "classes:\n";

  std::cout << " m_Targets: ";
  for (cmTargets::iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    std::cout << l->first << std::endl;
    }

  std::cout << " m_StartOutputDirectory; " << 
    m_StartOutputDirectory.c_str() << std::endl;
  std::cout << " m_HomeOutputDirectory; " << 
    m_HomeOutputDirectory.c_str() << std::endl;
  std::cout << " m_cmStartDirectory; " << 
    m_cmStartDirectory.c_str() << std::endl;
  std::cout << " m_cmHomeDirectory; " << 
    m_cmHomeDirectory.c_str() << std::endl;
  std::cout << " m_ProjectName; " <<  m_ProjectName.c_str() << std::endl;
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
    if(usedCommand->GetEnabled() && !cmSystemTools::GetFatalErrorOccured()  &&
       (!this->GetCMakeInstance()->GetScriptMode() || 
        usedCommand->IsScriptable()))
      {
      if(!usedCommand->InvokeInitialPass(lff.m_Arguments))
        {
        cmOStringStream error;
        error << "Error in cmake code at\n"
              << lff.m_FilePath << ":" << lff.m_Line << ":\n"
              << usedCommand->GetError();
        cmSystemTools::Error(error.str().c_str());
        result = false;
        if ( this->GetCMakeInstance()->GetScriptMode() )
          {
          cmSystemTools::SetFatalErrorOccured();
          }
        }
      else
        {
        // use the command
        keepCommand = true;
        m_UsedCommands.push_back(usedCommand);
        }
      }
    else if ( this->GetCMakeInstance()->GetScriptMode() && !usedCommand->IsScriptable() )
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << lff.m_FilePath << ":" << lff.m_Line << ":\n"
            << "Command " << usedCommand->GetName() << " not scriptable" << std::endl;
      cmSystemTools::Error(error.str().c_str());
      result = false;
      cmSystemTools::SetFatalErrorOccured();
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

// Parse the given CMakeLists.txt file executing all commands
//
bool cmMakefile::ReadListFile(const char* filename_in, const char *external_in)
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
                                      m_cmStartDirectory.c_str());
    external = external_abs.c_str();
    if (filename_in)
      {
      filename_abs =
        cmSystemTools::CollapseFullPath(filename_in,
                                        m_cmStartDirectory.c_str());
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
  if(!external && m_cmStartDirectory == m_cmHomeDirectory)
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
    if ( cmSystemTools::GetFatalErrorOccured() )
      {
      return true;
      }
    }

  // send scope ended to and function blockers
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
  const char* oldValue
    = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (oldValue && atof(oldValue) <= 1.2)
    {
    cmSystemTools::Error("You have requested backwards compatibility with CMake version 1.2 or earlier. This version of CMake only supports backwards compatibility with CMake 1.4 or later. For compatibility with 1.2 or earlier please use CMake 2.0");
    }
  
  for (cmTargets::iterator l = m_Targets.begin();
       l != m_Targets.end(); l++)
    {
    l->second.GenerateSourceFilesFromSourceLists(*this);
    l->second.AnalyzeLibDependencies(*this);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToTarget(const char* target,
                                     const std::vector<std::string>& depends,
                                     const cmCustomCommandLines& commandLines,
                                     cmTarget::CustomCommandType type,
                                     const char* comment)
{
  // Find the target to which to add the custom command.
  cmTargets::iterator ti = m_Targets.find(target);
  if(ti != m_Targets.end())
    {
    // Add the command to the appropriate build step for the target.
    const char* no_output = 0;
    cmCustomCommand cc(no_output, depends, commandLines, comment);
    switch(type)
      {
      case cmTarget::PRE_BUILD:
        ti->second.GetPreBuildCommands().push_back(cc);
        break;
      case cmTarget::PRE_LINK:
        ti->second.GetPreLinkCommands().push_back(cc);
        break;
      case cmTarget::POST_BUILD:
        ti->second.GetPostBuildCommands().push_back(cc);
        break;
      }

    // Add dependencies on commands CMake knows how to build.
    for(cmCustomCommandLines::const_iterator cli = commandLines.begin();
        cli != commandLines.end(); ++cli)
      {
      std::string cacheCommand = *cli->begin();
      if(const char* knownTarget =
         this->GetCacheManager()->GetCacheValue(cacheCommand.c_str()))
        {
        ti->second.AddUtility(knownTarget);
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToOutput(const char* output,
                                     const std::vector<std::string>& depends,
                                     const char* main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     bool replace)
{
  // Choose a source file on which to store the custom command.
  cmSourceFile* file = 0;
  if(main_dependency && main_dependency[0])
    {
    // The main dependency was specified.  Use it unless a different
    // custom command already used it.
    file = this->GetSource(main_dependency);
    if(file && file->GetCustomCommand() && !replace)
      {
      // The main dependency already has a custom command.
      if(commandLines == file->GetCustomCommand()->GetCommandLines())
        {
        // The existing custom command is identical.  Silently ignore
        // the duplicate.
        return;
        }
      else
        {
        // The existing custom command is different.  We need to
        // generate a rule file for this new command.
        file = 0;
        }
      }
    else
      {
      // The main dependency does not have a custom command or we are
      // allowed to replace it.  Use it to store the command.
      file = this->GetOrCreateSource(main_dependency);
      }
    }

  // Generate a rule file if the main dependency is not available.
  if(!file)
    {
    // Construct a rule file associated with the output produced.
    std::string outName = output;
    outName += ".rule";

    // Check if the rule file already exists.
    file = this->GetSource(outName.c_str());
    if(file && file->GetCustomCommand() && !replace)
      {
      // The rule file already exists.
      if(commandLines != file->GetCustomCommand()->GetCommandLines())
        {
        cmSystemTools::Error("Attempt to add a custom rule to output \"",
                             output, "\" which already has a custom rule.");
        }
      return;
      }

    // Create a cmSourceFile for the rule file.
    file = this->GetOrCreateSource(outName.c_str(), true);
    }

  // Always create the output and mark it generated.
  if(cmSourceFile* out = this->GetOrCreateSource(output, true))
    {
    out->SetProperty("GENERATED", "1");
    }

  // Construct a complete list of dependencies.
  std::vector<std::string> depends2(depends);
  if(main_dependency && main_dependency[0])
    {
    depends2.push_back(main_dependency);
    }

  // Attach the custom command to the file.
  if(file)
    {
    cmCustomCommand* cc =
      new cmCustomCommand(output, depends2, commandLines, comment);
    file->SetCustomCommand(cc);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandOldStyle(const char* target,
                                     const std::vector<std::string>& outputs,
                                     const std::vector<std::string>& depends,
                                     const char* source,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment)
{
  // Translate the old-style signature to one of the new-style
  // signatures.
  if(strcmp(source, target) == 0)
    {
    // In the old-style signature if the source and target were the
    // same then it added a post-build rule to the target.  Preserve
    // this behavior.
    this->AddCustomCommandToTarget(target, depends, commandLines,
                                   cmTarget::POST_BUILD, comment);
    return;
    }

  // Each output must get its own copy of this rule.
  cmsys::RegularExpression sourceFiles("\\.(C|M|c|c\\+\\+|cc|cpp|cxx|m|mm|"
                                       "rc|def|r|odl|idl|hpj|bat|h|h\\+\\+|"
                                       "hm|hpp|hxx|in|txx|inl)$");
  for(std::vector<std::string>::const_iterator oi = outputs.begin();
      oi != outputs.end(); ++oi)
    {
    // Get the name of this output.
    const char* output = oi->c_str();

    // Choose whether to use a main dependency.
    if(sourceFiles.find(source))
      {
      // The source looks like a real file.  Use it as the main dependency.
      this->AddCustomCommandToOutput(output, depends, source,
                                     commandLines, comment);
      }
    else
      {
      // The source may not be a real file.  Do not use a main dependency.
      const char* no_main_dependency = 0;
      std::vector<std::string> depends2 = depends;
      depends2.push_back(source);
      this->AddCustomCommandToOutput(output, depends2, no_main_dependency,
                                     commandLines, comment);
      }

    // If the rule was added to the source (and not a .rule file),
    // then add the source to the target to make sure the rule is
    // included.
    std::string sname = output;
    sname += ".rule";
    if(!this->GetSource(sname.c_str()))
      {
      if (m_Targets.find(target) != m_Targets.end())
        {
        m_Targets[target].GetSourceLists().push_back(source);
        }
      else
        {
        cmSystemTools::Error("Attempt to add a custom rule to a target "
                             "that does not exist yet for target ", target);
        return;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefile::AddUtilityCommand(const char* utilityName, bool all,
                                   const char* output,
                                   const std::vector<std::string>& depends,
                                   const char* command,
                                   const char* arg1,
                                   const char* arg2,
                                   const char* arg3,
                                   const char* arg4)
{
  // Construct the command line for the custom command.
  cmCustomCommandLine commandLine;
  commandLine.push_back(command);
  if(arg1)
    {
    commandLine.push_back(arg1);
    }
  if(arg2)
    {
    commandLine.push_back(arg2);
    }
  if(arg3)
    {
    commandLine.push_back(arg3);
    }
  if(arg4)
    {
    commandLine.push_back(arg4);
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Call the real signature of this method.
  this->AddUtilityCommand(utilityName, all, output, depends, commandLines);
}

//----------------------------------------------------------------------------
void cmMakefile::AddUtilityCommand(const char* utilityName, bool all,
                                   const char* output,
                                   const std::vector<std::string>& depends,
                                   const cmCustomCommandLines& commandLines)
{
  // Create a target instance for this utility.
  cmTarget target;
  target.SetType(cmTarget::UTILITY, utilityName);
  target.SetInAll(all);

  // Store the custom command in the target.
  cmCustomCommand cc(output, depends, commandLines, 0);
  target.GetPostBuildCommands().push_back(cc);

  // Add the target to the set of targets.
  m_Targets.insert(cmTargets::value_type(utilityName, target));
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  m_DefineFlags += " ";
  m_DefineFlags += flag;
}


void cmMakefile::RemoveDefineFlag(const char* flag)
{
  cmSystemTools::ReplaceString(m_DefineFlags, flag, " ");
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

void cmMakefile::InitializeFromParent()
{
  cmMakefile *parent = m_LocalGenerator->GetParent()->GetMakefile();
  
  // copy the definitions
  this->m_Definitions = parent->m_Definitions;

  // copy include paths
  this->m_IncludeDirectories = parent->m_IncludeDirectories;
  
  // define flags
  this->m_DefineFlags = parent->m_DefineFlags;
  
  // link libraries
  this->m_LinkLibraries = parent->m_LinkLibraries;
  
  // link directories
  this->m_LinkDirectories = parent->m_LinkDirectories;
  
  // the initial project name
  this->m_ProjectName = parent->m_ProjectName;
}

void cmMakefile::ConfigureSubDirectory(cmLocalGenerator *lg2)
{
  // copy our variables from the child makefile
  lg2->GetMakefile()->InitializeFromParent();
  lg2->GetMakefile()->MakeStartDirectoriesCurrent();
  
  // finally configure the subdir
  lg2->Configure();
}

void cmMakefile::AddSubDirectory(const char* sub, bool topLevel, bool preorder)
{
  // the source path must be made full if it isn't already
  std::string srcPath = sub;
  if (!cmSystemTools::FileIsFullPath(srcPath.c_str()))
    {
    srcPath = this->GetCurrentDirectory();
    srcPath += "/";
    srcPath += sub;
    }

  // binary path must be made full if it isn't already
  std::string binPath = sub;
  if (!cmSystemTools::FileIsFullPath(binPath.c_str()))
    {
    binPath = this->GetCurrentOutputDirectory();
    binPath += "/";
    binPath += sub;
    }
  
  
  this->AddSubDirectory(srcPath.c_str(), binPath.c_str(), 
                        topLevel, preorder, false);
}

                        
void cmMakefile::AddSubDirectory(const char* srcPath, const char *binPath,
                                 bool topLevel, bool preorder, 
                                 bool immediate)
{
  std::vector<cmLocalGenerator *>& children = m_LocalGenerator->GetChildren();
  // has this directory already been added? If so error
  unsigned int i;
  for (i = 0; i < children.size(); ++i)
    {
    if (srcPath == children[i]->GetMakefile()->GetStartDirectory())
      {
      cmSystemTools::Error("Attempt to add subdirectory multiple times for directory.\n", srcPath);
      return;
      }
    }
  
  // create a new local generator and set its parent
  cmLocalGenerator *lg2 = 
    m_LocalGenerator->GetGlobalGenerator()->CreateLocalGenerator();
  lg2->SetParent(m_LocalGenerator);
  m_LocalGenerator->GetGlobalGenerator()->AddLocalGenerator(lg2);
  
  // set the subdirs start dirs
  lg2->GetMakefile()->SetStartDirectory(srcPath);
  lg2->GetMakefile()->SetStartOutputDirectory(binPath);
  lg2->SetExcludeAll(!topLevel);
  lg2->GetMakefile()->SetPreOrder(preorder);
  
  if (immediate)
    {
    this->ConfigureSubDirectory(lg2);
    }
}

void cmMakefile::AddIncludeDirectory(const char* inc, bool before)
{
  // Don't add an include directory that is already present.  Yes,
  // this linear search results in n^2 behavior, but n won't be
  // getting much bigger than 20.  We cannot use a set because of
  // order dependency of the include path.
  std::vector<std::string>::iterator i = 
    std::find(m_IncludeDirectories.begin(),
              m_IncludeDirectories.end(), inc);
  if(i == m_IncludeDirectories.end())
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
  else
    {
    if(before)
      {
      // if this before and already in the path then remove it
      m_IncludeDirectories.erase(i);
      // WARNING: this *is* expensive (linear time) since it's a vector
      m_IncludeDirectories.insert(m_IncludeDirectories.begin(), inc);
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

#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(m_TemporaryDefinitionKey, 
                         cmVariableWatch::VARIABLE_MODIFIED_ACCESS);
    }
#endif
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
    if ( type == cmCacheManager::PATH || type == cmCacheManager::FILEPATH )
      {
      std::vector<std::string>::size_type cc;
      std::vector<std::string> files;
      std::string nvalue = "";
      cmSystemTools::ExpandListArgument(val, files);
      for ( cc = 0; cc < files.size(); cc ++ )
        {
        files[cc] = cmSystemTools::CollapseFullPath(files[cc].c_str());
        if ( cc > 0 )
          {
          nvalue += ";";
          }
        nvalue += files[cc];
        }

      this->GetCacheManager()->AddCacheEntry(name, nvalue.c_str(), doc, type);
      val = it.GetValue();
      }

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
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_MODIFIED_ACCESS);
    }
#endif
}


void cmMakefile::AddCacheDefinition(const char* name, bool value, const char* doc)
{
  bool val = value;
  cmCacheManager::CacheIterator it = 
    this->GetCacheManager()->GetCacheIterator(name);
  if(!it.IsAtEnd() && (it.GetType() == cmCacheManager::UNINITIALIZED) &&
     it.Initialized())
    {
    val = it.GetValueAsBool();
    }
  this->GetCacheManager()->AddCacheEntry(name, val, doc);
  this->AddDefinition(name, val);
}

void cmMakefile::RemoveDefinition(const char* name)
{
  m_Definitions.erase(DefinitionMap::key_type(name));
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS);
    }
#endif
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
      target.SetType(cmTarget::STATIC_LIBRARY, lname);
      break;
    case 1:
      target.SetType(cmTarget::SHARED_LIBRARY, lname);
      break;
    case 2:
      target.SetType(cmTarget::MODULE_LIBRARY, lname);
      break;
    default:
      target.SetType(cmTarget::STATIC_LIBRARY, lname);
    }

  // Clear its dependencies. Otherwise, dependencies might persist
  // over changes in CMakeLists.txt, making the information stale and
  // hence useless.
  target.ClearDependencyInformation( *this, lname );
  target.SetInAll(true);
  target.GetSourceLists() = srcs;
  target.SetMakefile(this);
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

cmTarget* cmMakefile::AddExecutable(const char *exeName, 
                                    const std::vector<std::string> &srcs)
{
  cmTarget target;
  target.SetType(cmTarget::EXECUTABLE, exeName);
  target.SetInAll(true);
  target.GetSourceLists() = srcs;
  target.SetMakefile(this);
  this->AddGlobalLinkInformation(exeName, target);
  cmTargets::iterator it = 
    m_Targets.insert(cmTargets::value_type(exeName,target)).first;
  
  // Add an entry into the cache 
  std::string exePath = exeName;
  exePath += "_CMAKE_PATH";
  this->GetCacheManager()->
    AddCacheEntry(exePath.c_str(),
                  this->GetCurrentOutputDirectory(),
                  "Path to an executable", cmCacheManager::INTERNAL);
  return &it->second;
}

cmSourceFile *cmMakefile::GetSourceFileWithOutput(const char *cname)
{
  std::string name = cname;
  std::string out;

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
      out = (*i)->GetCustomCommand()->GetOutput();
      std::string::size_type pos = out.rfind(name);
      // If the output matches exactly
      if (pos != out.npos &&
          pos == out.size() - name.size() &&
          (pos ==0 || out[pos-1] == '/'))
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
    else
      {
      cmSourceGroup *target = sg->lookupChild(name);

      if(target)
        {
        return target;
        }
      }
    }
  return 0;
}

void cmMakefile::AddSourceGroup(const char* name, const char* regex, const char *parent)
{
  // First see if the group exists.  If so, replace its regular expression.
  for(unsigned int i=0;i<m_SourceGroups.size();++i)
    {
    cmSourceGroup *sg = &m_SourceGroups[i];
                                
    std::string sgName = sg->GetName();
    if(!parent)
      {
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
    else
      {
      if(sgName == parent)
        {
        cmSourceGroup *localtarget = sg->lookupChild(name);
        if(localtarget)
          {
          if ( regex )
            {
            // We only want to set the regular expression.  If there are already
            // source files in the group, we don't want to remove them.
            localtarget->SetGroupRegex(regex);
            }
          } 
        else
          {
          sg->AddChild(cmSourceGroup(name, regex));
          }
        return;
        } 
      else 
        {
        cmSourceGroup *localtarget = sg->lookupChild(parent);

        if(localtarget)
          {
          cmSourceGroup *addtarget = localtarget->lookupChild(name);
                                                        
          if(addtarget)
            {
            if ( regex )
              {
              // We only want to set the regular expression.  If there are already
              // source files in the group, we don't want to remove them.
              addtarget->SetGroupRegex(regex);
              }
            } 
          else
            {
            localtarget->AddChild(cmSourceGroup(name, regex));
            }
          return;
          } 
        }
      }
    }
                
  // The group doesn't exist.  Add it.
  m_SourceGroups.push_back(cmSourceGroup(name, regex));
}

void cmMakefile::AddExtraDirectory(const char* dir)
{
  m_AuxSourceDirectories.push_back(dir);
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

bool cmMakefile::IsOn(const char* name) const
{
  const char* value = this->GetDefinition(name);
  return cmSystemTools::IsOn(value);
}

bool cmMakefile::IsSet(const char* name) const
{
  const char* value = this->GetDefinition(name);
  if ( !value )
    {
    return false;
    }

  if ( ! *value )
    {
    return false;
    }

  if ( cmSystemTools::IsNOTFOUND(value) )
    {
    return false;
    }

  return true;
}

const char* cmMakefile::GetRequiredDefinition(const char* name) const
{
  const char* ret = this->GetDefinition(name);
  if(!ret)
    {
    cmSystemTools::Error("Error required internal CMake variable not set, cmake may be not be built correctly.\n",
                         "Missing variable is:\n",
                         name);
    return "";
    }
  return ret;
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
#ifdef CMAKE_BUILD_WITH_CMAKE
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
#endif
  return def;
}

const char* cmMakefile::GetSafeDefinition(const char* def) const
{
  const char* ret = this->GetDefinition(def);
  if(!ret)
    {
    return "";
    }
  return ret;
}

std::vector<std::string> cmMakefile::GetDefinitions(int cacheonly /* = 0 */) const
{
  std::map<cmStdString, int> definitions;
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

  std::map<cmStdString, int>::iterator fit;
  for ( fit = definitions.begin(); fit != definitions.end(); fit ++ )
    {
    res.push_back(fit->first);
    }
  return res;
}


const char *cmMakefile::ExpandVariablesInString(std::string& source) const
{
  return this->ExpandVariablesInString(source, false, false);
}

const char *cmMakefile::ExpandVariablesInString(std::string& source,
                                                bool escapeQuotes,
                                                bool noEscapes,
                                                bool atOnly,
                                                const char* filename,
                                                long line,
                                                bool removeEmpty) const
{
  if ( source.empty() || source.find_first_of("$@\\") == source.npos)
    {
    return source.c_str();
    }
  // This method replaces ${VAR} and @VAR@ where VAR is looked up
  // with GetDefinition(), if not found in the map, nothing is expanded.
  // It also supports the $ENV{VAR} syntax where VAR is looked up in
  // the current environment variables.
  
  bool notParsed = true;
  if ( !atOnly )
    {
    cmCommandArgumentParserHelper parser;
    parser.SetMakefile(this);
    parser.SetLineFile(line, filename);
    parser.SetEscapeQuotes(escapeQuotes);
    parser.SetNoEscapeMode(noEscapes);
    int res = parser.ParseString(source.c_str(), 0);
    if ( res )
      {
      source = parser.GetResult();
      notParsed = false;
      }
    else
      {
      cmOStringStream error;
      error << "Syntax error in cmake code at\n"
            << (filename?filename:"(no filename given)") 
            << ":" << line << ":\n"
            << parser.GetError() << ", when parsing string \"" 
            << source.c_str() << "\"";
      const char* versionValue
        = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
      int major = 0;
      int minor = 0;
      if ( versionValue )
        {
        sscanf(versionValue, "%d.%d", &major, &minor);
        }
      if ( major < 2 || major == 2 && minor < 1 )
        {
        cmSystemTools::Error(error.str().c_str());
        cmSystemTools::SetFatalErrorOccured();
        return source.c_str();
        }
      else
        {
        cmSystemTools::Message(error.str().c_str());
        }
      //std::cerr << "[" << source.c_str() << "] results in: [" << parser.GetResult() << "]" << std::endl;
      }
    }

  if ( notParsed )
    {

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
            else if(filename && (var == "CMAKE_CURRENT_LIST_FILE"))
              {
              result += filename;
              found = true;
              }
            else if(line >= 0 && (var == "CMAKE_CURRENT_LIST_LINE"))
              {
              cmOStringStream ostr;
              ostr << line;
              result += ostr.str();
              found = true;
              }
            }
          // if found add to result, if not, then it gets blanked
          if (!found)
            {
            // if no definition is found then add the var back
            if(!removeEmpty && endVariableMarker == '@')
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
    }
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
    cmSourceGroup *result = sg->MatchChildrenFiles(source);
    if(result)
      {
      return *result;
      }
    }
  
  // Now search for a group whose regex matches the file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    cmSourceGroup *result = sg->MatchChildrenRegex(source);
    if(result)
      {
      return *result;
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
    this->ExpandVariablesInString(value, false, false, false, i->FilePath, i->Line);

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
  if ( !this->GetDefinition("CMAKE_CURRENT_SOURCE_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", this->GetHomeDirectory());
    }
}

void cmMakefile::SetHomeOutputDirectory(const char* lib)
{
  m_HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(m_HomeOutputDirectory);
  this->AddDefinition("CMAKE_BINARY_DIR", this->GetHomeOutputDirectory());
  if ( !this->GetDefinition("CMAKE_CURRENT_BINARY_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_BINARY_DIR", this->GetHomeOutputDirectory());
    }
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
    if ((*i)->GetSourceNameWithoutLastExtension() == sname &&
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
    bool headerFile = !(std::find( m_HeaderFileExtensions.begin(), m_HeaderFileExtensions.end(), ext ) ==
                        m_HeaderFileExtensions.end());
    file.SetName(name_no_ext.c_str(), path.c_str(), ext.c_str(), headerFile);
    }
  else
    {
    std::string relPath = cmSystemTools::GetFilenamePath(sourceName);
    if (relative && relPath.size())
      {
      // we need to keep the relative part of the filename
      std::string fullPathLessRel = path;
      std::string::size_type pos = fullPathLessRel.rfind(relPath);
      if (pos == std::string::npos)
        {
        cmSystemTools::Error(
          "CMake failed to properly look up relative cmSourceFile: ", 
          sourceName);
        }
      fullPathLessRel.erase(pos-1);
      file.SetName(sourceName, fullPathLessRel.c_str(),
                   this->GetSourceExtensions(),
                   this->GetHeaderExtensions());
      }
    else
      {
      file.SetName(cmSystemTools::GetFilenameName(src.c_str()).c_str(), 
                   path.c_str(),
                   this->GetSourceExtensions(),
                   this->GetHeaderExtensions());
      }
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

  
void cmMakefile::EnableLanguage(std::vector<std::string> const &  lang)
{
  m_LocalGenerator->GetGlobalGenerator()->EnableLanguage(lang, this);
}

void cmMakefile::ExpandSourceListArguments(
  std::vector<std::string> const& arguments, 
  std::vector<std::string>& newargs, unsigned int /* start */)
{
  // now expand the args
  unsigned int i;
  for(i = 0; i < arguments.size(); ++i)
    {
    // List expansion will have been done already.
    newargs.push_back(arguments[i]);
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
                                                       output,
                                                       this);

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

#ifdef CMAKE_BUILD_WITH_CMAKE
cmVariableWatch *cmMakefile::GetVariableWatch() const
{
  if ( this->GetCMakeInstance() &&
       this->GetCMakeInstance()->GetVariableWatch() )
    {
    return this->GetCMakeInstance()->GetVariableWatch();
    }
  return 0;
}
#endif

void cmMakefile::AddMacro(const char* name, const char* signature)
{
  if ( !name || !signature )
    {
    return;
    }
  m_MacrosMap[name] = signature;
}

void cmMakefile::GetListOfMacros(std::string& macros)
{
  StringStringMap::iterator it;
  macros = "";
  int cc = 0;
  for ( it = m_MacrosMap.begin(); it != m_MacrosMap.end(); ++it )
    {
    if ( cc > 0 )
      {
      macros += ";";
      }
    macros += it->first;
    cc ++;
    }
}

cmCacheManager *cmMakefile::GetCacheManager() const
{
  return this->GetCMakeInstance()->GetCacheManager();
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
  std::vector<std::string> path;
  cmSystemTools::GetPath(path, "CMAKE_LIBRARY_PATH");
  cmSystemTools::GetPath(path, "LIB");
  cmSystemTools::GetPath(path);
  // now add the path
  path.insert(path.end(), userPaths.begin(), userPaths.end());
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
  if(m_LocalGenerator->GetGlobalGenerator()->GetLanguageEnabled("C"))
    {
    std::string voidsize = this->GetRequiredDefinition("CMAKE_SIZEOF_VOID_P");
    int size = atoi(voidsize.c_str());
    std::vector<std::string> path64;
    if(size == 8)
      {
      // Convert each search path to possible 32- and 64-bit versions
      // of the names.  Check for the existence of each one here to
      // avoid repeating the check for every file search.
      for(std::vector<std::string>::iterator i = path.begin(); 
          i != path.end(); ++i)
        {
        std::string s = *i;
        std::string s2 = *i;
        cmSystemTools::ReplaceString(s, "lib/", "lib64/");
        if((s != *i) && cmSystemTools::FileIsDirectory(s.c_str()))
          {
          path64.push_back(s);
          }
        s2 += "64";
        if(cmSystemTools::FileIsDirectory(s2.c_str()))
          {
          path64.push_back(s2);
          }
        if(cmSystemTools::FileIsDirectory(i->c_str()))
          {
          path64.push_back(*i);
          }
        }
      // now look for the library in the 64 bit path
      std::string tmp = cmSystemTools::FindLibrary(name, path64);
      cmSystemTools::ConvertToUnixSlashes(tmp);
      return tmp;
      }
    }
  std::string tmp = cmSystemTools::FindLibrary(name, path);
  cmSystemTools::ConvertToUnixSlashes(tmp);
  return tmp;
}

std::string cmMakefile::GetModulesFile(const char* filename)
{
  std::vector<std::string> modulePath;
  const char* def = this->GetDefinition("CMAKE_MODULE_PATH");
  if(def)
    {
    cmSystemTools::ExpandListArgument(def, modulePath);
    }

  // Also search in the standard modules location.
  def = this->GetDefinition("CMAKE_ROOT");
  if(def)
    {
    std::string rootModules = def;
    rootModules += "/Modules";
    modulePath.push_back(rootModules);
    }
  //std::string  Look through the possible module directories.
  for(std::vector<std::string>::iterator i = modulePath.begin();
      i != modulePath.end(); ++i)
    {
    std::string itempl = *i;
    cmSystemTools::ConvertToUnixSlashes(itempl);
    itempl += "/";
    itempl += filename;
    if(cmSystemTools::FileExists(itempl.c_str()))
      {
      return itempl;
      }
    }
  return "";
}

void cmMakefile::ConfigureString(const std::string& input,
                                 std::string& output, bool atOnly,
                                 bool escapeQuotes)
{
  // Split input to handle one line at a time.
  std::string::const_iterator lineStart = input.begin();  
  while(lineStart != input.end())
    {
    // Find the end of this line.
    std::string::const_iterator lineEnd = lineStart;
    while(lineEnd != input.end() && *lineEnd != '\n')
      {
      ++lineEnd;
      }

    // Copy the line.
    std::string line(lineStart, lineEnd);

    // Skip the newline character.
    bool haveNewline = (lineEnd != input.end());
    if(haveNewline)
      {
      ++lineEnd;
      }

    // Replace #cmakedefine instances.
    if(m_cmDefineRegex.find(line))
      {
      const char* def = this->GetDefinition(m_cmDefineRegex.match(1).c_str());
      if(!cmSystemTools::IsOff(def))
        {
        cmSystemTools::ReplaceString(line, "#cmakedefine", "#define");
        output += line;
        }
      else
        {
        cmSystemTools::ReplaceString(line, "#cmakedefine", "#undef");
        output += "/* ";
        output += line;
        output += " */";
        }
      }
    else
      {
      output += line;
      }

    if(haveNewline)
      {
      output += "\n";
      }

    // Move to the next line.
    lineStart = lineEnd;
    }

  // Perform variable replacements.
  this->ExpandVariablesInString(output, escapeQuotes, true, atOnly, 0, -1, true);
//  this->RemoveVariablesInString(output, atOnly);
}

int cmMakefile::ConfigureFile(const char* infile, const char* outfile, 
                              bool copyonly, bool atOnly, bool escapeQuotes)
{
  int res = 1;
  if ( !cmSystemTools::FileExists(infile) )
    {
    cmSystemTools::Error("File ", infile, " does not exist.");
    return 0;
    }
  std::string soutfile = outfile;
  std::string sinfile = infile;
  this->AddCMakeDependFile(infile);
  cmSystemTools::ConvertToUnixSlashes(soutfile);
  mode_t perm = 0;
  cmSystemTools::GetPermissions(sinfile.c_str(), perm);
  std::string::size_type pos = soutfile.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = soutfile.substr(0, pos);
    cmSystemTools::MakeDirectory(path.c_str());
    }
  
  if(copyonly)
    {
    if ( !cmSystemTools::CopyFileIfDifferent(sinfile.c_str(),
                                             soutfile.c_str()))
      {
      return 0;
      }
    }
  else
    {
    std::string tempOutputFile = soutfile;
    tempOutputFile += ".tmp";
    std::ofstream fout(tempOutputFile.c_str());
    if(!fout)
      {
      cmSystemTools::Error(
        "Could not open file for write in copy operation ", 
        tempOutputFile.c_str());
      cmSystemTools::ReportLastSystemError("");
      return 0;
      }
    std::ifstream fin(sinfile.c_str());
    if(!fin)
      {
      cmSystemTools::Error("Could not open file for read in copy operation ",
                           sinfile.c_str());
      return 0;
      }

    // now copy input to output and expand variables in the
    // input file at the same time
    std::string inLine;
    std::string outLine;
    while( cmSystemTools::GetLineFromStream(fin, inLine) )
      {
      outLine = "";
      this->ConfigureString(inLine, outLine, atOnly, escapeQuotes);
      fout << outLine.c_str() << "\n";
      }
    // close the files before attempting to copy
    fin.close();
    fout.close();
    if ( !cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                             soutfile.c_str()) )
      {
      res = 0;
      }
    else
      {
      cmSystemTools::SetPermissions(soutfile.c_str(), perm);
      }
    cmSystemTools::RemoveFile(tempOutputFile.c_str());
    }
  return res;
}

void cmMakefile::AddWrittenFile(const char* file)
{ this->GetCMakeInstance()->AddWrittenFile(file); }

bool cmMakefile::HasWrittenFile(const char* file)
{ return this->GetCMakeInstance()->HasWrittenFile(file); }

bool cmMakefile::CheckInfiniteLoops()
{
  std::vector<std::string>::iterator it;
  for ( it = m_ListFiles.begin();
        it != m_ListFiles.end();
        ++ it )
    {
    if ( this->HasWrittenFile(it->c_str()) )
      {
      cmOStringStream str;
      str << "File " << it->c_str() << " is written by WRITE_FILE (or FILE WRITE) command and should not be used as input to CMake. Please use CONFIGURE_FILE to be safe. Refer to the note next to FILE WRITE command.";
      cmSystemTools::Error(str.str().c_str());
      return false;
      }
    }
  return true;
}

void cmMakefile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (!value)
    {
    value = "NOTFOUND";
    }
  m_Properties[prop] = value;
}

const char *cmMakefile::GetProperty(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmMakefile::GetPropertyAsBool(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    m_Properties.find(prop);
  if (i != m_Properties.end())
    {
    return cmSystemTools::IsOn(i->second.c_str());
    }
  return false;
}


cmTarget* cmMakefile::FindTarget(const char* name)
{
  cmTargets& tgts = this->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    if(l->first == name)
      {
      return &l->second;
      }
    }
  return 0;
}

cmTest* cmMakefile::CreateTest(const char* testName)
{
  if ( !testName )
    {
    return 0;
    }
  cmTest* test = this->GetTest(testName);
  if ( test )
    {
    return test;
    }
  test = new cmTest;
  test->SetName(testName);
  m_Tests.push_back(test);
  return test;
}

cmTest* cmMakefile::GetTest(const char* testName) const
{
  if ( !testName )
    {
    return 0;
    }
  std::vector<cmTest*>::const_iterator it;
  for ( it = m_Tests.begin(); it != m_Tests.end(); ++ it )
    {
    if ( strcmp((*it)->GetName(), testName) == 0 )
      {
      return *it;
      }
    }
  return 0;
}

const std::vector<cmTest*> *cmMakefile::GetTests() const
{
  return &m_Tests;
}

