/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCTestBinPacker_h
#define cmCTestBinPacker_h

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "cmCTestHardwareAllocator.h"

struct cmCTestBinPackerAllocation
{
  std::size_t ProcessIndex;
  int SlotsNeeded;
  std::string Id;

  bool operator==(const cmCTestBinPackerAllocation& other) const;
  bool operator!=(const cmCTestBinPackerAllocation& other) const;
};

bool cmAllocateCTestHardwareRoundRobin(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<cmCTestBinPackerAllocation>& allocations);

bool cmAllocateCTestHardwareBlock(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<cmCTestBinPackerAllocation>& allocations);

#endif
