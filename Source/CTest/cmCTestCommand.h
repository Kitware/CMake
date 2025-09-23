/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmCTest;
class cmExecutionStatus;
struct cmListFileArgument;

/** \class cmCTestCommand
 * \brief A superclass for all commands added to the CTestScriptHandler
 *
 * cmCTestCommand is the superclass for all commands that will be added to
 * the ctest script handlers parser.
 *
 */
class cmCTestCommand
{
public:
  cmCTestCommand(cmCTest* ctest)
    : CTest(ctest)
  {
  }

  virtual ~cmCTestCommand() = default;
  cmCTestCommand(cmCTestCommand const&) = default;
  cmCTestCommand& operator=(cmCTestCommand const&) = default;

  bool operator()(std::vector<cmListFileArgument> const& args,
                  cmExecutionStatus& status) const;

private:
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus&) const = 0;

protected:
  cmCTest* CTest = nullptr;
};
