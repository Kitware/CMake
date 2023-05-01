/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePathCommand.h"

#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCMakePath.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {
// Helper classes for argument parsing
template <typename Result>
class CMakePathArgumentParser : public cmArgumentParser<Result>
{
public:
  CMakePathArgumentParser()
    : cmArgumentParser<Result>()
  {
  }

  template <typename T>
  CMakePathArgumentParser& Bind(cm::static_string_view name, T Result::*member)
  {
    this->cmArgumentParser<Result>::Bind(name, member);
    return *this;
  }

  template <int Advance = 2>
  Result Parse(std::vector<std::string> const& args) const
  {
    this->Inputs.clear();

    return this->cmArgumentParser<Result>::Parse(
      cmMakeRange(args).advance(Advance), &this->Inputs);
  }

  const std::vector<std::string>& GetInputs() const { return this->Inputs; }

protected:
  mutable std::vector<std::string> Inputs;
};

// OUTPUT_VARIABLE is expected
template <typename Result>
class ArgumentParserWithOutputVariable : public CMakePathArgumentParser<Result>
{
public:
  ArgumentParserWithOutputVariable()
    : CMakePathArgumentParser<Result>()
  {
    this->Bind("OUTPUT_VARIABLE"_s, &Result::Output);
  }

  template <typename T>
  ArgumentParserWithOutputVariable& Bind(cm::static_string_view name,
                                         T Result::*member)
  {
    this->cmArgumentParser<Result>::Bind(name, member);
    return *this;
  }

  template <int Advance = 2>
  Result Parse(std::vector<std::string> const& args) const
  {
    return this->CMakePathArgumentParser<Result>::template Parse<Advance>(
      args);
  }
};

struct OutputVariable : public ArgumentParser::ParseResult
{
  cm::optional<ArgumentParser::NonEmpty<std::string>> Output;
};
// Usable when OUTPUT_VARIABLE is the only option
class OutputVariableParser
  : public ArgumentParserWithOutputVariable<OutputVariable>
{
};

struct NormalizeOption
{
  bool Normalize = false;
};
// Usable when NORMALIZE is the only option
class NormalizeParser : public CMakePathArgumentParser<NormalizeOption>
{
public:
  NormalizeParser() { this->Bind("NORMALIZE"_s, &NormalizeOption::Normalize); }
};

// retrieve value of input path from specified variable
bool getInputPath(const std::string& arg, cmExecutionStatus& status,
                  std::string& path)
{
  cmValue def = status.GetMakefile().GetDefinition(arg);
  if (!def) {
    status.SetError("undefined variable for input path.");
    return false;
  }

  path = *def;
  return true;
}

bool HandleGetCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  static std::map<cm::string_view,
                  std::function<cmCMakePath(const cmCMakePath&, bool)>> const
    actions{ { "ROOT_NAME"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetRootName();
               } },
             { "ROOT_DIRECTORY"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetRootDirectory();
               } },
             { "ROOT_PATH"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetRootPath();
               } },
             { "FILENAME"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetFileName();
               } },
             { "EXTENSION"_s,
               [](const cmCMakePath& path, bool last_only) -> cmCMakePath {
                 if (last_only) {
                   return path.GetExtension();
                 }
                 return path.GetWideExtension();
               } },
             { "STEM"_s,
               [](const cmCMakePath& path, bool last_only) -> cmCMakePath {
                 if (last_only) {
                   return path.GetStem();
                 }
                 return path.GetNarrowStem();
               } },
             { "RELATIVE_PART"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetRelativePath();
               } },
             { "PARENT_PATH"_s,
               [](const cmCMakePath& path, bool) -> cmCMakePath {
                 return path.GetParentPath();
               } } };

  if (args.size() < 4) {
    status.SetError("GET must be called with at least three arguments.");
    return false;
  }

  const auto& action = args[2];

  if (actions.find(action) == actions.end()) {
    status.SetError(
      cmStrCat("GET called with an unknown action: ", action, "."));
    return false;
  }

  struct Arguments
  {
    bool LastOnly = false;
  };

  CMakePathArgumentParser<Arguments> parser;
  if ((action == "EXTENSION"_s || action == "STEM"_s)) {
    parser.Bind("LAST_ONLY"_s, &Arguments::LastOnly);
  }

  Arguments const arguments = parser.Parse<3>(args);

  if (parser.GetInputs().size() != 1) {
    status.SetError("GET called with unexpected arguments.");
    return false;
  }
  if (parser.GetInputs().front().empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  std::string path;
  if (!getInputPath(args[1], status, path)) {
    return false;
  }

  auto result = actions.at(action)(path, arguments.LastOnly);

  status.GetMakefile().AddDefinition(parser.GetInputs().front(),
                                     result.String());

  return true;
}

