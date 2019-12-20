/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFindBase.h"

#include <cstddef>
#include <deque>
#include <map>
#include <utility>

#include <cmext/algorithm>

#include "cmMakefile.h"
#include "cmRange.h"
#include "cmSearchPath.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

cmFindBase::cmFindBase(cmExecutionStatus& status)
  : cmFindCommon(status)
{
  this->AlreadyInCache = false;
  this->AlreadyInCacheWithoutMetaInfo = false;
  this->NamesPerDir = false;
  this->NamesPerDirAllowed = false;
}

bool cmFindBase::ParseArguments(std::vector<std::string> const& argsIn)
{
  if (argsIn.size() < 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  // copy argsIn into args so it can be modified,
  // in the process extract the DOC "documentation"
  size_t size = argsIn.size();
  std::vector<std::string> args;
  bool foundDoc = false;
  for (unsigned int j = 0; j < size; ++j) {
    if (foundDoc || argsIn[j] != "DOC") {
      if (argsIn[j] == "ENV") {
        if (j + 1 < size) {
          j++;
          cmSystemTools::GetPath(args, argsIn[j].c_str());
        }
      } else {
        args.push_back(argsIn[j]);
      }
    } else {
      if (j + 1 < size) {
        foundDoc = true;
        this->VariableDocumentation = argsIn[j + 1];
        j++;
        if (j >= size) {
          break;
        }
      }
    }
  }
  if (args.size() < 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }
  this->VariableName = args[0];
  if (this->CheckForVariableInCache()) {
    this->AlreadyInCache = true;
    return true;
  }
  this->AlreadyInCache = false;

  // Find what search path locations have been enabled/disable
  this->SelectDefaultSearchModes();

  // Find the current root path mode.
  this->SelectDefaultRootPathMode();

  // Find the current bundle/framework search policy.
  this->SelectDefaultMacMode();

  bool newStyle = false;
  enum Doing
  {
    DoingNone,
    DoingNames,
    DoingPaths,
    DoingPathSuffixes,
    DoingHints
  };
  Doing doing = DoingNames; // assume it starts with a name
  for (unsigned int j = 1; j < args.size(); ++j) {
    if (args[j] == "NAMES") {
      doing = DoingNames;
      newStyle = true;
    } else if (args[j] == "PATHS") {
      doing = DoingPaths;
      newStyle = true;
    } else if (args[j] == "HINTS") {
      doing = DoingHints;
      newStyle = true;
    } else if (args[j] == "PATH_SUFFIXES") {
      doing = DoingPathSuffixes;
      newStyle = true;
    } else if (args[j] == "NAMES_PER_DIR") {
      doing = DoingNone;
      if (this->NamesPerDirAllowed) {
        this->NamesPerDir = true;
      } else {
        this->SetError("does not support NAMES_PER_DIR");
        return false;
      }
    } else if (args[j] == "NO_SYSTEM_PATH") {
      doing = DoingNone;
      this->NoDefaultPath = true;
    } else if (this->CheckCommonArgument(args[j])) {
      doing = DoingNone;
    } else {
      // Some common arguments were accidentally supported by CMake
      // 2.4 and 2.6.0 in the short-hand form of the command, so we
      // must support it even though it is not documented.
      if (doing == DoingNames) {
        this->Names.push_back(args[j]);
      } else if (doing == DoingPaths) {
        this->UserGuessArgs.push_back(args[j]);
      } else if (doing == DoingHints) {
        this->UserHintsArgs.push_back(args[j]);
      } else if (doing == DoingPathSuffixes) {
        this->AddPathSuffix(args[j]);
      }
    }
  }

  if (this->VariableDocumentation.empty()) {
    this->VariableDocumentation = "Where can ";
    if (this->Names.empty()) {
      this->VariableDocumentation += "the (unknown) library be found";
    } else if (this->Names.size() == 1) {
      this->VariableDocumentation +=
        "the " + this->Names.front() + " library be found";
    } else {
      this->VariableDocumentation += "one of the ";
      this->VariableDocumentation +=
        cmJoin(cmMakeRange(this->Names).retreat(1), ", ");
      this->VariableDocumentation +=
        " or " + this->Names.back() + " libraries be found";
    }
  }

  // look for old style
  // FIND_*(VAR name path1 path2 ...)
  if (!newStyle && !this->Names.empty()) {
    // All the short-hand arguments have been recorded as names.
    std::vector<std::string> shortArgs = this->Names;
    this->Names.clear(); // clear out any values in Names
    this->Names.push_back(shortArgs[0]);
    cm::append(this->UserGuessArgs, shortArgs.begin() + 1, shortArgs.end());
  }
  this->ExpandPaths();

  this->ComputeFinalPaths();

  return true;
}

void cmFindBase::ExpandPaths()
{
  if (!this->NoDefaultPath) {
    if (!this->NoPackageRootPath) {
      this->FillPackageRootPath();
    }
    if (!this->NoCMakePath) {
      this->FillCMakeVariablePath();
    }
    if (!this->NoCMakeEnvironmentPath) {
      this->FillCMakeEnvironmentPath();
    }
  }
  this->FillUserHintsPath();
  if (!this->NoDefaultPath) {
    if (!this->NoSystemEnvironmentPath) {
      this->FillSystemEnvironmentPath();
    }
    if (!this->NoCMakeSystemPath) {
      this->FillCMakeSystemVariablePath();
    }
  }
  this->FillUserGuessPath();
}

void cmFindBase::FillCMakeEnvironmentPath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::CMakeEnvironment];

  // Add CMAKE_*_PATH environment variables
  std::string var = cmStrCat("CMAKE_", this->CMakePathName, "_PATH");
  paths.AddEnvPrefixPath("CMAKE_PREFIX_PATH");
  paths.AddEnvPath(var);

  if (this->CMakePathName == "PROGRAM") {
    paths.AddEnvPath("CMAKE_APPBUNDLE_PATH");
  } else {
    paths.AddEnvPath("CMAKE_FRAMEWORK_PATH");
  }
  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillPackageRootPath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::PackageRoot];

  // Add the PACKAGE_ROOT_PATH from each enclosing find_package call.
  for (std::vector<std::string> const& pkgPaths :
       cmReverseRange(this->Makefile->FindPackageRootPathStack)) {
    paths.AddPrefixPaths(pkgPaths);
  }

  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillCMakeVariablePath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::CMake];

  // Add CMake variables of the same name as the previous environment
  // variables CMAKE_*_PATH to be used most of the time with -D
  // command line options
  std::string var = cmStrCat("CMAKE_", this->CMakePathName, "_PATH");
  paths.AddCMakePrefixPath("CMAKE_PREFIX_PATH");
  paths.AddCMakePath(var);

  if (this->CMakePathName == "PROGRAM") {
    paths.AddCMakePath("CMAKE_APPBUNDLE_PATH");
  } else {
    paths.AddCMakePath("CMAKE_FRAMEWORK_PATH");
  }
  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillSystemEnvironmentPath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::SystemEnvironment];

  // Add LIB or INCLUDE
  if (!this->EnvironmentPath.empty()) {
    paths.AddEnvPath(this->EnvironmentPath);
#if defined(_WIN32) || defined(__CYGWIN__)
    paths.AddEnvPrefixPath("PATH", true);
#endif
  }
  // Add PATH
  paths.AddEnvPath("PATH");
  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillCMakeSystemVariablePath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::CMakeSystem];

  std::string var = cmStrCat("CMAKE_SYSTEM_", this->CMakePathName, "_PATH");
  paths.AddCMakePrefixPath("CMAKE_SYSTEM_PREFIX_PATH");
  paths.AddCMakePath(var);

  if (this->CMakePathName == "PROGRAM") {
    paths.AddCMakePath("CMAKE_SYSTEM_APPBUNDLE_PATH");
  } else {
    paths.AddCMakePath("CMAKE_SYSTEM_FRAMEWORK_PATH");
  }
  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillUserHintsPath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::Hints];

  for (std::string const& p : this->UserHintsArgs) {
    paths.AddUserPath(p);
  }
  paths.AddSuffixes(this->SearchPathSuffixes);
}

