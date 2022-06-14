/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmConfigure.h" // IWYU pragma: keep

#include "cmMakefile.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <utility>

#include <cm/iterator>
#include <cm/memory>
#include <cm/optional>
#include <cm/type_traits> // IWYU pragma: keep
#include <cm/vector>
#include <cmext/algorithm>
#include <cmext/string_view>

#ifndef CMAKE_BOOTSTRAP
#  include <cm3p/json/value.h>
#  include <cm3p/json/writer.h>
#endif

#include "cmsys/FStream.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmCommandArgumentParserHelper.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmExecutionStatus.h"
#include "cmExpandedCommandArgument.h" // IWYU pragma: keep
#include "cmExportBuildFileGenerator.h"
#include "cmFileLockPool.h"
#include "cmFunctionBlocker.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h" // IWYU pragma: keep
#include "cmInstallSubdirectoryGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetLinkLibraryType.h"
#include "cmTest.h"
#include "cmTestGenerator.h" // IWYU pragma: keep
#include "cmVersion.h"
#include "cmWorkingDirectory.h"
#include "cmake.h"

#ifndef CMAKE_BOOTSTRAP
#  include "cmMakefileProfilingData.h"
#  include "cmVariableWatch.h"
#endif

class cmMessenger;

cmDirectoryId::cmDirectoryId(std::string s)
  : String(std::move(s))
{
}

// default is not to be building executables
cmMakefile::cmMakefile(cmGlobalGenerator* globalGenerator,
                       cmStateSnapshot const& snapshot)
  : GlobalGenerator(globalGenerator)
  , StateSnapshot(snapshot)
{
  this->IsSourceFileTryCompile = false;

  this->CheckSystemVars = this->GetCMakeInstance()->GetCheckSystemVars();

  this->SuppressSideEffects = false;

  // Setup the default include complaint regular expression (match nothing).
  this->ComplainFileRegularExpression = "^$";

  this->DefineFlags = " ";

  this->cmDefineRegex.compile("#([ \t]*)cmakedefine[ \t]+([A-Za-z_0-9]*)");
  this->cmDefine01Regex.compile("#([ \t]*)cmakedefine01[ \t]+([A-Za-z_0-9]*)");
  this->cmAtVarRegex.compile("(@[A-Za-z_0-9/.+-]+@)");
  this->cmNamedCurly.compile("^[A-Za-z0-9/_.+-]+{");

  this->StateSnapshot =
    this->StateSnapshot.GetState()->CreatePolicyScopeSnapshot(
      this->StateSnapshot);
  this->RecursionDepth = 0;

  // Enter a policy level for this directory.
  this->PushPolicy();

  // push empty loop block
  this->PushLoopBlockBarrier();

  // By default the check is not done.  It is enabled by
  // cmListFileCache in the top level if necessary.
  this->CheckCMP0000 = false;

#if !defined(CMAKE_BOOTSTRAP)
  this->AddSourceGroup("", "^.*$");
  this->AddSourceGroup("Source Files", CM_SOURCE_REGEX);
  this->AddSourceGroup("Header Files", CM_HEADER_REGEX);
  this->AddSourceGroup("Precompile Header File", CM_PCH_REGEX);
  this->AddSourceGroup("CMake Rules", "\\.rule$");
  this->AddSourceGroup("Resources", CM_RESOURCE_REGEX);
  this->AddSourceGroup("Object Files", "\\.(lo|o|obj)$");

  this->ObjectLibrariesSourceGroupIndex = this->SourceGroups.size();
  this->SourceGroups.emplace_back("Object Libraries", "^MATCH_NO_SOURCES$");
#endif
}

cmMakefile::~cmMakefile() = default;

cmDirectoryId cmMakefile::GetDirectoryId() const
{
  // Use the instance pointer value to uniquely identify this directory.
  // If we ever need to expose this to CMake language code we should
  // add a read-only property in cmMakefile::GetProperty.
  char buf[32];
  snprintf(buf, sizeof(buf), "(%p)",
           static_cast<void const*>(this)); // cast avoids format warning
  return std::string(buf);
}

void cmMakefile::IssueMessage(MessageType t, std::string const& text) const
{
  if (!this->ExecutionStatusStack.empty()) {
    if ((t == MessageType::FATAL_ERROR) ||
        (t == MessageType::INTERNAL_ERROR)) {
      this->ExecutionStatusStack.back()->SetNestedError();
    }
  }
  this->GetCMakeInstance()->IssueMessage(t, text, this->Backtrace);
}

bool cmMakefile::CheckCMP0037(std::string const& targetName,
                              cmStateEnums::TargetType targetType) const
{
  MessageType messageType = MessageType::AUTHOR_WARNING;
  std::ostringstream e;
  bool issueMessage = false;
  switch (this->GetPolicyStatus(cmPolicies::CMP0037)) {
    case cmPolicies::WARN:
      if (targetType != cmStateEnums::INTERFACE_LIBRARY) {
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0037) << "\n";
        issueMessage = true;
      }
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      break;
    case cmPolicies::NEW:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      issueMessage = true;
      messageType = MessageType::FATAL_ERROR;
      break;
  }
  if (issueMessage) {
    e << "The target name \"" << targetName
      << "\" is reserved or not valid for certain "
         "CMake features, such as generator expressions, and may result "
         "in undefined behavior.";
    this->IssueMessage(messageType, e.str());

    if (messageType == MessageType::FATAL_ERROR) {
      return false;
    }
  }
  return true;
}

void cmMakefile::MaybeWarnCMP0074(std::string const& pkg)
{
  // Warn if a <pkg>_ROOT variable we may use is set.
  std::string const varName = pkg + "_ROOT";
  cmValue var = this->GetDefinition(varName);
  std::string env;
  cmSystemTools::GetEnv(varName, env);

  bool const haveVar = cmNonempty(var);
  bool const haveEnv = !env.empty();
  if ((haveVar || haveEnv) && this->WarnedCMP0074.insert(varName).second) {
    std::ostringstream w;
    w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0074) << "\n";
    if (haveVar) {
      w << "CMake variable " << varName << " is set to:\n"
        << "  " << *var << "\n";
    }
    if (haveEnv) {
      w << "Environment variable " << varName << " is set to:\n"
        << "  " << env << "\n";
    }
    w << "For compatibility, CMake is ignoring the variable.";
    this->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
  }
}

cmBTStringRange cmMakefile::GetIncludeDirectoriesEntries() const
{
  return this->StateSnapshot.GetDirectory().GetIncludeDirectoriesEntries();
}

cmBTStringRange cmMakefile::GetCompileOptionsEntries() const
{
  return this->StateSnapshot.GetDirectory().GetCompileOptionsEntries();
}

cmBTStringRange cmMakefile::GetCompileDefinitionsEntries() const
{
  return this->StateSnapshot.GetDirectory().GetCompileDefinitionsEntries();
}

cmBTStringRange cmMakefile::GetLinkOptionsEntries() const
{
  return this->StateSnapshot.GetDirectory().GetLinkOptionsEntries();
}

cmBTStringRange cmMakefile::GetLinkDirectoriesEntries() const
{
  return this->StateSnapshot.GetDirectory().GetLinkDirectoriesEntries();
}

cmListFileBacktrace cmMakefile::GetBacktrace() const
{
  return this->Backtrace;
}

void cmMakefile::PrintCommandTrace(cmListFileFunction const& lff,
                                   cmListFileBacktrace const& bt,
                                   CommandMissingFromStack missing) const
{
  // Check if current file in the list of requested to trace...
  std::vector<std::string> const& trace_only_this_files =
    this->GetCMakeInstance()->GetTraceSources();
  std::string const& full_path = bt.Top().FilePath;
  std::string const& only_filename = cmSystemTools::GetFilenameName(full_path);
  bool trace = trace_only_this_files.empty();
  if (!trace) {
    for (std::string const& file : trace_only_this_files) {
      std::string::size_type const pos = full_path.rfind(file);
      trace = (pos != std::string::npos) &&
        ((pos + file.size()) == full_path.size()) &&
        (only_filename == cmSystemTools::GetFilenameName(file));
      if (trace) {
        break;
      }
    }
    // Do nothing if current file wasn't requested for trace...
    if (!trace) {
      return;
    }
  }

  std::ostringstream msg;
  std::vector<std::string> args;
  std::string temp;
  bool expand = this->GetCMakeInstance()->GetTraceExpand();

  args.reserve(lff.Arguments().size());
  for (cmListFileArgument const& arg : lff.Arguments()) {
    if (expand && arg.Delim != cmListFileArgument::Bracket) {
      temp = arg.Value;
      this->ExpandVariablesInString(temp);
      args.push_back(temp);
    } else {
      args.push_back(arg.Value);
    }
  }
  cm::optional<std::string> const& deferId = bt.Top().DeferId;

  switch (this->GetCMakeInstance()->GetTraceFormat()) {
    case cmake::TraceFormat::TRACE_JSON_V1: {
#ifndef CMAKE_BOOTSTRAP
      Json::Value val;
      Json::StreamWriterBuilder builder;
      builder["indentation"] = "";
      val["file"] = full_path;
      val["line"] = static_cast<Json::Value::Int64>(lff.Line());
      if (lff.Line() != lff.LineEnd()) {
        val["line_end"] = static_cast<Json::Value::Int64>(lff.LineEnd());
      }
      if (deferId) {
        val["defer"] = *deferId;
      }
      val["cmd"] = lff.OriginalName();
      val["args"] = Json::Value(Json::arrayValue);
      for (std::string const& arg : args) {
        val["args"].append(arg);
      }
      val["time"] = cmSystemTools::GetTime();
      val["frame"] = (missing == CommandMissingFromStack::Yes ? 1 : 0) +
        static_cast<Json::Value::UInt64>(this->ExecutionStatusStack.size());
      val["global_frame"] = (missing == CommandMissingFromStack::Yes ? 1 : 0) +
        static_cast<Json::Value::UInt64>(this->RecursionDepth);
      msg << Json::writeString(builder, val);
#endif
      break;
    }
    case cmake::TraceFormat::TRACE_HUMAN:
      msg << full_path << "(" << lff.Line() << "):";
      if (deferId) {
        msg << "DEFERRED:" << *deferId << ":";
      }
      msg << "  " << lff.OriginalName() << "(";

      for (std::string const& arg : args) {
        msg << arg << " ";
      }
      msg << ")";
      break;
    case cmake::TraceFormat::TRACE_UNDEFINED:
      msg << "INTERNAL ERROR: Trace format is TRACE_UNDEFINED";
      break;
  }

  auto& f = this->GetCMakeInstance()->GetTraceFile();
  if (f) {
    f << msg.str() << '\n';
  } else {
    cmSystemTools::Message(msg.str());
  }
}

// Helper class to make sure the call stack is valid.
class cmMakefileCall
{
public:
  cmMakefileCall(cmMakefile* mf, cmListFileFunction const& lff,
                 cm::optional<std::string> deferId, cmExecutionStatus& status)
    : Makefile(mf)
  {
    cmListFileContext const& lfc = cmListFileContext::FromListFileFunction(
      lff, this->Makefile->StateSnapshot.GetExecutionListFile(),
      std::move(deferId));
    this->Makefile->Backtrace = this->Makefile->Backtrace.Push(lfc);
    ++this->Makefile->RecursionDepth;
    this->Makefile->ExecutionStatusStack.push_back(&status);
#if !defined(CMAKE_BOOTSTRAP)
    if (this->Makefile->GetCMakeInstance()->IsProfilingEnabled()) {
      this->Makefile->GetCMakeInstance()->GetProfilingOutput().StartEntry(lff,
                                                                          lfc);
    }
#endif
  }

  ~cmMakefileCall()
  {
#if !defined(CMAKE_BOOTSTRAP)
    if (this->Makefile->GetCMakeInstance()->IsProfilingEnabled()) {
      this->Makefile->GetCMakeInstance()->GetProfilingOutput().StopEntry();
    }
#endif
    this->Makefile->ExecutionStatusStack.pop_back();
    --this->Makefile->RecursionDepth;
    this->Makefile->Backtrace = this->Makefile->Backtrace.Pop();
  }

  cmMakefileCall(const cmMakefileCall&) = delete;
  cmMakefileCall& operator=(const cmMakefileCall&) = delete;

private:
  cmMakefile* Makefile;
};

void cmMakefile::OnExecuteCommand(std::function<void()> callback)
{
  this->ExecuteCommandCallback = std::move(callback);
}

bool cmMakefile::ExecuteCommand(const cmListFileFunction& lff,
                                cmExecutionStatus& status,
                                cm::optional<std::string> deferId)
{
  bool result = true;

  // quick return if blocked
  if (this->IsFunctionBlocked(lff, status)) {
    // No error.
    return result;
  }

  if (this->ExecuteCommandCallback) {
    this->ExecuteCommandCallback();
  }

  // Place this call on the call stack.
  cmMakefileCall stack_manager(this, lff, std::move(deferId), status);
  static_cast<void>(stack_manager);

  // Check for maximum recursion depth.
  int depth = CMake_DEFAULT_RECURSION_LIMIT;
  cmValue depthStr = this->GetDefinition("CMAKE_MAXIMUM_RECURSION_DEPTH");
  if (depthStr) {
    std::istringstream s(*depthStr);
    int d;
    if (s >> d) {
      depth = d;
    }
  }
  if (this->RecursionDepth > depth) {
    std::ostringstream e;
    e << "Maximum recursion depth of " << depth << " exceeded";
    this->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  // Lookup the command prototype.
  if (cmState::Command command =
        this->GetState()->GetCommandByExactName(lff.LowerCaseName())) {
    // Decide whether to invoke the command.
    if (!cmSystemTools::GetFatalErrorOccurred()) {
      // if trace is enabled, print out invoke information
      if (this->GetCMakeInstance()->GetTrace()) {
        this->PrintCommandTrace(lff, this->Backtrace);
      }
      // Try invoking the command.
      bool invokeSucceeded = command(lff.Arguments(), status);
      bool hadNestedError = status.GetNestedError();
      if (!invokeSucceeded || hadNestedError) {
        if (!hadNestedError) {
          // The command invocation requested that we report an error.
          std::string const error =
            std::string(lff.OriginalName()) + " " + status.GetError();
          this->IssueMessage(MessageType::FATAL_ERROR, error);
        }
        result = false;
        if (this->GetCMakeInstance()->GetWorkingMode() != cmake::NORMAL_MODE) {
          cmSystemTools::SetFatalErrorOccurred();
        }
      }
    }
  } else {
    if (!cmSystemTools::GetFatalErrorOccurred()) {
      std::string error =
        cmStrCat("Unknown CMake command \"", lff.OriginalName(), "\".");
      this->IssueMessage(MessageType::FATAL_ERROR, error);
      result = false;
      cmSystemTools::SetFatalErrorOccurred();
    }
  }

  return result;
}

bool cmMakefile::IsImportedTargetGlobalScope() const
{
  return this->CurrentImportedTargetScope == ImportedTargetScope::Global;
}

class cmMakefile::IncludeScope
{
public:
  IncludeScope(cmMakefile* mf, std::string const& filenametoread,
               bool noPolicyScope);
  ~IncludeScope();
  void Quiet() { this->ReportError = false; }

  IncludeScope(const IncludeScope&) = delete;
  IncludeScope& operator=(const IncludeScope&) = delete;

private:
  cmMakefile* Makefile;
  bool NoPolicyScope;
  bool CheckCMP0011 = false;
  bool ReportError = true;
  void EnforceCMP0011();
};

cmMakefile::IncludeScope::IncludeScope(cmMakefile* mf,
                                       std::string const& filenametoread,
                                       bool noPolicyScope)
  : Makefile(mf)
  , NoPolicyScope(noPolicyScope)
{
  this->Makefile->Backtrace = this->Makefile->Backtrace.Push(
    cmListFileContext::FromListFilePath(filenametoread));

  this->Makefile->PushFunctionBlockerBarrier();

  this->Makefile->StateSnapshot =
    this->Makefile->GetState()->CreateIncludeFileSnapshot(
      this->Makefile->StateSnapshot, filenametoread);
  if (!this->NoPolicyScope) {
    // Check CMP0011 to determine the policy scope type.
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0011)) {
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
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to push a (strong) scope.
        this->Makefile->PushPolicy();
        break;
    }
  }
}

cmMakefile::IncludeScope::~IncludeScope()
{
  if (!this->NoPolicyScope) {
    // If we need to enforce policy CMP0011 then the top entry is the
    // one we pushed above.  If the entry is empty, then the included
    // script did not set any policies that might affect the includer so
    // we do not need to enforce the policy.
    if (this->CheckCMP0011 &&
        !this->Makefile->StateSnapshot.HasDefinedPolicyCMP0011()) {
      this->CheckCMP0011 = false;
    }

    // Pop the scope we pushed for the script.
    this->Makefile->PopPolicy();

    // We enforce the policy after the script's policy stack entry has
    // been removed.
    if (this->CheckCMP0011) {
      this->EnforceCMP0011();
    }
  }
  this->Makefile->PopSnapshot(this->ReportError);

  this->Makefile->PopFunctionBlockerBarrier(this->ReportError);

  this->Makefile->Backtrace = this->Makefile->Backtrace.Pop();
}