bool HandleSetCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.size() < 3 || args.size() > 4) {
    status.SetError("SET must be called with two or three arguments.");
    return false;
  }

  if (args[1].empty()) {
    status.SetError("Invalid name for path variable.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse(args);

  if (parser.GetInputs().size() != 1) {
    status.SetError("SET called with unexpected arguments.");
    return false;
  }

  auto path =
    cmCMakePath(parser.GetInputs().front(), cmCMakePath::native_format);

  if (arguments.Normalize) {
    path = path.Normal();
  }

  status.GetMakefile().AddDefinition(args[1], path.GenericString());

  return true;
}

bool HandleAppendCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  if (args[1].empty()) {
    status.SetError("Invalid name for path variable.");
    return false;
  }

  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  cmCMakePath path(status.GetMakefile().GetSafeDefinition(args[1]));
  for (const auto& input : parser.GetInputs()) {
    path /= input;
  }

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleAppendStringCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);
  for (const auto& input : parser.GetInputs()) {
    path += input;
  }

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleRemoveFilenameCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!parser.GetInputs().empty()) {
    status.SetError("REMOVE_FILENAME called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);
  path.RemoveFileName();

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleReplaceFilenameCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (parser.GetInputs().size() > 1) {
    status.SetError("REPLACE_FILENAME called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);
  path.ReplaceFileName(
    parser.GetInputs().empty() ? "" : parser.GetInputs().front());

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleRemoveExtensionCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::string>> Output;
    bool LastOnly = false;
  };

  static auto const parser =
    ArgumentParserWithOutputVariable<Arguments>{}.Bind("LAST_ONLY"_s,
                                                       &Arguments::LastOnly);

  Arguments const arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!parser.GetInputs().empty()) {
    status.SetError("REMOVE_EXTENSION called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);

  if (arguments.LastOnly) {
    path.RemoveExtension();
  } else {
    path.RemoveWideExtension();
  }

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleReplaceExtensionCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::string>> Output;
    bool LastOnly = false;
  };

  static auto const parser =
    ArgumentParserWithOutputVariable<Arguments>{}.Bind("LAST_ONLY"_s,
                                                       &Arguments::LastOnly);

  Arguments const arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (parser.GetInputs().size() > 1) {
    status.SetError("REPLACE_EXTENSION called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);
  cmCMakePath extension(
    parser.GetInputs().empty() ? "" : parser.GetInputs().front());

  if (arguments.LastOnly) {
    path.ReplaceExtension(extension);
  } else {
    path.ReplaceWideExtension(extension);
  }

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleNormalPathCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!parser.GetInputs().empty()) {
    status.SetError("NORMAL_PATH called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  auto path = cmCMakePath(inputPath).Normal();

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleTransformPathCommand(
  std::vector<std::string> const& args, cmExecutionStatus& status,
  const std::function<cmCMakePath(const cmCMakePath&,
                                  const std::string& base)>& transform,
  bool normalizeOption = false)
{
  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::string>> Output;
    cm::optional<std::string> BaseDirectory;
    bool Normalize = false;
  };

  auto parser = ArgumentParserWithOutputVariable<Arguments>{}.Bind(
    "BASE_DIRECTORY"_s, &Arguments::BaseDirectory);
  if (normalizeOption) {
    parser.Bind("NORMALIZE"_s, &Arguments::Normalize);
  }

  Arguments arguments = parser.Parse(args);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!parser.GetInputs().empty()) {
    status.SetError(cmStrCat(args[0], " called with unexpected arguments."));
    return false;
  }

  std::string baseDirectory;
  if (arguments.BaseDirectory) {
    baseDirectory = *arguments.BaseDirectory;
  } else {
    baseDirectory = status.GetMakefile().GetCurrentSourceDirectory();
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  auto path = transform(cmCMakePath(inputPath), baseDirectory);
  if (arguments.Normalize) {
    path = path.Normal();
  }

  status.GetMakefile().AddDefinition(
    arguments.Output ? *arguments.Output : args[1], path.String());

  return true;
}

bool HandleRelativePathCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  return HandleTransformPathCommand(
    args, status,
    [](const cmCMakePath& path, const std::string& base) -> cmCMakePath {
      return path.Relative(base);
    });
}

bool HandleAbsolutePathCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  return HandleTransformPathCommand(
    args, status,
    [](const cmCMakePath& path, const std::string& base) -> cmCMakePath {
      return path.Absolute(base);
    },
    true);
}

bool HandleNativePathCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() < 3 || args.size() > 4) {
    status.SetError("NATIVE_PATH must be called with two or three arguments.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse(args);

  if (parser.GetInputs().size() != 1) {
    status.SetError("NATIVE_PATH called with unexpected arguments.");
    return false;
  }
  if (parser.GetInputs().front().empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path(inputPath);
  if (arguments.Normalize) {
    path = path.Normal();
  }

  status.GetMakefile().AddDefinition(parser.GetInputs().front(),
                                     path.NativeString());

  return true;
}

bool HandleConvertCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  const auto pathSep = ";"_s;
#else
  const auto pathSep = ":"_s;
#endif
  const auto cmakePath = "TO_CMAKE_PATH_LIST"_s;
  const auto nativePath = "TO_NATIVE_PATH_LIST"_s;

  if (args.size() < 4 || args.size() > 5) {
    status.SetError("CONVERT must be called with three or four arguments.");
    return false;
  }

  const auto& action = args[2];

  if (action != cmakePath && action != nativePath) {
    status.SetError(
      cmStrCat("CONVERT called with an unknown action: ", action, "."));
    return false;
  }

  if (args[3].empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse<4>(args);

  if (!parser.GetInputs().empty()) {
    status.SetError("CONVERT called with unexpected arguments.");
    return false;
  }

  cmList paths;

  if (action == cmakePath) {
    paths = cmSystemTools::SplitString(args[1], pathSep.front());
  } else {
    paths.assign(args[1]);
  }

  for (auto& path : paths) {
    auto p = cmCMakePath(path,
                         action == cmakePath ? cmCMakePath::native_format
                                             : cmCMakePath::generic_format);
    if (arguments.Normalize) {
      p = p.Normal();
    }
    if (action == cmakePath) {
      path = p.GenericString();
    } else {
      path = p.NativeString();
    }
  }

  auto value = action == cmakePath ? paths.to_string() : paths.join(pathSep);
  status.GetMakefile().AddDefinition(args[3], value);

  return true;
}

bool HandleCompareCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.size() != 5) {
    status.SetError("COMPARE must be called with four arguments.");
    return false;
  }

  static std::map<cm::string_view,
                  std::function<bool(const cmCMakePath&,
                                     const cmCMakePath&)>> const operators{
    { "EQUAL"_s,
      [](const cmCMakePath& path1, const cmCMakePath& path2) -> bool {
        return path1 == path2;
      } },
    { "NOT_EQUAL"_s,
      [](const cmCMakePath& path1, const cmCMakePath& path2) -> bool {
        return path1 != path2;
      } }
  };

  const auto op = operators.find(args[2]);
  if (op == operators.end()) {
    status.SetError(cmStrCat(
      "COMPARE called with an unknown comparison operator: ", args[2], "."));
    return false;
  }

  if (args[4].empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  cmCMakePath path1(args[1]);
  cmCMakePath path2(args[3]);
  auto result = op->second(path1, path2);

  status.GetMakefile().AddDefinitionBool(args[4], result);

  return true;
}

bool HandleHasItemCommand(
  std::vector<std::string> const& args, cmExecutionStatus& status,
  const std::function<bool(const cmCMakePath&)>& has_item)
{
  if (args.size() != 3) {
    status.SetError(
      cmStrCat(args.front(), " must be called with two arguments."));
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  if (args[2].empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  cmCMakePath path(inputPath);
  auto result = has_item(path);

  status.GetMakefile().AddDefinitionBool(args[2], result);

  return true;
}

bool HandleHasRootNameCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasRootName(); });
}

bool HandleHasRootDirectoryCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasRootDirectory(); });
}

