/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstrumentationCommand.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <set>

#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmExperimental.h"
#include "cmInstrumentation.h"
#include "cmInstrumentationQuery.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

namespace {

bool isCharDigit(char ch)
{
  return std::isdigit(static_cast<unsigned char>(ch));
}
bool validateVersion(std::string const& key, std::string const& versionString,
                     int& version, cmExecutionStatus& status)
{
  if (!std::all_of(versionString.begin(), versionString.end(), isCharDigit)) {
    status.SetError(cmStrCat("given a non-integer ", key, "."));
    return false;
  }
  version = std::atoi(versionString.c_str());
  if (version != 1) {
    status.SetError(cmStrCat(
      "QUERY subcommand given an unsupported ", key, " \"", versionString,
      "\" (the only currently supported version is 1)."));
    return false;
  }
  return true;
}

template <typename E>
std::function<bool(std::string const&, E&)> EnumParser(
  std::vector<std::string> const toString)
{
  return [toString](std::string const& value, E& out) -> bool {
    for (size_t i = 0; i < toString.size(); ++i) {
      if (value == toString[i]) {
        out = (E)i;
        return true;
      }
    }
    return false;
  };
}
}

bool cmInstrumentationCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  // if (status->GetMakefile().GetPropertyKeys) {
  if (!cmExperimental::HasSupportEnabled(
        status.GetMakefile(), cmExperimental::Feature::Instrumentation)) {
    status.SetError(
      "requires the experimental Instrumentation flag to be enabled");
    return false;
  }

  if (args.empty()) {
    status.SetError("must be called with arguments.");
    return false;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    ArgumentParser::NonEmpty<std::string> ApiVersion;
    ArgumentParser::NonEmpty<std::string> DataVersion;
    ArgumentParser::NonEmpty<std::vector<std::string>> Queries;
    ArgumentParser::NonEmpty<std::vector<std::string>> Hooks;
    ArgumentParser::NonEmpty<std::vector<std::vector<std::string>>> Callbacks;
  };

  static auto const parser = cmArgumentParser<Arguments>{}
                               .Bind("API_VERSION"_s, &Arguments::ApiVersion)
                               .Bind("DATA_VERSION"_s, &Arguments::DataVersion)
                               .Bind("QUERIES"_s, &Arguments::Queries)
                               .Bind("HOOKS"_s, &Arguments::Hooks)
                               .Bind("CALLBACK"_s, &Arguments::Callbacks);

  std::vector<std::string> unparsedArguments;
  Arguments const arguments = parser.Parse(args, &unparsedArguments);

  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }
  if (!unparsedArguments.empty()) {
    status.SetError("given unknown argument \"" + unparsedArguments.front() +
                    "\".");
    return false;
  }
  int apiVersion;
  int dataVersion;
  if (!validateVersion("API_VERSION", arguments.ApiVersion, apiVersion,
                       status) ||
      !validateVersion("DATA_VERSION", arguments.DataVersion, dataVersion,
                       status)) {
    return false;
  }

  std::set<cmInstrumentationQuery::Query> queries;
  auto queryParser = EnumParser<cmInstrumentationQuery::Query>(
    cmInstrumentationQuery::QueryString);
  for (auto const& arg : arguments.Queries) {
    cmInstrumentationQuery::Query query;
    if (!queryParser(arg, query)) {
      status.SetError(
        cmStrCat("given invalid argument to QUERIES \"", arg, "\""));
      return false;
    }
    queries.insert(query);
  }

  std::set<cmInstrumentationQuery::Hook> hooks;
  auto hookParser = EnumParser<cmInstrumentationQuery::Hook>(
    cmInstrumentationQuery::HookString);
  for (auto const& arg : arguments.Hooks) {
    cmInstrumentationQuery::Hook hook;
    if (!hookParser(arg, hook)) {
      status.SetError(
        cmStrCat("given invalid argument to HOOKS \"", arg, "\""));
      return false;
    }
    hooks.insert(hook);
  }

  status.GetMakefile()
    .GetCMakeInstance()
    ->GetInstrumentation()
    ->WriteJSONQuery(queries, hooks, arguments.Callbacks);

  return true;
}