void cmMakefile::IncludeScope::EnforceCMP0011()
{
  // We check the setting of this policy again because the included
  // script might actually set this policy for its includer.
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0011)) {
    case cmPolicies::WARN:
      // Warn because the user did not set this policy.
      {
        std::ostringstream w;
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0011) << "\n"
          << "The included script\n  "
          << this->Makefile->GetBacktrace().Top().FilePath << "\n"
          << "affects policy settings.  "
          << "CMake is implying the NO_POLICY_SCOPE option for compatibility, "
          << "so the effects are applied to the including context.";
        this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS: {
      std::ostringstream e;
      /* clang-format off */
      e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0011) << "\n"
        << "The included script\n  "
        << this->Makefile->GetBacktrace().Top().FilePath << "\n"
        << "affects policy settings, so it requires this policy to be set.";
      /* clang-format on */
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    } break;
    case cmPolicies::OLD:
    case cmPolicies::NEW:
      // The script set this policy.  We assume the purpose of the
      // script is to initialize policies for its includer, and since
      // the policy is now set for later scripts, we do not warn.
      break;
  }
}

bool cmMakefile::ReadDependentFile(const std::string& filename,
                                   bool noPolicyScope)
{
  if (cmValue def = this->GetDefinition("CMAKE_CURRENT_LIST_FILE")) {
    this->AddDefinition("CMAKE_PARENT_LIST_FILE", *def);
  }
  std::string filenametoread = cmSystemTools::CollapseFullPath(
    filename, this->GetCurrentSourceDirectory());

  IncludeScope incScope(this, filenametoread, noPolicyScope);

  cmListFile listFile;
  if (!listFile.ParseFile(filenametoread.c_str(), this->GetMessenger(),
                          this->Backtrace)) {
    return false;
  }

  this->RunListFile(listFile, filenametoread);
  if (cmSystemTools::GetFatalErrorOccurred()) {
    incScope.Quiet();
  }
  return true;
}

class cmMakefile::ListFileScope
{
public:
  ListFileScope(cmMakefile* mf, std::string const& filenametoread)
    : Makefile(mf)
  {
    this->Makefile->Backtrace = this->Makefile->Backtrace.Push(
      cmListFileContext::FromListFilePath(filenametoread));

    this->Makefile->StateSnapshot =
      this->Makefile->GetState()->CreateInlineListFileSnapshot(
        this->Makefile->StateSnapshot, filenametoread);
    assert(this->Makefile->StateSnapshot.IsValid());

    this->Makefile->PushFunctionBlockerBarrier();
  }

  ~ListFileScope()
  {
    this->Makefile->PopSnapshot(this->ReportError);
    this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
    this->Makefile->Backtrace = this->Makefile->Backtrace.Pop();
  }

  void Quiet() { this->ReportError = false; }

  ListFileScope(const ListFileScope&) = delete;
  ListFileScope& operator=(const ListFileScope&) = delete;

private:
  cmMakefile* Makefile;
  bool ReportError = true;
};

class cmMakefile::DeferScope
{
public:
  DeferScope(cmMakefile* mf, std::string const& deferredInFile)
    : Makefile(mf)
  {
    cmListFileContext lfc;
    lfc.Line = cmListFileContext::DeferPlaceholderLine;
    lfc.FilePath = deferredInFile;
    this->Makefile->Backtrace = this->Makefile->Backtrace.Push(lfc);
    this->Makefile->DeferRunning = true;
  }

  ~DeferScope()
  {
    this->Makefile->DeferRunning = false;
    this->Makefile->Backtrace = this->Makefile->Backtrace.Pop();
  }

  DeferScope(const DeferScope&) = delete;
  DeferScope& operator=(const DeferScope&) = delete;

private:
  cmMakefile* Makefile;
};

class cmMakefile::DeferCallScope
{
public:
  DeferCallScope(cmMakefile* mf, std::string const& deferredFromFile)
    : Makefile(mf)
  {
    this->Makefile->StateSnapshot =
      this->Makefile->GetState()->CreateDeferCallSnapshot(
        this->Makefile->StateSnapshot, deferredFromFile);
    assert(this->Makefile->StateSnapshot.IsValid());
  }

  ~DeferCallScope() { this->Makefile->PopSnapshot(); }

  DeferCallScope(const DeferCallScope&) = delete;
  DeferCallScope& operator=(const DeferCallScope&) = delete;

private:
  cmMakefile* Makefile;
};

bool cmMakefile::ReadListFile(const std::string& filename)
{
  std::string filenametoread = cmSystemTools::CollapseFullPath(
    filename, this->GetCurrentSourceDirectory());

  ListFileScope scope(this, filenametoread);

  cmListFile listFile;
  if (!listFile.ParseFile(filenametoread.c_str(), this->GetMessenger(),
                          this->Backtrace)) {
    return false;
  }

  this->RunListFile(listFile, filenametoread);
  if (cmSystemTools::GetFatalErrorOccurred()) {
    scope.Quiet();
  }
  return true;
}

bool cmMakefile::ReadListFileAsString(const std::string& content,
                                      const std::string& virtualFileName)
{
  std::string filenametoread = cmSystemTools::CollapseFullPath(
    virtualFileName, this->GetCurrentSourceDirectory());

  ListFileScope scope(this, filenametoread);

  cmListFile listFile;
  if (!listFile.ParseString(content.c_str(), virtualFileName.c_str(),
                            this->GetMessenger(), this->Backtrace)) {
    return false;
  }

  this->RunListFile(listFile, filenametoread);
  if (cmSystemTools::GetFatalErrorOccurred()) {
    scope.Quiet();
  }
  return true;
}

void cmMakefile::RunListFile(cmListFile const& listFile,
                             std::string const& filenametoread,
                             DeferCommands* defer)
{
  // add this list file to the list of dependencies
  this->ListFiles.push_back(filenametoread);

  std::string currentParentFile =
    this->GetSafeDefinition("CMAKE_PARENT_LIST_FILE");
  std::string currentFile = this->GetSafeDefinition("CMAKE_CURRENT_LIST_FILE");

  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", filenametoread);
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                      cmSystemTools::GetFilenamePath(filenametoread));

  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");

  // Run the parsed commands.
  const size_t numberFunctions = listFile.Functions.size();
  for (size_t i = 0; i < numberFunctions; ++i) {
    cmExecutionStatus status(*this);
    this->ExecuteCommand(listFile.Functions[i], status);
    if (cmSystemTools::GetFatalErrorOccurred()) {
      break;
    }
    if (status.GetReturnInvoked()) {
      // Exit early due to return command.
      break;
    }
  }

  // Run any deferred commands.
  if (defer) {
    // Add a backtrace level indicating calls are deferred.
    DeferScope scope(this, filenametoread);

    // Iterate by index in case one deferred call schedules another.
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (size_t i = 0; i < defer->Commands.size(); ++i) {
      DeferCommand& d = defer->Commands[i];
      if (d.Id.empty()) {
        // Canceled.
        continue;
      }
      // Mark as executed.
      std::string id = std::move(d.Id);

      // The deferred call may have come from another file.
      DeferCallScope callScope(this, d.FilePath);

      cmExecutionStatus status(*this);
      this->ExecuteCommand(d.Command, status, std::move(id));
      if (cmSystemTools::GetFatalErrorOccurred()) {
        break;
      }
    }
  }

  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentParentFile);
  this->AddDefinition("CMAKE_CURRENT_LIST_FILE", currentFile);
  this->AddDefinition("CMAKE_CURRENT_LIST_DIR",
                      cmSystemTools::GetFilenamePath(currentFile));
  this->MarkVariableAsUsed("CMAKE_PARENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_FILE");
  this->MarkVariableAsUsed("CMAKE_CURRENT_LIST_DIR");
}

void cmMakefile::EnforceDirectoryLevelRules() const
{
  // Diagnose a violation of CMP0000 if necessary.
  if (this->CheckCMP0000) {
    std::ostringstream msg;
    msg << "No cmake_minimum_required command is present.  "
        << "A line of code such as\n"
        << "  cmake_minimum_required(VERSION " << cmVersion::GetMajorVersion()
        << "." << cmVersion::GetMinorVersion() << ")\n"
        << "should be added at the top of the file.  "
        << "The version specified may be lower if you wish to "
        << "support older CMake versions for this project.  "
        << "For more information run "
        << "\"cmake --help-policy CMP0000\".";
    switch (this->GetPolicyStatus(cmPolicies::CMP0000)) {
      case cmPolicies::WARN:
        // Warn because the user did not provide a minimum required
        // version.
        this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                               msg.str(), this->Backtrace);
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to use policy version 2.4 set in
        // cmListFileCache.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        // NEW behavior is to issue an error.
        this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                               msg.str(), this->Backtrace);
        cmSystemTools::SetFatalErrorOccurred();
        break;
    }
  }
}

void cmMakefile::AddEvaluationFile(
  const std::string& inputFile, const std::string& targetName,
  std::unique_ptr<cmCompiledGeneratorExpression> outputName,
  std::unique_ptr<cmCompiledGeneratorExpression> condition,
  const std::string& newLineCharacter, mode_t permissions, bool inputIsContent)
{
  this->EvaluationFiles.push_back(
    cm::make_unique<cmGeneratorExpressionEvaluationFile>(
      inputFile, targetName, std::move(outputName), std::move(condition),
      inputIsContent, newLineCharacter, permissions,
      this->GetPolicyStatus(cmPolicies::CMP0070)));
}

const std::vector<std::unique_ptr<cmGeneratorExpressionEvaluationFile>>&
cmMakefile::GetEvaluationFiles() const
{
  return this->EvaluationFiles;
}

std::vector<std::unique_ptr<cmExportBuildFileGenerator>> const&
cmMakefile::GetExportBuildFileGenerators() const
{
  return this->ExportBuildFileGenerators;
}

void cmMakefile::RemoveExportBuildFileGeneratorCMP0024(
  cmExportBuildFileGenerator* gen)
{
  auto it =
    std::find_if(this->ExportBuildFileGenerators.begin(),
                 this->ExportBuildFileGenerators.end(),
                 [gen](std::unique_ptr<cmExportBuildFileGenerator> const& p) {
                   return p.get() == gen;
                 });
  if (it != this->ExportBuildFileGenerators.end()) {
    this->ExportBuildFileGenerators.erase(it);
  }
}

void cmMakefile::AddExportBuildFileGenerator(
  std::unique_ptr<cmExportBuildFileGenerator> gen)
{
  this->ExportBuildFileGenerators.emplace_back(std::move(gen));
}

namespace {
struct file_not_persistent
{
  bool operator()(const std::string& path) const
  {
    return !(path.find("CMakeTmp") == std::string::npos &&
             cmSystemTools::FileExists(path));
  }
};
}

void cmMakefile::AddGeneratorAction(GeneratorAction&& action)
{
  assert(!this->GeneratorActionsInvoked);
  this->GeneratorActions.emplace_back(std::move(action), this->Backtrace);
}

void cmMakefile::GeneratorAction::operator()(cmLocalGenerator& lg,
                                             const cmListFileBacktrace& lfbt)
{
  if (cc) {
    CCAction(lg, lfbt, std::move(cc));
  } else {
    assert(Action);
    Action(lg, lfbt);
  }
}

void cmMakefile::DoGenerate(cmLocalGenerator& lg)
{
  // do all the variable expansions here
  this->ExpandVariablesCMP0019();

  // give all the commands a chance to do something
  // after the file has been parsed before generation
  for (auto& action : this->GeneratorActions) {
    action.Value(lg, action.Backtrace);
  }
  this->GeneratorActionsInvoked = true;

  // go through all configured files and see which ones still exist.
  // we don't want cmake to re-run if a configured file is created and deleted
  // during processing as that would make it a transient file that can't
  // influence the build process
  cm::erase_if(this->OutputFiles, file_not_persistent());

  // if a configured file is used as input for another configured file,
  // and then deleted it will show up in the input list files so we
  // need to scan those too
  cm::erase_if(this->ListFiles, file_not_persistent());
}

// Generate the output file
void cmMakefile::Generate(cmLocalGenerator& lg)
{
  this->DoGenerate(lg);
  cmValue oldValue = this->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if (oldValue &&
      cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, oldValue, "2.4")) {
    this->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      "You have set CMAKE_BACKWARDS_COMPATIBILITY to a CMake version less "
      "than 2.4. This version of CMake only supports backwards compatibility "
      "with CMake 2.4 or later. For compatibility with older versions please "
      "use any CMake 2.8.x release or lower.",
      this->Backtrace);
  }
}

namespace {
// There are still too many implicit backtraces through cmMakefile.  As a
// workaround we reset the backtrace temporarily.
struct BacktraceGuard
{
  BacktraceGuard(cmListFileBacktrace& lfbt, cmListFileBacktrace current)
    : Backtrace(lfbt)
    , Previous(lfbt)
  {
    this->Backtrace = std::move(current);
  }

  ~BacktraceGuard() { this->Backtrace = std::move(this->Previous); }

private:
  cmListFileBacktrace& Backtrace;
  cmListFileBacktrace Previous;
};
}

bool cmMakefile::ValidateCustomCommand(
  const cmCustomCommandLines& commandLines) const
{
  // TODO: More strict?
  for (cmCustomCommandLine const& cl : commandLines) {
    if (!cl.empty() && !cl[0].empty() && cl[0][0] == '"') {
      std::ostringstream e;
      e << "COMMAND may not contain literal quotes:\n  " << cl[0] << "\n";
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
    }
  }

  return true;
}

cmTarget* cmMakefile::GetCustomCommandTarget(
  const std::string& target, cmObjectLibraryCommands objLibCommands,
  const cmListFileBacktrace& lfbt) const
{
  // Find the target to which to add the custom command.
  auto ti = this->Targets.find(target);

  if (ti == this->Targets.end()) {
    MessageType messageType = MessageType::AUTHOR_WARNING;
    bool issueMessage = false;
    std::ostringstream e;
    switch (this->GetPolicyStatus(cmPolicies::CMP0040)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0040) << "\n";
        issueMessage = true;
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        issueMessage = true;
        messageType = MessageType::FATAL_ERROR;
        break;
    }

    if (issueMessage) {
      if (cmTarget const* t = this->FindTargetToUse(target)) {
        if (t->IsImported()) {
          e << "TARGET '" << target
            << "' is IMPORTED and does not build here.";
        } else {
          e << "TARGET '" << target << "' was not created in this directory.";
        }
      } else {
        e << "No TARGET '" << target
          << "' has been created in this directory.";
      }
      this->GetCMakeInstance()->IssueMessage(messageType, e.str(), lfbt);
    }

    return nullptr;
  }

  cmTarget* t = &ti->second;
  if (objLibCommands == cmObjectLibraryCommands::Reject &&
      t->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    std::ostringstream e;
    e << "Target \"" << target
      << "\" is an OBJECT library "
         "that may not have PRE_BUILD, PRE_LINK, or POST_BUILD commands.";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                                           lfbt);
    return nullptr;
  }
  if (t->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    std::ostringstream e;
    e << "Target \"" << target
      << "\" is an INTERFACE library "
         "that may not have PRE_BUILD, PRE_LINK, or POST_BUILD commands.";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e.str(),
                                           lfbt);
    return nullptr;
  }

  return t;
}

cmTarget* cmMakefile::AddCustomCommandToTarget(
  const std::string& target, cmCustomCommandType type,
  std::unique_ptr<cmCustomCommand> cc)
{
  const auto& byproducts = cc->GetByproducts();
  const auto& commandLines = cc->GetCommandLines();

  cmTarget* t = this->GetCustomCommandTarget(
    target, cmObjectLibraryCommands::Reject, this->Backtrace);

  // Validate custom commands.
  if (!t || !this->ValidateCustomCommand(commandLines)) {
    return t;
  }

  // Always create the byproduct sources and mark them generated.
  this->CreateGeneratedOutputs(byproducts);

  cc->SetCMP0116Status(this->GetPolicyStatus(cmPolicies::CMP0116));

  // Dispatch command creation to allow generator expressions in outputs.
  this->AddGeneratorAction(
    std::move(cc),
    [=](cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
        std::unique_ptr<cmCustomCommand> tcc) {
      BacktraceGuard guard(this->Backtrace, lfbt);
      tcc->SetBacktrace(lfbt);
      detail::AddCustomCommandToTarget(lg, cmCommandOrigin::Project, t, type,
                                       std::move(tcc));
    });

  return t;
}

void cmMakefile::AddCustomCommandToOutput(
  std::unique_ptr<cmCustomCommand> cc, const CommandSourceCallback& callback,
  bool replace)
{
  const auto& outputs = cc->GetOutputs();
  const auto& byproducts = cc->GetByproducts();
  const auto& commandLines = cc->GetCommandLines();

  // Make sure there is at least one output.
  if (outputs.empty()) {
    cmSystemTools::Error("Attempt to add a custom rule with no output!");
    return;
  }

  // Validate custom commands.
  if (!this->ValidateCustomCommand(commandLines)) {
    return;
  }

  // Always create the output sources and mark them generated.
  this->CreateGeneratedOutputs(outputs);
  this->CreateGeneratedOutputs(byproducts);

  cc->SetCMP0116Status(this->GetPolicyStatus(cmPolicies::CMP0116));

  // Dispatch command creation to allow generator expressions in outputs.
  this->AddGeneratorAction(
    std::move(cc),
    [=](cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
        std::unique_ptr<cmCustomCommand> tcc) {
      BacktraceGuard guard(this->Backtrace, lfbt);
      tcc->SetBacktrace(lfbt);
      cmSourceFile* sf = detail::AddCustomCommandToOutput(
        lg, cmCommandOrigin::Project, std::move(tcc), replace);
      if (callback && sf) {
        callback(sf);
      }
    });
}

