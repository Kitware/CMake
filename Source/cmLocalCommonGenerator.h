/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmLocalGenerator.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;

/** \class cmLocalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja local generators.
 */
class cmLocalCommonGenerator : public cmLocalGenerator
{
public:
  cmLocalCommonGenerator(cmGlobalGenerator* gg, cmMakefile* mf);
  ~cmLocalCommonGenerator() override;

  std::vector<std::string> const& GetConfigNames() const
  {
    return this->ConfigNames;
  }

  virtual std::string const& GetWorkingDirectory() const;

  std::string GetTargetFortranFlags(cmGeneratorTarget const* target,
                                    std::string const& config) override;

  std::string ComputeLongTargetDirectory(cmGeneratorTarget const* gt) const;
  std::string GetTargetDirectory(
    cmGeneratorTarget const* target,
    cmStateEnums::IntermediateDirKind kind) const override;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, cmObjectLocations>& mapping,
    std::string const& config, cmGeneratorTarget const* gt = nullptr) override;

protected:
  std::vector<std::string> ConfigNames;

  friend class cmCommonTargetGenerator;
};
