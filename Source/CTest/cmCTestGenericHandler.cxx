/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestGenericHandler.h"

#include "cmCTest.h"

cmCTestGenericHandler::cmCTestGenericHandler(cmCTest* ctest)
  : CTest(ctest)
{
  this->SetVerbose(ctest->GetExtraVerbose());
}

cmCTestGenericHandler::~cmCTestGenericHandler() = default;

bool cmCTestGenericHandler::StartResultingXML(cmCTest::Part part,
                                              char const* name,
                                              cmGeneratedFileStream& xofs)
{
  return this->CTest->StartResultingXML(part, name, this->SubmitIndex, xofs);
}

bool cmCTestGenericHandler::StartLogFile(char const* name,
                                         cmGeneratedFileStream& xofs)
{
  return this->CTest->StartLogFile(name, this->SubmitIndex, xofs);
}
