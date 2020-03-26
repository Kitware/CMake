/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCustomCommandGenerator.h"

#include <cstddef>
#include <memory>
#include <utility>

#include <cmext/algorithm>

#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
void AppendPaths(const std::vector<std::string>& inputs,
                 cmGeneratorExpression const& ge, cmLocalGenerator* lg,
                 std::string const& config, std::vector<std::string>& output)
{
  for (std::string const& in : inputs) {
    std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(in);
    std::vector<std::string> result =
      cmExpandedList(cge->Evaluate(lg, config));
    for (std::string& it : result) {
      cmSystemTools::ConvertToUnixSlashes(it);
      if (cmSystemTools::FileIsFullPath(it)) {
        it = cmSystemTools::CollapseFullPath(
          it, lg->GetMakefile()->GetHomeOutputDirectory());
      }
    }
    cm::append(output, result);
  }
}
}

cmCustomCommandGenerator::cmCustomCommandGenerator(cmCustomCommand const& cc,
                                                   std::string config,
                                                   cmLocalGenerator* lg)
  : CC(cc)
  , Config(std::move(config))
  , LG(lg)
  , OldStyle(cc.GetEscapeOldStyle())
  , MakeVars(cc.GetEscapeAllowMakeVars())
  , EmulatorsWithArguments(cc.GetCommandLines().size())
{
  cmGeneratorExpression ge(cc.GetBacktrace());

  const cmCustomCommandLines& cmdlines = this->CC.GetCommandLines();
  for (cmCustomCommandLine const& cmdline : cmdlines) {
    cmCustomCommandLine argv;
    for (std::string const& clarg : cmdline) {
      std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(clarg);
      std::string parsed_arg = cge->Evaluate(this->LG, this->Config);
      if (this->CC.GetCommandExpandLists()) {
        cm::append(argv, cmExpandedList(parsed_arg));
      } else {
        argv.push_back(std::move(parsed_arg));
      }
    }

    // Later code assumes at least one entry exists, but expanding
    // lists on an empty command may have left this empty.
    // FIXME: Should we define behavior for removing empty commands?
    if (argv.empty()) {
      argv.emplace_back();
    }

    this->CommandLines.push_back(std::move(argv));
  }

  AppendPaths(cc.GetByproducts(), ge, this->LG, this->Config,
              this->Byproducts);
  AppendPaths(cc.GetDepends(), ge, this->LG, this->Config, this->Depends);

  const std::string& workingdirectory = this->CC.GetWorkingDirectory();
  if (!workingdirectory.empty()) {
    std::unique_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(workingdirectory);
    this->WorkingDirectory = cge->Evaluate(this->LG, this->Config);
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
  return static_cast<unsigned int>(this->CC.GetCommandLines().size());
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

      const char* emulator_property =
        target->GetProperty("CROSSCOMPILING_EMULATOR");
      if (!emulator_property) {
        continue;
      }

      cmExpandList(emulator_property, this->EmulatorsWithArguments[c]);
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
    return target->GetLocation(this->Config).c_str();
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

std::string escapeForShellOldStyle(const std::string& str)
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

const char* cmCustomCommandGenerator::GetComment() const
{
  return this->CC.GetComment();
}

std::string cmCustomCommandGenerator::GetWorkingDirectory() const
{
  return this->WorkingDirectory;
}

std::vector<std::string> const& cmCustomCommandGenerator::GetOutputs() const
{
  return this->CC.GetOutputs();
}

std::vector<std::string> const& cmCustomCommandGenerator::GetByproducts() const
{
  return this->Byproducts;
}

std::vector<std::string> const& cmCustomCommandGenerator::GetDepends() const
{
  return this->Depends;
}
