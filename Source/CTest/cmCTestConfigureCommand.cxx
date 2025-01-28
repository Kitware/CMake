/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestConfigureCommand.h"

#include <chrono>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmDuration.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLWriter.h"
#include "cmake.h"

class cmCTestConfigureHandler : public cmCTestGenericHandler
{
public:
  cmCTestConfigureHandler(cmCTest* ctest)
    : cmCTestGenericHandler(ctest)
  {
  }

  int ProcessHandler() override;
};

std::unique_ptr<cmCTestGenericHandler>
cmCTestConfigureCommand::InitializeHandler(HandlerArguments& arguments,
                                           cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto const& args = static_cast<ConfigureArguments&>(arguments);

  std::string const buildDirectory = !args.Build.empty()
    ? args.Build
    : mf.GetDefinition("CTEST_BINARY_DIRECTORY");
  if (buildDirectory.empty()) {
    status.SetError("called with no build directory specified.  "
                    "Either use the BUILD argument or set the "
                    "CTEST_BINARY_DIRECTORY variable.");
    return nullptr;
  }

  std::string configureCommand = mf.GetDefinition("CTEST_CONFIGURE_COMMAND");
  if (configureCommand.empty()) {
    cmValue cmakeGenerator = mf.GetDefinition("CTEST_CMAKE_GENERATOR");
    if (!cmNonempty(cmakeGenerator)) {
      status.SetError(
        "called with no configure command specified.  "
        "If this is a  \"built with CMake\" project, set "
        "CTEST_CMAKE_GENERATOR. If not, set CTEST_CONFIGURE_COMMAND.");
      return nullptr;
    }

    std::string const sourceDirectory = !args.Source.empty()
      ? args.Source
      : mf.GetDefinition("CTEST_SOURCE_DIRECTORY");
    if (sourceDirectory.empty() ||
        !cmSystemTools::FileExists(sourceDirectory + "/CMakeLists.txt")) {
      status.SetError("called with invalid source directory.  "
                      "CTEST_SOURCE_DIRECTORY must be set to a directory "
                      "that contains CMakeLists.txt.");
      return nullptr;
    }

    bool const multiConfig = [&]() -> bool {
      cmake* cm = mf.GetCMakeInstance();
      auto gg = cm->CreateGlobalGenerator(cmakeGenerator);
      return gg && gg->IsMultiConfig();
    }();

    bool const buildTypeInOptions =
      args.Options.find("CMAKE_BUILD_TYPE=") != std::string::npos ||
      args.Options.find("CMAKE_BUILD_TYPE:STRING=") != std::string::npos;

    configureCommand = cmStrCat('"', cmSystemTools::GetCMakeCommand(), '"');

    auto const options = cmList(args.Options);
    for (std::string const& option : options) {
      configureCommand += " \"";
      configureCommand += option;
      configureCommand += "\"";
    }

    cmValue cmakeBuildType = mf.GetDefinition("CTEST_CONFIGURATION_TYPE");
    if (!multiConfig && !buildTypeInOptions && cmNonempty(cmakeBuildType)) {
      configureCommand += " \"-DCMAKE_BUILD_TYPE:STRING=";
      configureCommand += cmakeBuildType;
      configureCommand += "\"";
    }

    if (mf.IsOn("CTEST_USE_LAUNCHERS")) {
      configureCommand += " \"-DCTEST_USE_LAUNCHERS:BOOL=TRUE\"";
    }

    configureCommand += " \"-G";
    configureCommand += cmakeGenerator;
    configureCommand += "\"";

    cmValue cmakeGeneratorPlatform =
      mf.GetDefinition("CTEST_CMAKE_GENERATOR_PLATFORM");
    if (cmNonempty(cmakeGeneratorPlatform)) {
      configureCommand += " \"-A";
      configureCommand += *cmakeGeneratorPlatform;
      configureCommand += "\"";
    }

    cmValue cmakeGeneratorToolset =
      mf.GetDefinition("CTEST_CMAKE_GENERATOR_TOOLSET");
    if (cmNonempty(cmakeGeneratorToolset)) {
      configureCommand += " \"-T";
      configureCommand += *cmakeGeneratorToolset;
      configureCommand += "\"";
    }

    configureCommand += " \"-S";
    configureCommand += sourceDirectory;
    configureCommand += "\"";

    configureCommand += " \"-B";
    configureCommand += buildDirectory;
    configureCommand += "\"";
  }

  this->CTest->SetCTestConfiguration("ConfigureCommand", configureCommand,
                                     args.Quiet);

  if (cmValue labelsForSubprojects =
        mf.GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, args.Quiet);
  }

  auto handler = cm::make_unique<cmCTestConfigureHandler>(this->CTest);
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

