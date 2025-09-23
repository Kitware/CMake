/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackCygwinSourceGenerator
 * \brief A generator for cygwin source files
 */
class cmCPackCygwinSourceGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackCygwinSourceGenerator, cmCPackArchiveGenerator);

  /**
   * Construct generator
   */
  cmCPackCygwinSourceGenerator();
  ~cmCPackCygwinSourceGenerator() override;

protected:
  char const* GetPackagingInstallPrefix() override;
  int InitializeInternal() override;
  int PackageFiles() override;
  char const* GetOutputExtension() override;
  std::string InstallPrefix;
  std::string OutputExtension;
};
