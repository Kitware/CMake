/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmValue.h"

class cmMakefile;
class cmake;
struct cmDocumentationEntry;

/** \class cmGlobalJOMMakefileGenerator
 * \brief Write a JOM makefiles.
 *
 * cmGlobalJOMMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalJOMMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalJOMMakefileGenerator(cmake* cm);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalJOMMakefileGenerator>());
  }
  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalJOMMakefileGenerator::GetActualName();
  }
  // use NMake Makefiles in the name so that scripts/tests that depend on the
  // name NMake Makefiles will work
  static std::string GetActualName() { return "NMake Makefiles JOM"; }

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

protected:
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, int jobs, bool verbose,
    const cmBuildOptions& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

private:
  void PrintCompilerAdvice(std::ostream& os, std::string const& lang,
                           cmValue envVar) const override;
};
