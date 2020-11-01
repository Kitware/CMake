/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePathCommand.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCMakePath.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSubcommandTable.h"
#include "cmSystemTools.h"

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
    cmArgumentParser<Result>::Bind(name, member);
    return *this;
  }

  template <int Advance = 2>
  Result Parse(std::vector<std::string> const& args,
               std::vector<std::string>* keywordsMissingValue = nullptr,
               std::vector<std::string>* parsedKeywords = nullptr) const
  {
    this->Inputs.clear();

    return cmArgumentParser<Result>::Parse(cmMakeRange(args).advance(Advance),
                                           &this->Inputs, keywordsMissingValue,
                                           parsedKeywords);
  }

  const std::vector<std::string>& GetInputs() const { return Inputs; }

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
    cmArgumentParser<Result>::Bind(name, member);
    return *this;
  }

  template <int Advance = 2>
  Result Parse(std::vector<std::string> const& args) const
  {
    this->KeywordsMissingValue.clear();
    this->ParsedKeywords.clear();

    return CMakePathArgumentParser<Result>::template Parse<Advance>(
      args, &this->KeywordsMissingValue, &this->ParsedKeywords);
  }

  const std::vector<std::string>& GetKeywordsMissingValue() const
  {
    return this->KeywordsMissingValue;
  }
  const std::vector<std::string>& GetParsedKeywords() const
  {
    return this->ParsedKeywords;
  }

  bool checkOutputVariable(const Result& arguments,
                           cmExecutionStatus& status) const
  {
    if (std::find(this->GetKeywordsMissingValue().begin(),
                  this->GetKeywordsMissingValue().end(),
                  "OUTPUT_VARIABLE"_s) !=
        this->GetKeywordsMissingValue().end()) {
      status.SetError("OUTPUT_VARIABLE requires an argument.");
      return false;
    }

    if (std::find(this->GetParsedKeywords().begin(),
                  this->GetParsedKeywords().end(),
                  "OUTPUT_VARIABLE"_s) != this->GetParsedKeywords().end() &&
        arguments.Output.empty()) {
      status.SetError("Invalid name for output variable.");
      return false;
    }

    return true;
  }

private:
  mutable std::vector<std::string> KeywordsMissingValue;
  mutable std::vector<std::string> ParsedKeywords;
};

