/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGenExContext.h"

#include <utility>

#include <cm/optional>

#include "cmLocalGenerator.h"
#include "cmPolicies.h"

namespace cm {
namespace GenEx {

Context::Context(cmLocalGenerator const* lg, std::string config,
                 std::string language)
  : LG(lg)
  , Config(std::move(config))
  , Language(std::move(language))
{
}

void Context::SetCMP0189(cmPolicies::PolicyStatus cmp0189)
{
  this->CMP0189 = cmp0189;
}

cmPolicies::PolicyStatus Context::GetCMP0189() const
{
  if (this->CMP0189.has_value()) {
    return *this->CMP0189;
  }
  return this->LG->GetPolicyStatus(cmPolicies::CMP0189);
}

}
}
