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
#include "cmProperty.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
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
}

cmCTestBuildCommand::~cmCTestBuildCommand() = default;

cmCTestGenericHandler* cmCTestBuildCommand::InitializeHandler()
{
  cmCTestBuildHandler* handler = this->CTest->GetBuildHandler();
  handler->Initialize();

  this->Handler = handler;

  cmProp ctestBuildCommand =
    this->Makefile->GetDefinition("CTEST_BUILD_COMMAND");
  if (cmNonempty(ctestBuildCommand)) {
    this->CTest->SetCTestConfiguration("MakeCommand", *ctestBuildCommand,
                                       this->Quiet);
  } else {
    cmProp cmakeGeneratorName =
      this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");

    // Build configuration is determined by: CONFIGURATION argument,
    // or CTEST_BUILD_CONFIGURATION script variable, or
    // CTEST_CONFIGURATION_TYPE script variable, or ctest -C command
    // line argument... in that order.
    //
    cmProp ctestBuildConfiguration =
      this->Makefile->GetDefinition("CTEST_BUILD_CONFIGURATION");
    const std::string* cmakeBuildConfiguration = !this->Configuration.empty()
      ? &this->Configuration
      : (cmNonempty(ctestBuildConfiguration) ? ctestBuildConfiguration
                                             : &this->CTest->GetConfigType());

    const std::string* cmakeBuildAdditionalFlags = !this->Flags.empty()
      ? &this->Flags
      : this->Makefile->GetDefinition("CTEST_BUILD_FLAGS");
    const std::string* cmakeBuildTarget = !this->Target.empty()
      ? &this->Target
      : this->Makefile->GetDefinition("CTEST_BUILD_TARGET");

    if (cmNonempty(cmakeGeneratorName)) {
      if (!cmakeBuildConfiguration) {
        static const std::string sRelease = "Release";
        cmakeBuildConfiguration = &sRelease;
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
          cmSystemTools::SetFatalErrorOccured();
          return nullptr;
        }
      }
      if (cmakeBuildConfiguration->empty()) {
        const std::string* config = nullptr;
#ifdef CMAKE_INTDIR
        static const std::string sIntDir = CMAKE_INTDIR;
        config = &sIntDir;
#endif
        if (!config) {
          static const std::string sDebug = "Debug";
          config = &sDebug;
        }
        cmakeBuildConfiguration = config;
      }

      std::string dir = this->CTest->GetCTestConfiguration("BuildDirectory");
      std::string buildCommand =
        this->GlobalGenerator->GenerateCMakeBuildCommand(
          cmakeBuildTarget ? *cmakeBuildTarget : "", *cmakeBuildConfiguration,
          cmakeBuildAdditionalFlags ? *cmakeBuildAdditionalFlags : "",
          this->Makefile->IgnoreErrorsCMP0061());
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

  if (cmProp useLaunchers =
        this->Makefile->GetDefinition("CTEST_USE_LAUNCHERS")) {
    this->CTest->SetCTestConfiguration("UseLaunchers", *useLaunchers,
                                       this->Quiet);
  }

  if (cmProp labelsForSubprojects =
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
  bool ret = cmCTestHandlerCommand::InitialPass(args, status);
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
