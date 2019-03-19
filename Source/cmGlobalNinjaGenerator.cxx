/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalNinjaGenerator.h"

#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#include "cm_jsoncpp_writer.h"
#include "cmsys/FStream.hxx"
#include <algorithm>
#include <ctype.h>
#include <iterator>
#include <memory> // IWYU pragma: keep
#include <sstream>
#include <stdio.h>

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmFortranParser.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGeneratorTarget.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmNinjaLinkLineComputer.h"
#include "cmOutputConverter.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmVersion.h"
#include "cmake.h"

class cmLinkLineComputer;

const char* cmGlobalNinjaGenerator::NINJA_BUILD_FILE = "build.ninja";
const char* cmGlobalNinjaGenerator::NINJA_RULES_FILE = "rules.ninja";
const char* cmGlobalNinjaGenerator::INDENT = "  ";
#ifdef _WIN32
std::string const cmGlobalNinjaGenerator::SHELL_NOOP = "cd .";
#else
std::string const cmGlobalNinjaGenerator::SHELL_NOOP = ":";
#endif

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

cmLinkLineComputer* cmGlobalNinjaGenerator::CreateLinkLineComputer(
  cmOutputConverter* outputConverter,
  cmStateDirectory const& /* stateDir */) const
{
  return new cmNinjaLinkLineComputer(
    outputConverter,
    this->LocalGenerators[0]->GetStateSnapshot().GetDirectory(), this);
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
      sprintf(buf, ".%02x", static_cast<unsigned int>(i));
      encoded += buf;
    }
  }
  return encoded;
}

std::string cmGlobalNinjaGenerator::EncodeLiteral(const std::string& lit)
{
  std::string result = lit;
  cmSystemTools::ReplaceString(result, "$", "$$");
  cmSystemTools::ReplaceString(result, "\n", "$\n");
  return result;
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
  result = EncodeLiteral(result);
  cmSystemTools::ReplaceString(result, " ", "$ ");
  cmSystemTools::ReplaceString(result, ":", "$:");
  return result;
}

void cmGlobalNinjaGenerator::WriteBuild(
  std::ostream& os, const std::string& comment, const std::string& rule,
  const cmNinjaDeps& outputs, const cmNinjaDeps& implicitOuts,
  const cmNinjaDeps& explicitDeps, const cmNinjaDeps& implicitDeps,
  const cmNinjaDeps& orderOnlyDeps, const cmNinjaVars& variables,
  const std::string& rspfile, int cmdLineLimit, bool* usedResponseFile)
{
  // Make sure there is a rule.
  if (rule.empty()) {
    cmSystemTools::Error("No rule for WriteBuildStatement! called "
                         "with comment: " +
                         comment);
    return;
  }

  // Make sure there is at least one output file.
  if (outputs.empty()) {
    cmSystemTools::Error("No output files for WriteBuildStatement! called "
                         "with comment: " +
                         comment);
    return;
  }

  cmGlobalNinjaGenerator::WriteComment(os, comment);

  std::string arguments;

  // TODO: Better formatting for when there are multiple input/output files.

  // Write explicit dependencies.
  for (std::string const& explicitDep : explicitDeps) {
    arguments += " " + EncodePath(explicitDep);
  }

  // Write implicit dependencies.
  if (!implicitDeps.empty()) {
    arguments += " |";
    for (std::string const& implicitDep : implicitDeps) {
      arguments += " " + EncodePath(implicitDep);
    }
  }

  // Write order-only dependencies.
  if (!orderOnlyDeps.empty()) {
    arguments += " ||";
    for (std::string const& orderOnlyDep : orderOnlyDeps) {
      arguments += " " + EncodePath(orderOnlyDep);
    }
  }

  arguments += "\n";

  std::string build;

  // Write outputs files.
  build += "build";
  for (std::string const& output : outputs) {
    build += " " + EncodePath(output);
    if (this->ComputingUnknownDependencies) {
      this->CombinedBuildOutputs.insert(output);
    }
  }
  if (!implicitOuts.empty()) {
    build += " |";
    for (std::string const& implicitOut : implicitOuts) {
      build += " " + EncodePath(implicitOut);
    }
  }
  build += ":";

  // Write the rule.
  build += " " + rule;

  // Write the variables bound to this build statement.
  std::ostringstream variable_assignments;
  for (auto const& variable : variables) {
    cmGlobalNinjaGenerator::WriteVariable(variable_assignments, variable.first,
                                          variable.second, "", 1);
  }

  // check if a response file rule should be used
  std::string buildstr = build;
  std::string assignments = variable_assignments.str();
  bool useResponseFile = false;
  if (cmdLineLimit < 0 ||
      (cmdLineLimit > 0 &&
       (arguments.size() + buildstr.size() + assignments.size() + 1000) >
         static_cast<size_t>(cmdLineLimit))) {
    variable_assignments.str(std::string());
    cmGlobalNinjaGenerator::WriteVariable(variable_assignments, "RSP_FILE",
                                          rspfile, "", 1);
    assignments += variable_assignments.str();
    useResponseFile = true;
  }
  if (usedResponseFile) {
    *usedResponseFile = useResponseFile;
  }

  os << buildstr << arguments << assignments;
}

void cmGlobalNinjaGenerator::WritePhonyBuild(
  std::ostream& os, const std::string& comment, const cmNinjaDeps& outputs,
  const cmNinjaDeps& explicitDeps, const cmNinjaDeps& implicitDeps,
  const cmNinjaDeps& orderOnlyDeps, const cmNinjaVars& variables)
{
  this->WriteBuild(os, comment, "phony", outputs,
                   /*implicitOuts=*/cmNinjaDeps(), explicitDeps, implicitDeps,
                   orderOnlyDeps, variables);
}

void cmGlobalNinjaGenerator::AddCustomCommandRule()
{
  this->AddRule("CUSTOM_COMMAND", "$COMMAND", "$DESC",
                "Rule for running custom commands.",
                /*depfile*/ "",
                /*deptype*/ "",
                /*rspfile*/ "",
                /*rspcontent*/ "",
                /*restat*/ "", // bound on each build statement as needed
                /*generator*/ false);
}

void cmGlobalNinjaGenerator::WriteCustomCommandBuild(
  const std::string& command, const std::string& description,
  const std::string& comment, const std::string& depfile, bool uses_terminal,
  bool restat, const cmNinjaDeps& outputs, const cmNinjaDeps& deps,
  const cmNinjaDeps& orderOnly)
{
  std::string cmd = command; // NOLINT(*)
#ifdef _WIN32
  if (cmd.empty())
    // TODO Shouldn't an empty command be handled by ninja?
    cmd = "cmd.exe /c";
#endif

  this->AddCustomCommandRule();

  cmNinjaVars vars;
  vars["COMMAND"] = cmd;
  vars["DESC"] = EncodeLiteral(description);
  if (restat) {
    vars["restat"] = "1";
  }
  if (uses_terminal && SupportsConsolePool()) {
    vars["pool"] = "console";
  }
  if (!depfile.empty()) {
    vars["depfile"] = depfile;
  }
  this->WriteBuild(*this->BuildFileStream, comment, "CUSTOM_COMMAND", outputs,
                   /*implicitOuts=*/cmNinjaDeps(), deps, cmNinjaDeps(),
                   orderOnly, vars);

  if (this->ComputingUnknownDependencies) {
    // we need to track every dependency that comes in, since we are trying
    // to find dependencies that are side effects of build commands
    for (std::string const& dep : deps) {
      this->CombinedCustomCommandExplicitDependencies.insert(dep);
    }
  }
}

