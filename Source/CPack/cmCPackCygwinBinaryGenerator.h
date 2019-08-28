/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackCygwinBinaryGenerator_h
#define cmCPackCygwinBinaryGenerator_h

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
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension();
  std::string OutputExtension;
};

#endif
