/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>

class cmGeneratorTarget;
class cmInstallCxxModuleBmiGenerator;
class cmInstallFileSetGenerator;
class cmInstallFilesGenerator;
class cmInstallTargetGenerator;

/** \brief A member of an ExportSet
 *
 * This struct holds pointers to target and all relevant generators.
 */
class cmTargetExport
{
public:
  std::string TargetName;
  cmGeneratorTarget* Target;

  ///@name Generators
  ///@{
  cmInstallTargetGenerator* ArchiveGenerator;
  cmInstallTargetGenerator* RuntimeGenerator;
  cmInstallTargetGenerator* LibraryGenerator;
  cmInstallTargetGenerator* ObjectsGenerator;
  cmInstallTargetGenerator* FrameworkGenerator;
  cmInstallTargetGenerator* BundleGenerator;
  cmInstallFilesGenerator* HeaderGenerator;
  std::map<std::string, cmInstallFileSetGenerator*> FileSetGenerators;
  cmInstallCxxModuleBmiGenerator* CxxModuleBmiGenerator;
  ///@}

  bool NamelinkOnly = false;
  std::string XcFrameworkLocation;
};
