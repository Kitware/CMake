/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalNinjaGenerator.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <functional>
#include <sstream>
#include <utility>

#include <cm/iterator>
#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/memory>
#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>
#include <cm3p/json/writer.h>

#include "cmsys/FStream.hxx"

#include "cmCxxModuleMapper.h"
#include "cmDyndepCollation.h"
#include "cmFortranParser.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNinjaLinkLineComputer.h"
#include "cmOutputConverter.h"
#include "cmRange.h"
#include "cmScanDepFormat.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

const char* cmGlobalNinjaGenerator::NINJA_BUILD_FILE = "build.ninja";
const char* cmGlobalNinjaGenerator::NINJA_RULES_FILE =
  "CMakeFiles/rules.ninja";
const char* cmGlobalNinjaGenerator::INDENT = "  ";
#ifdef _WIN32
std::string const cmGlobalNinjaGenerator::SHELL_NOOP = "cd .";
#else
std::string const cmGlobalNinjaGenerator::SHELL_NOOP = ":";
#endif

namespace {
#ifdef _WIN32
bool DetectGCCOnWindows(cm::string_view compilerId, cm::string_view simulateId,
                        cm::string_view compilerFrontendVariant)
{
  return ((compilerId == "Clang"_s && compilerFrontendVariant == "GNU"_s) ||
          (simulateId != "MSVC"_s &&
           (compilerId == "GNU"_s || compilerId == "QCC"_s ||
            cmHasLiteralSuffix(compilerId, "Clang"))));
}
#endif
}

bool operator==(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return lhs.Target == rhs.Target && lhs.Config == rhs.Config &&
    lhs.GenexOutput == rhs.GenexOutput;
}

bool operator!=(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return !(lhs == rhs);
}

bool operator<(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return lhs.Target < rhs.Target ||
    (lhs.Target == rhs.Target &&
     (lhs.Config < rhs.Config ||
      (lhs.Config == rhs.Config && lhs.GenexOutput < rhs.GenexOutput)));
}

bool operator>(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return rhs < lhs;
}

bool operator<=(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return !(lhs > rhs);
}

bool operator>=(
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& lhs,
  const cmGlobalNinjaGenerator::ByConfig::TargetDependsClosureKey& rhs)
{
  return rhs <= lhs;
}

void cmGlobalNinjaGenerator::Indent(std::ostream& os, int count)
{
  for (int i = 0; i < count; ++i) {
    os << cmGlobalNinjaGenerator::INDENT;
  }
}

void cmGlobalNinjaGenerator::WriteDivider(std::ostream& os)
{
  os << "# ======================================"
        "=======================================\n";
}

void cmGlobalNinjaGenerator::WriteComment(std::ostream& os,
                                          const std::string& comment)
{
  if (comment.empty()) {
    return;
  }

  std::string::size_type lpos = 0;
  std::string::size_type rpos;
  os << "\n#############################################\n";
  while ((rpos = comment.find('\n', lpos)) != std::string::npos) {
    os << "# " << comment.substr(lpos, rpos - lpos) << "\n";
    lpos = rpos + 1;
  }
  os << "# " << comment.substr(lpos) << "\n\n";
}

std::unique_ptr<cmLinkLineComputer>
cmGlobalNinjaGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter,
  cmStateDirectory const& /* stateDir */) const
{
  return std::unique_ptr<cmLinkLineComputer>(
    cm::make_unique<cmNinjaLinkLineComputer>(
      outputConverter,
      this->LocalGenerators[0]->GetStateSnapshot().GetDirectory(), this));
}

std::string cmGlobalNinjaGenerator::EncodeRuleName(std::string const& name)
{
  // Ninja rule names must match "[a-zA-Z0-9_.-]+".  Use ".xx" to encode
  // "." and all invalid characters as hexadecimal.
  std::string encoded;
  for (char i : name) {
    if (isalnum(i) || i == '_' || i == '-') {
      encoded += i;
    } else {
      char buf[16];
      snprintf(buf, sizeof(buf), ".%02x", static_cast<unsigned int>(i));
      encoded += buf;
    }
  }
  return encoded;
}

std::string cmGlobalNinjaGenerator::EncodeLiteral(const std::string& lit)
{
  std::string result = lit;
  EncodeLiteralInplace(result);
  return result;
}

void cmGlobalNinjaGenerator::EncodeLiteralInplace(std::string& lit)
{
  cmSystemTools::ReplaceString(lit, "$", "$$");
  cmSystemTools::ReplaceString(lit, "\n", "$\n");
  if (this->IsMultiConfig()) {
    cmSystemTools::ReplaceString(lit, cmStrCat('$', this->GetCMakeCFGIntDir()),
                                 this->GetCMakeCFGIntDir());
  }
}

std::string cmGlobalNinjaGenerator::EncodePath(const std::string& path)
{
  std::string result = path;
#ifdef _WIN32
  if (this->IsGCCOnWindows())
    std::replace(result.begin(), result.end(), '\\', '/');
  else
    std::replace(result.begin(), result.end(), '/', '\\');
#endif
  this->EncodeLiteralInplace(result);
  cmSystemTools::ReplaceString(result, " ", "$ ");
  cmSystemTools::ReplaceString(result, ":", "$:");
  return result;
}

void cmGlobalNinjaGenerator::WriteBuild(std::ostream& os,
                                        cmNinjaBuild const& build,
                                        int cmdLineLimit,
                                        bool* usedResponseFile)
{
  // Make sure there is a rule.
  if (build.Rule.empty()) {
    cmSystemTools::Error(cmStrCat(
      "No rule for WriteBuild! called with comment: ", build.Comment));
    return;
  }

  // Make sure there is at least one output file.
  if (build.Outputs.empty()) {
    cmSystemTools::Error(cmStrCat(
      "No output files for WriteBuild! called with comment: ", build.Comment));
    return;
  }

  cmGlobalNinjaGenerator::WriteComment(os, build.Comment);

  // Write output files.
  std::string buildStr("build");
  {
    // Write explicit outputs
    for (std::string const& output : build.Outputs) {
      buildStr = cmStrCat(buildStr, ' ', this->EncodePath(output));
      if (this->ComputingUnknownDependencies) {
        this->CombinedBuildOutputs.insert(output);
      }
    }
    // Write implicit outputs
    if (!build.ImplicitOuts.empty()) {
      // Assume Ninja is new enough to support implicit outputs.
      // Callers should not populate this field otherwise.
      buildStr = cmStrCat(buildStr, " |");
      for (std::string const& implicitOut : build.ImplicitOuts) {
        buildStr = cmStrCat(buildStr, ' ', this->EncodePath(implicitOut));
        if (this->ComputingUnknownDependencies) {
          this->CombinedBuildOutputs.insert(implicitOut);
        }
      }
    }

    // Repeat some outputs, but expressed as absolute paths.
    // This helps Ninja handle absolute paths found in a depfile.
    // FIXME: Unfortunately this causes Ninja to stat the file twice.
    // We could avoid this if Ninja Issue 1251 were fixed.
    if (!build.WorkDirOuts.empty()) {
      if (this->SupportsImplicitOuts() && build.ImplicitOuts.empty()) {
        // Make them implicit outputs if supported by this version of Ninja.
        buildStr = cmStrCat(buildStr, " |");
      }
      for (std::string const& workdirOut : build.WorkDirOuts) {
        buildStr = cmStrCat(buildStr, " ${cmake_ninja_workdir}",
                            this->EncodePath(workdirOut));
      }
    }

    // Write the rule.
    buildStr = cmStrCat(buildStr, ": ", build.Rule);
  }

  std::string arguments;
  {
    // TODO: Better formatting for when there are multiple input/output files.

    // Write explicit dependencies.
    for (std::string const& explicitDep : build.ExplicitDeps) {
      arguments += cmStrCat(' ', this->EncodePath(explicitDep));
    }

    // Write implicit dependencies.
    if (!build.ImplicitDeps.empty()) {
      arguments += " |";
      for (std::string const& implicitDep : build.ImplicitDeps) {
        arguments += cmStrCat(' ', this->EncodePath(implicitDep));
      }
    }

    // Write order-only dependencies.
    if (!build.OrderOnlyDeps.empty()) {
      arguments += " ||";
      for (std::string const& orderOnlyDep : build.OrderOnlyDeps) {
        arguments += cmStrCat(' ', this->EncodePath(orderOnlyDep));
      }
    }

    arguments += '\n';
  }

  // Write the variables bound to this build statement.
  std::string assignments;
  {
    std::ostringstream variable_assignments;
    for (auto const& variable : build.Variables) {
      cmGlobalNinjaGenerator::WriteVariable(
        variable_assignments, variable.first, variable.second, "", 1);
    }

    // check if a response file rule should be used
    assignments = variable_assignments.str();
    bool useResponseFile = false;
    if (cmdLineLimit < 0 ||
        (cmdLineLimit > 0 &&
         (arguments.size() + buildStr.size() + assignments.size() + 1000) >
           static_cast<size_t>(cmdLineLimit))) {
      variable_assignments.str(std::string());
      cmGlobalNinjaGenerator::WriteVariable(variable_assignments, "RSP_FILE",
                                            build.RspFile, "", 1);
      assignments += variable_assignments.str();
      useResponseFile = true;
    }
    if (usedResponseFile) {
      *usedResponseFile = useResponseFile;
    }
  }

  os << buildStr << arguments << assignments << "\n";
}

void cmGlobalNinjaGenerator::AddCustomCommandRule()
{
  cmNinjaRule rule("CUSTOM_COMMAND");
  rule.Command = "$COMMAND";
  rule.Description = "$DESC";
  rule.Comment = "Rule for running custom commands.";
  this->AddRule(rule);
}

void cmGlobalNinjaGenerator::CCOutputs::Add(
  std::vector<std::string> const& paths)
{
  for (std::string const& path : paths) {
    std::string out = this->GG->ConvertToNinjaPath(path);
    if (!cmSystemTools::FileIsFullPath(out)) {
      // This output is expressed as a relative path.  Repeat it,
      // but expressed as an absolute path for Ninja Issue 1251.
      this->WorkDirOuts.emplace_back(out);
      this->GG->SeenCustomCommandOutput(this->GG->ConvertToNinjaAbsPath(path));
    }
    this->GG->SeenCustomCommandOutput(out);
    this->ExplicitOuts.emplace_back(std::move(out));
  }
}

void cmGlobalNinjaGenerator::WriteCustomCommandBuild(
  std::string const& command, std::string const& description,
  std::string const& comment, std::string const& depfile,
  std::string const& job_pool, bool uses_terminal, bool restat,
  std::string const& config, CCOutputs outputs, cmNinjaDeps explicitDeps,
  cmNinjaDeps orderOnlyDeps)
{
  this->AddCustomCommandRule();

  if (this->ComputingUnknownDependencies) {
    // we need to track every dependency that comes in, since we are trying
    // to find dependencies that are side effects of build commands
    for (std::string const& dep : explicitDeps) {
      this->CombinedCustomCommandExplicitDependencies.insert(dep);
    }
  }

  {
    cmNinjaBuild build("CUSTOM_COMMAND");
    build.Comment = comment;
    build.Outputs = std::move(outputs.ExplicitOuts);
    build.WorkDirOuts = std::move(outputs.WorkDirOuts);
    build.ExplicitDeps = std::move(explicitDeps);
    build.OrderOnlyDeps = std::move(orderOnlyDeps);

    cmNinjaVars& vars = build.Variables;
    {
      std::string cmd = command; // NOLINT(*)
#ifdef _WIN32
      if (cmd.empty())
        // TODO Shouldn't an empty command be handled by ninja?
        cmd = "cmd.exe /c";
#endif
      vars["COMMAND"] = std::move(cmd);
    }
    vars["DESC"] = this->EncodeLiteral(description);
    if (restat) {
      vars["restat"] = "1";
    }
    if (uses_terminal && this->SupportsDirectConsole()) {
      vars["pool"] = "console";
    } else if (!job_pool.empty()) {
      vars["pool"] = job_pool;
    }
    if (!depfile.empty()) {
      vars["depfile"] = depfile;
    }
    if (config.empty()) {
      this->WriteBuild(*this->GetCommonFileStream(), build);
    } else {
      this->WriteBuild(*this->GetImplFileStream(config), build);
    }
  }
}

void cmGlobalNinjaGenerator::AddMacOSXContentRule()
{
  cmNinjaRule rule("COPY_OSX_CONTENT");
  rule.Command = cmStrCat(this->CMakeCmd(), " -E copy $in $out");
  rule.Description = "Copying OS X Content $out";
  rule.Comment = "Rule for copying OS X bundle content file.";
  this->AddRule(rule);
}

void cmGlobalNinjaGenerator::WriteMacOSXContentBuild(std::string input,
                                                     std::string output,
                                                     const std::string& config)
{
  this->AddMacOSXContentRule();
  {
    cmNinjaBuild build("COPY_OSX_CONTENT");
    build.Outputs.push_back(std::move(output));
    build.ExplicitDeps.push_back(std::move(input));
    this->WriteBuild(*this->GetImplFileStream(config), build);
  }
}

