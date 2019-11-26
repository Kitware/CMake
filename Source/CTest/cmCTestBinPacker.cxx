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
 * 1) Determine if a test's resource requirements can fit within the resources
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
static bool AllocateCTestResources(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  const std::vector<std::string>& resourcesSorted, std::size_t currentIndex,
  std::vector<cmCTestBinPackerAllocation*>& allocations)
{
  // Iterate through all large enough resources until we find a solution
  std::size_t resourceIndex = 0;
  while (resourceIndex < resourcesSorted.size()) {
    auto const& resource = resources.at(resourcesSorted[resourceIndex]);
    if (resource.Free() >=
        static_cast<unsigned int>(allocations[currentIndex]->SlotsNeeded)) {
      // Preemptively allocate the resource
      allocations[currentIndex]->Id = resourcesSorted[resourceIndex];
      if (currentIndex + 1 >= allocations.size()) {
        // We have a solution
        return true;
      }

      // Move the resource up the list until it is sorted again
      auto resources2 = resources;
      auto resourcesSorted2 = resourcesSorted;
      resources2[resourcesSorted2[resourceIndex]].Locked +=
        allocations[currentIndex]->SlotsNeeded;
      AllocationStrategy::IncrementalSort(resources2, resourcesSorted2,
                                          resourceIndex);

      // Recurse one level deeper
      if (AllocateCTestResources<AllocationStrategy>(
            resources2, resourcesSorted2, currentIndex + 1, allocations)) {
        return true;
      }
    }

    // No solution found here, deallocate the resource and try the next one
    allocations[currentIndex]->Id.clear();
    auto freeSlots = resources.at(resourcesSorted.at(resourceIndex)).Free();
    do {
      ++resourceIndex;
    } while (resourceIndex < resourcesSorted.size() &&
             resources.at(resourcesSorted.at(resourceIndex)).Free() ==
               freeSlots);
  }

  // No solution was found
  return false;
}

template <typename AllocationStrategy>
static bool AllocateCTestResources(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
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
  std::vector<std::string> resourcesSorted;
  resourcesSorted.reserve(resources.size());
  for (auto const& res : resources) {
    resourcesSorted.push_back(res.first);
  }
  AllocationStrategy::InitialSort(resources, resourcesSorted);

  // Do the actual allocation
  return AllocateCTestResources<AllocationStrategy>(
    resources, resourcesSorted, std::size_t(0), allocationsPtr);
}

class RoundRobinAllocationStrategy
{
public:
  static void InitialSort(
    const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
    std::vector<std::string>& resourcesSorted);

  static void IncrementalSort(
    const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
    std::vector<std::string>& resourcesSorted, std::size_t lastAllocatedIndex);
};

void RoundRobinAllocationStrategy::InitialSort(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<std::string>& resourcesSorted)
{
  std::stable_sort(
    resourcesSorted.rbegin(), resourcesSorted.rend(),
    [&resources](const std::string& id1, const std::string& id2) {
      return resources.at(id1).Free() < resources.at(id2).Free();
    });
}

void RoundRobinAllocationStrategy::IncrementalSort(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<std::string>& resourcesSorted, std::size_t lastAllocatedIndex)
{
  auto tmp = resourcesSorted[lastAllocatedIndex];
  std::size_t i = lastAllocatedIndex;
  while (i < resourcesSorted.size() - 1 &&
         resources.at(resourcesSorted[i + 1]).Free() >
           resources.at(tmp).Free()) {
    resourcesSorted[i] = resourcesSorted[i + 1];
    ++i;
  }
  resourcesSorted[i] = tmp;
}

class BlockAllocationStrategy
{
public:
  static void InitialSort(
    const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
    std::vector<std::string>& resourcesSorted);

  static void IncrementalSort(
    const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
    std::vector<std::string>& resourcesSorted, std::size_t lastAllocatedIndex);
};

void BlockAllocationStrategy::InitialSort(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<std::string>& resourcesSorted)
{
  std::stable_sort(
    resourcesSorted.rbegin(), resourcesSorted.rend(),
    [&resources](const std::string& id1, const std::string& id2) {
      return resources.at(id1).Free() < resources.at(id2).Free();
    });
}

void BlockAllocationStrategy::IncrementalSort(
  const std::map<std::string, cmCTestResourceAllocator::Resource>&,
  std::vector<std::string>& resourcesSorted, std::size_t lastAllocatedIndex)
{
  auto tmp = resourcesSorted[lastAllocatedIndex];
  std::size_t i = lastAllocatedIndex;
  while (i > 0) {
    resourcesSorted[i] = resourcesSorted[i - 1];
    --i;
  }
  resourcesSorted[i] = tmp;
}
}

bool cmAllocateCTestResourcesRoundRobin(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations)
{
  return AllocateCTestResources<RoundRobinAllocationStrategy>(resources,
                                                              allocations);
}

bool cmAllocateCTestResourcesBlock(
  const std::map<std::string, cmCTestResourceAllocator::Resource>& resources,
  std::vector<cmCTestBinPackerAllocation>& allocations)
{
  return AllocateCTestResources<BlockAllocationStrategy>(resources,
                                                         allocations);
}
