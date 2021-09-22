/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmBuildCommand.h"

#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {

bool MainSignature(std::vector<std::string> const& args,
                   cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("requires at least one argument naming a CMake variable");
    return false;
  }

  // The cmake variable in which to store the result.
  std::string const& variable = args[0];

  // Parse remaining arguments.
  std::string configuration;
  std::string project_name;
  std::string target;
  std::string parallel;
  enum Doing
  {
    DoingNone,
    DoingConfiguration,
    DoingProjectName,
    DoingTarget,
    DoingParallel
  };
  Doing doing = DoingNone;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "CONFIGURATION") {
      doing = DoingConfiguration;
    } else if (args[i] == "PROJECT_NAME") {
      doing = DoingProjectName;
    } else if (args[i] == "TARGET") {
      doing = DoingTarget;
    } else if (args[i] == "PARALLEL_LEVEL") {
      doing = DoingParallel;
    } else if (doing == DoingConfiguration) {
      doing = DoingNone;
      configuration = args[i];
    } else if (doing == DoingProjectName) {
      doing = DoingNone;
      project_name = args[i];
    } else if (doing == DoingTarget) {
      doing = DoingNone;
      target = args[i];
    } else if (doing == DoingParallel) {
      doing = DoingNone;
      parallel = args[i];
    } else {
      status.SetError(cmStrCat("unknown argument \"", args[i], "\""));
      return false;
    }
  }

  // If null/empty CONFIGURATION argument, cmake --build uses 'Debug'
  // in the currently implemented multi-configuration global generators...
  // so we put this code here to end up with the same default configuration
  // as the original 2-arg build_command signature:
  //
  if (configuration.empty()) {
    cmSystemTools::GetEnv("CMAKE_CONFIG_TYPE", configuration);
  }
  if (configuration.empty()) {
    configuration = "Release";
  }

  cmMakefile& mf = status.GetMakefile();
  if (!project_name.empty()) {
    mf.IssueMessage(MessageType::AUTHOR_WARNING,
                    "Ignoring PROJECT_NAME option because it has no effect.");
  }

  std::string makecommand = mf.GetGlobalGenerator()->GenerateCMakeBuildCommand(
    target, configuration, parallel, "", mf.IgnoreErrorsCMP0061());

  mf.AddDefinition(variable, makecommand);

  return true;
}

bool TwoArgsSignature(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with less than two arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  std::string const& define = args[0];
  cmValue cacheValue = mf.GetDefinition(define);

  std::string configType;
  if (!cmSystemTools::GetEnv("CMAKE_CONFIG_TYPE", configType) ||
      configType.empty()) {
    configType = "Release";
  }

  std::string makecommand = mf.GetGlobalGenerator()->GenerateCMakeBuildCommand(
    "", configType, "", "", mf.IgnoreErrorsCMP0061());

  if (cacheValue) {
    return true;
  }
  mf.AddCacheDefinition(define, makecommand,
                        "Command used to build entire project "
                        "from the command line.",
                        cmStateEnums::STRING);
  return true;
}

} // namespace

bool cmBuildCommand(std::vector<std::string> const& args,
                    cmExecutionStatus& status)
{
  // Support the legacy signature of the command:
  if (args.size() == 2) {
    return TwoArgsSignature(args, status);
  }

  return MainSignature(args, status);
}
