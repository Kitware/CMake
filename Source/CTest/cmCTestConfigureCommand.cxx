/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestConfigureCommand.h"

#include <chrono>
#include <cstring>
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
  cmList options;

  if (!args.Options.empty()) {
    options.assign(args.Options);
  }

  if (this->CTest->GetCTestConfiguration("BuildDirectory").empty()) {
    status.SetError(
      "Build directory not specified. Either use BUILD "
      "argument to CTEST_CONFIGURE command or set CTEST_BINARY_DIRECTORY "
      "variable");
    return nullptr;
  }

  cmValue ctestConfigureCommand = mf.GetDefinition("CTEST_CONFIGURE_COMMAND");

  if (cmNonempty(ctestConfigureCommand)) {
    this->CTest->SetCTestConfiguration("ConfigureCommand",
                                       *ctestConfigureCommand, args.Quiet);
  } else {
    cmValue cmakeGeneratorName = mf.GetDefinition("CTEST_CMAKE_GENERATOR");
    if (cmNonempty(cmakeGeneratorName)) {
      std::string const& source_dir =
        this->CTest->GetCTestConfiguration("SourceDirectory");
      if (source_dir.empty()) {
        status.SetError(
          "Source directory not specified. Either use SOURCE "
          "argument to CTEST_CONFIGURE command or set CTEST_SOURCE_DIRECTORY "
          "variable");
        return nullptr;
      }

      std::string const cmlName = mf.GetSafeDefinition("CMAKE_LIST_FILE_NAME");
      std::string const cmakelists_file = cmStrCat(
        source_dir, "/", cmlName.empty() ? "CMakeLists.txt" : cmlName);
      if (!cmSystemTools::FileExists(cmakelists_file)) {
        std::ostringstream e;
        e << "CMakeLists.txt file does not exist [" << cmakelists_file << "]";
        status.SetError(e.str());
        return nullptr;
      }

      bool multiConfig = false;
      bool cmakeBuildTypeInOptions = false;

      auto gg =
        mf.GetCMakeInstance()->CreateGlobalGenerator(*cmakeGeneratorName);
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

      if (mf.IsOn("CTEST_USE_LAUNCHERS")) {
        cmakeConfigureCommand += " \"-DCTEST_USE_LAUNCHERS:BOOL=TRUE\"";
      }

      cmakeConfigureCommand += " \"-G";
      cmakeConfigureCommand += *cmakeGeneratorName;
      cmakeConfigureCommand += "\"";

      cmValue cmakeGeneratorPlatform =
        mf.GetDefinition("CTEST_CMAKE_GENERATOR_PLATFORM");
      if (cmNonempty(cmakeGeneratorPlatform)) {
        cmakeConfigureCommand += " \"-A";
        cmakeConfigureCommand += *cmakeGeneratorPlatform;
        cmakeConfigureCommand += "\"";
      }

      cmValue cmakeGeneratorToolset =
        mf.GetDefinition("CTEST_CMAKE_GENERATOR_TOOLSET");
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
      status.SetError(
        "Configure command is not specified. If this is a "
        "\"built with CMake\" project, set CTEST_CMAKE_GENERATOR. If not, "
        "set CTEST_CONFIGURE_COMMAND.");
      return nullptr;
    }
  }

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
