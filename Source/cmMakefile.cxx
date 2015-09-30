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
#include "cmCommands.h"
#include "cmState.h"
#include "cmOutputConverter.h"
#include "cmFunctionBlocker.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmListFileCache.h"
#include "cmCommandArgumentParserHelper.h"
#include "cmGeneratorExpression.h"
#include "cmTest.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
#  include "cmVariableWatch.h"
#endif
#include "cmInstallGenerator.h"
#include "cmTestGenerator.h"
#include "cmAlgorithms.h"
#include "cmake.h"
#include <stdlib.h> // required for atoi

#include <cmsys/RegularExpression.hxx>
#include <cmsys/FStream.hxx>
#include <cmsys/auto_ptr.hxx>

#include <list>
#include <ctype.h> // for isspace
#include <assert.h>

// default is not to be building executables
cmMakefile::cmMakefile(cmGlobalGenerator* globalGenerator,
                       cmState::Snapshot const& snapshot)
  : GlobalGenerator(globalGenerator),
    StateSnapshot(snapshot)
{
  this->IsSourceFileTryCompile = false;

  // Initialize these first since AddDefaultDefinitions calls AddDefinition
  this->WarnUnused = this->GetCMakeInstance()->GetWarnUnused();
  this->CheckSystemVars = this->GetCMakeInstance()->GetCheckSystemVars();

  this->SuppressWatches = false;

  // Setup the default include file regular expression (match everything).
  this->SetProperty("INCLUDE_REGULAR_EXPRESSION", "^.*$");
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

  this->AddDefaultDefinitions();

  this->cmDefineRegex.compile("#cmakedefine[ \t]+([A-Za-z_0-9]*)");
  this->cmDefine01Regex.compile("#cmakedefine01[ \t]+([A-Za-z_0-9]*)");
  this->cmAtVarRegex.compile("(@[A-Za-z_0-9/.+-]+@)");
  this->cmNamedCurly.compile("^[A-Za-z0-9/_.+-]+{");

  this->StateSnapshot = this->StateSnapshot.GetState()
      ->CreatePolicyScopeSnapshot(this->StateSnapshot);

  // Enter a policy level for this directory.
  this->PushPolicy();

  // push empty loop block
  this->PushLoopBlockBarrier();

  // By default the check is not done.  It is enabled by
  // cmListFileCache in the top level if necessary.
  this->CheckCMP0000 = false;

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

  {
  const char* dir = this->GetCMakeInstance()->GetHomeDirectory();
  this->AddDefinition("CMAKE_SOURCE_DIR", dir);
  this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR", dir);
  }
  {
  const char* dir = this->GetCMakeInstance()->GetHomeOutputDirectory();
  this->AddDefinition("CMAKE_BINARY_DIR", dir);
  this->AddDefinition("CMAKE_CURRENT_BINARY_DIR", dir);
  }
}

cmMakefile::~cmMakefile()
{
  cmDeleteAll(this->InstallGenerators);
  cmDeleteAll(this->TestGenerators);
  cmDeleteAll(this->SourceFiles);
  cmDeleteAll(this->Tests);
  cmDeleteAll(this->ImportedTargetsOwned);
  cmDeleteAll(this->FinalPassCommands);
  cmDeleteAll(this->FunctionBlockers);
  cmDeleteAll(this->EvaluationFiles);
  this->EvaluationFiles.clear();

  this->FunctionBlockers.clear();
}

//----------------------------------------------------------------------------
void cmMakefile::IssueMessage(cmake::MessageType t,
                              std::string const& text) const
{
  // Collect context information.
  if(!this->ExecutionStatusStack.empty())
    {
    if((t == cmake::FATAL_ERROR) || (t == cmake::INTERNAL_ERROR))
      {
      this->ExecutionStatusStack.back()->SetNestedError(true);
      }
    this->GetCMakeInstance()->IssueMessage(t, text, this->GetBacktrace());
    }
  else
    {
    cmListFileContext lfc;
    // We are not currently executing a command.  Add whatever context
    // information we have.
    lfc.FilePath = this->GetExecutionFilePath();

    if(!this->GetCMakeInstance()->GetIsInTryCompile())
      {
      cmOutputConverter converter(this->StateSnapshot);
      lfc.FilePath = converter.Convert(lfc.FilePath, cmOutputConverter::HOME);
      }
    lfc.Line = 0;
    this->GetCMakeInstance()->IssueMessage(t, text, lfc);
    }
}

cmStringRange cmMakefile::GetIncludeDirectoriesEntries() const
{
  return this->StateSnapshot.GetDirectory().GetIncludeDirectoriesEntries();
}

cmBacktraceRange cmMakefile::GetIncludeDirectoriesBacktraces() const
{
  return this->StateSnapshot.GetDirectory()
      .GetIncludeDirectoriesEntryBacktraces();
}

cmStringRange cmMakefile::GetCompileOptionsEntries() const
{
  return this->StateSnapshot.GetDirectory().GetCompileOptionsEntries();
}

cmBacktraceRange cmMakefile::GetCompileOptionsBacktraces() const
{
  return this->StateSnapshot.GetDirectory().GetCompileOptionsEntryBacktraces();
}

cmStringRange cmMakefile::GetCompileDefinitionsEntries() const
{
  return this->StateSnapshot.GetDirectory().GetCompileDefinitionsEntries();
}

cmBacktraceRange cmMakefile::GetCompileDefinitionsBacktraces() const
{
  return this->StateSnapshot.GetDirectory()
      .GetCompileDefinitionsEntryBacktraces();
}

//----------------------------------------------------------------------------
cmListFileBacktrace cmMakefile::GetBacktrace() const
{
  cmListFileBacktrace backtrace;
  if (!this->ContextStack.empty())
    {
    backtrace = cmListFileBacktrace(this->StateSnapshot,
                                    *this->ContextStack.back());
    }
  return backtrace;
}

//----------------------------------------------------------------------------
cmListFileBacktrace
cmMakefile::GetBacktrace(cmCommandContext const& cc) const
{
  cmState::Snapshot snp = this->StateSnapshot;
  return cmListFileBacktrace(snp, cc);
}

//----------------------------------------------------------------------------
cmListFileContext cmMakefile::GetExecutionContext() const
{
  return cmListFileContext::FromCommandContext(
        *this->ContextStack.back(),
        this->StateSnapshot.GetExecutionListFile());
}

//----------------------------------------------------------------------------
void cmMakefile::PrintCommandTrace(const cmListFileFunction& lff) const
{
  std::ostringstream msg;
  msg << this->GetExecutionFilePath() << "(" << lff.Line << "):  ";
  msg << lff.Name << "(";
  bool expand = this->GetCMakeInstance()->GetTraceExpand();
  std::string temp;
  for(std::vector<cmListFileArgument>::const_iterator i =
        lff.Arguments.begin(); i != lff.Arguments.end(); ++i)
    {
    if (expand)
      {
      temp = i->Value;
      this->ExpandVariablesInString(temp);
      msg << temp;
      }
    else
      {
      msg << i->Value;
      }
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
  if(cmCommand* proto = this->GetState()->GetCommand(name))
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
  IncludeScope(cmMakefile* mf, std::string const& filenametoread,
               bool noPolicyScope);
  ~IncludeScope();
  void Quiet() { this->ReportError = false; }
private:
  cmMakefile* Makefile;
  bool NoPolicyScope;
  bool CheckCMP0011;
  bool ReportError;
  void EnforceCMP0011();
};

//----------------------------------------------------------------------------
cmMakefile::IncludeScope::IncludeScope(cmMakefile* mf,
                                       std::string const& filenametoread,
                                       bool noPolicyScope):
  Makefile(mf), NoPolicyScope(noPolicyScope),
  CheckCMP0011(false), ReportError(true)
{
  this->Makefile->PushFunctionBlockerBarrier();

  this->Makefile->StateSnapshot =
      this->Makefile->GetState()->CreateCallStackSnapshot(
        this->Makefile->StateSnapshot,
        this->Makefile->ContextStack.back()->Name,
        this->Makefile->ContextStack.back()->Line,
        filenametoread);
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
}

//----------------------------------------------------------------------------
cmMakefile::IncludeScope::~IncludeScope()
{
  if(!this->NoPolicyScope)
    {
    // If we need to enforce policy CMP0011 then the top entry is the
    // one we pushed above.  If the entry is empty, then the included
    // script did not set any policies that might affect the includer so
    // we do not need to enforce the policy.
    if(this->CheckCMP0011
       && !this->Makefile->StateSnapshot.HasDefinedPolicyCMP0011())
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
  this->Makefile->PopPolicyBarrier(this->ReportError);

  this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
}

//----------------------------------------------------------------------------
void cmMakefile::IncludeScope::EnforceCMP0011()
{
  // We check the setting of this policy again because the included
  // script might actually set this policy for its includer.
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0011))
    {
    case cmPolicies::WARN:
      // Warn because the user did not set this policy.
      {
      std::ostringstream w;
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0011) << "\n"
        << "The included script\n  "
        << this->Makefile->GetExecutionFilePath() << "\n"
        << "affects policy settings.  "
        << "CMake is implying the NO_POLICY_SCOPE option for compatibility, "
        << "so the effects are applied to the including context.";
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      {
      std::ostringstream e;
      e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0011) << "\n"
        << "The included script\n  "
        << this->Makefile->GetExecutionFilePath() << "\n"
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

class cmParseFileScope
{
public:
  cmParseFileScope(cmMakefile* mf)
    : Makefile(mf)
  {
    this->Makefile->ContextStack.push_back(&this->Context);
  }

  ~cmParseFileScope()
  {
    this->Makefile->ContextStack.pop_back();
  }

private:
  cmMakefile* Makefile;
  cmCommandContext Context;
};

bool cmMakefile::ReadDependentFile(const char* filename, bool noPolicyScope)
{
  this->AddDefinition("CMAKE_PARENT_LIST_FILE",
                      this->GetDefinition("CMAKE_CURRENT_LIST_FILE"));
  std::string filenametoread =
    cmSystemTools::CollapseFullPath(filename,
                                    this->GetCurrentSourceDirectory());

  IncludeScope incScope(this, filenametoread, noPolicyScope);

  cmListFile listFile;
  {
  cmParseFileScope pfs(this);
  if (!listFile.ParseFile(filenametoread.c_str(), false, this))
    {
    return false;
    }
  }

  this->ReadListFile(listFile, filenametoread);
  if(cmSystemTools::GetFatalErrorOccured())
    {
    incScope.Quiet();
    }
  return true;
}

class cmMakefile::ListFileScope
{
public:
  ListFileScope(cmMakefile* mf, std::string const& filenametoread)
    : Makefile(mf), ReportError(true)
  {
    long line = 0;
    std::string name;
    if (!this->Makefile->ContextStack.empty())
      {
      line = this->Makefile->ContextStack.back()->Line;
      name = this->Makefile->ContextStack.back()->Name;
      }
    this->Makefile->StateSnapshot =
        this->Makefile->GetState()->CreateInlineListFileSnapshot(
          this->Makefile->StateSnapshot, name, line, filenametoread);
    assert(this->Makefile->StateSnapshot.IsValid());

    this->Makefile->PushFunctionBlockerBarrier();
  }

  ~ListFileScope()
  {
    this->Makefile->PopPolicyBarrier(this->ReportError);
    this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
  }

  void Quiet() { this->ReportError = false; }
private:
  cmMakefile* Makefile;
  bool ReportError;
};

bool cmMakefile::ReadListFile(const char* filename)
{
  std::string filenametoread =
    cmSystemTools::CollapseFullPath(filename,
                                    this->GetCurrentSourceDirectory());

  ListFileScope scope(this, filenametoread);

  cmListFile listFile;
  {
  cmParseFileScope pfs(this);
  if (!listFile.ParseFile(filenametoread.c_str(), false, this))
    {
    return false;
    }
  }

  this->ReadListFile(listFile, filenametoread);
  if(cmSystemTools::GetFatalErrorOccured())
    {
    scope.Quiet();
    }
  return true;
}

void cmMakefile::ReadListFile(cmListFile const& listFile,
                              std::string const& filenametoread)
{
  // add this list file to the list of dependencies
  this->ListFiles.push_back(filenametoread);

  std::string currentParentFile
      = this->GetSafeDefinition("CMAKE_PARENT_LIST_FILE");
  std::string currentFile
    = this->GetSafeDefinition("CMAKE_CURRENT_LIST_FILE");

  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", filenametoread.c_str());
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                       cmSystemTools::GetFilenamePath(filenametoread).c_str());

  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");

  // Run the parsed commands.
  const size_t numberFunctions = listFile.Functions.size();
  for(size_t i =0; i < numberFunctions; ++i)
    {
    cmExecutionStatus status;
    this->ExecuteCommand(listFile.Functions[i],status);
    if(cmSystemTools::GetFatalErrorOccured())
      {
      break;
      }
    if(status.GetReturnInvoked())
      {
      // Exit early due to return command.
      break;
      }
    }
  this->CheckForUnusedVariables();

  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile.c_str());
  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile.c_str());
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                      cmSystemTools::GetFilenamePath(currentFile).c_str());
  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");
}

//----------------------------------------------------------------------------
void cmMakefile::EnforceDirectoryLevelRules() const
{
  // Diagnose a violation of CMP0000 if necessary.
  if(this->CheckCMP0000)
    {
    std::ostringstream msg;
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
        this->IssueMessage(cmake::AUTHOR_WARNING, msg.str());
      case cmPolicies::OLD:
        // OLD behavior is to use policy version 2.4 set in
        // cmListFileCache.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // NEW behavior is to issue an error.
        this->IssueMessage(cmake::FATAL_ERROR, msg.str());
        cmSystemTools::SetFatalErrorOccured();
        return;
      }
    }
}

