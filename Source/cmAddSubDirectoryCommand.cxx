/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmAddSubDirectoryCommand.h"

#include <cstring>

#include "cmExecutionStatus.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool cmAddSubDirectoryCommand(std::vector<std::string> const& args,
                              cmExecutionStatus& status)
{
  if (args.empty()) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  cmMakefile& mf = status.GetMakefile();
  // store the binpath
  std::string const& srcArg = args.front();
  std::string binArg;

  bool excludeFromAll = false;

  // process the rest of the arguments looking for optional args
  for (std::string const& arg : cmMakeRange(args).advance(1)) {
    if (arg == "EXCLUDE_FROM_ALL") {
      excludeFromAll = true;
      continue;
    }
    if (binArg.empty()) {
      binArg = arg;
    } else {
      status.SetError("called with incorrect number of arguments");
      return false;
    }
  }

  // Compute the full path to the specified source directory.
  // Interpret a relative path with respect to the current source directory.
  std::string srcPath;
  if (cmSystemTools::FileIsFullPath(srcArg)) {
    srcPath = srcArg;
  } else {
    srcPath = cmStrCat(mf.GetCurrentSourceDirectory(), '/', srcArg);
  }
  if (!cmSystemTools::FileIsDirectory(srcPath)) {
    std::string error = cmStrCat("given source \"", srcArg,
                                 "\" which is not an existing directory.");
    status.SetError(error);
    return false;
  }
  srcPath =
    cmSystemTools::CollapseFullPath(srcPath, mf.GetHomeOutputDirectory());

  // Compute the full path to the binary directory.
  std::string binPath;
  if (binArg.empty()) {
    // No binary directory was specified.  If the source directory is
    // not a subdirectory of the current directory then it is an
    // error.
    if (!cmSystemTools::IsSubDirectory(srcPath,
                                       mf.GetCurrentSourceDirectory())) {
      status.SetError(
        cmStrCat("not given a binary directory but the given source ",
                 "directory \"", srcPath, "\" is not a subdirectory of \"",
                 mf.GetCurrentSourceDirectory(),
                 "\".  When specifying an "
                 "out-of-tree source a binary directory must be explicitly "
                 "specified."));
      return false;
    }

    // Remove the CurrentDirectory from the srcPath and replace it
    // with the CurrentOutputDirectory.
    const std::string& src = mf.GetCurrentSourceDirectory();
    const std::string& bin = mf.GetCurrentBinaryDirectory();
    size_t srcLen = src.length();
    size_t binLen = bin.length();
    if (srcLen > 0 && src.back() == '/') {
      --srcLen;
    }
    if (binLen > 0 && bin.back() == '/') {
      --binLen;
    }
    binPath = bin.substr(0, binLen) + srcPath.substr(srcLen);
  } else {
    // Use the binary directory specified.
    // Interpret a relative path with respect to the current binary directory.
    if (cmSystemTools::FileIsFullPath(binArg)) {
      binPath = binArg;
    } else {
      binPath = cmStrCat(mf.GetCurrentBinaryDirectory(), '/', binArg);
    }
  }
  binPath = cmSystemTools::CollapseFullPath(binPath);

  // Add the subdirectory using the computed full paths.
  mf.AddSubDirectory(srcPath, binPath, excludeFromAll, true);

  return true;
}
