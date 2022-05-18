/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCustomCommandGenerator.h"

#include <cstddef>
#include <memory>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmCryptoHash.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTransformDepfile.h"
#include "cmValue.h"

namespace {
std::string EvaluateSplitConfigGenex(
  cm::string_view input, cmGeneratorExpression const& ge, cmLocalGenerator* lg,
  bool useOutputConfig, std::string const& outputConfig,
  std::string const& commandConfig, cmGeneratorTarget const* target,
  std::set<BT<std::pair<std::string, bool>>>* utils = nullptr)
{
  std::string result;

  while (!input.empty()) {
    // Copy non-genex content directly to the result.
    std::string::size_type pos = input.find("$<");
    result += input.substr(0, pos);
    if (pos == std::string::npos) {
      break;
    }
    input = input.substr(pos);

    // Find the balanced end of this regex.
    size_t nestingLevel = 1;
    for (pos = 2; pos < input.size(); ++pos) {
      cm::string_view cur = input.substr(pos);
      if (cmHasLiteralPrefix(cur, "$<")) {
        ++nestingLevel;
        ++pos;
        continue;
      }
      if (cmHasLiteralPrefix(cur, ">")) {
        --nestingLevel;
        if (nestingLevel == 0) {
          ++pos;
          break;
        }
      }
    }

    // Split this genex from following input.
    cm::string_view genex = input.substr(0, pos);
    input = input.substr(pos);

    // Convert an outer COMMAND_CONFIG or OUTPUT_CONFIG to the matching config.
    std::string const* config =
      useOutputConfig ? &outputConfig : &commandConfig;
    if (nestingLevel == 0) {
      static cm::string_view const COMMAND_CONFIG = "$<COMMAND_CONFIG:"_s;
      static cm::string_view const OUTPUT_CONFIG = "$<OUTPUT_CONFIG:"_s;
      if (cmHasPrefix(genex, COMMAND_CONFIG)) {
        genex.remove_prefix(COMMAND_CONFIG.size());
        genex.remove_suffix(1);
        useOutputConfig = false;
        config = &commandConfig;
      } else if (cmHasPrefix(genex, OUTPUT_CONFIG)) {
        genex.remove_prefix(OUTPUT_CONFIG.size());
        genex.remove_suffix(1);
        useOutputConfig = true;
        config = &outputConfig;
      }
    }

    // Evaluate this genex in the selected configuration.
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(std::string(genex));
    result += cge->Evaluate(lg, *config, target);

    // Record targets referenced by the genex.
    if (utils) {
      // Use a cross-dependency if we referenced the command config.
      bool const cross = !useOutputConfig;
      for (cmGeneratorTarget* gt : cge->GetTargets()) {
        utils->emplace(BT<std::pair<std::string, bool>>(
          { gt->GetName(), cross }, cge->GetBacktrace()));
      }
    }
  }

  return result;
}

std::vector<std::string> EvaluateDepends(std::vector<std::string> const& paths,
                                         cmGeneratorExpression const& ge,
                                         cmLocalGenerator* lg,
                                         std::string const& outputConfig,
                                         std::string const& commandConfig)
{
  std::vector<std::string> depends;
  for (std::string const& p : paths) {
    std::string const& ep =
      EvaluateSplitConfigGenex(p, ge, lg, /*useOutputConfig=*/true,
                               /*outputConfig=*/outputConfig,
                               /*commandConfig=*/commandConfig,
                               /*target=*/nullptr);
    cm::append(depends, cmExpandedList(ep));
  }
  for (std::string& p : depends) {
    if (cmSystemTools::FileIsFullPath(p)) {
      p = cmSystemTools::CollapseFullPath(p);
    } else {
      cmSystemTools::ConvertToUnixSlashes(p);
    }
  }
  return depends;
}

std::vector<std::string> EvaluateOutputs(std::vector<std::string> const& paths,
                                         cmGeneratorExpression const& ge,
                                         cmLocalGenerator* lg,
                                         std::string const& config)
{
  std::vector<std::string> outputs;
  for (std::string const& p : paths) {
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(p);
    cm::append(outputs, lg->ExpandCustomCommandOutputPaths(*cge, config));
  }
  return outputs;
}

std::string EvaluateDepfile(std::string const& path,
                            cmGeneratorExpression const& ge,
                            cmLocalGenerator* lg, std::string const& config)
{
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(path);
  return cge->Evaluate(lg, config);
}
}

