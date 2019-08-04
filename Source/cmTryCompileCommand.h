/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmTryCompileCommand_h
#define cmTryCompileCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/memory>

#include "cmCommand.h"
#include "cmCoreTryCompile.h"

class cmExecutionStatus;

/** \class cmTryCompileCommand
 * \brief Specifies where to install some files
 *
 * cmTryCompileCommand is used to test if source code can be compiled
 */
class cmTryCompileCommand : public cmCoreTryCompile
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    return cm::make_unique<cmTryCompileCommand>();
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;
};

#endif
