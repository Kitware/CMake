/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestBuildCommand.h"

#include <sstream>
#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmCTest.h"
#include "cmCTestBuildHandler.h"
#include "cmCTestGenericHandler.h"
#include "cmCommand.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

std::unique_ptr<cmCommand> cmCTestBuildCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestBuildCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

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

std::unique_ptr<cmCTestGenericHandler> cmCTestBuildCommand::InitializeHandler()
{
  auto const& args = *this;
  auto handler = cm::make_unique<cmCTestBuildHandler>(this->CTest);

  cmValue ctestBuildCommand =
    this->Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if (cmNonempty(ctestBuildCommand)) {
    this->CTest->SetCTestConfiguration("MakeCommand", *ctestBuildCommand,
                                       args.Quiet);
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
    std::string cmakeBuildConfiguration = cmNonempty(args.Configuration)
      ? args.Configuration
      : cmNonempty(ctestBuildConfiguration) ? *ctestBuildConfiguration
                                            : this->CTest->GetConfigType();

    const std::string& cmakeBuildAdditionalFlags = cmNonempty(args.Flags)
      ? args.Flags
      : this->Makefile->GetSafeDefinition("CTEST_BUILD_FLAGS");
    const std::string& cmakeBuildTarget = cmNonempty(args.Target)
      ? args.Target
      : this->Makefile->GetSafeDefinition("CTEST_BUILD_TARGET");

    if (cmNonempty(cmakeGeneratorName)) {
      if (cmakeBuildConfiguration.empty()) {
        cmakeBuildConfiguration = "Release";
      }

      auto globalGenerator =
        this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
          *cmakeGeneratorName);
      if (!globalGenerator) {
        std::string e = cmStrCat("could not create generator named \"",
                                 *cmakeGeneratorName, '"');
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
        cmSystemTools::SetFatalErrorOccurred();
        return nullptr;
      }
      if (cmakeBuildConfiguration.empty()) {
        cmakeBuildConfiguration = "Debug";
      }

      std::string dir = this->CTest->GetCTestConfiguration("BuildDirectory");
      std::string buildCommand = globalGenerator->GenerateCMakeBuildCommand(
        cmakeBuildTarget, cmakeBuildConfiguration, args.ParallelLevel,
        cmakeBuildAdditionalFlags, this->Makefile->IgnoreErrorsCMP0061());
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "SetMakeCommand:" << buildCommand << "\n",
                         args.Quiet);
      this->CTest->SetCTestConfiguration("MakeCommand", buildCommand,
                                         args.Quiet);
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
                                       args.Quiet);
  }

  if (cmValue labelsForSubprojects =
        this->Makefile->GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, args.Quiet);
  }

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

void cmCTestBuildCommand::ProcessAdditionalValues(
  cmCTestGenericHandler* generic)
{
  auto const& args = *this;
  auto const* handler = static_cast<cmCTestBuildHandler*>(generic);
  if (!args.NumberErrors.empty()) {
    this->Makefile->AddDefinition(args.NumberErrors,
                                  std::to_string(handler->GetTotalErrors()));
  }
  if (!args.NumberWarnings.empty()) {
    this->Makefile->AddDefinition(args.NumberWarnings,
                                  std::to_string(handler->GetTotalWarnings()));
  }
}
