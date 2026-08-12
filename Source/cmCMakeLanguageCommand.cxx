/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCMakeLanguageCommand.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmDependencyProvider.h"
#include "cmExecutionStatus.h"
#include "cmExperimental.h"
#include "cmGeneratorTarget.h"
#include "cmGetPropertyCommand.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h" // IWYU pragma: keep
#include "cmPropertyMap.h"
#include "cmRange.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetPropertyComputer.h"
#include "cmTargetPropertyHelper.h"
#include "cmValue.h"
#include "cmake.h"

namespace cm {
enum class TargetType;
}

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

  std::string const code =
    cmJoin(cmMakeRange(expandedArgs.begin() + 2, expandedArgs.end()), " ");
  return makefile.ReadListFileAsString(
    code, cmStrCat(context.FilePath, ':', context.Line, ":EVAL"));
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
      status, cmStrCat("Unrecognized keyword: \"", unparsed.front(), '"'));
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
        cmStrCat("Unknown dependency provider method \"", method, '"'));
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

  std::string const& outputVariable = expandedArgs[1];
  makefile.AddDefinition(outputVariable, outputValue);
  return true;
}

bool cmCMakeLanguageCommandGET_EXPERIMENTAL_FEATURE_ENABLED(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  if (expandedArgs.size() != 3) {
    return FatalError(status,
                      "sub-command GET_EXPERIMENTAL_FEATURE_ENABLED expects "
                      "exactly two arguments");
  }

  auto const& featureName = expandedArgs[1];
  auto const& variableName = expandedArgs[2];

  if (auto feature = cmExperimental::FeatureByName(featureName)) {
    if (cmExperimental::HasSupportEnabled(makefile, *feature)) {
      makefile.AddDefinition(variableName, "TRUE");
    } else {
      makefile.AddDefinition(variableName, "FALSE");
    }
  } else {
    return FatalError(status,
                      cmStrCat("Experimental feature name \"", featureName,
                               "\" does not exist."));
  }

  return true;
}

struct PrintTargetsArgs : public ArgumentParser::ParseResult
{
  cm::optional<std::string> Regex;
  bool ImportedOnly = false;
  bool NoImported = false;
  bool IgnoreCase = false;
};

// Lists every target that currently exists, optionally filtered by a
// name REGEX and by imported state.  "Currently exists" means anything
// CMake has defined up to this call: targets in the current directory,
// its ancestors, and any already-processed subdirectories.  Walking the
// global generator's makefiles captures exactly that set; a name-keyed
// map sorts the output and dedupes imported targets, which are inherited
// into child makefiles and would otherwise be seen many times.
bool cmCMakeLanguageCommandPRINT_TARGETS(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  // Drop the leading "PRINT_TARGETS" subcommand keyword.
  std::vector<std::string> body(expandedArgs.begin() + 1, expandedArgs.end());

  auto const ArgsParser =
    cmArgumentParser<PrintTargetsArgs>()
      .Bind("REGEX"_s, &PrintTargetsArgs::Regex)
      .Bind("IMPORTED_ONLY"_s, &PrintTargetsArgs::ImportedOnly)
      .Bind("NO_IMPORTED"_s, &PrintTargetsArgs::NoImported)
      .Bind("IGNORE_CASE"_s, &PrintTargetsArgs::IgnoreCase);

  std::vector<std::string> unparsed;
  auto parsedArgs = ArgsParser.Parse(body, &unparsed);

  if (!unparsed.empty()) {
    return FatalError(
      status,
      cmStrCat(
        "Unknown argument(s) given to cmake_language(PRINT_TARGETS) call: \"",
        cmJoin(unparsed, "\" \""), "\"."));
  }
  if (parsedArgs.MaybeReportError(makefile)) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  if (parsedArgs.ImportedOnly && parsedArgs.NoImported) {
    return FatalError(status,
                      "IMPORTED_ONLY and NO_IMPORTED keywords are mutually "
                      "exclusive in cmake_language(PRINT_TARGETS) call.");
  }

  if (parsedArgs.IgnoreCase && !parsedArgs.Regex) {
    return FatalError(status,
                      "IGNORE_CASE keyword in cmake_language(PRINT_TARGETS) "
                      "call is only valid with REGEX.");
  }

  // Compile the optional REGEX up front so a bad pattern fails fast.  With
  // IGNORE_CASE the pattern and the candidate names are both lower-cased.
  cm::optional<cmsys::RegularExpression> regex;
  if (parsedArgs.Regex) {
    cmsys::RegularExpression re;
    std::string const pat = parsedArgs.IgnoreCase
      ? cmSystemTools::LowerCase(*parsedArgs.Regex)
      : *parsedArgs.Regex;
    if (!re.compile(pat)) {
      return FatalError(status,
                        cmStrCat("REGEX regular expression \"",
                                 *parsedArgs.Regex, "\" cannot compile."));
    }
    regex = std::move(re);
  }

  bool const includeNormal = !parsedArgs.ImportedOnly;
  bool const includeImported = !parsedArgs.NoImported;

  struct TargetInfo
  {
    cm::TargetType Type;
    bool Imported;
  };
  std::map<std::string, TargetInfo> targets;
  for (auto const& mf : makefile.GetGlobalGenerator()->GetMakefiles()) {
    if (includeNormal) {
      for (auto const& ti : mf->GetTargets()) {
        cmTarget const& t = ti.second;
        targets.insert({ t.GetName(), { t.GetType(), false } });
      }
    }
    if (includeImported) {
      for (cmTarget const* t : mf->GetImportedTargets()) {
        targets.insert({ t->GetName(), { t->GetType(), true } });
      }
    }
  }

  // Build the body first so the header is suppressed when a REGEX filters
  // everything out (matches cmake_language(PRINT_VARIABLES) behavior).
  std::string lines;
  bool anyMatched = false;
  for (auto const& t : targets) {
    if (regex) {
      std::string const subj =
        parsedArgs.IgnoreCase ? cmSystemTools::LowerCase(t.first) : t.first;
      if (!regex->find(subj)) {
        continue;
      }
    }
    lines +=
      cmStrCat("   ", t.first, " (", cmState::GetTargetTypeName(t.second.Type),
               t.second.Imported ? ", IMPORTED" : "", ")\n");
    anyMatched = true;
  }

  if (anyMatched) {
    // The message opens with a banner line.
    std::string out = "Printing targets...\n";
    // The header reflects the imported-filter mode and any REGEX in effect.
    char const* label = "All targets";
    if (parsedArgs.ImportedOnly) {
      label = "Imported targets";
    } else if (parsedArgs.NoImported) {
      label = "Non-imported targets";
    }
    out += cmStrCat(" ", label);
    if (parsedArgs.Regex) {
      out += cmStrCat(
        " matching REGEX '", *parsedArgs.Regex, "' (",
        parsedArgs.IgnoreCase ? "case insensitive" : "case sensitive", ")");
    }
    out += cmStrCat(":\n", lines);
    makefile.DisplayStatus(out, -1);
  }

  if (!anyMatched && parsedArgs.Regex) {
    makefile.IssueMessage(
      MessageType::WARNING,
      cmStrCat("No targets matching REGEX '", *parsedArgs.Regex, "' (",
               parsedArgs.IgnoreCase ? "case insensitive" : "case sensitive",
               ") in cmake_language(PRINT_TARGETS ...)."));
  }
  return true;
}

