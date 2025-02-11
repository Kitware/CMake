#include "cmInstrumentationQuery.h"

#include <algorithm>
#include <ctime>
#include <functional>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmJSONHelpers.h"
#include "cmStringAlgorithms.h"

std::vector<std::string> const cmInstrumentationQuery::QueryString{
  "staticSystemInformation", "dynamicSystemInformation"
};
std::vector<std::string> const cmInstrumentationQuery::HookString{
  "postGenerate",  "preBuild",        "postBuild",
  "preCMakeBuild", "postCMakeBuild",  "postTest",
  "postInstall",   "prepareForCDash", "manual"
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
      cmStrCat("Not a valid ", type, ": \"", value->asString(), "\""), value);
    return false;
  };
}
static auto const QueryHelper = EnumHelper<cmInstrumentationQuery::Query>(
  cmInstrumentationQuery::QueryString, "query");
static auto const QueryListHelper =
  JSONHelperBuilder::Vector<cmInstrumentationQuery::Query>(
    ErrorMessages::InvalidArray, QueryHelper);
static auto const HookHelper = EnumHelper<cmInstrumentationQuery::Hook>(
  cmInstrumentationQuery::HookString, "hook");
static auto const HookListHelper =
  JSONHelperBuilder::Vector<cmInstrumentationQuery::Hook>(
    ErrorMessages::InvalidArray, HookHelper);
static auto const CallbackHelper = JSONHelperBuilder::String();
static auto const CallbackListHelper = JSONHelperBuilder::Vector<std::string>(
  ErrorMessages::InvalidArray, CallbackHelper);
static auto const VersionHelper = JSONHelperBuilder::Int();

using QueryRoot = cmInstrumentationQuery::QueryJSONRoot;

static auto const QueryRootHelper =
  JSONHelperBuilder::Object<QueryRoot>(ErrorMessages::InvalidRootQueryObject,
                                       false)
    .Bind("version"_s, &QueryRoot::version, VersionHelper, true)
    .Bind("queries"_s, &QueryRoot::queries, QueryListHelper, false)
    .Bind("hooks"_s, &QueryRoot::hooks, HookListHelper, false)
    .Bind("callbacks"_s, &QueryRoot::callbacks, CallbackListHelper, false);

bool cmInstrumentationQuery::ReadJSON(std::string const& filename,
                                      std::string& errorMessage,
                                      std::set<Query>& queries,
                                      std::set<Hook>& hooks,
                                      std::vector<std::string>& callbacks)
{
  Json::Value root;
  this->parseState = cmJSONState(filename, &root);
  if (!this->parseState.errors.empty()) {
    std::cerr << this->parseState.GetErrorMessage(true) << std::endl;
    return false;
  }
  if (!QueryRootHelper(this->queryRoot, &root, &this->parseState)) {
    errorMessage = this->parseState.GetErrorMessage(true);
    return false;
  }
  std::move(this->queryRoot.queries.begin(), this->queryRoot.queries.end(),
            std::inserter(queries, queries.end()));
  std::move(this->queryRoot.hooks.begin(), this->queryRoot.hooks.end(),
            std::inserter(hooks, hooks.end()));
  std::move(this->queryRoot.callbacks.begin(), this->queryRoot.callbacks.end(),
            std::back_inserter(callbacks));
  return true;
}
