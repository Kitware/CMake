/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGeneratorInitializer_h
#define cmQtAutoGeneratorInitializer_h

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmQtAutoGenDigest.h"

#include <string>

class cmGeneratorTarget;

class cmQtAutoGeneratorInitializer
{
public:
  static std::string GetQtMajorVersion(cmGeneratorTarget const* target);
  static std::string GetQtMinorVersion(cmGeneratorTarget const* target,
                                       std::string const& qtVersionMajor);

  static void InitializeAutogenTarget(cmQtAutoGenDigest& digest);
  static void SetupAutoGenerateTarget(cmQtAutoGenDigest const& digest);
};

#endif
