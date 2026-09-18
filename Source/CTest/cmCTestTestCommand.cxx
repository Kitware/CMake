/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestTestCommand.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <map>
#include <ratio>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>

#include "cmArgumentParserTypes.h"
#include "cmCMakePresetsGraph.h"
#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestTestHandler.h"
#include "cmDuration.h"
#include "cmExecutionStatus.h"
#include "cmJSONState.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

using TestPreset = cmCMakePresetsGraph::TestPreset;

cm::optional<cmCTestTestCommand::ResolvedTestPreset>
cmCTestTestCommand::ResolveTestPreset(cmMakefile& mf,
                                      std::string const& presetArg,
                                      std::string const& presetsFileArg,
                                      cmExecutionStatus& status) const
{
  ResolvedTestPreset resolved;
  resolved.SourceDirectory = mf.GetSafeDefinition("CTEST_SOURCE_DIRECTORY");

  // Presets file is set according to the following priority order:
  // 1) The PRESETS_FILE option to ctest_test()
  // 2) CTEST_PRESETS_FILE script variable
  std::string const rawPresetsFile = !presetsFileArg.empty()
    ? presetsFileArg
    : mf.GetSafeDefinition("CTEST_PRESETS_FILE");

  resolved.PresetsFile = rawPresetsFile.empty()
    ? ""
    : cmSystemTools::CollapseFullPath(rawPresetsFile,
                                      resolved.SourceDirectory);

  // Preset name is set according to the following priority order:
  // 1) The PRESET option to ctest_test()
  // 2) CTEST_TEST_PRESET script variable
  // 3) CTEST_PRESET script variable (a warning is emitted if no test preset
  //    exists with this name)
  resolved.EffectivePreset = !presetArg.empty() ? presetArg
    : cmNonempty(mf.GetDefinition("CTEST_TEST_PRESET"))
    ? *mf.GetDefinition("CTEST_TEST_PRESET")
    : "";
  if (resolved.EffectivePreset.empty()) {
    cmValue v = mf.GetDefinition("CTEST_PRESET");
    if (cmNonempty(v)) {
      std::string presetError;
      auto presetCheck = TestPresetExists(*v, resolved.SourceDirectory,
                                          resolved.PresetsFile, presetError);
      if (presetCheck == PresetCheckResult::ReadError) {
        status.SetError(cmStrCat('\n', presetError));
        return cm::nullopt;
      }
      if (presetCheck == PresetCheckResult::Found) {
        resolved.EffectivePreset = *v;
      } else {
        cmCTestLog(this->CTest, WARNING,
                   "No test preset named \""
                     << *v << "\" found, ignoring CTEST_PRESET." << std::endl);
      }
    }
  }

  if (resolved.EffectivePreset.empty()) {
    return cm::optional<ResolvedTestPreset>(std::move(resolved));
  }

  resolved.PresetsGraph = cm::make_unique<cmCMakePresetsGraph>();
  if (!resolved.PresetsGraph->ReadProjectPresets(resolved.SourceDirectory,
                                                 resolved.PresetsFile)) {
    status.SetError(
      cmStrCat("\n Could not read presets from \"", resolved.SourceDirectory,
               "\":\n ", resolved.PresetsGraph->parseState.GetErrorMessage()));
    return cm::nullopt;
  }

  auto resolveResult = resolved.PresetsGraph->ResolvePreset(
    resolved.EffectivePreset, resolved.PresetsGraph->TestPresets);
  auto resolveError = cmCMakePresetsGraph::FormatPresetError<TestPreset>(
    resolveResult.StatusCode, resolveResult.ErrorPresetName,
    resolved.SourceDirectory);
  if (resolveError) {
    status.SetError(*resolveError);
    return cm::nullopt;
  }
  resolved.ExpandedPreset = resolveResult.Preset;

  return cm::optional<ResolvedTestPreset>(std::move(resolved));
}

