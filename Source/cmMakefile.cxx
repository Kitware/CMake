/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakefile.h"
#include "cmVersion.h"
#include "cmCommand.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSystemTools.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmCommands.h"
#include "cmCacheManager.h"
#include "cmFunctionBlocker.h"
#include "cmListFileCache.h"
#include "cmDocumentGeneratorExpressions.h"
#include "cmCommandArgumentParserHelper.h"
#include "cmDocumentCompileDefinitions.h"
#include "cmGeneratorExpression.h"
#include "cmTest.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
#  include "cmVariableWatch.h"
#endif
#include "cmInstallGenerator.h"
#include "cmTestGenerator.h"
#include "cmDefinitions.h"
#include "cmake.h"
#include <stdlib.h> // required for atoi

#include <cmsys/RegularExpression.hxx>

#include <cmsys/auto_ptr.hxx>

#include <stack>
#include <ctype.h> // for isspace
#include <assert.h>

class cmMakefile::Internals
{
public:
  std::stack<cmDefinitions, std::list<cmDefinitions> > VarStack;
  std::stack<std::set<cmStdString> > VarInitStack;
  std::stack<std::set<cmStdString> > VarUsageStack;
  bool IsSourceFileTryCompile;
};

// default is not to be building executables
cmMakefile::cmMakefile(): Internal(new Internals)
{
  const cmDefinitions& defs = cmDefinitions();
  const std::set<cmStdString> globalKeys = defs.LocalKeys();
  this->Internal->VarStack.push(defs);
  this->Internal->VarInitStack.push(globalKeys);
  this->Internal->VarUsageStack.push(globalKeys);
  this->Internal->IsSourceFileTryCompile = false;

  // Initialize these first since AddDefaultDefinitions calls AddDefinition
  this->WarnUnused = false;
  this->CheckSystemVars = false;

  // Setup the default include file regular expression (match everything).
  this->IncludeFileRegularExpression = "^.*$";
  // Setup the default include complaint regular expression (match nothing).
  this->ComplainFileRegularExpression = "^$";
  // Source and header file extensions that we can handle

  // Set up a list of source and header extensions
  // these are used to find files when the extension
  // is not given
  // The "c" extension MUST precede the "C" extension.
  this->SourceFileExtensions.push_back( "c" );
  this->SourceFileExtensions.push_back( "C" );

  this->SourceFileExtensions.push_back( "c++" );
  this->SourceFileExtensions.push_back( "cc" );
  this->SourceFileExtensions.push_back( "cpp" );
  this->SourceFileExtensions.push_back( "cxx" );
  this->SourceFileExtensions.push_back( "m" );
  this->SourceFileExtensions.push_back( "M" );
  this->SourceFileExtensions.push_back( "mm" );

  this->HeaderFileExtensions.push_back( "h" );
  this->HeaderFileExtensions.push_back( "hh" );
  this->HeaderFileExtensions.push_back( "h++" );
  this->HeaderFileExtensions.push_back( "hm" );
  this->HeaderFileExtensions.push_back( "hpp" );
  this->HeaderFileExtensions.push_back( "hxx" );
  this->HeaderFileExtensions.push_back( "in" );
  this->HeaderFileExtensions.push_back( "txx" );

  this->DefineFlags = " ";
  this->LocalGenerator = 0;

  this->AddDefaultDefinitions();
  this->Initialize();
  this->PreOrder = false;
  this->GeneratingBuildSystem = false;
}

cmMakefile::cmMakefile(const cmMakefile& mf): Internal(new Internals)
{
  this->Internal->VarStack.push(mf.Internal->VarStack.top().Closure());
  this->Internal->VarInitStack.push(mf.Internal->VarInitStack.top());
  this->Internal->VarUsageStack.push(mf.Internal->VarUsageStack.top());

  this->Prefix = mf.Prefix;
  this->AuxSourceDirectories = mf.AuxSourceDirectories;
  this->cmStartDirectory = mf.cmStartDirectory;
  this->StartOutputDirectory = mf.StartOutputDirectory;
  this->cmHomeDirectory = mf.cmHomeDirectory;
  this->HomeOutputDirectory = mf.HomeOutputDirectory;
  this->cmCurrentListFile = mf.cmCurrentListFile;
  this->ProjectName = mf.ProjectName;
  this->Targets = mf.Targets;
  this->SourceFiles = mf.SourceFiles;
  this->Tests = mf.Tests;
  this->LinkDirectories = mf.LinkDirectories;
  this->SystemIncludeDirectories = mf.SystemIncludeDirectories;
  this->ListFiles = mf.ListFiles;
  this->OutputFiles = mf.OutputFiles;
  this->LinkLibraries = mf.LinkLibraries;
  this->InstallGenerators = mf.InstallGenerators;
  this->TestGenerators = mf.TestGenerators;
  this->IncludeFileRegularExpression = mf.IncludeFileRegularExpression;
  this->ComplainFileRegularExpression = mf.ComplainFileRegularExpression;
  this->SourceFileExtensions = mf.SourceFileExtensions;
  this->HeaderFileExtensions = mf.HeaderFileExtensions;
  this->DefineFlags = mf.DefineFlags;
  this->DefineFlagsOrig = mf.DefineFlagsOrig;

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->SourceGroups = mf.SourceGroups;
#endif

  this->LocalGenerator = mf.LocalGenerator;
  this->FunctionBlockers = mf.FunctionBlockers;
  this->MacrosMap = mf.MacrosMap;
  this->SubDirectoryOrder = mf.SubDirectoryOrder;
  this->Properties = mf.Properties;
  this->PreOrder = mf.PreOrder;
  this->WarnUnused = mf.WarnUnused;
  this->Initialize();
  this->CheckSystemVars = mf.CheckSystemVars;
  this->ListFileStack = mf.ListFileStack;
  this->OutputToSource = mf.OutputToSource;
}

//----------------------------------------------------------------------------
void cmMakefile::Initialize()
{
  this->cmDefineRegex.compile("#cmakedefine[ \t]+([A-Za-z_0-9]*)");
  this->cmDefine01Regex.compile("#cmakedefine01[ \t]+([A-Za-z_0-9]*)");
  this->cmAtVarRegex.compile("(@[A-Za-z_0-9/.+-]+@)");

  // Enter a policy level for this directory.
  this->PushPolicy();

  // Protect the directory-level policies.
  this->PushPolicyBarrier();

  // By default the check is not done.  It is enabled by
  // cmListFileCache in the top level if necessary.
  this->CheckCMP0000 = false;
}

unsigned int cmMakefile::GetCacheMajorVersion()
{
  return this->GetCacheManager()->GetCacheMajorVersion();
}

unsigned int cmMakefile::GetCacheMinorVersion()
{
  return this->GetCacheManager()->GetCacheMinorVersion();
}

bool cmMakefile::NeedCacheCompatibility(int major, int minor)
{
  return this->GetCacheManager()->NeedCacheCompatibility(major, minor);
}

cmMakefile::~cmMakefile()
{
  for(std::vector<cmInstallGenerator*>::iterator
        i = this->InstallGenerators.begin();
      i != this->InstallGenerators.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmTestGenerator*>::iterator
        i = this->TestGenerators.begin();
      i != this->TestGenerators.end(); ++i)
    {
    delete *i;
    }
  for(std::vector<cmSourceFile*>::iterator i = this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
    {
    delete *i;
    }
  for(std::map<cmStdString, cmTest*>::iterator i = this->Tests.begin();
      i != this->Tests.end(); ++i)
    {
    delete i->second;
    }
  for(std::vector<cmTarget*>::iterator
        i = this->ImportedTargetsOwned.begin();
      i != this->ImportedTargetsOwned.end(); ++i)
    {
    delete *i;
    }
  for(unsigned int i=0; i < this->FinalPassCommands.size(); i++)
    {
    delete this->FinalPassCommands[i];
    }
  std::vector<cmFunctionBlocker*>::iterator pos;
  for (pos = this->FunctionBlockers.begin();
       pos != this->FunctionBlockers.end(); ++pos)
    {
    cmFunctionBlocker* b = *pos;
    delete b;
    }
  this->FunctionBlockers.clear();
  if (this->PolicyStack.size() != 1)
  {
    cmSystemTools::Error("Internal CMake Error, Policy Stack has not been"
      " popped properly");
  }
}

void cmMakefile::PrintStringVector(const char* s,
                                   const std::vector<std::string>& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::string>::const_iterator i = v.begin();
      i != v.end(); ++i)
    {
    std::cout << (*i).c_str() << " ";
    }
  std::cout << " )\n";
}

void cmMakefile
::PrintStringVector(const char* s,
                    const std::vector<std::pair<cmStdString, bool> >& v) const
{
  std::cout << s << ": ( \n";
  for(std::vector<std::pair<cmStdString, bool> >::const_iterator i
        = v.begin(); i != v.end(); ++i)
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

  std::cout << " this->Targets: ";
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); l++)
    {
    std::cout << l->first << std::endl;
    }

  std::cout << " this->StartOutputDirectory; " <<
    this->StartOutputDirectory.c_str() << std::endl;
  std::cout << " this->HomeOutputDirectory; " <<
    this->HomeOutputDirectory.c_str() << std::endl;
  std::cout << " this->cmStartDirectory; " <<
    this->cmStartDirectory.c_str() << std::endl;
  std::cout << " this->cmHomeDirectory; " <<
    this->cmHomeDirectory.c_str() << std::endl;
  std::cout << " this->ProjectName; "
            <<  this->ProjectName.c_str() << std::endl;
  this->PrintStringVector("this->LinkDirectories", this->LinkDirectories);
#if defined(CMAKE_BUILD_WITH_CMAKE)
  for( std::vector<cmSourceGroup>::const_iterator i =
         this->SourceGroups.begin(); i != this->SourceGroups.end(); ++i)
    {
    std::cout << "Source Group: " << i->GetName() << std::endl;
    }
#endif
}

bool cmMakefile::CommandExists(const char* name) const
{
  return this->GetCMakeInstance()->CommandExists(name);
}


//----------------------------------------------------------------------------
void cmMakefile::IssueMessage(cmake::MessageType t,
                              std::string const& text) const
{
  // Collect context information.
  cmListFileBacktrace backtrace;
  if(!this->CallStack.empty())
    {
    if((t == cmake::FATAL_ERROR) || (t == cmake::INTERNAL_ERROR))
      {
      this->CallStack.back().Status->SetNestedError(true);
      }
    this->GetBacktrace(backtrace);
    }
  else
    {
    cmListFileContext lfc;
    if(this->ListFileStack.empty())
      {
      // We are not processing the project.  Add the directory-level context.
      lfc.FilePath = this->GetCurrentDirectory();
      lfc.FilePath += "/CMakeLists.txt";
      }
    else
      {
      // We are processing the project but are not currently executing a
      // command.  Add whatever context information we have.
      lfc.FilePath = this->ListFileStack.back();
      }
    lfc.Line = 0;
    if(!this->GetCMakeInstance()->GetIsInTryCompile())
      {
      lfc.FilePath = this->LocalGenerator->Convert(lfc.FilePath.c_str(),
                                                   cmLocalGenerator::HOME);
      }
    backtrace.push_back(lfc);
    }

  // Issue the message.
  this->GetCMakeInstance()->IssueMessage(t, text, backtrace);
}

//----------------------------------------------------------------------------
bool cmMakefile::GetBacktrace(cmListFileBacktrace& backtrace) const
{
  if(this->CallStack.empty())
    {
    return false;
    }
  for(CallStackType::const_reverse_iterator i = this->CallStack.rbegin();
      i != this->CallStack.rend(); ++i)
    {
    cmListFileContext lfc = *(*i).Context;
    lfc.FilePath = this->LocalGenerator->Convert(lfc.FilePath.c_str(),
                                                 cmLocalGenerator::HOME);
    backtrace.push_back(lfc);
    }
  return true;
}

//----------------------------------------------------------------------------
void cmMakefile::PrintCommandTrace(const cmListFileFunction& lff)
{
  cmOStringStream msg;
  msg << lff.FilePath << "(" << lff.Line << "):  ";
  msg << lff.Name << "(";
  for(std::vector<cmListFileArgument>::const_iterator i =
        lff.Arguments.begin(); i != lff.Arguments.end(); ++i)
    {
    msg << i->Value;
    msg << " ";
    }
  msg << ")";
  cmSystemTools::Message(msg.str().c_str());
}

//----------------------------------------------------------------------------
bool cmMakefile::ExecuteCommand(const cmListFileFunction& lff,
                                cmExecutionStatus &status)
{
  bool result = true;

  // quick return if blocked
  if(this->IsFunctionBlocked(lff,status))
    {
    // No error.
    return result;
    }

  std::string name = lff.Name;

  // Place this call on the call stack.
  cmMakefileCall stack_manager(this, lff, status);
  static_cast<void>(stack_manager);

  // Lookup the command prototype.
  if(cmCommand* proto = this->GetCMakeInstance()->GetCommand(name.c_str()))
    {
    // Clone the prototype.
    cmsys::auto_ptr<cmCommand> pcmd(proto->Clone());
    pcmd->SetMakefile(this);

    // Decide whether to invoke the command.
    if(pcmd->GetEnabled() && !cmSystemTools::GetFatalErrorOccured()  &&
       (this->GetCMakeInstance()->GetWorkingMode() != cmake::SCRIPT_MODE
       || pcmd->IsScriptable()))

      {
      // if trace is enabled, print out invoke information
      if(this->GetCMakeInstance()->GetTrace())
        {
        this->PrintCommandTrace(lff);
        }
      // Try invoking the command.
      if(!pcmd->InvokeInitialPass(lff.Arguments,status) ||
         status.GetNestedError())
        {
        if(!status.GetNestedError())
          {
          // The command invocation requested that we report an error.
          this->IssueMessage(cmake::FATAL_ERROR, pcmd->GetError());
          }
        result = false;
        if ( this->GetCMakeInstance()->GetWorkingMode() != cmake::NORMAL_MODE)
          {
          cmSystemTools::SetFatalErrorOccured();
          }
        }
      else if(pcmd->HasFinalPass())
        {
        // use the command
        this->FinalPassCommands.push_back(pcmd.release());
        }
      }
    else if ( this->GetCMakeInstance()->GetWorkingMode() == cmake::SCRIPT_MODE
              && !pcmd->IsScriptable() )
      {
      std::string error = "Command ";
      error += pcmd->GetName();
      error += "() is not scriptable";
      this->IssueMessage(cmake::FATAL_ERROR, error);
      result = false;
      cmSystemTools::SetFatalErrorOccured();
      }
    }
  else
    {
    if(!cmSystemTools::GetFatalErrorOccured())
      {
      std::string error = "Unknown CMake command \"";
      error += lff.Name;
      error += "\".";
      this->IssueMessage(cmake::FATAL_ERROR, error);
      result = false;
      cmSystemTools::SetFatalErrorOccured();
      }
    }

  return result;
}

//----------------------------------------------------------------------------
class cmMakefile::IncludeScope
{
public:
  IncludeScope(cmMakefile* mf, const char* fname, bool noPolicyScope);
  ~IncludeScope();
  void Quiet() { this->ReportError = false; }
private:
  cmMakefile* Makefile;
  const char* File;
  bool NoPolicyScope;
  bool CheckCMP0011;
  bool ReportError;
  void EnforceCMP0011();
};

