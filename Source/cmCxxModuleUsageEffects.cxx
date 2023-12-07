/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCxxModuleUsageEffects.h"

cmCxxModuleUsageEffects::cmCxxModuleUsageEffects(
  cmGeneratorTarget const* /*gt*/)
  : Hash("0000000000000000000000000000000000000000")
{
  // TODO: collect information from the generator target as to what might
  // affect module consumption.
}

void cmCxxModuleUsageEffects::ApplyToTarget(cmTarget* /*tgt*/)
{
  // TODO: apply the information collected in the constructor
}

std::string const& cmCxxModuleUsageEffects::GetHash() const
{
  return this->Hash;
}
