/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeLanguageCommand.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmDependencyProvider.h"
#include "cmExecutionStatus.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

namespace {

bool FatalError(cmExecutionStatus& status, std::string const& error)
{
  status.SetError(error);
  cmSystemTools::SetFatalErrorOccurred();
  return false;
}

std::array<cm::static_string_view, 14> InvalidCommands{
  { // clang-format off
  "function"_s, "endfunction"_s,
  "macro"_s, "endmacro"_s,
  "if"_s, "elseif"_s, "else"_s, "endif"_s,
  "while"_s, "endwhile"_s,
  "foreach"_s, "endforeach"_s,
  "block"_s, "endblock"_s
  } // clang-format on
};

std::array<cm::static_string_view, 1> InvalidDeferCommands{
  {
    // clang-format off
  "return"_s,
  } // clang-format on
};

struct Defer
{
  std::string Id;
  std::string IdVar;
  cmMakefile* Directory = nullptr;
};

bool cmCMakeLanguageCommandCALL(std::vector<cmListFileArgument> const& args,
                                std::string const& callCommand,
                                size_t startArg, cm::optional<Defer> defer,
                                cmExecutionStatus& status)
{
  // ensure specified command is valid
  // start/end flow control commands are not allowed
  auto cmd = cmSystemTools::LowerCase(callCommand);
  if (std::find(InvalidCommands.cbegin(), InvalidCommands.cend(), cmd) !=
      InvalidCommands.cend()) {
    return FatalError(status,
                      cmStrCat("invalid command specified: "_s, callCommand));
  }
  if (defer &&
      std::find(InvalidDeferCommands.cbegin(), InvalidDeferCommands.cend(),
                cmd) != InvalidDeferCommands.cend()) {
    return FatalError(status,
                      cmStrCat("invalid command specified: "_s, callCommand));
  }

  cmMakefile& makefile = status.GetMakefile();
  cmListFileContext context = makefile.GetBacktrace().Top();

  std::vector<cmListFileArgument> funcArgs;
  funcArgs.reserve(args.size() - startArg);

  // The rest of the arguments are passed to the function call above
  for (size_t i = startArg; i < args.size(); ++i) {
    funcArgs.emplace_back(args[i].Value, args[i].Delim, context.Line);
  }
  cmListFileFunction func{ callCommand, context.Line, context.Line,
                           std::move(funcArgs) };

  if (defer) {
    if (defer->Id.empty()) {
      defer->Id = makefile.NewDeferId();
    }
    if (!defer->IdVar.empty()) {
      makefile.AddDefinition(defer->IdVar, defer->Id);
    }
    cmMakefile* deferMakefile =
      defer->Directory ? defer->Directory : &makefile;
    if (!deferMakefile->DeferCall(defer->Id, context.FilePath, func)) {
      return FatalError(
        status,
        cmStrCat("DEFER CALL may not be scheduled in directory:\n  "_s,
                 deferMakefile->GetCurrentBinaryDirectory(),
                 "\nat this time."_s));
    }
    return true;
  }
  return makefile.ExecuteCommand(func, status);
}

bool cmCMakeLanguageCommandDEFER(Defer const& defer,
                                 std::vector<std::string> const& args,
                                 size_t arg, cmExecutionStatus& status)
{
  cmMakefile* deferMakefile =
    defer.Directory ? defer.Directory : &status.GetMakefile();
  if (args[arg] == "CANCEL_CALL"_s) {
    ++arg; // Consume CANCEL_CALL.
    auto ids = cmMakeRange(args).advance(arg);
    for (std::string const& id : ids) {
      if (id[0] >= 'A' && id[0] <= 'Z') {
        return FatalError(
          status, cmStrCat("DEFER CANCEL_CALL unknown argument:\n  "_s, id));
      }
      if (!deferMakefile->DeferCancelCall(id)) {
        return FatalError(
          status,
          cmStrCat("DEFER CANCEL_CALL may not update directory:\n  "_s,
                   deferMakefile->GetCurrentBinaryDirectory(),
                   "\nat this time."_s));
      }
    }
    return true;
  }
  if (args[arg] == "GET_CALL_IDS"_s) {
    ++arg; // Consume GET_CALL_IDS.
    if (arg == args.size()) {
      return FatalError(status, "DEFER GET_CALL_IDS missing output variable");
    }
    std::string const& var = args[arg++];
    if (arg != args.size()) {
      return FatalError(status, "DEFER GET_CALL_IDS given too many arguments");
    }
    cm::optional<std::string> ids = deferMakefile->DeferGetCallIds();
    if (!ids) {
      return FatalError(
        status,
        cmStrCat("DEFER GET_CALL_IDS may not access directory:\n  "_s,
                 deferMakefile->GetCurrentBinaryDirectory(),
                 "\nat this time."_s));
    }
    status.GetMakefile().AddDefinition(var, *ids);
    return true;
  }
  if (args[arg] == "GET_CALL"_s) {
    ++arg; // Consume GET_CALL.
    if (arg == args.size()) {
      return FatalError(status, "DEFER GET_CALL missing id");
    }
    std::string const& id = args[arg++];
    if (arg == args.size()) {
      return FatalError(status, "DEFER GET_CALL missing output variable");
    }
    std::string const& var = args[arg++];
    if (arg != args.size()) {
      return FatalError(status, "DEFER GET_CALL given too many arguments");
    }
    if (id.empty()) {
      return FatalError(status, "DEFER GET_CALL id may not be empty");
    }
    if (id[0] >= 'A' && id[0] <= 'Z') {
      return FatalError(status,
                        cmStrCat("DEFER GET_CALL unknown argument:\n "_s, id));
    }
    cm::optional<std::string> call = deferMakefile->DeferGetCall(id);
    if (!call) {
      return FatalError(
        status,
        cmStrCat("DEFER GET_CALL may not access directory:\n  "_s,
                 deferMakefile->GetCurrentBinaryDirectory(),
                 "\nat this time."_s));
    }
    status.GetMakefile().AddDefinition(var, *call);
    return true;
  }
  return FatalError(status,
                    cmStrCat("DEFER operation unknown: "_s, args[arg]));
}

bool cmCMakeLanguageCommandEVAL(std::vector<cmListFileArgument> const& args,
                                cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  cmListFileContext context = makefile.GetBacktrace().Top();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  if (expandedArgs.size() < 2) {
    return FatalError(status, "called with incorrect number of arguments");
  }

  if (expandedArgs[1] != "CODE") {
    auto code_iter =
      std::find(expandedArgs.begin() + 2, expandedArgs.end(), "CODE");
    if (code_iter == expandedArgs.end()) {
      return FatalError(status, "called without CODE argument");
    }
    return FatalError(
      status,
      "called with unsupported arguments between EVAL and CODE arguments");
  }

  const std::string code =
    cmJoin(cmMakeRange(expandedArgs.begin() + 2, expandedArgs.end()), " ");
  return makefile.ReadListFileAsString(
    code, cmStrCat(context.FilePath, ":", context.Line, ":EVAL"));
}

bool cmCMakeLanguageCommandSET_DEPENDENCY_PROVIDER(
  std::vector<std::string> const& args, cmExecutionStatus& status)
{
  cmState* state = status.GetMakefile().GetState();
  if (!state->InTopLevelIncludes()) {
    return FatalError(
      status,
      "Dependency providers can only be set as part of the first call to "
      "project(). More specifically, cmake_language(SET_DEPENDENCY_PROVIDER) "
      "can only be called while the first project() command processes files "
      "listed in CMAKE_PROJECT_TOP_LEVEL_INCLUDES.");
  }

  struct SetProviderArgs
  {
    std::string Command;
    ArgumentParser::NonEmpty<std::vector<std::string>> Methods;
  };

  auto const ArgsParser =
    cmArgumentParser<SetProviderArgs>()
      .Bind("SET_DEPENDENCY_PROVIDER"_s, &SetProviderArgs::Command)
      .Bind("SUPPORTED_METHODS"_s, &SetProviderArgs::Methods);

  std::vector<std::string> unparsed;
  auto parsedArgs = ArgsParser.Parse(args, &unparsed);

  if (!unparsed.empty()) {
    return FatalError(
      status, cmStrCat("Unrecognized keyword: \"", unparsed.front(), "\""));
  }

  // We store the command that FetchContent_MakeAvailable() can call in a
  // global (but considered internal) property. If the provider doesn't
  // support this method, we set this property to an empty string instead.
  // This simplifies the logic in FetchContent_MakeAvailable() and doesn't
  // require us to define a new internal command or sub-command.
  std::string fcmasProperty = "__FETCHCONTENT_MAKEAVAILABLE_SERIAL_PROVIDER";

  if (parsedArgs.Command.empty()) {
    if (!parsedArgs.Methods.empty()) {
      return FatalError(status,
                        "Must specify a non-empty command name when provider "
                        "methods are given");
    }
    state->ClearDependencyProvider();
    state->SetGlobalProperty(fcmasProperty, "");
    return true;
  }

  cmState::Command command = state->GetCommand(parsedArgs.Command);
  if (!command) {
    return FatalError(status,
                      cmStrCat("Command \"", parsedArgs.Command,
                               "\" is not a defined command"));
  }

  if (parsedArgs.Methods.empty()) {
    return FatalError(status, "Must specify at least one provider method");
  }

  bool supportsFetchContentMakeAvailableSerial = false;
  std::vector<cmDependencyProvider::Method> methods;
  for (auto const& method : parsedArgs.Methods) {
    if (method == "FIND_PACKAGE") {
      methods.emplace_back(cmDependencyProvider::Method::FindPackage);
    } else if (method == "FETCHCONTENT_MAKEAVAILABLE_SERIAL") {
      supportsFetchContentMakeAvailableSerial = true;
      methods.emplace_back(
        cmDependencyProvider::Method::FetchContentMakeAvailableSerial);
    } else {
      return FatalError(
        status,
        cmStrCat("Unknown dependency provider method \"", method, "\""));
    }
  }

  state->SetDependencyProvider({ parsedArgs.Command, methods });
  state->SetGlobalProperty(
    fcmasProperty,
    supportsFetchContentMakeAvailableSerial ? parsedArgs.Command : "");

  return true;
}

bool cmCMakeLanguageCommandGET_MESSAGE_LOG_LEVEL(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  if (args.size() < 2 || expandedArgs.size() > 2) {
    return FatalError(
      status,
      "sub-command GET_MESSAGE_LOG_LEVEL expects exactly one argument");
  }

  Message::LogLevel logLevel = makefile.GetCurrentLogLevel();
  std::string outputValue = cmake::LogLevelToString(logLevel);

  const std::string& outputVariable = expandedArgs[1];
  makefile.AddDefinition(outputVariable, outputValue);
  return true;
}
}