//----------------------------------------------------------------------------
cmMakefile::IncludeScope::IncludeScope(cmMakefile* mf, const char* fname,
                                       bool noPolicyScope):
  Makefile(mf), File(fname), NoPolicyScope(noPolicyScope),
  CheckCMP0011(false), ReportError(true)
{
  if(!this->NoPolicyScope)
    {
    // Check CMP0011 to determine the policy scope type.
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0011))
      {
      case cmPolicies::WARN:
        // We need to push a scope to detect whether the script sets
        // any policies that would affect the includer and therefore
        // requires a warning.  We use a weak scope to simulate OLD
        // behavior by allowing policy changes to affect the includer.
        this->Makefile->PushPolicy(true);
        this->CheckCMP0011 = true;
        break;
      case cmPolicies::OLD:
        // OLD behavior is to not push a scope at all.
        this->NoPolicyScope = true;
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        // We should never make this policy required, but we handle it
        // here just in case.
        this->CheckCMP0011 = true;
      case cmPolicies::NEW:
        // NEW behavior is to push a (strong) scope.
        this->Makefile->PushPolicy();
        break;
      }
    }

  // The included file cannot pop our policy scope.
  this->Makefile->PushPolicyBarrier();
}

//----------------------------------------------------------------------------
cmMakefile::IncludeScope::~IncludeScope()
{
  // Enforce matching policy scopes inside the included file.
  this->Makefile->PopPolicyBarrier(this->ReportError);

  if(!this->NoPolicyScope)
    {
    // If we need to enforce policy CMP0011 then the top entry is the
    // one we pushed above.  If the entry is empty, then the included
    // script did not set any policies that might affect the includer so
    // we do not need to enforce the policy.
    if(this->CheckCMP0011 && this->Makefile->PolicyStack.back().empty())
      {
      this->CheckCMP0011 = false;
      }

    // Pop the scope we pushed for the script.
    this->Makefile->PopPolicy();

    // We enforce the policy after the script's policy stack entry has
    // been removed.
    if(this->CheckCMP0011)
      {
      this->EnforceCMP0011();
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefile::IncludeScope::EnforceCMP0011()
{
  // We check the setting of this policy again because the included
  // script might actually set this policy for its includer.
  cmPolicies* policies = this->Makefile->GetPolicies();
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0011))
    {
    case cmPolicies::WARN:
      // Warn because the user did not set this policy.
      {
      cmOStringStream w;
      w << policies->GetPolicyWarning(cmPolicies::CMP0011) << "\n"
        << "The included script\n  " << this->File << "\n"
        << "affects policy settings.  "
        << "CMake is implying the NO_POLICY_SCOPE option for compatibility, "
        << "so the effects are applied to the including context.";
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      {
      cmOStringStream e;
      e << policies->GetRequiredPolicyError(cmPolicies::CMP0011) << "\n"
        << "The included script\n  " << this->File << "\n"
        << "affects policy settings, so it requires this policy to be set.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      }
      break;
    case cmPolicies::OLD:
    case cmPolicies::NEW:
      // The script set this policy.  We assume the purpose of the
      // script is to initialize policies for its includer, and since
      // the policy is now set for later scripts, we do not warn.
      break;
    }
}

//----------------------------------------------------------------------------
// Parse the given CMakeLists.txt file executing all commands
//
bool cmMakefile::ReadListFile(const char* filename_in,
                              const char *external_in,
                              std::string* fullPath,
                              bool noPolicyScope)
{
  std::string currentParentFile
    = this->GetSafeDefinition("CMAKE_PARENT_LIST_FILE");
  std::string currentFile
    = this->GetSafeDefinition("CMAKE_CURRENT_LIST_FILE");
  this->AddDefinition("CMAKE_PARENT_LIST_FILE", filename_in);
  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");

  const char* external = 0;
  std::string external_abs;

  const char* filename = filename_in;
  std::string filename_abs;

  if (external_in)
    {
    external_abs =
      cmSystemTools::CollapseFullPath(external_in,
                                      this->cmStartDirectory.c_str());
    external = external_abs.c_str();
    if (filename_in)
      {
      filename_abs =
        cmSystemTools::CollapseFullPath(filename_in,
                                        this->cmStartDirectory.c_str());
      filename = filename_abs.c_str();
      }
    }

  // keep track of the current file being read
  if (filename)
    {
    if(this->cmCurrentListFile != filename)
      {
      this->cmCurrentListFile = filename;
      }
    }

  // Now read the input file
  const char *filenametoread= filename;

  if( external)
    {
    filenametoread= external;
    }

  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", filenametoread);
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                       cmSystemTools::GetFilenamePath(filenametoread).c_str());
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");

  // try to see if the list file is the top most
  // list file for a project, and if it is, then it
  // must have a project command.   If there is not
  // one, then cmake will provide one via the
  // cmListFileCache class.
  bool requireProjectCommand = false;
  if(!external && this->cmStartDirectory == this->cmHomeDirectory)
    {
    if(cmSystemTools::LowerCase(
      cmSystemTools::GetFilenameName(filename)) == "cmakelists.txt")
      {
      requireProjectCommand = true;
      }
    }

  // push the listfile onto the stack
  this->ListFileStack.push_back(filenametoread);
  if(fullPath!=0)
    {
    *fullPath=filenametoread;
    }
  cmListFile cacheFile;
  if( !cacheFile.ParseFile(filenametoread, requireProjectCommand, this) )
    {
    // pop the listfile off the stack
    this->ListFileStack.pop_back();
    if(fullPath!=0)
      {
      *fullPath = "";
      }
    this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
    this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
    this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
    this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
    this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                        cmSystemTools::GetFilenamePath(currentFile).c_str());
    this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");
    return false;
    }
  // add this list file to the list of dependencies
  this->ListFiles.push_back( filenametoread);

  // Enforce balanced blocks (if/endif, function/endfunction, etc.).
  {
  LexicalPushPop lexScope(this);
  IncludeScope incScope(this, filenametoread, noPolicyScope);

  // Run the parsed commands.
  const size_t numberFunctions = cacheFile.Functions.size();
  for(size_t i =0; i < numberFunctions; ++i)
    {
    cmExecutionStatus status;
    this->ExecuteCommand(cacheFile.Functions[i],status);
    if(cmSystemTools::GetFatalErrorOccured())
      {
      // Exit early due to error.
      lexScope.Quiet();
      incScope.Quiet();
      break;
      }
    if(status.GetReturnInvoked())
      {
      // Exit early due to return command.
      break;
      }
    }
  }

  // If this is the directory-level CMakeLists.txt file then perform
  // some extra checks.
  if(this->ListFileStack.size() == 1)
    {
    this->EnforceDirectoryLevelRules();
    }

  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                      cmSystemTools::GetFilenamePath(currentFile).c_str());
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");

  // pop the listfile off the stack
  this->ListFileStack.pop_back();

  // Check for unused variables
  this->CheckForUnusedVariables();

  return true;
}

//----------------------------------------------------------------------------
void cmMakefile::EnforceDirectoryLevelRules()
{
  // Diagnose a violation of CMP0000 if necessary.
  if(this->CheckCMP0000)
    {
    cmOStringStream msg;
    msg << "No cmake_minimum_required command is present.  "
        << "A line of code such as\n"
        << "  cmake_minimum_required(VERSION "
        << cmVersion::GetMajorVersion() << "."
        << cmVersion::GetMinorVersion()
        << ")\n"
        << "should be added at the top of the file.  "
        << "The version specified may be lower if you wish to "
        << "support older CMake versions for this project.  "
        << "For more information run "
        << "\"cmake --help-policy CMP0000\".";
    switch (this->GetPolicyStatus(cmPolicies::CMP0000))
      {
      case cmPolicies::WARN:
        // Warn because the user did not provide a mimimum required
        // version.
        this->IssueMessage(cmake::AUTHOR_WARNING, msg.str().c_str());
      case cmPolicies::OLD:
        // OLD behavior is to use policy version 2.4 set in
        // cmListFileCache.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // NEW behavior is to issue an error.
        this->IssueMessage(cmake::FATAL_ERROR, msg.str().c_str());
        cmSystemTools::SetFatalErrorOccured();
        return;
      }
    }
}

void cmMakefile::AddCommand(cmCommand* wg)
{
  this->GetCMakeInstance()->AddCommand(wg);
}

// Set the make file
void cmMakefile::SetLocalGenerator(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  // the source groups need to access the global generator
  // so don't create them until the lg is set
#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup
    ("Source Files",
     "\\.(C|M|c|c\\+\\+|cc|cpp|cxx|f|f90|for|fpp"
     "|ftn|m|mm|rc|def|r|odl|idl|hpj|bat)$");
  this->AddSourceGroup("Header Files", CM_HEADER_REGEX);
  this->AddSourceGroup("CMake Rules", "\\.rule$");
  this->AddSourceGroup("Resources", "\\.plist$");
  this->AddSourceGroup("Object Files", "\\.(lo|o|obj)$");
#endif

  this->WarnUnused = this->GetCMakeInstance()->GetWarnUnused();
  this->CheckSystemVars = this->GetCMakeInstance()->GetCheckSystemVars();
}

bool cmMakefile::NeedBackwardsCompatibility(unsigned int major,
                                            unsigned int minor,
                                            unsigned int patch)
{
  if(this->LocalGenerator)
    {
    return
      this->LocalGenerator->NeedBackwardsCompatibility(major, minor, patch);
    }
  else
    {
    return false;
    }
}


namespace
{
  struct file_not_persistent
  {
    bool operator()(const std::string& path) const
      {
      return !(path.find("CMakeTmp") == path.npos &&
               cmSystemTools::FileExists(path.c_str()));
      }
  };
}

void cmMakefile::FinalPass()
{
  // do all the variable expansions here
  this->ExpandVariablesCMP0019();

  // give all the commands a chance to do something
  // after the file has been parsed before generation
  for(std::vector<cmCommand*>::iterator i = this->FinalPassCommands.begin();
      i != this->FinalPassCommands.end(); ++i)
    {
    (*i)->FinalPass();
    }

  //go through all configured files and see which ones still exist.
  //we don't want cmake to re-run if a configured file is created and deleted
  //during processing as that would make it a transient file that can't
  //influence the build process

  //remove_if will move all items that don't have a valid file name to the
  //back of the vector
  std::vector<std::string>::iterator new_end = std::remove_if(
                                                this->OutputFiles.begin(),
                                                this->OutputFiles.end(),
                                                file_not_persistent());
  //we just have to erase all items at the back
  this->OutputFiles.erase(new_end, this->OutputFiles.end() );
}

