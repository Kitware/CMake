/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

class cmGeneratorTarget;
class cmTarget;

class cmCxxModuleUsageEffects
{
public:
  cmCxxModuleUsageEffects(cmGeneratorTarget const* gt);

  void ApplyToTarget(cmTarget* tgt);
  std::string const& GetHash() const;

private:
  std::string Hash;
};