bool cmCTestTestCommand::ExecuteHandlerCommand(TestArguments& args,
                                               cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  // Resolve the preset once here (rather than letting InitializeHandler()
  // resolve it again) so its Environment can be applied for the duration of
  // the whole test run, without parsing the presets file (and emitting errors,
  // etc.) more than once.
  this->CachedPresetResolution.emplace(
    ResolveTestPreset(mf, args.Preset, args.PresetsFile, status));

  cmSystemTools::SaveRestoreEnvironment restoreEnv;
  if (*this->CachedPresetResolution &&
      (*this->CachedPresetResolution)->ExpandedPreset) {
    for (auto const& var :
         (*this->CachedPresetResolution)->ExpandedPreset->Environment) {
      if (var.second) {
        cmSystemTools::PutEnv(cmStrCat(var.first, '=', *var.second));
      }
    }
  }

  // Special case for CTest environment variables specified in a preset, which
  // are handled by cmCTest much earlier than here, and would otherwise be
  // ignored.
  if (!this->CTest->UpdateStateFromEnvironment()) {
    return false;
  }

  return cmCTestHandlerCommand::ExecuteHandlerCommand(args, status);
}

std::unique_ptr<cmCTestGenericHandler> cmCTestTestCommand::InitializeHandler(
  HandlerArguments& arguments, cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto& args = static_cast<TestArguments&>(arguments);

  assert(this->CachedPresetResolution);
  cm::optional<ResolvedTestPreset> resolvedPreset =
    std::move(*this->CachedPresetResolution);
  this->CachedPresetResolution.reset();
  if (!resolvedPreset) {
    return nullptr;
  }
  TestPreset const* expandedPreset = resolvedPreset->ExpandedPreset;

  cmValue ctestTimeout = mf.GetDefinition("CTEST_TEST_TIMEOUT");

  cmDuration timeout;
  if (ctestTimeout) {
    timeout = cmDuration(atof(ctestTimeout->c_str()));
  } else {
    timeout = this->CTest->GetTimeOut();
    if (timeout <= cmDuration::zero()) {
      // By default use timeout of 10 minutes
      timeout = std::chrono::minutes(10);
    }
  }
  this->CTest->SetTimeOut(timeout);

  auto handler = this->InitializeActualHandler(args, status);

  // Load settings from the preset if one was specified.
  if (expandedPreset) {
    cmCTestApplyTestPresetToOptions(handler->TestOptions, *expandedPreset);

    if (!args.ParallelLevel && expandedPreset->Execution) {
      if (auto const& jobs = expandedPreset->Execution->Jobs) {
        ArgumentParser::Maybe<std::string> level;
        if (jobs->has_value()) {
          level = std::to_string(**jobs);
        }
        handler->ParallelLevel = level;
      }
    }

    if (args.Repeat.empty() && expandedPreset->Execution &&
        expandedPreset->Execution->Repeat) {
      auto const& rep = *expandedPreset->Execution->Repeat;
      using RepeatMode = TestPreset::ExecutionOptions::RepeatOptions::ModeEnum;
      std::string modeStr;
      switch (rep.Mode) {
        case RepeatMode::UntilFail:
          modeStr = "UNTIL_FAIL";
          break;
        case RepeatMode::UntilPass:
          modeStr = "UNTIL_PASS";
          break;
        case RepeatMode::AfterTimeout:
          modeStr = "AFTER_TIMEOUT";
          break;
      }
      handler->Repeat = cmStrCat(modeStr, ':', rep.Count);
    }

    if (args.TestLoad.empty() && expandedPreset->Execution &&
        expandedPreset->Execution->TestLoad) {
      args.TestLoad = std::to_string(*expandedPreset->Execution->TestLoad);
    }
  }

  if (args.ResourceSpecFile.empty()) {
    cmValue resourceSpecFile = mf.GetDefinition("CTEST_RESOURCE_SPEC_FILE");
    if (resourceSpecFile) {
      args.ResourceSpecFile = *resourceSpecFile;
    }
  }

  // Apply explicitly specified ctest_test() options,
  // overriding any conflicting preset values.
  if (!args.Start.empty() || !args.End.empty() || !args.Stride.empty()) {
    handler->TestOptions.TestsToRunInformation =
      cmStrCat(args.Start, ',', args.End, ',', args.Stride);
  }
  if (!args.Exclude.empty()) {
    handler->TestOptions.ExcludeRegularExpression = args.Exclude;
  }
  if (!args.Include.empty()) {
    handler->TestOptions.IncludeRegularExpression = args.Include;
  }
  if (!args.ExcludeLabel.empty()) {
    handler->TestOptions.ExcludeLabelRegularExpression = args.ExcludeLabel;
  }
  if (!args.IncludeLabel.empty()) {
    handler->TestOptions.LabelRegularExpression = args.IncludeLabel;
  }

  if (!args.ExcludeTestsFromFile.empty()) {
    handler->TestOptions.ExcludeTestListFile = args.ExcludeTestsFromFile;
  }
  if (!args.IncludeTestsFromFile.empty()) {
    handler->TestOptions.TestListFile = args.IncludeTestsFromFile;
  }

  if (!args.ExcludeFixture.empty()) {
    handler->TestOptions.ExcludeFixtureRegularExpression = args.ExcludeFixture;
  }
  if (!args.ExcludeFixtureSetup.empty()) {
    handler->TestOptions.ExcludeFixtureSetupRegularExpression =
      args.ExcludeFixtureSetup;
  }
  if (!args.ExcludeFixtureCleanup.empty()) {
    handler->TestOptions.ExcludeFixtureCleanupRegularExpression =
      args.ExcludeFixtureCleanup;
  }
  if (args.StopOnFailure) {
    handler->TestOptions.StopOnFailure = true;
  }
  if (args.ParallelLevel) {
    handler->ParallelLevel = *args.ParallelLevel;
  }
  if (!args.Repeat.empty()) {
    handler->Repeat = args.Repeat;
  }
  if (!args.ScheduleRandom.empty()) {
    handler->TestOptions.ScheduleRandom = cmValue(args.ScheduleRandom).IsOn();
  }
  if (!args.ResourceSpecFile.empty()) {
    handler->TestOptions.ResourceSpecFile = args.ResourceSpecFile;
  }
  if (!args.CoverageTool.empty()) {
    handler->TestOptions.CoverageTool = args.CoverageTool;
  }
  if (!args.StopTime.empty()) {
    this->CTest->SetStopTime(args.StopTime);
  }

  // Test load is determined by: TEST_LOAD argument,
  // or CTEST_TEST_LOAD script variable, or ctest --test-load
  // command line argument... in that order.
  unsigned long testLoad;
  cmValue ctestTestLoad = mf.GetDefinition("CTEST_TEST_LOAD");
  if (!args.TestLoad.empty()) {
    if (!cmStrToULong(args.TestLoad, &testLoad)) {
      testLoad = 0;
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'TEST_LOAD' : " << args.TestLoad
                                                    << std::endl);
    }
  } else if (cmNonempty(ctestTestLoad)) {
    if (!cmStrToULong(*ctestTestLoad, &testLoad)) {
      testLoad = 0;
      cmCTestLog(this->CTest, WARNING,
                 "Invalid value for 'CTEST_TEST_LOAD' : " << *ctestTestLoad
                                                          << std::endl);
    }
  } else {
    testLoad = this->CTest->GetTestLoad();
  }
  handler->SetTestLoad(testLoad);

  if (cmValue labelsForSubprojects =
        mf.GetDefinition("CTEST_LABELS_FOR_SUBPROJECTS")) {
    this->CTest->SetCTestConfiguration("LabelsForSubprojects",
                                       *labelsForSubprojects, args.Quiet);
  }

  if (!args.OutputJUnit.empty()) {
    handler->SetJUnitXMLFileName(args.OutputJUnit);
  }

  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

std::unique_ptr<cmCTestTestHandler>
cmCTestTestCommand::InitializeActualHandler(HandlerArguments&,
                                            cmExecutionStatus&) const
{
  return cm::make_unique<cmCTestTestHandler>(this->CTest);
}

bool cmCTestTestCommand::InitialPass(std::vector<std::string> const& args,
                                     cmExecutionStatus& status) const
{
  static auto const parser = MakeTestParser<TestArguments>();

  return this->Invoke(parser, args, status, [&](TestArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