bool HandleHasRootPathCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasRootPath(); });
}

bool HandleHasFilenameCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasFileName(); });
}

bool HandleHasExtensionCommand(std::vector<std::string> const& args,
                               cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasExtension(); });
}

bool HandleHasStemCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasStem(); });
}

bool HandleHasRelativePartCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasRelativePath(); });
}

bool HandleHasParentPathCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  return HandleHasItemCommand(
    args, status,
    [](const cmCMakePath& path) -> bool { return path.HasParentPath(); });
}

bool HandleIsAbsoluteCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("IS_ABSOLUTE must be called with two arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  if (args[2].empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  bool isAbsolute = cmCMakePath(inputPath).IsAbsolute();

  status.GetMakefile().AddDefinitionBool(args[2], isAbsolute);

  return true;
}

bool HandleIsRelativeCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("IS_RELATIVE must be called with two arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  if (args[2].empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  bool isRelative = cmCMakePath(inputPath).IsRelative();

  status.GetMakefile().AddDefinitionBool(args[2], isRelative);

  return true;
}

bool HandleIsPrefixCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  if (args.size() < 4 || args.size() > 5) {
    status.SetError("IS_PREFIX must be called with three or four arguments.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse(args);

  if (parser.GetInputs().size() != 2) {
    status.SetError("IS_PREFIX called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  const auto& input = parser.GetInputs().front();
  const auto& output = parser.GetInputs().back();

  if (output.empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  bool isPrefix;
  if (arguments.Normalize) {
    isPrefix =
      cmCMakePath(inputPath).Normal().IsPrefix(cmCMakePath(input).Normal());
  } else {
    isPrefix = cmCMakePath(inputPath).IsPrefix(input);
  }

  status.GetMakefile().AddDefinitionBool(output, isPrefix);

  return true;
}

bool HandleHashCommand(std::vector<std::string> const& args,
                       cmExecutionStatus& status)
{
  if (args.size() != 3) {
    status.SetError("HASH must be called with two arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  const auto& output = args[2];

  if (output.empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  auto hash = hash_value(cmCMakePath(inputPath).Normal());

  std::ostringstream out;
  out << std::setbase(16) << hash;

  status.GetMakefile().AddDefinition(output, out.str());

  return true;
}
} // anonymous namespace

bool cmCMakePathCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("must be called with at least two arguments.");
    return false;
  }

  static cmSubcommandTable const subcommand{
    { "GET"_s, HandleGetCommand },
    { "SET"_s, HandleSetCommand },
    { "APPEND"_s, HandleAppendCommand },
    { "APPEND_STRING"_s, HandleAppendStringCommand },
    { "REMOVE_FILENAME"_s, HandleRemoveFilenameCommand },
    { "REPLACE_FILENAME"_s, HandleReplaceFilenameCommand },
    { "REMOVE_EXTENSION"_s, HandleRemoveExtensionCommand },
    { "REPLACE_EXTENSION"_s, HandleReplaceExtensionCommand },
    { "NORMAL_PATH"_s, HandleNormalPathCommand },
    { "RELATIVE_PATH"_s, HandleRelativePathCommand },
    { "ABSOLUTE_PATH"_s, HandleAbsolutePathCommand },
    { "NATIVE_PATH"_s, HandleNativePathCommand },
    { "CONVERT"_s, HandleConvertCommand },
    { "COMPARE"_s, HandleCompareCommand },
    { "HAS_ROOT_NAME"_s, HandleHasRootNameCommand },
    { "HAS_ROOT_DIRECTORY"_s, HandleHasRootDirectoryCommand },
    { "HAS_ROOT_PATH"_s, HandleHasRootPathCommand },
    { "HAS_FILENAME"_s, HandleHasFilenameCommand },
    { "HAS_EXTENSION"_s, HandleHasExtensionCommand },
    { "HAS_STEM"_s, HandleHasStemCommand },
    { "HAS_RELATIVE_PART"_s, HandleHasRelativePartCommand },
    { "HAS_PARENT_PATH"_s, HandleHasParentPathCommand },
    { "IS_ABSOLUTE"_s, HandleIsAbsoluteCommand },
    { "IS_RELATIVE"_s, HandleIsRelativeCommand },
    { "IS_PREFIX"_s, HandleIsPrefixCommand },
    { "HASH"_s, HandleHashCommand }
  };

  return subcommand(args[0], args, status);
}
