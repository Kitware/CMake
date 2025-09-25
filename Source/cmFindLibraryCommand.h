/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmFindBase.h"

class cmExecutionStatus;

/** \class cmFindLibraryCommand
 * \brief Define a command to search for a library.
 *
 * cmFindLibraryCommand is used to define a CMake variable
 * that specifies a library. The command searches for a given
 * file in a list of directories.
 */
class cmFindLibraryCommand : public cmFindBase
{
public:
  cmFindLibraryCommand(cmExecutionStatus& status);

  bool InitialPass(std::vector<std::string> const& args);

protected:
  void AddArchitecturePaths(char const* suffix);
  void AddArchitecturePath(std::string const& dir,
                           std::string::size_type start_pos,
                           char const* suffix, bool fresh = true);
  std::string FindLibrary();

private:
  std::string FindNormalLibrary();
  std::string FindNormalLibraryNamesPerDir();
  std::string FindNormalLibraryDirsPerName();
  std::string FindFrameworkLibrary();
  std::string FindFrameworkLibraryNamesPerDir();
  std::string FindFrameworkLibraryDirsPerName();
};

bool cmFindLibrary(std::vector<std::string> const& args,
                   cmExecutionStatus& status);