void cmMakefile::AddCustomCommandOldStyle(
  const std::string& target, const std::vector<std::string>& outputs,
  const std::vector<std::string>& depends, const std::string& source,
  const cmCustomCommandLines& commandLines, const char* comment)
{
  auto cc = cm::make_unique<cmCustomCommand>();
  cc->SetDepends(depends);
  cc->SetCommandLines(commandLines);
  cc->SetComment(comment);

  // Translate the old-style signature to one of the new-style
  // signatures.
  if (source == target) {
    // In the old-style signature if the source and target were the
    // same then it added a post-build rule to the target.  Preserve
    // this behavior.
    this->AddCustomCommandToTarget(target, cmCustomCommandType::POST_BUILD,
                                   std::move(cc));
    return;
  }

  auto ti = this->Targets.find(target);
  cmTarget* t = ti != this->Targets.end() ? &ti->second : nullptr;

  auto addRuleFileToTarget = [=](cmSourceFile* sf) {
    // If the rule was added to the source (and not a .rule file),
    // then add the source to the target to make sure the rule is
    // included.
    if (!sf->GetPropertyAsBool("__CMAKE_RULE")) {
      if (t) {
        t->AddSource(sf->ResolveFullPath());
      } else {
        cmSystemTools::Error("Attempt to add a custom rule to a target "
                             "that does not exist yet for target " +
                             target);
      }
    }
  };

  // Each output must get its own copy of this rule.
  cmsys::RegularExpression sourceFiles(
    "\\.(C|M|c|c\\+\\+|cc|cpp|cxx|mpp|ixx|cppm|cu|m|mm|"
    "rc|def|r|odl|idl|hpj|bat|h|h\\+\\+|"
    "hm|hpp|hxx|in|txx|inl)$");

  // Choose whether to use a main dependency.
  if (sourceFiles.find(source)) {
    // The source looks like a real file.  Use it as the main dependency.
    for (std::string const& output : outputs) {
      auto cc1 = cm::make_unique<cmCustomCommand>(*cc);
      cc1->SetOutputs(output);
      cc1->SetMainDependency(source);
      this->AddCustomCommandToOutput(std::move(cc1), addRuleFileToTarget);
    }
  } else {
    cc->AppendDepends({ source });

    // The source may not be a real file.  Do not use a main dependency.
    for (std::string const& output : outputs) {
      auto cc1 = cm::make_unique<cmCustomCommand>(*cc);
      cc1->SetOutputs(output);
      this->AddCustomCommandToOutput(std::move(cc1), addRuleFileToTarget);
    }
  }
}

void cmMakefile::AppendCustomCommandToOutput(
  const std::string& output, const std::vector<std::string>& depends,
  const cmImplicitDependsList& implicit_depends,
  const cmCustomCommandLines& commandLines)
{
  // Validate custom commands.
  if (this->ValidateCustomCommand(commandLines)) {
    // Dispatch command creation to allow generator expressions in outputs.
    this->AddGeneratorAction(
      [=](cmLocalGenerator& lg, const cmListFileBacktrace& lfbt) {
        BacktraceGuard guard(this->Backtrace, lfbt);
        detail::AppendCustomCommandToOutput(lg, lfbt, output, depends,
                                            implicit_depends, commandLines);
      });
  }
}

cmTarget* cmMakefile::AddUtilityCommand(const std::string& utilityName,
                                        bool excludeFromAll,
                                        std::unique_ptr<cmCustomCommand> cc)
{
  const auto& depends = cc->GetDepends();
  const auto& byproducts = cc->GetByproducts();
  const auto& commandLines = cc->GetCommandLines();
  cmTarget* target = this->AddNewUtilityTarget(utilityName, excludeFromAll);

  // Validate custom commands.
  if ((commandLines.empty() && depends.empty()) ||
      !this->ValidateCustomCommand(commandLines)) {
    return target;
  }

  // Always create the byproduct sources and mark them generated.
  this->CreateGeneratedOutputs(byproducts);

  cc->SetCMP0116Status(this->GetPolicyStatus(cmPolicies::CMP0116));

  // Dispatch command creation to allow generator expressions in outputs.
  this->AddGeneratorAction(
    std::move(cc),
    [=](cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
        std::unique_ptr<cmCustomCommand> tcc) {
      BacktraceGuard guard(this->Backtrace, lfbt);
      tcc->SetBacktrace(lfbt);
      detail::AddUtilityCommand(lg, cmCommandOrigin::Project, target,
                                std::move(tcc));
    });

  return target;
}

static void s_AddDefineFlag(std::string const& flag, std::string& dflags)
{
  // remove any \n\r
  std::string::size_type initSize = dflags.size();
  dflags += ' ';
  dflags += flag;
  std::string::iterator flagStart = dflags.begin() + initSize + 1;
  std::replace(flagStart, dflags.end(), '\n', ' ');
  std::replace(flagStart, dflags.end(), '\r', ' ');
}

void cmMakefile::AddDefineFlag(std::string const& flag)
{
  if (flag.empty()) {
    return;
  }

  // Update the string used for the old DEFINITIONS property.
  s_AddDefineFlag(flag, this->DefineFlagsOrig);

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if (this->ParseDefineFlag(flag, false)) {
    return;
  }

  // Add this flag that does not look like a definition.
  s_AddDefineFlag(flag, this->DefineFlags);
}

static void s_RemoveDefineFlag(std::string const& flag, std::string& dflags)
{
  std::string::size_type const len = flag.length();
  // Remove all instances of the flag that are surrounded by
  // whitespace or the beginning/end of the string.
  for (std::string::size_type lpos = dflags.find(flag, 0);
       lpos != std::string::npos; lpos = dflags.find(flag, lpos)) {
    std::string::size_type rpos = lpos + len;
    if ((lpos <= 0 || isspace(dflags[lpos - 1])) &&
        (rpos >= dflags.size() || isspace(dflags[rpos]))) {
      dflags.erase(lpos, len);
    } else {
      ++lpos;
    }
  }
}

void cmMakefile::RemoveDefineFlag(std::string const& flag)
{
  // Check the length of the flag to remove.
  if (flag.empty()) {
    return;
  }

  // Update the string used for the old DEFINITIONS property.
  s_RemoveDefineFlag(flag, this->DefineFlagsOrig);

  // If this is really a definition, update COMPILE_DEFINITIONS.
  if (this->ParseDefineFlag(flag, true)) {
    return;
  }

  // Remove this flag that does not look like a definition.
  s_RemoveDefineFlag(flag, this->DefineFlags);
}

void cmMakefile::AddCompileDefinition(std::string const& option)
{
  this->AppendProperty("COMPILE_DEFINITIONS", option);
}

void cmMakefile::AddCompileOption(std::string const& option)
{
  this->AppendProperty("COMPILE_OPTIONS", option);
}

void cmMakefile::AddLinkOption(std::string const& option)
{
  this->AppendProperty("LINK_OPTIONS", option);
}

void cmMakefile::AddLinkDirectory(std::string const& directory, bool before)
{
  if (before) {
    this->StateSnapshot.GetDirectory().PrependLinkDirectoriesEntry(
      BT<std::string>(directory, this->Backtrace));
  } else {
    this->StateSnapshot.GetDirectory().AppendLinkDirectoriesEntry(
      BT<std::string>(directory, this->Backtrace));
  }
}

bool cmMakefile::ParseDefineFlag(std::string const& def, bool remove)
{
  // Create a regular expression to match valid definitions.
  static cmsys::RegularExpression valid("^[-/]D[A-Za-z_][A-Za-z0-9_]*(=.*)?$");

  // Make sure the definition matches.
  if (!valid.find(def)) {
    return false;
  }

  // Definitions with non-trivial values require a policy check.
  static cmsys::RegularExpression trivial(
    "^[-/]D[A-Za-z_][A-Za-z0-9_]*(=[A-Za-z0-9_.]+)?$");
  if (!trivial.find(def)) {
    // This definition has a non-trivial value.
    switch (this->GetPolicyStatus(cmPolicies::CMP0005)) {
      case cmPolicies::WARN:
        this->IssueMessage(MessageType::AUTHOR_WARNING,
                           cmPolicies::GetPolicyWarning(cmPolicies::CMP0005));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to not escape the value.  We should not
        // convert the definition to use the property.
        return false;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0005));
        return false;
      case cmPolicies::NEW:
        // NEW behavior is to escape the value.  Proceed to convert it
        // to an entry in the property.
        break;
    }
  }

  // Get the definition part after the flag.
  const char* define = def.c_str() + 2;

  if (remove) {
    if (cmValue cdefs = this->GetProperty("COMPILE_DEFINITIONS")) {
      // Expand the list.
      std::vector<std::string> defs = cmExpandedList(*cdefs);

      // Recompose the list without the definition.
      auto defEnd = std::remove(defs.begin(), defs.end(), define);
      auto defBegin = defs.begin();
      std::string ndefs = cmJoin(cmMakeRange(defBegin, defEnd), ";");

      // Store the new list.
      this->SetProperty("COMPILE_DEFINITIONS", ndefs);
    }
  } else {
    // Append the definition to the directory property.
    this->AppendProperty("COMPILE_DEFINITIONS", define);
  }

  return true;
}

void cmMakefile::InitializeFromParent(cmMakefile* parent)
{
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
  if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
    this->SetProperty("COMPILE_DEFINITIONS",
                      parent->GetProperty("COMPILE_DEFINITIONS"));
    std::vector<std::string> configs =
      this->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
    for (std::string const& config : configs) {
      std::string defPropName =
        cmStrCat("COMPILE_DEFINITIONS_", cmSystemTools::UpperCase(config));
      cmValue prop = parent->GetProperty(defPropName);
      this->SetProperty(defPropName, prop);
    }
  }

  // labels
  this->SetProperty("LABELS", parent->GetProperty("LABELS"));

  // link libraries
  this->SetProperty("LINK_LIBRARIES", parent->GetProperty("LINK_LIBRARIES"));

  // the initial project name
  this->StateSnapshot.SetProjectName(parent->StateSnapshot.GetProjectName());

  // Copy include regular expressions.
  this->ComplainFileRegularExpression = parent->ComplainFileRegularExpression;

  // Imported targets.
  this->ImportedTargets = parent->ImportedTargets;

  // Non-global Alias targets.
  this->AliasTargets = parent->AliasTargets;

  // Recursion depth.
  this->RecursionDepth = parent->RecursionDepth;
}

void cmMakefile::AddInstallGenerator(std::unique_ptr<cmInstallGenerator> g)
{
  if (g) {
    this->InstallGenerators.push_back(std::move(g));
  }
}

void cmMakefile::AddTestGenerator(std::unique_ptr<cmTestGenerator> g)
{
  if (g) {
    this->TestGenerators.push_back(std::move(g));
  }
}

void cmMakefile::PushFunctionScope(std::string const& fileName,
                                   const cmPolicies::PolicyMap& pm)
{
  this->StateSnapshot = this->GetState()->CreateFunctionCallSnapshot(
    this->StateSnapshot, fileName);
  assert(this->StateSnapshot.IsValid());

  this->PushLoopBlockBarrier();

#if !defined(CMAKE_BOOTSTRAP)
  this->GetGlobalGenerator()->GetFileLockPool().PushFunctionScope();
#endif

  this->PushFunctionBlockerBarrier();

  this->PushPolicy(true, pm);
}

void cmMakefile::PopFunctionScope(bool reportError)
{
  this->PopPolicy();

  this->PopSnapshot(reportError);

  this->PopFunctionBlockerBarrier(reportError);

#if !defined(CMAKE_BOOTSTRAP)
  this->GetGlobalGenerator()->GetFileLockPool().PopFunctionScope();
#endif

  this->PopLoopBlockBarrier();
}

void cmMakefile::PushMacroScope(std::string const& fileName,
                                const cmPolicies::PolicyMap& pm)
{
  this->StateSnapshot =
    this->GetState()->CreateMacroCallSnapshot(this->StateSnapshot, fileName);
  assert(this->StateSnapshot.IsValid());

  this->PushFunctionBlockerBarrier();

  this->PushPolicy(true, pm);
}

void cmMakefile::PopMacroScope(bool reportError)
{
  this->PopPolicy();
  this->PopSnapshot(reportError);

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
    : Makefile(mf)
  {
    std::string currentStart =
      cmStrCat(this->Makefile->StateSnapshot.GetDirectory().GetCurrentSource(),
               "/CMakeLists.txt");
    this->Makefile->StateSnapshot.SetListFile(currentStart);
    this->Makefile->StateSnapshot =
      this->Makefile->StateSnapshot.GetState()->CreatePolicyScopeSnapshot(
        this->Makefile->StateSnapshot);
    this->Makefile->PushFunctionBlockerBarrier();

    this->GG = mf->GetGlobalGenerator();
    this->CurrentMakefile = this->GG->GetCurrentMakefile();
    this->Snapshot = this->GG->GetCMakeInstance()->GetCurrentSnapshot();
    this->GG->GetCMakeInstance()->SetCurrentSnapshot(this->Snapshot);
    this->GG->SetCurrentMakefile(mf);
#if !defined(CMAKE_BOOTSTRAP)
    this->GG->GetFileLockPool().PushFileScope();
#endif
  }

  ~BuildsystemFileScope()
  {
    this->Makefile->PopFunctionBlockerBarrier(this->ReportError);
    this->Makefile->PopSnapshot(this->ReportError);
#if !defined(CMAKE_BOOTSTRAP)
    this->GG->GetFileLockPool().PopFileScope();
#endif
    this->GG->SetCurrentMakefile(this->CurrentMakefile);
    this->GG->GetCMakeInstance()->SetCurrentSnapshot(this->Snapshot);
  }

  void Quiet() { this->ReportError = false; }

  BuildsystemFileScope(const BuildsystemFileScope&) = delete;
  BuildsystemFileScope& operator=(const BuildsystemFileScope&) = delete;

private:
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;
  cmMakefile* CurrentMakefile;
  cmStateSnapshot Snapshot;
  bool ReportError = true;
};

void cmMakefile::Configure()
{
  std::string currentStart = cmStrCat(
    this->StateSnapshot.GetDirectory().GetCurrentSource(), "/CMakeLists.txt");

  // Add the bottom of all backtraces within this directory.
  // We will never pop this scope because it should be available
  // for messages during the generate step too.
  this->Backtrace =
    this->Backtrace.Push(cmListFileContext::FromListFilePath(currentStart));

  BuildsystemFileScope scope(this);

  // make sure the CMakeFiles dir is there
  std::string filesDir = cmStrCat(
    this->StateSnapshot.GetDirectory().GetCurrentBinary(), "/CMakeFiles");
  cmSystemTools::MakeDirectory(filesDir);

  assert(cmSystemTools::FileExists(currentStart, true));
  this->AddDefinition("CMAKE_PARENT_LIST_FILE", currentStart);

  cmListFile listFile;
  if (!listFile.ParseFile(currentStart.c_str(), this->GetMessenger(),
                          this->Backtrace)) {
    return;
  }
  if (this->IsRootMakefile()) {
    bool hasVersion = false;
    // search for the right policy command
    for (cmListFileFunction const& func : listFile.Functions) {
      if (func.LowerCaseName() == "cmake_minimum_required") {
        hasVersion = true;
        break;
      }
    }
    // if no policy command is found this is an error if they use any
    // non advanced functions or a lot of functions
    if (!hasVersion) {
      bool isProblem = true;
      if (listFile.Functions.size() < 30) {
        // the list of simple commands DO NOT ADD TO THIS LIST!!!!!
        // these commands must have backwards compatibility forever and
        // and that is a lot longer than your tiny mind can comprehend mortal
        std::set<std::string> allowedCommands;
        allowedCommands.insert("project");
        allowedCommands.insert("set");
        allowedCommands.insert("if");
        allowedCommands.insert("endif");
        allowedCommands.insert("else");
        allowedCommands.insert("elseif");
        allowedCommands.insert("add_executable");
        allowedCommands.insert("add_library");
        allowedCommands.insert("target_link_libraries");
        allowedCommands.insert("option");
        allowedCommands.insert("message");
        isProblem = false;
        for (cmListFileFunction const& func : listFile.Functions) {
          if (!cm::contains(allowedCommands, func.LowerCaseName())) {
            isProblem = true;
            break;
          }
        }
      }

      if (isProblem) {
        // Tell the top level cmMakefile to diagnose
        // this violation of CMP0000.
        this->SetCheckCMP0000(true);

        // Implicitly set the version for the user.
        cmPolicies::ApplyPolicyVersion(this, 2, 4, 0,
                                       cmPolicies::WarnCompat::Off);
      }
    }
    bool hasProject = false;
    // search for a project command
    for (cmListFileFunction const& func : listFile.Functions) {
      if (func.LowerCaseName() == "project") {
        hasProject = true;
        break;
      }
    }
    // if no project command is found, add one
    if (!hasProject) {
      this->GetCMakeInstance()->IssueMessage(
        MessageType::AUTHOR_WARNING,
        "No project() command is present.  The top-level CMakeLists.txt "
        "file must contain a literal, direct call to the project() command.  "
        "Add a line of code such as\n"
        "  project(ProjectName)\n"
        "near the top of the file, but after cmake_minimum_required().\n"
        "CMake is pretending there is a \"project(Project)\" command on "
        "the first line.",
        this->Backtrace);
      cmListFileFunction project{ "project",
                                  0,
                                  0,
                                  { { "Project", cmListFileArgument::Unquoted,
                                      0 },
                                    { "__CMAKE_INJECTED_PROJECT_COMMAND__",
                                      cmListFileArgument::Unquoted, 0 } } };
      listFile.Functions.insert(listFile.Functions.begin(), project);
    }
  }

  this->Defer = cm::make_unique<DeferCommands>();
  this->RunListFile(listFile, currentStart, this->Defer.get());
  this->Defer.reset();
  if (cmSystemTools::GetFatalErrorOccurred()) {
    scope.Quiet();
  }

  // at the end handle any old style subdirs
  std::vector<cmMakefile*> subdirs = this->UnConfiguredDirectories;

  // for each subdir recurse
  auto sdi = subdirs.begin();
  for (; sdi != subdirs.end(); ++sdi) {
    (*sdi)->StateSnapshot.InitializeFromParent_ForSubdirsCommand();
    this->ConfigureSubDirectory(*sdi);
  }

  this->AddCMakeDependFilesFromUser();
}

