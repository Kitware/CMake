/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmCTestResourceAllocator.h"

#include <utility>
#include <vector>

#include "cmCTestResourceSpec.h"

void cmCTestResourceAllocator::InitializeFromResourceSpec(
  cmCTestResourceSpec const& spec)
{
  this->Resources.clear();

  for (auto const& it : spec.LocalSocket.Resources) {
    auto& res = this->Resources[it.first];
    for (auto const& specRes : it.second) {
      res[specRes.Id].Total = specRes.Capacity;
      res[specRes.Id].Locked = 0;
    }
  }
}

std::map<std::string,
         std::map<std::string, cmCTestResourceAllocator::Resource>> const&
cmCTestResourceAllocator::GetResources() const
{
  return this->Resources;
}

bool cmCTestResourceAllocator::AllocateResource(std::string const& name,
                                                std::string const& id,
                                                unsigned int slots)
{
  auto it = this->Resources.find(name);
  if (it == this->Resources.end()) {
    return false;
  }

  auto resIt = it->second.find(id);
  if (resIt == it->second.end()) {
    return false;
  }

  if (resIt->second.Total < resIt->second.Locked + slots) {
    return false;
  }

  resIt->second.Locked += slots;
  return true;
}

bool cmCTestResourceAllocator::DeallocateResource(std::string const& name,
                                                  std::string const& id,
                                                  unsigned int slots)
{
  auto it = this->Resources.find(name);
  if (it == this->Resources.end()) {
    return false;
  }

  auto resIt = it->second.find(id);
  if (resIt == it->second.end()) {
    return false;
  }

  if (resIt->second.Locked < slots) {
    return false;
  }

  resIt->second.Locked -= slots;
  return true;
}

bool cmCTestResourceAllocator::Resource::operator==(
  Resource const& other) const
{
  return this->Total == other.Total && this->Locked == other.Locked;
}

bool cmCTestResourceAllocator::Resource::operator!=(
  Resource const& other) const
{
  return !(*this == other);
}
