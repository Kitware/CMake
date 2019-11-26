/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestStartCommand_h
#define cmCTestStartCommand_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCTestCommand.h"
#include "cmCommand.h"

class cmExecutionStatus;

/** \class cmCTestStart
 * \brief Run a ctest script
 *
 * cmCTestStartCommand defineds the command to start the nightly testing.
 */
class cmCTestStartCommand : public cmCTestCommand
{
public:
  cmCTestStartCommand();

  /**
   * This is a virtual constructor for the command.
   */
  std::unique_ptr<cmCommand> Clone() override
  {
    auto ni = cm::make_unique<cmCTestStartCommand>();
    ni->CTest = this->CTest;
    ni->CTestScriptHandler = this->CTestScriptHandler;
    ni->CreateNewTag = this->CreateNewTag;
    ni->Quiet = this->Quiet;
    return std::unique_ptr<cmCommand>(std::move(ni));
  }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  bool InitialPass(std::vector<std::string> const& args,
                   cmExecutionStatus& status) override;

  /**
   * Will this invocation of ctest_start create a new TAG file?
   */
  bool ShouldCreateNewTag() { return this->CreateNewTag; }

  /**
   * Should this invocation of ctest_start output non-error messages?
   */
  bool ShouldBeQuiet() { return this->Quiet; }

private:
  bool InitialCheckout(std::ostream& ofs, std::string const& sourceDir);
  bool CreateNewTag;
  bool Quiet;
};

#endif