void cmGlobalNinjaGenerator::WriteRule(std::ostream& os,
                                       cmNinjaRule const& rule)
{
  // -- Parameter checks
  // Make sure the rule has a name.
  if (rule.Name.empty()) {
    cmSystemTools::Error(cmStrCat(
      "No name given for WriteRule! called with comment: ", rule.Comment));
    return;
  }

  // Make sure a command is given.
  if (rule.Command.empty()) {
    cmSystemTools::Error(cmStrCat(
      "No command given for WriteRule! called with comment: ", rule.Comment));
    return;
  }

  // Make sure response file content is given
  if (!rule.RspFile.empty() && rule.RspContent.empty()) {
    cmSystemTools::Error(
      cmStrCat("rspfile but no rspfile_content given for WriteRule! "
               "called with comment: ",
               rule.Comment));
    return;
  }

  // -- Write rule
  // Write rule intro
  cmGlobalNinjaGenerator::WriteComment(os, rule.Comment);
  os << "rule " << rule.Name << '\n';

  // Write rule key/value pairs
  auto writeKV = [&os](const char* key, std::string const& value) {
    if (!value.empty()) {
      cmGlobalNinjaGenerator::Indent(os, 1);
      os << key << " = " << value << '\n';
    }
  };

  writeKV("depfile", rule.DepFile);
  writeKV("deps", rule.DepType);
  writeKV("command", rule.Command);
  writeKV("description", rule.Description);
  if (!rule.RspFile.empty()) {
    writeKV("rspfile", rule.RspFile);
    writeKV("rspfile_content", rule.RspContent);
  }
  writeKV("restat", rule.Restat);
  if (rule.Generator) {
    writeKV("generator", "1");
  }

  // Finish rule
  os << '\n';
}

void cmGlobalNinjaGenerator::WriteVariable(std::ostream& os,
                                           const std::string& name,
                                           const std::string& value,
                                           const std::string& comment,
                                           int indent)
{
  // Make sure we have a name.
  if (name.empty()) {
    cmSystemTools::Error(cmStrCat("No name given for WriteVariable! called "
                                  "with comment: ",
                                  comment));
    return;
  }

  std::string val;
  static std::unordered_set<std::string> const variablesShouldNotBeTrimmed = {
    "CODE_CHECK", "LAUNCHER"
  };
  if (variablesShouldNotBeTrimmed.find(name) ==
      variablesShouldNotBeTrimmed.end()) {
    val = cmTrimWhitespace(value);
  } else {
    val = value;
  }

  // Do not add a variable if the value is empty.
  if (val.empty()) {
    return;
  }

  cmGlobalNinjaGenerator::WriteComment(os, comment);
  cmGlobalNinjaGenerator::Indent(os, indent);
  os << name << " = " << val << "\n";
}

void cmGlobalNinjaGenerator::WriteInclude(std::ostream& os,
                                          const std::string& filename,
                                          const std::string& comment)
{
  cmGlobalNinjaGenerator::WriteComment(os, comment);
  os << "include " << filename << "\n";
}

void cmGlobalNinjaGenerator::WriteDefault(std::ostream& os,
                                          const cmNinjaDeps& targets,
                                          const std::string& comment)
{
  cmGlobalNinjaGenerator::WriteComment(os, comment);
  os << "default";
  for (std::string const& target : targets) {
    os << " " << target;
  }
  os << "\n";
}

cmGlobalNinjaGenerator::cmGlobalNinjaGenerator(cmake* cm)
  : cmGlobalCommonGenerator(cm)
{
#ifdef _WIN32
  cm->GetState()->SetWindowsShell(true);
#endif
  this->FindMakeProgramFile = "CMakeNinjaFindMake.cmake";
}

// Virtual public methods.

std::unique_ptr<cmLocalGenerator> cmGlobalNinjaGenerator::CreateLocalGenerator(
  cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalNinjaGenerator>(this, mf));
}

codecvt::Encoding cmGlobalNinjaGenerator::GetMakefileEncoding() const
{
  return this->NinjaExpectedEncoding;
}

cmDocumentationEntry cmGlobalNinjaGenerator::GetDocumentation()
{
  return { cmGlobalNinjaGenerator::GetActualName(),
           "Generates build.ninja files." };
}

// Implemented in all cmGlobaleGenerator sub-classes.
// Used in:
//   Source/cmLocalGenerator.cxx
//   Source/cmake.cxx
void cmGlobalNinjaGenerator::Generate()
{
  // Check minimum Ninja version.
  if (cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                    RequiredNinjaVersion())) {
    std::ostringstream msg;
    msg << "The detected version of Ninja (" << this->NinjaVersion;
    msg << ") is less than the version of Ninja required by CMake (";
    msg << cmGlobalNinjaGenerator::RequiredNinjaVersion() << ").";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return;
  }
  this->InitOutputPathPrefix();
  if (!this->OpenBuildFileStreams()) {
    return;
  }
  if (!this->OpenRulesFileStream()) {
    return;
  }

  for (auto& it : this->Configs) {
    it.second.TargetDependsClosures.clear();
  }

  this->TargetAll = this->NinjaOutputPath("all");
  this->CMakeCacheFile = this->NinjaOutputPath("CMakeCache.txt");
  this->DiagnosedCxxModuleNinjaSupport = false;
  this->ClangTidyExportFixesDirs.clear();
  this->ClangTidyExportFixesFiles.clear();

  this->PolicyCMP0058 =
    this->LocalGenerators[0]->GetMakefile()->GetPolicyStatus(
      cmPolicies::CMP0058);
  this->ComputingUnknownDependencies =
    (this->PolicyCMP0058 == cmPolicies::OLD ||
     this->PolicyCMP0058 == cmPolicies::WARN);

  this->cmGlobalGenerator::Generate();

  this->WriteAssumedSourceDependencies();
  this->WriteTargetAliases(*this->GetCommonFileStream());
  this->WriteFolderTargets(*this->GetCommonFileStream());
  this->WriteUnknownExplicitDependencies(*this->GetCommonFileStream());
  this->WriteBuiltinTargets(*this->GetCommonFileStream());

  if (cmSystemTools::GetErrorOccurredFlag()) {
    this->RulesFileStream->setstate(std::ios::failbit);
    for (auto const& config : this->Makefiles[0]->GetGeneratorConfigs(
           cmMakefile::IncludeEmptyConfig)) {
      this->GetImplFileStream(config)->setstate(std::ios::failbit);
      this->GetConfigFileStream(config)->setstate(std::ios::failbit);
    }
    this->GetCommonFileStream()->setstate(std::ios::failbit);
  }

  this->CloseCompileCommandsStream();
  this->CloseRulesFileStream();
  this->CloseBuildFileStreams();

#ifdef _WIN32
  // Older ninja tools will not be able to update metadata on Windows
  // when we are re-generating inside an existing 'ninja' invocation
  // because the outer tool has the files open for write.
  if (this->NinjaSupportsMetadataOnRegeneration ||
      !this->GetCMakeInstance()->GetRegenerateDuringBuild())
#endif
  {
    this->CleanMetaData();
  }

  this->RemoveUnknownClangTidyExportFixesFiles();
}

void cmGlobalNinjaGenerator::CleanMetaData()
{
  auto run_ninja_tool = [this](std::vector<char const*> const& args) {
    std::vector<std::string> command;
    command.push_back(this->NinjaCommand);
    command.emplace_back("-C");
    command.emplace_back(this->GetCMakeInstance()->GetHomeOutputDirectory());
    command.emplace_back("-t");
    for (auto const& arg : args) {
      command.emplace_back(arg);
    }
    std::string error;
    if (!cmSystemTools::RunSingleCommand(command, nullptr, &error, nullptr,
                                         nullptr,
                                         cmSystemTools::OUTPUT_NONE)) {
      this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                             cmStrCat("Running\n '",
                                                      cmJoin(command, "' '"),
                                                      "'\n"
                                                      "failed with:\n ",
                                                      error));
      cmSystemTools::SetFatalErrorOccurred();
    }
  };

  // Can the tools below expect 'build.ninja' to be loadable?
  bool const expectBuildManifest =
    !this->IsMultiConfig() && this->OutputPathPrefix.empty();

  // Skip some ninja tools if they need 'build.ninja' but it is missing.
  bool const missingBuildManifest = expectBuildManifest &&
    this->NinjaSupportsUnconditionalRecompactTool &&
    !cmSystemTools::FileExists("build.ninja");

  // The `recompact` tool loads the manifest. As above, we don't have a single
  // `build.ninja` to load for this in Ninja-Multi. This may be relaxed in the
  // future pending further investigation into how Ninja works upstream
  // (ninja#1721).
  if (this->NinjaSupportsUnconditionalRecompactTool &&
      !this->GetCMakeInstance()->GetRegenerateDuringBuild() &&
      expectBuildManifest && !missingBuildManifest) {
    run_ninja_tool({ "recompact" });
  }
  if (this->NinjaSupportsRestatTool && this->OutputPathPrefix.empty()) {
    // XXX(ninja): We only list `build.ninja` entry files here because CMake
    // *always* rewrites these files on a reconfigure. If CMake ever gets
    // smarter about this, all CMake-time created/edited files listed as
    // outputs for the reconfigure build statement will need to be listed here.
    cmNinjaDeps outputs;
    this->AddRebuildManifestOutputs(outputs);
    std::vector<const char*> args;
    args.reserve(outputs.size() + 1);
    args.push_back("restat");
    for (auto const& output : outputs) {
      args.push_back(output.c_str());
    }
    run_ninja_tool(args);
  }
}

bool cmGlobalNinjaGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (!this->cmGlobalGenerator::FindMakeProgram(mf)) {
    return false;
  }
  if (cmValue ninjaCommand = mf->GetDefinition("CMAKE_MAKE_PROGRAM")) {
    this->NinjaCommand = *ninjaCommand;
    std::vector<std::string> command;
    command.push_back(this->NinjaCommand);
    command.emplace_back("--version");
    std::string version;
    std::string error;
    if (!cmSystemTools::RunSingleCommand(command, &version, &error, nullptr,
                                         nullptr,
                                         cmSystemTools::OUTPUT_NONE)) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       cmStrCat("Running\n '", cmJoin(command, "' '"),
                                "'\n"
                                "failed with:\n ",
                                error));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    this->NinjaVersion = cmTrimWhitespace(version);
    this->CheckNinjaFeatures();
  }
  return true;
}

void cmGlobalNinjaGenerator::CheckNinjaFeatures()
{
  this->NinjaSupportsConsolePool =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForConsolePool());
  this->NinjaSupportsImplicitOuts = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion,
    cmGlobalNinjaGenerator::RequiredNinjaVersionForImplicitOuts());
  this->NinjaSupportsManifestRestat =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForManifestRestat());
  this->NinjaSupportsMultilineDepfile =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForMultilineDepfile());
  this->NinjaSupportsDyndepsCxx =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForDyndepsCxx());
  this->NinjaSupportsDyndepsFortran =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForDyndepsFortran());
  if (!this->NinjaSupportsDyndepsFortran) {
    // The ninja version number is not new enough to have upstream support.
    // Our ninja branch adds ".dyndep-#" to its version number,
    // where '#' is a feature-specific version number.  Extract it.
    static std::string const k_DYNDEP_ = ".dyndep-";
    std::string::size_type pos = this->NinjaVersion.find(k_DYNDEP_);
    if (pos != std::string::npos) {
      const char* fv = &this->NinjaVersion[pos + k_DYNDEP_.size()];
      unsigned long dyndep = 0;
      cmStrToULong(fv, &dyndep);
      if (dyndep == 1) {
        this->NinjaSupportsDyndepsFortran = true;
      }
    }
  }
  this->NinjaSupportsUnconditionalRecompactTool =
    !cmSystemTools::VersionCompare(
      cmSystemTools::OP_LESS, this->NinjaVersion,
      RequiredNinjaVersionForUnconditionalRecompactTool());
  this->NinjaSupportsRestatTool =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForRestatTool());
  this->NinjaSupportsMultipleOutputs =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForMultipleOutputs());
  this->NinjaSupportsMetadataOnRegeneration = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion,
    RequiredNinjaVersionForMetadataOnRegeneration());
#ifdef _WIN32
  this->NinjaSupportsCodePage =
    !cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, this->NinjaVersion,
                                   RequiredNinjaVersionForCodePage());
  if (this->NinjaSupportsCodePage) {
    this->CheckNinjaCodePage();
  } else {
    this->NinjaExpectedEncoding = codecvt::ANSI;
  }
#endif
}

void cmGlobalNinjaGenerator::CheckNinjaCodePage()
{
  std::vector<std::string> command{ this->NinjaCommand, "-t", "wincodepage" };
  std::string output;
  std::string error;
  int result;
  if (!cmSystemTools::RunSingleCommand(command, &output, &error, &result,
                                       nullptr, cmSystemTools::OUTPUT_NONE)) {
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           cmStrCat("Running\n '",
                                                    cmJoin(command, "' '"),
                                                    "'\n"
                                                    "failed with:\n ",
                                                    error));
    cmSystemTools::SetFatalErrorOccurred();
  } else if (result == 0) {
    std::istringstream outputStream(output);
    std::string line;
    bool found = false;
    while (cmSystemTools::GetLineFromStream(outputStream, line)) {
      if (cmHasLiteralPrefix(line, "Build file encoding: ")) {
        cm::string_view lineView(line);
        cm::string_view encoding =
          lineView.substr(cmStrLen("Build file encoding: "));
        if (encoding == "UTF-8") {
          // Ninja expects UTF-8. We use that internally. No conversion needed.
          this->NinjaExpectedEncoding = codecvt::None;
        } else {
          this->NinjaExpectedEncoding = codecvt::ANSI;
        }
        found = true;
        break;
      }
    }
    if (!found) {
      this->GetCMakeInstance()->IssueMessage(
        MessageType::WARNING,
        "Could not determine Ninja's code page, defaulting to UTF-8");
      this->NinjaExpectedEncoding = codecvt::None;
    }
  } else {
    this->NinjaExpectedEncoding = codecvt::ANSI;
  }
}