// Generate the output file
void cmMakefile::ConfigureFinalPass()
{
  this->FinalPass();
  const char* oldValue
    = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (oldValue && atof(oldValue) <= 1.2)
    {
    cmSystemTools::Error("You have requested backwards compatibility "
                         "with CMake version 1.2 or earlier. This version "
                         "of CMake only supports backwards compatibility "
                         "with CMake 1.4 or later. For compatibility with "
                         "1.2 or earlier please use CMake 2.0");
    }
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); l++)
    {
    l->second.FinishConfigure();
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToTarget(const char* target,
                                     const std::vector<std::string>& depends,
                                     const cmCustomCommandLines& commandLines,
                                     cmTarget::CustomCommandType type,
                                     const char* comment,
                                     const char* workingDir,
                                     bool escapeOldStyle)
{
  // Find the target to which to add the custom command.
  cmTargets::iterator ti = this->Targets.find(target);
  if(ti != this->Targets.end())
    {
    if(ti->second.GetType() == cmTarget::OBJECT_LIBRARY)
      {
      cmOStringStream e;
      e << "Target \"" << target << "\" is an OBJECT library "
        "that may not have PRE_BUILD, PRE_LINK, or POST_BUILD commands.";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    // Add the command to the appropriate build step for the target.
    std::vector<std::string> no_output;
    cmCustomCommand cc(this, no_output, depends,
                       commandLines, comment, workingDir);
    cc.SetEscapeOldStyle(escapeOldStyle);
    cc.SetEscapeAllowMakeVars(true);
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
    }
}

//----------------------------------------------------------------------------
cmSourceFile*
cmMakefile::AddCustomCommandToOutput(const std::vector<std::string>& outputs,
                                     const std::vector<std::string>& depends,
                                     const char* main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle)
{
  // Make sure there is at least one output.
  if(outputs.empty())
    {
    cmSystemTools::Error("Attempt to add a custom rule with no output!");
    return 0;
    }

  // Validate custom commands.  TODO: More strict?
  for(cmCustomCommandLines::const_iterator i=commandLines.begin();
      i != commandLines.end(); ++i)
    {
    cmCustomCommandLine const& cl = *i;
    if(!cl.empty() && !cl[0].empty() && cl[0][0] == '"')
      {
      cmOStringStream e;
      e << "COMMAND may not contain literal quotes:\n  " << cl[0] << "\n";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return 0;
      }
    }

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
        return file;
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
    cmGlobalGenerator* gg = this->LocalGenerator->GetGlobalGenerator();

    // Construct a rule file associated with the first output produced.
    std::string outName = gg->GenerateRuleFile(outputs[0]);

    // Check if the rule file already exists.
    file = this->GetSource(outName.c_str());
    if(file && file->GetCustomCommand() && !replace)
      {
      // The rule file already exists.
      if(commandLines != file->GetCustomCommand()->GetCommandLines())
        {
        cmSystemTools::Error("Attempt to add a custom rule to output \"",
                             outName.c_str(),
                             "\" which already has a custom rule.");
        }
      return file;
      }

    // Create a cmSourceFile for the rule file.
    file = this->GetOrCreateSource(outName.c_str(), true);
    file->SetProperty("__CMAKE_RULE", "1");
    }

  // Always create the output sources and mark them generated.
  for(std::vector<std::string>::const_iterator o = outputs.begin();
      o != outputs.end(); ++o)
    {
    if(cmSourceFile* out = this->GetOrCreateSource(o->c_str(), true))
      {
      out->SetProperty("GENERATED", "1");
      }
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
      new cmCustomCommand(this, outputs, depends2, commandLines,
                          comment, workingDir);
    cc->SetEscapeOldStyle(escapeOldStyle);
    cc->SetEscapeAllowMakeVars(true);
    file->SetCustomCommand(cc);
    this->UpdateOutputToSourceMap(outputs, file);
    }
  return file;
}

//----------------------------------------------------------------------------
void
cmMakefile::UpdateOutputToSourceMap(std::vector<std::string> const& outputs,
                                    cmSourceFile* source)
{
  for(std::vector<std::string>::const_iterator o = outputs.begin();
      o != outputs.end(); ++o)
    {
    this->UpdateOutputToSourceMap(*o, source);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::UpdateOutputToSourceMap(std::string const& output,
                                    cmSourceFile* source)
{
  this->OutputToSource[output] = source;
}

//----------------------------------------------------------------------------
cmSourceFile*
cmMakefile::AddCustomCommandToOutput(const char* output,
                                     const std::vector<std::string>& depends,
                                     const char* main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle)
{
  std::vector<std::string> outputs;
  outputs.push_back(output);
  return this->AddCustomCommandToOutput(outputs, depends, main_dependency,
                                        commandLines, comment, workingDir,
                                        replace, escapeOldStyle);
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
                                   cmTarget::POST_BUILD, comment, 0);
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
    cmSourceFile* sf;

    // Choose whether to use a main dependency.
    if(sourceFiles.find(source))
      {
      // The source looks like a real file.  Use it as the main dependency.
      sf = this->AddCustomCommandToOutput(output, depends, source,
                                          commandLines, comment, 0);
      }
    else
      {
      // The source may not be a real file.  Do not use a main dependency.
      const char* no_main_dependency = 0;
      std::vector<std::string> depends2 = depends;
      depends2.push_back(source);
      sf = this->AddCustomCommandToOutput(output, depends2, no_main_dependency,
                                          commandLines, comment, 0);
      }

    // If the rule was added to the source (and not a .rule file),
    // then add the source to the target to make sure the rule is
    // included.
    if(sf && !sf->GetPropertyAsBool("__CMAKE_RULE"))
      {
      if (this->Targets.find(target) != this->Targets.end())
        {
        this->Targets[target].AddSourceFile(sf);
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
void cmMakefile::AddUtilityCommand(const char* utilityName,
                                   bool excludeFromAll,
                                   const std::vector<std::string>& depends,
                                   const char* workingDirectory,
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
  this->AddUtilityCommand(utilityName, excludeFromAll, workingDirectory,
                          depends, commandLines);
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddUtilityCommand(const char* utilityName,
                              bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle, const char* comment)
{
  // Create a target instance for this utility.
  cmTarget* target = this->AddNewTarget(cmTarget::UTILITY, utilityName);
  if (excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  if(!comment)
    {
    // Use an empty comment to avoid generation of default comment.
    comment = "";
    }

  // Store the custom command in the target.
  std::string force = this->GetStartOutputDirectory();
  force += cmake::GetCMakeFilesDirectory();
  force += "/";
  force += utilityName;
  const char* no_main_dependency = 0;
  bool no_replace = false;
  this->AddCustomCommandToOutput(force.c_str(), depends,
                                 no_main_dependency,
                                 commandLines, comment,
                                 workingDirectory, no_replace,
                                 escapeOldStyle);
  cmSourceFile* sf = target->AddSource(force.c_str());

  // The output is not actually created so mark it symbolic.
  if(sf)
    {
    sf->SetProperty("SYMBOLIC", "1");
    }
  else
    {
    cmSystemTools::Error("Could not get source file entry for ",
                         force.c_str());
    }
  return target;
}

void cmMakefile::AddDefineFlag(const char* flag)
{
  if (!flag)
    {
    return;
    }

  // Update the string used for the old DEFINITIONS property.
  this->AddDefineFlag(flag, this->DefineFlagsOrig);

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if(this->ParseDefineFlag(flag, false))
    {
    return;
    }

  // Add this flag that does not look like a definition.
  this->AddDefineFlag(flag, this->DefineFlags);
}

void cmMakefile::AddDefineFlag(const char* flag, std::string& dflags)
{
  // remove any \n\r
  std::string ret = flag;
  std::string::size_type pos = 0;
  while((pos = ret.find('\n', pos)) != std::string::npos)
    {
    ret[pos] = ' ';
    pos++;
    }
  pos = 0;
  while((pos = ret.find('\r', pos)) != std::string::npos)
    {
    ret[pos] = ' ';
    pos++;
    }

  dflags += " ";
  dflags += ret;
}


void cmMakefile::RemoveDefineFlag(const char* flag)
{
  // Check the length of the flag to remove.
  std::string::size_type len = strlen(flag);
  if(len < 1)
    {
    return;
    }

  // Update the string used for the old DEFINITIONS property.
  this->RemoveDefineFlag(flag, len, this->DefineFlagsOrig);

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if(this->ParseDefineFlag(flag, true))
    {
    return;
    }

  // Remove this flag that does not look like a definition.
  this->RemoveDefineFlag(flag, len, this->DefineFlags);
}

void cmMakefile::RemoveDefineFlag(const char* flag,
                                  std::string::size_type len,
                                  std::string& dflags)
{
  // Remove all instances of the flag that are surrounded by
  // whitespace or the beginning/end of the string.
  for(std::string::size_type lpos = dflags.find(flag, 0);
      lpos != std::string::npos; lpos = dflags.find(flag, lpos))
    {
    std::string::size_type rpos = lpos + len;
    if((lpos <= 0 || isspace(dflags[lpos-1])) &&
       (rpos >= dflags.size() || isspace(dflags[rpos])))
      {
      dflags.erase(lpos, len);
      }
    else
      {
      ++lpos;
      }
    }
}

void cmMakefile::AddCompileOption(const char* option)
{
  this->AppendProperty("COMPILE_OPTIONS", option);
}

bool cmMakefile::ParseDefineFlag(std::string const& def, bool remove)
{
  // Create a regular expression to match valid definitions.
  static cmsys::RegularExpression
    valid("^[-/]D[A-Za-z_][A-Za-z0-9_]*(=.*)?$");

  // Make sure the definition matches.
  if(!valid.find(def.c_str()))
    {
    return false;
    }

  // VS6 IDE does not support definition values with spaces in
  // combination with '"', '$', or ';'.
  if((strcmp(this->LocalGenerator->GetGlobalGenerator()->GetName(),
             "Visual Studio 6") == 0) &&
     (def.find(" ") != def.npos && def.find_first_of("\"$;") != def.npos))
    {
    return false;
    }

  // Definitions with non-trivial values require a policy check.
  static cmsys::RegularExpression
    trivial("^[-/]D[A-Za-z_][A-Za-z0-9_]*(=[A-Za-z0-9_.]+)?$");
  if(!trivial.find(def.c_str()))
    {
    // This definition has a non-trivial value.
    switch(this->GetPolicyStatus(cmPolicies::CMP0005))
      {
      case cmPolicies::WARN:
        this->IssueMessage(
          cmake::AUTHOR_WARNING,
          this->GetPolicies()->GetPolicyWarning(cmPolicies::CMP0005)
          );
      case cmPolicies::OLD:
        // OLD behavior is to not escape the value.  We should not
        // convert the definition to use the property.
        return false;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          cmake::FATAL_ERROR,
          this->GetPolicies()->GetRequiredPolicyError(cmPolicies::CMP0005)
          );
        return false;
      case cmPolicies::NEW:
        // NEW behavior is to escape the value.  Proceed to convert it
        // to an entry in the property.
        break;
      }
    }

  // Get the definition part after the flag.
  const char* define = def.c_str() + 2;

  if(remove)
    {
    if(const char* cdefs = this->GetProperty("COMPILE_DEFINITIONS"))
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(cdefs, defs);

      // Recompose the list without the definition.
      std::string ndefs;
      const char* sep = "";
      for(std::vector<std::string>::const_iterator di = defs.begin();
          di != defs.end(); ++di)
        {
        if(*di != define)
          {
          ndefs += sep;
          sep = ";";
          ndefs += *di;
          }
        }

      // Store the new list.
      this->SetProperty("COMPILE_DEFINITIONS", ndefs.c_str());
      }
    }
  else
    {
    // Append the definition to the directory property.
    this->AppendProperty("COMPILE_DEFINITIONS", define);
    }

  return true;
}

void cmMakefile::AddLinkLibrary(const char* lib,
                                cmTarget::LinkLibraryType llt)
{
  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back(tmp);
}

void cmMakefile::AddLinkLibraryForTarget(const char *target,
                                         const char* lib,
                                         cmTarget::LinkLibraryType llt)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    cmTarget* tgt =
      this->GetCMakeInstance()->GetGlobalGenerator()->FindTarget(0,lib);
    if(tgt)
      {
      // CMake versions below 2.4 allowed linking to modules.
      bool allowModules = this->NeedBackwardsCompatibility(2,2);
      // if it is not a static or shared library then you can not link to it
      if(!((tgt->GetType() == cmTarget::STATIC_LIBRARY) ||
           (tgt->GetType() == cmTarget::SHARED_LIBRARY) ||
           tgt->IsExecutableWithExports()))
        {
        cmOStringStream e;
        e << "Target \"" << lib << "\" of type "
          << cmTarget::GetTargetTypeName(tgt->GetType())
          << " may not be linked into another target.  "
          << "One may link only to STATIC or SHARED libraries, or "
          << "to executables with the ENABLE_EXPORTS property set.";
        // in older versions of cmake linking to modules was allowed
        if( tgt->GetType() == cmTarget::MODULE_LIBRARY )
          {
          e << "\n"
            << "If you are developing a new project, re-organize it to avoid "
            << "linking to modules.  "
            << "If you are just trying to build an existing project, "
            << "set CMAKE_BACKWARDS_COMPATIBILITY to 2.2 or lower to allow "
            << "linking to modules.";
          }
        // if no modules are allowed then this is always an error
        if(!allowModules ||
           // if we allow modules but the type is not a module then it is
           // still an error
           (allowModules && tgt->GetType() != cmTarget::MODULE_LIBRARY))
          {
          this->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
          }
        }
      }
    i->second.AddLinkLibrary( *this, target, lib, llt );
    }
  else
    {
    cmOStringStream e;
    e << "Attempt to add link library \""
      << lib << "\" to target \""
      << target << "\" which is not built in this directory.";
    this->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    }
}

void cmMakefile::AddLinkDirectoryForTarget(const char *target,
                                           const char* d)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    if(this->IsAlias(target))
      {
      cmOStringStream e;
      e << "ALIAS target \"" << target << "\" "
        << "may not be linked into another target.";
      this->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
      return;
      }
    i->second.AddLinkDirectory( d );
    }
  else
    {
    cmSystemTools::Error
      ("Attempt to add link directories to non-existent target: ",
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

  if(!dir)
    {
    return;
    }
  // remove trailing slashes
  if(dir[strlen(dir)-1] == '/')
    {
    std::string newdir = dir;
    newdir = newdir.substr(0, newdir.size()-1);
    if(std::find(this->LinkDirectories.begin(),
                 this->LinkDirectories.end(),
                 newdir.c_str()) == this->LinkDirectories.end())
      {
      this->LinkDirectories.push_back(newdir);
      }
    }
  else
    {
    if(std::find(this->LinkDirectories.begin(),
                 this->LinkDirectories.end(), dir)
       == this->LinkDirectories.end())
      {
      this->LinkDirectories.push_back(dir);
      }
    }
}

void cmMakefile::InitializeFromParent()
{
  cmMakefile *parent = this->LocalGenerator->GetParent()->GetMakefile();

  // Initialize definitions with the closure of the parent scope.
  this->Internal->VarStack.top() = parent->Internal->VarStack.top().Closure();

  const std::vector<cmValueWithOrigin> parentIncludes =
                                        parent->GetIncludeDirectoriesEntries();
  this->IncludeDirectoriesEntries.insert(this->IncludeDirectoriesEntries.end(),
                                       parentIncludes.begin(),
                                       parentIncludes.end());

  const std::vector<cmValueWithOrigin> parentOptions =
                                        parent->GetCompileOptionsEntries();
  this->CompileOptionsEntries.insert(this->CompileOptionsEntries.end(),
                                     parentOptions.begin(),
                                     parentOptions.end());

  const std::vector<cmValueWithOrigin> parentDefines =
                                      parent->GetCompileDefinitionsEntries();
  this->CompileDefinitionsEntries.insert(this->CompileDefinitionsEntries.end(),
                                     parentDefines.begin(),
                                     parentDefines.end());

  this->SystemIncludeDirectories = parent->SystemIncludeDirectories;

  // define flags
  this->DefineFlags = parent->DefineFlags;
  this->DefineFlagsOrig = parent->DefineFlagsOrig;

  // Include transform property.  There is no per-config version.
  {
  const char* prop = "IMPLICIT_DEPENDS_INCLUDE_TRANSFORM";
  this->SetProperty(prop, parent->GetProperty(prop));
  }

  // compile definitions property and per-config versions
  {
  this->SetProperty("COMPILE_DEFINITIONS",
                    parent->GetProperty("COMPILE_DEFINITIONS"));
  std::vector<std::string> configs;
  this->GetConfigurations(configs);
  for(std::vector<std::string>::const_iterator ci = configs.begin();
      ci != configs.end(); ++ci)
    {
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += cmSystemTools::UpperCase(*ci);
    this->SetProperty(defPropName.c_str(),
                      parent->GetProperty(defPropName.c_str()));
    }
  }

  // link libraries
  this->LinkLibraries = parent->LinkLibraries;

  // link directories
  this->LinkDirectories = parent->LinkDirectories;

  // the initial project name
  this->ProjectName = parent->ProjectName;

  // Copy include regular expressions.
  this->IncludeFileRegularExpression = parent->IncludeFileRegularExpression;
  this->ComplainFileRegularExpression = parent->ComplainFileRegularExpression;

  // Imported targets.
  this->ImportedTargets = parent->ImportedTargets;
}

void cmMakefile::ConfigureSubDirectory(cmLocalGenerator *lg2)
{
  // copy our variables from the child makefile
  lg2->GetMakefile()->InitializeFromParent();
  lg2->GetMakefile()->MakeStartDirectoriesCurrent();
  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Entering             ";
    msg += lg2->GetMakefile()->GetCurrentDirectory();
    cmSystemTools::Message(msg.c_str());
    }
  // finally configure the subdir
  lg2->Configure();
  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Returning to         ";
    msg += this->GetCurrentDirectory();
    cmSystemTools::Message(msg.c_str());
    }
}

void cmMakefile::AddSubDirectory(const char* sub,
                                 bool excludeFromAll, bool preorder)
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
                        excludeFromAll, preorder, false);
}


void cmMakefile::AddSubDirectory(const char* srcPath, const char *binPath,
                                 bool excludeFromAll, bool preorder,
                                 bool immediate)
{
  // Make sure the binary directory is unique.
  if(!this->EnforceUniqueDir(srcPath, binPath))
    {
    return;
    }

  // create a new local generator and set its parent
  cmLocalGenerator *lg2 =
    this->LocalGenerator->GetGlobalGenerator()->CreateLocalGenerator();
  lg2->SetParent(this->LocalGenerator);
  this->LocalGenerator->GetGlobalGenerator()->AddLocalGenerator(lg2);

  // set the subdirs start dirs
  lg2->GetMakefile()->SetStartDirectory(srcPath);
  lg2->GetMakefile()->SetStartOutputDirectory(binPath);
  if(excludeFromAll)
    {
    lg2->GetMakefile()->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  lg2->GetMakefile()->SetPreOrder(preorder);

  if (immediate)
    {
    this->ConfigureSubDirectory(lg2);
    }
}

//----------------------------------------------------------------------------
void cmMakefile::AddIncludeDirectories(const std::vector<std::string> &incs,
                                       bool before)
{
  if (incs.empty())
    {
    return;
    }

  std::string incString;
  std::string sep;

  for(std::vector<std::string>::const_iterator li = incs.begin();
      li != incs.end(); ++li)
    {
    incString += sep + *li;
    sep = ";";
    }

  std::vector<cmValueWithOrigin>::iterator position =
                              before ? this->IncludeDirectoriesEntries.begin()
                                    : this->IncludeDirectoriesEntries.end();

  cmListFileBacktrace lfbt;
  this->GetBacktrace(lfbt);
  cmValueWithOrigin entry(incString, lfbt);
  this->IncludeDirectoriesEntries.insert(position, entry);

  // Property on each target:
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); ++l)
    {
    cmTarget &t = l->second;
    t.InsertInclude(entry, before);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddSystemIncludeDirectories(const std::set<cmStdString> &incs)
{
  for(std::set<cmStdString>::const_iterator li = incs.begin();
      li != incs.end(); ++li)
    {
    this->SystemIncludeDirectories.insert(*li);
    }

  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); ++l)
    {
    cmTarget &t = l->second;
    t.AddSystemIncludeDirectories(incs);
    }
}

