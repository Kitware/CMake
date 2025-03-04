/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLocalVisualStudioGenerator.h"

#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "windows.h"

#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmLocalVisualStudioGenerator::cmLocalVisualStudioGenerator(
  cmGlobalGenerator* gg, cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
}

cmLocalVisualStudioGenerator::~cmLocalVisualStudioGenerator() = default;

cmGlobalVisualStudioGenerator::VSVersion
cmLocalVisualStudioGenerator::GetVersion() const
{
  cmGlobalVisualStudioGenerator* gg =
    static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator);
  return gg->GetVersion();
}

void cmLocalVisualStudioGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, std::string>& mapping,
  cmGeneratorTarget const* gt)
{
  char const* custom_ext = gt->GetCustomObjectExtension();
  std::string dir_max = this->ComputeLongestObjectDirectory(gt);

  // Count the number of object files with each name.  Note that
  // windows file names are not case sensitive.
  std::map<std::string, int> counts;

  for (auto const& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectNameLower = cmSystemTools::LowerCase(
      cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath()));
    if (custom_ext) {
      objectNameLower += custom_ext;
    } else {
      objectNameLower +=
        this->GlobalGenerator->GetLanguageOutputExtension(*sf);
    }
    counts[objectNameLower] += 1;
  }

  // For all source files producing duplicate names we need unique
  // object name computation.

  for (auto& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectName =
      cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath());
    if (custom_ext) {
      objectName += custom_ext;
    } else {
      objectName += this->GlobalGenerator->GetLanguageOutputExtension(*sf);
    }
    if (counts[cmSystemTools::LowerCase(objectName)] > 1) {
      const_cast<cmGeneratorTarget*>(gt)->AddExplicitObjectName(sf);
      bool keptSourceExtension;
      objectName = this->GetObjectFileNameWithoutTarget(
        *sf, dir_max, &keptSourceExtension, custom_ext);
    }
    si.second = objectName;
  }
}

std::unique_ptr<cmCustomCommand>
cmLocalVisualStudioGenerator::MaybeCreateImplibDir(cmGeneratorTarget* target,
                                                   std::string const& config,
                                                   bool isFortran)
{
  std::unique_ptr<cmCustomCommand> pcc;

  // If an executable exports symbols then VS wants to create an
  // import library but forgets to create the output directory.
  // The Intel Fortran plugin always forgets to the directory.
  if (target->GetType() != cmStateEnums::EXECUTABLE &&
      !(isFortran && target->GetType() == cmStateEnums::SHARED_LIBRARY)) {
    return pcc;
  }
  std::string outDir =
    target->GetDirectory(config, cmStateEnums::RuntimeBinaryArtifact);
  std::string impDir =
    target->GetDirectory(config, cmStateEnums::ImportLibraryArtifact);
  if (impDir == outDir) {
    return pcc;
  }

  // Add a pre-build event to create the directory.
  cmCustomCommandLines commands = cmMakeSingleCommandLine(
    { cmSystemTools::GetCMakeCommand(), "-E", "make_directory", impDir });
  pcc = cm::make_unique<cmCustomCommand>();
  pcc->SetCommandLines(commands);
  pcc->SetStdPipesUTF8(true);
  pcc->SetEscapeOldStyle(false);
  pcc->SetEscapeAllowMakeVars(true);
  return pcc;
}

char const* cmLocalVisualStudioGenerator::ReportErrorLabel() const
{
  return ":VCReportError";
}

char const* cmLocalVisualStudioGenerator::GetReportErrorLabel() const
{
  return this->ReportErrorLabel();
}

std::string cmLocalVisualStudioGenerator::ConstructScript(
  cmCustomCommandGenerator const& ccg, std::string const& newline_text)
{
  bool useLocal = this->CustomCommandUseLocal();
  std::string workingDirectory = ccg.GetWorkingDirectory();

  // Avoid leading or trailing newlines.
  std::string newline;

  // Line to check for error between commands.
  std::string check_error;
  if (useLocal) {
    check_error = cmStrCat(newline_text, "if %errorlevel% neq 0 goto :cmEnd");
  } else {
    check_error = cmStrCat(newline_text, "if errorlevel 1 goto ",
                           this->GetReportErrorLabel());
  }

  // Store the script in a string.
  std::string script;

  // Open a local context.
  if (useLocal) {
    script = cmStrCat(newline, "setlocal");
    newline = newline_text;
  }

  if (!workingDirectory.empty()) {
    // Change the working directory.
    script = cmStrCat(script, newline, "cd ",
                      this->ConvertToOutputFormat(workingDirectory, SHELL),
                      check_error);
    newline = newline_text;

    // Change the working drive.
    if (workingDirectory.size() > 1 && workingDirectory[1] == ':') {
      script = cmStrCat(script, newline, workingDirectory[0],
                        workingDirectory[1], check_error);
      newline = newline_text;
    }
  }

  // for visual studio IDE add extra stuff to the PATH
  // if CMAKE_MSVCIDE_RUN_PATH is set.
  if (this->GetGlobalGenerator()->IsVisualStudio()) {
    cmValue extraPath =
      this->Makefile->GetDefinition("CMAKE_MSVCIDE_RUN_PATH");
    if (extraPath) {
      script = cmStrCat(script, newline, "set PATH=", *extraPath, ";%PATH%");
      newline = newline_text;
    }
  }

  // Write each command on a single line.
  for (unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c) {
    // Add this command line.
    std::string cmd = ccg.GetCommand(c);

    if (cmd.empty()) {
      continue;
    }

    // Start a new line.
    script += newline;
    newline = newline_text;

    // Use "call " before any invocations of .bat or .cmd files
    // invoked as custom commands.
    //
    std::string suffix;
    if (cmd.size() > 4) {
      suffix = cmSystemTools::LowerCase(cmd.substr(cmd.size() - 4));
      if (suffix == ".bat"_s || suffix == ".cmd"_s) {
        script += "call ";
      }
    }

    if (workingDirectory.empty()) {
      script += this->ConvertToOutputFormat(
        this->MaybeRelativeToCurBinDir(cmd), cmOutputConverter::SHELL);
    } else {
      script += this->ConvertToOutputFormat(cmd.c_str(), SHELL);
    }
    ccg.AppendArguments(c, script);

    // After each custom command, check for an error result.
    // If there was an error, jump to the VCReportError label,
    // skipping the run of any subsequent commands in this
    // sequence.
    script += check_error;
  }

  // Close the local context.
  if (useLocal) {
    // clang-format off
    script = cmStrCat(
        script
      , newline
      , ":cmEnd"
      , newline
      , "endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone"
      , newline
      , ":cmErrorLevel"
      , newline
      , "exit /b %1"
      , newline
      , ":cmDone"
      , newline
      , "if %errorlevel% neq 0 goto ", this->GetReportErrorLabel()
      );
    // clang-format on
  }

  return script;
}

std::string cmLocalVisualStudioGenerator::FinishConstructScript(
  VsProjectType projectType, std::string const& newline)
{
  bool useLocal = this->CustomCommandUseLocal();

  // Store the script in a string.
  std::string script;

  if (useLocal && projectType != VsProjectType::vcxproj) {
    // This label is not provided by MSBuild for C# projects.
    script += newline;
    script += this->GetReportErrorLabel();
  }

  return script;
}