bool cmGlobalNinjaGenerator::CheckLanguages(
  std::vector<std::string> const& languages, cmMakefile* mf) const
{
  if (cm::contains(languages, "Fortran")) {
    return this->CheckFortran(mf);
  }
  if (cm::contains(languages, "ISPC")) {
    return this->CheckISPC(mf);
  }
  if (cm::contains(languages, "Swift")) {
    const std::string architectures =
      mf->GetSafeDefinition("CMAKE_OSX_ARCHITECTURES");
    if (architectures.find_first_of(';') != std::string::npos) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       "multiple values for CMAKE_OSX_ARCHITECTURES not "
                       "supported with Swift");
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  }
  return true;
}

bool cmGlobalNinjaGenerator::CheckCxxModuleSupport()
{
  if (this->NinjaSupportsDyndepsCxx) {
    return true;
  }
  bool const diagnose = !this->DiagnosedCxxModuleNinjaSupport &&
    !this->CMakeInstance->GetIsInTryCompile();
  if (diagnose) {
    std::ostringstream e;
    /* clang-format off */
    e <<
      "The Ninja generator does not support C++20 modules "
      "using Ninja version \n"
      "  " << this->NinjaVersion << "\n"
      "due to lack of required features.  "
      "Ninja " << RequiredNinjaVersionForDyndepsCxx() <<
      " or higher is required."
      ;
    /* clang-format on */
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccurred();
  }
  return false;
}

bool cmGlobalNinjaGenerator::CheckFortran(cmMakefile* mf) const
{
  if (this->NinjaSupportsDyndepsFortran) {
    return true;
  }

  std::ostringstream e;
  /* clang-format off */
  e <<
    "The Ninja generator does not support Fortran using Ninja version\n"
    "  " << this->NinjaVersion << "\n"
    "due to lack of required features.  "
    "Ninja " << RequiredNinjaVersionForDyndepsFortran() <<
    " or higher is required."
    ;
  /* clang-format on */
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  cmSystemTools::SetFatalErrorOccurred();
  return false;
}

bool cmGlobalNinjaGenerator::CheckISPC(cmMakefile* mf) const
{
  if (this->NinjaSupportsMultipleOutputs) {
    return true;
  }

  std::ostringstream e;
  /* clang-format off */
  e <<
    "The Ninja generator does not support ISPC using Ninja version\n"
    "  " << this->NinjaVersion << "\n"
    "due to lack of required features.  "
    "Ninja " << RequiredNinjaVersionForMultipleOutputs() <<
    " or higher is required."
    ;
  /* clang-format on */
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  cmSystemTools::SetFatalErrorOccurred();
  return false;
}

void cmGlobalNinjaGenerator::EnableLanguage(
  std::vector<std::string> const& langs, cmMakefile* mf, bool optional)
{
  if (this->IsMultiConfig()) {
    mf->InitCMAKE_CONFIGURATION_TYPES("Debug;Release;RelWithDebInfo");
  }

  this->cmGlobalGenerator::EnableLanguage(langs, mf, optional);
  for (std::string const& l : langs) {
    if (l == "NONE") {
      continue;
    }
    this->ResolveLanguageCompiler(l, mf, optional);
#ifdef _WIN32
    std::string const& compilerId =
      mf->GetSafeDefinition(cmStrCat("CMAKE_", l, "_COMPILER_ID"));
    std::string const& simulateId =
      mf->GetSafeDefinition(cmStrCat("CMAKE_", l, "_SIMULATE_ID"));
    std::string const& compilerFrontendVariant = mf->GetSafeDefinition(
      cmStrCat("CMAKE_", l, "_COMPILER_FRONTEND_VARIANT"));
    if (DetectGCCOnWindows(compilerId, simulateId, compilerFrontendVariant)) {
      this->MarkAsGCCOnWindows();
    }
#endif
  }
}

// Implemented by:
//   cmGlobalUnixMakefileGenerator3
//   cmGlobalGhsMultiGenerator
//   cmGlobalVisualStudio10Generator
//   cmGlobalVisualStudio7Generator
//   cmGlobalXCodeGenerator
// Called by:
//   cmGlobalGenerator::Build()
std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalNinjaGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& /*projectName*/,
  const std::string& /*projectDir*/,
  std::vector<std::string> const& targetNames, const std::string& config,
  int jobs, bool verbose, const cmBuildOptions& /*buildOptions*/,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand;
  makeCommand.Add(this->SelectMakeProgram(makeProgram));

  if (verbose) {
    makeCommand.Add("-v");
  }

  if ((jobs != cmake::NO_BUILD_PARALLEL_LEVEL) &&
      (jobs != cmake::DEFAULT_BUILD_PARALLEL_LEVEL)) {
    makeCommand.Add("-j", std::to_string(jobs));
  }

  this->AppendNinjaFileArgument(makeCommand, config);

  makeCommand.Add(makeOptions.begin(), makeOptions.end());
  for (const auto& tname : targetNames) {
    if (!tname.empty()) {
      makeCommand.Add(tname);
    }
  }
  return { std::move(makeCommand) };
}

// Non-virtual public methods.

void cmGlobalNinjaGenerator::AddRule(cmNinjaRule const& rule)
{
  // Do not add the same rule twice.
  if (!this->Rules.insert(rule.Name).second) {
    return;
  }
  // Store command length
  this->RuleCmdLength[rule.Name] = static_cast<int>(rule.Command.size());
  // Write rule
  cmGlobalNinjaGenerator::WriteRule(*this->RulesFileStream, rule);
}

bool cmGlobalNinjaGenerator::HasRule(const std::string& name)
{
  return (this->Rules.find(name) != this->Rules.end());
}

// Private virtual overrides

void cmGlobalNinjaGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir = cmStrCat(gt->LocalGenerator->GetCurrentBinaryDirectory(),
                             '/', gt->LocalGenerator->GetTargetDirectory(gt),
                             '/', this->GetCMakeCFGIntDir(), '/');
  gt->ObjectDirectory = dir;
}

// Private methods

bool cmGlobalNinjaGenerator::OpenBuildFileStreams()
{
  if (!this->OpenFileStream(this->BuildFileStream,
                            cmGlobalNinjaGenerator::NINJA_BUILD_FILE)) {
    return false;
  }

  // Write a comment about this file.
  *this->BuildFileStream
    << "# This file contains all the build statements describing the\n"
    << "# compilation DAG.\n\n";

  return true;
}

bool cmGlobalNinjaGenerator::OpenFileStream(
  std::unique_ptr<cmGeneratedFileStream>& stream, const std::string& name)
{
  // Get a stream where to generate things.
  if (!stream) {
    // Compute Ninja's build file path.
    std::string path =
      cmStrCat(this->GetCMakeInstance()->GetHomeOutputDirectory(), '/', name);
    stream = cm::make_unique<cmGeneratedFileStream>(
      path, false, this->GetMakefileEncoding());
    if (!(*stream)) {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return false;
    }

    // Write the do not edit header.
    this->WriteDisclaimer(*stream);
  }

  return true;
}

cm::optional<std::set<std::string>> cmGlobalNinjaGenerator::ListSubsetWithAll(
  const std::set<std::string>& all, const std::set<std::string>& defaults,
  const std::vector<std::string>& items)
{
  std::set<std::string> result;

  for (auto const& item : items) {
    if (item == "all") {
      if (items.size() == 1) {
        result = defaults;
      } else {
        return cm::nullopt;
      }
    } else if (all.count(item)) {
      result.insert(item);
    } else {
      return cm::nullopt;
    }
  }

  return cm::make_optional(result);
}

void cmGlobalNinjaGenerator::CloseBuildFileStreams()
{
  if (this->BuildFileStream) {
    this->BuildFileStream.reset();
  } else {
    cmSystemTools::Error("Build file stream was not open.");
  }
}

bool cmGlobalNinjaGenerator::OpenRulesFileStream()
{
  if (!this->OpenFileStream(this->RulesFileStream,
                            cmGlobalNinjaGenerator::NINJA_RULES_FILE)) {
    return false;
  }

  // Write comment about this file.
  /* clang-format off */
  *this->RulesFileStream
    << "# This file contains all the rules used to get the outputs files\n"
    << "# built from the input files.\n"
    << "# It is included in the main '" << NINJA_BUILD_FILE << "'.\n\n"
    ;
  /* clang-format on */
  return true;
}

void cmGlobalNinjaGenerator::CloseRulesFileStream()
{
  if (this->RulesFileStream) {
    this->RulesFileStream.reset();
  } else {
    cmSystemTools::Error("Rules file stream was not open.");
  }
}

static void EnsureTrailingSlash(std::string& path)
{
  if (path.empty()) {
    return;
  }
  std::string::value_type last = path.back();
#ifdef _WIN32
  if (last != '\\') {
    path += '\\';
  }
#else
  if (last != '/') {
    path += '/';
  }
#endif
}

std::string const& cmGlobalNinjaGenerator::ConvertToNinjaPath(
  const std::string& path) const
{
  auto const f = this->ConvertToNinjaPathCache.find(path);
  if (f != this->ConvertToNinjaPathCache.end()) {
    return f->second;
  }

  std::string convPath =
    this->LocalGenerators[0]->MaybeRelativeToTopBinDir(path);
  convPath = this->NinjaOutputPath(convPath);
#ifdef _WIN32
  std::replace(convPath.begin(), convPath.end(), '/', '\\');
#endif
  return this->ConvertToNinjaPathCache.emplace(path, std::move(convPath))
    .first->second;
}

std::string cmGlobalNinjaGenerator::ConvertToNinjaAbsPath(
  std::string path) const
{
#ifdef _WIN32
  std::replace(path.begin(), path.end(), '/', '\\');
#endif
  return path;
}

void cmGlobalNinjaGenerator::AddAdditionalCleanFile(std::string fileName,
                                                    const std::string& config)
{
  this->Configs[config].AdditionalCleanFiles.emplace(std::move(fileName));
}

void cmGlobalNinjaGenerator::AddCXXCompileCommand(
  const std::string& commandLine, const std::string& sourceFile,
  const std::string& objPath)
{
  // Compute Ninja's build file path.
  std::string buildFileDir =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  if (!this->CompileCommandsStream) {
    std::string buildFilePath =
      cmStrCat(buildFileDir, "/compile_commands.json");
    if (this->ComputingUnknownDependencies) {
      this->CombinedBuildOutputs.insert(
        this->NinjaOutputPath("compile_commands.json"));
    }

    // Get a stream where to generate things.
    this->CompileCommandsStream =
      cm::make_unique<cmGeneratedFileStream>(buildFilePath);
    *this->CompileCommandsStream << "[\n";
  } else {
    *this->CompileCommandsStream << ",\n";
  }

  std::string sourceFileName = sourceFile;
  if (!cmSystemTools::FileIsFullPath(sourceFileName)) {
    sourceFileName = cmSystemTools::CollapseFullPath(
      sourceFileName, this->GetCMakeInstance()->GetHomeOutputDirectory());
  }

  /* clang-format off */
  *this->CompileCommandsStream << "{\n"
     << R"(  "directory": ")"
     << cmGlobalGenerator::EscapeJSON(buildFileDir) << "\",\n"
     << R"(  "command": ")"
     << cmGlobalGenerator::EscapeJSON(commandLine) << "\",\n"
     << R"(  "file": ")"
     << cmGlobalGenerator::EscapeJSON(sourceFileName) << "\",\n"
     << R"(  "output": ")"
     << cmGlobalGenerator::EscapeJSON(objPath) << "\"\n"
     << "}";
  /* clang-format on */
}

void cmGlobalNinjaGenerator::CloseCompileCommandsStream()
{
  if (this->CompileCommandsStream) {
    *this->CompileCommandsStream << "\n]";
    this->CompileCommandsStream.reset();
  }
}

void cmGlobalNinjaGenerator::WriteDisclaimer(std::ostream& os) const
{
  os << "# CMAKE generated file: DO NOT EDIT!\n"
     << "# Generated by \"" << this->GetName() << "\""
     << " Generator, CMake Version " << cmVersion::GetMajorVersion() << "."
     << cmVersion::GetMinorVersion() << "\n\n";
}

void cmGlobalNinjaGenerator::WriteAssumedSourceDependencies()
{
  for (auto const& asd : this->AssumedSourceDependencies) {
    CCOutputs outputs(this);
    outputs.ExplicitOuts.emplace_back(asd.first);
    cmNinjaDeps orderOnlyDeps;
    std::copy(asd.second.begin(), asd.second.end(),
              std::back_inserter(orderOnlyDeps));
    this->WriteCustomCommandBuild(
      /*command=*/"", /*description=*/"",
      "Assume dependencies for generated source file.",
      /*depfile*/ "", /*job_pool*/ "",
      /*uses_terminal*/ false,
      /*restat*/ true, std::string(), outputs, cmNinjaDeps(),
      std::move(orderOnlyDeps));
  }
}

std::string cmGlobalNinjaGenerator::OrderDependsTargetForTarget(
  cmGeneratorTarget const* target, const std::string& /*config*/) const
{
  return cmStrCat("cmake_object_order_depends_target_", target->GetName());
}