void cmFindBase::FillUserGuessPath()
{
  cmSearchPath& paths = this->LabeledPaths[PathLabel::Guess];

  for (std::string const& p : this->UserGuessArgs) {
    paths.AddUserPath(p);
  }
  paths.AddSuffixes(this->SearchPathSuffixes);
}

bool cmFindBase::CheckForVariableInCache()
{
  if (const char* cacheValue =
        this->Makefile->GetDefinition(this->VariableName)) {
    cmState* state = this->Makefile->GetState();
    const char* cacheEntry = state->GetCacheEntryValue(this->VariableName);
    bool found = !cmIsNOTFOUND(cacheValue);
    bool cached = cacheEntry != nullptr;
    if (found) {
      // If the user specifies the entry on the command line without a
      // type we should add the type and docstring but keep the
      // original value.  Tell the subclass implementations to do
      // this.
      if (cached &&
          state->GetCacheEntryType(this->VariableName) ==
            cmStateEnums::UNINITIALIZED) {
        this->AlreadyInCacheWithoutMetaInfo = true;
      }
      return true;
    }
    if (cached) {
      const char* hs =
        state->GetCacheEntryProperty(this->VariableName, "HELPSTRING");
      this->VariableDocumentation = hs ? hs : "(none)";
    }
  }
  return false;
}

