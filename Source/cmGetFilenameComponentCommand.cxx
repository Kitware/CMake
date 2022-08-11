/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGetFilenameComponentCommand.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

// cmGetFilenameComponentCommand
bool cmGetFilenameComponentCommand(std::vector<std::string> const& args,
                                   cmExecutionStatus& status)
{
  if (args.size() < 3) {
    status.SetError("called with incorrect number of arguments");
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  // Check and see if the value has been stored in the cache
  // already, if so use that value
  if (args.size() >= 4 && args.back() == "CACHE") {
    cmValue cacheValue = status.GetMakefile().GetDefinition(args.front());
    if (cacheValue && !cmIsNOTFOUND(*cacheValue)) {
      return true;
    }
  }

  std::string result;
  std::string filename = args[1];
  if (filename.find("[HKEY") != std::string::npos) {
    // Check the registry as the target application would view it.
    cmSystemTools::KeyWOW64 view = cmSystemTools::KeyWOW64_32;
    cmSystemTools::KeyWOW64 other_view = cmSystemTools::KeyWOW64_64;
    if (status.GetMakefile().PlatformIs64Bit()) {
      view = cmSystemTools::KeyWOW64_64;
      other_view = cmSystemTools::KeyWOW64_32;
    }
    cmSystemTools::ExpandRegistryValues(filename, view);
    if (filename.find("/registry") != std::string::npos) {
      std::string other = args[1];
      cmSystemTools::ExpandRegistryValues(other, other_view);
      if (other.find("/registry") == std::string::npos) {
        filename = other;
      }
    }
  }
  std::string storeArgs;
  std::string programArgs;
  if (args[2] == "DIRECTORY" || args[2] == "PATH") {
    result = cmSystemTools::GetFilenamePath(filename);
  } else if (args[2] == "NAME") {
    result = cmSystemTools::GetFilenameName(filename);
  } else if (args[2] == "PROGRAM") {
    for (unsigned int i = 2; i < args.size(); ++i) {
      if (args[i] == "PROGRAM_ARGS") {
        i++;
        if (i < args.size()) {
          storeArgs = args[i];
        }
      }
    }

    // First assume the path to the program was specified with no
    // arguments and with no quoting or escaping for spaces.
    // Only bother doing this if there is non-whitespace.
    if (!cmTrimWhitespace(filename).empty()) {
      result = cmSystemTools::FindProgram(filename);
    }

    // If that failed then assume a command-line string was given
    // and split the program part from the rest of the arguments.
    if (result.empty()) {
      std::string program;
      if (cmSystemTools::SplitProgramFromArgs(filename, program,
                                              programArgs)) {
        if (cmSystemTools::FileExists(program)) {
          result = program;
        } else {
          result = cmSystemTools::FindProgram(program);
        }
      }
      if (result.empty()) {
        programArgs.clear();
      }
    }
  } else if (args[2] == "EXT") {
    result = cmSystemTools::GetFilenameExtension(filename);
  } else if (args[2] == "NAME_WE") {
    result = cmSystemTools::GetFilenameWithoutExtension(filename);
  } else if (args[2] == "LAST_EXT") {
    result = cmSystemTools::GetFilenameLastExtension(filename);
  } else if (args[2] == "NAME_WLE") {
    result = cmSystemTools::GetFilenameWithoutLastExtension(filename);
  } else if (args[2] == "ABSOLUTE" || args[2] == "REALPATH") {
    // If the path given is relative, evaluate it relative to the
    // current source directory unless the user passes a different
    // base directory.
    std::string baseDir = status.GetMakefile().GetCurrentSourceDirectory();
    for (unsigned int i = 3; i < args.size(); ++i) {
      if (args[i] == "BASE_DIR") {
        ++i;
        if (i < args.size()) {
          baseDir = args[i];
        }
      }
    }
    // Collapse the path to its simplest form.
    result = cmSystemTools::CollapseFullPath(filename, baseDir);
    if (args[2] == "REALPATH") {
      // Resolve symlinks if possible
      result = cmSystemTools::GetRealPath(result);
    }
  } else {
    std::string err = "unknown component " + args[2];
    status.SetError(err);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (args.size() >= 4 && args.back() == "CACHE") {
    if (!programArgs.empty() && !storeArgs.empty()) {
      status.GetMakefile().AddCacheDefinition(
        storeArgs, programArgs, "",
        args[2] == "PATH" ? cmStateEnums::FILEPATH : cmStateEnums::STRING);
    }
    status.GetMakefile().AddCacheDefinition(
      args.front(), result, "",
      args[2] == "PATH" ? cmStateEnums::FILEPATH : cmStateEnums::STRING);
  } else {
    if (!programArgs.empty() && !storeArgs.empty()) {
      status.GetMakefile().AddDefinition(storeArgs, programArgs);
    }
    status.GetMakefile().AddDefinition(args.front(), result);
  }

  return true;
}
