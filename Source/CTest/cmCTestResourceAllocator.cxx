/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmCTestResourceAllocator.h"

#include <utility>
#include <vector>

#include "cmCTestResourceSpec.h"

void cmCTestResourceAllocator::InitializeFromResourceSpec(
  const cmCTestResourceSpec& spec)
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

const std::map<std::string,
               std::map<std::string, cmCTestResourceAllocator::Resource>>&
cmCTestResourceAllocator::GetResources() const
{
  return this->Resources;
}

bool cmCTestResourceAllocator::AllocateResource(const std::string& name,
                                                const std::string& id,
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

bool cmCTestResourceAllocator::DeallocateResource(const std::string& name,
                                                  const std::string& id,
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
  const Resource& other) const
{
  return this->Total == other.Total && this->Locked == other.Locked;
}

bool cmCTestResourceAllocator::Resource::operator!=(
  const Resource& other) const
{
  return !(*this == other);
}
