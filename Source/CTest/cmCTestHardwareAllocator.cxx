/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmCTestHardwareAllocator.h"

#include <utility>
#include <vector>

#include "cmCTestHardwareSpec.h"

void cmCTestHardwareAllocator::InitializeFromHardwareSpec(
  const cmCTestHardwareSpec& spec)
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
               std::map<std::string, cmCTestHardwareAllocator::Resource>>&
cmCTestHardwareAllocator::GetResources() const
{
  return this->Resources;
}

bool cmCTestHardwareAllocator::AllocateResource(const std::string& name,
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

bool cmCTestHardwareAllocator::DeallocateResource(const std::string& name,
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

bool cmCTestHardwareAllocator::Resource::operator==(
  const Resource& other) const
{
  return this->Total == other.Total && this->Locked == other.Locked;
}

bool cmCTestHardwareAllocator::Resource::operator!=(
  const Resource& other) const
{
  return !(*this == other);
}
