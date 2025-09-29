/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestDiscoverTests.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <string>
#include <vector>

#include <cmsys/RegularExpression.hxx>

#include "cmArgumentParserTypes.h"
#include "cmCTestTestHandler.h"
#include "cmDuration.h"
#include "cmEnvironment.h"
#include "cmExecutionStatus.h"
#include "cmList.h"
#include "cmProcessOutput.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTestDiscovery.h"

namespace {

struct DiscoveryProperties
{
  DiscoveryProperties(std::vector<std::string> const& props);

  std::vector<std::string> Environment;
  std::vector<std::string> EnvironmentModification;
  cmDuration Timeout = cmDuration(5.0);
  std::string WorkingDirectory;
};

DiscoveryProperties::DiscoveryProperties(std::vector<std::string> const& props)
{
  for (std::size_t i = 0; i < props.size(); i += 2) {
    auto const& key = props[i];
    auto const& val = props[i + 1];
    if (key == "ENVIRONMENT") {
      cmExpandList(val, this->Environment);
    } else if (key == "ENVIRONMENT_MODIFICATION") {
      cmExpandList(val, this->EnvironmentModification);
    } else if (key == "TIMEOUT") {
      this->Timeout = cmDuration(atof(val.c_str()));
    } else if (key == "WORKING_DIRECTORY") {
      this->WorkingDirectory = val;
    }
  }
}

std::string AddAnchors(std::string str)
{
  if (str.empty() || str.front() != '^') {
    str.insert(str.begin(), '^');
  }
  if (str.size() < 2 || str.back() != '$') {
    str.push_back('$');
  }
  return str;
}

class RegexReplacer
{
public:
  RegexReplacer(cmsys::RegularExpression& regex)
    : RegEx(regex)
  {
  }

  std::string operator()(std::string str)
  {
    for (int i = 0; i <= this->RegEx.num_groups(); ++i) {
      cmsys::SystemTools::ReplaceString(str, cmStrCat('\\', i),
                                        this->RegEx.match(i));
    }
    return str;
  }

private:
  cmsys::RegularExpression& RegEx;
};

} // namespace

bool cmCTestDiscoverTests(cmTestDiscoveryArgs const& args,
                          cmCTestTestHandler* handler,
                          cmExecutionStatus& status)
{
  cmsys::RegularExpression re;
  if (!re.compile(AddAnchors(args.DiscoveryMatch))) {
    std::string e = "DISCOVERY_MATCH failed to compile regex \"" +
      args.DiscoveryMatch + "\".";
    status.SetError(e);
    return false;
  }

  auto const props = DiscoveryProperties{ args.DiscoveryProperties };

  auto env = cmEnvironment{ cmSystemTools::GetEnvironmentVariables() };
  env.Update(props.Environment);
  if (!props.EnvironmentModification.empty()) {
    auto diff = cmEnvironmentModification{};
    if (!diff.Add(props.EnvironmentModification)) {
      return false;
    }
    diff.ApplyTo(env);
  }

  auto runCommand = std::vector<std::string>(args.Command);
  runCommand.reserve(runCommand.size() + args.DiscoveryArgs.size());
  std::copy(args.DiscoveryArgs.begin(), args.DiscoveryArgs.end(),
            std::back_inserter(runCommand));

  std::string stdOut;
  std::string stdErr;
  bool const res = cmSystemTools::RunSingleCommand(
    runCommand, &stdOut, &stdErr, nullptr, props.WorkingDirectory.c_str(),
    cmSystemTools::OUTPUT_NONE, props.Timeout, cmProcessOutput::Auto,
    env.GetVariables());
  if (!res) {
    status.SetError(cmStrCat(" failed to run command: ",
                             cmSystemTools::PrintSingleCommand(runCommand),
                             "\n", stdErr));
    return false;
  }

  auto replace = RegexReplacer{ re };

  std::vector<std::string> lines;
  cmSystemTools::Split(stdOut, lines);
  for (auto const& line : lines) {
    if (!re.find(line)) {
      continue;
    }

    auto const testName = replace(args.TestName);
    auto testArgs = std::vector<std::string>{ testName };
    testArgs.reserve(1 + args.Command.size() + args.TestArgs.size());
    std::copy(args.Command.begin(), args.Command.end(),
              std::back_inserter(testArgs));
    std::transform(args.TestArgs.begin(), args.TestArgs.end(),
                   std::back_inserter(testArgs), replace);

    if (!handler->AddTest(testArgs)) {
      return false;
    }

    auto testProperties = std::vector<std::string>{ testName, "PROPERTIES" };
    testProperties.reserve(2 + args.TestProperties.size());
    for (std::size_t i = 0; i < args.TestProperties.size(); i += 2) {
      testProperties.push_back(args.TestProperties[i]);
      testProperties.push_back(replace(args.TestProperties[i + 1]));
    }
    if (!handler->SetTestsProperties(testProperties)) {
      return false;
    }
  }

  return true;
}
