/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestEmptyBinaryDirectoryCommand.h"

#include "cmsys/Directory.hxx"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

// Try to remove the binary directory once
cmsys::Status TryToRemoveBinaryDirectoryOnce(std::string const& directoryPath)
{
  cmsys::Directory directory;
  directory.Load(directoryPath);

  // Make sure that CMakeCache.txt is deleted last.
  for (unsigned long i = 0; i < directory.GetNumberOfFiles(); ++i) {
    std::string path = directory.GetFile(i);

    if (path == "." || path == ".." || path == "CMakeCache.txt") {
      continue;
    }

    std::string fullPath = cmStrCat(directoryPath, "/", path);

    bool isDirectory = cmSystemTools::FileIsDirectory(fullPath) &&
      !cmSystemTools::FileIsSymlink(fullPath);

    cmsys::Status status;
    if (isDirectory) {
      status = cmSystemTools::RemoveADirectory(fullPath);
    } else {
      status = cmSystemTools::RemoveFile(fullPath);
    }
    if (!status) {
      return status;
    }
  }

  return cmSystemTools::RemoveADirectory(directoryPath);
}

/*
 * Empty Binary Directory
 */
bool EmptyBinaryDirectory(std::string const& sname, std::string& err)
{
  // try to avoid deleting root
  if (sname.size() < 2) {
    err = "path too short";
    return false;
  }

  // consider non existing target directory a success
  if (!cmSystemTools::FileExists(sname)) {
    return true;
  }

  // try to avoid deleting directories that we shouldn't
  std::string check = cmStrCat(sname, "/CMakeCache.txt");

  if (!cmSystemTools::FileExists(check)) {
    err = "path does not contain an existing CMakeCache.txt file";
    return false;
  }

  cmsys::Status status;
  for (int i = 0; i < 5; ++i) {
    status = TryToRemoveBinaryDirectoryOnce(sname);
    if (status) {
      return true;
    }
    cmSystemTools::Delay(100);
  }

  err = status.GetString();
  return false;
}

} // namespace

bool cmCTestEmptyBinaryDirectoryCommand(std::vector<std::string> const& args,
                                        cmExecutionStatus& status)
{
  if (args.size() != 1) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  std::string err;
  if (!EmptyBinaryDirectory(args[0], err)) {
    status.GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Did not remove the binary directory:\n ", args[0],
               "\nbecause:\n ", err));
    return true;
  }

  return true;
}
