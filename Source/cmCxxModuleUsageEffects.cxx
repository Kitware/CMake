/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCxxModuleUsageEffects.h"

#include <cm/optional>

#include "cmCryptoHash.h"
#include "cmGeneratorTarget.h"
#include "cmTarget.h"

cmCxxModuleUsageEffects::cmCxxModuleUsageEffects(cmGeneratorTarget const* gt)
{
  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_512);
  this->Hash = hasher.HashString(gt->GetName());

  // Collect compile features from the consuming target.
  for (auto const& feature : gt->Target->GetCompileFeaturesEntries()) {
    this->CompileFeatures.emplace_back(feature);
  }
}

void cmCxxModuleUsageEffects::ApplyToTarget(cmTarget* tgt)
{
  for (auto const& feature : this->CompileFeatures) {
    tgt->AppendProperty("COMPILE_FEATURES", feature.Value, feature.Backtrace);
  }
}

std::string const& cmCxxModuleUsageEffects::GetHash() const
{
  return this->Hash;
}