void cmMakefile::AddDefinition(const char* name, const char* value)
{
  if (!value )
    {
    return;
    }

#ifdef CMAKE_STRICT
  if (this->GetCMakeInstance())
    {
    this->GetCMakeInstance()->
      RecordPropertyAccess(name,cmProperty::VARIABLE);
    }
#endif

  this->Internal->VarStack.top().Set(name, value);
  if (this->Internal->VarUsageStack.size() &&
      this->VariableInitialized(name))
    {
    this->CheckForUnused("changing definition", name);
    this->Internal->VarUsageStack.top().erase(name);
    }
  this->Internal->VarInitStack.top().insert(name);

#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name,
                         cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
                         value,
                         this);
    }
#endif
}


void cmMakefile::AddCacheDefinition(const char* name, const char* value,
                                    const char* doc,
                                    cmCacheManager::CacheEntryType type,
                                    bool force)
{
  const char* val = value;
  cmCacheManager::CacheIterator it =
    this->GetCacheManager()->GetCacheIterator(name);
  if(!it.IsAtEnd() && (it.GetType() == cmCacheManager::UNINITIALIZED) &&
     it.Initialized())
    {
    // if this is not a force, then use the value from the cache
    // if it is a force, then use the value being passed in
    if(!force)
      {
      val = it.GetValue();
      }
    if ( type == cmCacheManager::PATH || type == cmCacheManager::FILEPATH )
      {
      std::vector<std::string>::size_type cc;
      std::vector<std::string> files;
      std::string nvalue = "";
      cmSystemTools::ExpandListArgument(val, files);
      for ( cc = 0; cc < files.size(); cc ++ )
        {
        if(!cmSystemTools::IsOff(files[cc].c_str()))
          {
          files[cc] = cmSystemTools::CollapseFullPath(files[cc].c_str());
          }
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
  // if there was a definition then remove it
  this->Internal->VarStack.top().Set(name, 0);
}


void cmMakefile::AddDefinition(const char* name, bool value)
{
  this->Internal->VarStack.top().Set(name, value? "ON" : "OFF");
  if (this->Internal->VarUsageStack.size() &&
      this->VariableInitialized(name))
    {
    this->CheckForUnused("changing definition", name);
    this->Internal->VarUsageStack.top().erase(name);
    }
  this->Internal->VarInitStack.top().insert(name);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
      value?"ON":"OFF", this);
    }
#endif
}

void cmMakefile::CheckForUnusedVariables() const
{
  if (!this->WarnUnused)
    {
    return;
    }
  const cmDefinitions& defs = this->Internal->VarStack.top();
  const std::set<cmStdString>& locals = defs.LocalKeys();
  std::set<cmStdString>::const_iterator it = locals.begin();
  for (; it != locals.end(); ++it)
    {
    this->CheckForUnused("out of scope", it->c_str());
    }
}

void cmMakefile::MarkVariableAsUsed(const char* var)
{
  this->Internal->VarUsageStack.top().insert(var);
}

bool cmMakefile::VariableInitialized(const char* var) const
{
  if(this->Internal->VarInitStack.top().find(var) !=
      this->Internal->VarInitStack.top().end())
    {
    return true;
    }
  return false;
}

bool cmMakefile::VariableUsed(const char* var) const
{
  if(this->Internal->VarUsageStack.top().find(var) !=
      this->Internal->VarUsageStack.top().end())
    {
    return true;
    }
  return false;
}

void cmMakefile::CheckForUnused(const char* reason, const char* name) const
{
  if (this->WarnUnused && !this->VariableUsed(name))
    {
    cmStdString path;
    cmListFileBacktrace bt;
    if (this->CallStack.size())
      {
      const cmListFileContext* file = this->CallStack.back().Context;
      bt.push_back(*file);
      path = file->FilePath.c_str();
      }
    else
      {
      path = this->GetStartDirectory();
      path += "/CMakeLists.txt";
      cmListFileContext lfc;
      lfc.FilePath = path;
      lfc.Line = 0;
      bt.push_back(lfc);
      }
    if (this->CheckSystemVars ||
        cmSystemTools::IsSubDirectory(path.c_str(),
                                      this->GetHomeDirectory()) ||
        (cmSystemTools::IsSubDirectory(path.c_str(),
                                      this->GetHomeOutputDirectory()) &&
        !cmSystemTools::IsSubDirectory(path.c_str(),
                                cmake::GetCMakeFilesDirectory())))
      {
      cmOStringStream msg;
      msg << "unused variable (" << reason << ") \'" << name << "\'";
      this->GetCMakeInstance()->IssueMessage(cmake::AUTHOR_WARNING,
                                             msg.str().c_str(),
                                             bt);
      }
    }
}

void cmMakefile::RemoveDefinition(const char* name)
{
  this->Internal->VarStack.top().Set(name, 0);
  if (this->Internal->VarUsageStack.size() &&
      this->VariableInitialized(name))
    {
    this->CheckForUnused("unsetting", name);
    this->Internal->VarUsageStack.top().erase(name);
    }
  this->Internal->VarInitStack.top().insert(name);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS,
      0, this);
    }
#endif
}

void cmMakefile::RemoveCacheDefinition(const char* name)
{
  this->GetCacheManager()->RemoveCacheEntry(name);
}

void cmMakefile::SetProjectName(const char* p)
{
  this->ProjectName = p;
}


void cmMakefile::AddGlobalLinkInformation(const char* name, cmTarget& target)
{
  // for these targets do not add anything
  switch(target.GetType())
    {
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
      return;
    default:;
    }
  std::vector<std::string>::iterator j;
  for(j = this->LinkDirectories.begin();
      j != this->LinkDirectories.end(); ++j)
    {
    target.AddLinkDirectory(j->c_str());
    }
  target.MergeLinkLibraries( *this, name, this->LinkLibraries );
}


void cmMakefile::AddAlias(const char* lname, cmTarget *tgt)
{
  this->AliasTargets[lname] = tgt;
  this->LocalGenerator->GetGlobalGenerator()->AddAlias(lname, tgt);
}

