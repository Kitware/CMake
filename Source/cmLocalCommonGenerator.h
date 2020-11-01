/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include "cmLocalGenerator.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmSourceFile;

/** \class cmLocalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja local generators.
 */
class cmLocalCommonGenerator : public cmLocalGenerator
{
public:
  cmLocalCommonGenerator(cmGlobalGenerator* gg, cmMakefile* mf,
                         std::string wd);
  ~cmLocalCommonGenerator() override;

  std::vector<std::string> const& GetConfigNames() const
  {
    return this->ConfigNames;
  }

  std::string GetWorkingDirectory() const { return this->WorkingDirectory; }

  std::string GetTargetFortranFlags(cmGeneratorTarget const* target,
                                    std::string const& config) override;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, std::string>& mapping,
    cmGeneratorTarget const* gt = nullptr) override;

protected:
  std::string WorkingDirectory;

  std::vector<std::string> ConfigNames;

  friend class cmCommonTargetGenerator;
};
