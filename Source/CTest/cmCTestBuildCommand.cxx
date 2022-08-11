/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestBuildCommand.h"

#include <sstream>

#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestBuildHandler.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

class cmExecutionStatus;

void cmCTestBuildCommand::BindArguments()
{
  this->cmCTestHandlerCommand::BindArguments();
  this->Bind("NUMBER_ERRORS"_s, this->NumberErrors);
  this->Bind("NUMBER_WARNINGS"_s, this->NumberWarnings);
  this->Bind("TARGET"_s, this->Target);
  this->Bind("CONFIGURATION"_s, this->Configuration);
  this->Bind("FLAGS"_s, this->Flags);
  this->Bind("PROJECT_NAME"_s, this->ProjectName);
  this->Bind("PARALLEL_LEVEL"_s, this->ParallelLevel);
}

cmCTestBuildCommand::~cmCTestBuildCommand() = default;

cmCTestGenericHandler* cmCTestBuildCommand::InitializeHandler()
{
  cmCTestBuildHandler* handler = this->CTest->GetBuildHandler();
  handler->Initialize();

  this->Handler = handler;

  cmValue ctestBuildCommand =
    this->Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if (cmNonempty(ctestBuildCommand)) {
    this->CTest->SetCTestConfiguration("MakeCommand", *ctestBuildCommand,
                                       this->Quiet);
  } else {
    cmValue cmakeGeneratorName =
      this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");

    // Build configuration is determined by: CONFIGURATION argument,
    // or CTEST_BUILD_CONFIGURATION script variable, or
    // CTEST_CONFIGURATION_TYPE script variable, or ctest -C command
    // line argument... in that order.
    //
    cmValue ctestBuildConfiguration =
      this->Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    std::string cmakeBuildConfiguration = cmNonempty(this->Configuration)
      ? this->Configuration
      : cmNonempty(ctestBuildConfiguration) ? *ctestBuildConfiguration
                                            : this->CTest->GetConfigType();

    const std::string& cmakeBuildAdditionalFlags = cmNonempty(this->Flags)
      ? this->Flags
      : this->Makefile->GetSafeDefinition("CTEST_BUILD_FLAGS");
    const std::string& cmakeBuildTarget = cmNonempty(this->Target)
      ? this->Target
      : this->Makefile->GetSafeDefinition("CTEST_BUILD_TARGET");

    if (cmNonempty(cmakeGeneratorName)) {
      if (cmakeBuildConfiguration.empty()) {
        cmakeBuildConfiguration = "Release";
      }
      if (this->GlobalGenerator) {
        if (this->GlobalGenerator->GetName() != *cmakeGeneratorName) {
          this->GlobalGenerator.reset();
        }
      }
      if (!this->GlobalGenerator) {
        this->GlobalGenerator =
          this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
            *cmakeGeneratorName);
        if (!this->GlobalGenerator) {
          std::string e = cmStrCat("could not create generator named \"",
                                   *cmakeGeneratorName, '"');
          this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
          cmSystemTools::SetFatalErrorOccurred();
          return nullptr;
        }
      }
      if (cmakeBuildConfiguration.empty()) {
#ifdef CMAKE_INTDIR
        cmakeBuildConfiguration = CMAKE_INTDIR;
#else
        cmakeBuildConfiguration = "Debug";
#endif
      }

      std::string dir = this->CTest->GetCTestConfiguration("BuildDirectory");
      std::string buildCommand =
        this->GlobalGenerator->GenerateCMakeBuildCommand(
          cmakeBuildTarget, cmakeBuildConfiguration, this->ParallelLevel,
          cmakeBuildAdditionalFlags, this->Makefile->IgnoreErrorsCMP0061());
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "SetMakeCommand:" << buildCommand << "\n",
                         this->Quiet);
      this->CTest->SetCTestConfiguration("MakeCommand", buildCommand,
                                         this->Quiet);
    } else {
      std::ostringstream ostr;
      /* clang-format off */
      ostr << "has no project to build. If this is a "
        "\"built with CMake\" project, verify that CTEST_CMAKE_GENERATOR "
        "is set. Otherwise, set CTEST_BUILD_COMMAND to build the project "
        "with a custom command line.";
      /* clang-format on */
      this->SetError(ostr.str());
      return nullptr;
    }
  }

  if (cmValue useLaunchers =
        this->Makefile->GetDefinition("CTEST_USE_LAUNCHERS")) {
    this->CTest->SetCTestConfiguration("UseLaunchers", *useLaunchers,
                                       this->Quiet);
  }

  if (cmValue labelsForSubprojects =
        this->Makefile->GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, this->Quiet);
  }

  handler->SetQuiet(this->Quiet);
  return handler;
}

bool cmCTestBuildCommand::InitialPass(std::vector<std::string> const& args,
                                      cmExecutionStatus& status)
{
  bool ret = this->cmCTestHandlerCommand::InitialPass(args, status);
  if (!this->NumberErrors.empty()) {
    this->Makefile->AddDefinition(
      this->NumberErrors, std::to_string(this->Handler->GetTotalErrors()));
  }
  if (!this->NumberWarnings.empty()) {
    this->Makefile->AddDefinition(
      this->NumberWarnings, std::to_string(this->Handler->GetTotalWarnings()));
  }
  return ret;
}
