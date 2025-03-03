/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestBuildCommand.h"

#include <sstream>
#include <utility>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmCTestBuildHandler.h"
#include "cmCTestGenericHandler.h"
#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

bool cmCTestBuildCommand::InitialPass(std::vector<std::string> const& args,
                                      cmExecutionStatus& status) const
{
  static auto const parser =
    cmArgumentParser<BuildArguments>{ MakeHandlerParser<BuildArguments>() }
      .Bind("NUMBER_ERRORS"_s, &BuildArguments::NumberErrors)
      .Bind("NUMBER_WARNINGS"_s, &BuildArguments::NumberWarnings)
      .Bind("TARGET"_s, &BuildArguments::Target)
      .Bind("CONFIGURATION"_s, &BuildArguments::Configuration)
      .Bind("FLAGS"_s, &BuildArguments::Flags)
      .Bind("PROJECT_NAME"_s, &BuildArguments::ProjectName)
      .Bind("PARALLEL_LEVEL"_s, &BuildArguments::ParallelLevel);

  return this->Invoke(parser, args, status, [&](BuildArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}

std::unique_ptr<cmCTestGenericHandler> cmCTestBuildCommand::InitializeHandler(
  HandlerArguments& arguments, cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto const& args = static_cast<BuildArguments&>(arguments);
  auto handler = cm::make_unique<cmCTestBuildHandler>(this->CTest);

  cmValue ctestBuildCommand = mf.GetDefinition("CTEST_BUILD_COMMAND");
  if (cmNonempty(ctestBuildCommand)) {
    this->CTest->SetCTestConfiguration("MakeCommand", *ctestBuildCommand,
                                       args.Quiet);
  } else {
    cmValue cmakeGeneratorName = mf.GetDefinition("CTEST_CMAKE_GENERATOR");

    // Build configuration is determined by: CONFIGURATION argument,
    // or CTEST_BUILD_CONFIGURATION script variable, or
    // CTEST_CONFIGURATION_TYPE script variable, or ctest -C command
    // line argument... in that order.
    //
    cmValue ctestBuildConfiguration =
      mf.GetDefinition("CTEST_BUILD_CONFIGURATION");
    std::string cmakeBuildConfiguration = cmNonempty(args.Configuration)
      ? args.Configuration
      : cmNonempty(ctestBuildConfiguration) ? *ctestBuildConfiguration
                                            : this->CTest->GetConfigType();

    std::string const& cmakeBuildAdditionalFlags = cmNonempty(args.Flags)
      ? args.Flags
      : mf.GetSafeDefinition("CTEST_BUILD_FLAGS");
    std::string const& cmakeBuildTarget = cmNonempty(args.Target)
      ? args.Target
      : mf.GetSafeDefinition("CTEST_BUILD_TARGET");

    if (cmNonempty(cmakeGeneratorName)) {
      if (cmakeBuildConfiguration.empty()) {
        cmakeBuildConfiguration = "Release";
      }

      auto globalGenerator =
        mf.GetCMakeInstance()->CreateGlobalGenerator(*cmakeGeneratorName);
      if (!globalGenerator) {
        std::string e = cmStrCat("could not create generator named \"",
                                 *cmakeGeneratorName, '"');
        mf.IssueMessage(MessageType::FATAL_ERROR, e);
        cmSystemTools::SetFatalErrorOccurred();
        return nullptr;
      }
      if (cmakeBuildConfiguration.empty()) {
        cmakeBuildConfiguration = "Debug";
      }

      std::string dir = this->CTest->GetCTestConfiguration("BuildDirectory");
      std::string buildCommand = globalGenerator->GenerateCMakeBuildCommand(
        cmakeBuildTarget, cmakeBuildConfiguration, args.ParallelLevel,
        cmakeBuildAdditionalFlags, false);
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
      status.SetError(ostr.str());
      return nullptr;
    }
  }

  if (cmValue useLaunchers = mf.GetDefinition("CTEST_USE_LAUNCHERS")) {
    this->CTest->SetCTestConfiguration("UseLaunchers", *useLaunchers,
                                       args.Quiet);
  }

  if (cmValue labelsForSubprojects =
        mf.GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, args.Quiet);
  }

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

void cmCTestBuildCommand::ProcessAdditionalValues(
  cmCTestGenericHandler* generic, HandlerArguments const& arguments,
  cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto const& args = static_cast<BuildArguments const&>(arguments);
  auto const* handler = static_cast<cmCTestBuildHandler*>(generic);
  if (!args.NumberErrors.empty()) {
    mf.AddDefinition(args.NumberErrors,
                     std::to_string(handler->GetTotalErrors()));
  }
  if (!args.NumberWarnings.empty()) {
    mf.AddDefinition(args.NumberWarnings,
                     std::to_string(handler->GetTotalWarnings()));
  }
}
