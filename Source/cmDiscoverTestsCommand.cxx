/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDiscoverTestsCommand.h"

#include <cstddef>
#include <map>
#include <ostream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmTestDiscovery.h"
#include "cmTestGenerator.h"

namespace {

struct Arguments : cmTestDiscoveryArgs
{
  bool CommandExpandLists = false;
  ArgumentParser::MaybeEmpty<std::vector<std::string>> Configurations;
};

class DiscoveryGenerator : public cmTestGenerator
{
public:
  DiscoveryGenerator(Arguments arguments, cmListFileBacktrace backtrace)
    : cmTestGenerator(nullptr, arguments.Configurations)
    , Args{ std::move(arguments) }
    , Backtrace{ std::move(backtrace) }
  {
  }

  void Compute(cmLocalGenerator* lg) override
  {
    this->cmTestGenerator::Compute(lg);
    this->BuildDependenciesByConfig.clear();
    if (!lg->GetMakefile()->IsOn("CMAKE_TESTING_ENABLED")) {
      return;
    }
    bool const filterConfigs =
      !lg->GetMakefile()
         ->GetGeneratorConfigs(cmMakefile::OnlyMultiConfig)
         .empty();
    for (std::string const& config : lg->GetMakefile()->GetGeneratorConfigs(
           cmMakefile::IncludeEmptyConfig)) {
      if (filterConfigs && !this->GeneratesForConfig(config)) {
        continue;
      }
      cmTestGenerator::BuildDependencies& deps =
        this->BuildDependenciesByConfig[config];
      if (!cmTestGenerator::EvaluateBuildDependencies(
            lg, config, this->Backtrace, "discover_tests", this->Args.Command,
            this->Args.BuildDepends, deps, lg->GetMakefile())) {
        deps = {};
        continue;
      }
      for (cmGeneratorTarget* target : deps.Targets) {
        lg->AddDirectoryTestPrepDependency(config,
                                           { target->GetName(), target });
      }
      for (cmTestGenerator::BuildDependencies::FileDependency const& file :
           deps.Files) {
        lg->AddDirectoryTestPrepDependency(
          config, { file.Path, nullptr, file.Owner, file.Generated });
      }
    }
  }

private:
  void GenerateProperties(std::ostream& os, Indent indent,
                          std::vector<std::string> const& props,
                          std::string const& config, cmGeneratorExpression& ge)
  {
    for (std::size_t i = 0; i < props.size(); i += 2) {
      os << '\n'
         << indent << Quote(props[i]) << ' '
         << Quote(ge.Parse(props[i + 1])->Evaluate(this->LG, config));
    }
  }

  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override
  {
    // Set up generator expression evaluation context.
    cmGeneratorExpression ge(*this->LG->GetMakefile()->GetCMakeInstance(),
                             this->Backtrace);

    std::vector<std::string> const buildDepends =
      this->LG->GetGlobalGenerator()->GetTestBuildDependencyPaths(
        config, this->BuildDependenciesByConfig.at(config));

    auto const in = indent.Next();
    os << indent << "discover_tests(COMMAND ";
    this->GenerateCommand(os, this->Args.Command, config,
                          this->Args.CommandExpandLists, ge);
    os << '\n' << in << "DISCOVERY_ARGS";
    for (auto const& arg : this->Args.DiscoveryArgs) {
      os << ' ' << Quote(arg);
    }
    os << '\n' << in << "DISCOVERY_MATCH " << Quote(this->Args.DiscoveryMatch);
    if (!this->Args.DiscoveryProperties.empty()) {
      os << '\n' << in << "DISCOVERY_PROPERTIES";
      this->GenerateProperties(os, in.Next(), this->Args.DiscoveryProperties,
                               config, ge);
    }
    os << '\n' << in << "TEST_NAME " << Quote(this->Args.TestName);
    os << '\n' << in << "TEST_ARGS";
    for (auto const& arg : this->Args.TestArgs) {
      os << ' ' << Quote(arg);
    }
    os << '\n' << in << "TEST_PROPERTIES";
    this->GenerateProperties(os, in.Next(), this->Args.TestProperties, config,
                             ge);
    os << '\n' << in.Next();
    this->GenerateBacktrace(os, this->Backtrace);
    if (!buildDepends.empty()) {
      os << '\n' << in << "BUILD_DEPENDS";
      for (std::string const& dep : buildDepends) {
        os << ' ' << Quote(dep);
      }
    }
    os << '\n' << in << ")\n";
  }

  Arguments Args;
  cmListFileBacktrace Backtrace;
  std::map<std::string, cmTestGenerator::BuildDependencies>
    BuildDependenciesByConfig;
};

bool SetsFixtureRepeatMode(std::vector<std::string> const& properties)
{
  for (std::size_t i = 0; i < properties.size(); i += 2) {
    if (properties[i] == "FIXTURE_REPEAT_MODE"_s ||
        properties[i] == "_CMAKE_DEFAULT_FIXTURE_REPEAT_MODE"_s) {
      return true;
    }
  }
  return false;
}

} // namespace

bool cmDiscoverTestsCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  static auto const parser =
    cmArgumentParser<Arguments>{ cmTestDiscoveryParser<Arguments>() }
      .Bind("COMMAND_EXPAND_LISTS"_s, &Arguments::CommandExpandLists)
      .Bind("CONFIGURATIONS"_s, &Arguments::Configurations);

  auto unparsed = std::vector<std::string>{};
  Arguments arguments = parser.Parse(args, &unparsed);
  if (arguments.MaybeReportError(status.GetMakefile())) {
    return true;
  }

  if (!unparsed.empty()) {
    status.SetError(
      cmStrCat(" given unknown argument \"", unparsed.front(), "\"."));
    return false;
  }

  if (arguments.DiscoveryProperties.size() % 2 != 0) {
    status.SetError(" DISCOVERY_PROPERTIES must be key-value pairs.");
    return false;
  }

  if (arguments.TestProperties.size() % 2 != 0) {
    status.SetError(" TEST_PROPERTIES must be key-value pairs.");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // The discovered tests are created while ctest runs, too late for the
  // policy to reach them.  Only NEW needs carrying through.  With nothing
  // recorded, ctest already uses the EACH_TEST_SEPARATELY behavior of
  // CMake 4.4 and below.
  if (mf.GetPolicyStatus(cmPolicies::CMP0224) == cmPolicies::NEW &&
      !SetsFixtureRepeatMode(arguments.TestProperties)) {
    arguments.TestProperties.emplace_back(
      "_CMAKE_DEFAULT_FIXTURE_REPEAT_MODE");
    arguments.TestProperties.emplace_back("AROUND_EACH_REPEAT");
  }

  mf.AddTestGenerator(cm::make_unique<DiscoveryGenerator>(std::move(arguments),
                                                          mf.GetBacktrace()));
  return true;
}
