/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmWriteFileCommand.h"

#include "cmsys/FStream.hxx"

#include "cm_sys_stat.h"

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

// cmLibraryCommand
bool cmWriteFileCommand(std::vector<std::string> const& args,
                        cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }
  std::string message;
  auto i = args.begin();

  std::string const& fileName = *i;
  bool overwrite = true;
  i++;

  for (; i != args.end(); ++i) {
    if (*i == "APPEND") {
      overwrite = false;
    } else {
      message += *i;
    }
  }

  if (!status.GetMakefile().CanIWriteThisFile(fileName)) {
    std::string e =
      "attempted to write a file: " + fileName + " into a source directory.";
    status.SetError(e);
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir);

  mode_t mode = 0;
  bool writable = false;

  // Set permissions to writable
  if (cmSystemTools::GetPermissions(fileName.c_str(), mode)) {
#if defined(_MSC_VER) || defined(__MINGW32__)
    writable = (mode & S_IWRITE) != 0;
    mode_t newMode = mode | S_IWRITE;
#else
    writable = mode & S_IWUSR;
    mode_t newMode = mode | S_IWUSR | S_IWGRP;
#endif
    if (!writable) {
      cmSystemTools::SetPermissions(fileName.c_str(), newMode);
    }
  }
  // If GetPermissions fails, pretend like it is ok. File open will fail if
  // the file is not writable
  cmsys::ofstream file(fileName.c_str(),
                       overwrite ? std::ios::out : std::ios::app);
  if (!file) {
    std::string error =
      cmStrCat("Internal CMake error when trying to open file: ", fileName,
               " for writing.");
    status.SetError(error);
    return false;
  }
  file << message << '\n';
  file.close();
  if (mode && !writable) {
    cmSystemTools::SetPermissions(fileName.c_str(), mode);
  }

  return true;
}
