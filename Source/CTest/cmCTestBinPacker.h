/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "cmCTestResourceAllocator.h"

struct cmCTestBinPackerAllocation
{
  std::size_t ProcessIndex;
  int SlotsNeeded;
  std::string Id;

  bool operator==(cmCTestBinPackerAllocation const& other) const;
  bool operator!=(cmCTestBinPackerAllocation const& other) const;
};

bool cmAllocateCTestResourcesRoundRobin(
  std::map<std::string, cmCTestResourceAllocator::Resource> const& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations);

bool cmAllocateCTestResourcesBlock(
  std::map<std::string, cmCTestResourceAllocator::Resource> const& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations);
