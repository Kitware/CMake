/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalUnixMakefileGenerator3.h"

class cmMakefile;
class cmake;

/** \class cmGlobalMSYSMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalMSYSMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalMSYSMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalMSYSMakefileGenerator(cmake* cm);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalMSYSMakefileGenerator>());
  }

  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalMSYSMakefileGenerator::GetActualName();
  }
  static std::string GetActualName() { return "MSYS Makefiles"; }

  /** Get the documentation entry for this generator.  */
  static cmDocumentationEntry GetDocumentation();

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

private:
  std::string FindMinGW(std::string const& makeloc);
};