struct OutputVariable
{
  std::string Output;
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
  auto def = status.GetMakefile().GetDefinition(arg);
  if (def == nullptr) {
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
             { "RELATIVE_PATH"_s,
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

bool HandleAppendCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
  }

  cmCMakePath path(status.GetMakefile().GetSafeDefinition(args[1]));
  for (const auto& input : parser.GetInputs()) {
    path /= input;
  }

  status.GetMakefile().AddDefinition(
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleConcatCommand(std::vector<std::string> const& args,
                         cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleRemoveFilenameCommand(std::vector<std::string> const& args,
                                 cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleReplaceFilenameCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleRemoveExtensionCommand(std::vector<std::string> const& args,
                                  cmExecutionStatus& status)
{
  struct Arguments
  {
    std::string Output;
    bool LastOnly = false;
  };

  static auto const parser =
    ArgumentParserWithOutputVariable<Arguments>{}.Bind("LAST_ONLY"_s,
                                                       &Arguments::LastOnly);

  Arguments const arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleReplaceExtensionCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  struct Arguments
  {
    std::string Output;
    bool LastOnly = false;
  };

  static auto const parser =
    ArgumentParserWithOutputVariable<Arguments>{}.Bind("LAST_ONLY"_s,
                                                       &Arguments::LastOnly);

  Arguments const arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleNormalPathCommand(std::vector<std::string> const& args,
                             cmExecutionStatus& status)
{
  static OutputVariableParser const parser{};

  const auto arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
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
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

  return true;
}

bool HandleTransformPathCommand(
  std::vector<std::string> const& args, cmExecutionStatus& status,
  const std::function<cmCMakePath(const cmCMakePath&,
                                  const std::string& base)>& transform,
  bool normalizeOption = false)
{
  struct Arguments
  {
    std::string Output;
    std::string BaseDirectory;
    bool Normalize = false;
  };

  auto parser = ArgumentParserWithOutputVariable<Arguments>{}.Bind(
    "BASE_DIRECTORY"_s, &Arguments::BaseDirectory);
  if (normalizeOption) {
    parser.Bind("NORMALIZE"_s, &Arguments::Normalize);
  }

  Arguments arguments = parser.Parse(args);

  if (!parser.checkOutputVariable(arguments, status)) {
    return false;
  }

  if (!parser.GetInputs().empty()) {
    status.SetError(cmStrCat(args[0], " called with unexpected arguments."));
    return false;
  }

  if (std::find(parser.GetKeywordsMissingValue().begin(),
                parser.GetKeywordsMissingValue().end(), "BASE_DIRECTORY"_s) !=
      parser.GetKeywordsMissingValue().end()) {
    status.SetError("BASE_DIRECTORY requires an argument.");
    return false;
  }

  if (std::find(parser.GetParsedKeywords().begin(),
                parser.GetParsedKeywords().end(),
                "BASE_DIRECTORY"_s) == parser.GetParsedKeywords().end()) {
    arguments.BaseDirectory = status.GetMakefile().GetCurrentSourceDirectory();
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  auto path = transform(cmCMakePath(inputPath), arguments.BaseDirectory);
  if (arguments.Normalize) {
    path = path.Normal();
  }

  status.GetMakefile().AddDefinition(
    arguments.Output.empty() ? args[1] : arguments.Output, path.String());

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

bool HandleProximatePathCommand(std::vector<std::string> const& args,
                                cmExecutionStatus& status)
{
  return HandleTransformPathCommand(
    args, status,
    [](const cmCMakePath& path, const std::string& base) -> cmCMakePath {
      return path.Proximate(base);
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

bool HandleCMakePathCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3 || args.size() > 4) {
    status.SetError("CMAKE_PATH must be called with two or three arguments.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse(args);

  if (parser.GetInputs().size() != 1) {
    status.SetError("CMAKE_PATH called with unexpected arguments.");
    return false;
  }

  if (args[1].empty()) {
    status.SetError("Invalid name for output variable.");
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

  std::vector<std::string> paths;

  if (action == cmakePath) {
    paths = cmSystemTools::SplitString(args[1], pathSep.front());
  } else {
    cmExpandList(args[1], paths);
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

  auto value = cmJoin(paths, action == cmakePath ? ";"_s : pathSep);
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

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  cmCMakePath path1(inputPath);
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

bool HandleHasRelativePathCommand(std::vector<std::string> const& args,
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
  if (args.size() < 3 || args.size() > 4) {
    status.SetError("HASH must be called with two or three arguments.");
    return false;
  }

  static NormalizeParser const parser;

  const auto arguments = parser.Parse(args);

  if (parser.GetInputs().size() != 1) {
    status.SetError("HASH called with unexpected arguments.");
    return false;
  }

  std::string inputPath;
  if (!getInputPath(args[1], status, inputPath)) {
    return false;
  }

  const auto& output = parser.GetInputs().front();

  if (output.empty()) {
    status.SetError("Invalid name for output variable.");
    return false;
  }

  auto hash = hash_value(arguments.Normalize ? cmCMakePath(inputPath).Normal()
                                             : cmCMakePath(inputPath));

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
    { "APPEND"_s, HandleAppendCommand },
    { "CONCAT"_s, HandleConcatCommand },
    { "REMOVE_FILENAME"_s, HandleRemoveFilenameCommand },
    { "REPLACE_FILENAME"_s, HandleReplaceFilenameCommand },
    { "REMOVE_EXTENSION"_s, HandleRemoveExtensionCommand },
    { "REPLACE_EXTENSION"_s, HandleReplaceExtensionCommand },
    { "NORMAL_PATH"_s, HandleNormalPathCommand },
    { "RELATIVE_PATH"_s, HandleRelativePathCommand },
    { "PROXIMATE_PATH"_s, HandleProximatePathCommand },
    { "ABSOLUTE_PATH"_s, HandleAbsolutePathCommand },
    { "CMAKE_PATH"_s, HandleCMakePathCommand },
    { "NATIVE_PATH"_s, HandleNativePathCommand },
    { "CONVERT"_s, HandleConvertCommand },
    { "COMPARE"_s, HandleCompareCommand },
    { "HAS_ROOT_NAME"_s, HandleHasRootNameCommand },
    { "HAS_ROOT_DIRECTORY"_s, HandleHasRootDirectoryCommand },
    { "HAS_ROOT_PATH"_s, HandleHasRootPathCommand },
    { "HAS_FILENAME"_s, HandleHasFilenameCommand },
    { "HAS_EXTENSION"_s, HandleHasExtensionCommand },
    { "HAS_STEM"_s, HandleHasStemCommand },
    { "HAS_RELATIVE_PATH"_s, HandleHasRelativePathCommand },
    { "HAS_PARENT_PATH"_s, HandleHasParentPathCommand },
    { "IS_ABSOLUTE"_s, HandleIsAbsoluteCommand },
    { "IS_RELATIVE"_s, HandleIsRelativeCommand },
    { "IS_PREFIX"_s, HandleIsPrefixCommand },
    { "HASH"_s, HandleHashCommand }
  };

  return subcommand(args[0], args, status);
}
