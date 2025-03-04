/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmExternalMakefileProjectGenerator.h"

class cmGeneratorTarget;
class cmLocalGenerator;
class cmMakefile;
class cmXMLWriter;

/** \class cmExtraCodeBlocksGenerator
 * \brief Write CodeBlocks project files for Makefile based projects
 */
class cmExtraCodeBlocksGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraCodeBlocksGenerator();

  static cmExternalMakefileProjectGeneratorFactory* GetFactory();

  void Generate() override;

private:
  struct CbpUnit
  {
    std::vector<cmGeneratorTarget const*> Targets;
  };

  void CreateProjectFile(std::vector<cmLocalGenerator*> const& lgs);

  void CreateNewProjectFile(std::vector<cmLocalGenerator*> const& lgs,
                            std::string const& filename);
  std::string CreateDummyTargetFile(cmLocalGenerator* lg,
                                    cmGeneratorTarget* target) const;

  std::string GetCBCompilerId(cmMakefile const* mf);
  int GetCBTargetType(cmGeneratorTarget* target);
  std::string BuildMakeCommand(std::string const& make,
                               std::string const& makefile,
                               std::string const& target,
                               std::string const& makeFlags);
  void AppendTarget(cmXMLWriter& xml, std::string const& targetName,
                    cmGeneratorTarget* target, std::string const& make,
                    cmLocalGenerator const* lg, std::string const& compiler,
                    std::string const& makeFlags);
};
