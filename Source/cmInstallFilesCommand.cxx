/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallFilesCommand.h"

#include <cm/memory>

#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

class cmListFileBacktrace;

static std::string FindInstallSource(cmMakefile& makefile, const char* name);
static void CreateInstallGenerator(cmMakefile& makefile,
                                   std::string const& dest,
                                   std::vector<std::string> const& files);
static void FinalAction(cmMakefile& makefile, std::string const& dest,
                        std::vector<std::string> const& args);

bool cmInstallFilesCommand(std::vector<std::string> const& args,
                           cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();

  // Enable the install target.
  mf.GetGlobalGenerator()->EnableInstallTarget();

  std::string const& dest = args[0];

  if ((args.size() > 1) && (args[1] == "FILES")) {
    std::vector<std::string> files;
    for (std::string const& arg : cmMakeRange(args).advance(2)) {
      // Find the source location for each file listed.
      files.push_back(FindInstallSource(mf, arg.c_str()));
    }
    CreateInstallGenerator(mf, dest, files);
  } else {
    std::vector<std::string> finalArgs(args.begin() + 1, args.end());
    mf.AddGeneratorAction(
      [dest, finalArgs](cmLocalGenerator& lg, const cmListFileBacktrace&) {
        FinalAction(*lg.GetMakefile(), dest, finalArgs);
      });
  }

  mf.GetGlobalGenerator()->AddInstallComponent(
    mf.GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME"));

  return true;
}

static void FinalAction(cmMakefile& makefile, std::string const& dest,
                        std::vector<std::string> const& args)
{
  std::string testf;
  std::string const& ext = args[0];
  std::vector<std::string> installFiles;

  // two different options
  if (args.size() > 1) {
    // now put the files into the list
    auto s = args.begin();
    ++s;
    // for each argument, get the files
    for (; s != args.end(); ++s) {
      // replace any variables
      std::string const& temps = *s;
      if (!cmSystemTools::GetFilenamePath(temps).empty()) {
        testf = cmSystemTools::GetFilenamePath(temps) + "/" +
          cmSystemTools::GetFilenameWithoutLastExtension(temps) + ext;
      } else {
        testf = cmSystemTools::GetFilenameWithoutLastExtension(temps) + ext;
      }

      // add to the result
      installFiles.push_back(FindInstallSource(makefile, testf.c_str()));
    }
  } else // reg exp list
  {
    std::vector<std::string> files;
    std::string const& regex = args[0];
    cmSystemTools::Glob(makefile.GetCurrentSourceDirectory(), regex, files);

    auto s = files.begin();
    // for each argument, get the files
    for (; s != files.end(); ++s) {
      installFiles.push_back(FindInstallSource(makefile, s->c_str()));
    }
  }

  CreateInstallGenerator(makefile, dest, installFiles);
}

static void CreateInstallGenerator(cmMakefile& makefile,
                                   std::string const& dest,
                                   std::vector<std::string> const& files)
{
  // Construct the destination.  This command always installs under
  // the prefix.  We skip the leading slash given by the user.
  std::string destination = dest.substr(1);
  cmSystemTools::ConvertToUnixSlashes(destination);
  if (destination.empty()) {
    destination = ".";
  }

  // Use a file install generator.
  const std::string no_permissions;
  const std::string no_rename;
  bool no_exclude_from_all = false;
  std::string no_component =
    makefile.GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME");
  std::vector<std::string> no_configurations;
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(&makefile);
  makefile.AddInstallGenerator(cm::make_unique<cmInstallFilesGenerator>(
    files, destination, false, no_permissions, no_configurations, no_component,
    message, no_exclude_from_all, no_rename, false, makefile.GetBacktrace()));
}

/**
 * Find a file in the build or source tree for installation given a
 * relative path from the CMakeLists.txt file.  This will favor files
 * present in the build tree.  If a full path is given, it is just
 * returned.
 */
static std::string FindInstallSource(cmMakefile& makefile, const char* name)
{
  if (cmSystemTools::FileIsFullPath(name) ||
      cmGeneratorExpression::Find(name) == 0) {
    // This is a full path.
    return name;
  }

  // This is a relative path.
  std::string tb = cmStrCat(makefile.GetCurrentBinaryDirectory(), '/', name);
  std::string ts = cmStrCat(makefile.GetCurrentSourceDirectory(), '/', name);

  if (cmSystemTools::FileExists(tb)) {
    // The file exists in the binary tree.  Use it.
    return tb;
  }
  if (cmSystemTools::FileExists(ts)) {
    // The file exists in the source tree.  Use it.
    return ts;
  }
  // The file doesn't exist.  Assume it will be present in the
  // binary tree when the install occurs.
  return tb;
}