void cmGlobalNinjaGenerator::AppendTargetOutputs(
  cmGeneratorTarget const* target, cmNinjaDeps& outputs,
  const std::string& config, cmNinjaTargetDepends depends) const
{
  // for frameworks, we want the real name, not sample name
  // frameworks always appear versioned, and the build.ninja
  // will always attempt to manage symbolic links instead
  // of letting cmOSXBundleGenerator do it.
  bool realname = target->IsFrameworkOnApple();

  switch (target->GetType()) {
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY: {
      if (depends == DependOnTargetOrdering) {
        outputs.push_back(this->OrderDependsTargetForTarget(target, config));
        break;
      }
    }
      CM_FALLTHROUGH;
    case cmStateEnums::EXECUTABLE: {
      if (target->IsApple() && target->HasImportLibrary(config)) {
        outputs.push_back(this->ConvertToNinjaPath(target->GetFullPath(
          config, cmStateEnums::ImportLibraryArtifact, realname)));
      }
      outputs.push_back(this->ConvertToNinjaPath(target->GetFullPath(
        config, cmStateEnums::RuntimeBinaryArtifact, realname)));
      break;
    }
    case cmStateEnums::OBJECT_LIBRARY: {
      if (depends == DependOnTargetOrdering) {
        outputs.push_back(this->OrderDependsTargetForTarget(target, config));
        break;
      }
    }
      CM_FALLTHROUGH;
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::UTILITY: {
      std::string path =
        cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(), '/',
                 target->GetName());
      std::string output = this->ConvertToNinjaPath(path);
      if (target->Target->IsPerConfig()) {
        output = this->BuildAlias(output, config);
      }
      outputs.push_back(output);
      break;
    }

    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
  }
}

void cmGlobalNinjaGenerator::AppendTargetDepends(
  cmGeneratorTarget const* target, cmNinjaDeps& outputs,
  const std::string& config, const std::string& fileConfig,
  cmNinjaTargetDepends depends)
{
  if (target->GetType() == cmStateEnums::GLOBAL_TARGET) {
    // These depend only on other CMake-provided targets, e.g. "all".
    for (BT<std::pair<std::string, bool>> const& util :
         target->GetUtilities()) {
      std::string d =
        cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(), '/',
                 util.Value.first);
      outputs.push_back(this->BuildAlias(this->ConvertToNinjaPath(d), config));
    }
  } else {
    cmNinjaDeps outs;

    auto computeISPCOuputs = [](cmGlobalNinjaGenerator* gg,
                                cmGeneratorTarget const* depTarget,
                                cmNinjaDeps& outputDeps,
                                const std::string& targetConfig) {
      if (depTarget->CanCompileSources()) {
        auto headers = depTarget->GetGeneratedISPCHeaders(targetConfig);
        if (!headers.empty()) {
          std::transform(headers.begin(), headers.end(), headers.begin(),
                         gg->MapToNinjaPath());
          outputDeps.insert(outputDeps.end(), headers.begin(), headers.end());
        }
        auto objs = depTarget->GetGeneratedISPCObjects(targetConfig);
        if (!objs.empty()) {
          std::transform(objs.begin(), objs.end(), objs.begin(),
                         gg->MapToNinjaPath());
          outputDeps.insert(outputDeps.end(), objs.begin(), objs.end());
        }
      }
    };

    for (cmTargetDepend const& targetDep :
         this->GetTargetDirectDepends(target)) {
      if (!targetDep->IsInBuildSystem()) {
        continue;
      }
      if (targetDep.IsCross()) {
        this->AppendTargetOutputs(targetDep, outs, fileConfig, depends);
        computeISPCOuputs(this, targetDep, outs, fileConfig);
      } else {
        this->AppendTargetOutputs(targetDep, outs, config, depends);
        computeISPCOuputs(this, targetDep, outs, config);
      }
    }
    std::sort(outs.begin(), outs.end());
    cm::append(outputs, outs);
  }
}

void cmGlobalNinjaGenerator::AppendTargetDependsClosure(
  cmGeneratorTarget const* target, std::unordered_set<std::string>& outputs,
  const std::string& config, const std::string& fileConfig, bool genexOutput,
  bool omit_self)
{

  // try to locate the target in the cache
  ByConfig::TargetDependsClosureKey key{
    target,
    config,
    genexOutput,
  };
  auto find = this->Configs[fileConfig].TargetDependsClosures.lower_bound(key);

  if (find == this->Configs[fileConfig].TargetDependsClosures.end() ||
      find->first != key) {
    // We now calculate the closure outputs by inspecting the dependent
    // targets recursively.
    // For that we have to distinguish between a local result set that is only
    // relevant for filling the cache entries properly isolated and a global
    // result set that is relevant for the result of the top level call to
    // AppendTargetDependsClosure.
    std::unordered_set<std::string>
      this_outs; // this will be the new cache entry

    for (auto const& dep_target : this->GetTargetDirectDepends(target)) {
      if (!dep_target->IsInBuildSystem()) {
        continue;
      }

      if (!this->IsSingleConfigUtility(target) &&
          !this->IsSingleConfigUtility(dep_target) &&
          this->EnableCrossConfigBuild() && !dep_target.IsCross() &&
          !genexOutput) {
        continue;
      }

      if (dep_target.IsCross()) {
        this->AppendTargetDependsClosure(dep_target, this_outs, fileConfig,
                                         fileConfig, genexOutput, false);
      } else {
        this->AppendTargetDependsClosure(dep_target, this_outs, config,
                                         fileConfig, genexOutput, false);
      }
    }
    find = this->Configs[fileConfig].TargetDependsClosures.emplace_hint(
      find, key, std::move(this_outs));
  }

  // now fill the outputs of the final result from the newly generated cache
  // entry
  outputs.insert(find->second.begin(), find->second.end());

  // finally generate the outputs of the target itself, if applicable
  cmNinjaDeps outs;
  if (!omit_self) {
    this->AppendTargetOutputs(target, outs, config, DependOnTargetArtifact);
  }
  outputs.insert(outs.begin(), outs.end());
}

void cmGlobalNinjaGenerator::AddTargetAlias(const std::string& alias,
                                            cmGeneratorTarget* target,
                                            const std::string& config)
{
  std::string outputPath = this->NinjaOutputPath(alias);
  std::string buildAlias = this->BuildAlias(outputPath, config);
  cmNinjaDeps outputs;
  if (config != "all") {
    this->AppendTargetOutputs(target, outputs, config, DependOnTargetArtifact);
  }
  // Mark the target's outputs as ambiguous to ensure that no other target
  // uses the output as an alias.
  for (std::string const& output : outputs) {
    this->TargetAliases[output].GeneratorTarget = nullptr;
    this->DefaultTargetAliases[output].GeneratorTarget = nullptr;
    for (const std::string& config2 :
         this->Makefiles.front()->GetGeneratorConfigs(
           cmMakefile::IncludeEmptyConfig)) {
      this->Configs[config2].TargetAliases[output].GeneratorTarget = nullptr;
    }
  }

  // Insert the alias into the map.  If the alias was already present in the
  // map and referred to another target, mark it as ambiguous.
  TargetAlias ta;
  ta.GeneratorTarget = target;
  ta.Config = config;

  auto newAliasGlobal =
    this->TargetAliases.insert(std::make_pair(buildAlias, ta));
  if (newAliasGlobal.second &&
      newAliasGlobal.first->second.GeneratorTarget != target) {
    newAliasGlobal.first->second.GeneratorTarget = nullptr;
  }

  auto newAliasConfig =
    this->Configs[config].TargetAliases.insert(std::make_pair(outputPath, ta));
  if (newAliasConfig.second &&
      newAliasConfig.first->second.GeneratorTarget != target) {
    newAliasConfig.first->second.GeneratorTarget = nullptr;
  }
  if (this->DefaultConfigs.count(config)) {
    auto newAliasDefaultGlobal =
      this->DefaultTargetAliases.insert(std::make_pair(outputPath, ta));
    if (newAliasDefaultGlobal.second &&
        newAliasDefaultGlobal.first->second.GeneratorTarget != target) {
      newAliasDefaultGlobal.first->second.GeneratorTarget = nullptr;
    }
  }
}

void cmGlobalNinjaGenerator::WriteTargetAliases(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Target aliases.\n\n";

  cmNinjaBuild build("phony");
  build.Outputs.emplace_back();
  for (auto const& ta : this->TargetAliases) {
    // Don't write ambiguous aliases.
    if (!ta.second.GeneratorTarget) {
      continue;
    }

    // Don't write alias if there is a already a custom command with
    // matching output
    if (this->HasCustomCommandOutput(ta.first)) {
      continue;
    }

    build.Outputs.front() = ta.first;
    build.ExplicitDeps.clear();
    if (ta.second.Config == "all") {
      for (auto const& config : this->CrossConfigs) {
        this->AppendTargetOutputs(ta.second.GeneratorTarget,
                                  build.ExplicitDeps, config,
                                  DependOnTargetArtifact);
      }
    } else {
      this->AppendTargetOutputs(ta.second.GeneratorTarget, build.ExplicitDeps,
                                ta.second.Config, DependOnTargetArtifact);
    }
    this->WriteBuild(this->EnableCrossConfigBuild() &&
                         (ta.second.Config == "all" ||
                          this->CrossConfigs.count(ta.second.Config))
                       ? os
                       : *this->GetImplFileStream(ta.second.Config),
                     build);
  }

  if (this->IsMultiConfig()) {
    for (auto const& config : this->Makefiles.front()->GetGeneratorConfigs(
           cmMakefile::IncludeEmptyConfig)) {
      for (auto const& ta : this->Configs[config].TargetAliases) {
        // Don't write ambiguous aliases.
        if (!ta.second.GeneratorTarget) {
          continue;
        }

        // Don't write alias if there is a already a custom command with
        // matching output
        if (this->HasCustomCommandOutput(ta.first)) {
          continue;
        }

        build.Outputs.front() = ta.first;
        build.ExplicitDeps.clear();
        this->AppendTargetOutputs(ta.second.GeneratorTarget,
                                  build.ExplicitDeps, config,
                                  DependOnTargetArtifact);
        this->WriteBuild(*this->GetConfigFileStream(config), build);
      }
    }

    if (!this->DefaultConfigs.empty()) {
      for (auto const& ta : this->DefaultTargetAliases) {
        // Don't write ambiguous aliases.
        if (!ta.second.GeneratorTarget) {
          continue;
        }

        // Don't write alias if there is a already a custom command with
        // matching output
        if (this->HasCustomCommandOutput(ta.first)) {
          continue;
        }

        build.Outputs.front() = ta.first;
        build.ExplicitDeps.clear();
        for (auto const& config : this->DefaultConfigs) {
          this->AppendTargetOutputs(ta.second.GeneratorTarget,
                                    build.ExplicitDeps, config,
                                    DependOnTargetArtifact);
        }
        this->WriteBuild(*this->GetDefaultFileStream(), build);
      }
    }
  }
}

void cmGlobalNinjaGenerator::WriteFolderTargets(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Folder targets.\n\n";

  std::map<std::string, DirectoryTarget> dirTargets =
    this->ComputeDirectoryTargets();

  for (auto const& it : dirTargets) {
    cmNinjaBuild build("phony");
    cmGlobalNinjaGenerator::WriteDivider(os);
    std::string const& currentBinaryDir = it.first;
    DirectoryTarget const& dt = it.second;
    std::vector<std::string> configs =
      dt.LG->GetMakefile()->GetGeneratorConfigs(
        cmMakefile::IncludeEmptyConfig);

    // Setup target
    cmNinjaDeps configDeps;
    build.Comment = cmStrCat("Folder: ", currentBinaryDir);
    build.Outputs.emplace_back();
    std::string const buildDirAllTarget =
      this->ConvertToNinjaPath(cmStrCat(currentBinaryDir, "/all"));
    for (auto const& config : configs) {
      build.ExplicitDeps.clear();
      build.Outputs.front() = this->BuildAlias(buildDirAllTarget, config);
      configDeps.emplace_back(build.Outputs.front());
      for (DirectoryTarget::Target const& t : dt.Targets) {
        if (!this->IsExcludedFromAllInConfig(t, config)) {
          this->AppendTargetOutputs(t.GT, build.ExplicitDeps, config,
                                    DependOnTargetArtifact);
        }
      }
      for (DirectoryTarget::Dir const& d : dt.Children) {
        if (!d.ExcludeFromAll) {
          build.ExplicitDeps.emplace_back(this->BuildAlias(
            this->ConvertToNinjaPath(cmStrCat(d.Path, "/all")), config));
        }
      }
      // Write target
      this->WriteBuild(this->EnableCrossConfigBuild() &&
                           this->CrossConfigs.count(config)
                         ? os
                         : *this->GetImplFileStream(config),
                       build);
    }

    // Add shortcut target
    if (this->IsMultiConfig()) {
      for (auto const& config : configs) {
        build.ExplicitDeps = { this->BuildAlias(buildDirAllTarget, config) };
        build.Outputs.front() = buildDirAllTarget;
        this->WriteBuild(*this->GetConfigFileStream(config), build);
      }

      if (!this->DefaultFileConfig.empty()) {
        build.ExplicitDeps.clear();
        for (auto const& config : this->DefaultConfigs) {
          build.ExplicitDeps.push_back(
            this->BuildAlias(buildDirAllTarget, config));
        }
        build.Outputs.front() = buildDirAllTarget;
        this->WriteBuild(*this->GetDefaultFileStream(), build);
      }
    }

    // Add target for all configs
    if (this->EnableCrossConfigBuild()) {
      build.ExplicitDeps.clear();
      for (auto const& config : this->CrossConfigs) {
        build.ExplicitDeps.push_back(
          this->BuildAlias(buildDirAllTarget, config));
      }
      build.Outputs.front() = this->BuildAlias(buildDirAllTarget, "all");
      this->WriteBuild(os, build);
    }
  }
}

