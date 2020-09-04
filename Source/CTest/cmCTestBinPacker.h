/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

  bool operator==(const cmCTestBinPackerAllocation& other) const;
  bool operator!=(const cmCTestBinPackerAllocation& other) const;
};

bool cmAllocateCTestResourcesRoundRobin(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations);

bool cmAllocateCTestResourcesBlock(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations);
