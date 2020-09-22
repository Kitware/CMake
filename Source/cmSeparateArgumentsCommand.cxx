/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSeparateArgumentsCommand.h"

#include <algorithm>

#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

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
    if (cmProp def = status.GetMakefile().GetDefinition(var)) {
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
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("UNIX_COMMAND"_s, &Arguments::UnixCommand)
      .Bind("WINDOWS_COMMAND"_s, &Arguments::WindowsCommand)
      .Bind("NATIVE_COMMAND"_s, &Arguments::NativeCommand);

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

  if (unparsedArguments.size() > 1) {
    status.SetError("given unexpected argument(s)");
    return false;
  }

  std::string& command = unparsedArguments.front();

  if (command.empty()) {
    status.GetMakefile().AddDefinition(var, command);
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

  // preserve semicolons in arguments
  std::for_each(values.begin(), values.end(), [](std::string& value) {
    std::string::size_type pos = 0;
    while ((pos = value.find_first_of(';', pos)) != std::string::npos) {
      value.insert(pos, 1, '\\');
      pos += 2;
    }
  });
  auto value = cmJoin(values, ";");
  status.GetMakefile().AddDefinition(var, value);

  return true;
}