void cmMakefile::AddEvaluationFile(const std::string& inputFile,
                     cmsys::auto_ptr<cmCompiledGeneratorExpression> outputName,
                     cmsys::auto_ptr<cmCompiledGeneratorExpression> condition,
                     bool inputIsContent)
{
  this->EvaluationFiles.push_back(
              new cmGeneratorExpressionEvaluationFile(inputFile, outputName,
                                                      condition,
                                                      inputIsContent));
}

std::vector<cmGeneratorExpressionEvaluationFile*>
cmMakefile::GetEvaluationFiles() const
{
  return this->EvaluationFiles;
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
  std::vector<std::string>::iterator new_output_files_end = std::remove_if(
                                                     this->OutputFiles.begin(),
                                                     this->OutputFiles.end(),
                                                     file_not_persistent() );
  //we just have to erase all items at the back
  this->OutputFiles.erase(new_output_files_end, this->OutputFiles.end() );

  //if a configured file is used as input for another configured file,
  //and then deleted it will show up in the input list files so we
  //need to scan those too
  std::vector<std::string>::iterator new_list_files_end = std::remove_if(
                                                   this->ListFiles.begin(),
                                                   this->ListFiles.end(),
                                                   file_not_persistent() );

  this->ListFiles.erase(new_list_files_end, this->ListFiles.end() );
}

// Generate the output file
void cmMakefile::ConfigureFinalPass()
{
  this->FinalPass();
  const char* oldValue
    = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (oldValue && cmSystemTools::VersionCompare(
        cmSystemTools::OP_LESS, oldValue, "2.4"))
    {
    this->IssueMessage(
      cmake::FATAL_ERROR,
      "You have set CMAKE_BACKWARDS_COMPATIBILITY to a CMake version less "
      "than 2.4. This version of CMake only supports backwards compatibility "
      "with CMake 2.4 or later. For compatibility with older versions please "
      "use any CMake 2.8.x release or lower.");
    }
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); l++)
    {
    if (l->second.GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      continue;
      }
    l->second.FinishConfigure();
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandToTarget(const std::string& target,
                                   const std::vector<std::string>& byproducts,
                                     const std::vector<std::string>& depends,
                                     const cmCustomCommandLines& commandLines,
                                     cmTarget::CustomCommandType type,
                                     const char* comment,
                                     const char* workingDir,
                                     bool escapeOldStyle,
                                     bool uses_terminal)
{
  // Find the target to which to add the custom command.
  cmTargets::iterator ti = this->Targets.find(target);

  if(ti == this->Targets.end())
    {
    cmake::MessageType messageType = cmake::AUTHOR_WARNING;
    bool issueMessage = false;
    std::ostringstream e;
    switch(this->GetPolicyStatus(cmPolicies::CMP0040))
      {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0040) << "\n";
        issueMessage = true;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        issueMessage = true;
        messageType = cmake::FATAL_ERROR;
      }

    if(issueMessage)
      {
      e << "The target name \"" << target << "\" is unknown in this context.";
      IssueMessage(messageType, e.str());
      }

      return;
    }

  if(ti->second.GetType() == cmTarget::OBJECT_LIBRARY)
    {
    std::ostringstream e;
    e << "Target \"" << target << "\" is an OBJECT library "
      "that may not have PRE_BUILD, PRE_LINK, or POST_BUILD commands.";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }
  if(ti->second.GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    std::ostringstream e;
    e << "Target \"" << target << "\" is an INTERFACE library "
      "that may not have PRE_BUILD, PRE_LINK, or POST_BUILD commands.";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }

  // Always create the byproduct sources and mark them generated.
  for(std::vector<std::string>::const_iterator o = byproducts.begin();
      o != byproducts.end(); ++o)
    {
    if(cmSourceFile* out = this->GetOrCreateSource(*o, true))
      {
      out->SetProperty("GENERATED", "1");
      }
    }

  // Add the command to the appropriate build step for the target.
  std::vector<std::string> no_output;
  cmCustomCommand cc(this, no_output, byproducts, depends,
                     commandLines, comment, workingDir);
  cc.SetEscapeOldStyle(escapeOldStyle);
  cc.SetEscapeAllowMakeVars(true);
  cc.SetUsesTerminal(uses_terminal);
  switch(type)
    {
    case cmTarget::PRE_BUILD:
      ti->second.AddPreBuildCommand(cc);
      break;
    case cmTarget::PRE_LINK:
      ti->second.AddPreLinkCommand(cc);
      break;
    case cmTarget::POST_BUILD:
      ti->second.AddPostBuildCommand(cc);
      break;
    }
}

//----------------------------------------------------------------------------
cmSourceFile*
cmMakefile::AddCustomCommandToOutput(const std::vector<std::string>& outputs,
                                  const std::vector<std::string>& byproducts,
                                     const std::vector<std::string>& depends,
                                     const std::string& main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle,
                                     bool uses_terminal)
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
      std::ostringstream e;
      e << "COMMAND may not contain literal quotes:\n  " << cl[0] << "\n";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return 0;
      }
    }

  // Choose a source file on which to store the custom command.
  cmSourceFile* file = 0;
  if(!commandLines.empty() && !main_dependency.empty())
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
    else if (!file)
      {
      file = this->CreateSource(main_dependency);
      }
    }

  // Generate a rule file if the main dependency is not available.
  if(!file)
    {
    cmGlobalGenerator* gg = this->GetGlobalGenerator();

    // Construct a rule file associated with the first output produced.
    std::string outName = gg->GenerateRuleFile(outputs[0]);

    // Check if the rule file already exists.
    file = this->GetSource(outName);
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
    if (!file)
      {
      file = this->CreateSource(outName, true);
      }
    file->SetProperty("__CMAKE_RULE", "1");
    }

  // Always create the output sources and mark them generated.
  for(std::vector<std::string>::const_iterator o = outputs.begin();
      o != outputs.end(); ++o)
    {
    if(cmSourceFile* out = this->GetOrCreateSource(*o, true))
      {
      out->SetProperty("GENERATED", "1");
      }
    }
  for(std::vector<std::string>::const_iterator o = byproducts.begin();
      o != byproducts.end(); ++o)
    {
    if(cmSourceFile* out = this->GetOrCreateSource(*o, true))
      {
      out->SetProperty("GENERATED", "1");
      }
    }

  // Attach the custom command to the file.
  if(file)
    {
    // Construct a complete list of dependencies.
    std::vector<std::string> depends2(depends);
    if(!main_dependency.empty())
      {
      depends2.push_back(main_dependency);
      }

    cmCustomCommand* cc =
      new cmCustomCommand(this, outputs, byproducts, depends2,
                          commandLines, comment, workingDir);
    cc->SetEscapeOldStyle(escapeOldStyle);
    cc->SetEscapeAllowMakeVars(true);
    cc->SetUsesTerminal(uses_terminal);
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
  OutputToSourceMap::iterator i = this->OutputToSource.find(output);
  if(i != this->OutputToSource.end())
    {
    // Multiple custom commands produce the same output but may
    // be attached to a different source file (MAIN_DEPENDENCY).
    // LinearGetSourceFileWithOutput would return the first one,
    // so keep the mapping for the first one.
    //
    // TODO: Warn the user about this case.  However, the VS 8 generator
    // triggers it for separate generate.stamp rules in ZERO_CHECK and
    // individual targets.
    return;
    }
  this->OutputToSource[output] = source;
}

//----------------------------------------------------------------------------
cmSourceFile*
cmMakefile::AddCustomCommandToOutput(const std::string& output,
                                     const std::vector<std::string>& depends,
                                     const std::string& main_dependency,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment,
                                     const char* workingDir,
                                     bool replace,
                                     bool escapeOldStyle,
                                     bool uses_terminal)
{
  std::vector<std::string> outputs;
  outputs.push_back(output);
  std::vector<std::string> no_byproducts;
  return this->AddCustomCommandToOutput(outputs, no_byproducts,
                                        depends, main_dependency,
                                        commandLines, comment, workingDir,
                                        replace, escapeOldStyle,
                                        uses_terminal);
}

//----------------------------------------------------------------------------
void
cmMakefile::AddCustomCommandOldStyle(const std::string& target,
                                     const std::vector<std::string>& outputs,
                                     const std::vector<std::string>& depends,
                                     const std::string& source,
                                     const cmCustomCommandLines& commandLines,
                                     const char* comment)
{
  // Translate the old-style signature to one of the new-style
  // signatures.
  if(source == target)
    {
    // In the old-style signature if the source and target were the
    // same then it added a post-build rule to the target.  Preserve
    // this behavior.
    std::vector<std::string> no_byproducts;
    this->AddCustomCommandToTarget(target, no_byproducts,
                                   depends, commandLines,
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
      std::string no_main_dependency = "";
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
        this->Targets[target].AddSource(sf->GetFullPath());
        }
      else
        {
        cmSystemTools::Error("Attempt to add a custom rule to a target "
                             "that does not exist yet for target ",
                             target.c_str());
        return;
        }
      }
    }
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddUtilityCommand(const std::string& utilityName,
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
  return this->AddUtilityCommand(utilityName, excludeFromAll, workingDirectory,
                                 depends, commandLines);
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle, const char* comment,
                              bool uses_terminal)
{
  std::vector<std::string> no_byproducts;
  return this->AddUtilityCommand(utilityName, excludeFromAll, workingDirectory,
                                 no_byproducts, depends, commandLines,
                                 escapeOldStyle, comment, uses_terminal);
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddUtilityCommand(const std::string& utilityName,
                              bool excludeFromAll,
                              const char* workingDirectory,
                              const std::vector<std::string>& byproducts,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              bool escapeOldStyle, const char* comment,
                              bool uses_terminal)
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
  if (!commandLines.empty() || !depends.empty())
    {
    std::string force = this->GetCurrentBinaryDirectory();
    force += cmake::GetCMakeFilesDirectory();
    force += "/";
    force += utilityName;
    std::vector<std::string> forced;
    forced.push_back(force);
    std::string no_main_dependency = "";
    bool no_replace = false;
    this->AddCustomCommandToOutput(forced, byproducts,
                                   depends, no_main_dependency,
                                   commandLines, comment,
                                   workingDirectory, no_replace,
                                   escapeOldStyle, uses_terminal);
    cmSourceFile* sf = target->AddSourceCMP0049(force);

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

    // Always create the byproduct sources and mark them generated.
    for(std::vector<std::string>::const_iterator o = byproducts.begin();
        o != byproducts.end(); ++o)
      {
      if(cmSourceFile* out = this->GetOrCreateSource(*o, true))
        {
        out->SetProperty("GENERATED", "1");
        }
      }
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
  std::string::size_type initSize = dflags.size();
  dflags += std::string(" ") + flag;
  std::string::iterator flagStart = dflags.begin() + initSize + 1;
  std::replace(flagStart, dflags.end(), '\n', ' ');
  std::replace(flagStart, dflags.end(), '\r', ' ');
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
  if((this->GetGlobalGenerator()->GetName() == "Visual Studio 6") &&
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
          cmPolicies::GetPolicyWarning(cmPolicies::CMP0005)
          );
      case cmPolicies::OLD:
        // OLD behavior is to not escape the value.  We should not
        // convert the definition to use the property.
        return false;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          cmake::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0005)
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
      std::vector<std::string>::const_iterator defEnd =
          std::remove(defs.begin(), defs.end(), define);
      std::vector<std::string>::const_iterator defBegin =
          defs.begin();
      std::string ndefs = cmJoin(cmMakeRange(defBegin, defEnd), ";");

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

void cmMakefile::AddLinkLibrary(const std::string& lib,
                                cmTarget::LinkLibraryType llt)
{
  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back(tmp);
}

void cmMakefile::AddLinkLibraryForTarget(const std::string& target,
                                         const std::string& lib,
                                         cmTarget::LinkLibraryType llt)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    cmTarget* tgt = this->GetGlobalGenerator()->FindTarget(lib);
    if(tgt)
      {
      // if it is not a static or shared library then you can not link to it
      if(!((tgt->GetType() == cmTarget::STATIC_LIBRARY) ||
           (tgt->GetType() == cmTarget::SHARED_LIBRARY) ||
           (tgt->GetType() == cmTarget::INTERFACE_LIBRARY) ||
           tgt->IsExecutableWithExports()))
        {
        std::ostringstream e;
        e << "Target \"" << lib << "\" of type "
          << cmTarget::GetTargetTypeName(tgt->GetType())
          << " may not be linked into another target.  "
          << "One may link only to STATIC or SHARED libraries, or "
          << "to executables with the ENABLE_EXPORTS property set.";
        this->IssueMessage(cmake::FATAL_ERROR, e.str());
        }
      }
    i->second.AddLinkLibrary( *this, target, lib, llt );
    }
  else
    {
    std::ostringstream e;
    e << "Attempt to add link library \""
      << lib << "\" to target \""
      << target << "\" which is not built in this directory.";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
}

