/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include <cmFastbuildTargetGenerator.h>

class cmGeneratorTarget;

class cmFastbuildUtilityTargetGenerator : public cmFastbuildTargetGenerator
{
public:
  cmFastbuildUtilityTargetGenerator(cmGeneratorTarget* gt,
                                    std::string configParam);

  void Generate() override;
};
