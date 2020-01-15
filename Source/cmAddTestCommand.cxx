/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddTestCommand.h"

#include <cm/memory>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmTest.h"
#include "cmTestGenerator.h"

static bool cmAddTestCommandHandleNameMode(
  std::vector<std::string> const& args, cmExecutionStatus& status);

bool cmAddTestCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (!args.empty() && args[0] == "NAME") {
    return cmAddTestCommandHandleNameMode(args, status);
  }

  // First argument is the name of the test Second argument is the name of
  // the executable to run (a target or external program) Remaining arguments
  // are the arguments to pass to the executable
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  // Collect the command with arguments.
  std::vector<std::string> command(args.begin() + 1, args.end());

  // Create the test but add a generator only the first time it is
  // seen.  This preserves behavior from before test generators.
  cmTest* test = mf.GetTest(args[0]);
  if (test) {
    // If the test was already added by a new-style signature do not
    // allow it to be duplicated.
    if (!test->GetOldStyle()) {
      status.SetError(cmStrCat(" given test name \"", args[0],
                               "\" which already exists in this directory."));
      return false;
    }
  } else {
    test = mf.CreateTest(args[0]);
    test->SetOldStyle(true);
    mf.AddTestGenerator(cm::make_unique<cmTestGenerator>(test));
  }
  test->SetCommand(command);

  return true;
}

bool cmAddTestCommandHandleNameMode(std::vector<std::string> const& args,
                                    cmExecutionStatus& status)
{
  std::string name;
  std::vector<std::string> configurations;
  std::string working_directory;
  std::vector<std::string> command;
  bool command_expand_lists = false;

  // Read the arguments.
  enum Doing
  {
    DoingName,
    DoingCommand,
    DoingConfigs,
    DoingWorkingDirectory,
    DoingNone
  };
  Doing doing = DoingName;
  for (unsigned int i = 1; i < args.size(); ++i) {
    if (args[i] == "COMMAND") {
      if (!command.empty()) {
        status.SetError(" may be given at most one COMMAND.");
        return false;
      }
      doing = DoingCommand;
    } else if (args[i] == "CONFIGURATIONS") {
      if (!configurations.empty()) {
        status.SetError(" may be given at most one set of CONFIGURATIONS.");
        return false;
      }
      doing = DoingConfigs;
    } else if (args[i] == "WORKING_DIRECTORY") {
      if (!working_directory.empty()) {
        status.SetError(" may be given at most one WORKING_DIRECTORY.");
        return false;
      }
      doing = DoingWorkingDirectory;
    } else if (args[i] == "COMMAND_EXPAND_LISTS") {
      if (command_expand_lists) {
        status.SetError(" may be given at most one COMMAND_EXPAND_LISTS.");
        return false;
      }
      command_expand_lists = true;
      doing = DoingNone;
    } else if (doing == DoingName) {
      name = args[i];
      doing = DoingNone;
    } else if (doing == DoingCommand) {
      command.push_back(args[i]);
    } else if (doing == DoingConfigs) {
      configurations.push_back(args[i]);
    } else if (doing == DoingWorkingDirectory) {
      working_directory = args[i];
      doing = DoingNone;
    } else {
      status.SetError(cmStrCat(" given unknown argument:\n  ", args[i], "\n"));
      return false;
    }
  }

  // Require a test name.
  if (name.empty()) {
    status.SetError(" must be given non-empty NAME.");
    return false;
  }

  // Require a command.
  if (command.empty()) {
    status.SetError(" must be given non-empty COMMAND.");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Require a unique test name within the directory.
  if (mf.GetTest(name)) {
    status.SetError(cmStrCat(" given test NAME \"", name,
                             "\" which already exists in this directory."));
    return false;
  }

  // Add the test.
  cmTest* test = mf.CreateTest(name);
  test->SetOldStyle(false);
  test->SetCommand(command);
  if (!working_directory.empty()) {
    test->SetProperty("WORKING_DIRECTORY", working_directory.c_str());
  }
  test->SetCommandExpandLists(command_expand_lists);
  mf.AddTestGenerator(cm::make_unique<cmTestGenerator>(test, configurations));

  return true;
}
