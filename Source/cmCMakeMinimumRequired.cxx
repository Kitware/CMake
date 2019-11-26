/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakeMinimumRequired.h"

#include <cstdio>
#include <sstream>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"
#include "cmVersion.h"

namespace {
bool EnforceUnknownArguments(std::string const& version_max,
                             std::vector<std::string> const& unknown_arguments,
                             cmExecutionStatus& status);
}

// cmCMakeMinimumRequired
bool cmCMakeMinimumRequired(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  // Process arguments.
  std::string version_string;
  bool doing_version = false;
  std::vector<std::string> unknown_arguments;
  for (std::string const& arg : args) {
    if (arg == "VERSION") {
      doing_version = true;
    } else if (arg == "FATAL_ERROR") {
      if (doing_version) {
        status.SetError("called with no value for VERSION.");
        return false;
      }
      doing_version = false;
    } else if (doing_version) {
      doing_version = false;
      version_string = arg;
    } else {
      unknown_arguments.push_back(arg);
    }
  }
  if (doing_version) {
    status.SetError("called with no value for VERSION.");
    return false;
  }

  // Make sure there was a version to check.
  if (version_string.empty()) {
    return EnforceUnknownArguments(std::string(), unknown_arguments, status);
  }

  // Separate the <min> version and any trailing ...<max> component.
  std::string::size_type const dd = version_string.find("...");
  std::string const version_min = version_string.substr(0, dd);
  std::string const version_max = dd != std::string::npos
    ? version_string.substr(dd + 3, std::string::npos)
    : std::string();
  if (dd != std::string::npos &&
      (version_min.empty() || version_max.empty())) {
    std::ostringstream e;
    e << "VERSION \"" << version_string
      << R"(" does not have a version on both sides of "...".)";
    status.SetError(e.str());
    return false;
  }

  // Save the required version string.
  status.GetMakefile().AddDefinition("CMAKE_MINIMUM_REQUIRED_VERSION",
                                     version_min);

  // Get the current version number.
  unsigned int current_major = cmVersion::GetMajorVersion();
  unsigned int current_minor = cmVersion::GetMinorVersion();
  unsigned int current_patch = cmVersion::GetPatchVersion();
  unsigned int current_tweak = cmVersion::GetTweakVersion();

  // Parse at least two components of the version number.
  // Use zero for those not specified.
  unsigned int required_major = 0;
  unsigned int required_minor = 0;
  unsigned int required_patch = 0;
  unsigned int required_tweak = 0;
  if (sscanf(version_min.c_str(), "%u.%u.%u.%u", &required_major,
             &required_minor, &required_patch, &required_tweak) < 2) {
    std::ostringstream e;
    e << "could not parse VERSION \"" << version_min << "\".";
    status.SetError(e.str());
    return false;
  }

  // Compare the version numbers.
  if ((current_major < required_major) ||
      (current_major == required_major && current_minor < required_minor) ||
      (current_major == required_major && current_minor == required_minor &&
       current_patch < required_patch) ||
      (current_major == required_major && current_minor == required_minor &&
       current_patch == required_patch && current_tweak < required_tweak)) {
    // The current version is too low.
    std::ostringstream e;
    e << "CMake " << version_min
      << " or higher is required.  You are running version "
      << cmVersion::GetCMakeVersion();
    status.GetMakefile().IssueMessage(MessageType::FATAL_ERROR, e.str());
    cmSystemTools::SetFatalErrorOccured();
    return true;
  }

  // The version is not from the future, so enforce unknown arguments.
  if (!EnforceUnknownArguments(version_max, unknown_arguments, status)) {
    return false;
  }

  if (required_major < 2 || (required_major == 2 && required_minor < 4)) {
    status.GetMakefile().IssueMessage(
      MessageType::AUTHOR_WARNING,
      "Compatibility with CMake < 2.4 is not supported by CMake >= 3.0.");
    status.GetMakefile().SetPolicyVersion("2.4", version_max);
  } else {
    status.GetMakefile().SetPolicyVersion(version_min, version_max);
  }

  return true;
}

namespace {
bool EnforceUnknownArguments(std::string const& version_max,
                             std::vector<std::string> const& unknown_arguments,
                             cmExecutionStatus& status)
{
  if (unknown_arguments.empty()) {
    return true;
  }

  // Consider the max version if at least two components were given.
  unsigned int max_major = 0;
  unsigned int max_minor = 0;
  unsigned int max_patch = 0;
  unsigned int max_tweak = 0;
  if (sscanf(version_max.c_str(), "%u.%u.%u.%u", &max_major, &max_minor,
             &max_patch, &max_tweak) >= 2) {
    unsigned int current_major = cmVersion::GetMajorVersion();
    unsigned int current_minor = cmVersion::GetMinorVersion();
    unsigned int current_patch = cmVersion::GetPatchVersion();
    unsigned int current_tweak = cmVersion::GetTweakVersion();

    if ((current_major < max_major) ||
        (current_major == max_major && current_minor < max_minor) ||
        (current_major == max_major && current_minor == max_minor &&
         current_patch < max_patch) ||
        (current_major == max_major && current_minor == max_minor &&
         current_patch == max_patch && current_tweak < max_tweak)) {
      // A ...<max> version was given that is larger than the current
      // version of CMake, so tolerate unknown arguments.
      return true;
    }
  }

  std::ostringstream e;
  e << "called with unknown argument \"" << unknown_arguments[0] << "\".";
  status.SetError(e.str());
  return false;
}
}