void cmMakefile::AddLinkDirectoryForTarget(const std::string& target,
                                           const std::string& d)
{
  cmTargets::iterator i = this->Targets.find(target);
  if ( i != this->Targets.end())
    {
    if(this->IsAlias(target))
      {
      std::ostringstream e;
      e << "ALIAS target \"" << target << "\" "
        << "may not be linked into another target.";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    i->second.AddLinkDirectory( d );
    }
  else
    {
    cmSystemTools::Error
      ("Attempt to add link directories to non-existent target: ",
       target.c_str(), " for directory ", d.c_str());
    }
}

void cmMakefile::AddLinkLibrary(const std::string& lib)
{
  this->AddLinkLibrary(lib,cmTarget::GENERAL);
}

void cmMakefile::InitializeFromParent(cmMakefile* parent)
{
  this->StateSnapshot.InitializeFromParent();

  this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR",
                      this->GetCurrentSourceDirectory());
  this->AddDefinition("CMAKE_CURRENT_BINARY_DIR",
                      this->GetCurrentBinaryDirectory());

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
  cmPolicies::PolicyStatus polSt = this->GetPolicyStatus(cmPolicies::CMP0043);
  if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD)
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
      const char* prop = parent->GetProperty(defPropName);
      this->SetProperty(defPropName, prop);
      }
    }

  // link libraries
  this->LinkLibraries = parent->LinkLibraries;

  // link directories
  this->SetProperty("LINK_DIRECTORIES",
                    parent->GetProperty("LINK_DIRECTORIES"));

  // the initial project name
  this->SetProjectName(parent->GetProjectName());

  // Copy include regular expressions.
  this->ComplainFileRegularExpression = parent->ComplainFileRegularExpression;

  // Imported targets.
  this->ImportedTargets = parent->ImportedTargets;
}

void cmMakefile::PushFunctionScope(std::string const& fileName,
                                   const cmPolicies::PolicyMap& pm)
{
  this->StateSnapshot =
      this->GetState()->CreateFunctionCallSnapshot(
        this->StateSnapshot,
        this->ContextStack.back()->Name, this->ContextStack.back()->Line,
        fileName);
  assert(this->StateSnapshot.IsValid());

  this->PushLoopBlockBarrier();

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->GetGlobalGenerator()->GetFileLockPool().PushFunctionScope();
#endif

  this->PushFunctionBlockerBarrier();

  this->PushPolicy(true, pm);
}

void cmMakefile::PopFunctionScope(bool reportError)
{
  this->PopPolicy();

  this->PopPolicyBarrier(reportError);

  this->PopFunctionBlockerBarrier(reportError);

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->GetGlobalGenerator()->GetFileLockPool().PopFunctionScope();
#endif

  this->PopLoopBlockBarrier();

  this->CheckForUnusedVariables();
}

void cmMakefile::PushMacroScope(std::string const& fileName,
                                const cmPolicies::PolicyMap& pm)
{
  this->StateSnapshot =
      this->GetState()->CreateMacroCallSnapshot(
        this->StateSnapshot,
        this->ContextStack.back()->Name, this->ContextStack.back()->Line,
        fileName);
  assert(this->StateSnapshot.IsValid());

  this->PushFunctionBlockerBarrier();

  this->PushPolicy(true, pm);
}

void cmMakefile::PopMacroScope(bool reportError)
{
  this->PopPolicy();
  this->PopPolicyBarrier(reportError);

  this->PopFunctionBlockerBarrier(reportError);
}

bool cmMakefile::IsRootMakefile() const
{
  return !this->StateSnapshot.GetBuildsystemDirectoryParent().IsValid();
}

class cmMakefile::BuildsystemFileScope
{
public:
  BuildsystemFileScope(cmMakefile* mf)
    : Makefile(mf), ReportError(true)
  {
    std::string currentStart =
        this->Makefile->StateSnapshot.GetDirectory().GetCurrentSource();
    currentStart += "/CMakeLists.txt";
    this->Makefile->StateSnapshot.SetListFile(currentStart);
    this->Makefile->StateSnapshot = this->Makefile->StateSnapshot.GetState()
        ->CreatePolicyScopeSnapshot(this->Makefile->StateSnapshot);
    this->Makefile->PushFunctionBlockerBarrier();

    this->GG = mf->GetGlobalGenerator();
    this->CurrentMakefile = this->GG->GetCurrentMakefile();
    this->Snapshot = this->GG->GetCMakeInstance()->GetCurrentSnapshot();
    this->GG->GetCMakeInstance()->SetCurrentSnapshot(this->Snapshot);
    this->GG->SetCurrentMakefile(mf);
#if defined(CMAKE_BUILD_WITH_CMAKE)
    this->GG->GetFileLockPool().PushFileScope();
#endif
  }

  ~BuildsystemFileScope()
  {
    this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
    this->Makefile->PopPolicyBarrier(this->ReportError);
#if defined(CMAKE_BUILD_WITH_CMAKE)
    this->GG->GetFileLockPool().PopFileScope();
#endif
    this->GG->SetCurrentMakefile(this->CurrentMakefile);
    this->GG->GetCMakeInstance()->SetCurrentSnapshot(this->Snapshot);
  }

  void Quiet() { this->ReportError = false; }
private:
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;
  cmMakefile* CurrentMakefile;
  cmState::Snapshot Snapshot;
  bool ReportError;
};

//----------------------------------------------------------------------------
void cmMakefile::Configure()
{
  BuildsystemFileScope scope(this);

  // make sure the CMakeFiles dir is there
  std::string filesDir = this->StateSnapshot.GetDirectory().GetCurrentBinary();
  filesDir += cmake::GetCMakeFilesDirectory();
  cmSystemTools::MakeDirectory(filesDir.c_str());

  std::string currentStart =
      this->StateSnapshot.GetDirectory().GetCurrentSource();
  currentStart += "/CMakeLists.txt";
  assert(cmSystemTools::FileExists(currentStart.c_str(), true));
  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentStart.c_str());

  cmListFile listFile;
  {
  cmParseFileScope pfs(this);
  if (!listFile.ParseFile(currentStart.c_str(), this->IsRootMakefile(), this))
    {
    return;
    }
  }
  this->ReadListFile(listFile, currentStart);
  if(cmSystemTools::GetFatalErrorOccured())
    {
    scope.Quiet();
    }

   // at the end handle any old style subdirs
  std::vector<cmMakefile*> subdirs = this->UnConfiguredDirectories;

  // for each subdir recurse
  std::vector<cmMakefile*>::iterator sdi = subdirs.begin();
  for (; sdi != subdirs.end(); ++sdi)
    {
    this->ConfigureSubDirectory(*sdi);
    }

  this->AddCMakeDependFilesFromUser();
}

void cmMakefile::ConfigureSubDirectory(cmMakefile *mf)
{
  mf->InitializeFromParent(this);
  std::string currentStart = mf->GetCurrentSourceDirectory();
  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Entering             ";
    msg += currentStart;
    cmSystemTools::Message(msg.c_str());
    }

  std::string const currentStartFile = currentStart + "/CMakeLists.txt";
  if (!cmSystemTools::FileExists(currentStartFile, true))
    {
    // The file is missing.  Check policy CMP0014.
    std::ostringstream e;
    e << "The source directory\n"
      << "  " << currentStart << "\n"
      << "does not contain a CMakeLists.txt file.";
    switch (this->GetPolicyStatus(cmPolicies::CMP0014))
      {
      case cmPolicies::WARN:
        // Print the warning.
        e << "\n"
          << "CMake does not support this case but it used "
          << "to work accidentally and is being allowed for "
          << "compatibility."
          << "\n"
          << cmPolicies::GetPolicyWarning(cmPolicies::CMP0014);
        this->IssueMessage(cmake::AUTHOR_WARNING, e.str());
      case cmPolicies::OLD:
        // OLD behavior does not warn.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        e << "\n"
          << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0014);
      case cmPolicies::NEW:
        // NEW behavior prints the error.
        this->IssueMessage(cmake::FATAL_ERROR, e.str());
      }
    return;
    }
  // finally configure the subdir
  mf->Configure();

  if (this->GetCMakeInstance()->GetDebugOutput())
    {
    std::string msg="   Returning to         ";
    msg += this->GetCurrentSourceDirectory();
    cmSystemTools::Message(msg.c_str());
    }
}

void cmMakefile::AddSubDirectory(const std::string& srcPath,
                                 const std::string& binPath,
                                 bool excludeFromAll,
                                 bool immediate)
{
  // Make sure the binary directory is unique.
  if(!this->EnforceUniqueDir(srcPath, binPath))
    {
    return;
    }

  cmState::Snapshot newSnapshot = this->GetState()
      ->CreateBuildsystemDirectorySnapshot(this->StateSnapshot,
                                           this->ContextStack.back()->Name,
                                           this->ContextStack.back()->Line);

  cmMakefile* subMf = new cmMakefile(this->GlobalGenerator, newSnapshot);
  this->GetGlobalGenerator()->AddMakefile(subMf);

  // set the subdirs start dirs
  subMf->SetCurrentSourceDirectory(srcPath);
  subMf->SetCurrentBinaryDirectory(binPath);
  if(excludeFromAll)
    {
    subMf->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
    }

  if (immediate)
    {
    this->ConfigureSubDirectory(subMf);
    }
  else
    {
    this->UnConfiguredDirectories.push_back(subMf);
    }
}

void cmMakefile::SetCurrentSourceDirectory(const std::string& dir)
{
  this->StateSnapshot.GetDirectory().SetCurrentSource(dir);
  this->AddDefinition("CMAKE_CURRENT_SOURCE_DIR",
                      this->StateSnapshot.GetDirectory().GetCurrentSource());
}

const char* cmMakefile::GetCurrentSourceDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentSource();
}

void cmMakefile::SetCurrentBinaryDirectory(const std::string& dir)
{
  this->StateSnapshot.GetDirectory().SetCurrentBinary(dir);
  const char* binDir = this->StateSnapshot.GetDirectory().GetCurrentBinary();
  cmSystemTools::MakeDirectory(binDir);
  this->AddDefinition("CMAKE_CURRENT_BINARY_DIR", binDir);
}

const char* cmMakefile::GetCurrentBinaryDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentBinary();
}

void cmMakefile::AddGeneratorTarget(cmTarget* t, cmGeneratorTarget* gt)
{
  this->GeneratorTargets[t] = gt;
  this->GetGlobalGenerator()->AddGeneratorTarget(t, gt);
}

//----------------------------------------------------------------------------
void cmMakefile::AddIncludeDirectories(const std::vector<std::string> &incs,
                                       bool before)
{
  if (incs.empty())
    {
    return;
    }

  cmListFileBacktrace lfbt = this->GetBacktrace();
  std::string entryString = cmJoin(incs, ";");
  if (before)
    {
    this->StateSnapshot.GetDirectory()
          .PrependIncludeDirectoriesEntry(entryString, lfbt);
    }
  else
    {
    this->StateSnapshot.GetDirectory()
          .AppendIncludeDirectoriesEntry(entryString, lfbt);
    }

  // Property on each target:
  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); ++l)
    {
    cmTarget &t = l->second;
    t.InsertInclude(entryString, lfbt, before);
    }
}

//----------------------------------------------------------------------------
void
cmMakefile::AddSystemIncludeDirectories(const std::set<std::string> &incs)
{
  this->SystemIncludeDirectories.insert(incs.begin(), incs.end());

  for (cmTargets::iterator l = this->Targets.begin();
       l != this->Targets.end(); ++l)
    {
    cmTarget &t = l->second;
    t.AddSystemIncludeDirectories(incs);
    }
}

void cmMakefile::AddDefinition(const std::string& name, const char* value)
{
  if (!value )
    {
    return;
    }

  if (this->VariableInitialized(name))
    {
    this->LogUnused("changing definition", name);
    }
  this->StateSnapshot.SetDefinition(name, value);

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


void cmMakefile::AddCacheDefinition(const std::string& name, const char* value,
                                    const char* doc,
                                    cmState::CacheEntryType type,
                                    bool force)
{
  bool haveVal = value ? true : false;
  std::string val = haveVal ? value : "";
  const char* existingValue =
    this->GetState()->GetInitializedCacheValue(name);
  if(existingValue
      && (this->GetState()->GetCacheEntryType(name)
                                            == cmState::UNINITIALIZED))
    {
    // if this is not a force, then use the value from the cache
    // if it is a force, then use the value being passed in
    if(!force)
      {
      val = existingValue;
      haveVal = true;
      }
    if ( type == cmState::PATH || type == cmState::FILEPATH )
      {
      std::vector<std::string>::size_type cc;
      std::vector<std::string> files;
      std::string nvalue = "";
      cmSystemTools::ExpandListArgument(val, files);
      for ( cc = 0; cc < files.size(); cc ++ )
        {
        if(!cmSystemTools::IsOff(files[cc].c_str()))
          {
          files[cc] = cmSystemTools::CollapseFullPath(files[cc]);
          }
        if ( cc > 0 )
          {
          nvalue += ";";
          }
        nvalue += files[cc];
        }

      this->GetState()->AddCacheEntry(name, nvalue.c_str(), doc, type);
      val = this->GetState()->GetInitializedCacheValue(name);
      haveVal = true;
      }

    }
  this->GetState()->AddCacheEntry(name, haveVal ? val.c_str() : 0,
                                          doc, type);
  // if there was a definition then remove it
  this->StateSnapshot.RemoveDefinition(name);
}


void cmMakefile::AddDefinition(const std::string& name, bool value)
{
  if (this->VariableInitialized(name))
    {
    this->LogUnused("changing definition", name);
    }

  this->StateSnapshot.SetDefinition(name, value ? "ON" : "OFF");

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
  const std::vector<std::string>& unused = this->StateSnapshot.UnusedKeys();
  std::vector<std::string>::const_iterator it = unused.begin();
  for (; it != unused.end(); ++it)
    {
    this->LogUnused("out of scope", *it);
    }
}

void cmMakefile::MarkVariableAsUsed(const std::string& var)
{
  this->StateSnapshot.GetDefinition(var);
}

bool cmMakefile::VariableInitialized(const std::string& var) const
{
  return this->StateSnapshot.IsInitialized(var);
}

void cmMakefile::LogUnused(const char* reason,
                                const std::string& name) const
{
  if (this->WarnUnused)
    {
    std::string path;
    cmListFileContext lfc;
    if (!this->ExecutionStatusStack.empty())
      {
      lfc = this->GetExecutionContext();
      path = lfc.FilePath;
      }
    else
      {
      path = this->GetCurrentSourceDirectory();
      path += "/CMakeLists.txt";
      lfc.FilePath = path;
      lfc.Line = 0;
      }
    cmOutputConverter converter(this->StateSnapshot);
    lfc.FilePath = converter.Convert(lfc.FilePath, cmOutputConverter::HOME);

    if (this->CheckSystemVars ||
        cmSystemTools::IsSubDirectory(path,
                                      this->GetHomeDirectory()) ||
        (cmSystemTools::IsSubDirectory(path,
                                      this->GetHomeOutputDirectory()) &&
        !cmSystemTools::IsSubDirectory(path,
                                cmake::GetCMakeFilesDirectory())))
      {
      std::ostringstream msg;
      msg << "unused variable (" << reason << ") \'" << name << "\'";
      this->GetCMakeInstance()->IssueMessage(cmake::AUTHOR_WARNING,
                                             msg.str(),
                                             lfc);
      }
    }
}

void cmMakefile::RemoveDefinition(const std::string& name)
{
  if (this->VariableInitialized(name))
    {
    this->LogUnused("unsetting", name);
    }
  this->StateSnapshot.RemoveDefinition(name);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv )
    {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS,
      0, this);
    }