struct PrintVariablesArgs : public ArgumentParser::ParseResult
{
  bool All = false;
  ArgumentParser::NonEmpty<std::vector<std::string>> Named;
  cm::optional<std::string> NameRegex;
  cm::optional<std::string> ValueRegex;
  bool IgnoreCase = false;
  // Internal: set by the deprecated cmake_print_variables() module wrapper to
  // request the historical flush single-line output (no banner, undefined
  // names printed as name="").  Not part of the public interface.
  bool CmakePrintVariables = false;
};

// Banner opening a cmake_language(PRINT_VARIABLES) status message.  Suppressed
// in the deprecated cmake_print_variables() (legacy) path.
cm::static_string_view const PrintVariablesBanner =
  "Printing variables...\n"_s;

// NAMED mode.  By default (matching cmake_language(PRINT_PROPERTIES)) this
// emits a " Named variables:" header followed by one `name = "value"` entry
// per line; a name that is not defined prints as `name = <NOTFOUND>` in the
// order given.  When `legacy` is set (the deprecated cmake_print_variables()
// wrapper) it instead emits the historical flush single line
// "name=\"value\" ; ..." with no header and undefined names as name="".
// Values go through GetDefinition so users see the same
// in-scope/cache-fallback value they'd get from ${var}.
void PrintVariablesNamed(cmMakefile& makefile,
                         std::vector<std::string> const& names, bool legacy)
{
  if (legacy) {
    std::string msg;
    bool first = true;
    for (std::string const& name : names) {
      if (!first) {
        msg += " ; ";
      }
      first = false;
      cmValue v = makefile.GetDefinition(name);
      msg += cmStrCat(name, "=\"", v ? *v : std::string(), "\"");
    }
    makefile.DisplayStatus(msg, -1);
    return;
  }

  std::string out = cmStrCat(PrintVariablesBanner, " Named variables:\n");
  for (std::string const& name : names) {
    cmValue v = makefile.GetDefinition(name);
    if (v) {
      out += cmStrCat("   ", name, " = \"", *v, "\"\n");
    } else {
      out += cmStrCat("   ", name, " = <NOTFOUND>\n");
    }
  }
  makefile.DisplayStatus(out, -1);
}

