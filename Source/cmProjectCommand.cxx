/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmProjectCommand.h"

#include <array>
#include <cstdio>
#include <limits>
#include <set>
#include <utility>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmExperimental.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {

bool IncludeByVariable(cmExecutionStatus& status, std::string const& variable);
void TopLevelCMakeVarCondSet(cmMakefile& mf, std::string const& name,
                             std::string const& value);

struct ProjectArguments : ArgumentParser::ParseResult
{
  cm::optional<std::string> ProjectName;
  cm::optional<std::string> Version;
  cm::optional<std::string> CompatVersion;
  cm::optional<std::string> Description;
  cm::optional<std::string> HomepageURL;
  cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Languages;
};

struct ProjectArgumentParser : public cmArgumentParser<void>
{
  ProjectArgumentParser& BindKeywordMissingValue(
    std::vector<cm::string_view>& ref)
  {
    this->cmArgumentParser<void>::BindKeywordMissingValue(
      [&ref](Instance&, cm::string_view arg) { ref.emplace_back(arg); });
    return *this;
  }
};

} // namespace

bool cmProjectCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  std::vector<std::string> unparsedArgs;
  std::vector<cm::string_view> missingValueKeywords;
  std::vector<cm::string_view> parsedKeywords;
  ProjectArguments prArgs;
  ProjectArgumentParser parser;
  parser.BindKeywordMissingValue(missingValueKeywords)
    .BindParsedKeywords(parsedKeywords)
    .Bind(0, prArgs.ProjectName)
    .Bind("VERSION"_s, prArgs.Version)
    .Bind("DESCRIPTION"_s, prArgs.Description)
    .Bind("HOMEPAGE_URL"_s, prArgs.HomepageURL)
    .Bind("LANGUAGES"_s, prArgs.Languages);

  cmMakefile& mf = status.GetMakefile();
  bool enableCompatVersion = cmExperimental::HasSupportEnabled(
    mf, cmExperimental::Feature::ExportPackageInfo);

  if (enableCompatVersion) {
    parser.Bind("COMPAT_VERSION"_s, prArgs.CompatVersion);
  }

  parser.Parse(args, &unparsedArgs, 0);

  if (!prArgs.ProjectName) {
    status.SetError("PROJECT called with incorrect number of arguments");
    return false;
  }

  if (mf.IsRootMakefile() &&
      !mf.GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION")) {
    mf.IssueMessage(
      MessageType::AUTHOR_WARNING,
      "cmake_minimum_required() should be called prior to this top-level "
      "project() call. Please see the cmake-commands(7) manual for usage "
      "documentation of both commands.");
  }

  if (!IncludeByVariable(status, "CMAKE_PROJECT_INCLUDE_BEFORE")) {
    return false;
  }

  if (!IncludeByVariable(
        status, "CMAKE_PROJECT_" + *prArgs.ProjectName + "_INCLUDE_BEFORE")) {
    return false;
  }

  mf.SetProjectName(*prArgs.ProjectName);

  cmPolicies::PolicyStatus cmp0180 = mf.GetPolicyStatus(cmPolicies::CMP0180);

  std::string varName = cmStrCat(*prArgs.ProjectName, "_BINARY_DIR"_s);
  bool nonCacheVarAlreadySet = mf.IsNormalDefinitionSet(varName);
  mf.AddCacheDefinition(varName, mf.GetCurrentBinaryDirectory(),
                        "Value Computed by CMake", cmStateEnums::STATIC);
  if (cmp0180 == cmPolicies::NEW || nonCacheVarAlreadySet) {
    mf.AddDefinition(varName, mf.GetCurrentBinaryDirectory());
  }

  varName = cmStrCat(*prArgs.ProjectName, "_SOURCE_DIR"_s);
  nonCacheVarAlreadySet = mf.IsNormalDefinitionSet(varName);
  mf.AddCacheDefinition(varName, mf.GetCurrentSourceDirectory(),
                        "Value Computed by CMake", cmStateEnums::STATIC);
  if (cmp0180 == cmPolicies::NEW || nonCacheVarAlreadySet) {
    mf.AddDefinition(varName, mf.GetCurrentSourceDirectory());
  }

  mf.AddDefinition("PROJECT_BINARY_DIR", mf.GetCurrentBinaryDirectory());
  mf.AddDefinition("PROJECT_SOURCE_DIR", mf.GetCurrentSourceDirectory());

  mf.AddDefinition("PROJECT_NAME", *prArgs.ProjectName);

  mf.AddDefinitionBool("PROJECT_IS_TOP_LEVEL", mf.IsRootMakefile());

  varName = cmStrCat(*prArgs.ProjectName, "_IS_TOP_LEVEL"_s);
  nonCacheVarAlreadySet = mf.IsNormalDefinitionSet(varName);
  mf.AddCacheDefinition(varName, mf.IsRootMakefile() ? "ON" : "OFF",
                        "Value Computed by CMake", cmStateEnums::STATIC);
  if (cmp0180 == cmPolicies::NEW || nonCacheVarAlreadySet) {
    mf.AddDefinition(varName, mf.IsRootMakefile() ? "ON" : "OFF");
  }

  TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_NAME", *prArgs.ProjectName);

  std::set<cm::string_view> seenKeywords;
  for (cm::string_view keyword : parsedKeywords) {
    if (seenKeywords.find(keyword) != seenKeywords.end()) {
      mf.IssueMessage(MessageType::FATAL_ERROR,
                      cmStrCat(keyword, " may be specified at most once."));
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }
    seenKeywords.insert(keyword);
  }

  for (cm::string_view keyword : missingValueKeywords) {
    mf.IssueMessage(MessageType::WARNING,
                    cmStrCat(keyword,
                             " keyword not followed by a value or was "
                             "followed by a value that expanded to nothing."));
  }

  if (!unparsedArgs.empty()) {
    if (prArgs.Languages) {
      mf.IssueMessage(
        MessageType::WARNING,
        cmStrCat("the following parameters must be specified after LANGUAGES "
                 "keyword: ",
                 cmJoin(unparsedArgs, ", "), '.'));
    } else if (prArgs.Version || prArgs.Description || prArgs.HomepageURL) {
      mf.IssueMessage(MessageType::FATAL_ERROR,
                      "project with VERSION, DESCRIPTION or HOMEPAGE_URL must "
                      "use LANGUAGES before language names.");
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }
  } else if (prArgs.Languages && prArgs.Languages->empty()) {
    prArgs.Languages->emplace_back("NONE");
  }

  if (prArgs.CompatVersion && !prArgs.Version) {
    mf.IssueMessage(MessageType::FATAL_ERROR,
                    "project with COMPAT_VERSION must also provide VERSION.");
    cmSystemTools::SetFatalErrorOccurred();
    return true;
  }

  cmsys::RegularExpression vx(
    R"(^([0-9]+(\.[0-9]+(\.[0-9]+(\.[0-9]+)?)?)?)?$)");

  constexpr std::size_t MAX_VERSION_COMPONENTS = 4u;
  std::string version_string;
  std::array<std::string, MAX_VERSION_COMPONENTS> version_components;

  if (prArgs.Version) {
    if (!vx.find(*prArgs.Version)) {
      std::string e =
        R"(VERSION ")" + *prArgs.Version + R"(" format invalid.)";
      mf.IssueMessage(MessageType::FATAL_ERROR, e);
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }

    cmPolicies::PolicyStatus const cmp0096 =
      mf.GetPolicyStatus(cmPolicies::CMP0096);

    if (cmp0096 == cmPolicies::OLD || cmp0096 == cmPolicies::WARN) {
      constexpr size_t maxIntLength =
        std::numeric_limits<unsigned>::digits10 + 2;
      char vb[MAX_VERSION_COMPONENTS][maxIntLength];
      unsigned v[MAX_VERSION_COMPONENTS] = { 0, 0, 0, 0 };
      int const vc = std::sscanf(prArgs.Version->c_str(), "%u.%u.%u.%u", &v[0],
                                 &v[1], &v[2], &v[3]);
      for (auto i = 0u; i < MAX_VERSION_COMPONENTS; ++i) {
        if (static_cast<int>(i) < vc) {
          std::snprintf(vb[i], maxIntLength, "%u", v[i]);
          version_string += &"."[static_cast<std::size_t>(i == 0)];
          version_string += vb[i];
          version_components[i] = vb[i];
        } else {
          vb[i][0] = '\x00';
        }
      }
    } else {
      // The regex above verified that we have a .-separated string of
      // non-negative integer components.  Keep the original string.
      version_string = std::move(*prArgs.Version);
      // Split the integer components.
      auto components = cmSystemTools::SplitString(version_string, '.');
      for (auto i = 0u; i < components.size(); ++i) {
        version_components[i] = std::move(components[i]);
      }
    }
  }

  if (prArgs.CompatVersion) {
    if (!vx.find(*prArgs.CompatVersion)) {
      std::string e =
        R"(COMPAT_VERSION ")" + *prArgs.CompatVersion + R"(" format invalid.)";
      mf.IssueMessage(MessageType::FATAL_ERROR, e);
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }

    if (cmSystemTools::VersionCompareGreater(*prArgs.CompatVersion,
                                             version_string)) {
      mf.IssueMessage(MessageType::FATAL_ERROR,
                      "COMPAT_VERSION must be less than or equal to VERSION");
      cmSystemTools::SetFatalErrorOccurred();
      return true;
    }
  }

  auto createVariables = [&](cm::string_view var, std::string const& val) {
    mf.AddDefinition(cmStrCat("PROJECT_"_s, var), val);
    mf.AddDefinition(cmStrCat(*prArgs.ProjectName, "_"_s, var), val);
    TopLevelCMakeVarCondSet(mf, cmStrCat("CMAKE_PROJECT_"_s, var), val);
  };

  createVariables("VERSION"_s, version_string);
  createVariables("VERSION_MAJOR"_s, version_components[0]);
  createVariables("VERSION_MINOR"_s, version_components[1]);
  createVariables("VERSION_PATCH"_s, version_components[2]);
  createVariables("VERSION_TWEAK"_s, version_components[3]);
  createVariables("DESCRIPTION"_s, prArgs.Description.value_or(""));
  createVariables("HOMEPAGE_URL"_s, prArgs.HomepageURL.value_or(""));

  if (enableCompatVersion) {
    createVariables("COMPAT_VERSION"_s, prArgs.CompatVersion.value_or(""));
  }

  if (unparsedArgs.empty() && !prArgs.Languages) {
    // if no language is specified do c and c++
    mf.EnableLanguage({ "C", "CXX" }, false);
  } else {
    if (!unparsedArgs.empty()) {
      mf.EnableLanguage(unparsedArgs, false);
    }
    if (prArgs.Languages) {
      mf.EnableLanguage(*prArgs.Languages, false);
    }
  }

  if (!IncludeByVariable(status, "CMAKE_PROJECT_INCLUDE")) {
    return false;
  }

  if (!IncludeByVariable(
        status, "CMAKE_PROJECT_" + *prArgs.ProjectName + "_INCLUDE")) {
    return false;
  }

  return true;
}

