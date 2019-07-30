/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestUploadHandler_h
#define cmCTestUploadHandler_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCTestGenericHandler.h"

#include <set>
#include <string>

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

  /*
   * The main entry point for this class
   */
  int ProcessHandler() override;

  void Initialize() override;

  /** Specify a set of files to submit.  */
  void SetFiles(std::set<std::string> const& files);

private:
  std::set<std::string> Files;
};

#endif