// ALL mode: enumerate every regular variable in scope and
// every cache entry.  A name with both a regular variable and a cache
// entry is printed on two distinct lines so the user sees the full
// picture, including the value the cache holds even when a regular
// variable shadows it.  Values are read via the snapshot and cache APIs
// directly to avoid firing variable-watch callbacks (which would
// otherwise trip CMP0218 when CMAKE_WARN_DEPRECATED or
// CMAKE_ERROR_DEPRECATED are in scope).
bool PrintVariablesAll(cmMakefile& makefile, PrintVariablesArgs const& parsed,
                       cmExecutionStatus& status)
{
  cm::optional<cmsys::RegularExpression> nameRegex;
  cm::optional<cmsys::RegularExpression> valueRegex;
  if (parsed.NameRegex) {
    cmsys::RegularExpression re;
    std::string const pat = parsed.IgnoreCase
      ? cmSystemTools::LowerCase(*parsed.NameRegex)
      : *parsed.NameRegex;
    if (!re.compile(pat)) {
      return FatalError(status,
                        cmStrCat("NAME_REGEX regular expression \"",
                                 *parsed.NameRegex, "\" cannot compile."));
    }
    nameRegex = std::move(re);
  }
  if (parsed.ValueRegex) {
    cmsys::RegularExpression re;
    std::string const pat = parsed.IgnoreCase
      ? cmSystemTools::LowerCase(*parsed.ValueRegex)
      : *parsed.ValueRegex;
    if (!re.compile(pat)) {
      return FatalError(status,
                        cmStrCat("VALUE_REGEX regular expression \"",
                                 *parsed.ValueRegex, "\" cannot compile."));
    }
    valueRegex = std::move(re);
  }

  auto matches = [&](std::string const& name, std::string const& value) {
    if (nameRegex) {
      std::string const subj =
        parsed.IgnoreCase ? cmSystemTools::LowerCase(name) : name;
      if (!nameRegex->find(subj)) {
        return false;
      }
    }
    if (valueRegex) {
      std::string const subj =
        parsed.IgnoreCase ? cmSystemTools::LowerCase(value) : value;
      if (!valueRegex->find(subj)) {
        return false;
      }
    }
    return true;
  };

  cmStateSnapshot const snapshot = makefile.GetStateSnapshot();
  cmState* state = makefile.GetState();

  // GetDefinitions() already unions ClosureKeys() with the cache keys and
  // sorts the result; just dedupe so a name that lives in both lists isn't
  // visited twice.
  auto names = makefile.GetDefinitions();
  names.erase(std::unique(names.begin(), names.end()), names.end());

  // Build the body first so nothing is printed when a regex filters
  // everything out; the warning below covers that case.
  std::string body;
  bool anyMatched = false;
  for (std::string const& name : names) {
    cmValue regular = snapshot.GetDefinition(name);
    if (regular && matches(name, *regular)) {
      body += cmStrCat("   ", name, " = \"", *regular, "\"\n");
      anyMatched = true;
    }
    cmValue cached = state->GetInitializedCacheValue(name);
    if (cached && matches(name, *cached)) {
      auto const type = state->GetCacheEntryType(name);
      body += cmStrCat("   CACHE{", name, "}");
      if (type != cmStateEnums::UNINITIALIZED) {
        body += cmStrCat(":", cmState::CacheEntryTypeToString(type));
      }
      body += cmStrCat(" = \"", *cached, "\"\n");
      anyMatched = true;
    }
  }

  if (anyMatched) {
    cmValue listFile = snapshot.GetDefinition("CMAKE_CURRENT_LIST_FILE");
    std::string out =
      cmStrCat(PrintVariablesBanner, " Variables in scope at '",
               listFile ? *listFile : std::string("<unknown>"), "'");
    if (parsed.NameRegex || parsed.ValueRegex) {
      out += " matching";
      if (parsed.NameRegex) {
        out += cmStrCat(" name '", *parsed.NameRegex, "'");
      }
      if (parsed.NameRegex && parsed.ValueRegex) {
        out += " and";
      }
      if (parsed.ValueRegex) {
        out += cmStrCat(" value '", *parsed.ValueRegex, "'");
      }
      out += parsed.IgnoreCase ? " (case insensitive)" : " (case sensitive)";
    }
    out += cmStrCat(":\n", body);
    makefile.DisplayStatus(out, -1);
  }

  if (!anyMatched && (parsed.NameRegex || parsed.ValueRegex)) {
    std::string msg = "No variables in scope matching";
    if (parsed.NameRegex) {
      msg += cmStrCat(" name '", *parsed.NameRegex, "'");
    }
    if (parsed.NameRegex && parsed.ValueRegex) {
      msg += " and";
    }
    if (parsed.ValueRegex) {
      msg += cmStrCat(" value '", *parsed.ValueRegex, "'");
    }
    msg += parsed.IgnoreCase ? " (case insensitive)" : " (case sensitive)";
    msg += " in cmake_language(PRINT_VARIABLES ...).";
    makefile.IssueMessage(MessageType::WARNING, msg);
  }
  return true;
}