cmCustomCommandGenerator::cmCustomCommandGenerator(
  cmCustomCommand const& cc, std::string config, cmLocalGenerator* lg,
  bool transformDepfile, cm::optional<std::string> crossConfig,
  std::function<std::string(const std::string&, const std::string&)>
    computeInternalDepfile)
  : CC(&cc)
  , OutputConfig(crossConfig ? *crossConfig : config)
  , CommandConfig(std::move(config))
  , Target(cc.GetTarget())
  , LG(lg)
  , OldStyle(cc.GetEscapeOldStyle())
  , MakeVars(cc.GetEscapeAllowMakeVars())
  , EmulatorsWithArguments(cc.GetCommandLines().size())
  , ComputeInternalDepfile(std::move(computeInternalDepfile))
{
  if (!this->ComputeInternalDepfile) {
    this->ComputeInternalDepfile =
      [this](const std::string& cfg, const std::string& file) -> std::string {
      return this->GetInternalDepfileName(cfg, file);
    };
  }

  cmGeneratorExpression ge(cc.GetBacktrace());
  cmGeneratorTarget const* target{ lg->FindGeneratorTargetToUse(
    this->Target) };

  bool const distinctConfigs = this->OutputConfig != this->CommandConfig;

  const cmCustomCommandLines& cmdlines = this->CC->GetCommandLines();
  for (cmCustomCommandLine const& cmdline : cmdlines) {
    cmCustomCommandLine argv;
    // For the command itself, we default to the COMMAND_CONFIG.
    bool useOutputConfig = false;
    for (std::string const& clarg : cmdline) {
      std::string parsed_arg = EvaluateSplitConfigGenex(
        clarg, ge, this->LG, useOutputConfig, this->OutputConfig,
        this->CommandConfig, target, &this->Utilities);
      if (this->CC->GetCommandExpandLists()) {
        cm::append(argv, cmExpandedList(parsed_arg));
      } else {
        argv.push_back(std::move(parsed_arg));
      }

      if (distinctConfigs) {
        // For remaining arguments, we default to the OUTPUT_CONFIG.
        useOutputConfig = true;
      }
    }

    if (!argv.empty()) {
      // If the command references an executable target by name,
      // collect the target to add a target-level dependency on it.
      cmGeneratorTarget* gt = this->LG->FindGeneratorTargetToUse(argv.front());
      if (gt && gt->GetType() == cmStateEnums::EXECUTABLE) {
        // GetArgv0Location uses the command config, so use a cross-dependency.
        bool const cross = true;
        this->Utilities.emplace(BT<std::pair<std::string, bool>>(
          { gt->GetName(), cross }, cc.GetBacktrace()));
      }
    } else {
      // Later code assumes at least one entry exists, but expanding
      // lists on an empty command may have left this empty.
      // FIXME: Should we define behavior for removing empty commands?
      argv.emplace_back();
    }

    this->CommandLines.push_back(std::move(argv));
  }

  if (transformDepfile && !this->CommandLines.empty() &&
      !cc.GetDepfile().empty() &&
      this->LG->GetGlobalGenerator()->DepfileFormat()) {
    cmCustomCommandLine argv;
    argv.push_back(cmSystemTools::GetCMakeCommand());
    argv.emplace_back("-E");
    argv.emplace_back("cmake_transform_depfile");
    argv.push_back(this->LG->GetGlobalGenerator()->GetName());
    switch (*this->LG->GetGlobalGenerator()->DepfileFormat()) {
      case cmDepfileFormat::GccDepfile:
        argv.emplace_back("gccdepfile");
        break;
      case cmDepfileFormat::MakeDepfile:
        argv.emplace_back("makedepfile");
        break;
      case cmDepfileFormat::MSBuildAdditionalInputs:
        argv.emplace_back("MSBuildAdditionalInputs");
        break;
    }
    argv.push_back(this->LG->GetSourceDirectory());
    argv.push_back(this->LG->GetCurrentSourceDirectory());
    argv.push_back(this->LG->GetBinaryDirectory());
    argv.push_back(this->LG->GetCurrentBinaryDirectory());
    argv.push_back(this->GetFullDepfile());
    argv.push_back(this->GetInternalDepfile());

    this->CommandLines.push_back(std::move(argv));
  }

  this->Outputs =
    EvaluateOutputs(cc.GetOutputs(), ge, this->LG, this->OutputConfig);
  this->Byproducts =
    EvaluateOutputs(cc.GetByproducts(), ge, this->LG, this->OutputConfig);
  this->Depends = EvaluateDepends(cc.GetDepends(), ge, this->LG,
                                  this->OutputConfig, this->CommandConfig);

  const std::string& workingdirectory = this->CC->GetWorkingDirectory();
  if (!workingdirectory.empty()) {
    this->WorkingDirectory = EvaluateSplitConfigGenex(
      workingdirectory, ge, this->LG, true, this->OutputConfig,
      this->CommandConfig, target);
    // Convert working directory to a full path.
    if (!this->WorkingDirectory.empty()) {
      std::string const& build_dir = this->LG->GetCurrentBinaryDirectory();
      this->WorkingDirectory =
        cmSystemTools::CollapseFullPath(this->WorkingDirectory, build_dir);
    }
  }

  this->FillEmulatorsWithArguments();
}