void cmGlobalNinjaGenerator::AddMacOSXContentRule()
{
  cmLocalGenerator* lg = this->LocalGenerators[0];

  std::ostringstream cmd;
  cmd << lg->ConvertToOutputFormat(cmSystemTools::GetCMakeCommand(),
                                   cmOutputConverter::SHELL)
      << " -E copy $in $out";

  this->AddRule("COPY_OSX_CONTENT", cmd.str(), "Copying OS X Content $out",
                "Rule for copying OS X bundle content file.",
                /*depfile*/ "",
                /*deptype*/ "",
                /*rspfile*/ "",
                /*rspcontent*/ "",
                /*restat*/ "",
                /*generator*/ false);
}

void cmGlobalNinjaGenerator::WriteMacOSXContentBuild(const std::string& input,
                                                     const std::string& output)
{
  this->AddMacOSXContentRule();

  cmNinjaDeps outputs;
  outputs.push_back(output);
  cmNinjaDeps deps;
  deps.push_back(input);
  cmNinjaVars vars;

  this->WriteBuild(*this->BuildFileStream, "", "COPY_OSX_CONTENT", outputs,
                   /*implicitOuts=*/cmNinjaDeps(), deps, cmNinjaDeps(),
                   cmNinjaDeps(), cmNinjaVars());
}

void cmGlobalNinjaGenerator::WriteRule(
  std::ostream& os, const std::string& name, const std::string& command,
  const std::string& description, const std::string& comment,
  const std::string& depfile, const std::string& deptype,
  const std::string& rspfile, const std::string& rspcontent,
  const std::string& restat, bool generator)
{
  // Make sure the rule has a name.
  if (name.empty()) {
    cmSystemTools::Error("No name given for WriteRuleStatement! called "
                         "with comment: " +
                         comment);
    return;
  }

  // Make sure a command is given.
  if (command.empty()) {
    cmSystemTools::Error("No command given for WriteRuleStatement! called "
                         "with comment: " +
                         comment);
    return;
  }

  cmGlobalNinjaGenerator::WriteComment(os, comment);

  // Write the rule.
  os << "rule " << name << "\n";

  // Write the depfile if any.
  if (!depfile.empty()) {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "depfile = " << depfile << "\n";
  }

  // Write the deptype if any.
  if (!deptype.empty()) {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "deps = " << deptype << "\n";
  }

  // Write the command.
  cmGlobalNinjaGenerator::Indent(os, 1);
  os << "command = " << command << "\n";

  // Write the description if any.
  if (!description.empty()) {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "description = " << description << "\n";
  }

  if (!rspfile.empty()) {
    if (rspcontent.empty()) {
      cmSystemTools::Error("No rspfile_content given!" + comment);
      return;
    }
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "rspfile = " << rspfile << "\n";
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "rspfile_content = " << rspcontent << "\n";
  }

  if (!restat.empty()) {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "restat = " << restat << "\n";
  }

  if (generator) {
    cmGlobalNinjaGenerator::Indent(os, 1);
    os << "generator = 1\n";
  }

  os << "\n";
}

void cmGlobalNinjaGenerator::WriteVariable(std::ostream& os,
                                           const std::string& name,
                                           const std::string& value,
                                           const std::string& comment,
                                           int indent)
{
  // Make sure we have a name.
  if (name.empty()) {
    cmSystemTools::Error("No name given for WriteVariable! called "
                         "with comment: " +
                         comment);
    return;
  }

  // Do not add a variable if the value is empty.
  std::string val = cmSystemTools::TrimWhitespace(value);
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
  , BuildFileStream(nullptr)
  , RulesFileStream(nullptr)
  , CompileCommandsStream(nullptr)
  , UsingGCCOnWindows(false)
  , ComputingUnknownDependencies(false)
  , PolicyCMP0058(cmPolicies::WARN)
  , NinjaSupportsConsolePool(false)
  , NinjaSupportsImplicitOuts(false)
  , NinjaSupportsManifestRestat(false)
  , NinjaSupportsMultilineDepfile(false)
  , NinjaSupportsDyndeps(0)
{
#ifdef _WIN32
  cm->GetState()->SetWindowsShell(true);
#endif
  // // Ninja is not ported to non-Unix OS yet.
  // this->ForceUnixPaths = true;
  this->FindMakeProgramFile = "CMakeNinjaFindMake.cmake";
}

// Virtual public methods.

cmLocalGenerator* cmGlobalNinjaGenerator::CreateLocalGenerator(cmMakefile* mf)
{
  return new cmLocalNinjaGenerator(this, mf);
}

codecvt::Encoding cmGlobalNinjaGenerator::GetMakefileEncoding() const
{
#ifdef _WIN32
  // Ninja on Windows does not support non-ANSI characters.
  // https://github.com/ninja-build/ninja/issues/1195
  return codecvt::ANSI;
#else
  // No encoding conversion needed on other platforms.
  return codecvt::None;
#endif
}

void cmGlobalNinjaGenerator::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalNinjaGenerator::GetActualName();
  entry.Brief = "Generates build.ninja files.";
}

// Implemented in all cmGlobaleGenerator sub-classes.
// Used in:
//   Source/cmLocalGenerator.cxx
//   Source/cmake.cxx
void cmGlobalNinjaGenerator::Generate()
{
  // Check minimum Ninja version.
  if (cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
                                    this->NinjaVersion.c_str(),
                                    RequiredNinjaVersion().c_str())) {
    std::ostringstream msg;
    msg << "The detected version of Ninja (" << this->NinjaVersion;
    msg << ") is less than the version of Ninja required by CMake (";
    msg << cmGlobalNinjaGenerator::RequiredNinjaVersion() << ").";
    this->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                           msg.str());
    return;
  }
  this->OpenBuildFileStream();
  this->OpenRulesFileStream();

  this->TargetDependsClosures.clear();

  this->InitOutputPathPrefix();
  this->TargetAll = this->NinjaOutputPath("all");
  this->CMakeCacheFile = this->NinjaOutputPath("CMakeCache.txt");

  this->PolicyCMP0058 =
    this->LocalGenerators[0]->GetMakefile()->GetPolicyStatus(
      cmPolicies::CMP0058);
  this->ComputingUnknownDependencies =
    (this->PolicyCMP0058 == cmPolicies::OLD ||
     this->PolicyCMP0058 == cmPolicies::WARN);

  this->cmGlobalGenerator::Generate();

  this->WriteAssumedSourceDependencies();
  this->WriteTargetAliases(*this->BuildFileStream);
  this->WriteFolderTargets(*this->BuildFileStream);
  this->WriteUnknownExplicitDependencies(*this->BuildFileStream);
  this->WriteBuiltinTargets(*this->BuildFileStream);

  if (cmSystemTools::GetErrorOccuredFlag()) {
    this->RulesFileStream->setstate(std::ios::failbit);
    this->BuildFileStream->setstate(std::ios::failbit);
  }

  this->CloseCompileCommandsStream();
  this->CloseRulesFileStream();
  this->CloseBuildFileStream();
}