void cmMakefile::ConfigureSubDirectory(cmMakefile* mf)
{
  mf->InitializeFromParent(this);
  std::string currentStart = mf->GetCurrentSourceDirectory();
  if (this->GetCMakeInstance()->GetDebugOutput()) {
    std::string msg = cmStrCat("   Entering             ", currentStart);
    cmSystemTools::Message(msg);
  }

  std::string const currentStartFile = currentStart + "/CMakeLists.txt";
  if (!cmSystemTools::FileExists(currentStartFile, true)) {
    // The file is missing.  Check policy CMP0014.
    std::ostringstream e;
    /* clang-format off */
    e << "The source directory\n"
      << "  " << currentStart << "\n"
      << "does not contain a CMakeLists.txt file.";
    /* clang-format on */
    switch (this->GetPolicyStatus(cmPolicies::CMP0014)) {
      case cmPolicies::WARN:
        // Print the warning.
        /* clang-format off */
        e << "\n"
          << "CMake does not support this case but it used "
          << "to work accidentally and is being allowed for "
          << "compatibility."
          << "\n"
          << cmPolicies::GetPolicyWarning(cmPolicies::CMP0014);
        /* clang-format on */
        this->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior does not warn.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        e << "\n" << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0014);
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior prints the error.
        this->IssueMessage(MessageType::FATAL_ERROR, e.str());
        break;
    }
    return;
  }
  // finally configure the subdir
  mf->Configure();

  if (this->GetCMakeInstance()->GetDebugOutput()) {
    std::string msg =
      cmStrCat("   Returning to         ", this->GetCurrentSourceDirectory());
    cmSystemTools::Message(msg);
  }
}

void cmMakefile::AddSubDirectory(const std::string& srcPath,
                                 const std::string& binPath,
                                 bool excludeFromAll, bool immediate)
{
  if (this->DeferRunning) {
    this->IssueMessage(
      MessageType::FATAL_ERROR,
      "Subdirectories may not be created during deferred execution.");
    return;
  }

  // Make sure the binary directory is unique.
  if (!this->EnforceUniqueDir(srcPath, binPath)) {
    return;
  }

  cmStateSnapshot newSnapshot =
    this->GetState()->CreateBuildsystemDirectorySnapshot(this->StateSnapshot);

  newSnapshot.GetDirectory().SetCurrentSource(srcPath);
  newSnapshot.GetDirectory().SetCurrentBinary(binPath);

  cmSystemTools::MakeDirectory(binPath);

  auto subMfu =
    cm::make_unique<cmMakefile>(this->GlobalGenerator, newSnapshot);
  auto* subMf = subMfu.get();
  this->GetGlobalGenerator()->AddMakefile(std::move(subMfu));

  if (excludeFromAll) {
    subMf->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
  }

  if (immediate) {
    this->ConfigureSubDirectory(subMf);
  } else {
    this->UnConfiguredDirectories.push_back(subMf);
  }

  this->AddInstallGenerator(cm::make_unique<cmInstallSubdirectoryGenerator>(
    subMf, binPath, this->GetBacktrace()));
}

const std::string& cmMakefile::GetCurrentSourceDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentSource();
}

const std::string& cmMakefile::GetCurrentBinaryDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentBinary();
}

std::vector<cmTarget*> cmMakefile::GetImportedTargets() const
{
  std::vector<cmTarget*> tgts;
  tgts.reserve(this->ImportedTargets.size());
  for (auto const& impTarget : this->ImportedTargets) {
    tgts.push_back(impTarget.second);
  }
  return tgts;
}

void cmMakefile::AddIncludeDirectories(const std::vector<std::string>& incs,
                                       bool before)
{
  if (incs.empty()) {
    return;
  }

  std::string entryString = cmJoin(incs, ";");
  if (before) {
    this->StateSnapshot.GetDirectory().PrependIncludeDirectoriesEntry(
      BT<std::string>(entryString, this->Backtrace));
  } else {
    this->StateSnapshot.GetDirectory().AppendIncludeDirectoriesEntry(
      BT<std::string>(entryString, this->Backtrace));
  }

  // Property on each target:
  for (auto& target : this->Targets) {
    cmTarget& t = target.second;
    t.InsertInclude(BT<std::string>(entryString, this->Backtrace), before);
  }
}

void cmMakefile::AddSystemIncludeDirectories(const std::set<std::string>& incs)
{
  if (incs.empty()) {
    return;
  }

  this->SystemIncludeDirectories.insert(incs.begin(), incs.end());

  for (auto& target : this->Targets) {
    cmTarget& t = target.second;
    t.AddSystemIncludeDirectories(incs);
  }
}

void cmMakefile::AddDefinition(const std::string& name, cm::string_view value)
{
  this->StateSnapshot.SetDefinition(name, value);

#ifndef CMAKE_BOOTSTRAP
  cmVariableWatch* vv = this->GetVariableWatch();
  if (vv) {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
                         value.data(), this);
  }
#endif
}

void cmMakefile::AddDefinitionBool(const std::string& name, bool value)
{
  this->AddDefinition(name, value ? "ON" : "OFF");
}

void cmMakefile::AddCacheDefinition(const std::string& name, const char* value,
                                    const char* doc,
                                    cmStateEnums::CacheEntryType type,
                                    bool force)
{
  cmValue existingValue = this->GetState()->GetInitializedCacheValue(name);
  // must be outside the following if() to keep it alive long enough
  std::string nvalue;

  if (existingValue &&
      (this->GetState()->GetCacheEntryType(name) ==
       cmStateEnums::UNINITIALIZED)) {
    // if this is not a force, then use the value from the cache
    // if it is a force, then use the value being passed in
    if (!force) {
      value = existingValue->c_str();
    }
    if (type == cmStateEnums::PATH || type == cmStateEnums::FILEPATH) {
      std::vector<std::string>::size_type cc;
      std::vector<std::string> files;
      nvalue = value ? value : "";

      cmExpandList(nvalue, files);
      nvalue.clear();
      for (cc = 0; cc < files.size(); cc++) {
        if (!cmIsOff(files[cc])) {
          files[cc] = cmSystemTools::CollapseFullPath(files[cc]);
        }
        if (cc > 0) {
          nvalue += ";";
        }
        nvalue += files[cc];
      }

      this->GetCMakeInstance()->AddCacheEntry(name, nvalue, doc, type);
      nvalue = *this->GetState()->GetInitializedCacheValue(name);
      value = nvalue.c_str();
    }
  }
  this->GetCMakeInstance()->AddCacheEntry(name, value, doc, type);
  switch (this->GetPolicyStatus(cmPolicies::CMP0126)) {
    case cmPolicies::WARN:
      if (this->PolicyOptionalWarningEnabled("CMAKE_POLICY_WARNING_CMP0126") &&
          this->IsNormalDefinitionSet(name)) {
        this->IssueMessage(
          MessageType::AUTHOR_WARNING,
          cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0126),
                   "\nFor compatibility with older versions of CMake, normal "
                   "variable \"",
                   name, "\" will be removed from the current scope."));
      }
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // if there was a definition then remove it
      this->StateSnapshot.RemoveDefinition(name);
      break;
    case cmPolicies::NEW:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      break;
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

void cmMakefile::MaybeWarnUninitialized(std::string const& variable,
                                        const char* sourceFilename) const
{
  // check to see if we need to print a warning
  // if strict mode is on and the variable has
  // not been "cleared"/initialized with a set(foo ) call
  if (this->GetCMakeInstance()->GetWarnUninitialized() &&
      !this->VariableInitialized(variable)) {
    if (this->CheckSystemVars ||
        (sourceFilename && this->IsProjectFile(sourceFilename))) {
      std::ostringstream msg;
      msg << "uninitialized variable \'" << variable << "\'";
      this->IssueMessage(MessageType::AUTHOR_WARNING, msg.str());
    }
  }
}

void cmMakefile::RemoveDefinition(const std::string& name)
{
  this->StateSnapshot.RemoveDefinition(name);
#ifndef CMAKE_BOOTSTRAP
  cmVariableWatch* vv = this->GetVariableWatch();
  if (vv) {
    vv->VariableAccessed(name, cmVariableWatch::VARIABLE_REMOVED_ACCESS,
                         nullptr, this);
  }
#endif
}

void cmMakefile::RemoveCacheDefinition(const std::string& name) const
{
  this->GetState()->RemoveCacheEntry(name);
}

void cmMakefile::SetProjectName(std::string const& p)
{
  this->StateSnapshot.SetProjectName(p);
}

void cmMakefile::AddGlobalLinkInformation(cmTarget& target)
{
  // for these targets do not add anything
  switch (target.GetType()) {
    case cmStateEnums::UTILITY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
      return;
    default:;
  }

  if (cmValue linkLibsProp = this->GetProperty("LINK_LIBRARIES")) {
    std::vector<std::string> linkLibs = cmExpandedList(*linkLibsProp);

    for (auto j = linkLibs.begin(); j != linkLibs.end(); ++j) {
      std::string libraryName = *j;
      cmTargetLinkLibraryType libType = GENERAL_LibraryType;
      if (libraryName == "optimized") {
        libType = OPTIMIZED_LibraryType;
        ++j;
        libraryName = *j;
      } else if (libraryName == "debug") {
        libType = DEBUG_LibraryType;
        ++j;
        libraryName = *j;
      }
      // This is equivalent to the target_link_libraries plain signature.
      target.AddLinkLibrary(*this, libraryName, libType);
      target.AppendProperty(
        "INTERFACE_LINK_LIBRARIES",
        target.GetDebugGeneratorExpressions(libraryName, libType));
    }
  }
}

void cmMakefile::AddAlias(const std::string& lname, std::string const& tgtName,
                          bool globallyVisible)
{
  this->AliasTargets[lname] = tgtName;
  if (globallyVisible) {
    this->GetGlobalGenerator()->AddAlias(lname, tgtName);
  }
}

cmTarget* cmMakefile::AddLibrary(const std::string& lname,
                                 cmStateEnums::TargetType type,
                                 const std::vector<std::string>& srcs,
                                 bool excludeFromAll)
{
  assert(type == cmStateEnums::STATIC_LIBRARY ||
         type == cmStateEnums::SHARED_LIBRARY ||
         type == cmStateEnums::MODULE_LIBRARY ||
         type == cmStateEnums::OBJECT_LIBRARY ||
         type == cmStateEnums::INTERFACE_LIBRARY);

  cmTarget* target = this->AddNewTarget(type, lname);
  // Clear its dependencies. Otherwise, dependencies might persist
  // over changes in CMakeLists.txt, making the information stale and
  // hence useless.
  target->ClearDependencyInformation(*this);
  if (excludeFromAll) {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
  }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(*target);
  return target;
}

cmTarget* cmMakefile::AddExecutable(const std::string& exeName,
                                    const std::vector<std::string>& srcs,
                                    bool excludeFromAll)
{
  cmTarget* target = this->AddNewTarget(cmStateEnums::EXECUTABLE, exeName);
  if (excludeFromAll) {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
  }
  target->AddSources(srcs);
  this->AddGlobalLinkInformation(*target);
  return target;
}

cmTarget* cmMakefile::AddNewTarget(cmStateEnums::TargetType type,
                                   const std::string& name)
{
  return &this->CreateNewTarget(name, type).first;
}

std::pair<cmTarget&, bool> cmMakefile::CreateNewTarget(
  const std::string& name, cmStateEnums::TargetType type,
  cmTarget::PerConfig perConfig)
{
  auto ib = this->Targets.emplace(
    name, cmTarget(name, type, cmTarget::VisibilityNormal, this, perConfig));
  auto it = ib.first;
  if (!ib.second) {
    return std::make_pair(std::ref(it->second), false);
  }
  this->OrderedTargets.push_back(&it->second);
  this->GetGlobalGenerator()->IndexTarget(&it->second);
  this->GetStateSnapshot().GetDirectory().AddNormalTargetName(name);
  return std::make_pair(std::ref(it->second), true);
}

cmTarget* cmMakefile::AddNewUtilityTarget(const std::string& utilityName,
                                          bool excludeFromAll)
{
  cmTarget* target = this->AddNewTarget(cmStateEnums::UTILITY, utilityName);
  if (excludeFromAll) {
    target->SetProperty("EXCLUDE_FROM_ALL", "TRUE");
  }
  return target;
}

namespace {
}

#if !defined(CMAKE_BOOTSTRAP)
cmSourceGroup* cmMakefile::GetSourceGroup(
  const std::vector<std::string>& name) const
{
  cmSourceGroup* sg = nullptr;

  // first look for source group starting with the same as the one we want
  for (cmSourceGroup const& srcGroup : this->SourceGroups) {
    std::string const& sgName = srcGroup.GetName();
    if (sgName == name[0]) {
      sg = const_cast<cmSourceGroup*>(&srcGroup);
      break;
    }
  }

  if (sg != nullptr) {
    // iterate through its children to find match source group
    for (unsigned int i = 1; i < name.size(); ++i) {
      sg = sg->LookupChild(name[i]);
      if (sg == nullptr) {
        break;
      }
    }
  }
  return sg;
}

void cmMakefile::AddSourceGroup(const std::string& name, const char* regex)
{
  std::vector<std::string> nameVector;
  nameVector.push_back(name);
  this->AddSourceGroup(nameVector, regex);
}

void cmMakefile::AddSourceGroup(const std::vector<std::string>& name,
                                const char* regex)
{
  cmSourceGroup* sg = nullptr;
  std::vector<std::string> currentName;
  int i = 0;
  const int lastElement = static_cast<int>(name.size() - 1);
  for (i = lastElement; i >= 0; --i) {
    currentName.assign(name.begin(), name.begin() + i + 1);
    sg = this->GetSourceGroup(currentName);
    if (sg != nullptr) {
      break;
    }
  }

  // i now contains the index of the last found component
  if (i == lastElement) {
    // group already exists, replace its regular expression
    if (regex && sg) {
      // We only want to set the regular expression.  If there are already
      // source files in the group, we don't want to remove them.
      sg->SetGroupRegex(regex);
    }
    return;
  }
  if (i == -1) {
    // group does not exist nor belong to any existing group
    // add its first component
    this->SourceGroups.emplace_back(name[0], regex);
    sg = this->GetSourceGroup(currentName);
    i = 0; // last component found
  }
  if (!sg) {
    cmSystemTools::Error("Could not create source group ");
    return;
  }
  // build the whole source group path
  for (++i; i <= lastElement; ++i) {
    sg->AddChild(cmSourceGroup(name[i], nullptr, sg->GetFullName().c_str()));
    sg = sg->LookupChild(name[i]);
  }

  sg->SetGroupRegex(regex);
}

cmSourceGroup* cmMakefile::GetOrCreateSourceGroup(
  const std::vector<std::string>& folders)
{
  cmSourceGroup* sg = this->GetSourceGroup(folders);
  if (sg == nullptr) {
    this->AddSourceGroup(folders);
    sg = this->GetSourceGroup(folders);
  }
  return sg;
}

cmSourceGroup* cmMakefile::GetOrCreateSourceGroup(const std::string& name)
{
  std::string delimiters;
  if (cmValue p = this->GetDefinition("SOURCE_GROUP_DELIMITER")) {
    delimiters = *p;
  } else {
    delimiters = "/\\";
  }
  return this->GetOrCreateSourceGroup(cmTokenize(name, delimiters));
}

/**
 * Find a source group whose regular expression matches the filename
 * part of the given source name.  Search backward through the list of
 * source groups, and take the first matching group found.  This way
 * non-inherited SOURCE_GROUP commands will have precedence over
 * inherited ones.
 */
cmSourceGroup* cmMakefile::FindSourceGroup(
  const std::string& source, std::vector<cmSourceGroup>& groups) const
{
  // First search for a group that lists the file explicitly.
  for (auto sg = groups.rbegin(); sg != groups.rend(); ++sg) {
    cmSourceGroup* result = sg->MatchChildrenFiles(source);
    if (result) {
      return result;
    }
  }

  // Now search for a group whose regex matches the file.
  for (auto sg = groups.rbegin(); sg != groups.rend(); ++sg) {
    cmSourceGroup* result = sg->MatchChildrenRegex(source);
    if (result) {
      return result;
    }
  }

  // Shouldn't get here, but just in case, return the default group.
  return groups.data();
}
#endif

static bool mightExpandVariablesCMP0019(const char* s)
{
  return s && *s && strstr(s, "${") && strchr(s, '}');
}