bool cmCMakeLanguageCommandPRINT_VARIABLES(
  std::vector<cmListFileArgument> const& args, cmExecutionStatus& status)
{
  cmMakefile& makefile = status.GetMakefile();
  std::vector<std::string> expandedArgs;
  makefile.ExpandArguments(args, expandedArgs);

  // Drop the leading "PRINT_VARIABLES" subcommand keyword.
  std::vector<std::string> body(expandedArgs.begin() + 1, expandedArgs.end());

  auto const ArgsParser =
    cmArgumentParser<PrintVariablesArgs>()
      .Bind("ALL"_s, &PrintVariablesArgs::All)
      .Bind("NAMED"_s, &PrintVariablesArgs::Named)
      .Bind("NAME_REGEX"_s, &PrintVariablesArgs::NameRegex)
      .Bind("VALUE_REGEX"_s, &PrintVariablesArgs::ValueRegex)
      .Bind("IGNORE_CASE"_s, &PrintVariablesArgs::IgnoreCase)
      .Bind("__CMAKE_PRINT_VARIABLES"_s,
            &PrintVariablesArgs::CmakePrintVariables);

  std::vector<std::string> unparsed;
  auto parsedArgs = ArgsParser.Parse(body, &unparsed);

  // No bareword form: every token must belong to ALL, NAMED, or a filter.
  if (!unparsed.empty()) {
    return FatalError(
      status,
      cmStrCat("Unknown argument(s) given to cmake_language(PRINT_VARIABLES)"
               ": \"",
               cmJoin(unparsed, "\" \""), "\"."));
  }

  if (parsedArgs.MaybeReportError(makefile)) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  bool const hasNamed = !parsedArgs.Named.empty();
  bool const hasFilters =
    parsedArgs.NameRegex || parsedArgs.ValueRegex || parsedArgs.IgnoreCase;

  // ALL and NAMED are mutually exclusive modes.
  if (parsedArgs.All && hasNamed) {
    return FatalError(status,
                      "ALL and NAMED keywords in "
                      "cmake_language(PRINT_VARIABLES) call "
                      "are mutually exclusive.");
  }

  // The filter keywords narrow an enumeration, so they only make sense with
  // ALL (explicit or implicit), never with NAMED.
  if (hasNamed && hasFilters) {
    return FatalError(status,
                      "NAME_REGEX, VALUE_REGEX, and IGNORE_CASE in "
                      "cmake_language(PRINT_VARIABLES) call "
                      "are only valid with ALL, not NAMED.");
  }

  // __CMAKE_PRINT_VARIABLES selects the legacy single-line NAMED output; it is
  // meaningless when enumerating variables (ALL, explicit or implicit).
  if (!hasNamed && parsedArgs.CmakePrintVariables) {
    return FatalError(status,
                      "__CMAKE_PRINT_VARIABLES in "
                      "cmake_language(PRINT_VARIABLES) call is only valid "
                      "with NAMED.");
  }

  // Default to ALL when neither mode keyword is given.
  bool const all = parsedArgs.All || !hasNamed;
  if (all) {
    return PrintVariablesAll(makefile, parsedArgs, status);
  }

  PrintVariablesNamed(makefile, parsedArgs.Named,
                      parsedArgs.CmakePrintVariables);
  return true;
}
// Walks every target reachable from `root` through any linkage - PUBLIC,
// INTERFACE, PRIVATE, and `$<LINK_ONLY:>`-wrapped deps.  At each level we
// union the link implementation and link interface, each queried with both
// UseTo::Compile and UseTo::Link, so deps that only appear under LINK_ONLY
// (which evaluates to empty under UseTo::Compile) still come through.
std::vector<cmGeneratorTarget const*> CollectDependentTargets(
  cmGeneratorTarget const* root, std::string const& config)
{
  std::vector<cmGeneratorTarget const*> deps;
  std::set<cmGeneratorTarget const*> visited;
  visited.insert(root);
  std::vector<cmGeneratorTarget const*> queue;
  queue.push_back(root);
  while (!queue.empty()) {
    cmGeneratorTarget const* cur = queue.back();
    queue.pop_back();
    auto visit = [&](std::vector<cmLinkItem> const& libs) {
      for (cmLinkItem const& item : libs) {
        cmGeneratorTarget const* dep = item.Target;
        if (!dep || !visited.insert(dep).second) {
          continue;
        }
        deps.push_back(dep);
        queue.push_back(dep);
      }
    };
    for (auto useTo : { cmGeneratorTarget::UseTo::Compile,
                        cmGeneratorTarget::UseTo::Link }) {
      if (cmLinkImplementationLibraries const* impl =
            cur->GetLinkImplementationLibraries(config, useTo)) {
        visit(impl->Libraries);
      }
      if (cmLinkInterfaceLibraries const* iface =
            cur->GetLinkInterfaceLibraries(config, root, useTo)) {
        visit(iface->Libraries);
      }
    }
  }
  return deps;
}

// Append one property line to `out`:
//   "   <entityName>.<propertyName> = \"<value>\""
// A null `value` writes "<NOTFOUND>" instead of "= \"...\"".
void WritePropertyLine(std::string& out, std::string const& entityName,
                       std::string const& propertyName, cmValue value)
{
  out += cmStrCat("   ", entityName, ".", propertyName);
  if (value) {
    out += cmStrCat(" = \"", *value, "\"");
  } else {
    out += " = <NOTFOUND>";
  }
  out += "\n";
}

enum class BlockKind
{
  Explicit,
  All,
};

enum class HeaderSuffix
{
  None,
  Reachable,
};

void EmitBlockHeader(
  std::string& out, std::string const& entityName,
  std::string const& entityType, BlockKind kind, HeaderSuffix suffix,
  cm::optional<std::string> const& nameRegexStr = cm::nullopt,
  cm::optional<std::string> const& valueRegexStr = cm::nullopt)
{
  out +=
    cmStrCat(" ", (kind == BlockKind::All ? "All properties" : "Properties"),
             " for ", entityType, " ", entityName);
  if (suffix == HeaderSuffix::Reachable) {
    out += " (and all reachable)";
  }
  if (kind == BlockKind::All && (nameRegexStr || valueRegexStr)) {
    out += " matching";
    if (nameRegexStr) {
      out += cmStrCat(" name '", *nameRegexStr, "'");
    }
    if (nameRegexStr && valueRegexStr) {
      out += " and";
    }
    if (valueRegexStr) {
      out += cmStrCat(" value '", *valueRegexStr, "'");
    }
  }
  out += ":\n";
}

// Build the "no properties matched" warning text used when the regex filter
// wipes everything out for a target's ALL block.  Returned without any
// "Warning:" prefix - callers feed it to IssueMessage, which adds the
// standard "CMake Warning at <file>:<line>" framing.
std::string EmptyMatchWarningMessage(
  std::string const& entityName, std::string const& entityType,
  cm::optional<std::string> const& nameRegexStr,
  cm::optional<std::string> const& valueRegexStr)
{
  std::string msg =
    cmStrCat("No properties for ", entityType, " ", entityName, " matching");
  if (nameRegexStr) {
    msg += cmStrCat(" name '", *nameRegexStr, "'");
  }
  if (nameRegexStr && valueRegexStr) {
    msg += " and";
  }
  if (valueRegexStr) {
    msg += cmStrCat(" value '", *valueRegexStr, "'");
  }
  msg += " in cmake_language(PRINT_PROPERTIES ...).";
  return msg;
}

// Entity kinds supported by cmake_language(PRINT_PROPERTIES).
enum class EntityKind
{
  Target,
  Source,
  Test,
  Directory,
  Cache
};

// Display name used in PRINT_PROPERTIES output ("TARGET", "SOURCE", etc.).
char const* EntityTypeName(EntityKind kind)
{
  switch (kind) {
    case EntityKind::Target:
      return "TARGET";
    case EntityKind::Source:
      return "SOURCE";
    case EntityKind::Test:
      return "TEST";
    case EntityKind::Directory:
      return "DIRECTORY";
    case EntityKind::Cache:
      return "CACHE";
  }
  return "";
}