cmFindBaseDebugState::cmFindBaseDebugState(std::string commandName,
                                           cmFindBase const* findBase)
  : FindCommand(findBase)
  , CommandName(std::move(commandName))
{
}

cmFindBaseDebugState::~cmFindBaseDebugState()
{
  if (this->FindCommand->DebugMode) {
    std::string buffer =
      cmStrCat(this->CommandName, " called with the following settings:\n");
    buffer += cmStrCat("  VAR: ", this->FindCommand->VariableName, "\n");
    buffer += cmStrCat(
      "  NAMES: ", cmWrap("\"", this->FindCommand->Names, "\"", "\n         "),
      "\n");
    buffer += cmStrCat(
      "  Documentation: ", this->FindCommand->VariableDocumentation, "\n");
    buffer += "  Framework\n";
    buffer += cmStrCat("    Only Search Frameworks: ",
                       this->FindCommand->SearchFrameworkOnly, "\n");

    buffer += cmStrCat("    Search Frameworks Last: ",
                       this->FindCommand->SearchFrameworkLast, "\n");
    buffer += cmStrCat("    Search Frameworks First: ",
                       this->FindCommand->SearchFrameworkFirst, "\n");
    buffer += "  AppBundle\n";
    buffer += cmStrCat("    Only Search AppBundle: ",
                       this->FindCommand->SearchAppBundleOnly, "\n");
    buffer += cmStrCat("    Search AppBundle Last: ",
                       this->FindCommand->SearchAppBundleLast, "\n");
    buffer += cmStrCat("    Search AppBundle First: ",
                       this->FindCommand->SearchAppBundleFirst, "\n");

    if (this->FindCommand->NoDefaultPath) {
      buffer += "  NO_DEFAULT_PATH Enabled\n";
    } else {
      buffer += cmStrCat(
        "  CMAKE_FIND_USE_CMAKE_PATH: ", !this->FindCommand->NoCMakePath, "\n",
        "  CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH: ",
        !this->FindCommand->NoCMakeEnvironmentPath, "\n",
        "  CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH: ",
        !this->FindCommand->NoSystemEnvironmentPath, "\n",
        "  CMAKE_FIND_USE_CMAKE_SYSTEM_PATH: ",
        !this->FindCommand->NoCMakeSystemPath, "\n");
    }

    buffer +=
      cmStrCat(this->CommandName, " considered the following locations:\n");
    for (auto const& state : this->FailedSearchLocations) {
      std::string path = cmStrCat("  ", state.path);
      if (!state.regexName.empty()) {
        path = cmStrCat(path, "/", state.regexName);
      }
      buffer += cmStrCat(path, "\n");
    }

    if (!this->FoundSearchLocation.path.empty()) {
      buffer += cmStrCat("The item was found at\n  ",
                         this->FoundSearchLocation.path, "\n");
    } else {
      buffer += "The item was not found.\n";
    }

    this->FindCommand->DebugMessage(buffer);
  }
}

void cmFindBaseDebugState::FoundAt(std::string const& path,
                                   std::string regexName)
{
  if (this->FindCommand->DebugMode) {
    this->FoundSearchLocation = DebugLibState{ std::move(regexName), path };
  }
}

void cmFindBaseDebugState::FailedAt(std::string const& path,
                                    std::string regexName)
{
  if (this->FindCommand->DebugMode) {
    this->FailedSearchLocations.emplace_back(std::move(regexName), path);
  }
}