#endif
}

void cmMakefile::RemoveCacheDefinition(const std::string& name)
{
  this->GetState()->RemoveCacheEntry(name);
}

void cmMakefile::SetProjectName(std::string const& p)
{
  this->StateSnapshot.SetProjectName(p);
}

std::string cmMakefile::GetProjectName() const
{
  return this->StateSnapshot.GetProjectName();
}

void cmMakefile::AddGlobalLinkInformation(const std::string& name,
                                          cmTarget& target)
{
  // for these targets do not add anything
  switch(target.GetType())
    {
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INTERFACE_LIBRARY:
      return;
    default:;
    }
  if (const char* linkDirsProp = this->GetProperty("LINK_DIRECTORIES"))
    {
    std::vector<std::string> linkDirs;
    cmSystemTools::ExpandListArgument(linkDirsProp, linkDirs);

    for(std::vector<std::string>::iterator j = linkDirs.begin();
        j != linkDirs.end(); ++j)
      {
      std::string newdir = *j;
      // remove trailing slashes
      if(*j->rbegin() == '/')
        {
        newdir = j->substr(0, j->size()-1);
        }
      if(std::find(this->LinkDirectories.begin(),
                   this->LinkDirectories.end(), newdir)
          == this->LinkDirectories.end())
        {
        target.AddLinkDirectory(*j);
        }
      }
    }
  target.MergeLinkLibraries( *this, name, this->LinkLibraries );
}


void cmMakefile::AddAlias(const std::string& lname, cmTarget *tgt)
{
  this->AliasTargets[lname] = tgt;
  this->GetGlobalGenerator()->AddAlias(lname, tgt);
}

