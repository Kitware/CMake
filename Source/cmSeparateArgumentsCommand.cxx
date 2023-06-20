/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSeparateArgumentsCommand.h"

#include <algorithm>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

// cmSeparateArgumentsCommand
bool cmSeparateArgumentsCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("must be given at least one argument.");
    return false;
  }

  std::string const& var = args.front();

  if (args.size() == 1) {
    // Original space-replacement version of command.
    if (cmValue def = status.GetMakefile().GetDefinition(var)) {
      std::string value = *def;
      std::replace(value.begin(), value.end(), ' ', ';');
      status.GetMakefile().AddDefinition(var, value);
    }

    return true;
  }

  struct Arguments
  {
    bool UnixCommand = false;
    bool WindowsCommand = false;
    bool NativeCommand = false;
    bool Program = false;
    bool SeparateArgs = false;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("UNIX_COMMAND"_s, &Arguments::UnixCommand)
      .Bind("WINDOWS_COMMAND"_s, &Arguments::WindowsCommand)
      .Bind("NATIVE_COMMAND"_s, &Arguments::NativeCommand)
      .Bind("PROGRAM"_s, &Arguments::Program)
      .Bind("SEPARATE_ARGS"_s, &Arguments::SeparateArgs);

  std::vector<std::string> unparsedArguments;
  Arguments arguments =
    parser.Parse(cmMakeRange(args).advance(1), &unparsedArguments);

  if (!arguments.UnixCommand && !arguments.WindowsCommand &&
      !arguments.NativeCommand) {
    status.SetError("missing required option: 'UNIX_COMMAND' or "
                    "'WINDOWS_COMMAND' or 'NATIVE_COMMAND'");
    return false;
  }
  if ((arguments.UnixCommand && arguments.WindowsCommand) ||
      (arguments.UnixCommand && arguments.NativeCommand) ||
      (arguments.WindowsCommand && arguments.NativeCommand)) {
    status.SetError("'UNIX_COMMAND', 'WINDOWS_COMMAND' and 'NATIVE_COMMAND' "
                    "are mutually exclusive");
    return false;
  }
  if (arguments.SeparateArgs && !arguments.Program) {
    status.SetError("`SEPARATE_ARGS` option requires `PROGRAM' option");
    return false;
  }

  if (unparsedArguments.size() > 1) {
    status.SetError("given unexpected argument(s)");
    return false;
  }

  if (unparsedArguments.empty()) {
    status.GetMakefile().AddDefinition(var, cm::string_view{});
    return true;
  }

  std::string& command = unparsedArguments.front();

  if (command.empty()) {
    status.GetMakefile().AddDefinition(var, command);
    return true;
  }

  if (arguments.Program && !arguments.SeparateArgs) {
    std::string program;
    std::string programArgs;

    // First assume the path to the program was specified with no
    // arguments and with no quoting or escaping for spaces.
    // Only bother doing this if there is non-whitespace.
    if (!cmTrimWhitespace(command).empty()) {
      program = cmSystemTools::FindProgram(command);
    }

    // If that failed then assume a command-line string was given
    // and split the program part from the rest of the arguments.
    if (program.empty()) {
      if (cmSystemTools::SplitProgramFromArgs(command, program, programArgs)) {
        if (!cmSystemTools::FileExists(program)) {
          program = cmSystemTools::FindProgram(program);
        }
      }
    }

    if (!program.empty()) {
      program += cmStrCat(';', programArgs);
    }

    status.GetMakefile().AddDefinition(var, program);
    return true;
  }

  // split command given
  std::vector<std::string> values;

  if (arguments.NativeCommand) {
#if defined(_WIN32)
    arguments.WindowsCommand = true;
#else
    arguments.UnixCommand = true;
#endif
  }

  if (arguments.UnixCommand) {
    cmSystemTools::ParseUnixCommandLine(command.c_str(), values);
  } else {
    cmSystemTools::ParseWindowsCommandLine(command.c_str(), values);
  }

  if (arguments.Program) {
    // check program exist
    if (!cmSystemTools::FileExists(values.front())) {
      auto result = cmSystemTools::FindProgram(values.front());
      if (result.empty()) {
        values.clear();
      } else {
        values.front() = result;
      }
    }
  }

  // preserve semicolons in arguments
  std::for_each(values.begin(), values.end(), [](std::string& value) {
    std::string::size_type pos = 0;
    while ((pos = value.find_first_of(';', pos)) != std::string::npos) {
      value.insert(pos, 1, '\\');
      pos += 2;
    }
  });
  auto value = cmList::to_string(values);
  status.GetMakefile().AddDefinition(var, value);

  return true;
}
