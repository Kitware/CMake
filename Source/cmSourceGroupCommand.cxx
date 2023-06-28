/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSourceGroupCommand.h"

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <utility>

#include <cmext/algorithm>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceGroup.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

using ParsedArguments = std::map<std::string, std::vector<std::string>>;
using ExpectedOptions = std::vector<std::string>;

const std::string kTreeOptionName = "TREE";
const std::string kPrefixOptionName = "PREFIX";
const std::string kFilesOptionName = "FILES";
const std::string kRegexOptionName = "REGULAR_EXPRESSION";
const std::string kSourceGroupOptionName = "<sg_name>";

std::vector<std::string> tokenizePath(const std::string& path)
{
  return cmTokenize(path, "\\/");
}

std::set<std::string> getSourceGroupFilesPaths(
  const std::string& root, const std::vector<std::string>& files)
{
  std::set<std::string> ret;
  const std::string::size_type rootLength = root.length();

  for (std::string const& file : files) {
    ret.insert(file.substr(rootLength + 1)); // +1 to also omnit last '/'
  }

  return ret;
}

bool rootIsPrefix(const std::string& root,
                  const std::vector<std::string>& files, std::string& error)
{
  for (std::string const& file : files) {
    if (!cmHasPrefix(file, root)) {
      error = cmStrCat("ROOT: ", root, " is not a prefix of file: ", file);
      return false;
    }
  }

  return true;
}

std::vector<std::string> prepareFilesPathsForTree(
  const std::vector<std::string>& filesPaths,
  const std::string& currentSourceDir)
{
  std::vector<std::string> prepared;
  prepared.reserve(filesPaths.size());

  for (auto const& filePath : filesPaths) {
    std::string fullPath =
      cmSystemTools::CollapseFullPath(filePath, currentSourceDir);
    // If provided file path is actually not a directory, silently ignore it.
    if (cmSystemTools::FileIsDirectory(fullPath)) {
      continue;
    }

    // Handle directory that doesn't exist yet.
    if (!fullPath.empty() &&
        (fullPath.back() == '/' || fullPath.back() == '\\')) {
      continue;
    }

    prepared.emplace_back(std::move(fullPath));
  }

  return prepared;
}

bool addFilesToItsSourceGroups(const std::string& root,
                               const std::set<std::string>& sgFilesPaths,
                               const std::string& prefix, cmMakefile& makefile,
                               std::string& errorMsg)
{
  cmSourceGroup* sg;

  for (std::string const& sgFilesPath : sgFilesPaths) {

    std::vector<std::string> tokenizedPath;
    if (!prefix.empty()) {
      tokenizedPath = tokenizePath(cmStrCat(prefix, '/', sgFilesPath));
    } else {
      tokenizedPath = tokenizePath(sgFilesPath);
    }

    if (!tokenizedPath.empty()) {
      tokenizedPath.pop_back();

      if (tokenizedPath.empty()) {
        tokenizedPath.emplace_back();
      }

      sg = makefile.GetOrCreateSourceGroup(tokenizedPath);

      if (!sg) {
        errorMsg = "Could not create source group for file: " + sgFilesPath;
        return false;
      }
      const std::string fullPath =
        cmSystemTools::CollapseFullPath(sgFilesPath, root);
      sg->AddGroupFile(fullPath);
    }
  }

  return true;
}

ExpectedOptions getExpectedOptions()
{
  ExpectedOptions options;

  options.push_back(kTreeOptionName);
  options.push_back(kPrefixOptionName);
  options.push_back(kFilesOptionName);
  options.push_back(kRegexOptionName);

  return options;
}

bool isExpectedOption(const std::string& argument,
                      const ExpectedOptions& expectedOptions)
{
  return cm::contains(expectedOptions, argument);
}

void parseArguments(const std::vector<std::string>& args,
                    ParsedArguments& parsedArguments)
{
  const ExpectedOptions expectedOptions = getExpectedOptions();
  size_t i = 0;

  // at this point we know that args vector is not empty

  // if first argument is not one of expected options it's source group name
  if (!isExpectedOption(args[0], expectedOptions)) {
    // get source group name and go to next argument
    parsedArguments[kSourceGroupOptionName].push_back(args[0]);
    ++i;
  }

  for (; i < args.size();) {
    // get current option and increment index to go to next argument
    const std::string& currentOption = args[i++];

    // create current option entry in parsed arguments
    std::vector<std::string>& currentOptionArguments =
      parsedArguments[currentOption];

    // collect option arguments while we won't find another expected option
    while (i < args.size() && !isExpectedOption(args[i], expectedOptions)) {
      currentOptionArguments.push_back(args[i++]);
    }
  }
}

} // namespace