void cmMakefile::ExpandVariablesCMP0019()
{
  // Drop this ancient compatibility behavior with a policy.
  cmPolicies::PolicyStatus pol = this->GetPolicyStatus(cmPolicies::CMP0019);
  if (pol != cmPolicies::OLD && pol != cmPolicies::WARN) {
    return;
  }
  std::ostringstream w;

  cmValue includeDirs = this->GetProperty("INCLUDE_DIRECTORIES");
  if (includeDirs && mightExpandVariablesCMP0019(includeDirs->c_str())) {
    std::string dirs = *includeDirs;
    this->ExpandVariablesInString(dirs, true, true);
    if (pol == cmPolicies::WARN && dirs != *includeDirs) {
      /* clang-format off */
      w << "Evaluated directory INCLUDE_DIRECTORIES\n"
        << "  " << *includeDirs << "\n"
        << "as\n"
        << "  " << dirs << "\n";
      /* clang-format on */
    }
    this->SetProperty("INCLUDE_DIRECTORIES", dirs);
  }

  // Also for each target's INCLUDE_DIRECTORIES property:
  for (auto& target : this->Targets) {
    cmTarget& t = target.second;
    if (t.GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        t.GetType() == cmStateEnums::GLOBAL_TARGET) {
      continue;
    }
    includeDirs = t.GetProperty("INCLUDE_DIRECTORIES");
    if (includeDirs && mightExpandVariablesCMP0019(includeDirs->c_str())) {
      std::string dirs = *includeDirs;
      this->ExpandVariablesInString(dirs, true, true);
      if (pol == cmPolicies::WARN && dirs != *includeDirs) {
        /* clang-format off */
        w << "Evaluated target " << t.GetName() << " INCLUDE_DIRECTORIES\n"
          << "  " << *includeDirs << "\n"
          << "as\n"
          << "  " << dirs << "\n";
        /* clang-format on */
      }
      t.SetProperty("INCLUDE_DIRECTORIES", dirs);
    }
  }

  if (cmValue linkDirsProp = this->GetProperty("LINK_DIRECTORIES")) {
    if (mightExpandVariablesCMP0019(linkDirsProp->c_str())) {
      std::string d = *linkDirsProp;
      const std::string orig = d;
      this->ExpandVariablesInString(d, true, true);
      if (pol == cmPolicies::WARN && d != orig) {
        /* clang-format off */
        w << "Evaluated link directories\n"
          << "  " << orig << "\n"
          << "as\n"
          << "  " << d << "\n";
        /* clang-format on */
      }
    }
  }

  if (cmValue linkLibsProp = this->GetProperty("LINK_LIBRARIES")) {
    std::vector<std::string> linkLibs = cmExpandedList(*linkLibsProp);

    for (auto l = linkLibs.begin(); l != linkLibs.end(); ++l) {
      std::string libName = *l;
      if (libName == "optimized"_s || libName == "debug"_s) {
        ++l;
        libName = *l;
      }
      if (mightExpandVariablesCMP0019(libName.c_str())) {
        const std::string orig = libName;
        this->ExpandVariablesInString(libName, true, true);
        if (pol == cmPolicies::WARN && libName != orig) {
          /* clang-format off */
        w << "Evaluated link library\n"
          << "  " << orig << "\n"
          << "as\n"
          << "  " << libName << "\n";
          /* clang-format on */
        }
      }
    }
  }

  if (!w.str().empty()) {
    std::ostringstream m;
    /* clang-format off */
    m << cmPolicies::GetPolicyWarning(cmPolicies::CMP0019)
      << "\n"
      << "The following variable evaluations were encountered:\n"
      << w.str();
    /* clang-format on */
    this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                           m.str(), this->Backtrace);
  }
}

bool cmMakefile::IsOn(const std::string& name) const
{
  return cmIsOn(this->GetDefinition(name));
}

bool cmMakefile::IsSet(const std::string& name) const
{
  cmValue value = this->GetDefinition(name);
  if (!value) {
    return false;
  }

  if (value->empty()) {
    return false;
  }

  if (cmIsNOTFOUND(*value)) {
    return false;
  }

  return true;
}

bool cmMakefile::PlatformIs32Bit() const
{
  if (cmValue plat_abi = this->GetDefinition("CMAKE_INTERNAL_PLATFORM_ABI")) {
    if (*plat_abi == "ELF X32") {
      return false;
    }
  }
  if (cmValue sizeof_dptr = this->GetDefinition("CMAKE_SIZEOF_VOID_P")) {
    return atoi(sizeof_dptr->c_str()) == 4;
  }
  return false;
}

bool cmMakefile::PlatformIs64Bit() const
{
  if (cmValue sizeof_dptr = this->GetDefinition("CMAKE_SIZEOF_VOID_P")) {
    return atoi(sizeof_dptr->c_str()) == 8;
  }
  return false;
}

bool cmMakefile::PlatformIsx32() const
{
  if (cmValue plat_abi = this->GetDefinition("CMAKE_INTERNAL_PLATFORM_ABI")) {
    if (*plat_abi == "ELF X32") {
      return true;
    }
  }
  return false;
}

cmMakefile::AppleSDK cmMakefile::GetAppleSDKType() const
{
  std::string sdkRoot;
  sdkRoot = this->GetSafeDefinition("CMAKE_OSX_SYSROOT");
  sdkRoot = cmSystemTools::LowerCase(sdkRoot);

  struct
  {
    std::string name;
    AppleSDK sdk;
  } const sdkDatabase[]{
    { "appletvos", AppleSDK::AppleTVOS },
    { "appletvsimulator", AppleSDK::AppleTVSimulator },
    { "iphoneos", AppleSDK::IPhoneOS },
    { "iphonesimulator", AppleSDK::IPhoneSimulator },
    { "watchos", AppleSDK::WatchOS },
    { "watchsimulator", AppleSDK::WatchSimulator },
  };

  for (auto const& entry : sdkDatabase) {
    if (cmHasPrefix(sdkRoot, entry.name) ||
        sdkRoot.find(std::string("/") + entry.name) != std::string::npos) {
      return entry.sdk;
    }
  }

  return AppleSDK::MacOS;
}

bool cmMakefile::PlatformIsAppleEmbedded() const
{
  return this->GetAppleSDKType() != AppleSDK::MacOS;
}

const char* cmMakefile::GetSONameFlag(const std::string& language) const
{
  std::string name = "CMAKE_SHARED_LIBRARY_SONAME";
  if (!language.empty()) {
    name += "_";
    name += language;
  }
  name += "_FLAG";
  return this->GetDefinition(name).GetCStr();
}

bool cmMakefile::CanIWriteThisFile(std::string const& fileName) const
{
  if (!this->IsOn("CMAKE_DISABLE_SOURCE_CHANGES")) {
    return true;
  }
  // If we are doing an in-source build, then the test will always fail
  if (cmSystemTools::SameFile(this->GetHomeDirectory(),
                              this->GetHomeOutputDirectory())) {
    return !this->IsOn("CMAKE_DISABLE_IN_SOURCE_BUILD");
  }

  return !cmSystemTools::IsSubDirectory(fileName, this->GetHomeDirectory()) ||
    cmSystemTools::IsSubDirectory(fileName, this->GetHomeOutputDirectory()) ||
    cmSystemTools::SameFile(fileName, this->GetHomeOutputDirectory());
}

const std::string& cmMakefile::GetRequiredDefinition(
  const std::string& name) const
{
  static std::string const empty;
  cmValue def = this->GetDefinition(name);
  if (!def) {
    cmSystemTools::Error("Error required internal CMake variable not "
                         "set, cmake may not be built correctly.\n"
                         "Missing variable is:\n" +
                         name);
    return empty;
  }
  return *def;
}

bool cmMakefile::IsDefinitionSet(const std::string& name) const
{
  cmValue def = this->StateSnapshot.GetDefinition(name);
  if (!def) {
    def = this->GetState()->GetInitializedCacheValue(name);
  }
#ifndef CMAKE_BOOTSTRAP
  if (cmVariableWatch* vv = this->GetVariableWatch()) {
    if (!def) {
      vv->VariableAccessed(
        name, cmVariableWatch::UNKNOWN_VARIABLE_DEFINED_ACCESS, nullptr, this);
    }
  }
#endif
  return def != nullptr;
}

bool cmMakefile::IsNormalDefinitionSet(const std::string& name) const
{
  cmValue def = this->StateSnapshot.GetDefinition(name);
#ifndef CMAKE_BOOTSTRAP
  if (cmVariableWatch* vv = this->GetVariableWatch()) {
    if (!def) {
      vv->VariableAccessed(
        name, cmVariableWatch::UNKNOWN_VARIABLE_DEFINED_ACCESS, nullptr, this);
    }
  }
#endif
  return def != nullptr;
}

cmValue cmMakefile::GetDefinition(const std::string& name) const
{
  cmValue def = this->StateSnapshot.GetDefinition(name);
  if (!def) {
    def = this->GetState()->GetInitializedCacheValue(name);
  }
#ifndef CMAKE_BOOTSTRAP
  cmVariableWatch* vv = this->GetVariableWatch();
  if (vv && !this->SuppressSideEffects) {
    bool const watch_function_executed =
      vv->VariableAccessed(name,
                           def ? cmVariableWatch::VARIABLE_READ_ACCESS
                               : cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS,
                           def.GetCStr(), this);

    if (watch_function_executed) {
      // A callback was executed and may have caused re-allocation of the
      // variable storage.  Look it up again for now.
      // FIXME: Refactor variable storage to avoid this problem.
      def = this->StateSnapshot.GetDefinition(name);
      if (!def) {
        def = this->GetState()->GetInitializedCacheValue(name);
      }
    }
  }
#endif
  return def;
}

const std::string& cmMakefile::GetSafeDefinition(const std::string& name) const
{
  return this->GetDefinition(name);
}

bool cmMakefile::GetDefExpandList(const std::string& name,
                                  std::vector<std::string>& out,
                                  bool emptyArgs) const
{
  cmValue def = this->GetDefinition(name);
  if (!def) {
    return false;
  }
  cmExpandList(*def, out, emptyArgs);
  return true;
}

std::vector<std::string> cmMakefile::GetDefinitions() const
{
  std::vector<std::string> res = this->StateSnapshot.ClosureKeys();
  cm::append(res, this->GetState()->GetCacheEntryKeys());
  std::sort(res.begin(), res.end());
  return res;
}

const std::string& cmMakefile::ExpandVariablesInString(
  std::string& source) const
{
  return this->ExpandVariablesInString(source, false, false);
}

const std::string& cmMakefile::ExpandVariablesInString(
  std::string& source, bool escapeQuotes, bool noEscapes, bool atOnly,
  const char* filename, long line, bool removeEmpty, bool replaceAt) const
{
  bool compareResults = false;
  MessageType mtype = MessageType::LOG;
  std::string errorstr;
  std::string original;

  // Sanity check the @ONLY mode.
  if (atOnly && (!noEscapes || !removeEmpty)) {
    // This case should never be called.  At-only is for
    // configure-file/string which always does no escapes.
    this->IssueMessage(MessageType::INTERNAL_ERROR,
                       "ExpandVariablesInString @ONLY called "
                       "on something with escapes.");
    return source;
  }

  // Variables used in the WARN case.
  std::string newResult;
  std::string newErrorstr;
  MessageType newError = MessageType::LOG;

  switch (this->GetPolicyStatus(cmPolicies::CMP0053)) {
    case cmPolicies::WARN: {
      // Save the original string for the warning.
      original = source;
      newResult = source;
      compareResults = true;
      // Suppress variable watches to avoid calling hooks twice. Suppress new
      // dereferences since the OLD behavior is still what is actually used.
      this->SuppressSideEffects = true;
      newError = this->ExpandVariablesInStringNew(
        newErrorstr, newResult, escapeQuotes, noEscapes, atOnly, filename,
        line, replaceAt);
      this->SuppressSideEffects = false;
      CM_FALLTHROUGH;
    }
    case cmPolicies::OLD:
      mtype = this->ExpandVariablesInStringOld(errorstr, source, escapeQuotes,
                                               noEscapes, atOnly, filename,
                                               line, removeEmpty, true);
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    // Messaging here would be *very* verbose.
    case cmPolicies::NEW:
      mtype = this->ExpandVariablesInStringNew(errorstr, source, escapeQuotes,
                                               noEscapes, atOnly, filename,
                                               line, replaceAt);
      break;
  }

  // If it's an error in either case, just report the error...
  if (mtype != MessageType::LOG) {
    if (mtype == MessageType::FATAL_ERROR) {
      cmSystemTools::SetFatalErrorOccurred();
    }
    this->IssueMessage(mtype, errorstr);
  }
  // ...otherwise, see if there's a difference that needs to be warned about.
  else if (compareResults && (newResult != source || newError != mtype)) {
    std::string msg =
      cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0053), '\n');

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

    if (newError == mtype) {
      std::string msg_new = newResult;
      cmSystemTools::ReplaceString(msg_new, "\n", "\n  ");
      msg += "but the new evaluation rules produce:\n  '";
      msg += msg_new;
      msg += "'\n";
    } else {
      std::string msg_err = newErrorstr;
      cmSystemTools::ReplaceString(msg_err, "\n", "\n  ");
      msg += "but the new evaluation rules produce an error:\n  ";
      msg += msg_err;
      msg += "\n";
    }

    msg +=
      "Using the old result for compatibility since the policy is not set.";

    this->IssueMessage(MessageType::AUTHOR_WARNING, msg);
  }

  return source;
}