void cmGlobalNinjaGenerator::WriteUnknownExplicitDependencies(std::ostream& os)
{
  if (!this->ComputingUnknownDependencies) {
    return;
  }

  // We need to collect the set of known build outputs.
  // Start with those generated by WriteBuild calls.
  // No other method needs this so we can take ownership
  // of the set locally and throw it out when we are done.
  std::set<std::string> knownDependencies;
  knownDependencies.swap(this->CombinedBuildOutputs);

  // now write out the unknown explicit dependencies.

  // union the configured files, evaluations files and the
  // CombinedBuildOutputs,
  // and then difference with CombinedExplicitDependencies to find the explicit
  // dependencies that we have no rule for

  cmGlobalNinjaGenerator::WriteDivider(os);
  /* clang-format off */
  os << "# Unknown Build Time Dependencies.\n"
     << "# Tell Ninja that they may appear as side effects of build rules\n"
     << "# otherwise ordered by order-only dependencies.\n\n";
  /* clang-format on */

  // get the list of files that cmake itself has generated as a
  // product of configuration.

  for (const auto& lg : this->LocalGenerators) {
    // get the vector of files created by this makefile and convert them
    // to ninja paths, which are all relative in respect to the build directory
    for (std::string const& file : lg->GetMakefile()->GetOutputFiles()) {
      knownDependencies.insert(this->ConvertToNinjaPath(file));
    }
    if (!this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
      // get list files which are implicit dependencies as well and will be
      // phony for rebuild manifest
      for (std::string const& j : lg->GetMakefile()->GetListFiles()) {
        knownDependencies.insert(this->ConvertToNinjaPath(j));
      }
    }
    for (const auto& li : lg->GetMakefile()->GetEvaluationFiles()) {
      // get all the files created by generator expressions and convert them
      // to ninja paths
      for (std::string const& evaluationFile : li->GetFiles()) {
        knownDependencies.insert(this->ConvertToNinjaPath(evaluationFile));
      }
    }
  }
  knownDependencies.insert(this->CMakeCacheFile);

  for (auto const& ta : this->TargetAliases) {
    knownDependencies.insert(this->ConvertToNinjaPath(ta.first));
  }

  // remove all source files we know will exist.
  for (auto const& i : this->AssumedSourceDependencies) {
    knownDependencies.insert(this->ConvertToNinjaPath(i.first));
  }

  // now we difference with CombinedCustomCommandExplicitDependencies to find
  // the list of items we know nothing about.
  // We have encoded all the paths in CombinedCustomCommandExplicitDependencies
  // and knownDependencies so no matter if unix or windows paths they
  // should all match now.

  std::vector<std::string> unknownExplicitDepends;
  this->CombinedCustomCommandExplicitDependencies.erase(this->TargetAll);

  std::set_difference(this->CombinedCustomCommandExplicitDependencies.begin(),
                      this->CombinedCustomCommandExplicitDependencies.end(),
                      knownDependencies.begin(), knownDependencies.end(),
                      std::back_inserter(unknownExplicitDepends));

  std::vector<std::string> warnExplicitDepends;
  if (!unknownExplicitDepends.empty()) {
    cmake* cmk = this->GetCMakeInstance();
    std::string const& buildRoot = cmk->GetHomeOutputDirectory();
    bool const inSource = (buildRoot == cmk->GetHomeDirectory());
    bool const warn = (!inSource && (this->PolicyCMP0058 == cmPolicies::WARN));
    cmNinjaBuild build("phony");
    build.Outputs.emplace_back("");
    for (std::string const& ued : unknownExplicitDepends) {
      // verify the file is in the build directory
      std::string const absDepPath =
        cmSystemTools::CollapseFullPath(ued, buildRoot);
      if (cmSystemTools::IsSubDirectory(absDepPath, buildRoot)) {
        // Generate phony build statement
        build.Outputs[0] = ued;
        this->WriteBuild(os, build);
        // Add to warning on demand
        if (warn && warnExplicitDepends.size() < 10) {
          warnExplicitDepends.push_back(ued);
        }
      }
    }
  }

  if (!warnExplicitDepends.empty()) {
    std::ostringstream w;
    /* clang-format off */
    w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0058) << "\n"
      "This project specifies custom command DEPENDS on files "
      "in the build tree that are not specified as the OUTPUT or "
      "BYPRODUCTS of any add_custom_command or add_custom_target:\n"
      " " << cmJoin(warnExplicitDepends, "\n ") <<
      "\n"
      "For compatibility with versions of CMake that did not have "
      "the BYPRODUCTS option, CMake is generating phony rules for "
      "such files to convince 'ninja' to build."
      "\n"
      "Project authors should add the missing BYPRODUCTS or OUTPUT "
      "options to the custom commands that produce these files."
      ;
    /* clang-format on */
    this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                           w.str());
  }
}

void cmGlobalNinjaGenerator::WriteBuiltinTargets(std::ostream& os)
{
  // Write headers.
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Built-in targets\n\n";

  this->WriteTargetRebuildManifest(os);
  this->WriteTargetClean(os);
  this->WriteTargetHelp(os);

  for (auto const& config : this->Makefiles[0]->GetGeneratorConfigs(
         cmMakefile::IncludeEmptyConfig)) {
    this->WriteTargetDefault(*this->GetConfigFileStream(config));
  }

  if (!this->DefaultFileConfig.empty()) {
    this->WriteTargetDefault(*this->GetDefaultFileStream());
  }
}

void cmGlobalNinjaGenerator::WriteTargetDefault(std::ostream& os)
{
  if (!this->HasOutputPathPrefix()) {
    cmNinjaDeps all;
    all.push_back(this->TargetAll);
    cmGlobalNinjaGenerator::WriteDefault(os, all,
                                         "Make the all target the default.");
  }
}

void cmGlobalNinjaGenerator::WriteTargetRebuildManifest(std::ostream& os)
{
  if (this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    return;
  }

  cmake* cm = this->GetCMakeInstance();
  const auto& lg = this->LocalGenerators[0];

  {
    cmNinjaRule rule("RERUN_CMAKE");
    rule.Command = cmStrCat(
      this->CMakeCmd(), " --regenerate-during-build",
      cm->GetIgnoreWarningAsError() ? " --compile-no-warning-as-error" : "",
      " -S",
      lg->ConvertToOutputFormat(lg->GetSourceDirectory(),
                                cmOutputConverter::SHELL),
      " -B",
      lg->ConvertToOutputFormat(lg->GetBinaryDirectory(),
                                cmOutputConverter::SHELL));
    rule.Description = "Re-running CMake...";
    rule.Comment = "Rule for re-running cmake.";
    rule.Generator = true;
    WriteRule(*this->RulesFileStream, rule);
  }

  cmNinjaBuild reBuild("RERUN_CMAKE");
  reBuild.Comment = "Re-run CMake if any of its inputs changed.";
  this->AddRebuildManifestOutputs(reBuild.Outputs);

  for (const auto& localGen : this->LocalGenerators) {
    for (std::string const& fi : localGen->GetMakefile()->GetListFiles()) {
      reBuild.ImplicitDeps.push_back(this->ConvertToNinjaPath(fi));
    }
  }
  reBuild.ImplicitDeps.push_back(this->CMakeCacheFile);

  // Use 'console' pool to get non buffered output of the CMake re-run call
  // Available since Ninja 1.5
  if (this->SupportsDirectConsole()) {
    reBuild.Variables["pool"] = "console";
  }

  if (this->SupportsManifestRestat() && cm->DoWriteGlobVerifyTarget()) {
    {
      cmNinjaRule rule("VERIFY_GLOBS");
      rule.Command =
        cmStrCat(this->CMakeCmd(), " -P ",
                 lg->ConvertToOutputFormat(cm->GetGlobVerifyScript(),
                                           cmOutputConverter::SHELL));
      rule.Description = "Re-checking globbed directories...";
      rule.Comment = "Rule for re-checking globbed directories.";
      rule.Generator = true;
      this->WriteRule(*this->RulesFileStream, rule);
    }

    cmNinjaBuild phonyBuild("phony");
    phonyBuild.Comment = "Phony target to force glob verification run.";
    phonyBuild.Outputs.push_back(
      cmStrCat(cm->GetGlobVerifyScript(), "_force"));
    this->WriteBuild(os, phonyBuild);

    reBuild.Variables["restat"] = "1";
    std::string const verifyScriptFile =
      this->NinjaOutputPath(cm->GetGlobVerifyScript());
    std::string const verifyStampFile =
      this->NinjaOutputPath(cm->GetGlobVerifyStamp());
    {
      cmNinjaBuild vgBuild("VERIFY_GLOBS");
      vgBuild.Comment =
        "Re-run CMake to check if globbed directories changed.";
      vgBuild.Outputs.push_back(verifyStampFile);
      vgBuild.ImplicitDeps = phonyBuild.Outputs;
      vgBuild.Variables = reBuild.Variables;
      this->WriteBuild(os, vgBuild);
    }
    reBuild.Variables.erase("restat");
    reBuild.ImplicitDeps.push_back(verifyScriptFile);
    reBuild.ExplicitDeps.push_back(verifyStampFile);
  } else if (!this->SupportsManifestRestat() &&
             cm->DoWriteGlobVerifyTarget()) {
    std::ostringstream msg;
    msg << "The detected version of Ninja:\n"
        << "  " << this->NinjaVersion << "\n"
        << "is less than the version of Ninja required by CMake for adding "
           "restat dependencies to the build.ninja manifest regeneration "
           "target:\n"
        << "  "
        << cmGlobalNinjaGenerator::RequiredNinjaVersionForManifestRestat()
        << "\n";
    msg << "Any pre-check scripts, such as those generated for file(GLOB "
           "CONFIGURE_DEPENDS), will not be run by Ninja.";
    this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                           msg.str());
  }

  std::sort(reBuild.ImplicitDeps.begin(), reBuild.ImplicitDeps.end());
  reBuild.ImplicitDeps.erase(
    std::unique(reBuild.ImplicitDeps.begin(), reBuild.ImplicitDeps.end()),
    reBuild.ImplicitDeps.end());

  this->WriteBuild(os, reBuild);

  {
    cmNinjaBuild build("phony");
    build.Comment = "A missing CMake input file is not an error.";
    std::set_difference(std::make_move_iterator(reBuild.ImplicitDeps.begin()),
                        std::make_move_iterator(reBuild.ImplicitDeps.end()),
                        this->CustomCommandOutputs.begin(),
                        this->CustomCommandOutputs.end(),
                        std::back_inserter(build.Outputs));
    this->WriteBuild(os, build);
  }
}

std::string cmGlobalNinjaGenerator::CMakeCmd() const
{
  const auto& lgen = this->LocalGenerators.at(0);
  return lgen->ConvertToOutputFormat(cmSystemTools::GetCMakeCommand(),
                                     cmOutputConverter::SHELL);
}

std::string cmGlobalNinjaGenerator::NinjaCmd() const
{
  const auto& lgen = this->LocalGenerators[0];
  if (lgen != nullptr) {
    return lgen->ConvertToOutputFormat(this->NinjaCommand,
                                       cmOutputConverter::SHELL);
  }
  return "ninja";
}

bool cmGlobalNinjaGenerator::SupportsDirectConsole() const
{
  return this->NinjaSupportsConsolePool;
}

bool cmGlobalNinjaGenerator::SupportsImplicitOuts() const
{
  return this->NinjaSupportsImplicitOuts;
}

bool cmGlobalNinjaGenerator::SupportsManifestRestat() const
{
  return this->NinjaSupportsManifestRestat;
}

bool cmGlobalNinjaGenerator::SupportsMultilineDepfile() const
{
  return this->NinjaSupportsMultilineDepfile;
}

bool cmGlobalNinjaGenerator::WriteTargetCleanAdditional(std::ostream& os)
{
  const auto& lgr = this->LocalGenerators.at(0);
  std::string cleanScriptRel = "CMakeFiles/clean_additional.cmake";
  std::string cleanScriptAbs =
    cmStrCat(lgr->GetBinaryDirectory(), '/', cleanScriptRel);
  std::vector<std::string> configs =
    this->Makefiles[0]->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  // Check if there are additional files to clean
  bool empty = true;
  for (auto const& config : configs) {
    auto const it = this->Configs.find(config);
    if (it != this->Configs.end() &&
        !it->second.AdditionalCleanFiles.empty()) {
      empty = false;
      break;
    }
  }
  if (empty) {
    // Remove cmake clean script file if it exists
    cmSystemTools::RemoveFile(cleanScriptAbs);
    return false;
  }

  // Write cmake clean script file
  {
    cmGeneratedFileStream fout(cleanScriptAbs);
    if (!fout) {
      return false;
    }
    fout << "# Additional clean files\ncmake_minimum_required(VERSION 3.16)\n";
    for (auto const& config : configs) {
      auto const it = this->Configs.find(config);
      if (it != this->Configs.end() &&
          !it->second.AdditionalCleanFiles.empty()) {
        fout << "\nif(\"${CONFIG}\" STREQUAL \"\" OR \"${CONFIG}\" STREQUAL \""
             << config << "\")\n";
        fout << "  file(REMOVE_RECURSE\n";
        for (std::string const& acf : it->second.AdditionalCleanFiles) {
          fout << "  "
               << cmOutputConverter::EscapeForCMake(
                    this->ConvertToNinjaPath(acf))
               << '\n';
        }
        fout << "  )\n";
        fout << "endif()\n";
      }
    }
  }
  // Register clean script file
  lgr->GetMakefile()->AddCMakeOutputFile(cleanScriptAbs);

  // Write rule
  {
    cmNinjaRule rule("CLEAN_ADDITIONAL");
    rule.Command = cmStrCat(
      this->CMakeCmd(), " -DCONFIG=$CONFIG -P ",
      lgr->ConvertToOutputFormat(this->NinjaOutputPath(cleanScriptRel),
                                 cmOutputConverter::SHELL));
    rule.Description = "Cleaning additional files...";
    rule.Comment = "Rule for cleaning additional files.";
    WriteRule(*this->RulesFileStream, rule);
  }

  // Write build
  {
    cmNinjaBuild build("CLEAN_ADDITIONAL");
    build.Comment = "Clean additional files.";
    build.Outputs.emplace_back();
    for (auto const& config : configs) {
      build.Outputs.front() = this->BuildAlias(
        this->NinjaOutputPath(this->GetAdditionalCleanTargetName()), config);
      build.Variables["CONFIG"] = config;
      this->WriteBuild(os, build);
    }
    if (this->IsMultiConfig()) {
      build.Outputs.front() =
        this->NinjaOutputPath(this->GetAdditionalCleanTargetName());
      build.Variables["CONFIG"] = "";
      this->WriteBuild(os, build);
    }
  }
  // Return success
  return true;
}