unsigned int cmCustomCommandGenerator::GetNumberOfCommands() const
{
  return static_cast<unsigned int>(this->CommandLines.size());
}

void cmCustomCommandGenerator::FillEmulatorsWithArguments()
{
  if (!this->LG->GetMakefile()->IsOn("CMAKE_CROSSCOMPILING")) {
    return;
  }

  for (unsigned int c = 0; c < this->GetNumberOfCommands(); ++c) {
    std::string const& argv0 = this->CommandLines[c][0];
    cmGeneratorTarget* target = this->LG->FindGeneratorTargetToUse(argv0);
    if (target && target->GetType() == cmStateEnums::EXECUTABLE &&
        !target->IsImported()) {

      cmValue emulator_property =
        target->GetProperty("CROSSCOMPILING_EMULATOR");
      if (!emulator_property) {
        continue;
      }

      cmExpandList(*emulator_property, this->EmulatorsWithArguments[c]);
    }
  }
}

std::vector<std::string> cmCustomCommandGenerator::GetCrossCompilingEmulator(
  unsigned int c) const
{
  if (c >= this->EmulatorsWithArguments.size()) {
    return std::vector<std::string>();
  }
  return this->EmulatorsWithArguments[c];
}

const char* cmCustomCommandGenerator::GetArgv0Location(unsigned int c) const
{
  std::string const& argv0 = this->CommandLines[c][0];
  cmGeneratorTarget* target = this->LG->FindGeneratorTargetToUse(argv0);
  if (target && target->GetType() == cmStateEnums::EXECUTABLE &&
      (target->IsImported() ||
       target->GetProperty("CROSSCOMPILING_EMULATOR") ||
       !this->LG->GetMakefile()->IsOn("CMAKE_CROSSCOMPILING"))) {
    return target->GetLocation(this->CommandConfig).c_str();
  }
  return nullptr;
}

bool cmCustomCommandGenerator::HasOnlyEmptyCommandLines() const
{
  for (size_t i = 0; i < this->CommandLines.size(); ++i) {
    for (size_t j = 0; j < this->CommandLines[i].size(); ++j) {
      if (!this->CommandLines[i][j].empty()) {
        return false;
      }
    }
  }
  return true;
}

std::string cmCustomCommandGenerator::GetCommand(unsigned int c) const
{
  std::vector<std::string> emulator = this->GetCrossCompilingEmulator(c);
  if (!emulator.empty()) {
    return emulator[0];
  }
  if (const char* location = this->GetArgv0Location(c)) {
    return std::string(location);
  }

  return this->CommandLines[c][0];
}

