/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallSubdirectoryGenerator_h
#define cmInstallSubdirectoryGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmInstallGenerator.h"

#include <iosfwd>
#include <string>

class cmLocalGenerator;
class cmMakefile;

/** \class cmInstallSubdirectoryGenerator
 * \brief Generate target installation rules.
 */
class cmInstallSubdirectoryGenerator : public cmInstallGenerator
{
public:
  cmInstallSubdirectoryGenerator(cmMakefile* makefile,
                                 const char* binaryDirectory,
                                 bool excludeFromAll);
  ~cmInstallSubdirectoryGenerator() override;

  bool HaveInstall() override;
  void CheckCMP0082(bool& haveSubdirectoryInstall,
                    bool& haveInstallAfterSubdirectory) override;

  bool Compute(cmLocalGenerator* lg) override;

protected:
  void GenerateScript(std::ostream& os) override;

  cmMakefile* Makefile;
  std::string BinaryDirectory;
  cmLocalGenerator* LocalGenerator;
};

#endif