void cmGlobalNinjaGenerator::WriteTargetClean(std::ostream& os)
{
  // -- Additional clean target
  bool additionalFiles = this->WriteTargetCleanAdditional(os);

  // -- Default clean target
  // Write rule
  {
    cmNinjaRule rule("CLEAN");
    rule.Command = cmStrCat(this->NinjaCmd(), " $FILE_ARG -t clean $TARGETS");
    rule.Description = "Cleaning all built files...";
    rule.Comment = "Rule for cleaning all built files.";
    WriteRule(*this->RulesFileStream, rule);
  }

  auto const configs = this->Makefiles.front()->GetGeneratorConfigs(
    cmMakefile::IncludeEmptyConfig);

  // Write build
  {
    cmNinjaBuild build("CLEAN");
    build.Comment = "Clean all the built files.";
    build.Outputs.emplace_back();

    for (auto const& config : configs) {
      build.Outputs.front() = this->BuildAlias(
        this->NinjaOutputPath(this->GetCleanTargetName()), config);
      if (this->IsMultiConfig()) {
        build.Variables["TARGETS"] = cmStrCat(
          this->BuildAlias(
            this->NinjaOutputPath(GetByproductsForCleanTargetName()), config),
          " ", this->NinjaOutputPath(GetByproductsForCleanTargetName()));
      }
      build.ExplicitDeps.clear();
      if (additionalFiles) {
        build.ExplicitDeps.push_back(this->BuildAlias(
          this->NinjaOutputPath(this->GetAdditionalCleanTargetName()),
          config));
      }
      for (auto const& fileConfig : configs) {
        if (fileConfig != config && !this->EnableCrossConfigBuild()) {
          continue;
        }
        if (this->IsMultiConfig()) {
          build.Variables["FILE_ARG"] = cmStrCat(
            "-f ",
            this->NinjaOutputPath(
              cmGlobalNinjaMultiGenerator::GetNinjaImplFilename(fileConfig)));
        }
        this->WriteBuild(*this->GetImplFileStream(fileConfig), build);
      }
    }

    if (this->EnableCrossConfigBuild()) {
      build.Outputs.front() = this->BuildAlias(
        this->NinjaOutputPath(this->GetCleanTargetName()), "all");
      build.ExplicitDeps.clear();

      if (additionalFiles) {
        for (auto const& config : this->CrossConfigs) {
          build.ExplicitDeps.push_back(this->BuildAlias(
            this->NinjaOutputPath(this->GetAdditionalCleanTargetName()),
            config));
        }
      }

      std::vector<std::string> byproducts;
      byproducts.reserve(this->CrossConfigs.size());
      for (auto const& config : this->CrossConfigs) {
        byproducts.push_back(this->BuildAlias(
          this->NinjaOutputPath(GetByproductsForCleanTargetName()), config));
      }
      byproducts.emplace_back(GetByproductsForCleanTargetName());
      build.Variables["TARGETS"] = cmJoin(byproducts, " ");

      for (auto const& fileConfig : configs) {
        build.Variables["FILE_ARG"] = cmStrCat(
          "-f ",
          this->NinjaOutputPath(
            cmGlobalNinjaMultiGenerator::GetNinjaImplFilename(fileConfig)));
        this->WriteBuild(*this->GetImplFileStream(fileConfig), build);
      }
    }
  }

  if (this->IsMultiConfig()) {
    cmNinjaBuild build("phony");
    build.Outputs.emplace_back(
      this->NinjaOutputPath(this->GetCleanTargetName()));
    build.ExplicitDeps.emplace_back();

    for (auto const& config : configs) {
      build.ExplicitDeps.front() = this->BuildAlias(
        this->NinjaOutputPath(this->GetCleanTargetName()), config);
      this->WriteBuild(*this->GetConfigFileStream(config), build);
    }

    if (!this->DefaultConfigs.empty()) {
      build.ExplicitDeps.clear();
      for (auto const& config : this->DefaultConfigs) {
        build.ExplicitDeps.push_back(this->BuildAlias(
          this->NinjaOutputPath(this->GetCleanTargetName()), config));
      }
      this->WriteBuild(*this->GetDefaultFileStream(), build);
    }
  }

  // Write byproducts
  if (this->IsMultiConfig()) {
    cmNinjaBuild build("phony");
    build.Comment = "Clean byproducts.";
    build.Outputs.emplace_back(
      this->ConvertToNinjaPath(GetByproductsForCleanTargetName()));
    build.ExplicitDeps = this->ByproductsForCleanTarget;
    this->WriteBuild(os, build);

    for (auto const& config : configs) {
      build.Outputs.front() = this->BuildAlias(
        this->ConvertToNinjaPath(GetByproductsForCleanTargetName()), config);
      build.ExplicitDeps = this->Configs[config].ByproductsForCleanTarget;
      this->WriteBuild(os, build);
    }
  }
}

void cmGlobalNinjaGenerator::WriteTargetHelp(std::ostream& os)
{
  {
    cmNinjaRule rule("HELP");
    rule.Command = cmStrCat(this->NinjaCmd(), " -t targets");
    rule.Description = "All primary targets available:";
    rule.Comment = "Rule for printing all primary targets available.";
    WriteRule(*this->RulesFileStream, rule);
  }
  {
    cmNinjaBuild build("HELP");
    build.Comment = "Print all primary targets available.";
    build.Outputs.push_back(this->NinjaOutputPath("help"));
    this->WriteBuild(os, build);
  }
}

void cmGlobalNinjaGenerator::InitOutputPathPrefix()
{
  this->OutputPathPrefix =
    this->LocalGenerators[0]->GetMakefile()->GetSafeDefinition(
      "CMAKE_NINJA_OUTPUT_PATH_PREFIX");
  EnsureTrailingSlash(this->OutputPathPrefix);
}

std::string cmGlobalNinjaGenerator::NinjaOutputPath(
  std::string const& path) const
{
  if (!this->HasOutputPathPrefix() || cmSystemTools::FileIsFullPath(path)) {
    return path;
  }
  return cmStrCat(this->OutputPathPrefix, path);
}

void cmGlobalNinjaGenerator::StripNinjaOutputPathPrefixAsSuffix(
  std::string& path)
{
  if (path.empty()) {
    return;
  }
  EnsureTrailingSlash(path);
  cmStripSuffixIfExists(path, this->OutputPathPrefix);
}

#if !defined(CMAKE_BOOTSTRAP)

/*

We use the following approach to support Fortran.  Each target already
has a <target>.dir/ directory used to hold intermediate files for CMake.
For each target, a FortranDependInfo.json file is generated by CMake with
information about include directories, module directories, and the locations
the per-target directories for target dependencies.

Compilation of source files within a target is split into the following steps:

1. Preprocess all sources, scan preprocessed output for module dependencies.
   This step is done with independent build statements for each source,
   and can therefore be done in parallel.

    rule Fortran_PREPROCESS
      depfile = $DEP_FILE
      command = gfortran -cpp $DEFINES $INCLUDES $FLAGS -E $in -o $out &&
                cmake -E cmake_ninja_depends \
                  --tdi=FortranDependInfo.json --lang=Fortran \
                  --src=$out --out=$out --dep=$DEP_FILE --obj=$OBJ_FILE \
                  --ddi=$DYNDEP_INTERMEDIATE_FILE

    build src.f90-pp.f90 | src.f90.o.ddi: Fortran_PREPROCESS src.f90
      OBJ_FILE = src.f90.o
      DEP_FILE = src.f90.o.d
      DYNDEP_INTERMEDIATE_FILE = src.f90.o.ddi

   The ``cmake -E cmake_ninja_depends`` tool reads the preprocessed output
   and generates the ninja depfile for preprocessor dependencies.  It also
   generates a "ddi" file (in a format private to CMake) that lists the
   object file that compilation will produce along with the module names
   it provides and/or requires.  The "ddi" file is an implicit output
   because it should not appear in "$out" but is generated by the rule.

2. Consolidate the per-source module dependencies saved in the "ddi"
   files from all sources to produce a ninja "dyndep" file, ``Fortran.dd``.

    rule Fortran_DYNDEP
      command = cmake -E cmake_ninja_dyndep \
                  --tdi=FortranDependInfo.json --lang=Fortran --dd=$out $in

    build Fortran.dd: Fortran_DYNDEP src1.f90.o.ddi src2.f90.o.ddi

   The ``cmake -E cmake_ninja_dyndep`` tool reads the "ddi" files from all
   sources in the target and the ``FortranModules.json`` files from targets
   on which the target depends.  It computes dependency edges on compilations
   that require modules to those that provide the modules.  This information
   is placed in the ``Fortran.dd`` file for ninja to load later.  It also
   writes the expected location of modules provided by this target into
   ``FortranModules.json`` for use by dependent targets.

3. Compile all sources after loading dynamically discovered dependencies
   of the compilation build statements from their ``dyndep`` bindings.

    rule Fortran_COMPILE
      command = gfortran $INCLUDES $FLAGS -c $in -o $out

    build src1.f90.o: Fortran_COMPILE src1.f90-pp.f90 || Fortran.dd
      dyndep = Fortran.dd

   The "dyndep" binding tells ninja to load dynamically discovered
   dependency information from ``Fortran.dd``.  This adds information
   such as:

    build src1.f90.o | mod1.mod: dyndep
      restat = 1

   This tells ninja that ``mod1.mod`` is an implicit output of compiling
   the object file ``src1.f90.o``.  The ``restat`` binding tells it that
   the timestamp of the output may not always change.  Additionally:

    build src2.f90.o: dyndep | mod1.mod

   This tells ninja that ``mod1.mod`` is a dependency of compiling the
   object file ``src2.f90.o``.  This ensures that ``src1.f90.o`` and
   ``mod1.mod`` will always be up to date before ``src2.f90.o`` is built
   (because the latter consumes the module).
*/

namespace {

struct cmSourceInfo
{
  cmScanDepInfo ScanDep;
  std::vector<std::string> Includes;
};

cm::optional<cmSourceInfo> cmcmd_cmake_ninja_depends_fortran(
  std::string const& arg_tdi, std::string const& arg_src);
}

int cmcmd_cmake_ninja_depends(std::vector<std::string>::const_iterator argBeg,
                              std::vector<std::string>::const_iterator argEnd)
{
  std::string arg_tdi;
  std::string arg_src;
  std::string arg_out;
  std::string arg_dep;
  std::string arg_obj;
  std::string arg_ddi;
  std::string arg_lang;
  for (std::string const& arg : cmMakeRange(argBeg, argEnd)) {
    if (cmHasLiteralPrefix(arg, "--tdi=")) {
      arg_tdi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--src=")) {
      arg_src = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--out=")) {
      arg_out = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--dep=")) {
      arg_dep = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--obj=")) {
      arg_obj = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--ddi=")) {
      arg_ddi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--lang=")) {
      arg_lang = arg.substr(7);
    } else if (cmHasLiteralPrefix(arg, "--pp=")) {
      // CMake 3.26 and below used '--pp=' instead of '--src=' and '--out='.
      arg_src = arg.substr(5);
      arg_out = arg_src;
    } else {
      cmSystemTools::Error(
        cmStrCat("-E cmake_ninja_depends unknown argument: ", arg));
      return 1;
    }
  }
  if (arg_tdi.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --tdi=");
    return 1;
  }
  if (arg_src.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --src=");
    return 1;
  }
  if (arg_out.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --out=");
    return 1;
  }
  if (arg_dep.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --dep=");
    return 1;
  }
  if (arg_obj.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --obj=");
    return 1;
  }
  if (arg_ddi.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --ddi=");
    return 1;
  }
  if (arg_lang.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --lang=");
    return 1;
  }

  cm::optional<cmSourceInfo> info;
  if (arg_lang == "Fortran") {
    info = cmcmd_cmake_ninja_depends_fortran(arg_tdi, arg_src);
  } else {
    cmSystemTools::Error(
      cmStrCat("-E cmake_ninja_depends does not understand the ", arg_lang,
               " language"));
    return 1;
  }

  if (!info) {
    // The error message is already expected to have been output.
    return 1;
  }

  info->ScanDep.PrimaryOutput = arg_obj;

  {
    cmGeneratedFileStream depfile(arg_dep);
    depfile << cmSystemTools::ConvertToUnixOutputPath(arg_out) << ":";
    for (std::string const& include : info->Includes) {
      depfile << " \\\n " << cmSystemTools::ConvertToUnixOutputPath(include);
    }
    depfile << "\n";
  }

  if (!cmScanDepFormat_P1689_Write(arg_ddi, info->ScanDep)) {
    cmSystemTools::Error(
      cmStrCat("-E cmake_ninja_depends failed to write ", arg_ddi));
    return 1;
  }
  return 0;
}

