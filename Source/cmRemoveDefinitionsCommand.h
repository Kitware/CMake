/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmRemoveDefinitionsCommand_h
#define cmRemoveDefinitionsCommand_h

#include "cmConfigure.h"

#include <string>
#include <vector>

#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmRemoveDefinitionsCommand
 * \brief Specify a list of compiler defines
 *
 * cmRemoveDefinitionsCommand specifies a list of compiler defines.
 * These defines will
 * be removed from the compile command.
 */
class cmRemoveDefinitionsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  cmCommand* Clone() CM_OVERRIDE { return new cmRemoveDefinitionsCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) CM_OVERRIDE;

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  std::string GetName() const CM_OVERRIDE { return "remove_definitions"; }
};

#endif