static bool checkArgumentsPreconditions(const ParsedArguments& parsedArguments,
                                        std::string& errorMsg);

static bool processTree(cmMakefile& mf, ParsedArguments& parsedArguments,
                        std::string& errorMsg);

static bool checkSingleParameterArgumentPreconditions(
  const std::string& argument, const ParsedArguments& parsedArguments,
  std::string& errorMsg);

bool cmSourceGroupCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // If only two arguments are given, the pre-1.8 version of the
  // command is being invoked.
  bool isShortTreeSyntax =
    ((args.size() == 2) && (args[0] == kTreeOptionName) &&
     cmSystemTools::FileIsDirectory(args[1]));
  if (args.size() == 2 && args[1] != kFilesOptionName && !isShortTreeSyntax) {
    cmSourceGroup* sg = mf.GetOrCreateSourceGroup(args[0]);

    if (!sg) {
      status.SetError("Could not create or find source group");
      return false;
    }

    sg->SetGroupRegex(args[1].c_str());
    return true;
  }

  ParsedArguments parsedArguments;
  std::string errorMsg;

  parseArguments(args, parsedArguments);

  if (!checkArgumentsPreconditions(parsedArguments, errorMsg)) {
    return false;
  }

  if (parsedArguments.find(kTreeOptionName) != parsedArguments.end()) {
    if (!processTree(mf, parsedArguments, errorMsg)) {
      status.SetError(errorMsg);
      return false;
    }
  } else {
    if (parsedArguments.find(kSourceGroupOptionName) ==
        parsedArguments.end()) {
      status.SetError("Missing source group name.");
      return false;
    }

    cmSourceGroup* sg = mf.GetOrCreateSourceGroup(args[0]);

    if (!sg) {
      status.SetError("Could not create or find source group");
      return false;
    }

    // handle regex
    if (parsedArguments.find(kRegexOptionName) != parsedArguments.end()) {
      const std::string& sgRegex = parsedArguments[kRegexOptionName].front();
      sg->SetGroupRegex(sgRegex.c_str());
    }

    // handle files
    const std::vector<std::string>& filesArguments =
      parsedArguments[kFilesOptionName];
    for (auto const& filesArg : filesArguments) {
      std::string src = filesArg;
      src =
        cmSystemTools::CollapseFullPath(src, mf.GetCurrentSourceDirectory());
      sg->AddGroupFile(src);
    }
  }

  return true;
}

static bool checkArgumentsPreconditions(const ParsedArguments& parsedArguments,
                                        std::string& errorMsg)
{
  return checkSingleParameterArgumentPreconditions(
           kPrefixOptionName, parsedArguments, errorMsg) &&
    checkSingleParameterArgumentPreconditions(kTreeOptionName, parsedArguments,
                                              errorMsg) &&
    checkSingleParameterArgumentPreconditions(kRegexOptionName,
                                              parsedArguments, errorMsg);
}

static bool processTree(cmMakefile& mf, ParsedArguments& parsedArguments,
                        std::string& errorMsg)
{
  const std::string root =
    cmSystemTools::CollapseFullPath(parsedArguments[kTreeOptionName].front());
  std::string prefix = parsedArguments[kPrefixOptionName].empty()
    ? ""
    : parsedArguments[kPrefixOptionName].front();

  std::vector<std::string> files = parsedArguments[kFilesOptionName];
  if (files.empty()) {
    const std::vector<std::unique_ptr<cmSourceFile>>& srcFiles =
      mf.GetSourceFiles();
    for (const auto& srcFile : srcFiles) {
      if (!srcFile->GetIsGenerated()) {
        files.push_back(srcFile->GetLocation().GetFullPath());
      }
    }
  }

  const std::vector<std::string> filesVector =
    prepareFilesPathsForTree(files, mf.GetCurrentSourceDirectory());

  if (!rootIsPrefix(root, filesVector, errorMsg)) {
    return false;
  }

  std::set<std::string> sourceGroupPaths =
    getSourceGroupFilesPaths(root, filesVector);

  return addFilesToItsSourceGroups(root, sourceGroupPaths, prefix, mf,
                                   errorMsg);
}

static bool checkSingleParameterArgumentPreconditions(
  const std::string& argument, const ParsedArguments& parsedArguments,
  std::string& errorMsg)
{
  auto foundArgument = parsedArguments.find(argument);
  if (foundArgument != parsedArguments.end()) {
    const std::vector<std::string>& optionArguments = foundArgument->second;

    if (optionArguments.empty()) {
      errorMsg = argument + " argument given without an argument.";
      return false;
    }
    if (optionArguments.size() > 1) {
      errorMsg = "too many arguments passed to " + argument + ".";
      return false;
    }
  }

  return true;
}