MessageType cmMakefile::ExpandVariablesInStringOld(
  std::string& errorstr, std::string& source, bool escapeQuotes,
  bool noEscapes, bool atOnly, const char* filename, long line,
  bool removeEmpty, bool replaceAt) const
{
  // Fast path strings without any special characters.
  if (source.find_first_of("$@\\") == std::string::npos) {
    return MessageType::LOG;
  }

  // Special-case the @ONLY mode.
  if (atOnly) {
    // Store an original copy of the input.
    std::string input = source;

    // Start with empty output.
    source.clear();

    // Look for one @VAR@ at a time.
    const char* in = input.c_str();
    while (this->cmAtVarRegex.find(in)) {
      // Get the range of the string to replace.
      const char* first = in + this->cmAtVarRegex.start();
      const char* last = in + this->cmAtVarRegex.end();

      // Store the unchanged part of the string now.
      source.append(in, first - in);

      // Lookup the definition of VAR.
      std::string var(first + 1, last - first - 2);
      if (cmValue val = this->GetDefinition(var)) {
        // Store the value in the output escaping as requested.
        if (escapeQuotes) {
          source.append(cmEscapeQuotes(*val));
        } else {
          source.append(*val);
        }
      }

      // Continue looking for @VAR@ further along the string.
      in = last;
    }

    // Append the rest of the unchanged part of the string.
    source.append(in);

    return MessageType::LOG;
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
  int res = parser.ParseString(source, 0);
  const char* emsg = parser.GetError();
  MessageType mtype = MessageType::LOG;
  if (res && !emsg[0]) {
    source = parser.GetResult();
  } else {
    // Construct the main error message.
    std::ostringstream error;
    error << "Syntax error in cmake code ";
    if (filename && line > 0) {
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
    mtype = MessageType::FATAL_ERROR;
    if (!res) {
      // This is a real argument parsing error.  Use policy CMP0010 to
      // decide whether it is an error.
      switch (this->GetPolicyStatus(cmPolicies::CMP0010)) {
        case cmPolicies::WARN:
          error << "\n" << cmPolicies::GetPolicyWarning(cmPolicies::CMP0010);
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          // OLD behavior is to just warn and continue.
          mtype = MessageType::AUTHOR_WARNING;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          error << "\n"
                << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0010);
          break;
        case cmPolicies::NEW:
          // NEW behavior is to report the error.
          break;
      }
    }
    errorstr = error.str();
  }
  return mtype;
}

enum t_domain
{
  NORMAL,
  ENVIRONMENT,
  CACHE
};

struct t_lookup
{
  t_domain domain = NORMAL;
  size_t loc = 0;
};

bool cmMakefile::IsProjectFile(const char* filename) const
{
  return cmSystemTools::IsSubDirectory(filename, this->GetHomeDirectory()) ||
    (cmSystemTools::IsSubDirectory(filename, this->GetHomeOutputDirectory()) &&
     !cmSystemTools::IsSubDirectory(filename, "/CMakeFiles"));
}

int cmMakefile::GetRecursionDepth() const
{
  return this->RecursionDepth;
}

void cmMakefile::SetRecursionDepth(int recursionDepth)
{
  this->RecursionDepth = recursionDepth;
}

std::string cmMakefile::NewDeferId() const
{
  return this->GetGlobalGenerator()->NewDeferId();
}

bool cmMakefile::DeferCall(std::string id, std::string file,
                           cmListFileFunction lff)
{
  if (!this->Defer) {
    return false;
  }
  this->Defer->Commands.emplace_back(
    DeferCommand{ std::move(id), std::move(file), std::move(lff) });
  return true;
}

bool cmMakefile::DeferCancelCall(std::string const& id)
{
  if (!this->Defer) {
    return false;
  }
  for (DeferCommand& dc : this->Defer->Commands) {
    if (dc.Id == id) {
      dc.Id.clear();
    }
  }
  return true;
}

cm::optional<std::string> cmMakefile::DeferGetCallIds() const
{
  cm::optional<std::string> ids;
  if (this->Defer) {
    ids = cmJoin(
      cmMakeRange(this->Defer->Commands)
        .filter([](DeferCommand const& dc) -> bool { return !dc.Id.empty(); })
        .transform(
          [](DeferCommand const& dc) -> std::string const& { return dc.Id; }),
      ";");
  }
  return ids;
}

cm::optional<std::string> cmMakefile::DeferGetCall(std::string const& id) const
{
  cm::optional<std::string> call;
  if (this->Defer) {
    std::string tmp;
    for (DeferCommand const& dc : this->Defer->Commands) {
      if (dc.Id == id) {
        tmp = dc.Command.OriginalName();
        for (cmListFileArgument const& arg : dc.Command.Arguments()) {
          tmp = cmStrCat(tmp, ';', arg.Value);
        }
        break;
      }
    }
    call = std::move(tmp);
  }
  return call;
}

MessageType cmMakefile::ExpandVariablesInStringNew(
  std::string& errorstr, std::string& source, bool escapeQuotes,
  bool noEscapes, bool atOnly, const char* filename, long line,
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
  std::vector<t_lookup> openstack;
  bool error = false;
  bool done = false;
  MessageType mtype = MessageType::LOG;

  cmState* state = this->GetCMakeInstance()->GetState();

  static const std::string lineVar = "CMAKE_CURRENT_LIST_LINE";
  do {
    char inc = *in;
    switch (inc) {
      case '}':
        if (!openstack.empty()) {
          t_lookup var = openstack.back();
          openstack.pop_back();
          result.append(last, in - last);
          std::string const& lookup = result.substr(var.loc);
          cmValue value = nullptr;
          std::string varresult;
          std::string svalue;
          switch (var.domain) {
            case NORMAL:
              if (filename && lookup == lineVar) {
                cmListFileContext const& top = this->Backtrace.Top();
                if (top.DeferId) {
                  varresult = cmStrCat("DEFERRED:"_s, *top.DeferId);
                } else {
                  varresult = std::to_string(line);
                }
              } else {
                value = this->GetDefinition(lookup);
              }
              break;
            case ENVIRONMENT:
              if (cmSystemTools::GetEnv(lookup, svalue)) {
                value = cmValue(svalue);
              }
              break;
            case CACHE:
              value = state->GetCacheEntryValue(lookup);
              break;
          }
          // Get the string we're meant to append to.
          if (value) {
            if (escapeQuotes) {
              varresult = cmEscapeQuotes(*value);
            } else {
              varresult = *value;
            }
          } else if (!this->SuppressSideEffects) {
            this->MaybeWarnUninitialized(lookup, filename);
          }
          result.replace(var.loc, result.size() - var.loc, varresult);
          // Start looking from here on out.
          last = in + 1;
        }
        break;
      case '$':
        if (!atOnly) {
          t_lookup lookup;
          const char* next = in + 1;
          const char* start = nullptr;
          char nextc = *next;
          if (nextc == '{') {
            // Looking for a variable.
            start = in + 2;
            lookup.domain = NORMAL;
          } else if (nextc == '<') {
          } else if (!nextc) {
            result.append(last, next - last);
            last = next;
          } else if (cmHasLiteralPrefix(next, "ENV{")) {
            // Looking for an environment variable.
            start = in + 5;
            lookup.domain = ENVIRONMENT;
          } else if (cmHasLiteralPrefix(next, "CACHE{")) {
            // Looking for a cache variable.
            start = in + 7;
            lookup.domain = CACHE;
          } else {
            if (this->cmNamedCurly.find(next)) {
              errorstr = "Syntax $" +
                std::string(next, this->cmNamedCurly.end()) +
                "{} is not supported.  Only ${}, $ENV{}, "
                "and $CACHE{} are allowed.";
              mtype = MessageType::FATAL_ERROR;
              error = true;
            }
          }
          if (start) {
            result.append(last, in - last);
            last = start;
            in = start - 1;
            lookup.loc = result.size();
            openstack.push_back(lookup);
          }
          break;
        }
        CM_FALLTHROUGH;
      case '\\':
        if (!noEscapes) {
          const char* next = in + 1;
          char nextc = *next;
          if (nextc == 't') {
            result.append(last, in - last);
            result.append("\t");
            last = next + 1;
          } else if (nextc == 'n') {
            result.append(last, in - last);
            result.append("\n");
            last = next + 1;
          } else if (nextc == 'r') {
            result.append(last, in - last);
            result.append("\r");
            last = next + 1;
          } else if (nextc == ';' && openstack.empty()) {
            // Handled in ExpandListArgument; pass the backslash literally.
          } else if (isalnum(nextc) || nextc == '\0') {
            errorstr += "Invalid character escape '\\";
            if (nextc) {
              errorstr += nextc;
              errorstr += "'.";
            } else {
              errorstr += "' (at end of input).";
            }
            error = true;
          } else {
            // Take what we've found so far, skipping the escape character.
            result.append(last, in - last);
            // Start tracking from the next character.
            last = in + 1;
          }
          // Skip the next character since it was escaped, but don't read past
          // the end of the string.
          if (*last) {
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
        if (replaceAt) {
          const char* nextAt = strchr(in + 1, '@');
          if (nextAt && nextAt != in + 1 &&
              nextAt ==
                in + 1 +
                  strspn(in + 1,
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz"
                         "0123456789/_.+-")) {
            std::string variable(in + 1, nextAt - in - 1);

            std::string varresult;
            if (filename && variable == lineVar) {
              varresult = std::to_string(line);
            } else {
              cmValue def = this->GetDefinition(variable);
              if (def) {
                varresult = *def;
              } else if (!this->SuppressSideEffects) {
                this->MaybeWarnUninitialized(variable, filename);
              }
            }

            if (escapeQuotes) {
              varresult = cmEscapeQuotes(varresult);
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
        CM_FALLTHROUGH;
      default: {
        if (!openstack.empty() &&
            !(isalnum(inc) || inc == '_' || inc == '/' || inc == '.' ||
              inc == '+' || inc == '-')) {
          errorstr += "Invalid character (\'";
          errorstr += inc;
          result.append(last, in - last);
          errorstr += "\') in a variable name: "
                      "'" +
            result.substr(openstack.back().loc) + "'";
          mtype = MessageType::FATAL_ERROR;
          error = true;
        }
        break;
      }
    }
    // Look at the next character.
  } while (!error && !done && *++in);

  // Check for open variable references yet.
  if (!error && !openstack.empty()) {
    // There's an open variable reference waiting.  Policy CMP0010 flags
    // whether this is an error or not.  The new parser now enforces
    // CMP0010 as well.
    errorstr += "There is an unterminated variable reference.";
    error = true;
  }

  if (error) {
    std::ostringstream emsg;
    emsg << "Syntax error in cmake code ";
    if (filename) {
      // This filename and line number may be more specific than the
      // command context because one command invocation can have
      // arguments on multiple lines.
      emsg << "at\n"
           << "  " << filename << ":" << line << "\n";
    }
    emsg << "when parsing string\n"
         << "  " << source << "\n";
    emsg << errorstr;
    mtype = MessageType::FATAL_ERROR;
    errorstr = emsg.str();
  } else {
    // Append the rest of the unchanged part of the string.
    result.append(last);

    source = result;
  }

  return mtype;
}

void cmMakefile::RemoveVariablesInString(std::string& source,
                                         bool atOnly) const
{
  if (!atOnly) {
    cmsys::RegularExpression var("(\\${[A-Za-z_0-9]*})");
    while (var.find(source)) {
      source.erase(var.start(), var.end() - var.start());
    }
  }

  if (!atOnly) {
    cmsys::RegularExpression varb("(\\$ENV{[A-Za-z_0-9]*})");
    while (varb.find(source)) {
      source.erase(varb.start(), varb.end() - varb.start());
    }
  }
  cmsys::RegularExpression var2("(@[A-Za-z_0-9]*@)");
  while (var2.find(source)) {
    source.erase(var2.start(), var2.end() - var2.start());
  }
}

void cmMakefile::InitCMAKE_CONFIGURATION_TYPES(std::string const& genDefault)
{
  if (this->GetDefinition("CMAKE_CONFIGURATION_TYPES")) {
    return;
  }
  std::string initConfigs;
  if (this->GetCMakeInstance()->GetIsInTryCompile() ||
      !cmSystemTools::GetEnv("CMAKE_CONFIGURATION_TYPES", initConfigs)) {
    initConfigs = genDefault;
  }
  this->AddCacheDefinition(
    "CMAKE_CONFIGURATION_TYPES", initConfigs,
    "Semicolon separated list of supported configuration types, "
    "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
    "anything else will be ignored.",
    cmStateEnums::STRING);
}

std::string cmMakefile::GetDefaultConfiguration() const
{
  if (this->GetGlobalGenerator()->IsMultiConfig()) {
    return std::string{};
  }
  return this->GetSafeDefinition("CMAKE_BUILD_TYPE");
}

std::vector<std::string> cmMakefile::GetGeneratorConfigs(
  GeneratorConfigQuery mode) const
{
  std::vector<std::string> configs;
  if (this->GetGlobalGenerator()->IsMultiConfig()) {
    this->GetDefExpandList("CMAKE_CONFIGURATION_TYPES", configs);
  } else if (mode != cmMakefile::OnlyMultiConfig) {
    const std::string& buildType = this->GetSafeDefinition("CMAKE_BUILD_TYPE");
    if (!buildType.empty()) {
      configs.emplace_back(buildType);
    }
  }
  if (mode == cmMakefile::IncludeEmptyConfig && configs.empty()) {
    configs.emplace_back();
  }
  return configs;
}

bool cmMakefile::IsFunctionBlocked(const cmListFileFunction& lff,
                                   cmExecutionStatus& status)
{
  // if there are no blockers get out of here
  if (this->FunctionBlockers.empty()) {
    return false;
  }

  return this->FunctionBlockers.top()->IsFunctionBlocked(lff, status);
}

void cmMakefile::PushFunctionBlockerBarrier()
{
  this->FunctionBlockerBarriers.push_back(this->FunctionBlockers.size());
}

void cmMakefile::PopFunctionBlockerBarrier(bool reportError)
{
  // Remove any extra entries pushed on the barrier.
  FunctionBlockersType::size_type barrier =
    this->FunctionBlockerBarriers.back();
  while (this->FunctionBlockers.size() > barrier) {
    std::unique_ptr<cmFunctionBlocker> fb(
      std::move(this->FunctionBlockers.top()));
    this->FunctionBlockers.pop();
    if (reportError) {
      // Report the context in which the unclosed block was opened.
      cmListFileContext const& lfc = fb->GetStartingContext();
      std::ostringstream e;
      /* clang-format off */
      e << "A logical block opening on the line\n"
        << "  " << lfc << "\n"
        << "is not closed.";
      /* clang-format on */
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      reportError = false;
    }
  }

  // Remove the barrier.
  this->FunctionBlockerBarriers.pop_back();
}

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

bool cmMakefile::ExpandArguments(std::vector<cmListFileArgument> const& inArgs,
                                 std::vector<std::string>& outArgs) const
{
  std::string const& filename = this->GetBacktrace().Top().FilePath;
  std::string value;
  outArgs.reserve(inArgs.size());
  for (cmListFileArgument const& i : inArgs) {
    // No expansion in a bracket argument.
    if (i.Delim == cmListFileArgument::Bracket) {
      outArgs.push_back(i.Value);
      continue;
    }
    // Expand the variables in the argument.
    value = i.Value;
    this->ExpandVariablesInString(value, false, false, false, filename.c_str(),
                                  i.Line, false, false);

    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if (i.Delim == cmListFileArgument::Quoted) {
      outArgs.push_back(value);
    } else {
      cmExpandList(value, outArgs);
    }
  }
  return !cmSystemTools::GetFatalErrorOccurred();
}

bool cmMakefile::ExpandArguments(
  std::vector<cmListFileArgument> const& inArgs,
  std::vector<cmExpandedCommandArgument>& outArgs) const
{
  std::string const& filename = this->GetBacktrace().Top().FilePath;
  std::string value;
  outArgs.reserve(inArgs.size());
  for (cmListFileArgument const& i : inArgs) {
    // No expansion in a bracket argument.
    if (i.Delim == cmListFileArgument::Bracket) {
      outArgs.emplace_back(i.Value, true);
      continue;
    }
    // Expand the variables in the argument.
    value = i.Value;
    this->ExpandVariablesInString(value, false, false, false, filename.c_str(),
                                  i.Line, false, false);

    // If the argument is quoted, it should be one argument.
    // Otherwise, it may be a list of arguments.
    if (i.Delim == cmListFileArgument::Quoted) {
      outArgs.emplace_back(value, true);
    } else {
      std::vector<std::string> stringArgs = cmExpandedList(value);
      for (std::string const& stringArg : stringArgs) {
        outArgs.emplace_back(stringArg, false);
      }
    }
  }
  return !cmSystemTools::GetFatalErrorOccurred();
}

void cmMakefile::AddFunctionBlocker(std::unique_ptr<cmFunctionBlocker> fb)
{
  if (!this->ExecutionStatusStack.empty()) {
    // Record the context in which the blocker is created.
    fb->SetStartingContext(this->Backtrace.Top());
  }

  this->FunctionBlockers.push(std::move(fb));
}

std::unique_ptr<cmFunctionBlocker> cmMakefile::RemoveFunctionBlocker()
{
  assert(!this->FunctionBlockers.empty());
  assert(this->FunctionBlockerBarriers.empty() ||
         this->FunctionBlockers.size() > this->FunctionBlockerBarriers.back());

  auto b = std::move(this->FunctionBlockers.top());
  this->FunctionBlockers.pop();
  return b;
}

std::string const& cmMakefile::GetHomeDirectory() const
{
  return this->GetCMakeInstance()->GetHomeDirectory();
}

std::string const& cmMakefile::GetHomeOutputDirectory() const
{
  return this->GetCMakeInstance()->GetHomeOutputDirectory();
}

void cmMakefile::SetScriptModeFile(std::string const& scriptfile)
{
  this->AddDefinition("CMAKE_SCRIPT_MODE_FILE", scriptfile);
}

void cmMakefile::SetArgcArgv(const std::vector<std::string>& args)
{
  this->AddDefinition("CMAKE_ARGC", std::to_string(args.size()));
  // this->MarkVariableAsUsed("CMAKE_ARGC");

  for (unsigned int t = 0; t < args.size(); ++t) {
    std::ostringstream tmpStream;
    tmpStream << "CMAKE_ARGV" << t;
    this->AddDefinition(tmpStream.str(), args[t]);
    // this->MarkVariableAsUsed(tmpStream.str().c_str());
  }
}

cmSourceFile* cmMakefile::GetSource(const std::string& sourceName,
                                    cmSourceFileLocationKind kind) const
{
  // First check "Known" paths (avoids the creation of cmSourceFileLocation)
  if (kind == cmSourceFileLocationKind::Known) {
    auto sfsi = this->KnownFileSearchIndex.find(sourceName);
    if (sfsi != this->KnownFileSearchIndex.end()) {
      return sfsi->second;
    }
  }

  cmSourceFileLocation sfl(this, sourceName, kind);
  auto name = this->GetCMakeInstance()->StripExtension(sfl.GetName());
#if defined(_WIN32) || defined(__APPLE__)
  name = cmSystemTools::LowerCase(name);
#endif
  auto sfsi = this->SourceFileSearchIndex.find(name);
  if (sfsi != this->SourceFileSearchIndex.end()) {
    for (auto* sf : sfsi->second) {
      if (sf->Matches(sfl)) {
        return sf;
      }
    }
  }
  return nullptr;
}

cmSourceFile* cmMakefile::CreateSource(const std::string& sourceName,
                                       bool generated,
                                       cmSourceFileLocationKind kind)
{
  auto sf = cm::make_unique<cmSourceFile>(this, sourceName, generated, kind);
  auto name =
    this->GetCMakeInstance()->StripExtension(sf->GetLocation().GetName());
#if defined(_WIN32) || defined(__APPLE__)
  name = cmSystemTools::LowerCase(name);
#endif
  this->SourceFileSearchIndex[name].push_back(sf.get());
  // for "Known" paths add direct lookup (used for faster lookup in GetSource)
  if (kind == cmSourceFileLocationKind::Known) {
    this->KnownFileSearchIndex[sourceName] = sf.get();
  }

  this->SourceFiles.push_back(std::move(sf));

  return this->SourceFiles.back().get();
}

cmSourceFile* cmMakefile::GetOrCreateSource(const std::string& sourceName,
                                            bool generated,
                                            cmSourceFileLocationKind kind)
{
  if (cmSourceFile* esf = this->GetSource(sourceName, kind)) {
    return esf;
  }
  return this->CreateSource(sourceName, generated, kind);
}

cmSourceFile* cmMakefile::GetOrCreateGeneratedSource(
  const std::string& sourceName)
{
  cmSourceFile* sf =
    this->GetOrCreateSource(sourceName, true, cmSourceFileLocationKind::Known);
  sf->MarkAsGenerated(); // In case we did not create the source file.
  return sf;
}

void cmMakefile::CreateGeneratedOutputs(
  const std::vector<std::string>& outputs)
{
  for (std::string const& o : outputs) {
    if (cmGeneratorExpression::Find(o) == std::string::npos) {
      this->GetOrCreateGeneratedSource(o);
    }
  }
}

void cmMakefile::AddTargetObject(std::string const& tgtName,
                                 std::string const& objFile)
{
  cmSourceFile* sf =
    this->GetOrCreateSource(objFile, true, cmSourceFileLocationKind::Known);
  sf->SetObjectLibrary(tgtName);
  sf->SetProperty("EXTERNAL_OBJECT", "1");
#if !defined(CMAKE_BOOTSTRAP)
  this->SourceGroups[this->ObjectLibrariesSourceGroupIndex].AddGroupFile(
    sf->ResolveFullPath());
#endif
}

void cmMakefile::EnableLanguage(std::vector<std::string> const& lang,
                                bool optional)
{
  if (this->DeferRunning) {
    this->IssueMessage(
      MessageType::FATAL_ERROR,
      "Languages may not be enabled during deferred execution.");
    return;
  }
  if (const char* def = this->GetGlobalGenerator()->GetCMakeCFGIntDir()) {
    this->AddDefinition("CMAKE_CFG_INTDIR", def);
  }
  // If RC is explicitly listed we need to do it after other languages.
  // On some platforms we enable RC implicitly while enabling others.
  // Do not let that look like recursive enable_language(RC).
  std::vector<std::string> langs;
  std::vector<std::string> langsRC;
  langs.reserve(lang.size());
  for (std::string const& i : lang) {
    if (i == "RC") {
      langsRC.push_back(i);
    } else {
      langs.push_back(i);
    }
  }
  if (!langs.empty()) {
    this->GetGlobalGenerator()->EnableLanguage(langs, this, optional);
  }
  if (!langsRC.empty()) {
    this->GetGlobalGenerator()->EnableLanguage(langsRC, this, optional);
  }
}

int cmMakefile::TryCompile(const std::string& srcdir,
                           const std::string& bindir,
                           const std::string& projectName,
                           const std::string& targetName, bool fast, int jobs,
                           const std::vector<std::string>* cmakeArgs,
                           std::string& output)
{
  this->IsSourceFileTryCompile = fast;
  // does the binary directory exist ? If not create it...
  if (!cmSystemTools::FileIsDirectory(bindir)) {
    cmSystemTools::MakeDirectory(bindir);
  }

  // change to the tests directory and run cmake
  // use the cmake object instead of calling cmake
  cmWorkingDirectory workdir(bindir);
  if (workdir.Failed()) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "Failed to set working directory to " + bindir + " : " +
                         std::strerror(workdir.GetLastResult()));
    cmSystemTools::SetFatalErrorOccurred();
    this->IsSourceFileTryCompile = false;
    return 1;
  }

  // make sure the same generator is used
  // use this program as the cmake to be run, it should not
  // be run that way but the cmake object requires a vailid path
  cmake cm(cmake::RoleProject, cmState::Project,
           cmState::ProjectKind::TryCompile);
  auto gg = cm.CreateGlobalGenerator(this->GetGlobalGenerator()->GetName());
  if (!gg) {
    this->IssueMessage(MessageType::INTERNAL_ERROR,
                       "Global generator '" +
                         this->GetGlobalGenerator()->GetName() +
                         "' could not be created.");
    cmSystemTools::SetFatalErrorOccurred();
    this->IsSourceFileTryCompile = false;
    return 1;
  }
  gg->RecursionDepth = this->RecursionDepth;
  cm.SetGlobalGenerator(std::move(gg));

  // do a configure
  cm.SetHomeDirectory(srcdir);
  cm.SetHomeOutputDirectory(bindir);
  cm.SetGeneratorInstance(this->GetSafeDefinition("CMAKE_GENERATOR_INSTANCE"));
  cm.SetGeneratorPlatform(this->GetSafeDefinition("CMAKE_GENERATOR_PLATFORM"));
  cm.SetGeneratorToolset(this->GetSafeDefinition("CMAKE_GENERATOR_TOOLSET"));
  cm.LoadCache();
  if (!cm.GetGlobalGenerator()->IsMultiConfig()) {
    if (cmValue config =
          this->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION")) {
      // Tell the single-configuration generator which one to use.
      // Add this before the user-provided CMake arguments in case
      // one of the arguments is -DCMAKE_BUILD_TYPE=...
      cm.AddCacheEntry("CMAKE_BUILD_TYPE", config, "Build configuration",
                       cmStateEnums::STRING);
    }
  }
  cmValue recursionDepth =
    this->GetDefinition("CMAKE_MAXIMUM_RECURSION_DEPTH");
  if (recursionDepth) {
    cm.AddCacheEntry("CMAKE_MAXIMUM_RECURSION_DEPTH", recursionDepth,
                     "Maximum recursion depth", cmStateEnums::STRING);
  }
  // if cmake args were provided then pass them in
  if (cmakeArgs) {
    // FIXME: Workaround to ignore unused CLI variables in try-compile.
    //
    // Ideally we should use SetArgs for options like --no-warn-unused-cli.
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
    // unused arguments and ignoring options such as --no-warn-unused-cli.
    cm.SetWarnUnusedCli(false);
    // cm.SetArgs(*cmakeArgs, true);

    cm.SetCacheArgs(*cmakeArgs);
  }
  // to save time we pass the EnableLanguage info directly
  cm.GetGlobalGenerator()->EnableLanguagesFromGenerator(
    this->GetGlobalGenerator(), this);
  if (this->IsOn("CMAKE_SUPPRESS_DEVELOPER_WARNINGS")) {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "TRUE", "",
                     cmStateEnums::INTERNAL);
  } else {
    cm.AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "FALSE", "",
                     cmStateEnums::INTERNAL);
  }
  if (cm.Configure() != 0) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "Failed to configure test project build system.");
    cmSystemTools::SetFatalErrorOccurred();
    this->IsSourceFileTryCompile = false;
    return 1;
  }

  if (cm.Generate() != 0) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "Failed to generate test project build system.");
    cmSystemTools::SetFatalErrorOccurred();
    this->IsSourceFileTryCompile = false;
    return 1;
  }

  // finally call the generator to actually build the resulting project
  int ret = this->GetGlobalGenerator()->TryCompile(
    jobs, srcdir, bindir, projectName, targetName, fast, output, this);

  this->IsSourceFileTryCompile = false;
  return ret;
}

