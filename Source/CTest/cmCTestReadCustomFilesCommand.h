/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestReadCustomFilesCommand_h
#define cmCTestReadCustomFilesCommand_h

#include "cmConfigure.h"

#include "cmCTestCommand.h"

#include <string>
#include <vector>

class cmCommand;
class cmExecutionStatus;

/** \class cmCTestReadCustomFiles
 * \brief Run a ctest script
 *
 * cmLibrarysCommand defines a list of executable (i.e., test)
 * programs to create.
 */
class cmCTestReadCustomFilesCommand : public cmCTestCommand
{
public:
  cmCTestReadCustomFilesCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE
  {
    cmCTestReadCustomFilesCommand* ni = new cmCTestReadCustomFilesCommand;
    ni->CTest = this->CTest;
    return ni;
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "ctest_read_custom_files"; }
};

#endif
