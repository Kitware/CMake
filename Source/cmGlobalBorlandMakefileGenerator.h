/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalUnixMakefileGenerator3.h"

class cmLocalGenerator;
class cmMakefile;
class cmake;
struct cmDocumentationEntry;

/** \class cmGlobalBorlandMakefileGenerator
 * \brief Write a Borland makefiles.
 *
 * cmGlobalBorlandMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalBorlandMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalBorlandMakefileGenerator(cmake* cm);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalBorlandMakefileGenerator>());
  }

  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalBorlandMakefileGenerator::GetActualName();
  }
  static std::string GetActualName() { return "Borland Makefiles"; }

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

  //! Create a local generator appropriate to this Global Generator
  std::unique_ptr<cmLocalGenerator> CreateLocalGenerator(
    cmMakefile* mf) override;

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  bool AllowNotParallel() const override { return false; }
  bool AllowDeleteOnError() const override { return false; }
  bool CanEscapeOctothorpe() const override { return true; }

protected:
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    const std::string& makeProgram, const std::string& projectName,
    const std::string& projectDir, std::vector<std::string> const& targetNames,
    const std::string& config, int jobs, bool verbose,
    const cmBuildOptions& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  void PrintBuildCommandAdvice(std::ostream& os, int jobs) const override;
};
