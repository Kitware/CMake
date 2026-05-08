#include "cmInstrumentationQuery.h"

#include <algorithm>
#include <ctime>
#include <functional>
#include <iterator>
#include <set>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmStringAlgorithms.h"

std::vector<std::string> const cmInstrumentationQuery::OptionString{
  "staticSystemInformation",
  "dynamicSystemInformation",
  "captureOutput",
  "cdashSubmit",
  "cdashVerbose",
  "trace"
};
std::vector<std::string> const cmInstrumentationQuery::HookString{
  "postGenerate",    "preBuild",  "postBuild",        "preCMakeBuild",
  "postCMakeBuild",  "postCTest", "postCMakeInstall", "postCMakeWorkflow",
  "prepareForCDash", "manual"
};

namespace ErrorMessages {
using ErrorGenerator =
  std::function<void(Json::Value const*, cmJSONState* state)>;
ErrorGenerator ErrorGeneratorBuilder(std::string const& errorMessage)
{
  return [errorMessage](Json::Value const* value, cmJSONState* state) -> void {
    state->AddErrorAtValue(errorMessage, value);
  };
};

static ErrorGenerator InvalidArray = ErrorGeneratorBuilder("Invalid Array");
JsonErrors::ErrorGenerator InvalidRootQueryObject(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState*) -> std::string {
      return "root object";
    })(errorType, extraFields);
}
};

using JSONHelperBuilder = cmJSONHelperBuilder;
using Version = cmInstrumentationQuery::Version;

template <typename E>
static std::function<bool(E&, Json::Value const*, cmJSONState*)> EnumHelper(
  std::vector<std::string> const toString, std::string const& type)
{
  return [toString, type](E& out, Json::Value const* value,
                          cmJSONState* state) -> bool {
    for (size_t i = 0; i < toString.size(); ++i) {
      if (value->asString() == toString[i]) {
        out = (E)i;
        return true;
      }
    }
    state->AddErrorAtValue(
      cmStrCat("Not a valid ", type, ": \"", value->asString(), '"'), value);
    return false;
  };
}
static auto const OptionHelper = EnumHelper<cmInstrumentationQuery::Option>(
  cmInstrumentationQuery::OptionString, "option");
static auto const OptionListHelper =
  JSONHelperBuilder::Vector<cmInstrumentationQuery::Option>(
    ErrorMessages::InvalidArray, OptionHelper);
static auto const HookHelper = EnumHelper<cmInstrumentationQuery::Hook>(
  cmInstrumentationQuery::HookString, "hook");
static auto const HookListHelper =
  JSONHelperBuilder::Vector<cmInstrumentationQuery::Hook>(
    ErrorMessages::InvalidArray, HookHelper);
static auto const CallbackHelper = JSONHelperBuilder::String();
static auto const CallbackListHelper = JSONHelperBuilder::Vector<std::string>(
  ErrorMessages::InvalidArray, CallbackHelper);

JsonErrors::ErrorGenerator InvalidVersionObject(
  JsonErrors::ObjectError errorType, Json::Value::Members const& extraFields)
{
  return JsonErrors::INVALID_NAMED_OBJECT(
    [](Json::Value const*, cmJSONState*) -> std::string {
      return "version object";
    })(errorType, extraFields);
}

static auto const VersionObjectHelper =
  JSONHelperBuilder::Object<Version>(InvalidVersionObject, false)
    .Bind("major"_s, &Version::Major, JSONHelperBuilder::Int(), true)
    .Bind("minor"_s, &Version::Minor, JSONHelperBuilder::Int(), false);

bool VersionHelper(Version& out, Json::Value const* value, cmJSONState* state)
{
  out.Minor = 0;
  if (value->isInt()) {
    out.Major = value->asInt();
  } else if (value->isObject()) {
    if (!VersionObjectHelper(out, value, state)) {
      return false;
    }
  } else {
    state->AddErrorAtValue("Version must be an integer or object", value);
    return false;
  }
  return true;
}

using QueryRoot = cmInstrumentationQuery::QueryJSONRoot;

static auto const QueryRootHelper =
  JSONHelperBuilder::Object<QueryRoot>(ErrorMessages::InvalidRootQueryObject,
                                       false)
    .Bind("version"_s, &QueryRoot::version, VersionHelper, true)
    .Bind("options"_s, &QueryRoot::options, OptionListHelper, false)
    .Bind("hooks"_s, &QueryRoot::hooks, HookListHelper, false)
    .Bind("callbacks"_s, &QueryRoot::callbacks, CallbackListHelper, false);

static auto const QueryRootVersionOnlyHelper =
  JSONHelperBuilder::Object<QueryRoot>(ErrorMessages::InvalidRootQueryObject,
                                       true)
    .Bind("version"_s, &QueryRoot::version, VersionHelper, true);

bool cmInstrumentationQuery::ReadJSON(std::string const& filename,
                                      std::string& errorMessage,
                                      std::set<Option>& options,
                                      std::set<Hook>& hooks,
                                      std::vector<Callback>& callbacks)
{
  Json::Value root;
  this->parseState = cmJSONState(filename, &root);
  if (!this->parseState.errors.empty()) {
    errorMessage = this->parseState.GetErrorMessage(true);
    return false;
  }
  if (!QueryRootVersionOnlyHelper(this->queryRoot, &root, &this->parseState)) {
    errorMessage = this->parseState.GetErrorMessage(true);
    return false;
  }
  if (!ValidDataVersion(this->queryRoot.version)) {
    // Ignore invalid data versions
    return true;
  }
  if (!QueryRootHelper(this->queryRoot, &root, &this->parseState)) {
    errorMessage = this->parseState.GetErrorMessage(true);
    return false;
  }
  std::move(this->queryRoot.options.begin(), this->queryRoot.options.end(),
            std::inserter(options, options.end()));
  std::move(this->queryRoot.hooks.begin(), this->queryRoot.hooks.end(),
            std::inserter(hooks, hooks.end()));
  for (auto const& callback : this->queryRoot.callbacks) {
    callbacks.push_back({ callback, this->queryRoot.version });
  }
  return true;
}

bool cmInstrumentationQuery::ValidDataVersion(Version version)
{
  auto const latest = LatestDataVersion();
  return version.Major == latest.Major && version.Minor == latest.Minor;
}

Version cmInstrumentationQuery::LatestDataVersion()
{
  Version latest;
  latest.Major = 1;
  latest.Minor = 0;
  return latest;
}
