/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmListFileCache.h"

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
  std::vector<BT<std::string>> CompileFeatures;
};