bool cmMakefile::GetIsSourceFileTryCompile() const
{
  return this->IsSourceFileTryCompile;
}

cmake* cmMakefile::GetCMakeInstance() const
{
  return this->GlobalGenerator->GetCMakeInstance();
}

cmMessenger* cmMakefile::GetMessenger() const
{
  return this->GetCMakeInstance()->GetMessenger();
}

cmGlobalGenerator* cmMakefile::GetGlobalGenerator() const
{
  return this->GlobalGenerator;
}

#ifndef CMAKE_BOOTSTRAP
cmVariableWatch* cmMakefile::GetVariableWatch() const
{
  if (this->GetCMakeInstance() &&
      this->GetCMakeInstance()->GetVariableWatch()) {
    return this->GetCMakeInstance()->GetVariableWatch();
  }
  return nullptr;
}
#endif

cmState* cmMakefile::GetState() const
{
  return this->GetCMakeInstance()->GetState();
}

void cmMakefile::DisplayStatus(const std::string& message, float s) const
{
  cmake* cm = this->GetCMakeInstance();
  if (cm->GetWorkingMode() == cmake::FIND_PACKAGE_MODE) {
    // don't output any STATUS message in FIND_PACKAGE_MODE, since they will
    // directly be fed to the compiler, which will be confused.
    return;
  }
  cm->UpdateProgress(message, s);
}

std::string cmMakefile::GetModulesFile(const std::string& filename,
                                       bool& system, bool debug,
                                       std::string& debugBuffer) const
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
  cmValue cmakeModulePath = this->GetDefinition("CMAKE_MODULE_PATH");
  if (cmakeModulePath) {
    std::vector<std::string> modulePath = cmExpandedList(*cmakeModulePath);

    // Look through the possible module directories.
    for (std::string itempl : modulePath) {
      cmSystemTools::ConvertToUnixSlashes(itempl);
      itempl += "/";
      itempl += filename;
      if (cmSystemTools::FileExists(itempl)) {
        moduleInCMakeModulePath = itempl;
        break;
      }
      if (debug) {
        debugBuffer = cmStrCat(debugBuffer, "  ", itempl, "\n");
      }
    }
  }

  // Always search in the standard modules location.
  moduleInCMakeRoot =
    cmStrCat(cmSystemTools::GetCMakeRoot(), "/Modules/", filename);
  cmSystemTools::ConvertToUnixSlashes(moduleInCMakeRoot);
  if (!cmSystemTools::FileExists(moduleInCMakeRoot)) {
    if (debug) {
      debugBuffer = cmStrCat(debugBuffer, "  ", moduleInCMakeRoot, "\n");
    }
    moduleInCMakeRoot.clear();
  }

  // Normally, prefer the files found in CMAKE_MODULE_PATH. Only when the file
  // from which we are being called is located itself in CMAKE_ROOT, then
  // prefer results from CMAKE_ROOT depending on the policy setting.
  system = false;
  result = moduleInCMakeModulePath;
  if (result.empty()) {
    system = true;
    result = moduleInCMakeRoot;
  }

  if (!moduleInCMakeModulePath.empty() && !moduleInCMakeRoot.empty()) {
    cmValue currentFile = this->GetDefinition("CMAKE_CURRENT_LIST_FILE");
    std::string mods = cmSystemTools::GetCMakeRoot() + "/Modules/";
    if (currentFile && cmSystemTools::IsSubDirectory(*currentFile, mods)) {
      switch (this->GetPolicyStatus(cmPolicies::CMP0017)) {
        case cmPolicies::WARN: {
          std::ostringstream e;
          /* clang-format off */
          e << "File " << *currentFile << " includes "
            << moduleInCMakeModulePath
            << " (found via CMAKE_MODULE_PATH) which shadows "
            << moduleInCMakeRoot  << ". This may cause errors later on .\n"
            << cmPolicies::GetPolicyWarning(cmPolicies::CMP0017);
          /* clang-format on */

          this->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
          CM_FALLTHROUGH;
        }
        case cmPolicies::OLD:
          system = false;
          result = moduleInCMakeModulePath;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          system = true;
          result = moduleInCMakeRoot;
          break;
      }
    }
  }

  return result;
}

void cmMakefile::ConfigureString(const std::string& input, std::string& output,
                                 bool atOnly, bool escapeQuotes) const
{
  // Split input to handle one line at a time.
  std::string::const_iterator lineStart = input.begin();
  while (lineStart != input.end()) {
    // Find the end of this line.
    std::string::const_iterator lineEnd = lineStart;
    while (lineEnd != input.end() && *lineEnd != '\n') {
      ++lineEnd;
    }

    // Copy the line.
    std::string line(lineStart, lineEnd);

    // Skip the newline character.
    bool haveNewline = (lineEnd != input.end());
    if (haveNewline) {
      ++lineEnd;
    }

    // Replace #cmakedefine instances.
    if (this->cmDefineRegex.find(line)) {
      cmValue def = this->GetDefinition(this->cmDefineRegex.match(2));
      if (!cmIsOff(def)) {
        const std::string indentation = this->cmDefineRegex.match(1);
        cmSystemTools::ReplaceString(line, "#" + indentation + "cmakedefine",
                                     "#" + indentation + "define");
        output += line;
      } else {
        output += "/* #undef ";
        output += this->cmDefineRegex.match(2);
        output += " */";
      }
    } else if (this->cmDefine01Regex.find(line)) {
      const std::string indentation = this->cmDefine01Regex.match(1);
      cmValue def = this->GetDefinition(this->cmDefine01Regex.match(2));
      cmSystemTools::ReplaceString(line, "#" + indentation + "cmakedefine01",
                                   "#" + indentation + "define");
      output += line;
      if (!cmIsOff(def)) {
        output += " 1";
      } else {
        output += " 0";
      }
    } else {
      output += line;
    }

    if (haveNewline) {
      output += "\n";
    }

    // Move to the next line.
    lineStart = lineEnd;
  }

  // Perform variable replacements.
  const char* filename = nullptr;
  long lineNumber = -1;
  if (!this->Backtrace.Empty()) {
    const auto& currentTrace = this->Backtrace.Top();
    filename = currentTrace.FilePath.c_str();
    lineNumber = currentTrace.Line;
  }
  this->ExpandVariablesInString(output, escapeQuotes, true, atOnly, filename,
                                lineNumber, true, true);
}

int cmMakefile::ConfigureFile(const std::string& infile,
                              const std::string& outfile, bool copyonly,
                              bool atOnly, bool escapeQuotes,
                              mode_t permissions, cmNewLineStyle newLine)
{
  int res = 1;
  if (!this->CanIWriteThisFile(outfile)) {
    cmSystemTools::Error("Attempt to write file: " + outfile +
                         " into a source directory.");
    return 0;
  }
  if (!cmSystemTools::FileExists(infile)) {
    cmSystemTools::Error("File " + infile + " does not exist.");
    return 0;
  }
  std::string soutfile = outfile;
  const std::string& sinfile = infile;
  this->AddCMakeDependFile(sinfile);
  cmSystemTools::ConvertToUnixSlashes(soutfile);

  // Re-generate if non-temporary outputs are missing.
  // when we finalize the configuration we will remove all
  // output files that now don't exist.
  this->AddCMakeOutputFile(soutfile);

  if (permissions == 0) {
    cmSystemTools::GetPermissions(sinfile, permissions);
  }

  std::string::size_type pos = soutfile.rfind('/');
  if (pos != std::string::npos) {
    std::string path = soutfile.substr(0, pos);
    cmSystemTools::MakeDirectory(path);
  }

  if (copyonly) {
    if (!cmSystemTools::CopyFileIfDifferent(sinfile, soutfile)) {
      this->IssueMessage(MessageType::FATAL_ERROR,
                         cmSystemTools::GetLastSystemError());
      return 0;
    }
    if (!cmSystemTools::SetPermissions(soutfile, permissions)) {
      this->IssueMessage(MessageType::FATAL_ERROR,
                         cmSystemTools::GetLastSystemError());
      return 0;
    }
  } else {
    std::string newLineCharacters;
    std::ios::openmode omode = std::ios::out | std::ios::trunc;
    if (newLine.IsValid()) {
      newLineCharacters = newLine.GetCharacters();
      omode |= std::ios::binary;
    } else {
      newLineCharacters = "\n";
    }
    std::string tempOutputFile = cmStrCat(soutfile, ".tmp");
    cmsys::ofstream fout(tempOutputFile.c_str(), omode);
    if (!fout) {
      cmSystemTools::Error("Could not open file for write in copy operation " +
                           tempOutputFile);
      cmSystemTools::ReportLastSystemError("");
      return 0;
    }
    cmsys::ifstream fin(sinfile.c_str());
    if (!fin) {
      cmSystemTools::Error("Could not open file for read in copy operation " +
                           sinfile);
      return 0;
    }

    cmsys::FStream::BOM bom = cmsys::FStream::ReadBOM(fin);
    if (bom != cmsys::FStream::BOM_None && bom != cmsys::FStream::BOM_UTF8) {
      std::ostringstream e;
      e << "File starts with a Byte-Order-Mark that is not UTF-8:\n  "
        << sinfile;
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return 0;
    }
    // rewind to copy BOM to output file
    fin.seekg(0);

    // now copy input to output and expand variables in the
    // input file at the same time
    std::string inLine;
    std::string outLine;
    while (cmSystemTools::GetLineFromStream(fin, inLine)) {
      outLine.clear();
      this->ConfigureString(inLine, outLine, atOnly, escapeQuotes);
      fout << outLine << newLineCharacters;
    }
    // close the files before attempting to copy
    fin.close();
    fout.close();
    if (!cmSystemTools::CopyFileIfDifferent(tempOutputFile, soutfile)) {
      this->IssueMessage(MessageType::FATAL_ERROR,
                         cmSystemTools::GetLastSystemError());
      res = 0;
    } else {
      if (!cmSystemTools::SetPermissions(soutfile, permissions)) {
        this->IssueMessage(MessageType::FATAL_ERROR,
                           cmSystemTools::GetLastSystemError());
        res = 0;
      }
    }
    cmSystemTools::RemoveFile(tempOutputFile);
  }
  return res;
}

void cmMakefile::SetProperty(const std::string& prop, const char* value)
{
  this->StateSnapshot.GetDirectory().SetProperty(prop, value, this->Backtrace);
}
void cmMakefile::SetProperty(const std::string& prop, cmValue value)
{
  this->StateSnapshot.GetDirectory().SetProperty(prop, value, this->Backtrace);
}

void cmMakefile::AppendProperty(const std::string& prop,
                                const std::string& value, bool asString)
{
  this->StateSnapshot.GetDirectory().AppendProperty(prop, value, asString,
                                                    this->Backtrace);
}

cmValue cmMakefile::GetProperty(const std::string& prop) const
{
  // Check for computed properties.
  static std::string output;
  if (prop == "TESTS") {
    std::vector<std::string> keys;
    // get list of keys
    const auto* t = this;
    std::transform(
      t->Tests.begin(), t->Tests.end(), std::back_inserter(keys),
      [](decltype(t->Tests)::value_type const& pair) { return pair.first; });
    output = cmJoin(keys, ";");
    return cmValue(output);
  }

  return this->StateSnapshot.GetDirectory().GetProperty(prop);
}

cmValue cmMakefile::GetProperty(const std::string& prop, bool chain) const
{
  return this->StateSnapshot.GetDirectory().GetProperty(prop, chain);
}

