/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestMemCheckCommand_h
#define cmCTestMemCheckCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <utility>

#include "cm_memory.hxx"

#include "cmCTestTestCommand.h"
#include "cmCommand.h"

class cmCTestGenericHandler;

/** \class cmCTestMemCheck
 * \brief Run a ctest script
 *
 * cmCTestMemCheckCommand defineds the command to test the project.
 */
class cmCTestMemCheckCommand : public cmCTestTestCommand
{
public:
  cmCTestMemCheckCommand();

  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestMemCheckCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

protected:
  cmCTestGenericHandler* InitializeActualHandler() override;

  void ProcessAdditionalValues(cmCTestGenericHandler* handler) override;

  enum
  {
    ctm_DEFECT_COUNT = ctt_LAST,
    ctm_LAST
  };
};

#endif
