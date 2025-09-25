/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmBuildOptions.h"
#include "cmGlobalGeneratorFactory.h"
#include "cmGlobalUnixMakefileGenerator3.h"

class cmMakefile;
class cmake;

/** \class cmGlobalWatcomWMakeGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalWatcomWMakeGenerator manages nmake build process for a tree
 */
class cmGlobalWatcomWMakeGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalWatcomWMakeGenerator(cmake* cm);
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory()
  {
    return std::unique_ptr<cmGlobalGeneratorFactory>(
      new cmGlobalGeneratorSimpleFactory<cmGlobalWatcomWMakeGenerator>());
  }
  //! Get the name for the generator.
  std::string GetName() const override
  {
    return cmGlobalWatcomWMakeGenerator::GetActualName();
  }
  static std::string GetActualName() { return "Watcom WMake"; }

  /** Get the documentation entry for this generator.  */
  static cmDocumentationEntry GetDocumentation();

  /** Tell the generator about the target system.  */
  bool SetSystemName(std::string const& s, cmMakefile* mf) override;

  /**
   * Try to determine system information such as shared library
   * extension, pthreads, byte order etc.
   */
  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;

  bool AllowNotParallel() const override { return false; }
  bool AllowDeleteOnError() const override { return false; }

  bool IsGNUMakeJobServerAware() const override { return false; }

protected:
  std::vector<GeneratedMakeCommand> GenerateBuildCommand(
    std::string const& makeProgram, std::string const& projectName,
    std::string const& projectDir, std::vector<std::string> const& targetNames,
    std::string const& config, int jobs, bool verbose,
    cmBuildOptions const& buildOptions = cmBuildOptions(),
    std::vector<std::string> const& makeOptions =
      std::vector<std::string>()) override;

  void PrintBuildCommandAdvice(std::ostream& os, int jobs) const override;
};
