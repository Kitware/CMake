/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallFilesCommand.h"

#include "cmGeneratorExpression.h"
#include "cmGlobalGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallGenerator.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmSystemTools.h"

class cmExecutionStatus;

static std::string FindInstallSource(cmMakefile& makefile, const char* name);
static void CreateInstallGenerator(cmMakefile& makefile,
                                   std::string const& dest,
                                   std::vector<std::string> const& files);
static void FinalAction(cmMakefile& makefile, std::string const& dest,
                        std::vector<std::string> const& args);

bool cmInstallFilesCommand::InitialPass(std::vector<std::string> const& args,
                                        cmExecutionStatus&)
{
  if (args.size() < 2) {
    this->SetError("called with incorrect number of arguments");
    return false;
  }

  // Enable the install target.
  this->Makefile->GetGlobalGenerator()->EnableInstallTarget();

  std::string const& dest = args[0];

  if ((args.size() > 1) && (args[1] == "FILES")) {
    std::vector<std::string> files;
    for (std::string const& arg : cmMakeRange(args).advance(2)) {
      // Find the source location for each file listed.
      files.push_back(FindInstallSource(*this->Makefile, arg.c_str()));
    }
    CreateInstallGenerator(*this->Makefile, dest, files);
  } else {
    std::vector<std::string> finalArgs(args.begin() + 1, args.end());
    this->Makefile->AddFinalAction([dest, finalArgs](cmMakefile& makefile) {
      FinalAction(makefile, dest, finalArgs);
    });
  }

  this->Makefile->GetGlobalGenerator()->AddInstallComponent(
    this->Makefile->GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME"));

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
    std::vector<std::string>::const_iterator s = args.begin();
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

    std::vector<std::string>::iterator s = files.begin();
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
  const char* no_permissions = "";
  const char* no_rename = "";
  bool no_exclude_from_all = false;
  std::string no_component =
    makefile.GetSafeDefinition("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME");
  std::vector<std::string> no_configurations;
  cmInstallGenerator::MessageLevel message =
    cmInstallGenerator::SelectMessageLevel(&makefile);
  makefile.AddInstallGenerator(new cmInstallFilesGenerator(
    files, destination.c_str(), false, no_permissions, no_configurations,
    no_component.c_str(), message, no_exclude_from_all, no_rename));
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
  std::string tb = makefile.GetCurrentBinaryDirectory();
  tb += "/";
  tb += name;
  std::string ts = makefile.GetCurrentSourceDirectory();
  ts += "/";
  ts += name;

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
