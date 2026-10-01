/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCPackAPKGenerator.h"

#include <ostream>
#include <string>
#include <vector>

#include "cmCPackLog.h"
#include "cmSystemTools.h"

cmCPackAPKGenerator::cmCPackAPKGenerator() = default;

cmCPackAPKGenerator::~cmCPackAPKGenerator() = default;

int cmCPackAPKGenerator::InitializeInternal()
{
  // The staging tree is the root of the APK.
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "0");
  return this->Superclass::InitializeInternal();
}

int cmCPackAPKGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Toplevel: " << this->toplevel << std::endl);

  if (!this->ReadListFile("Internal/CPack/CPackAPK.cmake")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error while executing CPackAPK.cmake" << std::endl);
    return 0;
  }

  // CPackAPK.cmake writes the package to the file name CPack asked for.
  for (std::string const& packageFileName : this->packageFileNames) {
    if (!cmSystemTools::FileExists(packageFileName)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "APK package was not generated at '" << packageFileName
                                                         << "'" << std::endl);
      return 0;
    }
  }

  return 1;
}