bool cmGlobalNinjaGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (!this->cmGlobalGenerator::FindMakeProgram(mf)) {
    return false;
  }
  if (const char* ninjaCommand = mf->GetDefinition("CMAKE_MAKE_PROGRAM")) {
    this->NinjaCommand = ninjaCommand;
    std::vector<std::string> command;
    command.push_back(this->NinjaCommand);
    command.emplace_back("--version");
    std::string version;
    std::string error;
    if (!cmSystemTools::RunSingleCommand(command, &version, &error, nullptr,
                                         nullptr,
                                         cmSystemTools::OUTPUT_NONE)) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       "Running\n '" + cmJoin(command, "' '") +
                         "'\n"
                         "failed with:\n " +
                         error);
      cmSystemTools::SetFatalErrorOccured();
      return false;
    }
    this->NinjaVersion = cmSystemTools::TrimWhitespace(version);
    this->CheckNinjaFeatures();
  }
  return true;
}

void cmGlobalNinjaGenerator::CheckNinjaFeatures()
{
  this->NinjaSupportsConsolePool = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion.c_str(),
    RequiredNinjaVersionForConsolePool().c_str());
  this->NinjaSupportsImplicitOuts = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion.c_str(),
    cmGlobalNinjaGenerator::RequiredNinjaVersionForImplicitOuts().c_str());
  this->NinjaSupportsManifestRestat = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion.c_str(),
    RequiredNinjaVersionForManifestRestat().c_str());
  this->NinjaSupportsMultilineDepfile = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NinjaVersion.c_str(),
    RequiredNinjaVersionForMultilineDepfile().c_str());
  {
    // Our ninja branch adds ".dyndep-#" to its version number,
    // where '#' is a feature-specific version number.  Extract it.
    static std::string const k_DYNDEP_ = ".dyndep-";
    std::string::size_type pos = this->NinjaVersion.find(k_DYNDEP_);
    if (pos != std::string::npos) {
      const char* fv = this->NinjaVersion.c_str() + pos + k_DYNDEP_.size();
      cmSystemTools::StringToULong(fv, &this->NinjaSupportsDyndeps);
    }
  }
}

bool cmGlobalNinjaGenerator::CheckLanguages(
  std::vector<std::string> const& languages, cmMakefile* mf) const
{
  if (std::find(languages.begin(), languages.end(), "Fortran") !=
      languages.end()) {
    return this->CheckFortran(mf);
  }
  return true;
}

bool cmGlobalNinjaGenerator::CheckFortran(cmMakefile* mf) const
{
  if (this->NinjaSupportsDyndeps == 1) {
    return true;
  }

  std::ostringstream e;
  if (this->NinjaSupportsDyndeps == 0) {
    /* clang-format off */
    e <<
      "The Ninja generator does not support Fortran using Ninja version\n"
      "  " + this->NinjaVersion + "\n"
      "due to lack of required features.  "
      "Kitware has implemented the required features but as of this version "
      "of CMake they have not been integrated to upstream ninja.  "
      "Pending integration, Kitware maintains a branch at:\n"
      "  https://github.com/Kitware/ninja/tree/features-for-fortran#readme\n"
      "with the required features.  "
      "One may build ninja from that branch to get support for Fortran."
      ;
    /* clang-format on */
  } else {
    /* clang-format off */
    e <<
      "The Ninja generator in this version of CMake does not support Fortran "
      "using Ninja version\n"
      "  " + this->NinjaVersion + "\n"
      "because its 'dyndep' feature version is " <<
      this->NinjaSupportsDyndeps << ".  "
      "This version of CMake is aware only of 'dyndep' feature version 1."
      ;
    /* clang-format on */
  }
  mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
  cmSystemTools::SetFatalErrorOccured();
  return false;
}

