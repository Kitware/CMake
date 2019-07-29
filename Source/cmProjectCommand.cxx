/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmProjectCommand.h"

#include "cmsys/RegularExpression.hxx"
#include <array>
#include <cstdio>
#include <functional>
#include <limits>
#include <sstream>
#include <utility>

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

// cmProjectCommand
bool cmProjectCommand::InitialPass(std::vector<std::string> const& args,
                                   cmExecutionStatus&)
{
  if (args.empty()) {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
  }

  if (!this->IncludeByVariable("CMAKE_PROJECT_INCLUDE_BEFORE")) {
    return false;
  }

  std::string const& projectName = args[0];

  this->Makefile->SetProjectName(projectName);

  this->Makefile->AddCacheDefinition(
    projectName + "_BINARY_DIR",
    this->Makefile->GetCurrentBinaryDirectory().c_str(),
    "Value Computed by CMake", cmStateEnums::STATIC);
  this->Makefile->AddCacheDefinition(
    projectName + "_SOURCE_DIR",
    this->Makefile->GetCurrentSourceDirectory().c_str(),
    "Value Computed by CMake", cmStateEnums::STATIC);

  this->Makefile->AddDefinition("PROJECT_BINARY_DIR",
                                this->Makefile->GetCurrentBinaryDirectory());
  this->Makefile->AddDefinition("PROJECT_SOURCE_DIR",
                                this->Makefile->GetCurrentSourceDirectory());

  this->Makefile->AddDefinition("PROJECT_NAME", projectName);

  // Set the CMAKE_PROJECT_NAME variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one, so that
  // CMAKE_PROJECT_NAME will match PROJECT_NAME, and cmake --build
  // will work.
  if (!this->Makefile->GetDefinition("CMAKE_PROJECT_NAME") ||
      (this->Makefile->IsRootMakefile())) {
    this->Makefile->AddDefinition("CMAKE_PROJECT_NAME", projectName);
    this->Makefile->AddCacheDefinition(
      "CMAKE_PROJECT_NAME", projectName.c_str(), "Value Computed by CMake",
      cmStateEnums::STATIC);
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
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
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
        std::string msg =
          "the following parameters must be specified after LANGUAGES "
          "keyword: ";
        msg += cmJoin(languages, ", ");
        msg += '.';
        this->Makefile->IssueMessage(MessageType::WARNING, msg);
      }
    } else if (args[i] == "VERSION") {
      if (haveVersion) {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                     "VERSION may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveVersion = true;
      if (missedValueReporter) {
        missedValueReporter();
      }
      doing = DoingVersion;
      missedValueReporter = [this, &resetReporter]() {
        this->Makefile->IssueMessage(
          MessageType::WARNING,
          "VERSION keyword not followed by a value or was followed by a "
          "value that expanded to nothing.");
        resetReporter();
      };
    } else if (args[i] == "DESCRIPTION") {
      if (haveDescription) {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          "DESCRIPTION may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveDescription = true;
      if (missedValueReporter) {
        missedValueReporter();
      }
      doing = DoingDescription;
      missedValueReporter = [this, &resetReporter]() {
        this->Makefile->IssueMessage(
          MessageType::WARNING,
          "DESCRIPTION keyword not followed by a value or was followed "
          "by a value that expanded to nothing.");
        resetReporter();
      };
    } else if (args[i] == "HOMEPAGE_URL") {
      if (haveHomepage) {
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          "HOMEPAGE_URL may be specified at most once.");
        cmSystemTools::SetFatalErrorOccured();
        return true;
      }
      haveHomepage = true;
      doing = DoingHomepage;
      missedValueReporter = [this, &resetReporter]() {
        this->Makefile->IssueMessage(
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
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "project with VERSION, DESCRIPTION or HOMEPAGE_URL must "
      "use LANGUAGES before language names.");
    cmSystemTools::SetFatalErrorOccured();
    return true;
  }
  if (haveLanguages && languages.empty()) {
    languages.emplace_back("NONE");
  }

  cmPolicies::PolicyStatus const cmp0048 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0048);
  if (haveVersion) {
    // Set project VERSION variables to given values
    if (cmp0048 == cmPolicies::OLD || cmp0048 == cmPolicies::WARN) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "VERSION not allowed unless CMP0048 is set to NEW");
      cmSystemTools::SetFatalErrorOccured();
      return true;
    }

    cmsys::RegularExpression vx(
      R"(^([0-9]+(\.[0-9]+(\.[0-9]+(\.[0-9]+)?)?)?)?$)");
    if (!vx.find(version)) {
      std::string e = R"(VERSION ")" + version + R"(" format invalid.)";
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      cmSystemTools::SetFatalErrorOccured();
      return true;
    }

    cmPolicies::PolicyStatus const cmp0096 =
      this->Makefile->GetPolicyStatus(cmPolicies::CMP0096);

    constexpr std::size_t MAX_VERSION_COMPONENTS = 4u;
    std::string version_string;
    std::array<std::string, MAX_VERSION_COMPONENTS> version_components;

    if (cmp0096 == cmPolicies::OLD || cmp0096 == cmPolicies::WARN) {
      char vb[MAX_VERSION_COMPONENTS][std::numeric_limits<unsigned>::digits10];
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
    this->Makefile->AddDefinition("PROJECT_VERSION", version_string);
    this->Makefile->AddDefinition(vv, version_string);
    vv = projectName + "_VERSION_MAJOR";
    this->Makefile->AddDefinition("PROJECT_VERSION_MAJOR",
                                  version_components[0]);
    this->Makefile->AddDefinition(vv, version_components[0]);
    vv = projectName + "_VERSION_MINOR";
    this->Makefile->AddDefinition("PROJECT_VERSION_MINOR",
                                  version_components[1]);
    this->Makefile->AddDefinition(vv, version_components[1]);
    vv = projectName + "_VERSION_PATCH";
    this->Makefile->AddDefinition("PROJECT_VERSION_PATCH",
                                  version_components[2]);
    this->Makefile->AddDefinition(vv, version_components[2]);
    vv = projectName + "_VERSION_TWEAK";
    this->Makefile->AddDefinition("PROJECT_VERSION_TWEAK",
                                  version_components[3]);
    this->Makefile->AddDefinition(vv, version_components[3]);
    // Also, try set top level variables
    TopLevelCMakeVarCondSet("CMAKE_PROJECT_VERSION", version_string.c_str());
    TopLevelCMakeVarCondSet("CMAKE_PROJECT_VERSION_MAJOR",
                            version_components[0].c_str());
    TopLevelCMakeVarCondSet("CMAKE_PROJECT_VERSION_MINOR",
                            version_components[1].c_str());
    TopLevelCMakeVarCondSet("CMAKE_PROJECT_VERSION_PATCH",
                            version_components[2].c_str());
    TopLevelCMakeVarCondSet("CMAKE_PROJECT_VERSION_TWEAK",
                            version_components[3].c_str());
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
    if (this->Makefile->IsRootMakefile()) {
      vv.emplace_back("CMAKE_PROJECT_VERSION");
      vv.emplace_back("CMAKE_PROJECT_VERSION_MAJOR");
      vv.emplace_back("CMAKE_PROJECT_VERSION_MINOR");
      vv.emplace_back("CMAKE_PROJECT_VERSION_PATCH");
      vv.emplace_back("CMAKE_PROJECT_VERSION_TWEAK");
    }
    std::string vw;
    for (std::string const& i : vv) {
      const char* const v = this->Makefile->GetDefinition(i);
      if (v && *v) {
        if (cmp0048 == cmPolicies::WARN) {
          if (!injectedProjectCommand) {
            vw += "\n  ";
            vw += i;
          }
        } else {
          this->Makefile->AddDefinition(i, "");
        }
      }
    }
    if (!vw.empty()) {
      std::ostringstream w;
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0048)
        << "\nThe following variable(s) would be set to empty:" << vw;
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
    }
  }

  this->Makefile->AddDefinition("PROJECT_DESCRIPTION", description);
  this->Makefile->AddDefinition(projectName + "_DESCRIPTION", description);
  TopLevelCMakeVarCondSet("CMAKE_PROJECT_DESCRIPTION", description.c_str());

  this->Makefile->AddDefinition("PROJECT_HOMEPAGE_URL", homepage);
  this->Makefile->AddDefinition(projectName + "_HOMEPAGE_URL", homepage);
  TopLevelCMakeVarCondSet("CMAKE_PROJECT_HOMEPAGE_URL", homepage.c_str());

  if (languages.empty()) {
    // if no language is specified do c and c++
    languages = { "C", "CXX" };
  }
  this->Makefile->EnableLanguage(languages, false);

  if (!this->IncludeByVariable("CMAKE_PROJECT_INCLUDE")) {
    return false;
  }

  if (!this->IncludeByVariable("CMAKE_PROJECT_" + projectName + "_INCLUDE")) {
    return false;
  }

  return true;
}

bool cmProjectCommand::IncludeByVariable(const std::string& variable)
{
  const char* const include = this->Makefile->GetDefinition(variable);
  if (!include) {
    return true;
  }

  const bool readit = this->Makefile->ReadDependentFile(include);
  if (readit) {
    return true;
  }

  if (cmSystemTools::GetFatalErrorOccured()) {
    return true;
  }

  std::string m = "could not find file:\n"
                  "  ";
  m += include;
  this->SetError(m);
  return false;
}

void cmProjectCommand::TopLevelCMakeVarCondSet(const char* const name,
                                               const char* const value)
{
  // Set the CMAKE_PROJECT_XXX variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one.
  if (!this->Makefile->GetDefinition(name) ||
      (this->Makefile->IsRootMakefile())) {
    this->Makefile->AddDefinition(name, value);
    this->Makefile->AddCacheDefinition(name, value, "Value Computed by CMake",
                                       cmStateEnums::STATIC);
  }
}
