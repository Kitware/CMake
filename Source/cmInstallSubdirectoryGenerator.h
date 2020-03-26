/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmInstallSubdirectoryGenerator_h
#define cmInstallSubdirectoryGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmInstallGenerator.h"

class cmLocalGenerator;
class cmMakefile;

/** \class cmInstallSubdirectoryGenerator
 * \brief Generate target installation rules.
 */
class cmInstallSubdirectoryGenerator : public cmInstallGenerator
{
public:
  cmInstallSubdirectoryGenerator(cmMakefile* makefile,
                                 std::string binaryDirectory,
                                 bool excludeFromAll);
  ~cmInstallSubdirectoryGenerator() override;

  bool HaveInstall() override;
  void CheckCMP0082(bool& haveSubdirectoryInstall,
                    bool& haveInstallAfterSubdirectory) override;

  bool Compute(cmLocalGenerator* lg) override;

protected:
  void GenerateScript(std::ostream& os) override;

  cmMakefile* const Makefile;
  std::string const BinaryDirectory;
  cmLocalGenerator* LocalGenerator;
};

#endif
