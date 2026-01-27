/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmLocalCommonGenerator.h"
#include "cmOutputConverter.h"

class cmGlobalFastbuildGenerator;
class cmGlobalGenerator;
class cmMakefile;

class cmLocalFastbuildGenerator : public cmLocalCommonGenerator
{
public:
  cmLocalFastbuildGenerator(cmGlobalGenerator* gg, cmMakefile* makefile);

  void Generate() override;

  void AppendFlagEscape(std::string& flags,
                        std::string const& rawFlag) const override;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, cmObjectLocations>& mapping,
    std::string const& config, cmGeneratorTarget const* gt = nullptr) override;

  cmGlobalFastbuildGenerator const* GetGlobalFastbuildGenerator() const;
  cmGlobalFastbuildGenerator* GetGlobalFastbuildGenerator();

  void AdditionalCleanFiles(std::string const& config);

private:
  std::string ConvertToIncludeReference(
    std::string const& path, cmOutputConverter::OutputFormat format) override;
};