namespace {

cm::optional<cmSourceInfo> cmcmd_cmake_ninja_depends_fortran(
  std::string const& arg_tdi, std::string const& arg_src)
{
  cm::optional<cmSourceInfo> info;
  cmFortranCompiler fc;
  std::vector<std::string> includes;
  std::string dir_top_bld;
  std::string module_dir;
  {
    Json::Value tdio;
    Json::Value const& tdi = tdio;
    {
      cmsys::ifstream tdif(arg_tdi.c_str(), std::ios::in | std::ios::binary);
      Json::Reader reader;
      if (!reader.parse(tdif, tdio, false)) {
        cmSystemTools::Error(
          cmStrCat("-E cmake_ninja_depends failed to parse ", arg_tdi,
                   reader.getFormattedErrorMessages()));
        return info;
      }
    }

    dir_top_bld = tdi["dir-top-bld"].asString();
    if (!dir_top_bld.empty() && !cmHasLiteralSuffix(dir_top_bld, "/")) {
      dir_top_bld += '/';
    }

    Json::Value const& tdi_include_dirs = tdi["include-dirs"];
    if (tdi_include_dirs.isArray()) {
      for (auto const& tdi_include_dir : tdi_include_dirs) {
        includes.push_back(tdi_include_dir.asString());
      }
    }

    Json::Value const& tdi_module_dir = tdi["module-dir"];
    module_dir = tdi_module_dir.asString();
    if (!module_dir.empty() && !cmHasLiteralSuffix(module_dir, "/")) {
      module_dir += '/';
    }

    Json::Value const& tdi_compiler_id = tdi["compiler-id"];
    fc.Id = tdi_compiler_id.asString();

    Json::Value const& tdi_submodule_sep = tdi["submodule-sep"];
    fc.SModSep = tdi_submodule_sep.asString();

    Json::Value const& tdi_submodule_ext = tdi["submodule-ext"];
    fc.SModExt = tdi_submodule_ext.asString();
  }

  cmFortranSourceInfo finfo;
  std::set<std::string> defines;
  cmFortranParser parser(fc, includes, defines, finfo);
  if (!cmFortranParser_FilePush(&parser, arg_src.c_str())) {
    cmSystemTools::Error(
      cmStrCat("-E cmake_ninja_depends failed to open ", arg_src));
    return info;
  }
  if (cmFortran_yyparse(parser.Scanner) != 0) {
    // Failed to parse the file.
    return info;
  }

  info = cmSourceInfo();
  for (std::string const& provide : finfo.Provides) {
    cmSourceReqInfo src_info;
    src_info.LogicalName = provide;
    if (!module_dir.empty()) {
      std::string mod = cmStrCat(module_dir, provide);
      if (!dir_top_bld.empty() && cmHasPrefix(mod, dir_top_bld)) {
        mod = mod.substr(dir_top_bld.size());
      }
      src_info.CompiledModulePath = std::move(mod);
    }
    info->ScanDep.Provides.emplace_back(src_info);
  }
  for (std::string const& require : finfo.Requires) {
    // Require modules not provided in the same source.
    if (finfo.Provides.count(require)) {
      continue;
    }
    cmSourceReqInfo src_info;
    src_info.LogicalName = require;
    info->ScanDep.Requires.emplace_back(src_info);
  }
  for (std::string const& include : finfo.Includes) {
    info->Includes.push_back(include);
  }
  return info;
}
}

bool cmGlobalNinjaGenerator::WriteDyndepFile(
  std::string const& dir_top_src, std::string const& dir_top_bld,
  std::string const& dir_cur_src, std::string const& dir_cur_bld,
  std::string const& arg_dd, std::vector<std::string> const& arg_ddis,
  std::string const& module_dir,
  std::vector<std::string> const& linked_target_dirs,
  std::string const& arg_lang, std::string const& arg_modmapfmt,
  cmCxxModuleExportInfo const& export_info)
{
  // Setup path conversions.
  {
    cmStateSnapshot snapshot = this->GetCMakeInstance()->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentSource(dir_cur_src);
    snapshot.GetDirectory().SetCurrentBinary(dir_cur_bld);
    auto mfd = cm::make_unique<cmMakefile>(this, snapshot);
    auto lgd = this->CreateLocalGenerator(mfd.get());
    lgd->SetRelativePathTop(dir_top_src, dir_top_bld);
    this->Makefiles.push_back(std::move(mfd));
    this->LocalGenerators.push_back(std::move(lgd));
  }

  std::vector<cmScanDepInfo> objects;
  for (std::string const& arg_ddi : arg_ddis) {
    cmScanDepInfo info;
    if (!cmScanDepFormat_P1689_Parse(arg_ddi, &info)) {
      cmSystemTools::Error(
        cmStrCat("-E cmake_ninja_dyndep failed to parse ddi file ", arg_ddi));
      return false;
    }
    objects.push_back(std::move(info));
  }

  CxxModuleUsage usages;

  // Map from module name to module file path, if known.
  struct AvailableModuleInfo
  {
    std::string BmiPath;
    bool IsPrivate;
  };
  std::map<std::string, AvailableModuleInfo> mod_files;

  // Populate the module map with those provided by linked targets first.
  for (std::string const& linked_target_dir : linked_target_dirs) {
    std::string const ltmn =
      cmStrCat(linked_target_dir, '/', arg_lang, "Modules.json");
    Json::Value ltm;
    cmsys::ifstream ltmf(ltmn.c_str(), std::ios::in | std::ios::binary);
    if (!ltmf) {
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to open ",
                                    ltmn, " for module information"));
      return false;
    }
    Json::Reader reader;
    if (!reader.parse(ltmf, ltm, false)) {
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                    linked_target_dir,
                                    reader.getFormattedErrorMessages()));
      return false;
    }
    if (ltm.isObject()) {
      Json::Value const& target_modules = ltm["modules"];
      if (target_modules.isObject()) {
        for (auto i = target_modules.begin(); i != target_modules.end(); ++i) {
          Json::Value const& visible_module = *i;
          if (visible_module.isObject()) {
            Json::Value const& bmi_path = visible_module["bmi"];
            Json::Value const& is_private = visible_module["is-private"];
            mod_files[i.key().asString()] = AvailableModuleInfo{
              bmi_path.asString(),
              is_private.asBool(),
            };
          }
        }
      }
      Json::Value const& target_modules_references = ltm["references"];
      if (target_modules_references.isObject()) {
        for (auto i = target_modules_references.begin();
             i != target_modules_references.end(); ++i) {
          if (i->isObject()) {
            Json::Value const& reference_path = (*i)["path"];
            CxxModuleReference module_reference;
            if (reference_path.isString()) {
              module_reference.Path = reference_path.asString();
            }
            Json::Value const& reference_method = (*i)["lookup-method"];
            if (reference_method.isString()) {
              std::string reference = reference_method.asString();
              if (reference == "by-name") {
                module_reference.Method = LookupMethod::ByName;
              } else if (reference == "include-angle") {
                module_reference.Method = LookupMethod::IncludeAngle;
              } else if (reference == "include-quote") {
                module_reference.Method = LookupMethod::IncludeQuote;
              }
            }
            usages.Reference[i.key().asString()] = module_reference;
          }
        }
      }
      Json::Value const& target_modules_usage = ltm["usages"];
      if (target_modules_usage.isObject()) {
        for (auto i = target_modules_usage.begin();
             i != target_modules_usage.end(); ++i) {
          if (i->isArray()) {
            for (auto j = i->begin(); j != i->end(); ++j) {
              usages.Usage[i.key().asString()].insert(j->asString());
            }
          }
        }
      }
    }
  }

  cm::optional<CxxModuleMapFormat> modmap_fmt;
  if (arg_modmapfmt.empty()) {
    // nothing to do.
  } else if (arg_modmapfmt == "clang") {
    modmap_fmt = CxxModuleMapFormat::Clang;
  } else if (arg_modmapfmt == "gcc") {
    modmap_fmt = CxxModuleMapFormat::Gcc;
  } else if (arg_modmapfmt == "msvc") {
    modmap_fmt = CxxModuleMapFormat::Msvc;
  } else {
    cmSystemTools::Error(
      cmStrCat("-E cmake_ninja_dyndep does not understand the ", arg_modmapfmt,
               " module map format"));
    return false;
  }

  auto module_ext = CxxModuleMapExtension(modmap_fmt);

  // Extend the module map with those provided by this target.
  // We do this after loading the modules provided by linked targets
  // in case we have one of the same name that must be preferred.
  Json::Value target_modules = Json::objectValue;
  for (cmScanDepInfo const& object : objects) {
    for (auto const& p : object.Provides) {
      std::string mod;
      if (!p.CompiledModulePath.empty()) {
        // The scanner provided the path to the module file.
        mod = p.CompiledModulePath;
        if (!cmSystemTools::FileIsFullPath(mod)) {
          // Treat relative to work directory (top of build tree).
          mod = cmSystemTools::CollapseFullPath(mod, dir_top_bld);
        }
      } else {
        // Assume the module file path matches the logical module name.
        std::string safe_logical_name =
          p.LogicalName; // TODO: needs fixing for header units
        cmSystemTools::ReplaceString(safe_logical_name, ":", "-");
        mod = cmStrCat(module_dir, safe_logical_name, module_ext);
      }
      mod_files[p.LogicalName] = AvailableModuleInfo{
        mod,
        false, // Always visible within our own target.
      };
      Json::Value& module_info = target_modules[p.LogicalName] =
        Json::objectValue;
      module_info["bmi"] = mod;
      module_info["is-private"] =
        cmDyndepCollation::IsObjectPrivate(object.PrimaryOutput, export_info);
    }
  }

  cmGeneratedFileStream ddf(arg_dd);
  ddf << "ninja_dyndep_version = 1.0\n";

  {
    CxxModuleLocations locs;
    locs.RootDirectory = ".";
    locs.PathForGenerator = [this](std::string path) -> std::string {
      path = this->ConvertToNinjaPath(path);
#  ifdef _WIN32
      if (this->IsGCCOnWindows()) {
        std::replace(path.begin(), path.end(), '\\', '/');
      }
#  endif
      return path;
    };
    locs.BmiLocationForModule =
      [&mod_files](std::string const& logical) -> CxxBmiLocation {
      auto m = mod_files.find(logical);
      if (m != mod_files.end()) {
        if (m->second.IsPrivate) {
          return CxxBmiLocation::Private();
        }
        return CxxBmiLocation::Known(m->second.BmiPath);
      }
      return CxxBmiLocation::Unknown();
    };

    // Insert information about the current target's modules.
    if (modmap_fmt) {
      auto cycle_modules = CxxModuleUsageSeed(locs, objects, usages);
      if (!cycle_modules.empty()) {
        cmSystemTools::Error(
          cmStrCat("Circular dependency detected in the C++ module import "
                   "graph. See modules named: \"",
                   cmJoin(cycle_modules, R"(", ")"_s), '"'));
        return false;
      }
    }

    cmNinjaBuild build("dyndep");
    build.Outputs.emplace_back("");
    for (cmScanDepInfo const& object : objects) {
      build.Outputs[0] = this->ConvertToNinjaPath(object.PrimaryOutput);
      build.ImplicitOuts.clear();
      for (auto const& p : object.Provides) {
        build.ImplicitOuts.push_back(
          this->ConvertToNinjaPath(mod_files[p.LogicalName].BmiPath));
      }
      build.ImplicitDeps.clear();
      for (auto const& r : object.Requires) {
        auto mit = mod_files.find(r.LogicalName);
        if (mit != mod_files.end()) {
          build.ImplicitDeps.push_back(
            this->ConvertToNinjaPath(mit->second.BmiPath));
        }
      }
      build.Variables.clear();
      if (!object.Provides.empty()) {
        build.Variables.emplace("restat", "1");
      }

      if (modmap_fmt) {
        auto mm = CxxModuleMapContent(*modmap_fmt, locs, object, usages);

        // XXX(modmap): If changing this path construction, change
        // `cmNinjaTargetGenerator::WriteObjectBuildStatements` to generate the
        // corresponding file path.
        cmGeneratedFileStream mmf(cmStrCat(object.PrimaryOutput, ".modmap"));
        mmf << mm;
      }

      this->WriteBuild(ddf, build);
    }
  }

  Json::Value target_module_info = Json::objectValue;
  target_module_info["modules"] = target_modules;

  auto& target_usages = target_module_info["usages"] = Json::objectValue;
  for (auto const& u : usages.Usage) {
    auto& mod_usage = target_usages[u.first] = Json::arrayValue;
    for (auto const& v : u.second) {
      mod_usage.append(v);
    }
  }

  auto name_for_method = [](LookupMethod method) -> cm::static_string_view {
    switch (method) {
      case LookupMethod::ByName:
        return "by-name"_s;
      case LookupMethod::IncludeAngle:
        return "include-angle"_s;
      case LookupMethod::IncludeQuote:
        return "include-quote"_s;
    }
    assert(false && "unsupported lookup method");
    return ""_s;
  };

  auto& target_references = target_module_info["references"] =
    Json::objectValue;
  for (auto const& r : usages.Reference) {
    auto& mod_ref = target_references[r.first] = Json::objectValue;
    mod_ref["path"] = r.second.Path;
    mod_ref["lookup-method"] = std::string(name_for_method(r.second.Method));
  }

  // Store the map of modules provided by this target in a file for
  // use by dependents that reference this target in linked-target-dirs.
  std::string const target_mods_file = cmStrCat(
    cmSystemTools::GetFilenamePath(arg_dd), '/', arg_lang, "Modules.json");
  cmGeneratedFileStream tmf(target_mods_file);
  tmf << target_module_info;

  cmDyndepMetadataCallbacks cb;
  cb.ModuleFile =
    [mod_files](std::string const& name) -> cm::optional<std::string> {
    auto m = mod_files.find(name);
    if (m != mod_files.end()) {
      return m->second.BmiPath;
    }
    return {};
  };

  return cmDyndepCollation::WriteDyndepMetadata(arg_lang, objects, export_info,
                                                cb);
}

