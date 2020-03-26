/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmProjectCommand.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <limits>
#include <utility>

#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static bool IncludeByVariable(cmExecutionStatus& status,
                              const std::string& variable);
static void TopLevelCMakeVarCondSet(cmMakefile& mf, std::string const& name,
                                    std::string const& value);

bool cmProjectCommand(std::vector<std::string> const& args,
                      cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("PROJECT called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  if (!IncludeByVariable(status, "CMAKE_PROJECT_INCLUDE_BEFORE")) {
    return false;
  }

  std::string const& projectName = args[0];

  if (!IncludeByVariable(status,
                         "CMAKE_PROJECT_" + projectName + "_INCLUDE_BEFORE")) {
    return false;
  }

  mf.SetProjectName(projectName);

  mf.AddCacheDefinition(projectName + "_BINARY_DIR",
                        mf.GetCurrentBinaryDirectory().c_str(),
                        "Value Computed by CMake", cmStateEnums::STATIC);
  mf.AddCacheDefinition(projectName + "_SOURCE_DIR",
                        mf.GetCurrentSourceDirectory().c_str(),
                        "Value Computed by CMake", cmStateEnums::STATIC);

  mf.AddDefinition("PROJECT_BINARY_DIR", mf.GetCurrentBinaryDirectory());
  mf.AddDefinition("PROJECT_SOURCE_DIR", mf.GetCurrentSourceDirectory());

  mf.AddDefinition("PROJECT_NAME", projectName);

  // Set the CMAKE_PROJECT_NAME variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one, so that
  // CMAKE_PROJECT_NAME will match PROJECT_NAME, and cmake --build
  // will work.
  if (!mf.GetDefinition("CMAKE_PROJECT_NAME") || mf.IsRootMakefile()) {
    mf.AddDefinition("CMAKE_PROJECT_NAME", projectName);
    mf.AddCacheDefinition("CMAKE_PROJECT_NAME", projectName.c_str(),
                          "Value Computed by CMake", cmStateEnums::STATIC);
  }

  bool haveVersion = false;
  bool haveLanguages = false;
  bool haveDescription = false;
  bool haveHomepage = false;
  bool injectedProjectCommand = false;
  std::string version;
  std::string description;
  std::string homepage;
  std::vector<std::string> languages;
  std::function<void()> missedValueReporter;
  auto resetReporter = [&missedValueReporter]() {
    missedValueReporter = std::function<void()>();
  };
  enum Doing
  {
    DoingDescription,
    DoingHomepage,
    DoingLanguages,
    DoingVersion
  };
  Doing doing = DoingLanguages;
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "LANGUAGES") {
      if (haveLanguages) {
        mf.IssueMessage(MessageType::FATAL_ERROR,
                        "LANGUAGES may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveLanguages = true;
      if (missedValueReporter) {
        missedValueReporter();
      }
      doing = DoingLanguages;
      if (!languages.empty()) {
        std::string msg = cmStrCat(
          "the following parameters must be specified after LANGUAGES "
          "keyword: ",
          cmJoin(languages, ", "), '.');
        mf.IssueMessage(MessageType::WARNING, msg);
      }
    } else if (args[i] == "VERSION") {
      if (haveVersion) {
        mf.IssueMessage(MessageType::FATAL_ERROR,
                        "VERSION may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveVersion = true;
      if (missedValueReporter) {
        missedValueReporter();
      }
      doing = DoingVersion;
      missedValueReporter = [&mf, &resetReporter]() {
        mf.IssueMessage(
          MessageType::WARNING,
          "VERSION keyword not followed by a value or was followed by a "
          "value that expanded to nothing.");
        resetReporter();
      };
    } else if (args[i] == "DESCRIPTION") {
      if (haveDescription) {
        mf.IssueMessage(MessageType::FATAL_ERROR,
                        "DESCRIPTION may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveDescription = true;
      if (missedValueReporter) {
        missedValueReporter();
      }
      doing = DoingDescription;
      missedValueReporter = [&mf, &resetReporter]() {
        mf.IssueMessage(
          MessageType::WARNING,
          "DESCRIPTION keyword not followed by a value or was followed "
          "by a value that expanded to nothing.");
        resetReporter();
      };
    } else if (args[i] == "HOMEPAGE_URL") {
      if (haveHomepage) {
        mf.IssueMessage(MessageType::FATAL_ERROR,
                        "HOMEPAGE_URL may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveHomepage = true;
      doing = DoingHomepage;
      missedValueReporter = [&mf, &resetReporter]() {
        mf.IssueMessage(
          MessageType::WARNING,
          "HOMEPAGE_URL keyword not followed by a value or was followed "
          "by a value that expanded to nothing.");
        resetReporter();
      };
    } else if (i == 1 && args[i] == "__CMAKE_INJECTED_PROJECT_COMMAND__") {
      injectedProjectCommand = true;
    } else if (doing == DoingVersion) {
      doing = DoingLanguages;
      version = args[i];
      resetReporter();
    } else if (doing == DoingDescription) {
      doing = DoingLanguages;
      description = args[i];
      resetReporter();
    } else if (doing == DoingHomepage) {
      doing = DoingLanguages;
      homepage = args[i];
      resetReporter();
    } else // doing == DoingLanguages
    {
      languages.push_back(args[i]);
    }
  }

  if (missedValueReporter) {
    missedValueReporter();
  }

  if ((haveVersion || haveDescription || haveHomepage) && !haveLanguages &&
      !languages.empty()) {
    mf.IssueMessage(MessageType::FATAL_ERROR,
                    "project with VERSION, DESCRIPTION or HOMEPAGE_URL must "
                    "use LANGUAGES before language names.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
  }
  if (haveLanguages && languages.empty()) {
    languages.emplace_back("NONE");
  }

  cmPolicies::PolicyStatus const cmp0048 =
    mf.GetPolicyStatus(cmPolicies::CMP0048);
  if (haveVersion) {
    // Set project VERSION variables to given values
    if (cmp0048 == cmPolicies::OLD || cmp0048 == cmPolicies::WARN) {
      mf.IssueMessage(MessageType::FATAL_ERROR,
                      "VERSION not allowed unless CMP0048 is set to NEW");
      cmSystemTools::SetFatalErrorOccured();
      return true;
    }

    cmsys::RegularExpression vx(
      R"(^([0-9]+(\.[0-9]+(\.[0-9]+(\.[0-9]+)?)?)?)?$)");
    if (!vx.find(version)) {
      std::string e = R"(VERSION ")" + version + R"(" format invalid.)";
      mf.IssueMessage(MessageType::FATAL_ERROR, e);
      cmSystemTools::SetFatalErrorOccured();
      return true;
    }

    cmPolicies::PolicyStatus const cmp0096 =
      mf.GetPolicyStatus(cmPolicies::CMP0096);

    constexpr std::size_t MAX_VERSION_COMPONENTS = 4u;
    std::string version_string;
    std::array<std::string, MAX_VERSION_COMPONENTS> version_components;

    if (cmp0096 == cmPolicies::OLD || cmp0096 == cmPolicies::WARN) {
      char vb[MAX_VERSION_COMPONENTS]
             [std::numeric_limits<unsigned>::digits10 + 2];
      unsigned v[MAX_VERSION_COMPONENTS] = { 0, 0, 0, 0 };
      const int vc = std::sscanf(version.c_str(), "%u.%u.%u.%u", &v[0], &v[1],
                                 &v[2], &v[3]);
      for (auto i = 0u; i < MAX_VERSION_COMPONENTS; ++i) {
        if (int(i) < vc) {
          std::sprintf(vb[i], "%u", v[i]);
          version_string += &"."[std::size_t(i == 0)];
          version_string += vb[i];
          version_components[i] = vb[i];
        } else {
          vb[i][0] = '\x00';
        }
      }
    } else {
      // The regex above verified that we have a .-separated string of
      // non-negative integer components.  Keep the original string.
      version_string = std::move(version);
      // Split the integer components.
      auto components = cmSystemTools::SplitString(version_string, '.');
      for (auto i = 0u; i < components.size(); ++i) {
        version_components[i] = std::move(components[i]);
      }
    }

    std::string vv;
    vv = projectName + "_VERSION";
    mf.AddDefinition("PROJECT_VERSION", version_string);
    mf.AddDefinition(vv, version_string);
    vv = projectName + "_VERSION_MAJOR";
    mf.AddDefinition("PROJECT_VERSION_MAJOR", version_components[0]);
    mf.AddDefinition(vv, version_components[0]);
    vv = projectName + "_VERSION_MINOR";
    mf.AddDefinition("PROJECT_VERSION_MINOR", version_components[1]);
    mf.AddDefinition(vv, version_components[1]);
    vv = projectName + "_VERSION_PATCH";
    mf.AddDefinition("PROJECT_VERSION_PATCH", version_components[2]);
    mf.AddDefinition(vv, version_components[2]);
    vv = projectName + "_VERSION_TWEAK";
    mf.AddDefinition("PROJECT_VERSION_TWEAK", version_components[3]);
    mf.AddDefinition(vv, version_components[3]);
    // Also, try set top level variables
    TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_VERSION", version_string);
    TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_VERSION_MAJOR",
                            version_components[0]);
    TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_VERSION_MINOR",
                            version_components[1]);
    TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_VERSION_PATCH",
                            version_components[2]);
    TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_VERSION_TWEAK",
                            version_components[3]);
  } else if (cmp0048 != cmPolicies::OLD) {
    // Set project VERSION variables to empty
    std::vector<std::string> vv = { "PROJECT_VERSION",
                                    "PROJECT_VERSION_MAJOR",
                                    "PROJECT_VERSION_MINOR",
                                    "PROJECT_VERSION_PATCH",
                                    "PROJECT_VERSION_TWEAK",
                                    projectName + "_VERSION",
                                    projectName + "_VERSION_MAJOR",
                                    projectName + "_VERSION_MINOR",
                                    projectName + "_VERSION_PATCH",
                                    projectName + "_VERSION_TWEAK" };
    if (mf.IsRootMakefile()) {
      vv.emplace_back("CMAKE_PROJECT_VERSION");
      vv.emplace_back("CMAKE_PROJECT_VERSION_MAJOR");
      vv.emplace_back("CMAKE_PROJECT_VERSION_MINOR");
      vv.emplace_back("CMAKE_PROJECT_VERSION_PATCH");
      vv.emplace_back("CMAKE_PROJECT_VERSION_TWEAK");
    }
    std::string vw;
    for (std::string const& i : vv) {
      const char* const v = mf.GetDefinition(i);
      if (v && *v) {
        if (cmp0048 == cmPolicies::WARN) {
          if (!injectedProjectCommand) {
            vw += "\n  ";
            vw += i;
          }
        } else {
          mf.AddDefinition(i, "");
        }
      }
    }
    if (!vw.empty()) {
      mf.IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0048),
                 "\nThe following variable(s) would be set to empty:", vw));
    }
  }

  mf.AddDefinition("PROJECT_DESCRIPTION", description);
  mf.AddDefinition(projectName + "_DESCRIPTION", description);
  TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_DESCRIPTION", description);

  mf.AddDefinition("PROJECT_HOMEPAGE_URL", homepage);
  mf.AddDefinition(projectName + "_HOMEPAGE_URL", homepage);
  TopLevelCMakeVarCondSet(mf, "CMAKE_PROJECT_HOMEPAGE_URL", homepage);

  if (languages.empty()) {
    // if no language is specified do c and c++
    languages = { "C", "CXX" };
  }
  mf.EnableLanguage(languages, false);

  if (!IncludeByVariable(status, "CMAKE_PROJECT_INCLUDE")) {
    return false;
  }

  if (!IncludeByVariable(status,
                         "CMAKE_PROJECT_" + projectName + "_INCLUDE")) {
    return false;
  }

  return true;
}

static bool IncludeByVariable(cmExecutionStatus& status,
                              const std::string& variable)
{
  cmMakefile& mf = status.GetMakefile();
  const char* const include = mf.GetDefinition(variable);
  if (!include) {
    return true;
  }

  const bool readit = mf.ReadDependentFile(include);
  if (readit) {
    return true;
  }

  if (cmSystemTools::GetFatalErrorOccured()) {
    return true;
  }

  status.SetError(cmStrCat("could not find file:\n  ", include));
  return false;
}

static void TopLevelCMakeVarCondSet(cmMakefile& mf, std::string const& name,
                                    std::string const& value)
{
  // Set the CMAKE_PROJECT_XXX variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one.
  if (!mf.GetDefinition(name) || mf.IsRootMakefile()) {
    mf.AddDefinition(name, value);
    mf.AddCacheDefinition(name, value.c_str(), "Value Computed by CMake",
                          cmStateEnums::STATIC);
  }
}
