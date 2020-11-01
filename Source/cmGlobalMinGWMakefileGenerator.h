/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>

#include "cmGlobalUnixMakefileGenerator3.h"

/** \class cmGlobalMinGWMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalMinGWMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalMinGWMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalMinGWMakefileGenerator(cmake* cm);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalMinGWMakefileGenerator>());
  }
  //! Get the name for the generator.
  virtual std::string GetName() const
  {
    return cmGlobalMinGWMakefileGenerator::GetActualName();
  }
  static std::string GetActualName() { return "MinGW Makefiles"; }

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  virtual void EnableLanguage(std::vector<std::string> const& languages,
                              cmMakefile*, bool optional);
};