int cmCTestConfigureHandler::ProcessHandler()
{
  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT,
                     "Configure project" << std::endl, this->Quiet);
  std::string cCommand =
    this->CTest->GetCTestConfiguration("ConfigureCommand");
  if (cCommand.empty()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot find ConfigureCommand key in the DartConfiguration.tcl"
                 << std::endl);
    return -1;
  }

  std::string buildDirectory =
    this->CTest->GetCTestConfiguration("BuildDirectory");
  if (buildDirectory.empty()) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot find BuildDirectory  key in the DartConfiguration.tcl"
                 << std::endl);
    return -1;
  }

  auto elapsed_time_start = std::chrono::steady_clock::now();
  std::string output;
  int retVal = 0;
  bool res = false;
  if (!this->CTest->GetShowOnly()) {
    cmGeneratedFileStream os;
    if (!this->StartResultingXML(cmCTest::PartConfigure, "Configure", os)) {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
                 "Cannot open configure file" << std::endl);
      return 1;
    }
    std::string start_time = this->CTest->CurrentTime();
    auto start_time_time = std::chrono::system_clock::now();

    cmGeneratedFileStream ofs;
    this->StartLogFile("Configure", ofs);
    cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                       "Configure with command: " << cCommand << std::endl,
                       this->Quiet);
    res = this->CTest->RunMakeCommand(cCommand, output, &retVal,
                                      buildDirectory.c_str(),
                                      cmDuration::zero(), ofs);

    if (ofs) {
      ofs.close();
    }

    if (os) {
      cmXMLWriter xml(os);
      this->CTest->StartXML(xml, this->CMake, this->AppendXML);
      this->CTest->GenerateSubprojectsOutput(xml);
      xml.StartElement("Configure");
      xml.Element("StartDateTime", start_time);
      xml.Element("StartConfigureTime", start_time_time);
      xml.Element("ConfigureCommand", cCommand);
      cmCTestOptionalLog(this->CTest, DEBUG, "End" << std::endl, this->Quiet);
      xml.Element("Log", output);
      xml.Element("ConfigureStatus", retVal);
      xml.Element("EndDateTime", this->CTest->CurrentTime());
      xml.Element("EndConfigureTime", std::chrono::system_clock::now());
      xml.Element("ElapsedMinutes",
                  std::chrono::duration_cast<std::chrono::minutes>(
                    std::chrono::steady_clock::now() - elapsed_time_start)
                    .count());
      xml.EndElement(); // Configure
      this->CTest->EndXML(xml);
    }
  } else {
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "Configure with command: " << cCommand << std::endl,
                       this->Quiet);
  }
  if (!res || retVal) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error(s) when configuring the project" << std::endl);
    return -1;
  }
  return 0;
}

bool cmCTestConfigureCommand::InitialPass(std::vector<std::string> const& args,
                                          cmExecutionStatus& status) const
{
  using Args = ConfigureArguments;
  static auto const parser =
    cmArgumentParser<Args>{ MakeHandlerParser<Args>() } //
      .Bind("OPTIONS"_s, &ConfigureArguments::Options);

  return this->Invoke(parser, args, status, [&](ConfigureArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
