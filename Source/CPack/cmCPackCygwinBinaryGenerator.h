/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmCPackArchiveGenerator.h"

/** \class cmCPackCygwinBinaryGenerator
 * \brief A generator for TarBZip2 files
 */
class cmCPackCygwinBinaryGenerator : public cmCPackArchiveGenerator
{
public:
  cmCPackTypeMacro(cmCPackCygwinBinaryGenerator, cmCPackArchiveGenerator);

  /**
   * Construct generator
   */
  cmCPackCygwinBinaryGenerator();
  ~cmCPackCygwinBinaryGenerator() override;

protected:
  int InitializeInternal() override;
  int PackageFiles() override;
  char const* GetOutputExtension() override;
  std::string OutputExtension;
};
