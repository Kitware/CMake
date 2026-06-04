/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmBuildSbomBuilder.h"
#include "cmSbomArguments.h"
class cmExportSet;
class cmGeneratorTarget;
class cmLocalGenerator;

/** \class cmBuildSbomGenerator
 * \brief Thin wrapper around cmBuildSbomBuilder for the build-tree SBOM case.
 *
 * Stored on cmMakefile at configure time.  At generate time,
 * ComputeBuildFileGenerators() calls Compute(lg) to resolve export set
 * targets and supply the local generator — mirroring
 * cmExportBuildFileGenerator.
 */
class cmBuildSbomGenerator
{
public:
  cmBuildSbomGenerator(cmSbomArguments args,
                       std::vector<cmExportSet*> exportSets,
                       std::string outputFile)
    : OutputFile(std::move(outputFile))
    , Builder(cm::make_unique<cmBuildSbomBuilder>(std::move(args),
                                                  std::move(exportSets)))
  {
  }

  void Compute(cmLocalGenerator* lg);

  std::string const& GetOutputFile() const { return this->OutputFile; }

  /** True if this SBOM directly describes `target`.  Used by peer SBOMs to
   *  attribute cross-references when install(export) provenance is absent. */
  bool CoversTarget(cmGeneratorTarget const* target) const
  {
    return this->Builder->CoversTarget(target);
  }

  bool GenerateForBuild(std::ostream& os, std::string const& config)
  {
    return this->Builder->Generate(os, config);
  }

  std::string const& GetPackageName() const
  {
    return this->Builder->GetPackageName();
  }

  /** Open the output file and write the SBOM document to it. */
  bool GenerateForBuild(std::string const& config);

private:
  std::string OutputFile;
  std::unique_ptr<cmBuildSbomBuilder> Builder;
};
