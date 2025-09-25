/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmExternalMakefileProjectGenerator.h"

class cmGeneratedFileStream;
class cmLocalGenerator;

/** \class cmExtraKateGenerator
 * \brief Write Kate project files for Makefile or ninja based projects
 */
class cmExtraKateGenerator : public cmExternalMakefileProjectGenerator
{
public:
  cmExtraKateGenerator();

  static cmExternalMakefileProjectGeneratorFactory* GetFactory();

  void Generate() override;

private:
  void CreateKateProjectFile(cmLocalGenerator const& lg) const;
  void CreateDummyKateProjectFile(cmLocalGenerator const& lg) const;
  void WriteTargets(cmLocalGenerator const& lg,
                    cmGeneratedFileStream& fout) const;
  void AppendTarget(cmGeneratedFileStream& fout, std::string const& target,
                    std::vector<std::string> const& configs,
                    std::string const& make, std::string const& makeArgs,
                    std::string const& path,
                    std::string const& homeOutputDir) const;

  std::string GenerateFilesString(cmLocalGenerator const& lg) const;
  std::string GetPathBasename(std::string const& path) const;
  std::string GenerateProjectName(std::string const& name,
                                  std::string const& type,
                                  std::string const& path) const;

  std::string ProjectName;
  bool UseNinja;
};
