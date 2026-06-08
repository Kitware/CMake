/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmSourceGroupCommand.h"

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <utility>

#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmCMakePath.h"
#include "cmDiagnostics.h"
#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceGroup.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
template <typename Args>
bool ProcessTree(Args const& args, cmExecutionStatus& status)
{
  cmMakefile& mf = status.GetMakefile();

  auto const& currentSourceDir = mf.GetCurrentSourceDirectory();
  std::vector<std::string> files;
  if (args.Files) {
    files.reserve(args.Files->size());

    for (auto const& file : *args.Files) {
      if (file.empty()) {
        continue;
      }
      std::string fullPath =
        cmSystemTools::CollapseFullPath(file, currentSourceDir);
      files.emplace_back(std::move(fullPath));
    }
  } else {
    std::vector<std::unique_ptr<cmSourceFile>> const& sources =
      mf.GetSourceFiles();
    for (auto const& src : sources) {
      if (!src->GetIsGenerated()) {
        files.push_back(cmSystemTools::CollapseFullPath(
          src->GetLocation().GetFullPath(), currentSourceDir));
      }
    }
  }

  // final checks
  cmCMakePath const root{ cmSystemTools::CollapseFullPath(*args.Tree) };
  cmCMakePath const prefix{
    cmCMakePath{ args.Prefix ? *args.Prefix : "" }.Normal()
  };

  auto it = files.begin();
  while (it != files.end()) {
    if (it->empty() || cmSystemTools::FileIsDirectory(*it) ||
        (it->back() == '/' || it->back() == '\\')) {
      // Ignore any empty files or directories
      it = files.erase(it);
      continue;
    }

    cmCMakePath file{ *it };
    if (!root.IsPrefix(file)) {
      status.SetError(cmStrCat('"', root.GenericString(),
                               "\" is not a prefix of file \"",
                               file.GenericString(), '"'));
      return false;
    }

    // source groups generation
    cmCMakePath sourceGroup = prefix / file.Relative(root);
    if (sourceGroup.HasParentPath()) {
      sourceGroup = sourceGroup.GetParentPath();
    }
    std::vector<std::string> tokenizedSG =
      cmTokenize(sourceGroup.GenericString(), '/', cmTokenizerMode::New);

    auto* sg = mf.GetOrCreateSourceGroup(tokenizedSG);
    if (!sg) {
      status.SetError(cmStrCat("could not create source group for file \"",
                               file.GenericString(), '"'));
      return false;
    }
    sg->AddGroupFile(file.GenericString());

    ++it;
  }
  return true;
}
} // namespace

bool cmSourceGroupCommand(std::vector<std::string> const& args,
                          cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  static cm::string_view const Keywords[]{ "TREE"_s, "PREFIX"_s, "FILES"_s,
                                           "REGULAR_EXPRESSION"_s };
  cmMakefile& mf = status.GetMakefile();

  if (args.size() == 2 && !cm::contains(Keywords, args[0]) &&
      !cm::contains(Keywords, args[1])) {
    // The pre-1.8 version of the command is being invoked.
    cmSourceGroup* sg = mf.GetOrCreateSourceGroup(args[0]);
    if (!sg) {
      status.SetError("Could not create or find source group.");
      return false;
    }
    sg->SetGroupRegex(args[1]);
    return true;
  }

  struct Arguments : public ArgumentParser::ParseResult
  {
    cm::optional<std::string> GroupName;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Files;
    cm::optional<std::string> Regex;
    cm::optional<ArgumentParser::NonEmpty<std::string>> Tree;
    cm::optional<ArgumentParser::MaybeEmpty<std::string>> Prefix;
  };

  auto unsupportedKeyword =
    [&mf](Arguments&, cm::string_view key,
          cm::string_view /*value */) -> ArgumentParser::Continue {
    mf.IssueDiagnostic(
      cmDiagnostics::CMD_AUTHOR,
      cmStrCat("keyword \"", key, "\" will be ignored in this context."));
    return ArgumentParser::Continue::Yes;
  };
  // to distinguish REGULAR_EXPRESSION without values from with an empty string
  auto handleRegex = [](Arguments& result,
                        cm::string_view value) -> ArgumentParser::Continue {
    if (value.data()) {
      result.Regex = std::string{ value };
    }
    return ArgumentParser::Continue::No;
  };

  std::vector<std::string> unexpectedArgs;
  auto parser =
    cmArgumentParser<Arguments>{}.Bind("FILES"_s, &Arguments::Files);

  if (cm::contains(args, "TREE")) {
    // this is the TREE syntax
    parser.Bind("TREE"_s, &Arguments::Tree)
      .Bind("PREFIX"_s, &Arguments::Prefix)
      .Bind("REGULAR_EXPRESSION"_s, unsupportedKeyword);
  } else {
    // assume that first argument is the group name
    parser.Bind(0, &Arguments::GroupName)
      .Bind("REGULAR_EXPRESSION"_s, handleRegex, 0)
      .Bind("TREE"_s, unsupportedKeyword)
      .Bind("PREFIX"_s, unsupportedKeyword);
  }

  auto parsedArgs = parser.Parse(args, &unexpectedArgs);

  // do various checks for arguments consistency
  if (!parsedArgs.Check("", &unexpectedArgs, status)) {
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }
  if (!parsedArgs.Tree && !parsedArgs.GroupName) {
    status.SetError("missing source group name.");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.Tree) {
    if (!ProcessTree(parsedArgs, status)) {
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
  } else {
    if (!parsedArgs.Files && !parsedArgs.Regex) {
      // group is not created
      return true;
    }

    auto* sg = mf.GetOrCreateSourceGroup(*parsedArgs.GroupName);
    if (!sg) {
      status.SetError("could not create or find source group");
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    if (parsedArgs.Regex) {
      if (!sg->SetGroupRegex(*parsedArgs.Regex)) {
        status.SetError(cmStrCat("regular expression \"", *parsedArgs.Regex,
                                 "\" is invalid"));
        return false;
      }
    }
    if (parsedArgs.Files) {
      auto const& currentSourceDir = mf.GetCurrentSourceDirectory();
      for (auto const& file : *parsedArgs.Files) {
        sg->AddGroupFile(
          cmSystemTools::CollapseFullPath(file, currentSourceDir));
      }
    }
  }

  return true;
}