// Dispatch a property lookup to the appropriate per-entity helper.
bool GetPropertyHelper(cmExecutionStatus& status, EntityKind kind,
                       std::string const& name,
                       std::string const& propertyName, cmValue& out)
{
  switch (kind) {
    case EntityKind::Target:
      return GetPropertyCommand::LookupTargetProperty(status, name,
                                                      propertyName, out);
    case EntityKind::Source:
      return GetPropertyCommand::LookupSourceProperty(status, name,
                                                      propertyName, out);
    case EntityKind::Test:
      return GetPropertyCommand::LookupTestProperty(status, name, propertyName,
                                                    out);
    case EntityKind::Directory:
      return GetPropertyCommand::LookupDirectoryProperty(status, name,
                                                         propertyName, out);
    case EntityKind::Cache:
      return GetPropertyCommand::LookupCacheProperty(status, name,
                                                     propertyName, out);
  }
  return false;
}

// Emit one ALL-mode block: prints all properties matching the specified
// regexes for a head target (`targets[0]`) and optionally any reachable
// dependencies (`targets[1]` and on).
bool EmitAllPropertiesBlock(std::string& out, std::string const& headerName,
                            std::vector<cmTarget*> const& targets,
                            HeaderSuffix suffix,
                            cm::optional<cmsys::RegularExpression>& nameRegex,
                            cm::optional<cmsys::RegularExpression>& valueRegex,
                            cm::optional<std::string> const& nameRegexStr,
                            cm::optional<std::string> const& valueRegexStr)
{
  std::string groupBuf;
  bool groupEmitted = false;
  for (cmTarget* t : targets) {
    std::string const& rowName =
      (t == targets.front()) ? headerName : t->GetName();
    for (auto const& kv : t->GetExtendedProperties().GetList()) {
      if (nameRegex && !nameRegex->find(kv.first)) {
        continue;
      }
      if (valueRegex && !valueRegex->find(kv.second)) {
        continue;
      }
      WritePropertyLine(groupBuf, rowName, kv.first, cmValue(kv.second));
      groupEmitted = true;
    }
  }
  if (!groupEmitted) {
    return false;
  }
  EmitBlockHeader(out, headerName, "TARGET", BlockKind::All, suffix,
                  nameRegexStr, valueRegexStr);
  out += groupBuf;
  return true;
}

// Outcome of emitting a NAMED-mode block.
enum class NamedBlockResult
{
  Failed,  // a property lookup hard-failed
  Emitted, // at least one property line was written
  Empty,   // every requested property was skipped; header suppressed
};

// Emit one block of user-specified properties for a head target (`targets[0]`)
// and optionally any reachable dependencies (`targets[1]` and on).
NamedBlockResult EmitNamedPropertiesBlock(
  std::string& out, std::string const& headerName,
  std::vector<cmTarget*> const& targets,
  std::vector<std::string> const& namedProperties, HeaderSuffix suffix,
  cmMakefile& mf)
{
  bool headerWritten = false;
  for (cmTarget* t : targets) {
    std::string const& rowName =
      (t == targets.front()) ? headerName : t->GetName();
    for (std::string const& propName : namedProperties) {
      // A computed location property may not be read from a non-imported
      // target; skip it with a warning rather than triggering the getter's
      // fatal error.
      if (!t->IsImported() &&
          cmTargetPropertyComputer::IsComputedLocationProperty(t->GetType(),
                                                               propName)) {
        mf.IssueMessage(
          MessageType::WARNING,
          cmStrCat("The ", propName,
                   " property may not be read from non-imported target \"",
                   rowName, "\"; skipping."));
        continue;
      }
      cmValue v;
      if (cmGetTargetProperty(rowName, t, propName, mf, v) !=
          cmGetTargetPropertyResult::Success) {
        return NamedBlockResult::Failed;
      }
      if (!headerWritten) {
        EmitBlockHeader(out, headerName, "TARGET", BlockKind::Explicit,
                        suffix);
        headerWritten = true;
      }
      WritePropertyLine(out, rowName, propName, v);
    }
  }
  return headerWritten ? NamedBlockResult::Emitted : NamedBlockResult::Empty;
}

bool PrintPropertiesConfigureTime(
  cmExecutionStatus& status, std::vector<std::string> const& namedProperties,
  bool all, std::vector<std::string> const& entityNames, EntityKind kind,
  cm::optional<std::string> const& propertyNameRegexStr,
  cm::optional<std::string> const& propertyValueRegexStr,
  cm::optional<cmsys::RegularExpression>& propertyNameRegex,
  cm::optional<cmsys::RegularExpression>& propertyValueRegex,
  std::string const& messagePrefix)
{
  cmMakefile& makefile = status.GetMakefile();
  bool const hasRegex = propertyNameRegexStr || propertyValueRegexStr;
  std::string out = messagePrefix;
  bool anyEmitted = false;
  for (auto const& entityName : entityNames) {
    if (kind == EntityKind::Target) {
      cmTarget* target = makefile.FindTargetToUse(entityName);
      if (!target) {
        out += cmStrCat("\n No such TARGET \"", entityName, "\" !\n\n");
        anyEmitted = true;
        continue;
      }
      if (all) {
        bool const emitted = EmitAllPropertiesBlock(
          out, entityName, { target }, HeaderSuffix::None, propertyNameRegex,
          propertyValueRegex, propertyNameRegexStr, propertyValueRegexStr);
        if (emitted) {
          anyEmitted = true;
        } else if (hasRegex) {
          makefile.IssueMessage(MessageType::WARNING,
                                EmptyMatchWarningMessage(
                                  entityName, "TARGET", propertyNameRegexStr,
                                  propertyValueRegexStr));
        }
      } else {
        NamedBlockResult const result = EmitNamedPropertiesBlock(
          out, entityName, { target }, namedProperties, HeaderSuffix::None,
          makefile);
        if (result == NamedBlockResult::Failed) {
          status.SetError(cmStrCat(
            "failed to retrieve properties for TARGET \"", entityName, "\""));
          return false;
        }
        if (result == NamedBlockResult::Emitted) {
          anyEmitted = true;
        }
      }
    } else {
      EmitBlockHeader(out, entityName, EntityTypeName(kind),
                      BlockKind::Explicit, HeaderSuffix::None);
      for (auto const& propertyName : namedProperties) {
        cmValue v;
        if (!GetPropertyHelper(status, kind, entityName, propertyName, v)) {
          return false;
        }
        WritePropertyLine(out, entityName, propertyName, v);
      }
      anyEmitted = true;
    }
  }
  if (anyEmitted) {
    makefile.DisplayStatus(out, -1);
  }
  return true;
}

