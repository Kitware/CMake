/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestUploadHandler_h
#define cmCTestUploadHandler_h

#include "cmConfigure.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"

/** \class cmCTestUploadHandler
 * \brief Helper class for CTest
 *
 * Submit arbitrary files
 *
 */
class cmCTestUploadHandler : public cmCTestGenericHandler
{
public:
  typedef cmCTestGenericHandler Superclass;

  cmCTestUploadHandler();
  ~cmCTestUploadHandler() CM_OVERRIDE {}

  /*
   * The main entry point for this class
   */
  int ProcessHandler() CM_OVERRIDE;

  void Initialize() CM_OVERRIDE;

  /** Specify a set of files to submit.  */
  void SetFiles(cmCTest::SetOfStrings const& files);

private:
  cmCTest::SetOfStrings Files;
};

#endif
