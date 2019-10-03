/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestBinPacker.h"

#include <algorithm>
#include <utility>

bool cmCTestBinPackerAllocation::operator==(
  const cmCTestBinPackerAllocation& other) const
{
  return this->ProcessIndex == other.ProcessIndex &&
    this->SlotsNeeded == other.SlotsNeeded && this->Id == other.Id;
}

bool cmCTestBinPackerAllocation::operator!=(
  const cmCTestBinPackerAllocation& other) const
{
  return !(*this == other);
}

namespace {

/*
 * The following algorithm is used to do two things:
 *
 * 1) Determine if a test's hardware requirements can fit within the hardware
 *    present on the system, and
 * 2) Do the actual allocation
 *
 * This algorithm performs a recursive search, looking for a bin pack that will
 * fit the specified requirements. It has a template to specify different
 * optimization strategies. If it ever runs out of room, it backtracks as far
 * down the stack as it needs to and tries a different combination until no
 * more combinations can be tried.
 */
template <typename AllocationStrategy>
static bool AllocateCTestHardware(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  const std::vector<std::string>& hardwareSorted, std::size_t currentIndex,
  std::vector<cmCTestBinPackerAllocation*>& allocations)
{
  // Iterate through all large enough resources until we find a solution
  std::size_t hardwareIndex = 0;
  while (hardwareIndex < hardwareSorted.size()) {
    auto const& resource = hardware.at(hardwareSorted[hardwareIndex]);
    if (resource.Free() >=
        static_cast<unsigned int>(allocations[currentIndex]->SlotsNeeded)) {
      // Preemptively allocate the resource
      allocations[currentIndex]->Id = hardwareSorted[hardwareIndex];
      if (currentIndex + 1 >= allocations.size()) {
        // We have a solution
        return true;
      }

      // Move the resource up the list until it is sorted again
      auto hardware2 = hardware;
      auto hardwareSorted2 = hardwareSorted;
      hardware2[hardwareSorted2[hardwareIndex]].Locked +=
        allocations[currentIndex]->SlotsNeeded;
      AllocationStrategy::IncrementalSort(hardware2, hardwareSorted2,
                                          hardwareIndex);

      // Recurse one level deeper
      if (AllocateCTestHardware<AllocationStrategy>(
            hardware2, hardwareSorted2, currentIndex + 1, allocations)) {
        return true;
      }
    }

    // No solution found here, deallocate the resource and try the next one
    allocations[currentIndex]->Id.clear();
    auto freeSlots = hardware.at(hardwareSorted.at(hardwareIndex)).Free();
    do {
      ++hardwareIndex;
    } while (hardwareIndex < hardwareSorted.size() &&
             hardware.at(hardwareSorted.at(hardwareIndex)).Free() ==
               freeSlots);
  }

  // No solution was found
  return false;
}

template <typename AllocationStrategy>
static bool AllocateCTestHardware(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<cmCTestBinPackerAllocation>& allocations)
{
  // Sort the resource requirements in descending order by slots needed
  std::vector<cmCTestBinPackerAllocation*> allocationsPtr;
  allocationsPtr.reserve(allocations.size());
  for (auto& allocation : allocations) {
    allocationsPtr.push_back(&allocation);
  }
  std::stable_sort(
    allocationsPtr.rbegin(), allocationsPtr.rend(),
    [](cmCTestBinPackerAllocation* a1, cmCTestBinPackerAllocation* a2) {
      return a1->SlotsNeeded < a2->SlotsNeeded;
    });

  // Sort the resources according to sort strategy
  std::vector<std::string> hardwareSorted;
  hardwareSorted.reserve(hardware.size());
  for (auto const& hw : hardware) {
    hardwareSorted.push_back(hw.first);
  }
  AllocationStrategy::InitialSort(hardware, hardwareSorted);

  // Do the actual allocation
  return AllocateCTestHardware<AllocationStrategy>(
    hardware, hardwareSorted, std::size_t(0), allocationsPtr);
}

class RoundRobinAllocationStrategy
{
public:
  static void InitialSort(
    const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
    std::vector<std::string>& hardwareSorted);

  static void IncrementalSort(
    const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
    std::vector<std::string>& hardwareSorted, std::size_t lastAllocatedIndex);
};

void RoundRobinAllocationStrategy::InitialSort(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<std::string>& hardwareSorted)
{
  std::stable_sort(
    hardwareSorted.rbegin(), hardwareSorted.rend(),
    [&hardware](const std::string& id1, const std::string& id2) {
      return hardware.at(id1).Free() < hardware.at(id2).Free();
    });
}

void RoundRobinAllocationStrategy::IncrementalSort(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<std::string>& hardwareSorted, std::size_t lastAllocatedIndex)
{
  auto tmp = hardwareSorted[lastAllocatedIndex];
  std::size_t i = lastAllocatedIndex;
  while (i < hardwareSorted.size() - 1 &&
         hardware.at(hardwareSorted[i + 1]).Free() > hardware.at(tmp).Free()) {
    hardwareSorted[i] = hardwareSorted[i + 1];
    ++i;
  }
  hardwareSorted[i] = tmp;
}

class BlockAllocationStrategy
{
public:
  static void InitialSort(
    const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
    std::vector<std::string>& hardwareSorted);

  static void IncrementalSort(
    const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
    std::vector<std::string>& hardwareSorted, std::size_t lastAllocatedIndex);
};

void BlockAllocationStrategy::InitialSort(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<std::string>& hardwareSorted)
{
  std::stable_sort(
    hardwareSorted.rbegin(), hardwareSorted.rend(),
    [&hardware](const std::string& id1, const std::string& id2) {
      return hardware.at(id1).Free() < hardware.at(id2).Free();
    });
}

void BlockAllocationStrategy::IncrementalSort(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>&,
  std::vector<std::string>& hardwareSorted, std::size_t lastAllocatedIndex)
{
  auto tmp = hardwareSorted[lastAllocatedIndex];
  std::size_t i = lastAllocatedIndex;
  while (i > 0) {
    hardwareSorted[i] = hardwareSorted[i - 1];
    --i;
  }
  hardwareSorted[i] = tmp;
}
}

bool cmAllocateCTestHardwareRoundRobin(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<cmCTestBinPackerAllocation>& allocations)
{
  return AllocateCTestHardware<RoundRobinAllocationStrategy>(hardware,
                                                             allocations);
}

bool cmAllocateCTestHardwareBlock(
  const std::map<std::string, cmCTestHardwareAllocator::Resource>& hardware,
  std::vector<cmCTestBinPackerAllocation>& allocations)
{
  return AllocateCTestHardware<BlockAllocationStrategy>(hardware, allocations);
}
