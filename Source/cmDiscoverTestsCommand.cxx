/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmDiscoverTestsCommand.h"

#include <cstddef>
#include <ostream>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmScriptGenerator.h"
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
    os << '\n' << in << ")\n";
  }

  Arguments Args;
  cmListFileBacktrace Backtrace;
};

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
    status.SetError(" given unknown argument \"" + unparsed.front() + "\".");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  mf.AddTestGenerator(cm::make_unique<DiscoveryGenerator>(std::move(arguments),
                                                          mf.GetBacktrace()));
  return true;
}
