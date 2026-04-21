/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestConfigureCommand.h"

#include <chrono>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCMakePresetsGraph.h"
#include "cmCTest.h"
#include "cmDuration.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmInstrumentation.h"
#include "cmInstrumentationQuery.h"
#include "cmJSONState.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLWriter.h"
#include "cmake.h"

using ConfigurePreset = cmCMakePresetsGraph::ConfigurePreset;

namespace {

bool ConstructConfigureCommand(cmExecutionStatus& status, cmMakefile& mf,
                               std::string const sourceDirectory,
                               std::string const buildDirectory,
                               std::string const options,
                               std::string const presetName,
                               std::string& configureCommand)
{
  configureCommand = cmStrCat('"', cmSystemTools::GetCMakeCommand(), '"');
  configureCommand += " \"-S";
  configureCommand += cmSystemTools::CollapseFullPath(sourceDirectory);
  configureCommand += "\"";

  if (!buildDirectory.empty()) {
    configureCommand += " \"-B";
    configureCommand += cmSystemTools::CollapseFullPath(buildDirectory);
    configureCommand += "\"";
  }

  cmValue cmakeGenerator = mf.GetDefinition("CTEST_CMAKE_GENERATOR");
  if (cmNonempty(cmakeGenerator)) {
    configureCommand += " \"-G";
    configureCommand += cmakeGenerator;
    configureCommand += "\"";
  }

  bool presetProvidesBuildDir = false;
  bool presetProvidesGenerator = false;

  if (!presetName.empty()) {
    cmCMakePresetsGraph presetsGraph;
    if (!presetsGraph.ReadProjectPresets(sourceDirectory)) {
      status.SetError(
        cmStrCat("Could not read presets from \"", sourceDirectory,
                 "\": ", presetsGraph.parseState.GetErrorMessage()));
      return false;
    }

    auto resolveResult =
      presetsGraph.ResolvePreset(presetName, presetsGraph.ConfigurePresets);
    auto resolveError =
      cmCMakePresetsGraph::FormatPresetError<ConfigurePreset>(
        resolveResult.StatusCode, resolveResult.ErrorPresetName,
        sourceDirectory);
    if (resolveError) {
      status.SetError(*resolveError);
      return false;
    }

    auto const* expandedPreset = resolveResult.Preset;

    configureCommand += " \"--preset\"";
    configureCommand += " \"";
    configureCommand += presetName;
    configureCommand += "\"";

    if (!expandedPreset->BinaryDir.empty()) {
      presetProvidesBuildDir = true;
    }

    if (!expandedPreset->Generator.empty()) {
      presetProvidesGenerator = true;
    }
  }

  if (buildDirectory.empty() && !presetProvidesBuildDir) {
    status.SetError("called with no build directory specified.  "
                    "Either use the BUILD argument or set the "
                    "CTEST_BINARY_DIRECTORY variable.");
    return false;
  }

  if (!cmNonempty(cmakeGenerator) && !presetProvidesGenerator) {
    status.SetError(
      "called with no configure command specified.  "
      "If this is a  \"built with CMake\" project, set "
      "CTEST_CMAKE_GENERATOR. If not, set CTEST_CONFIGURE_COMMAND.");
    return false;
  }

  if (mf.IsOn("CTEST_USE_LAUNCHERS")) {
    configureCommand += " \"-DCTEST_USE_LAUNCHERS:BOOL=TRUE\"";
  }

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

  // Append OPTIONS to the configure command.
  bool const multiConfig = [&]() -> bool {
    cmake* cm = mf.GetCMakeInstance();
    auto gg = cm->CreateGlobalGenerator(cmakeGenerator);
    return gg && gg->IsMultiConfig();
  }();

  bool const buildTypeInOptions =
    options.find("CMAKE_BUILD_TYPE=") != std::string::npos ||
    options.find("CMAKE_BUILD_TYPE:STRING=") != std::string::npos;

  auto const optionsList = cmList(options);
  for (std::string const& option : optionsList) {
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

  return true;
}

} // namespace

bool cmCTestConfigureCommand::ExecuteConfigure(ConfigureArguments const& args,
                                               cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  std::string buildDirectory = !args.Build.empty()
    ? args.Build
    : mf.GetDefinition("CTEST_BINARY_DIRECTORY");

  std::string const sourceDirectory = !args.Source.empty()
    ? args.Source
    : mf.GetDefinition("CTEST_SOURCE_DIRECTORY");
  if (sourceDirectory.empty() ||
      !cmSystemTools::FileExists(sourceDirectory + "/CMakeLists.txt")) {
    status.SetError("called with invalid source directory.  "
                    "CTEST_SOURCE_DIRECTORY must be set to a directory "
                    "that contains CMakeLists.txt.");
    return false;
  }

  std::string configureCommand = mf.GetDefinition("CTEST_CONFIGURE_COMMAND");
  if (configureCommand.empty() &&
      !ConstructConfigureCommand(status, mf, sourceDirectory, buildDirectory,
                                 args.Options, args.Preset,
                                 configureCommand)) {
    return false;
  }

  cmCTestOptionalLog(this->CTest, HANDLER_OUTPUT, "Configure project\n",
                     args.Quiet);

  if (this->CTest->GetShowOnly()) {
    cmCTestOptionalLog(this->CTest, DEBUG,
                       "Configure with command: " << configureCommand << '\n',
                       args.Quiet);
    if (!args.ReturnValue.empty()) {
      mf.AddDefinition(args.ReturnValue, "0");
    }
    return true;
  }

  if (!cmSystemTools::MakeDirectory(buildDirectory)) {
    status.SetError(cmStrCat("cannot create directory ", buildDirectory));
    return false;
  }

  cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                     "Configure with command: " << configureCommand << '\n',
                     args.Quiet);

