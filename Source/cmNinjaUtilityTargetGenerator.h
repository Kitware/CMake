/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmNinjaTargetGenerator.h"

class cmGeneratorTarget;

class cmNinjaUtilityTargetGenerator : public cmNinjaTargetGenerator
{
public:
  cmNinjaUtilityTargetGenerator(cmGeneratorTarget* target);
  ~cmNinjaUtilityTargetGenerator() override;

  void Generate(const std::string& config) override;

private:
  void WriteUtilBuildStatements(std::string const& config,
                                std::string const& fileConfig);
};