bool cmMakefile::GetPropertyAsBool(const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

std::vector<std::string> cmMakefile::GetPropertyKeys() const
{
  return this->StateSnapshot.GetDirectory().GetPropertyKeys();
}

void cmMakefile::CheckProperty(const std::string& prop) const
{
  // Certain properties need checking.
  if (prop == "LINK_LIBRARIES") {
    if (cmValue value = this->GetProperty(prop)) {
      // Look for <LINK_LIBRARY:> internal pattern
      static cmsys::RegularExpression linkPattern(
        "(^|;)(</?LINK_(LIBRARY|GROUP):[^;>]*>)(;|$)");
      if (!linkPattern.find(value)) {
        return;
      }

      // Report an error.
      this->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Property ", prop, " contains the invalid item \"",
                 linkPattern.match(2), "\". The ", prop,
                 " property may contain the generator-expression \"$<LINK_",
                 linkPattern.match(3),
                 ":...>\" which may be used to specify how the libraries are "
                 "linked."));
    }
  }
}

cmTarget* cmMakefile::FindLocalNonAliasTarget(const std::string& name) const
{
  auto i = this->Targets.find(name);
  if (i != this->Targets.end()) {
    return &i->second;
  }
  return nullptr;
}

cmTest* cmMakefile::CreateTest(const std::string& testName)
{
  cmTest* test = this->GetTest(testName);
  if (test) {
    return test;
  }
  auto newTest = cm::make_unique<cmTest>(this);
  test = newTest.get();
  newTest->SetName(testName);
  this->Tests[testName] = std::move(newTest);
  return test;
}

cmTest* cmMakefile::GetTest(const std::string& testName) const
{
  auto mi = this->Tests.find(testName);
  if (mi != this->Tests.end()) {
    return mi->second.get();
  }
  return nullptr;
}

void cmMakefile::GetTests(const std::string& config,
                          std::vector<cmTest*>& tests) const
{
  for (const auto& generator : this->GetTestGenerators()) {
    if (generator->TestsForConfig(config)) {
      tests.push_back(generator->GetTest());
    }
  }
}

void cmMakefile::AddCMakeDependFilesFromUser()
{
  std::vector<std::string> deps;
  if (cmValue deps_str = this->GetProperty("CMAKE_CONFIGURE_DEPENDS")) {
    cmExpandList(*deps_str, deps);
  }
  for (std::string const& dep : deps) {
    if (cmSystemTools::FileIsFullPath(dep)) {
      this->AddCMakeDependFile(dep);
    } else {
      std::string f = cmStrCat(this->GetCurrentSourceDirectory(), '/', dep);
      this->AddCMakeDependFile(f);
    }
  }
}

std::string cmMakefile::FormatListFileStack() const
{
  std::vector<std::string> listFiles;
  cmStateSnapshot snp = this->StateSnapshot;
  while (snp.IsValid()) {
    listFiles.push_back(snp.GetExecutionListFile());
    snp = snp.GetCallStackParent();
  }
  std::reverse(listFiles.begin(), listFiles.end());
  std::ostringstream tmp;
  size_t depth = listFiles.size();
  if (depth > 0) {
    auto it = listFiles.end();
    do {
      if (depth != listFiles.size()) {
        tmp << "\n                ";
      }
      --it;
      tmp << "[";
      tmp << depth;
      tmp << "]\t";
      tmp << *it;
      depth--;
    } while (it != listFiles.begin());
  }
  return tmp.str();
}

void cmMakefile::PushScope()
{
  this->StateSnapshot =
    this->GetState()->CreateVariableScopeSnapshot(this->StateSnapshot);
  this->PushLoopBlockBarrier();

#if !defined(CMAKE_BOOTSTRAP)
  this->GetGlobalGenerator()->GetFileLockPool().PushFunctionScope();
#endif
}

void cmMakefile::PopScope()
{
#if !defined(CMAKE_BOOTSTRAP)
  this->GetGlobalGenerator()->GetFileLockPool().PopFunctionScope();
#endif

  this->PopLoopBlockBarrier();

  this->PopSnapshot();
}

void cmMakefile::RaiseScope(const std::string& var, const char* varDef)
{
  if (var.empty()) {
    return;
  }

  if (!this->StateSnapshot.RaiseScope(var, varDef)) {
    std::ostringstream m;
    m << "Cannot set \"" << var << "\": current scope has no parent.";
    this->IssueMessage(MessageType::AUTHOR_WARNING, m.str());
    return;
  }

#ifndef CMAKE_BOOTSTRAP
  cmVariableWatch* vv = this->GetVariableWatch();
  if (vv) {
    vv->VariableAccessed(var, cmVariableWatch::VARIABLE_MODIFIED_ACCESS,
                         varDef, this);
  }
#endif
}

cmTarget* cmMakefile::AddImportedTarget(const std::string& name,
                                        cmStateEnums::TargetType type,
                                        bool global)
{
  // Create the target.
  std::unique_ptr<cmTarget> target(
    new cmTarget(name, type,
                 global ? cmTarget::VisibilityImportedGlobally
                        : cmTarget::VisibilityImported,
                 this, cmTarget::PerConfig::Yes));

  // Add to the set of available imported targets.
  this->ImportedTargets[name] = target.get();
  this->GetGlobalGenerator()->IndexTarget(target.get());
  this->GetStateSnapshot().GetDirectory().AddImportedTargetName(name);

  // Transfer ownership to this cmMakefile object.
  this->ImportedTargetsOwned.push_back(std::move(target));
  return this->ImportedTargetsOwned.back().get();
}

cmTarget* cmMakefile::FindTargetToUse(const std::string& name,
                                      bool excludeAliases) const
{
  // Look for an imported target.  These take priority because they
  // are more local in scope and do not have to be globally unique.
  auto targetName = name;
  if (!excludeAliases) {
    // Look for local alias targets.
    auto alias = this->AliasTargets.find(name);
    if (alias != this->AliasTargets.end()) {
      targetName = alias->second;
    }
  }
  auto imported = this->ImportedTargets.find(targetName);
  if (imported != this->ImportedTargets.end()) {
    return imported->second;
  }

  // Look for a target built in this directory.
  if (cmTarget* t = this->FindLocalNonAliasTarget(name)) {
    return t;
  }

  // Look for a target built in this project.
  return this->GetGlobalGenerator()->FindTarget(name, excludeAliases);
}

bool cmMakefile::IsAlias(const std::string& name) const
{
  if (cm::contains(this->AliasTargets, name)) {
    return true;
  }
  return this->GetGlobalGenerator()->IsAlias(name);
}

bool cmMakefile::EnforceUniqueName(std::string const& name, std::string& msg,
                                   bool isCustom) const
{
  if (this->IsAlias(name)) {
    std::ostringstream e;
    e << "cannot create target \"" << name
      << "\" because an alias with the same name already exists.";
    msg = e.str();
    return false;
  }
  if (cmTarget* existing = this->FindTargetToUse(name)) {
    // The name given conflicts with an existing target.  Produce an
    // error in a compatible way.
    if (existing->IsImported()) {
      // Imported targets were not supported in previous versions.
      // This is new code, so we can make it an error.
      std::ostringstream e;
      e << "cannot create target \"" << name
        << "\" because an imported target with the same name already exists.";
      msg = e.str();
      return false;
    }
    // target names must be globally unique
    switch (this->GetPolicyStatus(cmPolicies::CMP0002)) {
      case cmPolicies::WARN:
        this->IssueMessage(MessageType::AUTHOR_WARNING,
                           cmPolicies::GetPolicyWarning(cmPolicies::CMP0002));
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        return true;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0002));
        return true;
      case cmPolicies::NEW:
        break;
    }

    // The conflict is with a non-imported target.
    // Allow this if the user has requested support.
    cmake* cm = this->GetCMakeInstance();
    if (isCustom && existing->GetType() == cmStateEnums::UTILITY &&
        this != existing->GetMakefile() &&
        cm->GetState()->GetGlobalPropertyAsBool(
          "ALLOW_DUPLICATE_CUSTOM_TARGETS")) {
      return true;
    }

    // Produce an error that tells the user how to work around the
    // problem.
    std::ostringstream e;
    e << "cannot create target \"" << name
      << "\" because another target with the same name already exists.  "
      << "The existing target is ";
    switch (existing->GetType()) {
      case cmStateEnums::EXECUTABLE:
        e << "an executable ";
        break;
      case cmStateEnums::STATIC_LIBRARY:
        e << "a static library ";
        break;
      case cmStateEnums::SHARED_LIBRARY:
        e << "a shared library ";
        break;
      case cmStateEnums::MODULE_LIBRARY:
        e << "a module library ";
        break;
      case cmStateEnums::UTILITY:
        e << "a custom target ";
        break;
      case cmStateEnums::INTERFACE_LIBRARY:
        e << "an interface library ";
        break;
      default:
        break;
    }
    e << "created in source directory \""
      << existing->GetMakefile()->GetCurrentSourceDirectory() << "\".  "
      << "See documentation for policy CMP0002 for more details.";
    msg = e.str();
    return false;
  }
  return true;
}

bool cmMakefile::EnforceUniqueDir(const std::string& srcPath,
                                  const std::string& binPath) const
{
  // Make sure the binary directory is unique.
  cmGlobalGenerator* gg = this->GetGlobalGenerator();
  if (gg->BinaryDirectoryIsNew(binPath)) {
    return true;
  }
  std::ostringstream e;
  switch (this->GetPolicyStatus(cmPolicies::CMP0013)) {
    case cmPolicies::WARN:
      // Print the warning.
      /* clang-format off */
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
      /* clang-format on */
      this->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
      CM_FALLTHROUGH;
    case cmPolicies::OLD:
      // OLD behavior does not warn.
      return true;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0013) << "\n";
      CM_FALLTHROUGH;
    case cmPolicies::NEW:
      // NEW behavior prints the error.
      /* clang-format off */
      e << "The binary directory\n"
        << "  " << binPath << "\n"
        << "is already used to build a source directory.  "
        << "It cannot be used to build source directory\n"
        << "  " << srcPath << "\n"
        << "Specify a unique binary directory name.";
      /* clang-format on */
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      break;
  }

  return false;
}

static std::string const matchVariables[] = {
  "CMAKE_MATCH_0", "CMAKE_MATCH_1", "CMAKE_MATCH_2", "CMAKE_MATCH_3",
  "CMAKE_MATCH_4", "CMAKE_MATCH_5", "CMAKE_MATCH_6", "CMAKE_MATCH_7",
  "CMAKE_MATCH_8", "CMAKE_MATCH_9"
};

static std::string const nMatchesVariable = "CMAKE_MATCH_COUNT";

void cmMakefile::ClearMatches()
{
  cmValue nMatchesStr = this->GetDefinition(nMatchesVariable);
  if (!nMatchesStr) {
    return;
  }
  int nMatches = atoi(nMatchesStr->c_str());
  for (int i = 0; i <= nMatches; i++) {
    std::string const& var = matchVariables[i];
    std::string const& s = this->GetSafeDefinition(var);
    if (!s.empty()) {
      this->AddDefinition(var, "");
      this->MarkVariableAsUsed(var);
    }
  }
  this->AddDefinition(nMatchesVariable, "0");
  this->MarkVariableAsUsed(nMatchesVariable);
}

void cmMakefile::StoreMatches(cmsys::RegularExpression& re)
{
  char highest = 0;
  for (int i = 0; i < 10; i++) {
    std::string const& m = re.match(i);
    if (!m.empty()) {
      std::string const& var = matchVariables[i];
      this->AddDefinition(var, m);
      this->MarkVariableAsUsed(var);
      highest = static_cast<char>('0' + i);
    }
  }
  char nMatches[] = { highest, '\0' };
  this->AddDefinition(nMatchesVariable, nMatches);
  this->MarkVariableAsUsed(nMatchesVariable);
}

cmStateSnapshot cmMakefile::GetStateSnapshot() const
{
  return this->StateSnapshot;
}

const char* cmMakefile::GetDefineFlagsCMP0059() const
{
  return this->DefineFlagsOrig.c_str();
}

cmPolicies::PolicyStatus cmMakefile::GetPolicyStatus(cmPolicies::PolicyID id,
                                                     bool parent_scope) const
{
  return this->StateSnapshot.GetPolicy(id, parent_scope);
}

bool cmMakefile::PolicyOptionalWarningEnabled(std::string const& var) const
{
  // Check for an explicit CMAKE_POLICY_WARNING_CMP<NNNN> setting.
  if (cmValue val = this->GetDefinition(var)) {
    return cmIsOn(val);
  }
  // Enable optional policy warnings with --debug-output, --trace,
  // or --trace-expand.
  cmake* cm = this->GetCMakeInstance();
  return cm->GetDebugOutput() || cm->GetTrace();
}

bool cmMakefile::SetPolicy(const char* id, cmPolicies::PolicyStatus status)
{
  cmPolicies::PolicyID pid;
  if (!cmPolicies::GetPolicyID(id, /* out */ pid)) {
    std::ostringstream e;
    e << "Policy \"" << id << "\" is not known to this version of CMake.";
    this->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return false;
  }
  return this->SetPolicy(pid, status);
}

bool cmMakefile::SetPolicy(cmPolicies::PolicyID id,
                           cmPolicies::PolicyStatus status)
{
  // A REQUIRED_ALWAYS policy may be set only to NEW.
  if (status != cmPolicies::NEW &&
      cmPolicies::GetPolicyStatus(id) == cmPolicies::REQUIRED_ALWAYS) {
    std::string msg = cmPolicies::GetRequiredAlwaysPolicyError(id);
    this->IssueMessage(MessageType::FATAL_ERROR, msg);
    return false;
  }

  // Deprecate old policies.
  if (status == cmPolicies::OLD && id <= cmPolicies::CMP0097 &&
      !(this->GetCMakeInstance()->GetIsInTryCompile() &&
        (
          // Policies set by cmCoreTryCompile::TryCompileCode.
          id == cmPolicies::CMP0065 || id == cmPolicies::CMP0083 ||
          id == cmPolicies::CMP0091)) &&
      (!this->IsSet("CMAKE_WARN_DEPRECATED") ||
       this->IsOn("CMAKE_WARN_DEPRECATED"))) {
    this->IssueMessage(MessageType::DEPRECATION_WARNING,
                       cmPolicies::GetPolicyDeprecatedWarning(id));
  }

  this->StateSnapshot.SetPolicy(id, status);
  return true;
}

cmMakefile::PolicyPushPop::PolicyPushPop(cmMakefile* m)
  : Makefile(m)
{
  this->Makefile->PushPolicy();
}

cmMakefile::PolicyPushPop::~PolicyPushPop()
{
  this->Makefile->PopPolicy();
}

void cmMakefile::PushPolicy(bool weak, cmPolicies::PolicyMap const& pm)
{
  this->StateSnapshot.PushPolicy(pm, weak);
}

void cmMakefile::PopPolicy()
{
  if (!this->StateSnapshot.PopPolicy()) {
    this->IssueMessage(MessageType::FATAL_ERROR,
                       "cmake_policy POP without matching PUSH");
  }
}

void cmMakefile::PopSnapshot(bool reportError)
{
  // cmStateSnapshot manages nested policy scopes within it.
  // Since the scope corresponding to the snapshot is closing,
  // reject any still-open nested policy scopes with an error.
  while (this->StateSnapshot.CanPopPolicyScope()) {
    if (reportError) {
      this->IssueMessage(MessageType::FATAL_ERROR,
                         "cmake_policy PUSH without matching POP");
      reportError = false;
    }
    this->PopPolicy();
  }

  this->StateSnapshot = this->GetState()->Pop(this->StateSnapshot);
  assert(this->StateSnapshot.IsValid());
}

bool cmMakefile::SetPolicyVersion(std::string const& version_min,
                                  std::string const& version_max)
{
  return cmPolicies::ApplyPolicyVersion(this, version_min, version_max,
                                        cmPolicies::WarnCompat::On);
}

bool cmMakefile::HasCMP0054AlreadyBeenReported(
  cmListFileContext const& context) const
{
  return !this->CMP0054ReportedIds.insert(context).second;
}

void cmMakefile::RecordPolicies(cmPolicies::PolicyMap& pm) const
{
  /* Record the setting of every policy.  */
  using PolicyID = cmPolicies::PolicyID;
  for (PolicyID pid = cmPolicies::CMP0000; pid != cmPolicies::CMPCOUNT;
       pid = static_cast<PolicyID>(pid + 1)) {
    pm.Set(pid, this->GetPolicyStatus(pid));
  }
}

bool cmMakefile::IgnoreErrorsCMP0061() const
{
  bool ignoreErrors = true;
  switch (this->GetPolicyStatus(cmPolicies::CMP0061)) {
    case cmPolicies::WARN:
      // No warning for this policy!
      CM_FALLTHROUGH;
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

cmMakefile::FunctionPushPop::FunctionPushPop(cmMakefile* mf,
                                             const std::string& fileName,
                                             cmPolicies::PolicyMap const& pm)
  : Makefile(mf)
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
  : Makefile(mf)
{
  this->Makefile->PushMacroScope(fileName, pm);
}

cmMakefile::MacroPushPop::~MacroPushPop()
{
  this->Makefile->PopMacroScope(this->ReportError);
}

cmMakefile::DebugFindPkgRAII::DebugFindPkgRAII(cmMakefile* mf,
                                               std::string const& pkg)
  : Makefile(mf)
  , OldValue(this->Makefile->DebugFindPkg)
{
  this->Makefile->DebugFindPkg =
    this->Makefile->GetCMakeInstance()->GetDebugFindPkgOutput(pkg);
}

cmMakefile::DebugFindPkgRAII::~DebugFindPkgRAII()
{
  this->Makefile->DebugFindPkg = this->OldValue;
}

bool cmMakefile::GetDebugFindPkgMode() const
{
  return this->DebugFindPkg;
}