cmTarget* cmMakefile::AddLibrary(const char* lname, cmTarget::TargetType type,
                            const std::vector<std::string> &srcs,
                            bool excludeFromAll)
{
  // wrong type ? default to STATIC
  if (    (type != cmTarget::STATIC_LIBRARY)
       && (type != cmTarget::SHARED_LIBRARY)
       && (type != cmTarget::MODULE_LIBRARY)
       && (type != cmTarget::OBJECT_LIBRARY))
    {
    this->IssueMessage(cmake::INTERNAL_ERROR,
                       "cmMakefile::AddLibrary given invalid target type.");
    type = cmTarget::STATIC_LIBRARY;
    }

  cmTarget* target = this->AddNewTarget(type, lname);
  // Clear its dependencies. Otherwise, dependencies might persist
  // over changes in CMakeLists.txt, making the information stale and
  // hence useless.
  target->ClearDependencyInformation( *this, lname );
  if(excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(lname, *target);
  return target;
}

cmTarget* cmMakefile::AddExecutable(const char *exeName,
                                    const std::vector<std::string> &srcs,
                                    bool excludeFromAll)
{
  cmTarget* target = this->AddNewTarget(cmTarget::EXECUTABLE, exeName);
  if(excludeFromAll)
    {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(exeName, *target);
  return target;
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddNewTarget(cmTarget::TargetType type, const char* name)
{
  cmTargets::iterator it =
    this->Targets.insert(cmTargets::value_type(name, cmTarget())).first;
  cmTarget& target = it->second;
  target.SetType(type, name);
  target.SetMakefile(this);
  this->LocalGenerator->GetGlobalGenerator()->AddTarget(&it->second);
  return &it->second;
}

cmSourceFile *cmMakefile::LinearGetSourceFileWithOutput(const char *cname)
{
  std::string name = cname;
  std::string out;

  // look through all the source files that have custom commands
  // and see if the custom command has the passed source file as an output
  for(std::vector<cmSourceFile*>::const_iterator i =
        this->SourceFiles.begin(); i != this->SourceFiles.end(); ++i)
    {
    // does this source file have a custom command?
    if ((*i)->GetCustomCommand())
      {
      // is the output of the custom command match the source files name
      const std::vector<std::string>& outputs =
        (*i)->GetCustomCommand()->GetOutputs();
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        out = *o;
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
    }

  // otherwise return NULL
  return 0;
}

cmSourceFile *cmMakefile::GetSourceFileWithOutput(const char *cname)
{
  std::string name = cname;

  // If the queried path is not absolute we use the backward compatible
  // linear-time search for an output with a matching suffix.
  if(!cmSystemTools::FileIsFullPath(cname))
    {
    return LinearGetSourceFileWithOutput(cname);
    }
  // Otherwise we use an efficient lookup map.
  OutputToSourceMap::iterator o = this->OutputToSource.find(name);
  if (o != this->OutputToSource.end())
    {
    return (*o).second;
    }
  return 0;
}

#if defined(CMAKE_BUILD_WITH_CMAKE)
cmSourceGroup* cmMakefile::GetSourceGroup(const std::vector<std::string>&name)
{
  cmSourceGroup* sg = 0;

  // first look for source group starting with the same as the one we wants
  for (std::vector<cmSourceGroup>::iterator sgIt = this->SourceGroups.begin();
       sgIt != this->SourceGroups.end(); ++sgIt)

    {
    std::string sgName = sgIt->GetName();
    if(sgName == name[0])
      {
      sg = &(*sgIt);
      break;
      }
    }

  if(sg != 0)
    {
    // iterate through its children to find match source group
    for(unsigned int i=1; i<name.size(); ++i)
      {
      sg = sg->lookupChild(name[i].c_str());
      if(sg == 0)
        {
        break;
        }
      }
    }
  return sg;
}

 void cmMakefile::AddSourceGroup(const char* name,
                                 const char* regex)
{
  if (name)
    {
    std::vector<std::string> nameVector;
    nameVector.push_back(name);
    AddSourceGroup(nameVector, regex);
    }
}

void cmMakefile::AddSourceGroup(const std::vector<std::string>& name,
                                const char* regex)
{
  cmSourceGroup* sg = 0;
  std::vector<std::string> currentName;
  int i = 0;
  const int lastElement = static_cast<int>(name.size()-1);
  for(i=lastElement; i>=0; --i)
    {
    currentName.assign(name.begin(), name.begin()+i+1);
    sg = this->GetSourceGroup(currentName);
    if(sg != 0)
      {
      break;
      }
    }

  // i now contains the index of the last found component
  if(i==lastElement)
    {
    // group already exists, replace its regular expression
    if ( regex )
      {
      // We only want to set the regular expression.  If there are already
      // source files in the group, we don't want to remove them.
      sg->SetGroupRegex(regex);
      }
    return;
    }
  else if(i==-1)
    {
    // group does not exist nor belong to any existing group
    // add its first component
    this->SourceGroups.push_back(cmSourceGroup(name[0].c_str(), regex));
    sg = this->GetSourceGroup(currentName);
    i = 0; // last component found
    }

  // build the whole source group path
  const char* fullname = sg->GetFullName();
  cmGlobalGenerator* gg = this->LocalGenerator->GetGlobalGenerator();
  if(strlen(fullname))
    {
    std::string guidName = "SG_Filter_";
    guidName += fullname;
    gg->CreateGUID(guidName.c_str());
    }
  for(++i; i<=lastElement; ++i)
    {
    sg->AddChild(cmSourceGroup(name[i].c_str(), 0, sg->GetFullName()));
    sg = sg->lookupChild(name[i].c_str());
    fullname = sg->GetFullName();
    if(strlen(fullname))
      {
      std::string guidName = "SG_Filter_";
      guidName += fullname;
      gg->CreateGUID(guidName.c_str());
      }
    }

  sg->SetGroupRegex(regex);
}

#endif

void cmMakefile::AddExtraDirectory(const char* dir)
{
  this->AuxSourceDirectories.push_back(dir);
}

static bool mightExpandVariablesCMP0019(const char* s)
{
  return s && *s && strstr(s,"${") && strchr(s,'}');
}

void cmMakefile::ExpandVariablesCMP0019()
{
  // Drop this ancient compatibility behavior with a policy.
  cmPolicies::PolicyStatus pol = this->GetPolicyStatus(cmPolicies::CMP0019);
  if(pol != cmPolicies::OLD && pol != cmPolicies::WARN)
    {
    return;
    }
  cmOStringStream w;

  const char *includeDirs = this->GetProperty("INCLUDE_DIRECTORIES");
  if(mightExpandVariablesCMP0019(includeDirs))
    {
    std::string dirs = includeDirs;
    this->ExpandVariablesInString(dirs, true, true);
    if(pol == cmPolicies::WARN && dirs != includeDirs)
      {
      w << "Evaluated directory INCLUDE_DIRECTORIES\n"
        << "  " << includeDirs << "\n"
        << "as\n"
        << "  " << dirs << "\n";
      }
    this->SetProperty("INCLUDE_DIRECTORIES", dirs.c_str());
    }

  // Also for each target's INCLUDE_DIRECTORIES property:
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); ++l)
    {
    cmTarget &t = l->second;
    includeDirs = t.GetProperty("INCLUDE_DIRECTORIES");
    if(mightExpandVariablesCMP0019(includeDirs))
      {
      std::string dirs = includeDirs;
      this->ExpandVariablesInString(dirs, true, true);
      if(pol == cmPolicies::WARN && dirs != includeDirs)
        {
        w << "Evaluated target " << t.GetName() << " INCLUDE_DIRECTORIES\n"
          << "  " << includeDirs << "\n"
          << "as\n"
          << "  " << dirs << "\n";
        }
      t.SetProperty("INCLUDE_DIRECTORIES", dirs.c_str());
      }
    }

  for(std::vector<std::string>::iterator d = this->LinkDirectories.begin();
      d != this->LinkDirectories.end(); ++d)
    {
    if(mightExpandVariablesCMP0019(d->c_str()))
      {
      std::string orig = *d;
      this->ExpandVariablesInString(*d, true, true);
      if(pol == cmPolicies::WARN && *d != orig)
        {
        w << "Evaluated link directory\n"
          << "  " << orig << "\n"
          << "as\n"
          << "  " << *d << "\n";
        }
      }
    }
  for(cmTarget::LinkLibraryVectorType::iterator l =
        this->LinkLibraries.begin();
      l != this->LinkLibraries.end(); ++l)
    {
    if(mightExpandVariablesCMP0019(l->first.c_str()))
      {
      std::string orig = l->first;
      this->ExpandVariablesInString(l->first, true, true);
      if(pol == cmPolicies::WARN && l->first != orig)
        {
        w << "Evaluated link library\n"
          << "  " << orig << "\n"
          << "as\n"
          << "  " << l->first << "\n";
        }
      }
    }

  if(!w.str().empty())
    {
    cmOStringStream m;
    m << this->GetPolicies()->GetPolicyWarning(cmPolicies::CMP0019)
      << "\n"
      << "The following variable evaluations were encountered:\n"
      << w.str();
    this->IssueMessage(cmake::AUTHOR_WARNING, m.str());
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

bool cmMakefile::PlatformIs64Bit() const
{
  if(const char* sizeof_dptr = this->GetDefinition("CMAKE_SIZEOF_VOID_P"))
    {
    return atoi(sizeof_dptr) == 8;
    }
  return false;
}

const char* cmMakefile::GetSONameFlag(const char* language) const
{
  std::string name = "CMAKE_SHARED_LIBRARY_SONAME";
  if(language)
    {
    name += "_";
    name += language;
    }
  name += "_FLAG";
  return GetDefinition(name.c_str());
}

bool cmMakefile::CanIWriteThisFile(const char* fileName)
{
  if ( !this->IsOn("CMAKE_DISABLE_SOURCE_CHANGES") )
    {
    return true;
    }
  // If we are doing an in-source build, then the test will always fail
  if ( cmSystemTools::SameFile(this->GetHomeDirectory(),
                               this->GetHomeOutputDirectory()) )
    {
    if ( this->IsOn("CMAKE_DISABLE_IN_SOURCE_BUILD") )
      {
      return false;
      }
    return true;
    }

  // Check if this is a subdirectory of the source tree but not a
  // subdirectory of the build tree
  if ( cmSystemTools::IsSubDirectory(fileName,
      this->GetHomeDirectory()) &&
    !cmSystemTools::IsSubDirectory(fileName,
      this->GetHomeOutputDirectory()) )
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
    cmSystemTools::Error("Error required internal CMake variable not "
                         "set, cmake may be not be built correctly.\n",
                         "Missing variable is:\n",
                         name);
    return "";
    }
  return ret;
}

bool cmMakefile::IsDefinitionSet(const char* name) const
{
  const char* def = this->Internal->VarStack.top().Get(name);
  this->Internal->VarUsageStack.top().insert(name);
  if(!def)
    {
    def = this->GetCacheManager()->GetCacheValue(name);
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  if(cmVariableWatch* vv = this->GetVariableWatch())
    {
    if(!def)
      {
      vv->VariableAccessed
        (name, cmVariableWatch::UNKNOWN_VARIABLE_DEFINED_ACCESS,
         def, this);
      }
    }
#endif
  return def?true:false;
}

const char* cmMakefile::GetDefinition(const char* name) const
{
#ifdef CMAKE_STRICT
  if (this->GetCMakeInstance())
    {
    this->GetCMakeInstance()->
      RecordPropertyAccess(name,cmProperty::VARIABLE);
    }
#endif
  if (this->WarnUnused)
    {
    this->Internal->VarUsageStack.top().insert(name);
    }
  const char* def = this->Internal->VarStack.top().Get(name);
  if(!def)
    {
    def = this->GetCacheManager()->GetCacheValue(name);
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    if ( def )
      {
      vv->VariableAccessed(name, cmVariableWatch::VARIABLE_READ_ACCESS,
        def, this);
      }
    else
      {
      // are unknown access allowed
      const char* allow = this->Internal->VarStack.top()
        .Get("CMAKE_ALLOW_UNKNOWN_VARIABLE_READ_ACCESS");
      if(cmSystemTools::IsOn(allow))
        {
        vv->VariableAccessed(name,
          cmVariableWatch::ALLOWED_UNKNOWN_VARIABLE_READ_ACCESS, def, this);
        }
      else
        {
        vv->VariableAccessed(name,
          cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS, def, this);
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

std::vector<std::string> cmMakefile
::GetDefinitions(int cacheonly /* = 0 */) const
{
  std::set<cmStdString> definitions;
  if ( !cacheonly )
    {
    definitions = this->Internal->VarStack.top().ClosureKeys();
    }
  cmCacheManager::CacheIterator cit =
    this->GetCacheManager()->GetCacheIterator();
  for ( cit.Begin(); !cit.IsAtEnd(); cit.Next() )
    {
    definitions.insert(cit.GetName());
    }

  std::vector<std::string> res;

  std::set<cmStdString>::iterator fit;
  for ( fit = definitions.begin(); fit != definitions.end(); fit ++ )
    {
    res.push_back(*fit);
    }
  return res;
}


const char *cmMakefile::ExpandVariablesInString(std::string& source)
{
  return this->ExpandVariablesInString(source, false, false);
}

const char *cmMakefile::ExpandVariablesInString(std::string& source,
                                                bool escapeQuotes,
                                                bool noEscapes,
                                                bool atOnly,
                                                const char* filename,
                                                long line,
                                                bool removeEmpty,
                                                bool replaceAt)
{
  if ( source.empty() || source.find_first_of("$@\\") == source.npos)
    {
    return source.c_str();
    }

  // Special-case the @ONLY mode.
  if(atOnly)
    {
    if(!noEscapes || !removeEmpty || !replaceAt)
      {
      // This case should never be called.  At-only is for
      // configure-file/string which always does no escapes.
      this->IssueMessage(cmake::INTERNAL_ERROR,
                         "ExpandVariablesInString @ONLY called "
                         "on something with escapes.");
      }

    // Store an original copy of the input.
    std::string input = source;

    // Start with empty output.
    source = "";

    // Look for one @VAR@ at a time.
    const char* in = input.c_str();
    while(this->cmAtVarRegex.find(in))
      {
      // Get the range of the string to replace.
      const char* first = in + this->cmAtVarRegex.start();
      const char* last =  in + this->cmAtVarRegex.end();

      // Store the unchanged part of the string now.
      source.append(in, first-in);

      // Lookup the definition of VAR.
      std::string var(first+1, last-first-2);
      if(const char* val = this->GetDefinition(var.c_str()))
        {
        // Store the value in the output escaping as requested.
        if(escapeQuotes)
          {
          source.append(cmSystemTools::EscapeQuotes(val));
          }
        else
          {
          source.append(val);
          }
        }

      // Continue looking for @VAR@ further along the string.
      in = last;
      }

    // Append the rest of the unchanged part of the string.
    source.append(in);

    return source.c_str();
    }

  // This method replaces ${VAR} and @VAR@ where VAR is looked up
  // with GetDefinition(), if not found in the map, nothing is expanded.
  // It also supports the $ENV{VAR} syntax where VAR is looked up in
  // the current environment variables.

  cmCommandArgumentParserHelper parser;
  parser.SetMakefile(this);
  parser.SetLineFile(line, filename);
  parser.SetEscapeQuotes(escapeQuotes);
  parser.SetNoEscapeMode(noEscapes);
  parser.SetReplaceAtSyntax(replaceAt);
  parser.SetRemoveEmpty(removeEmpty);
  int res = parser.ParseString(source.c_str(), 0);
  const char* emsg = parser.GetError();
  if ( res && !emsg[0] )
    {
    source = parser.GetResult();
    }
  else
    {
    // Construct the main error message.
    cmOStringStream error;
    error << "Syntax error in cmake code ";
    if(filename && line > 0)
      {
      // This filename and line number may be more specific than the
      // command context because one command invocation can have
      // arguments on multiple lines.
      error << "at\n"
            << "  " << filename << ":" << line << "\n";
      }
    error << "when parsing string\n"
          << "  " << source.c_str() << "\n";
    error << emsg;

    // If the parser failed ("res" is false) then this is a real
    // argument parsing error, so the policy applies.  Otherwise the
    // parser reported an error message without failing because the
    // helper implementation is unhappy, which has always reported an
    // error.
    cmake::MessageType mtype = cmake::FATAL_ERROR;
    if(!res)
      {
      // This is a real argument parsing error.  Use policy CMP0010 to
      // decide whether it is an error.
      switch(this->GetPolicyStatus(cmPolicies::CMP0010))
        {
        case cmPolicies::WARN:
          error << "\n"
                << (this->GetPolicies()
                    ->GetPolicyWarning(cmPolicies::CMP0010));
        case cmPolicies::OLD:
          // OLD behavior is to just warn and continue.
          mtype = cmake::AUTHOR_WARNING;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          error << "\n"
                << (this->GetPolicies()
                    ->GetRequiredPolicyError(cmPolicies::CMP0010));
        case cmPolicies::NEW:
          // NEW behavior is to report the error.
          cmSystemTools::SetFatalErrorOccured();
          break;
        }
      }
    this->IssueMessage(mtype, error.str());
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
/* Up to CMake 2.4 here only WIN32, UNIX and APPLE were set.
  With CMake must separate between target and host platform. In most cases
  the tests for WIN32, UNIX and APPLE will be for the target system, so an
  additional set of variables for the host system is required ->
  CMAKE_HOST_WIN32, CMAKE_HOST_UNIX, CMAKE_HOST_APPLE.
  WIN32, UNIX and APPLE are now set in the platform files in
  Modules/Platforms/.
  To keep cmake scripts (-P) and custom language and compiler modules
  working, these variables are still also set here in this place, but they
  will be reset in CMakeSystemSpecificInformation.cmake before the platform
  files are executed. */
#if defined(_WIN32)
  this->AddDefinition("WIN32", "1");
  this->AddDefinition("CMAKE_HOST_WIN32", "1");
#else
  this->AddDefinition("UNIX", "1");
  this->AddDefinition("CMAKE_HOST_UNIX", "1");
#endif
#if defined(__CYGWIN__)
  if(cmSystemTools::IsOn(cmSystemTools::GetEnv("CMAKE_LEGACY_CYGWIN_WIN32")))
    {
    this->AddDefinition("WIN32", "1");
    this->AddDefinition("CMAKE_HOST_WIN32", "1");
    }
#endif
#if defined(__APPLE__)
  this->AddDefinition("APPLE", "1");
  this->AddDefinition("CMAKE_HOST_APPLE", "1");
#endif

  char temp[1024];
  sprintf(temp, "%d", cmVersion::GetMinorVersion());
  this->AddDefinition("CMAKE_MINOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetMajorVersion());
  this->AddDefinition("CMAKE_MAJOR_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetPatchVersion());
  this->AddDefinition("CMAKE_PATCH_VERSION", temp);
  sprintf(temp, "%d", cmVersion::GetTweakVersion());
  this->AddDefinition("CMAKE_TWEAK_VERSION", temp);
  this->AddDefinition("CMAKE_VERSION", cmVersion::GetCMakeVersion());

  this->AddDefinition("CMAKE_FILES_DIRECTORY",
                      cmake::GetCMakeFilesDirectory());
}

//----------------------------------------------------------------------------
const char*
cmMakefile::GetConfigurations(std::vector<std::string>& configs,
                              bool single) const
{
  if(this->LocalGenerator->GetGlobalGenerator()->IsMultiConfig())
    {
    if(const char* configTypes =
       this->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
      {
      cmSystemTools::ExpandListArgument(configTypes, configs);
      }
    return 0;
    }
  else
    {
    const char* buildType = this->GetDefinition("CMAKE_BUILD_TYPE");
    if(single && buildType && *buildType)
      {
      configs.push_back(buildType);
      }
    return buildType;
    }
}

#if defined(CMAKE_BUILD_WITH_CMAKE)
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
#endif

bool cmMakefile::IsFunctionBlocked(const cmListFileFunction& lff,
                                   cmExecutionStatus &status)
{
  // if there are no blockers get out of here
  if (this->FunctionBlockers.begin() == this->FunctionBlockers.end())
    {
    return false;
    }

  // loop over all function blockers to see if any block this command
  // evaluate in reverse, this is critical for balanced IF statements etc
  std::vector<cmFunctionBlocker*>::reverse_iterator pos;
  for (pos = this->FunctionBlockers.rbegin();
       pos != this->FunctionBlockers.rend(); ++pos)
    {
    if((*pos)->IsFunctionBlocked(lff, *this, status))
      {
      return true;
      }
    }

  return false;
}

//----------------------------------------------------------------------------
void cmMakefile::PushFunctionBlockerBarrier()
{
  this->FunctionBlockerBarriers.push_back(this->FunctionBlockers.size());
}

//----------------------------------------------------------------------------
void cmMakefile::PopFunctionBlockerBarrier(bool reportError)
{
  // Remove any extra entries pushed on the barrier.
  FunctionBlockersType::size_type barrier =
    this->FunctionBlockerBarriers.back();
  while(this->FunctionBlockers.size() > barrier)
    {
    cmsys::auto_ptr<cmFunctionBlocker> fb(this->FunctionBlockers.back());
    this->FunctionBlockers.pop_back();
    if(reportError)
      {
      // Report the context in which the unclosed block was opened.
      cmListFileContext const& lfc = fb->GetStartingContext();
      cmOStringStream e;
      e << "A logical block opening on the line\n"
        << "  " << lfc << "\n"
        << "is not closed.";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      reportError = false;
      }
    }

  // Remove the barrier.
  this->FunctionBlockerBarriers.pop_back();
}

bool cmMakefile::ExpandArguments(
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
    this->ExpandVariablesInString(value, false, false, false,
                                  i->FilePath, i->Line,
                                  false, true);

    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if(i->Delim == cmListFileArgument::Quoted)
      {
      outArgs.push_back(value);
      }
    else
      {
      cmSystemTools::ExpandListArgument(value, outArgs);
      }
    }
  return !cmSystemTools::GetFatalErrorOccured();
}

//----------------------------------------------------------------------------
void cmMakefile::AddFunctionBlocker(cmFunctionBlocker* fb)
{
  if(!this->CallStack.empty())
    {
    // Record the context in which the blocker is created.
    fb->SetStartingContext(*(this->CallStack.back().Context));
    }

  this->FunctionBlockers.push_back(fb);
}

cmsys::auto_ptr<cmFunctionBlocker>
cmMakefile::RemoveFunctionBlocker(cmFunctionBlocker* fb,
                                  const cmListFileFunction& lff)
{
  // Find the function blocker stack barrier for the current scope.
  // We only remove a blocker whose index is not less than the barrier.
  FunctionBlockersType::size_type barrier = 0;
  if(!this->FunctionBlockerBarriers.empty())
    {
    barrier = this->FunctionBlockerBarriers.back();
    }

  // Search for the function blocker whose scope this command ends.
  for(FunctionBlockersType::size_type
        i = this->FunctionBlockers.size(); i > barrier; --i)
    {
    std::vector<cmFunctionBlocker*>::iterator pos =
      this->FunctionBlockers.begin() + (i - 1);
    if (*pos == fb)
      {
      // Warn if the arguments do not match, but always remove.
      if(!(*pos)->ShouldRemove(lff, *this))
        {
        cmListFileContext const& lfc = fb->GetStartingContext();
        cmOStringStream e;
        e << "A logical block opening on the line\n"
          << "  " << lfc << "\n"
          << "closes on the line\n"
          << "  " << lff << "\n"
          << "with mis-matching arguments.";
        this->IssueMessage(cmake::AUTHOR_WARNING, e.str());
        }
      cmFunctionBlocker* b = *pos;
      this->FunctionBlockers.erase(pos);
      return cmsys::auto_ptr<cmFunctionBlocker>(b);
      }
    }

  return cmsys::auto_ptr<cmFunctionBlocker>();
}

//----------------------------------------------------------------------------
cmMakefile::LexicalPushPop::LexicalPushPop(cmMakefile* mf):
  Makefile(mf), ReportError(true)
{
  this->Makefile->PushFunctionBlockerBarrier();
}

//----------------------------------------------------------------------------
cmMakefile::LexicalPushPop::~LexicalPushPop()
{
  this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
}

void cmMakefile::SetHomeDirectory(const char* dir)
{
  this->cmHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(this->cmHomeDirectory);
  this->AddDefinition("CMAKE_SOURCE_DIR", this->GetHomeDirectory());
  if ( !this->GetDefinition("CMAKE_CURRENT_SOURCE_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", this->GetHomeDirectory());
    }
}

void cmMakefile::SetHomeOutputDirectory(const char* lib)
{
  this->HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(this->HomeOutputDirectory);
  this->AddDefinition("CMAKE_BINARY_DIR", this->GetHomeOutputDirectory());
  if ( !this->GetDefinition("CMAKE_CURRENT_BINARY_DIR") )
    {
    this->AddDefinition("CMAKE_CURRENT_BINARY_DIR",
                        this->GetHomeOutputDirectory());
    }
}

void cmMakefile::SetScriptModeFile(const char* scriptfile)
{
  this->AddDefinition("CMAKE_SCRIPT_MODE_FILE", scriptfile);
}

void cmMakefile::SetArgcArgv(const std::vector<std::string>& args)
{
  cmOStringStream strStream;
  strStream << args.size();
  this->AddDefinition("CMAKE_ARGC", strStream.str().c_str());
  //this->MarkVariableAsUsed("CMAKE_ARGC");

  for (unsigned int t = 0; t < args.size(); ++t)
  {
    cmOStringStream tmpStream;
    tmpStream << "CMAKE_ARGV" << t;
    this->AddDefinition(tmpStream.str().c_str(), args[t].c_str());
    //this->MarkVariableAsUsed(tmpStream.str().c_str());
  }
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetSource(const char* sourceName)
{
  cmSourceFileLocation sfl(this, sourceName);
  for(std::vector<cmSourceFile*>::const_iterator
        sfi = this->SourceFiles.begin();
      sfi != this->SourceFiles.end(); ++sfi)
    {
    cmSourceFile* sf = *sfi;
    if(sf->Matches(sfl))
      {
      return sf;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetOrCreateSource(const char* sourceName,
                                            bool generated)
{
  if(cmSourceFile* esf = this->GetSource(sourceName))
    {
    return esf;
    }
  else
    {
    cmSourceFile* sf = new cmSourceFile(this, sourceName);
    if(generated)
      {
      sf->SetProperty("GENERATED", "1");
      }
    this->SourceFiles.push_back(sf);
    return sf;
    }
}

void cmMakefile::EnableLanguage(std::vector<std::string> const &  lang,
                               bool optional)
{
  this->AddDefinition("CMAKE_CFG_INTDIR",
                      this->LocalGenerator->GetGlobalGenerator()
                      ->GetCMakeCFGIntDir());
  this->LocalGenerator->GetGlobalGenerator()->EnableLanguage(lang, this,
                                                             optional);
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
                           bool fast,
                           const std::vector<std::string> *cmakeArgs,
                           std::string *output)
{
  this->Internal->IsSourceFileTryCompile = fast;
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
  cmGlobalGenerator *gg = cm.CreateGlobalGenerator
    (this->LocalGenerator->GetGlobalGenerator()->GetName());
  if (!gg)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile bad GlobalGenerator");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    this->Internal->IsSourceFileTryCompile = false;
    return 1;
    }
  cm.SetGlobalGenerator(gg);

  // do a configure
  cm.SetHomeDirectory(srcdir);
  cm.SetHomeOutputDirectory(bindir);
  cm.SetStartDirectory(srcdir);
  cm.SetStartOutputDirectory(bindir);
  cm.SetCMakeCommand(cmakeCommand.c_str());
  cm.SetGeneratorToolset(this->GetCMakeInstance()->GetGeneratorToolset());
  cm.LoadCache();
  if(!gg->IsMultiConfig())
    {
    if(const char* config =
       this->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION"))
      {
      // Tell the single-configuration generator which one to use.
      // Add this before the user-provided CMake arguments in case
      // one of the arguments is -DCMAKE_BUILD_TYPE=...
      cm.AddCacheEntry("CMAKE_BUILD_TYPE", config,
                       "Build configuration", cmCacheManager::STRING);
      }
    }
  // if cmake args were provided then pass them in
  if (cmakeArgs)
    {
    // FIXME: Workaround to ignore unused CLI variables in try-compile.
    //
    // Ideally we should use SetArgs to honor options like --warn-unused-vars.
    // However, there is a subtle problem when certain arguments are passed to
    // a macro wrapping around try_compile or try_run that does not escape
    // semicolons in its parameters but just passes ${ARGV} or ${ARGN}.  In
    // this case a list argument like "-DVAR=a;b" gets split into multiple
    // cmake arguments "-DVAR=a" and "b".  Currently SetCacheArgs ignores
    // argument "b" and uses just "-DVAR=a", leading to a subtle bug in that
    // the try_compile or try_run does not get the proper value of VAR.  If we
    // call SetArgs here then it would treat "b" as the source directory and
    // cause an error such as "The source directory .../CMakeFiles/CMakeTmp/b
    // does not exist", thus breaking the try_compile or try_run completely.
    //
    // Strictly speaking the bug is in the wrapper macro because the CMake
    // language has always flattened nested lists and the macro should escape
    // the semicolons in its arguments before forwarding them.  However, this
    // bug is so subtle that projects typically work anyway, usually because
    // the value VAR=a is sufficient for the try_compile or try_run to get the
    // correct result.  Calling SetArgs here would break such projects that
    // previously built.  Instead we work around the issue by never reporting
    // unused arguments and ignoring options such as --warn-unused-vars.
    cm.SetWarnUnusedCli(false);
    //cm.SetArgs(*cmakeArgs, true);

    cm.SetCacheArgs(*cmakeArgs);
    }
  // to save time we pass the EnableLanguage info directly
  gg->EnableLanguagesFromGenerator
    (this->LocalGenerator->GetGlobalGenerator(), this);
  if(this->IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS"))
    {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS",
                     "TRUE", "", cmCacheManager::INTERNAL);
    }
  else
    {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS",
                     "FALSE", "", cmCacheManager::INTERNAL);
    }
  if (cm.Configure() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile configure of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    this->Internal->IsSourceFileTryCompile = false;
    return 1;
    }

  if (cm.Generate() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile generation of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    this->Internal->IsSourceFileTryCompile = false;
    return 1;
    }

  // finally call the generator to actually build the resulting project
  int ret =
    this->LocalGenerator->GetGlobalGenerator()->TryCompile(srcdir,bindir,
                                                           projectName,
                                                           targetName,
                                                           fast,
                                                           output,
                                                           this);

  cmSystemTools::ChangeDirectory(cwd.c_str());
  this->Internal->IsSourceFileTryCompile = false;
  return ret;
}

bool cmMakefile::GetIsSourceFileTryCompile() const
{
  return this->Internal->IsSourceFileTryCompile;
}

cmake *cmMakefile::GetCMakeInstance() const
{
  if ( this->LocalGenerator && this->LocalGenerator->GetGlobalGenerator() )
    {
    return this->LocalGenerator->GetGlobalGenerator()->GetCMakeInstance();
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
  this->MacrosMap[name] = signature;
}

void cmMakefile::GetListOfMacros(std::string& macros)
{
  StringStringMap::iterator it;
  macros = "";
  int cc = 0;
  for ( it = this->MacrosMap.begin(); it != this->MacrosMap.end(); ++it )
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
  cmake* cm = this->GetLocalGenerator()->GetGlobalGenerator()
                                                          ->GetCMakeInstance();
  if (cm->GetWorkingMode() == cmake::FIND_PACKAGE_MODE)
    {
    // don't output any STATUS message in FIND_PACKAGE_MODE, since they will
    // directly be fed to the compiler, which will be confused.
    return;
    }
  cm->UpdateProgress(message, s);
}

std::string cmMakefile::GetModulesFile(const char* filename)
{
  std::string result;

  // We search the module always in CMAKE_ROOT and in CMAKE_MODULE_PATH,
  // and then decide based on the policy setting which one to return.
  // See CMP0017 for more details.
  // The specific problem was that KDE 4.5.0 installs a
  // FindPackageHandleStandardArgs.cmake which doesn't have the new features
  // of FPHSA.cmake introduced in CMake 2.8.3 yet, and by setting
  // CMAKE_MODULE_PATH also e.g. FindZLIB.cmake from cmake included
  // FPHSA.cmake from kdelibs and not from CMake, and tried to use the
  // new features, which were not there in the version from kdelibs, and so
  // failed ("
  std::string moduleInCMakeRoot;
  std::string moduleInCMakeModulePath;

  // Always search in CMAKE_MODULE_PATH:
  const char* cmakeModulePath = this->GetDefinition("CMAKE_MODULE_PATH");
  if(cmakeModulePath)
    {
    std::vector<std::string> modulePath;
    cmSystemTools::ExpandListArgument(cmakeModulePath, modulePath);

    //Look through the possible module directories.
    for(std::vector<std::string>::iterator i = modulePath.begin();
        i != modulePath.end(); ++i)
      {
      std::string itempl = *i;
      cmSystemTools::ConvertToUnixSlashes(itempl);
      itempl += "/";
      itempl += filename;
      if(cmSystemTools::FileExists(itempl.c_str()))
        {
        moduleInCMakeModulePath = itempl;
        break;
        }
      }
    }

  // Always search in the standard modules location.
  const char* cmakeRoot = this->GetDefinition("CMAKE_ROOT");
  if(cmakeRoot)
    {
    moduleInCMakeRoot = cmakeRoot;
    moduleInCMakeRoot += "/Modules/";
    moduleInCMakeRoot += filename;
    cmSystemTools::ConvertToUnixSlashes(moduleInCMakeRoot);
    if(!cmSystemTools::FileExists(moduleInCMakeRoot.c_str()))
      {
      moduleInCMakeRoot = "";
      }
    }

  // Normally, prefer the files found in CMAKE_MODULE_PATH. Only when the file
  // from which we are being called is located itself in CMAKE_ROOT, then
  // prefer results from CMAKE_ROOT depending on the policy setting.
  result = moduleInCMakeModulePath;
  if (result.size() == 0)
    {
    result = moduleInCMakeRoot;
    }

  if ((moduleInCMakeModulePath.size()>0) && (moduleInCMakeRoot.size()>0))
    {
    const char* currentFile = this->GetDefinition("CMAKE_CURRENT_LIST_FILE");
    if (currentFile && (strstr(currentFile, cmakeRoot) == currentFile))
      {
      switch (this->GetPolicyStatus(cmPolicies::CMP0017))
        {
        case cmPolicies::WARN:
        {
          cmOStringStream e;
          e << "File " << currentFile << " includes "
            << moduleInCMakeModulePath
            << " (found via CMAKE_MODULE_PATH) which shadows "
            << moduleInCMakeRoot  << ". This may cause errors later on .\n"
            << this->GetPolicies()->GetPolicyWarning(cmPolicies::CMP0017);

          this->IssueMessage(cmake::AUTHOR_WARNING, e.str());
           // break;  // fall through to OLD behaviour
        }
        case cmPolicies::OLD:
          result = moduleInCMakeModulePath;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
        default:
          result = moduleInCMakeRoot;
          break;
        }
      }
    }

  return result;
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
    if(this->cmDefineRegex.find(line))
      {
      const char* def =
        this->GetDefinition(this->cmDefineRegex.match(1).c_str());
      if(!cmSystemTools::IsOff(def))
        {
        cmSystemTools::ReplaceString(line, "#cmakedefine", "#define");
        output += line;
        }
      else
        {
        output += "/* #undef ";
        output += this->cmDefineRegex.match(1);
        output += " */";
        }
      }
    else if(this->cmDefine01Regex.find(line))
      {
      const char* def =
        this->GetDefinition(this->cmDefine01Regex.match(1).c_str());
      cmSystemTools::ReplaceString(line, "#cmakedefine01", "#define");
      output += line;
      if(!cmSystemTools::IsOff(def))
        {
        output += " 1";
        }
      else
        {
        output += " 0";
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
  this->ExpandVariablesInString(output, escapeQuotes, true,
                                atOnly, 0, -1, true);
}

int cmMakefile::ConfigureFile(const char* infile, const char* outfile,
                              bool copyonly, bool atOnly, bool escapeQuotes,
                              const cmNewLineStyle& newLine)
{
  int res = 1;
  if ( !this->CanIWriteThisFile(outfile) )
    {
    cmSystemTools::Error("Attempt to write file: ",
                         outfile, " into a source directory.");
    return 0;
    }
  if ( !cmSystemTools::FileExists(infile) )
    {
    cmSystemTools::Error("File ", infile, " does not exist.");
    return 0;
    }
  std::string soutfile = outfile;
  std::string sinfile = infile;
  this->AddCMakeDependFile(sinfile);
  cmSystemTools::ConvertToUnixSlashes(soutfile);

  // Re-generate if non-temporary outputs are missing.
  //when we finalize the configuration we will remove all
  //output files that now don't exist.
  this->AddCMakeOutputFile(soutfile);

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
    std::string newLineCharacters;
    std::ios_base::openmode omode = std::ios_base::out | std::ios_base::trunc;
    if (newLine.IsValid())
      {
      newLineCharacters = newLine.GetCharacters();
      omode |= std::ios::binary;
      }
    else
      {
      newLineCharacters = "\n";
      }
    std::string tempOutputFile = soutfile;
    tempOutputFile += ".tmp";
    std::ofstream fout(tempOutputFile.c_str(), omode);
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
      fout << outLine.c_str() << newLineCharacters;
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

void cmMakefile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  // handle special props
  std::string propname = prop;

  if ( propname == "LINK_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    if(value)
      {
      cmSystemTools::ExpandListArgument(value, varArgsExpanded);
      }
    this->SetLinkDirectories(varArgsExpanded);
    return;
    }
  if (propname == "INCLUDE_DIRECTORIES")
    {
    this->IncludeDirectoriesEntries.clear();
      if (!value)
        {
        return;
        }
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    this->IncludeDirectoriesEntries.push_back(
                                        cmValueWithOrigin(value, lfbt));
    return;
    }
  if (propname == "COMPILE_OPTIONS")
    {
    this->CompileOptionsEntries.clear();
      if (!value)
        {
        return;
        }
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    this->CompileOptionsEntries.push_back(cmValueWithOrigin(value, lfbt));
    return;
    }
  if (propname == "COMPILE_DEFINITIONS")
    {
    this->CompileDefinitionsEntries.clear();
    if (!value)
      {
      return;
      }
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    cmValueWithOrigin entry(value, lfbt);
    this->CompileDefinitionsEntries.push_back(entry);
    return;
    }

  if ( propname == "INCLUDE_REGULAR_EXPRESSION" )
    {
    this->SetIncludeRegularExpression(value);
    return;
    }

  if ( propname == "ADDITIONAL_MAKE_CLEAN_FILES" )
    {
    // This property is not inherrited
    if ( strcmp(this->GetCurrentDirectory(),
                this->GetStartDirectory()) != 0 )
      {
      return;
      }
    }

  this->Properties.SetProperty(prop,value,cmProperty::DIRECTORY);
}

void cmMakefile::AppendProperty(const char* prop, const char* value,
                                bool asString)
{
  if (!prop)
    {
    return;
    }

  // handle special props
  std::string propname = prop;

  if (propname == "INCLUDE_DIRECTORIES")
    {
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    this->IncludeDirectoriesEntries.push_back(
                                        cmValueWithOrigin(value, lfbt));
    return;
    }
  if (propname == "COMPILE_OPTIONS")
    {
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    this->CompileOptionsEntries.push_back(
                                        cmValueWithOrigin(value, lfbt));
    return;
    }
  if (propname == "COMPILE_DEFINITIONS")
    {
    cmListFileBacktrace lfbt;
    this->GetBacktrace(lfbt);
    this->CompileDefinitionsEntries.push_back(
                                        cmValueWithOrigin(value, lfbt));
    return;
    }
  if ( propname == "LINK_DIRECTORIES" )
    {
    std::vector<std::string> varArgsExpanded;
    cmSystemTools::ExpandListArgument(value, varArgsExpanded);
    for(std::vector<std::string>::const_iterator vi = varArgsExpanded.begin();
        vi != varArgsExpanded.end(); ++vi)
      {
      this->AddLinkDirectory(vi->c_str());
      }
    return;
    }

  this->Properties.AppendProperty(prop,value,cmProperty::DIRECTORY,asString);
}

const char *cmMakefile::GetPropertyOrDefinition(const char* prop)
{
  const char *ret = this->GetProperty(prop, cmProperty::DIRECTORY);
  if (!ret)
    {
    ret = this->GetDefinition(prop);
    }
  return ret;
}

const char *cmMakefile::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::DIRECTORY);
}

const char *cmMakefile::GetProperty(const char* prop,
                                    cmProperty::ScopeType scope)
{
  if(!prop)
    {
    return 0;
    }
  // watch for specific properties
  static std::string output;
  output = "";
  if (!strcmp("PARENT_DIRECTORY",prop))
    {
    if(cmLocalGenerator* plg = this->LocalGenerator->GetParent())
      {
      output = plg->GetMakefile()->GetStartDirectory();
      }
    return output.c_str();
    }
  else if (!strcmp("INCLUDE_REGULAR_EXPRESSION",prop) )
    {
    output = this->GetIncludeRegularExpression();
    return output.c_str();
    }
  else if (!strcmp("LISTFILE_STACK",prop))
    {
    for (std::deque<cmStdString>::iterator i = this->ListFileStack.begin();
         i != this->ListFileStack.end(); ++i)
      {
      if (i != this->ListFileStack.begin())
        {
        output += ";";
        }
      output += *i;
      }
    return output.c_str();
    }
  else if (!strcmp("VARIABLES",prop) || !strcmp("CACHE_VARIABLES",prop))
    {
    int cacheonly = 0;
    if ( !strcmp("CACHE_VARIABLES",prop) )
      {
      cacheonly = 1;
      }
    std::vector<std::string> vars = this->GetDefinitions(cacheonly);
    for (unsigned int cc = 0; cc < vars.size(); cc ++ )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += vars[cc];
      }
    return output.c_str();
    }
  else if (!strcmp("MACROS",prop))
    {
    this->GetListOfMacros(output);
    return output.c_str();
    }
  else if (!strcmp("DEFINITIONS",prop))
    {
    output += this->DefineFlagsOrig;
    return output.c_str();
    }
  else if (!strcmp("LINK_DIRECTORIES",prop))
    {
    cmOStringStream str;
    for (std::vector<std::string>::const_iterator
         it = this->GetLinkDirectories().begin();
         it != this->GetLinkDirectories().end();
         ++ it )
      {
      if ( it != this->GetLinkDirectories().begin())
        {
        str << ";";
        }
      str << it->c_str();
      }
    output = str.str();
    return output.c_str();
    }
  else if (!strcmp("INCLUDE_DIRECTORIES",prop))
    {
    std::string sep;
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->IncludeDirectoriesEntries.begin(),
        end = this->IncludeDirectoriesEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += it->Value;
      sep = ";";
      }
    return output.c_str();
    }
  else if (!strcmp("COMPILE_OPTIONS",prop))
    {
    std::string sep;
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->CompileOptionsEntries.begin(),
        end = this->CompileOptionsEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += it->Value;
      sep = ";";
      }
    return output.c_str();
    }
  else if (!strcmp("COMPILE_DEFINITIONS",prop))
    {
    std::string sep;
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->CompileDefinitionsEntries.begin(),
        end = this->CompileDefinitionsEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += it->Value;
      sep = ";";
      }
    return output.c_str();
    }

  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, scope, chain);
  if (chain)
    {
    if(this->LocalGenerator->GetParent())
      {
      return this->LocalGenerator->GetParent()->GetMakefile()->
        GetProperty(prop, scope);
      }
    return this->GetCMakeInstance()->GetProperty(prop,scope);
    }

  return retVal;
}

bool cmMakefile::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
const char* cmMakefile::GetFeature(const char* feature, const char* config)
{
  // TODO: Define accumulation policy for features (prepend, append, replace).
  // Currently we always replace.
  if(config && *config)
    {
    std::string featureConfig = feature;
    featureConfig += "_";
    featureConfig += cmSystemTools::UpperCase(config);
    if(const char* value = this->GetProperty(featureConfig.c_str()))
      {
      return value;
      }
    }
  if(const char* value = this->GetProperty(feature))
    {
    return value;
    }
  if(cmLocalGenerator* parent = this->LocalGenerator->GetParent())
    {
    return parent->GetMakefile()->GetFeature(feature, config);
    }
  return 0;
}

cmTarget* cmMakefile::FindTarget(const char* name, bool excludeAliases)
{
  if (!excludeAliases)
    {
    std::map<std::string, cmTarget*>::iterator i
                                              = this->AliasTargets.find(name);
    if (i != this->AliasTargets.end())
      {
      return i->second;
      }
    }
  cmTargets& tgts = this->GetTargets();

  cmTargets::iterator i = tgts.find ( name );
  if ( i != tgts.end() )
    {
    return &i->second;
    }

  return 0;
}

//----------------------------------------------------------------------------
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
  test = new cmTest(this);
  test->SetName(testName);
  this->Tests[testName] = test;
  return test;
}

//----------------------------------------------------------------------------
cmTest* cmMakefile::GetTest(const char* testName) const
{
  if(testName)
    {
    std::map<cmStdString, cmTest*>::const_iterator
      mi = this->Tests.find(testName);
    if(mi != this->Tests.end())
      {
      return mi->second;
      }
    }
  return 0;
}

std::string cmMakefile::GetListFileStack()
{
  cmOStringStream tmp;
  size_t depth = this->ListFileStack.size();
  if (depth > 0)
    {
    std::deque<cmStdString>::iterator it = this->ListFileStack.end();
    do
      {
      if (depth != this->ListFileStack.size())
        {
        tmp << "\n                ";
        }
      --it;
      tmp << "[";
      tmp << depth;
      tmp << "]\t";
      tmp << *it;
      depth--;
      }
    while (it != this->ListFileStack.begin());
    }
  return tmp.str();
}


void cmMakefile::PushScope()
{
  cmDefinitions* parent = &this->Internal->VarStack.top();
  const std::set<cmStdString>& init = this->Internal->VarInitStack.top();
  const std::set<cmStdString>& usage = this->Internal->VarUsageStack.top();
  this->Internal->VarStack.push(cmDefinitions(parent));
  this->Internal->VarInitStack.push(init);
  this->Internal->VarUsageStack.push(usage);
}

void cmMakefile::PopScope()
{
  cmDefinitions* current = &this->Internal->VarStack.top();
  std::set<cmStdString> init = this->Internal->VarInitStack.top();
  std::set<cmStdString> usage = this->Internal->VarUsageStack.top();
  const std::set<cmStdString>& locals = current->LocalKeys();
  // Remove initialization and usage information for variables in the local
  // scope.
  std::set<cmStdString>::const_iterator it = locals.begin();
  for (; it != locals.end(); ++it)
    {
    init.erase(*it);
    if (!this->VariableUsed(it->c_str()))
      {
      this->CheckForUnused("out of scope", it->c_str());
      }
    else
      {
      usage.erase(*it);
      }
    }
  this->Internal->VarStack.pop();
  this->Internal->VarInitStack.pop();
  this->Internal->VarUsageStack.pop();
  // Push initialization and usage up to the parent scope.
  it = init.begin();
  for (; it != init.end(); ++it)
    {
    this->Internal->VarInitStack.top().insert(*it);
    }
  it = usage.begin();
  for (; it != usage.end(); ++it)
    {
    this->Internal->VarUsageStack.top().insert(*it);
    }
}

void cmMakefile::RaiseScope(const char *var, const char *varDef)
{
  if (!var || !strlen(var))
    {
    return;
    }

  cmDefinitions& cur = this->Internal->VarStack.top();
  if(cmDefinitions* up = cur.GetParent())
    {
    // First localize the definition in the current scope.
    cur.Get(var);

    // Now update the definition in the parent scope.
    up->Set(var, varDef);
    }
  else if(cmLocalGenerator* plg = this->LocalGenerator->GetParent())
    {
    // Update the definition in the parent directory top scope.  This
    // directory's scope was initialized by the closure of the parent
    // scope, so we do not need to localize the definition first.
    cmMakefile* parent = plg->GetMakefile();
    if (varDef)
      {
      parent->AddDefinition(var, varDef);
      }
    else
      {
      parent->RemoveDefinition(var);
      }
    }
  else
    {
    cmOStringStream m;
    m << "Cannot set \"" << var << "\": current scope has no parent.";
    this->IssueMessage(cmake::AUTHOR_WARNING, m.str());
    }
}


// define properties
void cmMakefile::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("ADDITIONAL_MAKE_CLEAN_FILES", cmProperty::DIRECTORY,
     "Additional files to clean during the make clean stage.",
     "A list of files that will be cleaned as a part of the "
     "\"make clean\" stage. ");

  cm->DefineProperty
    ("CLEAN_NO_CUSTOM", cmProperty::DIRECTORY,
     "Should the output of custom commands be left.",
     "If this is true then the outputs of custom commands for this "
     "directory will not be removed during the \"make clean\" stage. ");

  cm->DefineProperty
    ("LISTFILE_STACK", cmProperty::DIRECTORY,
     "The current stack of listfiles being processed.",
     "This property is mainly useful when trying to debug errors "
     "in your CMake scripts. It returns a list of what list files "
     "are currently being processed, in order. So if one listfile "
     "does an INCLUDE command then that is effectively pushing "
     "the included listfile onto the stack.", false);

  cm->DefineProperty
    ("TEST_INCLUDE_FILE", cmProperty::DIRECTORY,
     "A cmake file that will be included when ctest is run.",
     "If you specify TEST_INCLUDE_FILE, that file will be "
     "included and processed when ctest is run on the directory.");

  cm->DefineProperty
    ("COMPILE_DEFINITIONS", cmProperty::DIRECTORY,
     "Preprocessor definitions for compiling a directory's sources.",
     "The COMPILE_DEFINITIONS property may be set to a "
     "semicolon-separated list of preprocessor "
     "definitions using the syntax VAR or VAR=value.  Function-style "
     "definitions are not supported.  CMake will automatically escape "
     "the value correctly for the native build system (note that CMake "
     "language syntax may require escapes to specify some values).  "
     "This property may be set on a per-configuration basis using the name "
     "COMPILE_DEFINITIONS_<CONFIG> where <CONFIG> is an upper-case name "
     "(ex. \"COMPILE_DEFINITIONS_DEBUG\").  "
     "This property will be initialized in each directory by its value "
     "in the directory's parent.\n"
     "CMake will automatically drop some definitions that "
     "are not supported by the native build tool.  "
     "The VS6 IDE does not support definition values with spaces "
     "(but NMake does).\n"
     CM_DOCUMENT_COMPILE_DEFINITIONS_DISCLAIMER);

  cm->DefineProperty
    ("COMPILE_DEFINITIONS_<CONFIG>", cmProperty::DIRECTORY,
     "Per-configuration preprocessor definitions in a directory.",
     "This is the configuration-specific version of COMPILE_DEFINITIONS.  "
     "This property will be initialized in each directory by its value "
     "in the directory's parent.\n");

  cm->DefineProperty
    ("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM", cmProperty::DIRECTORY,
     "Specify #include line transforms for dependencies in a directory.",
     "This property specifies rules to transform macro-like #include lines "
     "during implicit dependency scanning of C and C++ source files.  "
     "The list of rules must be semicolon-separated with each entry of "
     "the form \"A_MACRO(%)=value-with-%\" (the % must be literal).  "
     "During dependency scanning occurrences of A_MACRO(...) on #include "
     "lines will be replaced by the value given with the macro argument "
     "substituted for '%'.  For example, the entry\n"
     "  MYDIR(%)=<mydir/%>\n"
     "will convert lines of the form\n"
     "  #include MYDIR(myheader.h)\n"
     "to\n"
     "  #include <mydir/myheader.h>\n"
     "allowing the dependency to be followed.\n"
     "This property applies to sources in all targets within a directory.  "
     "The property value is initialized in each directory by its value "
     "in the directory's parent.");

  cm->DefineProperty
    ("EXCLUDE_FROM_ALL", cmProperty::DIRECTORY,
     "Exclude the directory from the all target of its parent.",
     "A property on a directory that indicates if its targets are excluded "
     "from the default build target. If it is not, then with a Makefile "
     "for example typing make will cause the targets to be built. "
     "The same concept applies to the default build of other generators.",
     false);

  cm->DefineProperty
    ("PARENT_DIRECTORY", cmProperty::DIRECTORY,
     "Source directory that added current subdirectory.",
     "This read-only property specifies the source directory that "
     "added the current source directory as a subdirectory of the build.  "
     "In the top-level directory the value is the empty-string.", false);

  cm->DefineProperty
    ("INCLUDE_REGULAR_EXPRESSION", cmProperty::DIRECTORY,
     "Include file scanning regular expression.",
     "This read-only property specifies the regular expression used "
     "during dependency scanning to match include files that should "
     "be followed.  See the include_regular_expression command.", false);

  cm->DefineProperty
    ("INTERPROCEDURAL_OPTIMIZATION", cmProperty::DIRECTORY,
     "Enable interprocedural optimization for targets in a directory.",
     "If set to true, enables interprocedural optimizations "
     "if they are known to be supported by the compiler.");

  cm->DefineProperty
    ("INTERPROCEDURAL_OPTIMIZATION_<CONFIG>", cmProperty::DIRECTORY,
     "Per-configuration interprocedural optimization for a directory.",
     "This is a per-configuration version of INTERPROCEDURAL_OPTIMIZATION.  "
     "If set, this property overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("VARIABLES", cmProperty::DIRECTORY,
     "List of variables defined in the current directory.",
     "This read-only property specifies the list of CMake variables "
     "currently defined.  "
     "It is intended for debugging purposes.", false);

  cm->DefineProperty
    ("CACHE_VARIABLES", cmProperty::DIRECTORY,
     "List of cache variables available in the current directory.",
     "This read-only property specifies the list of CMake cache "
     "variables currently defined.  "
     "It is intended for debugging purposes.", false);

  cm->DefineProperty
    ("MACROS", cmProperty::DIRECTORY,
     "List of macro commands available in the current directory.",
     "This read-only property specifies the list of CMake macros "
     "currently defined.  "
     "It is intended for debugging purposes.  "
     "See the macro command.", false);

  cm->DefineProperty
    ("DEFINITIONS", cmProperty::DIRECTORY,
     "For CMake 2.4 compatibility only.  Use COMPILE_DEFINITIONS instead.",
     "This read-only property specifies the list of flags given so far "
     "to the add_definitions command.  "
     "It is intended for debugging purposes.  "
     "Use the COMPILE_DEFINITIONS instead.", false);

  cm->DefineProperty
    ("INCLUDE_DIRECTORIES", cmProperty::DIRECTORY,
     "List of preprocessor include file search directories.",
     "This property specifies the list of directories given "
     "so far to the include_directories command.  "
     "This property exists on directories and targets.  "
     "In addition to accepting values from the include_directories "
     "command, values may be set directly on any directory or any "
     "target using the set_property command.  "
     "A target gets its initial value for this property from the value "
     "of the directory property.  "
     "A directory gets its initial value from its parent directory if "
     "it has one.  "
     "Both directory and target property values are adjusted by calls "
     "to the include_directories command."
     "\n"
     "The target property values are used by the generators to set "
     "the include paths for the compiler.  "
     "See also the include_directories command.");

  cm->DefineProperty
    ("COMPILE_OPTIONS", cmProperty::DIRECTORY,
     "List of options to pass to the compiler.",
     "This property specifies the list of directories given "
     "so far for this property.  "
     "This property exists on directories and targets."
     "\n"
     "The target property values are used by the generators to set "
     "the options for the compiler.\n"
     "Contents of COMPILE_OPTIONS may use \"generator expressions\" with "
     "the syntax \"$<...>\".  "
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS);

  cm->DefineProperty
    ("LINK_DIRECTORIES", cmProperty::DIRECTORY,
     "List of linker search directories.",
     "This read-only property specifies the list of directories given "
     "so far to the link_directories command.  "
     "It is intended for debugging purposes.", false);

  cm->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::DIRECTORY,
     "Specify a launcher for compile rules.",
     "See the global property of the same name for details.  "
     "This overrides the global property for a directory.",
     true);
  cm->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::DIRECTORY,
     "Specify a launcher for link rules.",
     "See the global property of the same name for details.  "
     "This overrides the global property for a directory.",
     true);
  cm->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::DIRECTORY,
     "Specify a launcher for custom rules.",
     "See the global property of the same name for details.  "
     "This overrides the global property for a directory.",
     true);

  cm->DefineProperty
    ("VS_GLOBAL_SECTION_PRE_<section>", cmProperty::DIRECTORY,
     "Specify a preSolution global section in Visual Studio.",
     "Setting a property like this generates an entry of the following form "
     "in the solution file:\n"
     "  GlobalSection(<section>) = preSolution\n"
     "    <contents based on property value>\n"
     "  EndGlobalSection\n"
     "The property must be set to a semicolon-separated list of key=value "
     "pairs. Each such pair will be transformed into an entry in the solution "
     "global section. Whitespace around key and value is ignored. List "
     "elements which do not contain an equal sign are skipped."
     "\n"
     "This property only works for Visual Studio 7 and above; it is ignored "
     "on other generators. The property only applies when set on a directory "
     "whose CMakeLists.txt contains a project() command.");
  cm->DefineProperty
    ("VS_GLOBAL_SECTION_POST_<section>", cmProperty::DIRECTORY,
     "Specify a postSolution global section in Visual Studio.",
     "Setting a property like this generates an entry of the following form "
     "in the solution file:\n"
     "  GlobalSection(<section>) = postSolution\n"
     "    <contents based on property value>\n"
     "  EndGlobalSection\n"
     "The property must be set to a semicolon-separated list of key=value "
     "pairs. Each such pair will be transformed into an entry in the solution "
     "global section. Whitespace around key and value is ignored. List "
     "elements which do not contain an equal sign are skipped."
     "\n"
     "This property only works for Visual Studio 7 and above; it is ignored "
     "on other generators. The property only applies when set on a directory "
     "whose CMakeLists.txt contains a project() command."
     "\n"
     "Note that CMake generates postSolution sections ExtensibilityGlobals "
     "and ExtensibilityAddIns by default. If you set the corresponding "
     "property, it will override the default section. For example, setting "
     "VS_GLOBAL_SECTION_POST_ExtensibilityGlobals will override the default "
     "contents of the ExtensibilityGlobals section, while keeping "
     "ExtensibilityAddIns on its default.");
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddImportedTarget(const char* name, cmTarget::TargetType type,
                              bool global)
{
  // Create the target.
  cmsys::auto_ptr<cmTarget> target(new cmTarget);
  target->SetType(type, name);
  target->SetMakefile(this);
  target->MarkAsImported();

  // Add to the set of available imported targets.
  this->ImportedTargets[name] = target.get();
  if(global)
    {
    this->LocalGenerator->GetGlobalGenerator()->AddTarget(target.get());
    }

  // Transfer ownership to this cmMakefile object.
  this->ImportedTargetsOwned.push_back(target.get());
  return target.release();
}

//----------------------------------------------------------------------------
cmTarget* cmMakefile::FindTargetToUse(const char* name, bool excludeAliases)
{
  // Look for an imported target.  These take priority because they
  // are more local in scope and do not have to be globally unique.
  std::map<cmStdString, cmTarget*>::const_iterator
    imported = this->ImportedTargets.find(name);
  if(imported != this->ImportedTargets.end())
    {
    return imported->second;
    }

  // Look for a target built in this directory.
  if(cmTarget* t = this->FindTarget(name, excludeAliases))
    {
    return t;
    }

  // Look for a target built in this project.
  return this->LocalGenerator->GetGlobalGenerator()->FindTarget(0, name,
                                                              excludeAliases);
}

//----------------------------------------------------------------------------
bool cmMakefile::IsAlias(const char *name)
{
  if (this->AliasTargets.find(name) != this->AliasTargets.end())
    return true;
  return this->GetLocalGenerator()->GetGlobalGenerator()->IsAlias(name);
}

//----------------------------------------------------------------------------
cmGeneratorTarget* cmMakefile::FindGeneratorTargetToUse(const char* name)
{
  cmTarget *t = this->FindTargetToUse(name);
  return this->LocalGenerator->GetGlobalGenerator()->GetGeneratorTarget(t);
}

//----------------------------------------------------------------------------
bool cmMakefile::EnforceUniqueName(std::string const& name, std::string& msg,
                                   bool isCustom)
{
  if(this->IsAlias(name.c_str()))
    {
    cmOStringStream e;
    e << "cannot create target \"" << name
      << "\" because an alias with the same name already exists.";
    msg = e.str();
    return false;
    }
  if(cmTarget* existing = this->FindTargetToUse(name.c_str()))
    {
    // The name given conflicts with an existing target.  Produce an
    // error in a compatible way.
    if(existing->IsImported())
      {
      // Imported targets were not supported in previous versions.
      // This is new code, so we can make it an error.
      cmOStringStream e;
      e << "cannot create target \"" << name
        << "\" because an imported target with the same name already exists.";
      msg = e.str();
      return false;
      }
    else
      {
      // target names must be globally unique
      switch (this->GetPolicyStatus(cmPolicies::CMP0002))
        {
        case cmPolicies::WARN:
          this->IssueMessage(cmake::AUTHOR_WARNING, this->GetPolicies()->
                             GetPolicyWarning(cmPolicies::CMP0002));
        case cmPolicies::OLD:
          return true;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          this->IssueMessage(cmake::FATAL_ERROR,
            this->GetPolicies()->GetRequiredPolicyError(cmPolicies::CMP0002)
            );
          return true;
        case cmPolicies::NEW:
          break;
        }

      // The conflict is with a non-imported target.
      // Allow this if the user has requested support.
      cmake* cm =
        this->LocalGenerator->GetGlobalGenerator()->GetCMakeInstance();
      if(isCustom && existing->GetType() == cmTarget::UTILITY &&
         this != existing->GetMakefile() &&
         cm->GetPropertyAsBool("ALLOW_DUPLICATE_CUSTOM_TARGETS"))
        {
        return true;
        }

      // Produce an error that tells the user how to work around the
      // problem.
      cmOStringStream e;
      e << "cannot create target \"" << name
        << "\" because another target with the same name already exists.  "
        << "The existing target is ";
      switch(existing->GetType())
        {
        case cmTarget::EXECUTABLE:
          e << "an executable ";
          break;
        case cmTarget::STATIC_LIBRARY:
          e << "a static library ";
          break;
        case cmTarget::SHARED_LIBRARY:
          e << "a shared library ";
          break;
        case cmTarget::MODULE_LIBRARY:
          e << "a module library ";
          break;
        case cmTarget::UTILITY:
          e << "a custom target ";
          break;
        default: break;
        }
      e << "created in source directory \""
        << existing->GetMakefile()->GetCurrentDirectory() << "\".  "
        << "See documentation for policy CMP0002 for more details.";
      msg = e.str();
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmMakefile::EnforceUniqueDir(const char* srcPath, const char* binPath)
{
  // Make sure the binary directory is unique.
  cmGlobalGenerator* gg = this->LocalGenerator->GetGlobalGenerator();
  if(gg->BinaryDirectoryIsNew(binPath))
    {
    return true;
    }
  cmOStringStream e;
  switch (this->GetPolicyStatus(cmPolicies::CMP0013))
    {
    case cmPolicies::WARN:
      // Print the warning.
      e << this->GetPolicies()->GetPolicyWarning(cmPolicies::CMP0013)
        << "\n"
        << "The binary directory\n"
        << "  " << binPath << "\n"
        << "is already used to build a source directory.  "
        << "This command uses it to build source directory\n"
        << "  " << srcPath << "\n"
        << "which can generate conflicting build files.  "
        << "CMake does not support this use case but it used "
        << "to work accidentally and is being allowed for "
        << "compatibility.";
      this->IssueMessage(cmake::AUTHOR_WARNING, e.str());
    case cmPolicies::OLD:
      // OLD behavior does not warn.
      return true;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      e << this->GetPolicies()->GetRequiredPolicyError(cmPolicies::CMP0013)
        << "\n";
    case cmPolicies::NEW:
      // NEW behavior prints the error.
      e << "The binary directory\n"
        << "  " << binPath << "\n"
        << "is already used to build a source directory.  "
        << "It cannot be used to build source directory\n"
        << "  " << srcPath << "\n"
        << "Specify a unique binary directory name.";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      break;
    }

  return false;
}

//----------------------------------------------------------------------------
cmPolicies::PolicyStatus
cmMakefile::GetPolicyStatus(cmPolicies::PolicyID id)
{
  // Get the current setting of the policy.
  cmPolicies::PolicyStatus cur = this->GetPolicyStatusInternal(id);

  // If the policy is required to be set to NEW but is not, ignore the
  // current setting and tell the caller.
  if(cur != cmPolicies::NEW)
    {
    if(cur == cmPolicies::REQUIRED_ALWAYS ||
       cur == cmPolicies::REQUIRED_IF_USED)
      {
      return cur;
      }
    cmPolicies::PolicyStatus def = this->GetPolicies()->GetPolicyStatus(id);
    if(def == cmPolicies::REQUIRED_ALWAYS ||
       def == cmPolicies::REQUIRED_IF_USED)
      {
      return def;
      }
    }

  // The current setting is okay.
  return cur;
}

//----------------------------------------------------------------------------
cmPolicies::PolicyStatus
cmMakefile::GetPolicyStatusInternal(cmPolicies::PolicyID id)
{
  // Is the policy set in our stack?
  for(PolicyStackType::reverse_iterator psi = this->PolicyStack.rbegin();
      psi != this->PolicyStack.rend(); ++psi)
    {
    PolicyStackEntry::const_iterator pse = psi->find(id);
    if(pse != psi->end())
      {
      return pse->second;
      }
    }

  // If we have a parent directory, recurse up to it.
  if(this->LocalGenerator->GetParent())
    {
    cmMakefile* parent = this->LocalGenerator->GetParent()->GetMakefile();
    return parent->GetPolicyStatusInternal(id);
    }

  // The policy is not set.  Use the default for this CMake version.
  return this->GetPolicies()->GetPolicyStatus(id);
}

bool cmMakefile::SetPolicy(const char *id,
                           cmPolicies::PolicyStatus status)
{
  cmPolicies::PolicyID pid;
  if (!this->GetPolicies()->GetPolicyID(id, /* out */ pid))
    {
    cmOStringStream e;
    e << "Policy \"" << id << "\" is not known to this version of CMake.";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }
  return this->SetPolicy(pid,status);
}

//----------------------------------------------------------------------------
bool cmMakefile::SetPolicy(cmPolicies::PolicyID id,
                           cmPolicies::PolicyStatus status)
{
  // A REQUIRED_ALWAYS policy may be set only to NEW.
  if(status != cmPolicies::NEW &&
     this->GetPolicies()->GetPolicyStatus(id) ==
     cmPolicies::REQUIRED_ALWAYS)
    {
    std::string msg =
      this->GetPolicies()->GetRequiredAlwaysPolicyError(id);
    this->IssueMessage(cmake::FATAL_ERROR, msg.c_str());
    return false;
    }

  // Update the policy stack from the top to the top-most strong entry.
  bool previous_was_weak = true;
  for(PolicyStackType::reverse_iterator psi = this->PolicyStack.rbegin();
      previous_was_weak && psi != this->PolicyStack.rend(); ++psi)
    {
    (*psi)[id] = status;
    previous_was_weak = psi->Weak;
    }

  // Special hook for presenting compatibility variable as soon as
  // the user requests it.
  if(id == cmPolicies::CMP0001 &&
     (status == cmPolicies::WARN || status == cmPolicies::OLD))
    {
    if(!(this->GetCacheManager()
         ->GetCacheValue("CMAKE_BACKWARDS_COMPATIBILITY")))
      {
      // Set it to 2.4 because that is the last version where the
      // variable had meaning.
      this->AddCacheDefinition
        ("CMAKE_BACKWARDS_COMPATIBILITY", "2.4",
         "For backwards compatibility, what version of CMake "
         "commands and "
         "syntax should this version of CMake try to support.",
         cmCacheManager::STRING);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
cmMakefile::PolicyPushPop::PolicyPushPop(cmMakefile* m, bool weak,
                                         cmPolicies::PolicyMap const& pm):
  Makefile(m), ReportError(true)
{
  this->Makefile->PushPolicy(weak, pm);
  this->Makefile->PushPolicyBarrier();
}

//----------------------------------------------------------------------------
cmMakefile::PolicyPushPop::~PolicyPushPop()
{
  this->Makefile->PopPolicyBarrier(this->ReportError);
  this->Makefile->PopPolicy();
}

//----------------------------------------------------------------------------
void cmMakefile::PushPolicy(bool weak, cmPolicies::PolicyMap const& pm)
{
  // Allocate a new stack entry.
  this->PolicyStack.push_back(PolicyStackEntry(pm, weak));
}

//----------------------------------------------------------------------------
void cmMakefile::PopPolicy()
{
  if(this->PolicyStack.size() > this->PolicyBarriers.back())
    {
    this->PolicyStack.pop_back();
    }
  else
    {
    this->IssueMessage(cmake::FATAL_ERROR,
                       "cmake_policy POP without matching PUSH");
    }
}

//----------------------------------------------------------------------------
void cmMakefile::PushPolicyBarrier()
{
  this->PolicyBarriers.push_back(this->PolicyStack.size());
}

//----------------------------------------------------------------------------
void cmMakefile::PopPolicyBarrier(bool reportError)
{
  // Remove any extra entries pushed on the barrier.
  PolicyStackType::size_type barrier = this->PolicyBarriers.back();
  while(this->PolicyStack.size() > barrier)
    {
    if(reportError)
      {
      this->IssueMessage(cmake::FATAL_ERROR,
                         "cmake_policy PUSH without matching POP");
      reportError = false;
      }
    this->PopPolicy();
    }

  // Remove the barrier.
  this->PolicyBarriers.pop_back();
}

bool cmMakefile::SetPolicyVersion(const char *version)
{
  return this->GetCMakeInstance()->GetPolicies()->
    ApplyPolicyVersion(this,version);
}

cmPolicies *cmMakefile::GetPolicies()
{
  if (!this->GetCMakeInstance())
  {
    return 0;
  }
  return this->GetCMakeInstance()->GetPolicies();
}

//----------------------------------------------------------------------------
void cmMakefile::RecordPolicies(cmPolicies::PolicyMap& pm)
{
  /* Record the setting of every policy.  */
  typedef cmPolicies::PolicyID PolicyID;
  for(PolicyID pid = cmPolicies::CMP0000;
      pid != cmPolicies::CMPCOUNT; pid = PolicyID(pid+1))
    {
    pm[pid] = this->GetPolicyStatus(pid);
    }
}