static std::string escapeForShellOldStyle(const std::string& str)
{
  std::string result;
#if defined(_WIN32) && !defined(__CYGWIN__)
  // if there are spaces
  std::string temp = str;
  if (temp.find(" ") != std::string::npos &&
      temp.find("\"") == std::string::npos) {
    result = cmStrCat('"', str, '"');
    return result;
  }
  return str;
#else
  for (const char* ch = str.c_str(); *ch != '\0'; ++ch) {
    if (*ch == ' ') {
      result += '\\';
    }
    result += *ch;
  }
  return result;
#endif
}

void cmCustomCommandGenerator::AppendArguments(unsigned int c,
                                               std::string& cmd) const
{
  unsigned int offset = 1;
  std::vector<std::string> emulator = this->GetCrossCompilingEmulator(c);
  if (!emulator.empty()) {
    for (unsigned j = 1; j < emulator.size(); ++j) {
      cmd += " ";
      if (this->OldStyle) {
        cmd += escapeForShellOldStyle(emulator[j]);
      } else {
        cmd +=
          this->LG->EscapeForShell(emulator[j], this->MakeVars, false, false,
                                   this->MakeVars && this->LG->IsNinjaMulti());
      }
    }

    offset = 0;
  }

  cmCustomCommandLine const& commandLine = this->CommandLines[c];
  for (unsigned int j = offset; j < commandLine.size(); ++j) {
    std::string arg;
    if (const char* location = j == 0 ? this->GetArgv0Location(c) : nullptr) {
      // GetCommand returned the emulator instead of the argv0 location,
      // so transform the latter now.
      arg = location;
    } else {
      arg = commandLine[j];
    }
    cmd += " ";
    if (this->OldStyle) {
      cmd += escapeForShellOldStyle(arg);
    } else {
      cmd +=
        this->LG->EscapeForShell(arg, this->MakeVars, false, false,
                                 this->MakeVars && this->LG->IsNinjaMulti());
    }
  }
}

std::string cmCustomCommandGenerator::GetDepfile() const
{
  const auto& depfile = this->CC->GetDepfile();
  if (depfile.empty()) {
    return "";
  }

  cmGeneratorExpression ge(this->CC->GetBacktrace());
  return EvaluateDepfile(depfile, ge, this->LG, this->OutputConfig);
}

std::string cmCustomCommandGenerator::GetFullDepfile() const
{
  std::string depfile = this->GetDepfile();
  if (depfile.empty()) {
    return "";
  }

  if (!cmSystemTools::FileIsFullPath(depfile)) {
    depfile = cmStrCat(this->LG->GetCurrentBinaryDirectory(), '/', depfile);
  }
  return cmSystemTools::CollapseFullPath(depfile);
}

std::string cmCustomCommandGenerator::GetInternalDepfileName(
  const std::string& /*config*/, const std::string& depfile)
{
  cmCryptoHash hash(cmCryptoHash::AlgoSHA256);
  std::string extension;
  switch (*this->LG->GetGlobalGenerator()->DepfileFormat()) {
    case cmDepfileFormat::GccDepfile:
    case cmDepfileFormat::MakeDepfile:
      extension = ".d";
      break;
    case cmDepfileFormat::MSBuildAdditionalInputs:
      extension = ".AdditionalInputs";
      break;
  }
  return cmStrCat(this->LG->GetBinaryDirectory(), "/CMakeFiles/d/",
                  hash.HashString(depfile), extension);
}

std::string cmCustomCommandGenerator::GetInternalDepfile() const
{
  std::string depfile = this->GetFullDepfile();
  if (depfile.empty()) {
    return "";
  }

  return this->ComputeInternalDepfile(this->OutputConfig, depfile);
}

const char* cmCustomCommandGenerator::GetComment() const
{
  return this->CC->GetComment();
}

std::string cmCustomCommandGenerator::GetWorkingDirectory() const
{
  return this->WorkingDirectory;
}

std::vector<std::string> const& cmCustomCommandGenerator::GetOutputs() const
{
  return this->Outputs;
}

std::vector<std::string> const& cmCustomCommandGenerator::GetByproducts() const
{
  return this->Byproducts;
}

std::vector<std::string> const& cmCustomCommandGenerator::GetDepends() const
{
  return this->Depends;
}

std::set<BT<std::pair<std::string, bool>>> const&
cmCustomCommandGenerator::GetUtilities() const
{
  return this->Utilities;
}