namespace {
bool IncludeByVariable(cmExecutionStatus& status, std::string const& variable)
{
  cmMakefile& mf = status.GetMakefile();
  cmValue include = mf.GetDefinition(variable);
  if (!include) {
    return true;
  }
  cmList includeFiles{ *include };

  bool failed = false;
  for (auto filePath : includeFiles) {
    // Any relative path without a .cmake extension is checked for valid cmake
    // modules. This logic should be consistent with CMake's include() command.
    // Otherwise default to checking relative path w.r.t. source directory
    if (!cmSystemTools::FileIsFullPath(filePath) &&
        !cmHasLiteralSuffix(filePath, ".cmake")) {
      std::string mfile = mf.GetModulesFile(cmStrCat(filePath, ".cmake"));
      if (mfile.empty()) {
        status.SetError(
          cmStrCat("could not find requested module:\n  ", filePath));
        failed = true;
        continue;
      }
      filePath = mfile;
    }
    std::string includeFile = cmSystemTools::CollapseFullPath(
      filePath, mf.GetCurrentSourceDirectory());
    if (!cmSystemTools::FileExists(includeFile)) {
      status.SetError(
        cmStrCat("could not find requested file:\n  ", filePath));
      failed = true;
      continue;
    }
    if (cmSystemTools::FileIsDirectory(includeFile)) {
      status.SetError(
        cmStrCat("requested file is a directory:\n  ", filePath));
      failed = true;
      continue;
    }

    bool const readit = mf.ReadDependentFile(filePath);
    if (readit) {
      // If the included file ran successfully, continue to the next file
      continue;
    }

    if (cmSystemTools::GetFatalErrorOccurred()) {
      failed = true;
      continue;
    }

    status.SetError(cmStrCat("could not load requested file:\n  ", filePath));
    failed = true;
  }
  // At this point all files were processed
  return !failed;
}

void TopLevelCMakeVarCondSet(cmMakefile& mf, std::string const& name,
                             std::string const& value)
{
  // Set the CMAKE_PROJECT_XXX variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one.
  if (!mf.GetDefinition(name) || mf.IsRootMakefile()) {
    mf.RemoveDefinition(name);
    mf.AddCacheDefinition(name, value, "Value Computed by CMake",
                          cmStateEnums::STATIC);
  }
}
}