int cmcmd_cmake_ninja_dyndep(std::vector<std::string>::const_iterator argBeg,
                             std::vector<std::string>::const_iterator argEnd)
{
  std::vector<std::string> arg_full =
    cmSystemTools::HandleResponseFile(argBeg, argEnd);

  std::string arg_dd;
  std::string arg_lang;
  std::string arg_tdi;
  std::string arg_modmapfmt;
  std::vector<std::string> arg_ddis;
  for (std::string const& arg : arg_full) {
    if (cmHasLiteralPrefix(arg, "--tdi=")) {
      arg_tdi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--lang=")) {
      arg_lang = arg.substr(7);
    } else if (cmHasLiteralPrefix(arg, "--dd=")) {
      arg_dd = arg.substr(5);
    } else if (cmHasLiteralPrefix(arg, "--modmapfmt=")) {
      arg_modmapfmt = arg.substr(12);
    } else if (!cmHasLiteralPrefix(arg, "--") &&
               cmHasLiteralSuffix(arg, ".ddi")) {
      arg_ddis.push_back(arg);
    } else {
      cmSystemTools::Error(
        cmStrCat("-E cmake_ninja_dyndep unknown argument: ", arg));
      return 1;
    }
  }
  if (arg_tdi.empty()) {
    cmSystemTools::Error("-E cmake_ninja_dyndep requires value for --tdi=");
    return 1;
  }
  if (arg_lang.empty()) {
    cmSystemTools::Error("-E cmake_ninja_dyndep requires value for --lang=");
    return 1;
  }
  if (arg_dd.empty()) {
    cmSystemTools::Error("-E cmake_ninja_dyndep requires value for --dd=");
    return 1;
  }

  Json::Value tdio;
  Json::Value const& tdi = tdio;
  {
    cmsys::ifstream tdif(arg_tdi.c_str(), std::ios::in | std::ios::binary);
    Json::Reader reader;
    if (!reader.parse(tdif, tdio, false)) {
      cmSystemTools::Error(cmStrCat("-E cmake_ninja_dyndep failed to parse ",
                                    arg_tdi,
                                    reader.getFormattedErrorMessages()));
      return 1;
    }
  }

  std::string const dir_cur_bld = tdi["dir-cur-bld"].asString();
  std::string const dir_cur_src = tdi["dir-cur-src"].asString();
  std::string const dir_top_bld = tdi["dir-top-bld"].asString();
  std::string const dir_top_src = tdi["dir-top-src"].asString();
  std::string module_dir = tdi["module-dir"].asString();
  if (!module_dir.empty() && !cmHasLiteralSuffix(module_dir, "/")) {
    module_dir += '/';
  }
  std::vector<std::string> linked_target_dirs;
  Json::Value const& tdi_linked_target_dirs = tdi["linked-target-dirs"];
  if (tdi_linked_target_dirs.isArray()) {
    for (auto const& tdi_linked_target_dir : tdi_linked_target_dirs) {
      linked_target_dirs.push_back(tdi_linked_target_dir.asString());
    }
  }
  std::string const compilerId = tdi["compiler-id"].asString();
  std::string const simulateId = tdi["compiler-simulate-id"].asString();
  std::string const compilerFrontendVariant =
    tdi["compiler-frontend-variant"].asString();

  auto export_info = cmDyndepCollation::ParseExportInfo(tdi);

  cmake cm(cmake::RoleInternal, cmState::Unknown);
  cm.SetHomeDirectory(dir_top_src);
  cm.SetHomeOutputDirectory(dir_top_bld);
  auto ggd = cm.CreateGlobalGenerator("Ninja");
  if (!ggd) {
    return 1;
  }
  cmGlobalNinjaGenerator& gg =
    cm::static_reference_cast<cmGlobalNinjaGenerator>(ggd);
#  ifdef _WIN32
  if (DetectGCCOnWindows(compilerId, simulateId, compilerFrontendVariant)) {
    gg.MarkAsGCCOnWindows();
  }
#  endif
  return gg.WriteDyndepFile(dir_top_src, dir_top_bld, dir_cur_src, dir_cur_bld,
                            arg_dd, arg_ddis, module_dir, linked_target_dirs,
                            arg_lang, arg_modmapfmt, *export_info)
    ? 0
    : 1;
}

#endif

bool cmGlobalNinjaGenerator::EnableCrossConfigBuild() const
{
  return !this->CrossConfigs.empty();
}

void cmGlobalNinjaGenerator::AppendDirectoryForConfig(
  const std::string& prefix, const std::string& config,
  const std::string& suffix, std::string& dir)
{
  if (!config.empty() && this->IsMultiConfig()) {
    dir += cmStrCat(prefix, config, suffix);
  }
}

std::set<std::string> cmGlobalNinjaGenerator::GetCrossConfigs(
  const std::string& fileConfig) const
{
  auto result = this->CrossConfigs;
  result.insert(fileConfig);
  return result;
}

bool cmGlobalNinjaGenerator::IsSingleConfigUtility(
  cmGeneratorTarget const* target) const
{
  return target->GetType() == cmStateEnums::UTILITY &&
    !this->PerConfigUtilityTargets.count(target->GetName());
}

const char* cmGlobalNinjaMultiGenerator::NINJA_COMMON_FILE =
  "CMakeFiles/common.ninja";
const char* cmGlobalNinjaMultiGenerator::NINJA_FILE_EXTENSION = ".ninja";

cmGlobalNinjaMultiGenerator::cmGlobalNinjaMultiGenerator(cmake* cm)
  : cmGlobalNinjaGenerator(cm)
{
  cm->GetState()->SetIsGeneratorMultiConfig(true);
  cm->GetState()->SetNinjaMulti(true);
}

cmDocumentationEntry cmGlobalNinjaMultiGenerator::GetDocumentation()
{
  return { cmGlobalNinjaMultiGenerator::GetActualName(),
           "Generates build-<Config>.ninja files." };
}

std::string cmGlobalNinjaMultiGenerator::ExpandCFGIntDir(
  const std::string& str, const std::string& config) const
{
  std::string result = str;
  cmSystemTools::ReplaceString(result, this->GetCMakeCFGIntDir(), config);
  return result;
}

bool cmGlobalNinjaMultiGenerator::OpenBuildFileStreams()
{
  if (!this->OpenFileStream(this->CommonFileStream,
                            cmGlobalNinjaMultiGenerator::NINJA_COMMON_FILE)) {
    return false;
  }

  if (!this->OpenFileStream(this->DefaultFileStream, NINJA_BUILD_FILE)) {
    return false;
  }
  *this->DefaultFileStream << "# Build using rules for '"
                           << this->DefaultFileConfig << "'.\n\n"
                           << "include "
                           << this->NinjaOutputPath(
                                GetNinjaImplFilename(this->DefaultFileConfig))
                           << "\n\n";

  // Write a comment about this file.
  *this->CommonFileStream
    << "# This file contains build statements common to all "
       "configurations.\n\n";

  auto const& configs =
    this->Makefiles[0]->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  return std::all_of(
    configs.begin(), configs.end(), [this](std::string const& config) -> bool {
      // Open impl file.
      if (!this->OpenFileStream(this->ImplFileStreams[config],
                                GetNinjaImplFilename(config))) {
        return false;
      }

      // Write a comment about this file.
      *this->ImplFileStreams[config]
        << "# This file contains build statements specific to the \"" << config
        << "\"\n# configuration.\n\n";

      // Open config file.
      if (!this->OpenFileStream(this->ConfigFileStreams[config],
                                GetNinjaConfigFilename(config))) {
        return false;
      }

      // Write a comment about this file.
      *this->ConfigFileStreams[config]
        << "# This file contains aliases specific to the \"" << config
        << "\"\n# configuration.\n\n"
        << "include " << this->NinjaOutputPath(GetNinjaImplFilename(config))
        << "\n\n";

      return true;
    });
}

void cmGlobalNinjaMultiGenerator::CloseBuildFileStreams()
{
  if (this->CommonFileStream) {
    this->CommonFileStream.reset();
  } else {
    cmSystemTools::Error("Common file stream was not open.");
  }

  if (this->DefaultFileStream) {
    this->DefaultFileStream.reset();
  } // No error if it wasn't open

  for (auto const& config : this->Makefiles[0]->GetGeneratorConfigs(
         cmMakefile::IncludeEmptyConfig)) {
    if (this->ImplFileStreams[config]) {
      this->ImplFileStreams[config].reset();
    } else {
      cmSystemTools::Error(
        cmStrCat("Impl file stream for \"", config, "\" was not open."));
    }
    if (this->ConfigFileStreams[config]) {
      this->ConfigFileStreams[config].reset();
    } else {
      cmSystemTools::Error(
        cmStrCat("Config file stream for \"", config, "\" was not open."));
    }
  }
}

void cmGlobalNinjaMultiGenerator::AppendNinjaFileArgument(
  GeneratedMakeCommand& command, const std::string& config) const
{
  if (!config.empty()) {
    command.Add("-f");
    command.Add(GetNinjaConfigFilename(config));
  }
}

std::string cmGlobalNinjaMultiGenerator::GetNinjaImplFilename(
  const std::string& config)
{
  return cmStrCat("CMakeFiles/impl-", config,
                  cmGlobalNinjaMultiGenerator::NINJA_FILE_EXTENSION);
}

std::string cmGlobalNinjaMultiGenerator::GetNinjaConfigFilename(
  const std::string& config)
{
  return cmStrCat("build-", config,
                  cmGlobalNinjaMultiGenerator::NINJA_FILE_EXTENSION);
}

void cmGlobalNinjaMultiGenerator::AddRebuildManifestOutputs(
  cmNinjaDeps& outputs) const
{
  for (auto const& config : this->Makefiles.front()->GetGeneratorConfigs(
         cmMakefile::IncludeEmptyConfig)) {
    outputs.push_back(this->NinjaOutputPath(GetNinjaImplFilename(config)));
    outputs.push_back(this->NinjaOutputPath(GetNinjaConfigFilename(config)));
  }
  if (!this->DefaultFileConfig.empty()) {
    outputs.push_back(this->NinjaOutputPath(NINJA_BUILD_FILE));
  }
}

void cmGlobalNinjaMultiGenerator::GetQtAutoGenConfigs(
  std::vector<std::string>& configs) const
{
  auto allConfigs =
    this->Makefiles[0]->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  configs.insert(configs.end(), cm::cbegin(allConfigs), cm::cend(allConfigs));
}

bool cmGlobalNinjaMultiGenerator::InspectConfigTypeVariables()
{
  cmList configsList{ this->Makefiles.front()->GetDefinition(
    "CMAKE_CONFIGURATION_TYPES") };
  if (configsList.empty()) {
    configsList.emplace_back();
  }
  std::set<std::string> configs(configsList.cbegin(), configsList.cend());

  this->DefaultFileConfig =
    this->Makefiles.front()->GetSafeDefinition("CMAKE_DEFAULT_BUILD_TYPE");
  if (this->DefaultFileConfig.empty()) {
    this->DefaultFileConfig = configsList.front();
  }
  if (!configs.count(this->DefaultFileConfig)) {
    std::ostringstream msg;
    msg << "The configuration specified by "
        << "CMAKE_DEFAULT_BUILD_TYPE (" << this->DefaultFileConfig
        << ") is not present in CMAKE_CONFIGURATION_TYPES";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return false;
  }

  cmList crossConfigsList{ this->Makefiles.front()->GetSafeDefinition(
    "CMAKE_CROSS_CONFIGS") };
  auto crossConfigs = ListSubsetWithAll(configs, configs, crossConfigsList);
  if (!crossConfigs) {
    std::ostringstream msg;
    msg << "CMAKE_CROSS_CONFIGS is not a subset of "
        << "CMAKE_CONFIGURATION_TYPES";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return false;
  }
  this->CrossConfigs = *crossConfigs;

  auto defaultConfigsString =
    this->Makefiles.front()->GetSafeDefinition("CMAKE_DEFAULT_CONFIGS");
  if (defaultConfigsString.empty()) {
    defaultConfigsString = this->DefaultFileConfig;
  }
  if (!defaultConfigsString.empty() &&
      defaultConfigsString != this->DefaultFileConfig &&
      (this->DefaultFileConfig.empty() || this->CrossConfigs.empty())) {
    std::ostringstream msg;
    msg << "CMAKE_DEFAULT_CONFIGS cannot be used without "
        << "CMAKE_DEFAULT_BUILD_TYPE or CMAKE_CROSS_CONFIGS";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return false;
  }

  cmList defaultConfigsList(defaultConfigsString);
  if (!this->DefaultFileConfig.empty()) {
    auto defaultConfigs =
      ListSubsetWithAll(this->GetCrossConfigs(this->DefaultFileConfig),
                        this->CrossConfigs, defaultConfigsList);
    if (!defaultConfigs) {
      std::ostringstream msg;
      msg << "CMAKE_DEFAULT_CONFIGS is not a subset of CMAKE_CROSS_CONFIGS";
      this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                             msg.str());
      return false;
    }
    this->DefaultConfigs = *defaultConfigs;
  }

  return true;
}

std::string cmGlobalNinjaMultiGenerator::GetDefaultBuildConfig() const
{
  return "";
}

std::string cmGlobalNinjaMultiGenerator::OrderDependsTargetForTarget(
  cmGeneratorTarget const* target, const std::string& config) const
{
  return cmStrCat("cmake_object_order_depends_target_", target->GetName(), '_',
                  cmSystemTools::UpperCase(config));
}