void cmGlobalNinjaGenerator::EnableLanguage(
  std::vector<std::string> const& langs, cmMakefile* mf, bool optional)
{
  this->cmGlobalGenerator::EnableLanguage(langs, mf, optional);
  for (std::string const& l : langs) {
    if (l == "NONE") {
      continue;
    }
    this->ResolveLanguageCompiler(l, mf, optional);
  }
#ifdef _WIN32
  if ((mf->GetSafeDefinition("CMAKE_C_SIMULATE_ID") != "MSVC") &&
      (mf->GetSafeDefinition("CMAKE_CXX_SIMULATE_ID") != "MSVC") &&
      (mf->IsOn("CMAKE_COMPILER_IS_MINGW") ||
       (mf->GetSafeDefinition("CMAKE_C_COMPILER_ID") == "GNU") ||
       (mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID") == "GNU") ||
       (mf->GetSafeDefinition("CMAKE_C_COMPILER_ID") == "Clang") ||
       (mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID") == "Clang"))) {
    this->UsingGCCOnWindows = true;
  }
#endif
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
  std::vector<std::string> const& targetNames, const std::string& /*config*/,
  bool /*fast*/, int jobs, bool verbose,
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

  makeCommand.Add(makeOptions.begin(), makeOptions.end());
  for (const auto& tname : targetNames) {
    if (!tname.empty()) {
      if (tname == "clean") {
        makeCommand.Add("-t", "clean");
      } else {
        makeCommand.Add(tname);
      }
    }
  }
  return { std::move(makeCommand) };
}

// Non-virtual public methods.

void cmGlobalNinjaGenerator::AddRule(
  const std::string& name, const std::string& command,
  const std::string& description, const std::string& comment,
  const std::string& depfile, const std::string& deptype,
  const std::string& rspfile, const std::string& rspcontent,
  const std::string& restat, bool generator)
{
  // Do not add the same rule twice.
  if (this->HasRule(name)) {
    return;
  }

  this->Rules.insert(name);
  cmGlobalNinjaGenerator::WriteRule(*this->RulesFileStream, name, command,
                                    description, comment, depfile, deptype,
                                    rspfile, rspcontent, restat, generator);

  this->RuleCmdLength[name] = static_cast<int>(command.size());
}

bool cmGlobalNinjaGenerator::HasRule(const std::string& name)
{
  RulesSetType::const_iterator rule = this->Rules.find(name);
  return (rule != this->Rules.end());
}

// Private virtual overrides

std::string cmGlobalNinjaGenerator::GetEditCacheCommand() const
{
  // Ninja by design does not run interactive tools in the terminal,
  // so our only choice is cmake-gui.
  return cmSystemTools::GetCMakeGUICommand();
}

void cmGlobalNinjaGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir;
  dir += gt->LocalGenerator->GetCurrentBinaryDirectory();
  dir += "/";
  dir += gt->LocalGenerator->GetTargetDirectory(gt);
  dir += "/";
  gt->ObjectDirectory = dir;
}

// Private methods

void cmGlobalNinjaGenerator::OpenBuildFileStream()
{
  // Compute Ninja's build file path.
  std::string buildFilePath =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  buildFilePath += "/";
  buildFilePath += cmGlobalNinjaGenerator::NINJA_BUILD_FILE;

  // Get a stream where to generate things.
  if (!this->BuildFileStream) {
    this->BuildFileStream = new cmGeneratedFileStream(
      buildFilePath, false, this->GetMakefileEncoding());
    if (!this->BuildFileStream) {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return;
    }
  }

  // Write the do not edit header.
  this->WriteDisclaimer(*this->BuildFileStream);

  // Write a comment about this file.
  *this->BuildFileStream
    << "# This file contains all the build statements describing the\n"
    << "# compilation DAG.\n\n";
}

void cmGlobalNinjaGenerator::CloseBuildFileStream()
{
  if (this->BuildFileStream) {
    delete this->BuildFileStream;
    this->BuildFileStream = nullptr;
  } else {
    cmSystemTools::Error("Build file stream was not open.");
  }
}

void cmGlobalNinjaGenerator::OpenRulesFileStream()
{
  // Compute Ninja's build file path.
  std::string rulesFilePath =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  rulesFilePath += "/";
  rulesFilePath += cmGlobalNinjaGenerator::NINJA_RULES_FILE;

  // Get a stream where to generate things.
  if (!this->RulesFileStream) {
    this->RulesFileStream = new cmGeneratedFileStream(
      rulesFilePath, false, this->GetMakefileEncoding());
    if (!this->RulesFileStream) {
      // An error message is generated by the constructor if it cannot
      // open the file.
      return;
    }
  }

  // Write the do not edit header.
  this->WriteDisclaimer(*this->RulesFileStream);

  // Write comment about this file.
  /* clang-format off */
  *this->RulesFileStream
    << "# This file contains all the rules used to get the outputs files\n"
    << "# built from the input files.\n"
    << "# It is included in the main '" << NINJA_BUILD_FILE << "'.\n\n"
    ;
  /* clang-format on */
}

void cmGlobalNinjaGenerator::CloseRulesFileStream()
{
  if (this->RulesFileStream) {
    delete this->RulesFileStream;
    this->RulesFileStream = nullptr;
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
  auto const f = ConvertToNinjaPathCache.find(path);
  if (f != ConvertToNinjaPathCache.end()) {
    return f->second;
  }

  cmLocalNinjaGenerator* ng =
    static_cast<cmLocalNinjaGenerator*>(this->LocalGenerators[0]);
  std::string const& bin_dir = ng->GetState()->GetBinaryDirectory();
  std::string convPath = ng->MaybeConvertToRelativePath(bin_dir, path);
  convPath = this->NinjaOutputPath(convPath);
#ifdef _WIN32
  std::replace(convPath.begin(), convPath.end(), '/', '\\');
#endif
  return ConvertToNinjaPathCache.emplace(path, std::move(convPath))
    .first->second;
}

void cmGlobalNinjaGenerator::AddCXXCompileCommand(
  const std::string& commandLine, const std::string& sourceFile)
{
  // Compute Ninja's build file path.
  std::string buildFileDir =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  if (!this->CompileCommandsStream) {
    std::string buildFilePath = buildFileDir + "/compile_commands.json";
    if (this->ComputingUnknownDependencies) {
      this->CombinedBuildOutputs.insert(
        this->NinjaOutputPath("compile_commands.json"));
    }

    // Get a stream where to generate things.
    this->CompileCommandsStream = new cmGeneratedFileStream(buildFilePath);
    *this->CompileCommandsStream << "[";
  } else {
    *this->CompileCommandsStream << "," << std::endl;
  }

  std::string sourceFileName = sourceFile;
  if (!cmSystemTools::FileIsFullPath(sourceFileName)) {
    sourceFileName = cmSystemTools::CollapseFullPath(
      sourceFileName, this->GetCMakeInstance()->GetHomeOutputDirectory());
  }

  /* clang-format off */
  *this->CompileCommandsStream << "\n{\n"
     << "  \"directory\": \""
     << cmGlobalGenerator::EscapeJSON(buildFileDir) << "\",\n"
     << "  \"command\": \""
     << cmGlobalGenerator::EscapeJSON(commandLine) << "\",\n"
     << "  \"file\": \""
     << cmGlobalGenerator::EscapeJSON(sourceFileName) << "\"\n"
     << "}";
  /* clang-format on */
}

void cmGlobalNinjaGenerator::CloseCompileCommandsStream()
{
  if (this->CompileCommandsStream) {
    *this->CompileCommandsStream << "\n]";
    delete this->CompileCommandsStream;
    this->CompileCommandsStream = nullptr;
  }
}

void cmGlobalNinjaGenerator::WriteDisclaimer(std::ostream& os)
{
  os << "# CMAKE generated file: DO NOT EDIT!\n"
     << "# Generated by \"" << this->GetName() << "\""
     << " Generator, CMake Version " << cmVersion::GetMajorVersion() << "."
     << cmVersion::GetMinorVersion() << "\n\n";
}

void cmGlobalNinjaGenerator::AddDependencyToAll(cmGeneratorTarget* target)
{
  this->AppendTargetOutputs(target, this->AllDependencies);
}

void cmGlobalNinjaGenerator::AddDependencyToAll(const std::string& input)
{
  this->AllDependencies.push_back(input);
}

void cmGlobalNinjaGenerator::WriteAssumedSourceDependencies()
{
  for (auto const& asd : this->AssumedSourceDependencies) {
    cmNinjaDeps orderOnlyDeps;
    std::copy(asd.second.begin(), asd.second.end(),
              std::back_inserter(orderOnlyDeps));
    WriteCustomCommandBuild(/*command=*/"", /*description=*/"",
                            "Assume dependencies for generated source file.",
                            /*depfile*/ "", /*uses_terminal*/ false,
                            /*restat*/ true, cmNinjaDeps(1, asd.first),
                            cmNinjaDeps(), orderOnlyDeps);
  }
}

std::string OrderDependsTargetForTarget(cmGeneratorTarget const* target)
{
  return "cmake_object_order_depends_target_" + target->GetName();
}

void cmGlobalNinjaGenerator::AppendTargetOutputs(
  cmGeneratorTarget const* target, cmNinjaDeps& outputs,
  cmNinjaTargetDepends depends)
{
  std::string configName =
    target->Target->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE");

  // for frameworks, we want the real name, not smple name
  // frameworks always appear versioned, and the build.ninja
  // will always attempt to manage symbolic links instead
  // of letting cmOSXBundleGenerator do it.
  bool realname = target->IsFrameworkOnApple();

  switch (target->GetType()) {
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY: {
      if (depends == DependOnTargetOrdering) {
        outputs.push_back(OrderDependsTargetForTarget(target));
        break;
      }
    }
    // FALLTHROUGH
    case cmStateEnums::EXECUTABLE: {
      outputs.push_back(this->ConvertToNinjaPath(target->GetFullPath(
        configName, cmStateEnums::RuntimeBinaryArtifact, realname)));
      break;
    }
    case cmStateEnums::OBJECT_LIBRARY: {
      if (depends == DependOnTargetOrdering) {
        outputs.push_back(OrderDependsTargetForTarget(target));
        break;
      }
    }
    // FALLTHROUGH
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::UTILITY: {
      std::string path =
        target->GetLocalGenerator()->GetCurrentBinaryDirectory() +
        std::string("/") + target->GetName();
      outputs.push_back(this->ConvertToNinjaPath(path));
      break;
    }

    default:
      return;
  }
}

void cmGlobalNinjaGenerator::AppendTargetDepends(
  cmGeneratorTarget const* target, cmNinjaDeps& outputs,
  cmNinjaTargetDepends depends)
{
  if (target->GetType() == cmStateEnums::GLOBAL_TARGET) {
    // These depend only on other CMake-provided targets, e.g. "all".
    std::set<BT<std::string>> const& utils = target->GetUtilities();
    for (BT<std::string> const& util : utils) {
      std::string d =
        target->GetLocalGenerator()->GetCurrentBinaryDirectory() + "/" +
        util.Value;
      outputs.push_back(this->ConvertToNinjaPath(d));
    }
  } else {
    cmNinjaDeps outs;
    cmTargetDependSet const& targetDeps = this->GetTargetDirectDepends(target);
    for (cmTargetDepend const& targetDep : targetDeps) {
      if (targetDep->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }
      this->AppendTargetOutputs(targetDep, outs, depends);
    }
    std::sort(outs.begin(), outs.end());
    outputs.insert(outputs.end(), outs.begin(), outs.end());
  }
}

void cmGlobalNinjaGenerator::AppendTargetDependsClosure(
  cmGeneratorTarget const* target, cmNinjaDeps& outputs)
{
  cmNinjaOuts outs;
  this->AppendTargetDependsClosure(target, outs, true);

  outputs.insert(outputs.end(), outs.begin(), outs.end());
}

void cmGlobalNinjaGenerator::AppendTargetDependsClosure(
  cmGeneratorTarget const* target, cmNinjaOuts& outputs, bool omit_self)
{

  // try to locate the target in the cache
  auto find = this->TargetDependsClosures.lower_bound(target);

  if (find == this->TargetDependsClosures.end() || find->first != target) {
    // We now calculate the closure outputs by inspecting the dependent
    // targets recursively.
    // For that we have to distinguish between a local result set that is only
    // relevant for filling the cache entries properly isolated and a global
    // result set that is relevant for the result of the top level call to
    // AppendTargetDependsClosure.
    auto const& targetDeps = this->GetTargetDirectDepends(target);
    cmNinjaOuts this_outs; // this will be the new cache entry

    for (auto const& dep_target : targetDeps) {
      if (dep_target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        continue;
      }

      // Collect the dependent targets for _this_ target
      this->AppendTargetDependsClosure(dep_target, this_outs, false);
    }
    find = this->TargetDependsClosures.emplace_hint(find, target,
                                                    std::move(this_outs));
  }

  // now fill the outputs of the final result from the newly generated cache
  // entry
  outputs.insert(find->second.begin(), find->second.end());

  // finally generate the outputs of the target itself, if applicable
  cmNinjaDeps outs;
  if (!omit_self) {
    this->AppendTargetOutputs(target, outs);
  }
  outputs.insert(outs.begin(), outs.end());
}

void cmGlobalNinjaGenerator::AddTargetAlias(const std::string& alias,
                                            cmGeneratorTarget* target)
{
  std::string buildAlias = this->NinjaOutputPath(alias);
  cmNinjaDeps outputs;
  this->AppendTargetOutputs(target, outputs);
  // Mark the target's outputs as ambiguous to ensure that no other target uses
  // the output as an alias.
  for (std::string const& output : outputs) {
    TargetAliases[output] = nullptr;
  }

  // Insert the alias into the map.  If the alias was already present in the
  // map and referred to another target, mark it as ambiguous.
  std::pair<TargetAliasMap::iterator, bool> newAlias =
    TargetAliases.insert(std::make_pair(buildAlias, target));
  if (newAlias.second && newAlias.first->second != target) {
    newAlias.first->second = nullptr;
  }
}

void cmGlobalNinjaGenerator::WriteTargetAliases(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Target aliases.\n\n";

  for (auto const& ta : TargetAliases) {
    // Don't write ambiguous aliases.
    if (!ta.second) {
      continue;
    }

    // Don't write alias if there is a already a custom command with
    // matching output
    if (this->HasCustomCommandOutput(ta.first)) {
      continue;
    }

    cmNinjaDeps deps;
    this->AppendTargetOutputs(ta.second, deps);

    this->WritePhonyBuild(os, "", cmNinjaDeps(1, ta.first), deps);
  }
}

void cmGlobalNinjaGenerator::WriteFolderTargets(std::ostream& os)
{
  cmGlobalNinjaGenerator::WriteDivider(os);
  os << "# Folder targets.\n\n";

  std::map<std::string, cmNinjaDeps> targetsPerFolder;
  for (cmLocalGenerator const* lg : this->LocalGenerators) {
    const std::string currentBinaryFolder(
      lg->GetStateSnapshot().GetDirectory().GetCurrentBinary());
    // The directory-level rule should depend on the target-level rules
    // for all targets in the directory.
    targetsPerFolder[currentBinaryFolder] = cmNinjaDeps();
    for (auto gt : lg->GetGeneratorTargets()) {
      cmStateEnums::TargetType const type = gt->GetType();
      if ((type == cmStateEnums::EXECUTABLE ||
           type == cmStateEnums::STATIC_LIBRARY ||
           type == cmStateEnums::SHARED_LIBRARY ||
           type == cmStateEnums::MODULE_LIBRARY ||
           type == cmStateEnums::OBJECT_LIBRARY ||
           type == cmStateEnums::UTILITY) &&
          !gt->GetPropertyAsBool("EXCLUDE_FROM_ALL")) {
        targetsPerFolder[currentBinaryFolder].push_back(gt->GetName());
      }
    }

    // The directory-level rule should depend on the directory-level
    // rules of the subdirectories.
    std::vector<cmStateSnapshot> const& children =
      lg->GetStateSnapshot().GetChildren();
    for (cmStateSnapshot const& state : children) {
      std::string const currentBinaryDir =
        state.GetDirectory().GetCurrentBinary();

      targetsPerFolder[currentBinaryFolder].push_back(
        this->ConvertToNinjaPath(currentBinaryDir + "/all"));
    }
  }

  std::string const rootBinaryDir =
    this->LocalGenerators[0]->GetBinaryDirectory();
  for (auto const& it : targetsPerFolder) {
    cmGlobalNinjaGenerator::WriteDivider(os);
    std::string const& currentBinaryDir = it.first;

    // Do not generate a rule for the root binary dir.
    if (rootBinaryDir.length() >= currentBinaryDir.length()) {
      continue;
    }

    std::string const comment = "Folder: " + currentBinaryDir;
    cmNinjaDeps output(1);
    output.push_back(this->ConvertToNinjaPath(currentBinaryDir + "/all"));

    this->WritePhonyBuild(os, comment, output, it.second);
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

  for (cmLocalGenerator* lg : this->LocalGenerators) {
    // get the vector of files created by this makefile and convert them
    // to ninja paths, which are all relative in respect to the build directory
    const std::vector<std::string>& files =
      lg->GetMakefile()->GetOutputFiles();
    for (std::string const& file : files) {
      knownDependencies.insert(this->ConvertToNinjaPath(file));
    }
    if (!this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
      // get list files which are implicit dependencies as well and will be
      // phony for rebuild manifest
      std::vector<std::string> const& lf = lg->GetMakefile()->GetListFiles();
      for (std::string const& j : lf) {
        knownDependencies.insert(this->ConvertToNinjaPath(j));
      }
    }
    std::vector<cmGeneratorExpressionEvaluationFile*> const& ef =
      lg->GetMakefile()->GetEvaluationFiles();
    for (cmGeneratorExpressionEvaluationFile* li : ef) {
      // get all the files created by generator expressions and convert them
      // to ninja paths
      std::vector<std::string> evaluationFiles = li->GetFiles();
      for (std::string const& evaluationFile : evaluationFiles) {
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

  std::string const rootBuildDirectory =
    this->GetCMakeInstance()->GetHomeOutputDirectory();
  bool const inSourceBuild =
    (rootBuildDirectory == this->GetCMakeInstance()->GetHomeDirectory());
  std::vector<std::string> warnExplicitDepends;
  for (std::string const& i : unknownExplicitDepends) {
    // verify the file is in the build directory
    std::string const absDepPath =
      cmSystemTools::CollapseFullPath(i, rootBuildDirectory.c_str());
    bool const inBuildDir =
      cmSystemTools::IsSubDirectory(absDepPath, rootBuildDirectory);
    if (inBuildDir) {
      cmNinjaDeps deps(1, i);
      this->WritePhonyBuild(os, "", deps, cmNinjaDeps());
      if (this->PolicyCMP0058 == cmPolicies::WARN && !inSourceBuild &&
          warnExplicitDepends.size() < 10) {
        warnExplicitDepends.push_back(i);
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

  this->WriteTargetAll(os);
  this->WriteTargetRebuildManifest(os);
  this->WriteTargetClean(os);
  this->WriteTargetHelp(os);
}

void cmGlobalNinjaGenerator::WriteTargetAll(std::ostream& os)
{
  cmNinjaDeps outputs;
  outputs.push_back(this->TargetAll);

  this->WritePhonyBuild(os, "The main all target.", outputs,
                        this->AllDependencies);

  if (!this->HasOutputPathPrefix()) {
    cmGlobalNinjaGenerator::WriteDefault(os, outputs,
                                         "Make the all target the default.");
  }
}

void cmGlobalNinjaGenerator::WriteTargetRebuildManifest(std::ostream& os)
{
  if (this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    return;
  }
  cmLocalGenerator* lg = this->LocalGenerators[0];

  std::ostringstream cmd;
  cmd << lg->ConvertToOutputFormat(cmSystemTools::GetCMakeCommand(),
                                   cmOutputConverter::SHELL)
      << " -S"
      << lg->ConvertToOutputFormat(lg->GetSourceDirectory(),
                                   cmOutputConverter::SHELL)
      << " -B"
      << lg->ConvertToOutputFormat(lg->GetBinaryDirectory(),
                                   cmOutputConverter::SHELL);
  WriteRule(*this->RulesFileStream, "RERUN_CMAKE", cmd.str(),
            "Re-running CMake...", "Rule for re-running cmake.",
            /*depfile=*/"",
            /*deptype=*/"",
            /*rspfile=*/"",
            /*rspcontent*/ "",
            /*restat=*/"",
            /*generator=*/true);

  cmNinjaDeps implicitDeps;
  cmNinjaDeps explicitDeps;
  for (cmLocalGenerator* localGen : this->LocalGenerators) {
    std::vector<std::string> const& lf =
      localGen->GetMakefile()->GetListFiles();
    for (std::string const& fi : lf) {
      implicitDeps.push_back(this->ConvertToNinjaPath(fi));
    }
  }
  implicitDeps.push_back(this->CMakeCacheFile);

  cmNinjaVars variables;
  // Use 'console' pool to get non buffered output of the CMake re-run call
  // Available since Ninja 1.5
  if (SupportsConsolePool()) {
    variables["pool"] = "console";
  }

  cmake* cm = this->GetCMakeInstance();
  if (this->SupportsManifestRestat() && cm->DoWriteGlobVerifyTarget()) {
    std::ostringstream verify_cmd;
    verify_cmd << lg->ConvertToOutputFormat(cmSystemTools::GetCMakeCommand(),
                                            cmOutputConverter::SHELL)
               << " -P "
               << lg->ConvertToOutputFormat(cm->GetGlobVerifyScript(),
                                            cmOutputConverter::SHELL);

    WriteRule(*this->RulesFileStream, "VERIFY_GLOBS", verify_cmd.str(),
              "Re-checking globbed directories...",
              "Rule for re-checking globbed directories.",
              /*depfile=*/"",
              /*deptype=*/"",
              /*rspfile=*/"",
              /*rspcontent*/ "",
              /*restat=*/"",
              /*generator=*/true);

    std::string verifyForce = cm->GetGlobVerifyScript() + "_force";
    cmNinjaDeps verifyForceDeps(1, this->NinjaOutputPath(verifyForce));

    this->WritePhonyBuild(os, "Phony target to force glob verification run.",
                          verifyForceDeps, cmNinjaDeps());

    variables["restat"] = "1";
    std::string const verifyScriptFile =
      this->NinjaOutputPath(cm->GetGlobVerifyScript());
    std::string const verifyStampFile =
      this->NinjaOutputPath(cm->GetGlobVerifyStamp());
    this->WriteBuild(os,
                     "Re-run CMake to check if globbed directories changed.",
                     "VERIFY_GLOBS",
                     /*outputs=*/cmNinjaDeps(1, verifyStampFile),
                     /*implicitOuts=*/cmNinjaDeps(),
                     /*explicitDeps=*/cmNinjaDeps(),
                     /*implicitDeps=*/verifyForceDeps,
                     /*orderOnlyDeps=*/cmNinjaDeps(), variables);

    variables.erase("restat");
    implicitDeps.push_back(verifyScriptFile);
    explicitDeps.push_back(verifyStampFile);
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

  std::sort(implicitDeps.begin(), implicitDeps.end());
  implicitDeps.erase(std::unique(implicitDeps.begin(), implicitDeps.end()),
                     implicitDeps.end());

  std::string const ninjaBuildFile = this->NinjaOutputPath(NINJA_BUILD_FILE);
  this->WriteBuild(os, "Re-run CMake if any of its inputs changed.",
                   "RERUN_CMAKE",
                   /*outputs=*/cmNinjaDeps(1, ninjaBuildFile),
                   /*implicitOuts=*/cmNinjaDeps(), explicitDeps, implicitDeps,
                   /*orderOnlyDeps=*/cmNinjaDeps(), variables);

  cmNinjaDeps missingInputs;
  std::set_difference(std::make_move_iterator(implicitDeps.begin()),
                      std::make_move_iterator(implicitDeps.end()),
                      CustomCommandOutputs.begin(), CustomCommandOutputs.end(),
                      std::back_inserter(missingInputs));

  this->WritePhonyBuild(os, "A missing CMake input file is not an error.",
                        missingInputs, cmNinjaDeps());
}

std::string cmGlobalNinjaGenerator::ninjaCmd() const
{
  cmLocalGenerator* lgen = this->LocalGenerators[0];
  if (lgen) {
    return lgen->ConvertToOutputFormat(this->NinjaCommand,
                                       cmOutputConverter::SHELL);
  }
  return "ninja";
}

bool cmGlobalNinjaGenerator::SupportsConsolePool() const
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

void cmGlobalNinjaGenerator::WriteTargetClean(std::ostream& os)
{
  WriteRule(*this->RulesFileStream, "CLEAN", ninjaCmd() + " -t clean",
            "Cleaning all built files...",
            "Rule for cleaning all built files.",
            /*depfile=*/"",
            /*deptype=*/"",
            /*rspfile=*/"",
            /*rspcontent*/ "",
            /*restat=*/"",
            /*generator=*/false);
  WriteBuild(os, "Clean all the built files.", "CLEAN",
             /*outputs=*/cmNinjaDeps(1, this->NinjaOutputPath("clean")),
             /*implicitOuts=*/cmNinjaDeps(),
             /*explicitDeps=*/cmNinjaDeps(),
             /*implicitDeps=*/cmNinjaDeps(),
             /*orderOnlyDeps=*/cmNinjaDeps(),
             /*variables=*/cmNinjaVars());
}

void cmGlobalNinjaGenerator::WriteTargetHelp(std::ostream& os)
{
  WriteRule(*this->RulesFileStream, "HELP", ninjaCmd() + " -t targets",
            "All primary targets available:",
            "Rule for printing all primary targets available.",
            /*depfile=*/"",
            /*deptype=*/"",
            /*rspfile=*/"",
            /*rspcontent*/ "",
            /*restat=*/"",
            /*generator=*/false);
  WriteBuild(os, "Print all primary targets available.", "HELP",
             /*outputs=*/cmNinjaDeps(1, this->NinjaOutputPath("help")),
             /*implicitOuts=*/cmNinjaDeps(),
             /*explicitDeps=*/cmNinjaDeps(),
             /*implicitDeps=*/cmNinjaDeps(),
             /*orderOnlyDeps=*/cmNinjaDeps(),
             /*variables=*/cmNinjaVars());
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
  return this->OutputPathPrefix + path;
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
                  --tdi=FortranDependInfo.json --pp=$out --dep=$DEP_FILE \
                  --obj=$OBJ_FILE --ddi=$DYNDEP_INTERMEDIATE_FILE \
                  --lang=Fortran

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

struct cmSourceInfo
{
  // Set of provided and required modules.
  std::set<std::string> Provides;
  std::set<std::string> Requires;

  // Set of files included in the translation unit.
  std::set<std::string> Includes;
};

static std::unique_ptr<cmSourceInfo> cmcmd_cmake_ninja_depends_fortran(
  std::string const& arg_tdi, std::string const& arg_pp);

int cmcmd_cmake_ninja_depends(std::vector<std::string>::const_iterator argBeg,
                              std::vector<std::string>::const_iterator argEnd)
{
  std::string arg_tdi;
  std::string arg_pp;
  std::string arg_dep;
  std::string arg_obj;
  std::string arg_ddi;
  std::string arg_lang;
  for (std::string const& arg : cmMakeRange(argBeg, argEnd)) {
    if (cmHasLiteralPrefix(arg, "--tdi=")) {
      arg_tdi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--pp=")) {
      arg_pp = arg.substr(5);
    } else if (cmHasLiteralPrefix(arg, "--dep=")) {
      arg_dep = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--obj=")) {
      arg_obj = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--ddi=")) {
      arg_ddi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--lang=")) {
      arg_lang = arg.substr(7);
    } else {
      cmSystemTools::Error("-E cmake_ninja_depends unknown argument: " + arg);
      return 1;
    }
  }
  if (arg_tdi.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --tdi=");
    return 1;
  }
  if (arg_pp.empty()) {
    cmSystemTools::Error("-E cmake_ninja_depends requires value for --pp=");
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

  std::unique_ptr<cmSourceInfo> info;
  if (arg_lang == "Fortran") {
    info = cmcmd_cmake_ninja_depends_fortran(arg_tdi, arg_pp);
  } else {
    cmSystemTools::Error("-E cmake_ninja_depends does not understand the " +
                         arg_lang + " language");
    return 1;
  }

  if (!info) {
    // The error message is already expected to have been output.
    return 1;
  }

  {
    cmGeneratedFileStream depfile(arg_dep);
    depfile << cmSystemTools::ConvertToUnixOutputPath(arg_pp) << ":";
    for (std::string const& include : info->Includes) {
      depfile << " \\\n " << cmSystemTools::ConvertToUnixOutputPath(include);
    }
    depfile << "\n";
  }

  Json::Value ddi(Json::objectValue);
  ddi["object"] = arg_obj;

  Json::Value& ddi_provides = ddi["provides"] = Json::arrayValue;
  for (std::string const& provide : info->Provides) {
    ddi_provides.append(provide);
  }
  Json::Value& ddi_requires = ddi["requires"] = Json::arrayValue;
  for (std::string const& r : info->Requires) {
    // Require modules not provided in the same source.
    if (!info->Provides.count(r)) {
      ddi_requires.append(r);
    }
  }

  cmGeneratedFileStream ddif(arg_ddi);
  ddif << ddi;
  if (!ddif) {
    cmSystemTools::Error("-E cmake_ninja_depends failed to write " + arg_ddi);
    return 1;
  }
  return 0;
}

std::unique_ptr<cmSourceInfo> cmcmd_cmake_ninja_depends_fortran(
  std::string const& arg_tdi, std::string const& arg_pp)
{
  cmFortranCompiler fc;
  std::vector<std::string> includes;
  {
    Json::Value tdio;
    Json::Value const& tdi = tdio;
    {
      cmsys::ifstream tdif(arg_tdi.c_str(), std::ios::in | std::ios::binary);
      Json::Reader reader;
      if (!reader.parse(tdif, tdio, false)) {
        cmSystemTools::Error("-E cmake_ninja_depends failed to parse " +
                             arg_tdi + reader.getFormattedErrorMessages());
        return nullptr;
      }
    }

    Json::Value const& tdi_include_dirs = tdi["include-dirs"];
    if (tdi_include_dirs.isArray()) {
      for (auto const& tdi_include_dir : tdi_include_dirs) {
        includes.push_back(tdi_include_dir.asString());
      }
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
  if (!cmFortranParser_FilePush(&parser, arg_pp.c_str())) {
    cmSystemTools::Error("-E cmake_ninja_depends failed to open " + arg_pp);
    return nullptr;
  }
  if (cmFortran_yyparse(parser.Scanner) != 0) {
    // Failed to parse the file.
    return nullptr;
  }

  auto info = cm::make_unique<cmSourceInfo>();
  info->Provides = finfo.Provides;
  info->Requires = finfo.Requires;
  info->Includes = finfo.Includes;
  return info;
}

struct cmDyndepObjectInfo
{
  std::string Object;
  std::vector<std::string> Provides;
  std::vector<std::string> Requires;
};

bool cmGlobalNinjaGenerator::WriteDyndepFile(
  std::string const& dir_top_src, std::string const& dir_top_bld,
  std::string const& dir_cur_src, std::string const& dir_cur_bld,
  std::string const& arg_dd, std::vector<std::string> const& arg_ddis,
  std::string const& module_dir,
  std::vector<std::string> const& linked_target_dirs,
  std::string const& arg_lang)
{
  // Setup path conversions.
  {
    cmStateSnapshot snapshot = this->GetCMakeInstance()->GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentSource(dir_cur_src);
    snapshot.GetDirectory().SetCurrentBinary(dir_cur_bld);
    snapshot.GetDirectory().SetRelativePathTopSource(dir_top_src.c_str());
    snapshot.GetDirectory().SetRelativePathTopBinary(dir_top_bld.c_str());
    auto mfd = cm::make_unique<cmMakefile>(this, snapshot);
    std::unique_ptr<cmLocalNinjaGenerator> lgd(
      static_cast<cmLocalNinjaGenerator*>(
        this->CreateLocalGenerator(mfd.get())));
    this->Makefiles.push_back(mfd.release());
    this->LocalGenerators.push_back(lgd.release());
  }

  std::vector<cmDyndepObjectInfo> objects;
  for (std::string const& arg_ddi : arg_ddis) {
    // Load the ddi file and compute the module file paths it provides.
    Json::Value ddio;
    Json::Value const& ddi = ddio;
    cmsys::ifstream ddif(arg_ddi.c_str(), std::ios::in | std::ios::binary);
    Json::Reader reader;
    if (!reader.parse(ddif, ddio, false)) {
      cmSystemTools::Error("-E cmake_ninja_dyndep failed to parse " + arg_ddi +
                           reader.getFormattedErrorMessages());
      return false;
    }

    cmDyndepObjectInfo info;
    info.Object = ddi["object"].asString();
    Json::Value const& ddi_provides = ddi["provides"];
    if (ddi_provides.isArray()) {
      for (auto const& ddi_provide : ddi_provides) {
        info.Provides.push_back(ddi_provide.asString());
      }
    }
    Json::Value const& ddi_requires = ddi["requires"];
    if (ddi_requires.isArray()) {
      for (auto const& ddi_require : ddi_requires) {
        info.Requires.push_back(ddi_require.asString());
      }
    }
    objects.push_back(std::move(info));
  }

  // Map from module name to module file path, if known.
  std::map<std::string, std::string> mod_files;

  // Populate the module map with those provided by linked targets first.
  for (std::string const& linked_target_dir : linked_target_dirs) {
    std::string const ltmn =
      linked_target_dir + "/" + arg_lang + "Modules.json";
    Json::Value ltm;
    cmsys::ifstream ltmf(ltmn.c_str(), std::ios::in | std::ios::binary);
    Json::Reader reader;
    if (ltmf && !reader.parse(ltmf, ltm, false)) {
      cmSystemTools::Error("-E cmake_ninja_dyndep failed to parse " +
                           linked_target_dir +
                           reader.getFormattedErrorMessages());
      return false;
    }
    if (ltm.isObject()) {
      for (Json::Value::iterator i = ltm.begin(); i != ltm.end(); ++i) {
        mod_files[i.key().asString()] = i->asString();
      }
    }
  }

  // Extend the module map with those provided by this target.
  // We do this after loading the modules provided by linked targets
  // in case we have one of the same name that must be preferred.
  Json::Value tm = Json::objectValue;
  for (cmDyndepObjectInfo const& object : objects) {
    for (std::string const& p : object.Provides) {
      std::string const mod = module_dir + p;
      mod_files[p] = mod;
      tm[p] = mod;
    }
  }

  cmGeneratedFileStream ddf(arg_dd);
  ddf << "ninja_dyndep_version = 1.0\n";

  for (cmDyndepObjectInfo const& object : objects) {
    std::string const ddComment;
    std::string const ddRule = "dyndep";
    cmNinjaDeps ddOutputs;
    cmNinjaDeps ddImplicitOuts;
    cmNinjaDeps ddExplicitDeps;
    cmNinjaDeps ddImplicitDeps;
    cmNinjaDeps ddOrderOnlyDeps;
    cmNinjaVars ddVars;

    ddOutputs.push_back(object.Object);
    for (std::string const& p : object.Provides) {
      ddImplicitOuts.push_back(this->ConvertToNinjaPath(mod_files[p]));
    }
    for (std::string const& r : object.Requires) {
      std::map<std::string, std::string>::iterator m = mod_files.find(r);
      if (m != mod_files.end()) {
        ddImplicitDeps.push_back(this->ConvertToNinjaPath(m->second));
      }
    }
    if (!object.Provides.empty()) {
      ddVars["restat"] = "1";
    }

    this->WriteBuild(ddf, ddComment, ddRule, ddOutputs, ddImplicitOuts,
                     ddExplicitDeps, ddImplicitDeps, ddOrderOnlyDeps, ddVars);
  }

  // Store the map of modules provided by this target in a file for
  // use by dependents that reference this target in linked-target-dirs.
  std::string const target_mods_file =
    cmSystemTools::GetFilenamePath(arg_dd) + "/" + arg_lang + "Modules.json";
  cmGeneratedFileStream tmf(target_mods_file);
  tmf << tm;

  return true;
}

int cmcmd_cmake_ninja_dyndep(std::vector<std::string>::const_iterator argBeg,
                             std::vector<std::string>::const_iterator argEnd)
{
  std::vector<std::string> arg_full =
    cmSystemTools::HandleResponseFile(argBeg, argEnd);

  std::string arg_dd;
  std::string arg_lang;
  std::string arg_tdi;
  std::vector<std::string> arg_ddis;
  for (std::string const& arg : arg_full) {
    if (cmHasLiteralPrefix(arg, "--tdi=")) {
      arg_tdi = arg.substr(6);
    } else if (cmHasLiteralPrefix(arg, "--lang=")) {
      arg_lang = arg.substr(7);
    } else if (cmHasLiteralPrefix(arg, "--dd=")) {
      arg_dd = arg.substr(5);
    } else if (!cmHasLiteralPrefix(arg, "--") &&
               cmHasLiteralSuffix(arg, ".ddi")) {
      arg_ddis.push_back(arg);
    } else {
      cmSystemTools::Error("-E cmake_ninja_dyndep unknown argument: " + arg);
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
      cmSystemTools::Error("-E cmake_ninja_dyndep failed to parse " + arg_tdi +
                           reader.getFormattedErrorMessages());
      return 1;
    }
  }

  std::string const dir_cur_bld = tdi["dir-cur-bld"].asString();
  std::string const dir_cur_src = tdi["dir-cur-src"].asString();
  std::string const dir_top_bld = tdi["dir-top-bld"].asString();
  std::string const dir_top_src = tdi["dir-top-src"].asString();
  std::string module_dir = tdi["module-dir"].asString();
  if (!module_dir.empty() && !cmHasLiteralSuffix(module_dir, "/")) {
    module_dir += "/";
  }
  std::vector<std::string> linked_target_dirs;
  Json::Value const& tdi_linked_target_dirs = tdi["linked-target-dirs"];
  if (tdi_linked_target_dirs.isArray()) {
    for (auto const& tdi_linked_target_dir : tdi_linked_target_dirs) {
      linked_target_dirs.push_back(tdi_linked_target_dir.asString());
    }
  }

  cmake cm(cmake::RoleInternal, cmState::Unknown);
  cm.SetHomeDirectory(dir_top_src);
  cm.SetHomeOutputDirectory(dir_top_bld);
  std::unique_ptr<cmGlobalNinjaGenerator> ggd(
    static_cast<cmGlobalNinjaGenerator*>(cm.CreateGlobalGenerator("Ninja")));
  if (!ggd ||
      !ggd->WriteDyndepFile(dir_top_src, dir_top_bld, dir_cur_src, dir_cur_bld,
                            arg_dd, arg_ddis, module_dir, linked_target_dirs,
                            arg_lang)) {
    return 1;
  }
  return 0;
}
