/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmCommand.h"

class cmCTest;
class cmCTestScriptHandler;

/** \class cmCTestCommand
 * \brief A superclass for all commands added to the CTestScriptHandler
 *
 * cmCTestCommand is the superclass for all commands that will be added to
 * the ctest script handlers parser.
 *
 */
class cmCTestCommand : public cmCommand
{
public:
  cmCTestCommand()
  {
    this->CTest = nullptr;
    this->CTestScriptHandler = nullptr;
  }

  cmCTest* CTest;
  cmCTestScriptHandler* CTestScriptHandler;
};
