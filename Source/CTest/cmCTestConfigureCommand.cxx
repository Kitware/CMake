/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestConfigureCommand.h"

#include <cstring>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmCTestConfigureHandler.h"
#include "cmCTestGenericHandler.h"
#include "cmCommand.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

std::unique_ptr<cmCommand> cmCTestConfigureCommand::Clone()
{
  auto ni = cm::make_unique<cmCTestConfigureCommand>();
  ni->CTest = this->CTest;
  return std::unique_ptr<cmCommand>(std::move(ni));
}

std::unique_ptr<cmCTestGenericHandler>
cmCTestConfigureCommand::InitializeHandler(HandlerArguments& arguments)
{
  auto const& args = static_cast<ConfigureArguments&>(arguments);
  cmList options;

  if (!args.Options.empty()) {
    options.assign(args.Options);
  }

  if (this->CTest->GetCTestConfiguration("BuildDirectory").empty()) {
    this->SetError(
      "Build directory not specified. Either use BUILD "
      "argument to CTEST_CONFIGURE command or set CTEST_BINARY_DIRECTORY "
      "variable");
    return nullptr;
  }

  cmValue ctestConfigureCommand =
    this->Makefile->GetDefinition("CTEST_CONFIGURE_COMMAND");

  if (cmNonempty(ctestConfigureCommand)) {
    this->CTest->SetCTestConfiguration("ConfigureCommand",
                                       *ctestConfigureCommand, args.Quiet);
  } else {
    cmValue cmakeGeneratorName =
      this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR");
    if (cmNonempty(cmakeGeneratorName)) {
      const std::string& source_dir =
        this->CTest->GetCTestConfiguration("SourceDirectory");
      if (source_dir.empty()) {
        this->SetError(
          "Source directory not specified. Either use SOURCE "
          "argument to CTEST_CONFIGURE command or set CTEST_SOURCE_DIRECTORY "
          "variable");
        return nullptr;
      }

      const std::string cmakelists_file = source_dir + "/CMakeLists.txt";
      if (!cmSystemTools::FileExists(cmakelists_file)) {
        std::ostringstream e;
        e << "CMakeLists.txt file does not exist [" << cmakelists_file << "]";
        this->SetError(e.str());
        return nullptr;
      }

      bool multiConfig = false;
      bool cmakeBuildTypeInOptions = false;

      auto gg = this->Makefile->GetCMakeInstance()->CreateGlobalGenerator(
        *cmakeGeneratorName);
      if (gg) {
        multiConfig = gg->IsMultiConfig();
        gg.reset();
      }

      std::string cmakeConfigureCommand =
        cmStrCat('"', cmSystemTools::GetCMakeCommand(), '"');

      for (std::string const& option : options) {
        cmakeConfigureCommand += " \"";
        cmakeConfigureCommand += option;
        cmakeConfigureCommand += "\"";

        if ((nullptr != strstr(option.c_str(), "CMAKE_BUILD_TYPE=")) ||
            (nullptr != strstr(option.c_str(), "CMAKE_BUILD_TYPE:STRING="))) {
          cmakeBuildTypeInOptions = true;
        }
      }

      if (!multiConfig && !cmakeBuildTypeInOptions &&
          !this->CTest->GetConfigType().empty()) {
        cmakeConfigureCommand += " \"-DCMAKE_BUILD_TYPE:STRING=";
        cmakeConfigureCommand += this->CTest->GetConfigType();
        cmakeConfigureCommand += "\"";
      }

      if (this->Makefile->IsOn("CTEST_USE_LAUNCHERS")) {
        cmakeConfigureCommand += " \"-DCTEST_USE_LAUNCHERS:BOOL=TRUE\"";
      }

      cmakeConfigureCommand += " \"-G";
      cmakeConfigureCommand += *cmakeGeneratorName;
      cmakeConfigureCommand += "\"";

      cmValue cmakeGeneratorPlatform =
        this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR_PLATFORM");
      if (cmNonempty(cmakeGeneratorPlatform)) {
        cmakeConfigureCommand += " \"-A";
        cmakeConfigureCommand += *cmakeGeneratorPlatform;
        cmakeConfigureCommand += "\"";
      }

      cmValue cmakeGeneratorToolset =
        this->Makefile->GetDefinition("CTEST_CMAKE_GENERATOR_TOOLSET");
      if (cmNonempty(cmakeGeneratorToolset)) {
        cmakeConfigureCommand += " \"-T";
        cmakeConfigureCommand += *cmakeGeneratorToolset;
        cmakeConfigureCommand += "\"";
      }

      cmakeConfigureCommand += " \"-S";
      cmakeConfigureCommand += source_dir;
      cmakeConfigureCommand += "\"";

      cmakeConfigureCommand += " \"-B";
      cmakeConfigureCommand +=
        this->CTest->GetCTestConfiguration("BuildDirectory");
      cmakeConfigureCommand += "\"";

      this->CTest->SetCTestConfiguration("ConfigureCommand",
                                         cmakeConfigureCommand, args.Quiet);
    } else {
      this->SetError(
        "Configure command is not specified. If this is a "
        "\"built with CMake\" project, set CTEST_CMAKE_GENERATOR. If not, "
        "set CTEST_CONFIGURE_COMMAND.");
      return nullptr;
    }
  }

  if (cmValue labelsForSubprojects =
        this->Makefile->GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, args.Quiet);
  }

  auto handler = cm::make_unique<cmCTestConfigureHandler>(this->CTest);
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

bool cmCTestConfigureCommand::InitialPass(std::vector<std::string> const& args,
                                          cmExecutionStatus& status)
{
  using Args = ConfigureArguments;
  static auto const parser =
    cmArgumentParser<Args>{ MakeHandlerParser<Args>() } //
      .Bind("OPTIONS"_s, &ConfigureArguments::Options);

  return this->Invoke(parser, args, status, [&](ConfigureArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