bool PrintTargetPropertiesDeferred(
  std::vector<std::string> targetNames,
  std::vector<std::string> namedProperties, bool all,
  cm::optional<std::string> propertyNameRegexStr,
  cm::optional<std::string> propertyValueRegexStr,
  cm::optional<cmsys::RegularExpression> propertyNameRegex,
  cm::optional<cmsys::RegularExpression> propertyValueRegex,
  bool followDependencies, std::string messagePrefix,
  cmExecutionStatus& status)
{
  cmListFileBacktrace const bt = status.GetMakefile().GetBacktrace();
  status.GetMakefile().AddGeneratorAction(
    [targetNames, namedProperties, all, propertyNameRegexStr,
     propertyValueRegexStr, propertyNameRegex, propertyValueRegex,
     followDependencies, messagePrefix, bt](
      cmLocalGenerator& lg, cmListFileBacktrace const& /*lambdaBt*/) mutable {
      std::vector<std::string> const configs =
        lg.GetMakefile()->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
      std::string const config =
        configs.empty() ? std::string() : configs.front();
      bool const hasRegex = propertyNameRegexStr || propertyValueRegexStr;
      cmake* cmakeInst = lg.GetMakefile()->GetCMakeInstance();

      std::string out = messagePrefix;
      bool anyEmitted = false;
      for (std::string const& name : targetNames) {
        cmGeneratorTarget* root = lg.FindGeneratorTargetToUse(name);
        if (!root) {
          out += cmStrCat("\n No such TARGET \"", name, "\" !\n\n");
          anyEmitted = true;
          continue;
        }
        std::vector<cmTarget*> targets;
        targets.push_back(root->Target);
        HeaderSuffix const suffix =
          followDependencies ? HeaderSuffix::Reachable : HeaderSuffix::None;
        if (followDependencies) {
          for (cmGeneratorTarget const* dep :
               CollectDependentTargets(root, config)) {
            targets.push_back(dep->Target);
          }
        }
        if (all) {
          bool const emitted = EmitAllPropertiesBlock(
            out, name, targets, suffix, propertyNameRegex, propertyValueRegex,
            propertyNameRegexStr, propertyValueRegexStr);
          if (emitted) {
            anyEmitted = true;
          } else if (hasRegex) {
            cmakeInst->IssueMessage(
              MessageType::WARNING,
              EmptyMatchWarningMessage(name, "TARGET", propertyNameRegexStr,
                                       propertyValueRegexStr),
              bt);
          }
        } else {
          NamedBlockResult const result = EmitNamedPropertiesBlock(
            out, name, targets, namedProperties, suffix, *lg.GetMakefile());
          if (result == NamedBlockResult::Failed) {
            cmakeInst->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("failed to retrieve properties for TARGET \"", name,
                       "\""),
              bt);
            return;
          }
          if (result == NamedBlockResult::Emitted) {
            anyEmitted = true;
          }
        }
      }
      if (anyEmitted) {
        lg.GetMakefile()->DisplayStatus(out, -1);
      }
    },
    cmMakefile::GeneratorActionWhen::AfterGeneratorTargets);
  return true;
}

