/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFindLibraryCommand_h
#define cmFindLibraryCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cm_memory.hxx"

#include "cmCommand.h"
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
  cmFindLibraryCommand();
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmFindLibraryCommand>();
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

protected:
  void AddArchitecturePaths(const char* suffix);
  void AddArchitecturePath(std::string const& dir,
                           std::string::size_type start_pos,
                           const char* suffix, bool fresh = true);
  std::string FindLibrary();

private:
  std::string FindNormalLibrary();
  std::string FindNormalLibraryNamesPerDir();
  std::string FindNormalLibraryDirsPerName();
  std::string FindFrameworkLibrary();
  std::string FindFrameworkLibraryNamesPerDir();
  std::string FindFrameworkLibraryDirsPerName();
};

#endif