cmTarget* cmMakefile::AddLibrary(const std::string& lname,
                            cmTarget::TargetType type,
                            const std::vector<std::string> &srcs,
                            bool excludeFromAll)
{
  // wrong type ? default to STATIC
  if (    (type != cmTarget::STATIC_LIBRARY)
       && (type != cmTarget::SHARED_LIBRARY)
       && (type != cmTarget::MODULE_LIBRARY)
       && (type != cmTarget::OBJECT_LIBRARY)
       && (type != cmTarget::INTERFACE_LIBRARY))
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
cmMakefile::AddNewTarget(cmTarget::TargetType type, const std::string& name)
{
  cmTargets::iterator it =
    this->Targets.insert(cmTargets::value_type(name, cmTarget())).first;
  cmTarget& target = it->second;
  target.SetType(type, name);
  target.SetMakefile(this);
  this->GetGlobalGenerator()->AddTarget(&it->second);
  return &it->second;
}

cmSourceFile*
cmMakefile::LinearGetSourceFileWithOutput(const std::string& name) const
{
  std::string out;

  // look through all the source files that have custom commands
  // and see if the custom command has the passed source file as an output
  for(std::vector<cmSourceFile*>::const_iterator i =
        this->SourceFiles.begin(); i != this->SourceFiles.end(); ++i)
    {
    // does this source file have a custom command?
    if ((*i)->GetCustomCommand())
      {
      // Does the output of the custom command match the source file name?
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

cmSourceFile *cmMakefile::GetSourceFileWithOutput(
                                                const std::string& name) const
{
  // If the queried path is not absolute we use the backward compatible
  // linear-time search for an output with a matching suffix.
  if(!cmSystemTools::FileIsFullPath(name.c_str()))
    {
    return this->LinearGetSourceFileWithOutput(name);
    }
  // Otherwise we use an efficient lookup map.
  OutputToSourceMap::const_iterator o = this->OutputToSource.find(name);
  if (o != this->OutputToSource.end())
    {
    return (*o).second;
    }
  return 0;
}

#if defined(CMAKE_BUILD_WITH_CMAKE)
cmSourceGroup*
cmMakefile::GetSourceGroup(const std::vector<std::string>&name) const
{
  cmSourceGroup* sg = 0;

  // first look for source group starting with the same as the one we wants
  for (std::vector<cmSourceGroup>::const_iterator
      sgIt = this->SourceGroups.begin();
      sgIt != this->SourceGroups.end(); ++sgIt)
    {
    std::string sgName = sgIt->GetName();
    if(sgName == name[0])
      {
      sg = const_cast<cmSourceGroup*>(&(*sgIt));
      break;
      }
    }

  if(sg != 0)
    {
    // iterate through its children to find match source group
    for(unsigned int i=1; i<name.size(); ++i)
      {
      sg = sg->LookupChild(name[i].c_str());
      if(sg == 0)
        {
        break;
        }
      }
    }
  return sg;
}

void cmMakefile::AddSourceGroup(const std::string& name,
                                 const char* regex)
{
  std::vector<std::string> nameVector;
  nameVector.push_back(name);
  AddSourceGroup(nameVector, regex);
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
    if ( regex && sg)
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
  if(!sg)
    {
    cmSystemTools::Error("Could not create source group ");
    return;
    }
  // build the whole source group path
  for(++i; i<=lastElement; ++i)
    {
    sg->AddChild(cmSourceGroup(name[i].c_str(), 0, sg->GetFullName()));
    sg = sg->LookupChild(name[i].c_str());
    }

  sg->SetGroupRegex(regex);
}

#endif

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
  std::ostringstream w;

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
    if (t.GetType() == cmTarget::INTERFACE_LIBRARY
        || t.GetType() == cmTarget::GLOBAL_TARGET)
      {
      continue;
      }
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

  if (const char* linkDirsProp = this->GetProperty("LINK_DIRECTORIES"))
    {
    if(mightExpandVariablesCMP0019(linkDirsProp))
      {
      std::string d = linkDirsProp;
      std::string orig = linkDirsProp;
      this->ExpandVariablesInString(d, true, true);
      if(pol == cmPolicies::WARN && d != orig)
        {
        w << "Evaluated link directories\n"
          << "  " << orig << "\n"
          << "as\n"
          << "  " << d << "\n";
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
    std::ostringstream m;
    m << cmPolicies::GetPolicyWarning(cmPolicies::CMP0019)
      << "\n"
      << "The following variable evaluations were encountered:\n"
      << w.str();
    this->IssueMessage(cmake::AUTHOR_WARNING, m.str());
    }
}

bool cmMakefile::IsOn(const std::string& name) const
{
  const char* value = this->GetDefinition(name);
  return cmSystemTools::IsOn(value);
}

bool cmMakefile::IsSet(const std::string& name) const
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

bool cmMakefile::PlatformIsAppleIos() const
{
  std::string sdkRoot;
  sdkRoot = this->GetSafeDefinition("CMAKE_OSX_SYSROOT");
  sdkRoot = cmSystemTools::LowerCase(sdkRoot);

  return sdkRoot.find("iphoneos") == 0 ||
         sdkRoot.find("/iphoneos") != std::string::npos ||
         sdkRoot.find("iphonesimulator") == 0 ||
         sdkRoot.find("/iphonesimulator") != std::string::npos;
}

const char* cmMakefile::GetSONameFlag(const std::string& language) const
{
  std::string name = "CMAKE_SHARED_LIBRARY_SONAME";
  if(!language.empty())
    {
    name += "_";
    name += language;
    }
  name += "_FLAG";
  return GetDefinition(name);
}

bool cmMakefile::CanIWriteThisFile(const char* fileName) const
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

const char* cmMakefile::GetRequiredDefinition(const std::string& name) const
{
  const char* ret = this->GetDefinition(name);
  if(!ret)
    {
    cmSystemTools::Error("Error required internal CMake variable not "
                         "set, cmake may be not be built correctly.\n",
                         "Missing variable is:\n",
                         name.c_str());
    return "";
    }
  return ret;
}

bool cmMakefile::IsDefinitionSet(const std::string& name) const
{
  const char* def = this->StateSnapshot.GetDefinition(name);
  if(!def)
    {
    def = this->GetState()->GetInitializedCacheValue(name);
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

const char* cmMakefile::GetDefinition(const std::string& name) const
{
  const char* def = this->StateSnapshot.GetDefinition(name);
  if(!def)
    {
    def = this->GetState()->GetInitializedCacheValue(name);
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmVariableWatch* vv = this->GetVariableWatch();
  if ( vv && !this->SuppressWatches )
    {
    if ( def )
      {
      vv->VariableAccessed(name, cmVariableWatch::VARIABLE_READ_ACCESS,
        def, this);
      }
    else
      {
      vv->VariableAccessed(name,
          cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS, def, this);
      }
    }
#endif
  return def;
}

const char* cmMakefile::GetSafeDefinition(const std::string& def) const
{
  const char* ret = this->GetDefinition(def);
  if(!ret)
    {
    return "";
    }
  return ret;
}

std::vector<std::string> cmMakefile::GetDefinitions() const
{
  std::vector<std::string> res = this->StateSnapshot.ClosureKeys();
  std::vector<std::string> cacheKeys = this->GetState()->GetCacheEntryKeys();
  res.insert(res.end(), cacheKeys.begin(), cacheKeys.end());
  std::sort(res.begin(), res.end());
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
                                                bool removeEmpty,
                                                bool replaceAt) const
{
  bool compareResults = false;
  cmake::MessageType mtype = cmake::LOG;
  std::string errorstr;
  std::string original;

  // Sanity check the @ONLY mode.
  if(atOnly && (!noEscapes || !removeEmpty))
    {
    // This case should never be called.  At-only is for
    // configure-file/string which always does no escapes.
    this->IssueMessage(cmake::INTERNAL_ERROR,
                       "ExpandVariablesInString @ONLY called "
                       "on something with escapes.");
    return source.c_str();
    }

  // Variables used in the WARN case.
  std::string newResult;
  std::string newErrorstr;
  cmake::MessageType newError = cmake::LOG;

  switch(this->GetPolicyStatus(cmPolicies::CMP0053))
    {
    case cmPolicies::WARN:
      {
      // Save the original string for the warning.
      original = source;
      newResult = source;
      compareResults = true;
      // Suppress variable watches to avoid calling hooks twice. Suppress new
      // dereferences since the OLD behavior is still what is actually used.
      this->SuppressWatches = true;
      newError =
        ExpandVariablesInStringNew(newErrorstr, newResult, escapeQuotes,
                                   noEscapes, atOnly, filename, line,
                                   removeEmpty, replaceAt);
      this->SuppressWatches = false;
      }
    case cmPolicies::OLD:
      mtype = ExpandVariablesInStringOld(errorstr, source, escapeQuotes,
                                         noEscapes, atOnly, filename,
                                         line, removeEmpty, true);
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      // Messaging here would be *very* verbose.
    case cmPolicies::NEW:
      mtype = ExpandVariablesInStringNew(errorstr, source, escapeQuotes,
                                         noEscapes, atOnly, filename,
                                         line, removeEmpty, replaceAt);
      break;
    }

  // If it's an error in either case, just report the error...
  if(mtype != cmake::LOG)
    {
    if(mtype == cmake::FATAL_ERROR)
      {
      cmSystemTools::SetFatalErrorOccured();
      }
    this->IssueMessage(mtype, errorstr);
    }
  // ...otherwise, see if there's a difference that needs to be warned about.
  else if(compareResults && (newResult != source || newError != mtype))
    {
    std::string msg =
      cmPolicies::GetPolicyWarning(cmPolicies::CMP0053);
    msg += "\n";

    std::string msg_input = original;
    cmSystemTools::ReplaceString(msg_input, "\n", "\n  ");
    msg += "For input:\n  '";
    msg += msg_input;
    msg += "'\n";

    std::string msg_old = source;
    cmSystemTools::ReplaceString(msg_old, "\n", "\n  ");
    msg += "the old evaluation rules produce:\n  '";
    msg += msg_old;
    msg += "'\n";

    if(newError == mtype)
      {
      std::string msg_new = newResult;
      cmSystemTools::ReplaceString(msg_new, "\n", "\n  ");
      msg += "but the new evaluation rules produce:\n  '";
      msg += msg_new;
      msg += "'\n";
      }
    else
      {
      std::string msg_err = newErrorstr;
      cmSystemTools::ReplaceString(msg_err, "\n", "\n  ");
      msg += "but the new evaluation rules produce an error:\n  ";
      msg += msg_err;
      msg += "\n";
      }

    msg +=
      "Using the old result for compatibility since the policy is not set.";

    this->IssueMessage(cmake::AUTHOR_WARNING, msg);
    }

  return source.c_str();
}

cmake::MessageType cmMakefile::ExpandVariablesInStringOld(
                                                std::string& errorstr,
                                                std::string& source,
                                                bool escapeQuotes,
                                                bool noEscapes,
                                                bool atOnly,
                                                const char* filename,
                                                long line,
                                                bool removeEmpty,
                                                bool replaceAt) const
{
  // Fast path strings without any special characters.
  if ( source.find_first_of("$@\\") == source.npos)
    {
    return cmake::LOG;
    }

  // Special-case the @ONLY mode.
  if(atOnly)
    {
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
      if(const char* val = this->GetDefinition(var))
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

    return cmake::LOG;
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
  cmake::MessageType mtype = cmake::LOG;
  if ( res && !emsg[0] )
    {
    source = parser.GetResult();
    }
  else
    {
    // Construct the main error message.
    std::ostringstream error;
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
          << "  " << source << "\n";
    error << emsg;

    // If the parser failed ("res" is false) then this is a real
    // argument parsing error, so the policy applies.  Otherwise the
    // parser reported an error message without failing because the
    // helper implementation is unhappy, which has always reported an
    // error.
    mtype = cmake::FATAL_ERROR;
    if(!res)
      {
      // This is a real argument parsing error.  Use policy CMP0010 to
      // decide whether it is an error.
      switch(this->GetPolicyStatus(cmPolicies::CMP0010))
        {
        case cmPolicies::WARN:
          error << "\n" << cmPolicies::GetPolicyWarning(cmPolicies::CMP0010);
        case cmPolicies::OLD:
          // OLD behavior is to just warn and continue.
          mtype = cmake::AUTHOR_WARNING;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          error << "\n"
                << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0010);
        case cmPolicies::NEW:
          // NEW behavior is to report the error.
          break;
        }
      }
    errorstr = error.str();
    }
  return mtype;
}

typedef enum
  {
  NORMAL,
  ENVIRONMENT,
  CACHE
  } t_domain;
struct t_lookup
  {
  t_lookup(): domain(NORMAL), loc(0) {}
  t_domain domain;
  size_t loc;
  };

cmake::MessageType cmMakefile::ExpandVariablesInStringNew(
                                            std::string& errorstr,
                                            std::string& source,
                                            bool escapeQuotes,
                                            bool noEscapes,
                                            bool atOnly,
                                            const char* filename,
                                            long line,
                                            bool removeEmpty,
                                            bool replaceAt) const
{
  // This method replaces ${VAR} and @VAR@ where VAR is looked up
  // with GetDefinition(), if not found in the map, nothing is expanded.
  // It also supports the $ENV{VAR} syntax where VAR is looked up in
  // the current environment variables.

  const char* in = source.c_str();
  const char* last = in;
  std::string result;
  result.reserve(source.size());
  std::stack<t_lookup> openstack;
  bool error = false;
  bool done = false;
  openstack.push(t_lookup());
  cmake::MessageType mtype = cmake::LOG;

  cmState* state = this->GetCMakeInstance()->GetState();

  do
    {
    char inc = *in;
    switch(inc)
      {
      case '}':
        if(openstack.size() > 1)
          {
          t_lookup var = openstack.top();
          openstack.pop();
          result.append(last, in - last);
          std::string const& lookup = result.substr(var.loc);
          const char* value = NULL;
          std::string varresult;
          static const std::string lineVar = "CMAKE_CURRENT_LIST_LINE";
          switch(var.domain)
            {
            case NORMAL:
              if(filename && lookup == lineVar)
                {
                std::ostringstream ostr;
                ostr << line;
                varresult = ostr.str();
                }
              else
                {
                value = this->GetDefinition(lookup);
                }
              break;
            case ENVIRONMENT:
              value = cmSystemTools::GetEnv(lookup.c_str());
              break;
            case CACHE:
              value = state->GetCacheEntryValue(lookup);
              break;
            }
          // Get the string we're meant to append to.
          if(value)
            {
            if(escapeQuotes)
              {
              varresult = cmSystemTools::EscapeQuotes(value);
              }
            else
              {
              varresult = value;
              }
            }
          else if(!removeEmpty)
            {
            // check to see if we need to print a warning
            // if strict mode is on and the variable has
            // not been "cleared"/initialized with a set(foo ) call
            if(this->GetCMakeInstance()->GetWarnUninitialized() &&
               !this->VariableInitialized(lookup))
              {
              if (this->CheckSystemVars ||
                  cmSystemTools::IsSubDirectory(filename,
                                                this->GetHomeDirectory()) ||
                  cmSystemTools::IsSubDirectory(filename,
                                             this->GetHomeOutputDirectory()))
                {
                std::ostringstream msg;
                cmListFileContext lfc;
                cmOutputConverter converter(this->StateSnapshot);
                lfc.FilePath =
                    converter.Convert(filename, cmOutputConverter::HOME);
                lfc.Line = line;
                msg << "uninitialized variable \'" << lookup << "\'";
                this->GetCMakeInstance()->IssueMessage(cmake::AUTHOR_WARNING,
                                                       msg.str(), lfc);
                }
              }
            }
          result.replace(var.loc, result.size() - var.loc, varresult);
          // Start looking from here on out.
          last = in + 1;
          }
        break;
      case '$':
        if(!atOnly)
          {
          t_lookup lookup;
          const char* next = in + 1;
          const char* start = NULL;
          char nextc = *next;
          if(nextc == '{')
            {
            // Looking for a variable.
            start = in + 2;
            lookup.domain = NORMAL;
            }
          else if(nextc == '<')
            {
            }
          else if(!nextc)
            {
            result.append(last, next - last);
            last = next;
            }
          else if(cmHasLiteralPrefix(next, "ENV{"))
            {
            // Looking for an environment variable.
            start = in + 5;
            lookup.domain = ENVIRONMENT;
            }
          else if(cmHasLiteralPrefix(next, "CACHE{"))
            {
            // Looking for a cache variable.
            start = in + 7;
            lookup.domain = CACHE;
            }
          else
            {
            if(this->cmNamedCurly.find(next))
              {
              errorstr = "Syntax $"
                  + std::string(next, this->cmNamedCurly.end())
                  + "{} is not supported.  Only ${}, $ENV{}, "
                    "and $CACHE{} are allowed.";
              mtype = cmake::FATAL_ERROR;
              error = true;
              }
            }
          if(start)
            {
            result.append(last, in - last);
            last = start;
            in = start - 1;
            lookup.loc = result.size();
            openstack.push(lookup);
            }
          break;
          }
      case '\\':
        if(!noEscapes)
          {
          const char* next = in + 1;
          char nextc = *next;
          if(nextc == 't')
            {
            result.append(last, in - last);
            result.append("\t");
            last = next + 1;
            }
          else if(nextc == 'n')
            {
            result.append(last, in - last);
            result.append("\n");
            last = next + 1;
            }
          else if(nextc == 'r')
            {
            result.append(last, in - last);
            result.append("\r");
            last = next + 1;
            }
          else if(nextc == ';' && openstack.size() == 1)
            {
            // Handled in ExpandListArgument; pass the backslash literally.
            }
          else if (isalnum(nextc) || nextc == '\0')
            {
            errorstr += "Invalid character escape '\\";
            if (nextc)
              {
              errorstr += nextc;
              errorstr += "'.";
              }
            else
              {
              errorstr += "' (at end of input).";
              }
            error = true;
            }
          else
            {
            // Take what we've found so far, skipping the escape character.
            result.append(last, in - last);
            // Start tracking from the next character.
            last = in + 1;
            }
          // Skip the next character since it was escaped, but don't read past
          // the end of the string.
          if(*last)
            {
            ++in;
            }
          }
        break;
      case '\n':
        // Onto the next line.
        ++line;
        break;
      case '\0':
        done = true;
        break;
      case '@':
        if(replaceAt)
          {
          const char* nextAt = strchr(in + 1, '@');
          if(nextAt && nextAt != in + 1 &&
             nextAt == in + 1 + strspn(in + 1,
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789/_.+-"))
            {
            std::string variable(in + 1, nextAt - in - 1);
            std::string varresult = this->GetSafeDefinition(variable);
            if(escapeQuotes)
              {
              varresult = cmSystemTools::EscapeQuotes(varresult);
              }
            // Skip over the variable.
            result.append(last, in - last);
            result.append(varresult);
            in = nextAt;
            last = in + 1;
            break;
            }
          }
        // Failed to find a valid @ expansion; treat it as literal.
        /* FALLTHROUGH */
      default:
        {
        if(openstack.size() > 1 &&
           !(isalnum(inc) || inc == '_' ||
             inc == '/' || inc == '.' ||
             inc == '+' || inc == '-'))
          {
          errorstr += "Invalid character (\'";
          errorstr += inc;
          result.append(last, in - last);
          errorstr += "\') in a variable name: "
                      "'" + result.substr(openstack.top().loc) + "'";
          mtype = cmake::FATAL_ERROR;
          error = true;
          }
        break;
        }
      }
    // Look at the next character.
    } while(!error && !done && *++in);

  // Check for open variable references yet.
  if(!error && openstack.size() != 1)
    {
    // There's an open variable reference waiting.  Policy CMP0010 flags
    // whether this is an error or not.  The new parser now enforces
    // CMP0010 as well.
    errorstr += "There is an unterminated variable reference.";
    error = true;
    }

  if(error)
    {
    std::ostringstream emsg;
    emsg << "Syntax error in cmake code ";
    if(filename)
      {
      // This filename and line number may be more specific than the
      // command context because one command invocation can have
      // arguments on multiple lines.
      emsg << "at\n"
            << "  " << filename << ":" << line << "\n";
      }
    emsg << "when parsing string\n"
         << "  " << source << "\n";
    emsg << errorstr;
    mtype = cmake::FATAL_ERROR;
    errorstr = emsg.str();
    }
  else
    {
    // Append the rest of the unchanged part of the string.
    result.append(last);

    source = result;
    }

  return mtype;
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
std::string
cmMakefile::GetConfigurations(std::vector<std::string>& configs,
                              bool singleConfig) const
{
  if(this->GetGlobalGenerator()->IsMultiConfig())
    {
    if(const char* configTypes =
       this->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
      {
      cmSystemTools::ExpandListArgument(configTypes, configs);
      }
    return "";
    }
  else
    {
    const std::string& buildType = this->GetSafeDefinition("CMAKE_BUILD_TYPE");
    if(singleConfig && !buildType.empty())
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
cmSourceGroup*
cmMakefile::FindSourceGroup(const char* source,
                            std::vector<cmSourceGroup> &groups) const
{
  // First search for a group that lists the file explicitly.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    cmSourceGroup *result = sg->MatchChildrenFiles(source);
    if(result)
      {
      return result;
      }
    }

  // Now search for a group whose regex matches the file.
  for(std::vector<cmSourceGroup>::reverse_iterator sg = groups.rbegin();
      sg != groups.rend(); ++sg)
    {
    cmSourceGroup *result = sg->MatchChildrenRegex(source);
    if(result)
      {
      return result;
      }
    }


  // Shouldn't get here, but just in case, return the default group.
  return &groups.front();
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
      std::ostringstream e;
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

//----------------------------------------------------------------------------
void cmMakefile::PushLoopBlock()
{
  assert(!this->LoopBlockCounter.empty());
  this->LoopBlockCounter.top()++;
}

void cmMakefile::PopLoopBlock()
{
  assert(!this->LoopBlockCounter.empty());
  assert(this->LoopBlockCounter.top() > 0);
  this->LoopBlockCounter.top()--;
}

void cmMakefile::PushLoopBlockBarrier()
{
  this->LoopBlockCounter.push(0);
}

void cmMakefile::PopLoopBlockBarrier()
{
  assert(!this->LoopBlockCounter.empty());
  assert(this->LoopBlockCounter.top() == 0);
  this->LoopBlockCounter.pop();
}

bool cmMakefile::IsLoopBlock() const
{
  assert(!this->LoopBlockCounter.empty());
  return !this->LoopBlockCounter.empty() && this->LoopBlockCounter.top() > 0;
}

std::string cmMakefile::GetExecutionFilePath() const
{
  assert(this->StateSnapshot.IsValid());
  return this->StateSnapshot.GetExecutionListFile();
}

//----------------------------------------------------------------------------
bool cmMakefile::ExpandArguments(
  std::vector<cmListFileArgument> const& inArgs,
  std::vector<std::string>& outArgs, const char* filename) const
{
  std::string efp = this->GetExecutionFilePath();
  if (!filename)
    {
    filename = efp.c_str();
    }
  std::vector<cmListFileArgument>::const_iterator i;
  std::string value;
  outArgs.reserve(inArgs.size());
  for(i = inArgs.begin(); i != inArgs.end(); ++i)
    {
    // No expansion in a bracket argument.
    if(i->Delim == cmListFileArgument::Bracket)
      {
      outArgs.push_back(i->Value);
      continue;
      }
    // Expand the variables in the argument.
    value = i->Value;
    this->ExpandVariablesInString(value, false, false, false,
                                  filename, i->Line, false, false);

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
bool cmMakefile::ExpandArguments(
  std::vector<cmListFileArgument> const& inArgs,
  std::vector<cmExpandedCommandArgument>& outArgs, const char* filename) const
{
  std::string efp = this->GetExecutionFilePath();
  if (!filename)
    {
    filename = efp.c_str();
    }
  std::vector<cmListFileArgument>::const_iterator i;
  std::string value;
  outArgs.reserve(inArgs.size());
  for(i = inArgs.begin(); i != inArgs.end(); ++i)
    {
    // No expansion in a bracket argument.
    if(i->Delim == cmListFileArgument::Bracket)
      {
      outArgs.push_back(cmExpandedCommandArgument(i->Value, true));
      continue;
      }
    // Expand the variables in the argument.
    value = i->Value;
    this->ExpandVariablesInString(value, false, false, false,
                                  filename, i->Line, false, false);

    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if(i->Delim == cmListFileArgument::Quoted)
      {
      outArgs.push_back(cmExpandedCommandArgument(value, true));
      }
    else
      {
      std::vector<std::string> stringArgs;
      cmSystemTools::ExpandListArgument(value, stringArgs);
      for(size_t j = 0; j < stringArgs.size(); ++j)
        {
        outArgs.push_back(cmExpandedCommandArgument(stringArgs[j], false));
        }
      }
    }
  return !cmSystemTools::GetFatalErrorOccured();
}

//----------------------------------------------------------------------------
void cmMakefile::AddFunctionBlocker(cmFunctionBlocker* fb)
{
  if(!this->ExecutionStatusStack.empty())
    {
    // Record the context in which the blocker is created.
    fb->SetStartingContext(this->GetExecutionContext());
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
        cmListFileContext closingContext =
            cmListFileContext::FromCommandContext(lff, lfc.FilePath);
        std::ostringstream e;
        e << "A logical block opening on the line\n"
          << "  " << lfc << "\n"
          << "closes on the line\n"
          << "  " << closingContext << "\n"
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

const char* cmMakefile::GetHomeDirectory() const
{
  return this->GetCMakeInstance()->GetHomeDirectory();
}

const char* cmMakefile::GetHomeOutputDirectory() const
{
  return this->GetCMakeInstance()->GetHomeOutputDirectory();
}

void cmMakefile::SetScriptModeFile(const char* scriptfile)
{
  this->AddDefinition("CMAKE_SCRIPT_MODE_FILE", scriptfile);
}

void cmMakefile::SetArgcArgv(const std::vector<std::string>& args)
{
  std::ostringstream strStream;
  strStream << args.size();
  this->AddDefinition("CMAKE_ARGC", strStream.str().c_str());
  //this->MarkVariableAsUsed("CMAKE_ARGC");

  for (unsigned int t = 0; t < args.size(); ++t)
  {
    std::ostringstream tmpStream;
    tmpStream << "CMAKE_ARGV" << t;
    this->AddDefinition(tmpStream.str(), args[t].c_str());
    //this->MarkVariableAsUsed(tmpStream.str().c_str());
  }
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetSource(const std::string& sourceName) const
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
cmSourceFile* cmMakefile::CreateSource(const std::string& sourceName,
                                       bool generated)
{
  cmSourceFile* sf = new cmSourceFile(this, sourceName);
  if(generated)
    {
    sf->SetProperty("GENERATED", "1");
    }
  this->SourceFiles.push_back(sf);
  return sf;
}

//----------------------------------------------------------------------------
cmSourceFile* cmMakefile::GetOrCreateSource(const std::string& sourceName,
                                            bool generated)
{
  if(cmSourceFile* esf = this->GetSource(sourceName))
    {
    return esf;
    }
  else
    {
    return this->CreateSource(sourceName, generated);
    }
}

void cmMakefile::EnableLanguage(std::vector<std::string> const &  lang,
                               bool optional)
{
  this->AddDefinition("CMAKE_CFG_INTDIR",
                      this->GetGlobalGenerator()->GetCMakeCFGIntDir());
  this->GetGlobalGenerator()->EnableLanguage(lang, this, optional);
}

int cmMakefile::TryCompile(const std::string& srcdir,
                           const std::string& bindir,
                           const std::string& projectName,
                           const std::string& targetName,
                           bool fast,
                           const std::vector<std::string> *cmakeArgs,
                           std::string& output)
{
  this->IsSourceFileTryCompile = fast;
  // does the binary directory exist ? If not create it...
  if (!cmSystemTools::FileIsDirectory(bindir))
    {
    cmSystemTools::MakeDirectory(bindir.c_str());
    }

  // change to the tests directory and run cmake
  // use the cmake object instead of calling cmake
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  // make sure the same generator is used
  // use this program as the cmake to be run, it should not
  // be run that way but the cmake object requires a vailid path
  cmake cm;
  cm.SetIsInTryCompile(true);
  cmGlobalGenerator *gg = cm.CreateGlobalGenerator
    (this->GetGlobalGenerator()->GetName());
  if (!gg)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile bad GlobalGenerator");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd);
    this->IsSourceFileTryCompile = false;
    return 1;
    }
  cm.SetGlobalGenerator(gg);

  // do a configure
  cm.SetHomeDirectory(srcdir);
  cm.SetHomeOutputDirectory(bindir);
  cm.SetGeneratorPlatform(this->GetCMakeInstance()->GetGeneratorPlatform());
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
                       "Build configuration", cmState::STRING);
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
  gg->EnableLanguagesFromGenerator(this->GetGlobalGenerator(), this);
  if(this->IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS"))
    {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS",
                     "TRUE", "", cmState::INTERNAL);
    }
  else
    {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS",
                     "FALSE", "", cmState::INTERNAL);
    }
  if (cm.Configure() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile configure of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd);
    this->IsSourceFileTryCompile = false;
    return 1;
    }

  if (cm.Generate() != 0)
    {
    cmSystemTools::Error(
      "Internal CMake error, TryCompile generation of cmake failed");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd);
    this->IsSourceFileTryCompile = false;
    return 1;
    }

  // finally call the generator to actually build the resulting project
  int ret = this->GetGlobalGenerator()->TryCompile(srcdir,bindir,
                                                   projectName,
                                                   targetName,
                                                   fast,
                                                   output,
                                                   this);

  cmSystemTools::ChangeDirectory(cwd);
  this->IsSourceFileTryCompile = false;
  return ret;
}

bool cmMakefile::GetIsSourceFileTryCompile() const
{
  return this->IsSourceFileTryCompile;
}

cmake *cmMakefile::GetCMakeInstance() const
{
  return this->GlobalGenerator->GetCMakeInstance();
}

cmGlobalGenerator* cmMakefile::GetGlobalGenerator() const
{
  return this->GlobalGenerator;
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

cmState *cmMakefile::GetState() const
{
  return this->GetCMakeInstance()->GetState();
}

void cmMakefile::DisplayStatus(const char* message, float s) const
{
  cmake* cm = this->GetCMakeInstance();
  if (cm->GetWorkingMode() == cmake::FIND_PACKAGE_MODE)
    {
    // don't output any STATUS message in FIND_PACKAGE_MODE, since they will
    // directly be fed to the compiler, which will be confused.
    return;
    }
  cm->UpdateProgress(message, s);
}

std::string cmMakefile::GetModulesFile(const char* filename) const
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
  if (result.empty())
    {
    result = moduleInCMakeRoot;
    }

  if (!moduleInCMakeModulePath.empty() && !moduleInCMakeRoot.empty())
    {
    const char* currentFile = this->GetDefinition("CMAKE_CURRENT_LIST_FILE");
    std::string mods = cmakeRoot + std::string("/Modules/");
    if (currentFile && strncmp(currentFile, mods.c_str(), mods.size()) == 0)
      {
      switch (this->GetPolicyStatus(cmPolicies::CMP0017))
        {
        case cmPolicies::WARN:
        {
          std::ostringstream e;
          e << "File " << currentFile << " includes "
            << moduleInCMakeModulePath
            << " (found via CMAKE_MODULE_PATH) which shadows "
            << moduleInCMakeRoot  << ". This may cause errors later on .\n"
            << cmPolicies::GetPolicyWarning(cmPolicies::CMP0017);

          this->IssueMessage(cmake::AUTHOR_WARNING, e.str());
           // break;  // fall through to OLD behaviour
        }
        case cmPolicies::OLD:
          result = moduleInCMakeModulePath;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          result = moduleInCMakeRoot;
          break;
        }
      }
    }

  return result;
}

void cmMakefile::ConfigureString(const std::string& input,
                                 std::string& output, bool atOnly,
                                 bool escapeQuotes) const
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
        this->GetDefinition(this->cmDefineRegex.match(1));
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
        this->GetDefinition(this->cmDefine01Regex.match(1));
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
                                atOnly, 0, -1, true, true);
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
    cmsys::ofstream fout(tempOutputFile.c_str(), omode);
    if(!fout)
      {
      cmSystemTools::Error(
        "Could not open file for write in copy operation ",
        tempOutputFile.c_str());
      cmSystemTools::ReportLastSystemError("");
      return 0;
      }
    cmsys::ifstream fin(sinfile.c_str());
    if(!fin)
      {
      cmSystemTools::Error("Could not open file for read in copy operation ",
                           sinfile.c_str());
      return 0;
      }

    cmsys::FStream::BOM bom = cmsys::FStream::ReadBOM(fin);
    if(bom != cmsys::FStream::BOM_None &&
       bom != cmsys::FStream::BOM_UTF8)
      {
      std::ostringstream e;
      e << "File starts with a Byte-Order-Mark that is not UTF-8:\n  "
        << sinfile;
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return 0;
      }
    // rewind to copy BOM to output file
    fin.seekg(0);


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
    cmSystemTools::RemoveFile(tempOutputFile);
    }
  return res;
}

void cmMakefile::SetProperty(const std::string& prop, const char* value)
{
  cmListFileBacktrace lfbt = this->GetBacktrace();
  this->StateSnapshot.GetDirectory().SetProperty(prop, value, lfbt);
}

void cmMakefile::AppendProperty(const std::string& prop,
                                const char* value,
                                bool asString)
{
  cmListFileBacktrace lfbt = this->GetBacktrace();
  this->StateSnapshot.GetDirectory().AppendProperty(prop, value,
                                                    asString, lfbt);
}

const char *cmMakefile::GetProperty(const std::string& prop) const
{
  return this->StateSnapshot.GetDirectory().GetProperty(prop);
}

const char *cmMakefile::GetProperty(const std::string& prop,
                                    bool chain) const
{
  return this->StateSnapshot.GetDirectory().GetProperty(prop, chain);
}

bool cmMakefile::GetPropertyAsBool(const std::string& prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

std::vector<std::string> cmMakefile::GetPropertyKeys() const
{
  return this->StateSnapshot.GetDirectory().GetPropertyKeys();
}

cmTarget* cmMakefile::FindTarget(const std::string& name,
                                 bool excludeAliases) const
{
  if (!excludeAliases)
    {
    TargetMap::const_iterator i = this->AliasTargets.find(name);
    if (i != this->AliasTargets.end())
      {
      return i->second;
      }
    }
  cmTargets::iterator i = this->Targets.find( name );
  if ( i != this->Targets.end() )
    {
    return &i->second;
    }

  return 0;
}

//----------------------------------------------------------------------------
cmTest* cmMakefile::CreateTest(const std::string& testName)
{
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
cmTest* cmMakefile::GetTest(const std::string& testName) const
{
  std::map<std::string, cmTest*>::const_iterator
    mi = this->Tests.find(testName);
  if(mi != this->Tests.end())
    {
    return mi->second;
    }
  return 0;
}

void cmMakefile::AddCMakeDependFilesFromUser()
{
  std::vector<std::string> deps;
  if(const char* deps_str = this->GetProperty("CMAKE_CONFIGURE_DEPENDS"))
    {
    cmSystemTools::ExpandListArgument(deps_str, deps);
    }
  for(std::vector<std::string>::iterator i = deps.begin();
      i != deps.end(); ++i)
    {
    if(cmSystemTools::FileIsFullPath(i->c_str()))
      {
      this->AddCMakeDependFile(*i);
      }
    else
      {
      std::string f = this->GetCurrentSourceDirectory();
      f += "/";
      f += *i;
      this->AddCMakeDependFile(f);
      }
    }
}

std::string cmMakefile::FormatListFileStack() const
{
  std::vector<std::string> listFiles;
  cmState::Snapshot snp = this->StateSnapshot;
  while (snp.IsValid())
    {
    listFiles.push_back(snp.GetExecutionListFile());
    snp = snp.GetCallStackParent();
    }
  std::reverse(listFiles.begin(), listFiles.end());
  std::ostringstream tmp;
  size_t depth = listFiles.size();
  if (depth > 0)
    {
    std::vector<std::string>::const_iterator it = listFiles.end();
    do
      {
      if (depth != listFiles.size())
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
    while (it != listFiles.begin());
    }
  return tmp.str();
}


void cmMakefile::PushScope()
{
  std::string commandName;
  long line = 0;
  if (!this->ContextStack.empty())
    {
    commandName = this->ContextStack.back()->Name;
    line = this->ContextStack.back()->Line;
    }
  this->StateSnapshot = this->GetState()->CreateVariableScopeSnapshot(
        this->StateSnapshot,
        commandName,
        line);
  this->PushLoopBlockBarrier();

#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->GetGlobalGenerator()->GetFileLockPool().PushFunctionScope();
#endif
}

void cmMakefile::PopScope()
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  this->GetGlobalGenerator()->GetFileLockPool().PopFunctionScope();
#endif

  this->PopLoopBlockBarrier();

  this->CheckForUnusedVariables();

  this->StateSnapshot =
      this->GetState()->Pop(this->StateSnapshot);
  assert(this->StateSnapshot.IsValid());
}

void cmMakefile::RaiseScope(const std::string& var, const char *varDef)
{
  if (var.empty())
    {
    return;
    }

  if (!this->StateSnapshot.RaiseScope(var, varDef))
    {
    std::ostringstream m;
    m << "Cannot set \"" << var << "\": current scope has no parent.";
    this->IssueMessage(cmake::AUTHOR_WARNING, m.str());
    }
}

//----------------------------------------------------------------------------
cmTarget*
cmMakefile::AddImportedTarget(const std::string& name,
                              cmTarget::TargetType type,
                              bool global)
{
  // Create the target.
  cmsys::auto_ptr<cmTarget> target(new cmTarget);
  target->SetType(type, name);
  target->MarkAsImported();
  target->SetMakefile(this);

  // Add to the set of available imported targets.
  this->ImportedTargets[name] = target.get();
  if(global)
    {
    this->GetGlobalGenerator()->AddTarget(target.get());
    }

  // Transfer ownership to this cmMakefile object.
  this->ImportedTargetsOwned.push_back(target.get());
  return target.release();
}

//----------------------------------------------------------------------------
cmTarget* cmMakefile::FindTargetToUse(const std::string& name,
                                      bool excludeAliases) const
{
  // Look for an imported target.  These take priority because they
  // are more local in scope and do not have to be globally unique.
  TargetMap::const_iterator
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
  return this->GetGlobalGenerator()->FindTarget(name, excludeAliases);
}

//----------------------------------------------------------------------------
bool cmMakefile::IsAlias(const std::string& name) const
{
  if (this->AliasTargets.find(name) != this->AliasTargets.end())
    return true;
  return this->GetGlobalGenerator()->IsAlias(name);
}

//----------------------------------------------------------------------------
cmGeneratorTarget*
cmMakefile::FindGeneratorTargetToUse(const std::string& name) const
{
  if (cmTarget *t = this->FindTargetToUse(name))
    {
    return this->GetGlobalGenerator()->GetGeneratorTarget(t);
    }
  return 0;
}

//----------------------------------------------------------------------------
bool cmMakefile::EnforceUniqueName(std::string const& name, std::string& msg,
                                   bool isCustom) const
{
  if(this->IsAlias(name))
    {
    std::ostringstream e;
    e << "cannot create target \"" << name
      << "\" because an alias with the same name already exists.";
    msg = e.str();
    return false;
    }
  if(cmTarget* existing = this->FindTargetToUse(name))
    {
    // The name given conflicts with an existing target.  Produce an
    // error in a compatible way.
    if(existing->IsImported())
      {
      // Imported targets were not supported in previous versions.
      // This is new code, so we can make it an error.
      std::ostringstream e;
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
          this->IssueMessage(cmake::AUTHOR_WARNING, cmPolicies::
                             GetPolicyWarning(cmPolicies::CMP0002));
        case cmPolicies::OLD:
          return true;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          this->IssueMessage(cmake::FATAL_ERROR,
            cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0002)
            );
          return true;
        case cmPolicies::NEW:
          break;
        }

      // The conflict is with a non-imported target.
      // Allow this if the user has requested support.
      cmake* cm = this->GetCMakeInstance();
      if(isCustom && existing->GetType() == cmTarget::UTILITY &&
         this != existing->GetMakefile() &&
         cm->GetState()
           ->GetGlobalPropertyAsBool("ALLOW_DUPLICATE_CUSTOM_TARGETS"))
        {
        return true;
        }

      // Produce an error that tells the user how to work around the
      // problem.
      std::ostringstream e;
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
        case cmTarget::INTERFACE_LIBRARY:
          e << "an interface library ";
          break;
        default: break;
        }
      e << "created in source directory \""
        << existing->GetMakefile()->GetCurrentSourceDirectory() << "\".  "
        << "See documentation for policy CMP0002 for more details.";
      msg = e.str();
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmMakefile::EnforceUniqueDir(const std::string& srcPath,
                                  const std::string& binPath) const
{
  // Make sure the binary directory is unique.
  cmGlobalGenerator* gg = this->GetGlobalGenerator();
  if(gg->BinaryDirectoryIsNew(binPath))
    {
    return true;
    }
  std::ostringstream e;
  switch (this->GetPolicyStatus(cmPolicies::CMP0013))
    {
    case cmPolicies::WARN:
      // Print the warning.
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0013)
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
      e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0013)
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
void cmMakefile::AddQtUiFileWithOptions(cmSourceFile *sf)
{
  this->QtUiFilesWithOptions.push_back(sf);
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> cmMakefile::GetQtUiFilesWithOptions() const
{
  return this->QtUiFilesWithOptions;
}

static std::string const matchVariables[] = {
  "CMAKE_MATCH_0",
  "CMAKE_MATCH_1",
  "CMAKE_MATCH_2",
  "CMAKE_MATCH_3",
  "CMAKE_MATCH_4",
  "CMAKE_MATCH_5",
  "CMAKE_MATCH_6",
  "CMAKE_MATCH_7",
  "CMAKE_MATCH_8",
  "CMAKE_MATCH_9"
};

static std::string const nMatchesVariable = "CMAKE_MATCH_COUNT";

//----------------------------------------------------------------------------
void cmMakefile::ClearMatches()
{
  const char* nMatchesStr = this->GetDefinition(nMatchesVariable);
  if (!nMatchesStr)
    {
    return;
    }
  int nMatches = atoi(nMatchesStr);
  for (int i=0; i<=nMatches; i++)
    {
    std::string const& var = matchVariables[i];
    std::string const& s = this->GetSafeDefinition(var);
    if(!s.empty())
      {
      this->AddDefinition(var, "");
      this->MarkVariableAsUsed(var);
      }
    }
  this->AddDefinition(nMatchesVariable, "0");
  this->MarkVariableAsUsed(nMatchesVariable);
}

//----------------------------------------------------------------------------
void cmMakefile::StoreMatches(cmsys::RegularExpression& re)
{
  char highest = 0;
  for (int i=0; i<10; i++)
    {
    std::string const& m = re.match(i);
    if(!m.empty())
      {
      std::string const& var = matchVariables[i];
      this->AddDefinition(var, m.c_str());
      this->MarkVariableAsUsed(var);
      highest = static_cast<char>('0' + i);
      }
    }
  char nMatches[] = {highest, '\0'};
  this->AddDefinition(nMatchesVariable, nMatches);
  this->MarkVariableAsUsed(nMatchesVariable);
}

cmState::Snapshot cmMakefile::GetStateSnapshot() const
{
  return this->StateSnapshot;
}

const char* cmMakefile::GetDefineFlagsCMP0059() const
{
  return this->DefineFlagsOrig.c_str();
}

//----------------------------------------------------------------------------
cmPolicies::PolicyStatus
cmMakefile::GetPolicyStatus(cmPolicies::PolicyID id) const
{
  return this->StateSnapshot.GetPolicy(id);
}

//----------------------------------------------------------------------------
bool cmMakefile::PolicyOptionalWarningEnabled(std::string const& var)
{
  // Check for an explicit CMAKE_POLICY_WARNING_CMP<NNNN> setting.
  if(!var.empty())
    {
    if(const char* val = this->GetDefinition(var))
      {
      return cmSystemTools::IsOn(val);
      }
    }
  // Enable optional policy warnings with --debug-output, --trace,
  // or --trace-expand.
  cmake* cm = this->GetCMakeInstance();
  return cm->GetDebugOutput() || cm->GetTrace();
}

bool cmMakefile::SetPolicy(const char *id,
                           cmPolicies::PolicyStatus status)
{
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id, /* out */ pid))
    {
    std::ostringstream e;
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
     cmPolicies::GetPolicyStatus(id) ==
     cmPolicies::REQUIRED_ALWAYS)
    {
    std::string msg =
      cmPolicies::GetRequiredAlwaysPolicyError(id);
    this->IssueMessage(cmake::FATAL_ERROR, msg);
    return false;
    }

  this->StateSnapshot.SetPolicy(id, status);
  return true;
}

//----------------------------------------------------------------------------
cmMakefile::PolicyPushPop::PolicyPushPop(cmMakefile* m, bool weak,
                                         cmPolicies::PolicyMap const& pm):
  Makefile(m), ReportError(true)
{
  this->Makefile->StateSnapshot = this->Makefile->StateSnapshot.GetState()
      ->CreatePolicyScopeSnapshot(this->Makefile->StateSnapshot);
  this->Makefile->PushPolicy(weak, pm);
}

//----------------------------------------------------------------------------
cmMakefile::PolicyPushPop::~PolicyPushPop()
{
  this->Makefile->PopPolicy();
  this->Makefile->PopPolicyBarrier(this->ReportError);
}

//----------------------------------------------------------------------------
void cmMakefile::PushPolicy(bool weak, cmPolicies::PolicyMap const& pm)
{
  this->StateSnapshot.PushPolicy(pm, weak);
}

//----------------------------------------------------------------------------
void cmMakefile::PopPolicy()
{
  if (!this->StateSnapshot.PopPolicy())
    {
    this->IssueMessage(cmake::FATAL_ERROR,
                       "cmake_policy POP without matching PUSH");
    }
}

//----------------------------------------------------------------------------
void cmMakefile::PopPolicyBarrier(bool reportError)
{
  while (!this->StateSnapshot.CanPopPolicyScope())
    {
    if(reportError)
      {
      this->IssueMessage(cmake::FATAL_ERROR,
                         "cmake_policy PUSH without matching POP");
      reportError = false;
      }
    this->PopPolicy();
    }

  this->StateSnapshot = this->GetState()->Pop(this->StateSnapshot);
  assert(this->StateSnapshot.IsValid());
}

//----------------------------------------------------------------------------
bool cmMakefile::SetPolicyVersion(const char *version)
{
  return cmPolicies::ApplyPolicyVersion(this,version);
}

//----------------------------------------------------------------------------
bool cmMakefile::HasCMP0054AlreadyBeenReported() const
{
  return !this->CMP0054ReportedIds.insert(this->GetExecutionContext()).second;
}

//----------------------------------------------------------------------------
void cmMakefile::RecordPolicies(cmPolicies::PolicyMap& pm)
{
  /* Record the setting of every policy.  */
  typedef cmPolicies::PolicyID PolicyID;
  for(PolicyID pid = cmPolicies::CMP0000;
      pid != cmPolicies::CMPCOUNT; pid = PolicyID(pid+1))
    {
    pm.Set(pid, this->GetPolicyStatus(pid));
    }
}

//----------------------------------------------------------------------------
bool cmMakefile::IgnoreErrorsCMP0061() const
{
  bool ignoreErrors = true;
  switch (this->GetPolicyStatus(cmPolicies::CMP0061))
    {
    case cmPolicies::WARN:
      // No warning for this policy!
    case cmPolicies::OLD:
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      ignoreErrors = false;
      break;
    }
  return ignoreErrors;
}

//----------------------------------------------------------------------------
#define FEATURE_STRING(F) , #F
static const char * const C_FEATURES[] = {
  0
  FOR_EACH_C_FEATURE(FEATURE_STRING)
};

static const char * const CXX_FEATURES[] = {
  0
  FOR_EACH_CXX_FEATURE(FEATURE_STRING)
};
#undef FEATURE_STRING

static const char * const C_STANDARDS[] = {
    "90"
  , "99"
  , "11"
};
static const char * const CXX_STANDARDS[] = {
    "98"
  , "11"
  , "14"
};

//----------------------------------------------------------------------------
bool cmMakefile::
AddRequiredTargetFeature(cmTarget *target, const std::string& feature,
                         std::string *error) const
{
  if (cmGeneratorExpression::Find(feature) != std::string::npos)
    {
    target->AppendProperty("COMPILE_FEATURES", feature.c_str());
    return true;
    }

  std::string lang;
  if (!this->CompileFeatureKnown(target, feature, lang, error))
    {
    return false;
    }

  const char* features = this->CompileFeaturesAvailable(lang, error);
  if (!features)
    {
    return false;
    }

  std::vector<std::string> availableFeatures;
  cmSystemTools::ExpandListArgument(features, availableFeatures);
  if (std::find(availableFeatures.begin(),
                availableFeatures.end(),
                feature) == availableFeatures.end())
    {
    std::ostringstream e;
    e << "The compiler feature \"" << feature
      << "\" is not known to " << lang << " compiler\n\""
      << this->GetDefinition("CMAKE_" + lang + "_COMPILER_ID")
      << "\"\nversion "
      << this->GetDefinition("CMAKE_" + lang + "_COMPILER_VERSION") << ".";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  target->AppendProperty("COMPILE_FEATURES", feature.c_str());

  return lang == "C"
      ? this->AddRequiredTargetCFeature(target, feature)
      : this->AddRequiredTargetCxxFeature(target, feature);
}

//----------------------------------------------------------------------------
bool cmMakefile::
CompileFeatureKnown(cmTarget const* target, const std::string& feature,
                    std::string& lang, std::string *error) const
{
  assert(cmGeneratorExpression::Find(feature) == std::string::npos);

  bool isCFeature = std::find_if(cmArrayBegin(C_FEATURES) + 1,
              cmArrayEnd(C_FEATURES), cmStrCmp(feature))
              != cmArrayEnd(C_FEATURES);
  if (isCFeature)
    {
    lang = "C";
    return true;
    }
  bool isCxxFeature = std::find_if(cmArrayBegin(CXX_FEATURES) + 1,
              cmArrayEnd(CXX_FEATURES), cmStrCmp(feature))
              != cmArrayEnd(CXX_FEATURES);
  if (isCxxFeature)
    {
    lang = "CXX";
    return true;
    }
  std::ostringstream e;
  if (error)
    {
    e << "specified";
    }
  else
    {
    e << "Specified";
    }
  e << " unknown feature \"" << feature << "\" for "
    "target \"" << target->GetName() << "\".";
  if (error)
    {
    *error = e.str();
    }
  else
    {
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
  return false;
}

//----------------------------------------------------------------------------
const char* cmMakefile::
CompileFeaturesAvailable(const std::string& lang, std::string *error) const
{
  const char* featuresKnown =
    this->GetDefinition("CMAKE_" + lang + "_COMPILE_FEATURES");

  if (!featuresKnown || !*featuresKnown)
    {
    std::ostringstream e;
    if (error)
      {
      e << "no";
      }
    else
      {
      e << "No";
      }
    e << " known features for " << lang << " compiler\n\""
      << this->GetDefinition("CMAKE_" + lang + "_COMPILER_ID")
      << "\"\nversion "
      << this->GetDefinition("CMAKE_" + lang + "_COMPILER_VERSION") << ".";
    if (error)
      {
      *error = e.str();
      }
    else
      {
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      }
    return 0;
    }
  return featuresKnown;
}

//----------------------------------------------------------------------------
bool cmMakefile::HaveStandardAvailable(cmTarget const* target,
                                      std::string const& lang,
                                      const std::string& feature) const
{
  return lang == "C"
      ? this->HaveCStandardAvailable(target, feature)
      : this->HaveCxxStandardAvailable(target, feature);
}

//----------------------------------------------------------------------------
bool cmMakefile::
HaveCStandardAvailable(cmTarget const* target,
                       const std::string& feature) const
{
  const char* defaultCStandard =
    this->GetDefinition("CMAKE_C_STANDARD_DEFAULT");
  if (!defaultCStandard)
    {
    std::ostringstream e;
    e << "CMAKE_C_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
      "not fully configured for this compiler.";
    this->IssueMessage(cmake::INTERNAL_ERROR, e.str());
    // Return true so the caller does not try to lookup the default standard.
    return true;
    }
  if (std::find_if(cmArrayBegin(C_STANDARDS), cmArrayEnd(C_STANDARDS),
                cmStrCmp(defaultCStandard)) == cmArrayEnd(C_STANDARDS))
    {
    std::ostringstream e;
    e << "The CMAKE_C_STANDARD_DEFAULT variable contains an "
         "invalid value: \"" << defaultCStandard << "\".";
    this->IssueMessage(cmake::INTERNAL_ERROR, e.str());
    return false;
    }

  bool needC90 = false;
  bool needC99 = false;
  bool needC11 = false;

  this->CheckNeededCLanguage(feature, needC90, needC99, needC11);

  const char *existingCStandard = target->GetProperty("C_STANDARD");
  if (!existingCStandard)
    {
    existingCStandard = defaultCStandard;
    }

  if (std::find_if(cmArrayBegin(C_STANDARDS), cmArrayEnd(C_STANDARDS),
                cmStrCmp(existingCStandard)) == cmArrayEnd(C_STANDARDS))
    {
    std::ostringstream e;
    e << "The C_STANDARD property on target \"" << target->GetName()
      << "\" contained an invalid value: \"" << existingCStandard << "\".";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  const char * const *existingCIt = existingCStandard
                                    ? std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp(existingCStandard))
                                    : cmArrayEnd(C_STANDARDS);

  if (needC11 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("11")))
    {
    return false;
    }
  else if(needC99 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("99")))
    {
    return false;
    }
  else if(needC90 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("90")))
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmMakefile::IsLaterStandard(std::string const& lang,
                                 std::string const& lhs,
                                 std::string const& rhs)
{
  if (lang == "C")
    {
    const char * const *rhsIt = std::find_if(cmArrayBegin(C_STANDARDS),
                                            cmArrayEnd(C_STANDARDS),
                                            cmStrCmp(rhs));

    return std::find_if(rhsIt, cmArrayEnd(C_STANDARDS),
                        cmStrCmp(lhs)) != cmArrayEnd(C_STANDARDS);
    }
  const char * const *rhsIt = std::find_if(cmArrayBegin(CXX_STANDARDS),
                                           cmArrayEnd(CXX_STANDARDS),
                                           cmStrCmp(rhs));

  return std::find_if(rhsIt, cmArrayEnd(CXX_STANDARDS),
                      cmStrCmp(lhs)) != cmArrayEnd(CXX_STANDARDS);
}

//----------------------------------------------------------------------------
bool cmMakefile::HaveCxxStandardAvailable(cmTarget const* target,
                                         const std::string& feature) const
{
  const char* defaultCxxStandard =
    this->GetDefinition("CMAKE_CXX_STANDARD_DEFAULT");
  if (!defaultCxxStandard)
    {
    std::ostringstream e;
    e << "CMAKE_CXX_STANDARD_DEFAULT is not set.  COMPILE_FEATURES support "
      "not fully configured for this compiler.";
    this->IssueMessage(cmake::INTERNAL_ERROR, e.str());
    // Return true so the caller does not try to lookup the default standard.
    return true;
    }
  if (std::find_if(cmArrayBegin(CXX_STANDARDS), cmArrayEnd(CXX_STANDARDS),
                cmStrCmp(defaultCxxStandard)) == cmArrayEnd(CXX_STANDARDS))
    {
    std::ostringstream e;
    e << "The CMAKE_CXX_STANDARD_DEFAULT variable contains an "
         "invalid value: \"" << defaultCxxStandard << "\".";
    this->IssueMessage(cmake::INTERNAL_ERROR, e.str());
    return false;
    }

  bool needCxx98 = false;
  bool needCxx11 = false;
  bool needCxx14 = false;
  this->CheckNeededCxxLanguage(feature, needCxx98, needCxx11, needCxx14);

  const char *existingCxxStandard = target->GetProperty("CXX_STANDARD");
  if (!existingCxxStandard)
    {
    existingCxxStandard = defaultCxxStandard;
    }

  if (std::find_if(cmArrayBegin(CXX_STANDARDS), cmArrayEnd(CXX_STANDARDS),
                cmStrCmp(existingCxxStandard)) == cmArrayEnd(CXX_STANDARDS))
    {
    std::ostringstream e;
    e << "The CXX_STANDARD property on target \"" << target->GetName()
      << "\" contained an invalid value: \"" << existingCxxStandard << "\".";
    this->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  const char * const *existingCxxIt = existingCxxStandard
                                    ? std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp(existingCxxStandard))
                                    : cmArrayEnd(CXX_STANDARDS);

  if (needCxx11 && existingCxxIt < std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp("11")))
    {
    return false;
    }
  else if(needCxx98 && existingCxxIt <
                                    std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp("98")))
    {
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
void cmMakefile::CheckNeededCxxLanguage(const std::string& feature,
                                        bool& needCxx98,
                                        bool& needCxx11,
                                        bool& needCxx14) const
{
  if (const char *propCxx98 =
          this->GetDefinition("CMAKE_CXX98_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propCxx98, props);
    needCxx98 = std::find(props.begin(), props.end(), feature) != props.end();
    }
  if (const char *propCxx11 =
          this->GetDefinition("CMAKE_CXX11_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propCxx11, props);
    needCxx11 = std::find(props.begin(), props.end(), feature) != props.end();
    }
  if (const char *propCxx14 =
          this->GetDefinition("CMAKE_CXX14_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propCxx14, props);
    needCxx14 = std::find(props.begin(), props.end(), feature) != props.end();
    }
}

//----------------------------------------------------------------------------
bool cmMakefile::
AddRequiredTargetCxxFeature(cmTarget *target,
                            const std::string& feature) const
{
  bool needCxx98 = false;
  bool needCxx11 = false;
  bool needCxx14 = false;

  this->CheckNeededCxxLanguage(feature, needCxx98, needCxx11, needCxx14);

  const char *existingCxxStandard = target->GetProperty("CXX_STANDARD");
  if (existingCxxStandard)
    {
    if (std::find_if(cmArrayBegin(CXX_STANDARDS), cmArrayEnd(CXX_STANDARDS),
                  cmStrCmp(existingCxxStandard)) == cmArrayEnd(CXX_STANDARDS))
      {
      std::ostringstream e;
      e << "The CXX_STANDARD property on target \"" << target->GetName()
        << "\" contained an invalid value: \"" << existingCxxStandard << "\".";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return false;
      }
    }
  const char * const *existingCxxIt = existingCxxStandard
                                    ? std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp(existingCxxStandard))
                                    : cmArrayEnd(CXX_STANDARDS);

  bool setCxx98 = needCxx98 && !existingCxxStandard;
  bool setCxx11 = needCxx11 && !existingCxxStandard;
  bool setCxx14 = needCxx14 && !existingCxxStandard;

  if (needCxx14 && existingCxxStandard && existingCxxIt <
                                    std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp("14")))
    {
    setCxx14 = true;
    }
  else if (needCxx11 && existingCxxStandard && existingCxxIt <
                                    std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp("11")))
    {
    setCxx11 = true;
    }
  else if(needCxx98 && existingCxxStandard && existingCxxIt <
                                    std::find_if(cmArrayBegin(CXX_STANDARDS),
                                      cmArrayEnd(CXX_STANDARDS),
                                      cmStrCmp("98")))
    {
    setCxx98 = true;
    }

  if (setCxx14)
    {
    target->SetProperty("CXX_STANDARD", "14");
    }
  else if (setCxx11)
    {
    target->SetProperty("CXX_STANDARD", "11");
    }
  else if (setCxx98)
    {
    target->SetProperty("CXX_STANDARD", "98");
    }
  return true;
}

//----------------------------------------------------------------------------
void cmMakefile::CheckNeededCLanguage(const std::string& feature,
                                        bool& needC90,
                                        bool& needC99,
                                        bool& needC11) const
{
  if (const char *propC90 =
          this->GetDefinition("CMAKE_C90_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propC90, props);
    needC90 = std::find(props.begin(), props.end(), feature) != props.end();
    }
  if (const char *propC99 =
          this->GetDefinition("CMAKE_C99_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propC99, props);
    needC99 = std::find(props.begin(), props.end(), feature) != props.end();
    }
  if (const char *propC11 =
          this->GetDefinition("CMAKE_C11_COMPILE_FEATURES"))
    {
    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(propC11, props);
    needC11 = std::find(props.begin(), props.end(), feature) != props.end();
    }
}

//----------------------------------------------------------------------------
bool cmMakefile::
AddRequiredTargetCFeature(cmTarget *target, const std::string& feature) const
{
  bool needC90 = false;
  bool needC99 = false;
  bool needC11 = false;

  this->CheckNeededCLanguage(feature, needC90, needC99, needC11);

  const char *existingCStandard = target->GetProperty("C_STANDARD");
  if (existingCStandard)
    {
    if (std::find_if(cmArrayBegin(C_STANDARDS), cmArrayEnd(C_STANDARDS),
                  cmStrCmp(existingCStandard)) == cmArrayEnd(C_STANDARDS))
      {
      std::ostringstream e;
      e << "The C_STANDARD property on target \"" << target->GetName()
        << "\" contained an invalid value: \"" << existingCStandard << "\".";
      this->IssueMessage(cmake::FATAL_ERROR, e.str());
      return false;
      }
    }
  const char * const *existingCIt = existingCStandard
                                    ? std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp(existingCStandard))
                                    : cmArrayEnd(C_STANDARDS);

  bool setC90 = needC90 && !existingCStandard;
  bool setC99 = needC99 && !existingCStandard;
  bool setC11 = needC11 && !existingCStandard;

  if (needC11 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("11")))
    {
    setC11 = true;
    }
  else if(needC99 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("99")))
    {
    setC99 = true;
    }
  else if(needC90 && existingCStandard && existingCIt <
                                    std::find_if(cmArrayBegin(C_STANDARDS),
                                      cmArrayEnd(C_STANDARDS),
                                      cmStrCmp("90")))
    {
    setC90 = true;
    }

  if (setC11)
    {
    target->SetProperty("C_STANDARD", "11");
    }
  else if (setC99)
    {
    target->SetProperty("C_STANDARD", "99");
    }
  else if (setC90)
    {
    target->SetProperty("C_STANDARD", "90");
    }
  return true;
}


cmMakefile::FunctionPushPop::FunctionPushPop(cmMakefile* mf,
                                             const std::string& fileName,
                                             cmPolicies::PolicyMap const& pm)
  : Makefile(mf), ReportError(true)
{
  this->Makefile->PushFunctionScope(fileName, pm);
}

cmMakefile::FunctionPushPop::~FunctionPushPop()
{
  this->Makefile->PopFunctionScope(this->ReportError);
}


cmMakefile::MacroPushPop::MacroPushPop(cmMakefile* mf,
                                       const std::string& fileName,
                                       const cmPolicies::PolicyMap& pm)
  : Makefile(mf), ReportError(true)
{
  this->Makefile->PushMacroScope(fileName, pm);
}

cmMakefile::MacroPushPop::~MacroPushPop()
{
  this->Makefile->PopMacroScope(this->ReportError);
}

cmMakefileCall::cmMakefileCall(cmMakefile* mf, const cmCommandContext& lfc,
                               cmExecutionStatus& status): Makefile(mf)
{
  this->Makefile->ContextStack.push_back(&lfc);
  this->Makefile->ExecutionStatusStack.push_back(&status);
}

cmMakefileCall::~cmMakefileCall()
{
  this->Makefile->ExecutionStatusStack.pop_back();
  this->Makefile->ContextStack.pop_back();
}