bool cmCMakeLanguageCommandPRINT_PROPERTIES(
  std::vector<std::string> const& args, cmExecutionStatus& status)
{
  struct PrintPropertiesArg : public ArgumentParser::ParseResult
  {
    bool All = false;
    ArgumentParser::NonEmpty<std::vector<std::string>> Named;
    cm::optional<std::string> PropertyNameRegex;
    cm::optional<std::string> PropertyValueRegex;
    // Internal: set by the deprecated cmake_print_properties() module wrapper
    // to select the historical leading-blank-line output instead of the
    // banner.  Not part of the public interface.
    bool CmakePrintProperties = false;
  };

  auto ArgsParser =
    cmArgumentParser<PrintPropertiesArg>()
      .Bind("ALL"_s, &PrintPropertiesArg::All)
      .Bind("NAMED"_s, &PrintPropertiesArg::Named)
      .Bind("PROPERTY_NAME_REGEX"_s, &PrintPropertiesArg::PropertyNameRegex)
      .Bind("PROPERTY_VALUE_REGEX"_s, &PrintPropertiesArg::PropertyValueRegex)
      .Bind("__CMAKE_PRINT_PROPERTIES"_s,
            &PrintPropertiesArg::CmakePrintProperties);

  std::vector<std::string> unparsed;
  auto parsedArgs = ArgsParser.Parse(args, &unparsed);

  if (unparsed.empty()) {
    return FatalError(status,
                      cmStrCat("mode keyword missing in ",
                               "cmake_language(PRINT_PROPERTIES) call, ",
                               "there must be exactly one of TARGETS SOURCES "
                               "TESTS DIRECTORIES CACHE_ENTRIES"));
  }

  if (parsedArgs.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  // Second parse args to get the mode
  struct PrintPropertiesModesArgs : public ArgumentParser::ParseResult
  {
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Targets;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Sources;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Tests;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> Directories;
    ArgumentParser::MaybeEmpty<std::vector<std::string>> CacheEntries;
    bool Deferred = false;
    bool FollowDependencies = false;
  };
  auto const ArgsParserMode =
    cmArgumentParser<PrintPropertiesModesArgs>()
      .Bind("TARGETS"_s, &PrintPropertiesModesArgs::Targets)
      .Bind("SOURCES"_s, &PrintPropertiesModesArgs::Sources)
      .Bind("TESTS"_s, &PrintPropertiesModesArgs::Tests)
      .Bind("DIRECTORIES"_s, &PrintPropertiesModesArgs::Directories)
      .Bind("CACHE_ENTRIES"_s, &PrintPropertiesModesArgs::CacheEntries)
      .Bind("DEFERRED"_s, &PrintPropertiesModesArgs::Deferred)
      .Bind("FOLLOW_DEPENDENCIES"_s,
            &PrintPropertiesModesArgs::FollowDependencies);

  std::vector<std::string> modeArgs = unparsed;
  unparsed.clear();
  auto parsedArgsMode = ArgsParserMode.Parse(modeArgs, &unparsed);

  if (!unparsed.empty()) {
    return FatalError(
      status, cmStrCat("Unknown keywords: \"", cmJoin(unparsed, " "), "\""));
  }

  if (parsedArgsMode.MaybeReportError(status.GetMakefile())) {
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }
  std::vector<std::string> modes;
  std::vector<std::string> items;
  EntityKind kind = EntityKind::Target;
  if (!parsedArgsMode.Targets.empty()) {
    modes.push_back("TARGETS");
    items = parsedArgsMode.Targets;
    kind = EntityKind::Target;
  }
  if (!parsedArgsMode.Sources.empty()) {
    modes.push_back("SOURCES");
    items = parsedArgsMode.Sources;
    kind = EntityKind::Source;
  }
  if (!parsedArgsMode.Tests.empty()) {
    modes.push_back("TESTS");
    items = parsedArgsMode.Tests;
    kind = EntityKind::Test;
  }
  if (!parsedArgsMode.Directories.empty()) {
    modes.push_back("DIRECTORIES");
    items = parsedArgsMode.Directories;
    kind = EntityKind::Directory;
  }
  if (!parsedArgsMode.CacheEntries.empty()) {
    modes.push_back("CACHE_ENTRIES");
    items = parsedArgsMode.CacheEntries;
    kind = EntityKind::Cache;
  }

  if (modes.empty()) {
    return FatalError(status,
                      cmStrCat("mode keyword missing in ",
                               "cmake_language(PRINT_PROPERTIES) call, ",
                               "there must be exactly one of TARGETS SOURCES "
                               "TESTS DIRECTORIES CACHE_ENTRIES"));
  }
  if (modes.size() > 1) {
    return FatalError(status,
                      cmStrCat("multiple mode keywords used in ",
                               "cmake_language(PRINT_PROPERTIES) call, ",
                               "there must be exactly one of TARGETS SOURCES "
                               "TESTS DIRECTORIES CACHE_ENTRIES."));
  }
  std::string const mode = modes[0];
  bool const isTargets = (mode == "TARGETS");
  bool const hasNamed = !parsedArgs.Named.empty();
  bool const hasRegex =
    parsedArgs.PropertyNameRegex || parsedArgs.PropertyValueRegex;

  if (!isTargets) {
    if (parsedArgs.All) {
      return FatalError(status,
                        cmStrCat("ALL keyword in ",
                                 "cmake_language(PRINT_PROPERTIES) call ",
                                 "is only valid with the TARGETS scope."));
    }
    if (hasRegex) {
      return FatalError(
        status,
        cmStrCat("PROPERTY_NAME_REGEX and PROPERTY_VALUE_REGEX in ",
                 "cmake_language(PRINT_PROPERTIES) call ",
                 "are only valid with the TARGETS scope and ALL."));
    }
    if (parsedArgsMode.Deferred) {
      return FatalError(status,
                        cmStrCat("DEFERRED keyword in ",
                                 "cmake_language(PRINT_PROPERTIES) call ",
                                 "is only valid with the TARGETS scope."));
    }
    if (parsedArgsMode.FollowDependencies) {
      return FatalError(status,
                        cmStrCat("FOLLOW_DEPENDENCIES keyword in ",
                                 "cmake_language(PRINT_PROPERTIES) call ",
                                 "is only valid with the TARGETS scope."));
    }
    if (!hasNamed) {
      return FatalError(status,
                        cmStrCat("NAMED keyword missing in ",
                                 "cmake_language(PRINT_PROPERTIES) call ",
                                 "with ", mode, " scope."));
    }
  } else {
    // ALL and NAMED are mutually exclusive on TARGETS.
    if (parsedArgs.All && hasNamed) {
      return FatalError(status,
                        cmStrCat("ALL and NAMED keywords in ",
                                 "cmake_language(PRINT_PROPERTIES) call ",
                                 "are mutually exclusive."));
    }
    // Regex filters require ALL - explicit or implicit.  Combining regex
    // with NAMED is an error.
    if (hasNamed && hasRegex) {
      return FatalError(
        status,
        cmStrCat("PROPERTY_NAME_REGEX and PROPERTY_VALUE_REGEX in ",
                 "cmake_language(PRINT_PROPERTIES) call ",
                 "are only valid with ALL, not NAMED."));
    }
  }

  // FOLLOW_DEPENDENCIES implies DEFERRED.
  if (parsedArgsMode.FollowDependencies) {
    parsedArgsMode.Deferred = true;
  }

  // Default to ALL when neither ALL nor NAMED is given (TARGETS only - the
  // non-TARGETS path already rejected this combination above).
  bool const all = parsedArgs.All || !hasNamed;

  // __CMAKE_PRINT_PROPERTIES marks the deprecated cmake_print_properties()
  // wrapper, which is always NAMED; reject it in ALL enumeration mode.
  if (all && parsedArgs.CmakePrintProperties) {
    return FatalError(status,
                      cmStrCat("__CMAKE_PRINT_PROPERTIES in ",
                               "cmake_language(PRINT_PROPERTIES) call ",
                               "is only valid with NAMED."));
  }

  // Compile regexes once up front so syntax errors are reported here rather
  // than from inside helpers (in particular, from the deferred lambda at
  // generate time).
  cm::optional<cmsys::RegularExpression> propertyNameRegex;
  cm::optional<cmsys::RegularExpression> propertyValueRegex;
  if (parsedArgs.PropertyNameRegex) {
    cmsys::RegularExpression re;
    if (!re.compile(*parsedArgs.PropertyNameRegex)) {
      return FatalError(status,
                        cmStrCat("PROPERTY_NAME_REGEX regular expression \"",
                                 *parsedArgs.PropertyNameRegex,
                                 "\" cannot compile."));
    }
    propertyNameRegex = std::move(re);
  }
  if (parsedArgs.PropertyValueRegex) {
    cmsys::RegularExpression re;
    if (!re.compile(*parsedArgs.PropertyValueRegex)) {
      return FatalError(status,
                        cmStrCat("PROPERTY_VALUE_REGEX regular expression \"",
                                 *parsedArgs.PropertyValueRegex,
                                 "\" cannot compile."));
    }
    propertyValueRegex = std::move(re);
  }

  // The message opens with a leading banner line.  The internal
  // __CMAKE_PRINT_PROPERTIES marker (not part of the public interface) selects
  // the legacy format instead; cmake_print_properties() sets it to reproduce
  // its historical leading blank line.
  std::string const messagePrefix = parsedArgs.CmakePrintProperties
    ? std::string("\n")
    : std::string("Printing properties...\n");

  if (parsedArgsMode.Deferred) {
    return PrintTargetPropertiesDeferred(
      items, parsedArgs.Named, all, parsedArgs.PropertyNameRegex,
      parsedArgs.PropertyValueRegex, std::move(propertyNameRegex),
      std::move(propertyValueRegex), parsedArgsMode.FollowDependencies,
      messagePrefix, status);
  }

  return PrintPropertiesConfigureTime(
    status, parsedArgs.Named, all, items, kind, parsedArgs.PropertyNameRegex,
    parsedArgs.PropertyValueRegex, propertyNameRegex, propertyValueRegex,
    messagePrefix);
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
  if (expArgs[expArg] == "EXIT"_s) {
    ++expArg; // consume "EXIT".

    if (!moreArgs()) {
      return FatalError(status, "EXIT requires one argument");
    }

    if (!status.GetMakefile().GetCMakeInstance()->RoleSupportsExitCode()) {
      return FatalError(status, "EXIT can be used only in SCRIPT mode");
    }

    long retCode = 0;

    if (!cmStrToLong(expArgs[expArg], &retCode)) {
      return FatalError(status,
                        cmStrCat("EXIT requires one integral argument, got \"",
                                 expArgs[expArg], '\"'));
    }

    status.SetExitCode(static_cast<int>(retCode));
    return true;
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
                                     "\nis not known.  "
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

  if (expArgs[expArg] == "GET_EXPERIMENTAL_FEATURE_ENABLED") {
    return cmCMakeLanguageCommandGET_EXPERIMENTAL_FEATURE_ENABLED(args,
                                                                  status);
  }

  if (expArgs[expArg] == "PRINT_TARGETS") {
    return cmCMakeLanguageCommandPRINT_TARGETS(args, status);
  }

  if (expArgs[expArg] == "PRINT_VARIABLES") {
    return cmCMakeLanguageCommandPRINT_VARIABLES(args, status);
  }

  if (expArgs[expArg] == "TRACE") {
    ++expArg; // Consume "TRACE".

    if (!moreArgs()) {
      return FatalError(status, "TRACE missing a boolean value");
    }

    bool const value = cmValue::IsOn(expArgs[expArg++]);
    bool expand = false;

    if (value && moreArgs()) {
      expand = (expArgs[expArg] == "EXPAND");
      if (!expand) {
        return FatalError(
          status,
          cmStrCat("TRACE ON given an invalid argument ", expArgs[expArg]));
      }
      ++expArg;
    }

    if (moreArgs()) {
      return FatalError(
        status,
        cmStrCat("TRACE O", value ? "N" : "FF", " given too many arguments"));
    }

    cmMakefile& makefile = status.GetMakefile();
    if (value) {
      makefile.GetCMakeInstance()->PushTraceCmd(expand);
      return true;
    }
    return makefile.GetCMakeInstance()->PopTraceCmd() ||
      FatalError(status, "TRACE OFF request without a corresponding TRACE ON");
  }

  if (expArgs[expArg] == "PRINT_PROPERTIES") {
    ++expArg;
    finishArgs();
    std::vector<std::string> const printPropertyArgs(expArgs.begin() + expArg,
                                                     expArgs.end());
    return cmCMakeLanguageCommandPRINT_PROPERTIES(printPropertyArgs, status);
  }

  return FatalError(status, "called with unknown meta-operation");
}
