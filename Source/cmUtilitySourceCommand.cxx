/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmUtilitySourceCommand.h"

#include <cstring>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmUtilitySourceCommand
bool cmUtilitySourceCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  auto arg = args.begin();

  // The first argument is the cache entry name.
  std::string const& cacheEntry = *arg++;
  const char* cacheValue = status.GetMakefile().GetDefinition(cacheEntry);
  // If it exists already and appears up to date then we are done.  If
  // the string contains "(IntDir)" but that is not the
  // CMAKE_CFG_INTDIR setting then the value is out of date.
  std::string const& intDir =
    status.GetMakefile().GetRequiredDefinition("CMAKE_CFG_INTDIR");

  bool haveCacheValue = false;
  if (status.GetMakefile().IsOn("CMAKE_CROSSCOMPILING")) {
    haveCacheValue = (cacheValue != nullptr);
    if (!haveCacheValue) {
      std::string msg = cmStrCat(
        "UTILITY_SOURCE is used in cross compiling mode for ", cacheEntry,
        ". If your intention is to run this executable, you need to "
        "preload the cache with the full path to a version of that "
        "program, which runs on this build machine.");
      cmSystemTools::Message(msg, "Warning");
    }
  } else {
    cmState* state = status.GetMakefile().GetState();
    haveCacheValue = (cacheValue &&
                      (strstr(cacheValue, "(IntDir)") == nullptr ||
                       (intDir == "$(IntDir)")) &&
                      (state->GetCacheMajorVersion() != 0 &&
                       state->GetCacheMinorVersion() != 0));
  }

  if (haveCacheValue) {
    return true;
  }

  // The second argument is the utility's executable name, which will be
  // needed later.
  std::string const& utilityName = *arg++;

  // The third argument specifies the relative directory of the source
  // of the utility.
  std::string const& relativeSource = *arg++;
  std::string utilitySource = status.GetMakefile().GetCurrentSourceDirectory();
  utilitySource = utilitySource + "/" + relativeSource;

  // If the directory doesn't exist, the source has not been included.
  if (!cmSystemTools::FileExists(utilitySource)) {
    return true;
  }

  // Make sure all the files exist in the source directory.
  while (arg != args.end()) {
    std::string file = utilitySource + "/" + *arg++;
    if (!cmSystemTools::FileExists(file)) {
      return true;
    }
  }

  // The source exists.
  const std::string& cmakeCFGout =
    status.GetMakefile().GetRequiredDefinition("CMAKE_CFG_INTDIR");
  std::string utilityDirectory =
    status.GetMakefile().GetCurrentBinaryDirectory();
  std::string exePath;
  if (status.GetMakefile().GetDefinition("EXECUTABLE_OUTPUT_PATH")) {
    exePath = status.GetMakefile().GetDefinition("EXECUTABLE_OUTPUT_PATH");
  }
  if (!exePath.empty()) {
    utilityDirectory = exePath;
  } else {
    utilityDirectory += "/" + relativeSource;
  }

  // Construct the cache entry for the executable's location.
  std::string utilityExecutable = utilityDirectory + "/" + cmakeCFGout + "/" +
    utilityName +
    status.GetMakefile().GetDefinition("CMAKE_EXECUTABLE_SUFFIX");

  // make sure we remove any /./ in the name
  cmSystemTools::ReplaceString(utilityExecutable, "/./", "/");

  // Enter the value into the cache.
  status.GetMakefile().AddCacheDefinition(
    cacheEntry, utilityExecutable.c_str(), "Path to an internal program.",
    cmStateEnums::FILEPATH);
  // add a value into the cache that maps from the
  // full path to the name of the project
  cmSystemTools::ConvertToUnixSlashes(utilityExecutable);
  status.GetMakefile().AddCacheDefinition(
    utilityExecutable, utilityName.c_str(), "Executable to project name.",
    cmStateEnums::INTERNAL);

  return true;
}