  int const submitIndex =
    args.SubmitIndex.empty() ? 0 : std::atoi(args.SubmitIndex.c_str());

  cmGeneratedFileStream logFile;
  this->CTest->StartLogFile("Configure", submitIndex, logFile);

  auto const startTime = std::chrono::system_clock::now();
  auto const startDateTime = this->CTest->CurrentTime();

  std::string output;
  int retVal = 0;
  bool const res = this->CTest->RunMakeCommand(configureCommand, output,
                                               &retVal, buildDirectory.c_str(),
                                               cmDuration::zero(), logFile);

  auto const endTime = std::chrono::system_clock::now();
  auto const endDateTime = this->CTest->CurrentTime();
  auto const elapsedMinutes =
    std::chrono::duration_cast<std::chrono::minutes>(endTime - startTime);

  cmCTestOptionalLog(this->CTest, DEBUG, "End\n", args.Quiet);

  if (!res || retVal) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Error(s) when configuring the project\n");
  }

  if (!args.ReturnValue.empty()) {
    mf.AddDefinition(args.ReturnValue, std::to_string(retVal));
  }

  if (cmValue value = mf.GetDefinition("CTEST_CHANGE_ID")) {
    this->CTest->SetCTestConfiguration("ChangeId", *value, args.Quiet);
  }

  if (cmValue value = mf.GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects", *value,
                                       args.Quiet);
  }

  cmGeneratedFileStream xmlFile;
  if (!this->CTest->StartResultingXML(cmCTest::PartConfigure, "Configure",
                                      submitIndex, xmlFile)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot open configure file" << std::endl);
    return false;
  }

  cmXMLWriter xml(xmlFile);
  this->CTest->StartXML(xml, mf.GetCMakeInstance(), args.Append);
  this->CTest->GenerateSubprojectsOutput(xml);
  xml.StartElement("Configure");
  xml.Element("StartDateTime", startDateTime);
  xml.Element("StartConfigureTime", startTime);
  xml.Element("ConfigureCommand", configureCommand);
  xml.Element("Log", output);
  xml.Element("ConfigureStatus", retVal);
  xml.Element("EndDateTime", endDateTime);
  xml.Element("EndConfigureTime", endTime);
  xml.Element("ElapsedMinutes", elapsedMinutes.count());

  this->CTest->GetInstrumentation().CollectTimingData(
    cmInstrumentationQuery::Hook::PrepareForCDash);
  this->CTest->ConvertInstrumentationSnippetsToXML(xml, "configure");

  xml.EndElement(); // Configure
  this->CTest->EndXML(xml);

  return res;
}

bool cmCTestConfigureCommand::InitialPass(std::vector<std::string> const& args,
                                          cmExecutionStatus& status) const
{
  using Args = ConfigureArguments;
  static auto const parser =
    cmArgumentParser<Args>{ MakeHandlerParser<Args>() } //
      .Bind("OPTIONS"_s, &ConfigureArguments::Options)
      .Bind("PRESET"_s, &ConfigureArguments::Preset);

  return this->Invoke(parser, args, status, [&](ConfigureArguments& a) {
    return this->ExecuteConfigure(a, status);
  });
}
