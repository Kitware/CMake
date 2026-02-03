/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstrumentationCommand.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <set>
#include <sstream>

#include <cmext/string_view>

#include <cm3p/json/reader.h>
#include <cm3p/json/value.h>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmInstrumentation.h"
#include "cmInstrumentationQuery.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmValue.h"
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
    status.SetError(cmStrCat("given a non-integer ", key, '.'));
    return false;
  }
  version = std::atoi(versionString.c_str());
  if (version != 1) {
    status.SetError(
      cmStrCat("given an unsupported ", key, " \"", versionString,
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
  if (args.empty()) {
    status.SetError("must be called with arguments.");
    return false;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    ArgumentParser::NonEmpty<std::string> ApiVersion;
    ArgumentParser::NonEmpty<std::string> DataVersion;
    ArgumentParser::NonEmpty<std::vector<std::string>> Options;
    ArgumentParser::NonEmpty<std::vector<std::string>> Hooks;
    ArgumentParser::NonEmpty<std::vector<std::vector<std::string>>> Callbacks;
    ArgumentParser::NonEmpty<std::vector<std::vector<std::string>>>
      CustomContent;
  };

  static auto const parser =
    cmArgumentParser<Arguments>{}
      .Bind("API_VERSION"_s, &Arguments::ApiVersion)
      .Bind("DATA_VERSION"_s, &Arguments::DataVersion)
      .Bind("OPTIONS"_s, &Arguments::Options)
      .Bind("HOOKS"_s, &Arguments::Hooks)
      .Bind("CALLBACK"_s, &Arguments::Callbacks)
      .Bind("CUSTOM_CONTENT"_s, &Arguments::CustomContent);

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

  std::set<cmInstrumentationQuery::Option> options;
  auto optionParser = EnumParser<cmInstrumentationQuery::Option>(
    cmInstrumentationQuery::OptionString);
  for (auto const& arg : arguments.Options) {
    cmInstrumentationQuery::Option option;
    if (!optionParser(arg, option)) {
      status.SetError(
        cmStrCat("given invalid argument to OPTIONS \"", arg, '"'));
      return false;
    }
    options.insert(option);
  }

  std::set<cmInstrumentationQuery::Hook> hooks;
  auto hookParser = EnumParser<cmInstrumentationQuery::Hook>(
    cmInstrumentationQuery::HookString);
  for (auto const& arg : arguments.Hooks) {
    cmInstrumentationQuery::Hook hook;
    if (!hookParser(arg, hook)) {
      status.SetError(
        cmStrCat("given invalid argument to HOOKS \"", arg, '"'));
      return false;
    }
    hooks.insert(hook);
  }

  // Generate custom content
  cmInstrumentation* instrumentation =
    status.GetMakefile().GetCMakeInstance()->GetInstrumentation();
  for (auto const& content : arguments.CustomContent) {
    if (content.size() != 3) {
      status.SetError("CUSTOM_CONTENT expected 3 arguments");
      return false;
    }
    std::string const label = content[0];
    std::string const type = content[1];
    std::string const contentString = content[2];
    Json::Value value;
    if (type == "STRING") {
      value = contentString;
    } else if (type == "BOOL") {
      value = !cmValue(contentString).IsOff();
    } else if (type == "LIST") {
      value = Json::arrayValue;
      for (auto const& item : cmList(contentString)) {
        value.append(item);
      }
    } else if (type == "JSON") {
      Json::CharReaderBuilder builder;
      std::istringstream iss(contentString);
      if (!Json::parseFromStream(builder, iss, &value, nullptr)) {
        status.SetError(
          cmStrCat("failed to parse custom content as JSON: ", contentString));
        return false;
      }
    } else {
      status.SetError(
        cmStrCat("got an invalid type for CUSTOM_CONTENT: ", type));
      return false;
    }
    instrumentation->AddCustomContent(content.front(), value);
  }

  // Write query file
  instrumentation->WriteJSONQuery(options, hooks, arguments.Callbacks);

  return true;
}
