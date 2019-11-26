/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackCygwinSourceGenerator_h
#define cmCPackCygwinSourceGenerator_h

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
  const char* GetPackagingInstallPrefix();
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension();
  std::string InstallPrefix;
  std::string OutputExtension;
};

#endif