bool cmCMakeLanguageCommand(std::vector<cmListFileArgument> const& args,
                            cmExecutionStatus& status)
{
  std::vector<std::string> expArgs;
  size_t rawArg = 0;
  size_t expArg = 0;

  // Helper to consume and expand one raw argument at a time.
  auto moreArgs = [&]() -> bool {
    while (expArg >= expArgs.size()) {
      if (rawArg >= args.size()) {
        return false;
      }
      std::vector<cmListFileArgument> tmpArg;
      tmpArg.emplace_back(args[rawArg++]);
      status.GetMakefile().ExpandArguments(tmpArg, expArgs);
    }
    return true;
  };
  auto finishArgs = [&]() {
    std::vector<cmListFileArgument> tmpArgs(args.begin() + rawArg, args.end());
    status.GetMakefile().ExpandArguments(tmpArgs, expArgs);
    rawArg = args.size();
  };

  if (!moreArgs()) {
    return FatalError(status, "called with incorrect number of arguments");
  }

  if (expArgs[expArg] == "SET_DEPENDENCY_PROVIDER"_s) {
    finishArgs();
    return cmCMakeLanguageCommandSET_DEPENDENCY_PROVIDER(expArgs, status);
  }

  cm::optional<Defer> maybeDefer;
  if (expArgs[expArg] == "DEFER"_s) {
    ++expArg; // Consume "DEFER".

    if (!moreArgs()) {
      return FatalError(status, "DEFER requires at least one argument");
    }

    Defer defer;

    // Process optional arguments.
    while (moreArgs()) {
      if (expArgs[expArg] == "CALL"_s) {
        break;
      }
      if (expArgs[expArg] == "CANCEL_CALL"_s ||
          expArgs[expArg] == "GET_CALL_IDS"_s ||
          expArgs[expArg] == "GET_CALL"_s) {
        if (!defer.Id.empty() || !defer.IdVar.empty()) {
          return FatalError(status,
                            cmStrCat("DEFER "_s, expArgs[expArg],
                                     " does not accept ID or ID_VAR."_s));
        }
        finishArgs();
        return cmCMakeLanguageCommandDEFER(defer, expArgs, expArg, status);
      }
      if (expArgs[expArg] == "DIRECTORY"_s) {
        ++expArg; // Consume "DIRECTORY".
        if (defer.Directory) {
          return FatalError(status,
                            "DEFER given multiple DIRECTORY arguments");
        }
        if (!moreArgs()) {
          return FatalError(status, "DEFER DIRECTORY missing value");
        }
        std::string dir = expArgs[expArg++];
        if (dir.empty()) {
          return FatalError(status, "DEFER DIRECTORY may not be empty");
        }
        dir = cmSystemTools::CollapseFullPath(
          dir, status.GetMakefile().GetCurrentSourceDirectory());
        defer.Directory =
          status.GetMakefile().GetGlobalGenerator()->FindMakefile(dir);
        if (!defer.Directory) {
          return FatalError(status,
                            cmStrCat("DEFER DIRECTORY:\n  "_s, dir,
                                     "\nis not known.  "_s,
                                     "It may not have been processed yet."_s));
        }
      } else if (expArgs[expArg] == "ID"_s) {
        ++expArg; // Consume "ID".
        if (!defer.Id.empty()) {
          return FatalError(status, "DEFER given multiple ID arguments");
        }
        if (!moreArgs()) {
          return FatalError(status, "DEFER ID missing value");
        }
        defer.Id = expArgs[expArg++];
        if (defer.Id.empty()) {
          return FatalError(status, "DEFER ID may not be empty");
        }
        if (defer.Id[0] >= 'A' && defer.Id[0] <= 'Z') {
          return FatalError(status, "DEFER ID may not start in A-Z.");
        }
      } else if (expArgs[expArg] == "ID_VAR"_s) {
        ++expArg; // Consume "ID_VAR".
        if (!defer.IdVar.empty()) {
          return FatalError(status, "DEFER given multiple ID_VAR arguments");
        }
        if (!moreArgs()) {
          return FatalError(status, "DEFER ID_VAR missing variable name");
        }
        defer.IdVar = expArgs[expArg++];
        if (defer.IdVar.empty()) {
          return FatalError(status, "DEFER ID_VAR may not be empty");
        }
      } else {
        return FatalError(
          status, cmStrCat("DEFER unknown option:\n  "_s, expArgs[expArg]));
      }
    }

    if (!(moreArgs() && expArgs[expArg] == "CALL"_s)) {
      return FatalError(status, "DEFER must be followed by a CALL argument");
    }

    maybeDefer = std::move(defer);
  }

  if (expArgs[expArg] == "CALL") {
    ++expArg; // Consume "CALL".

    // CALL requires a command name.
    if (!moreArgs()) {
      return FatalError(status, "CALL missing command name");
    }
    std::string const& callCommand = expArgs[expArg++];

    // CALL accepts no further expanded arguments.
    if (expArg != expArgs.size()) {
      return FatalError(status, "CALL command's arguments must be literal");
    }

    // Run the CALL.
    return cmCMakeLanguageCommandCALL(args, callCommand, rawArg,
                                      std::move(maybeDefer), status);
  }

  if (expArgs[expArg] == "EVAL") {
    return cmCMakeLanguageCommandEVAL(args, status);
  }

  if (expArgs[expArg] == "GET_MESSAGE_LOG_LEVEL") {
    return cmCMakeLanguageCommandGET_MESSAGE_LOG_LEVEL(args, status);
  }

  return FatalError(status, "called with unknown meta-operation");
}
