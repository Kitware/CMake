#include <cstddef> // IWYU pragma: keep
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cmCTestBinPacker.h"
#include "cmCTestResourceAllocator.h"

struct ExpectedPackResult
{
  std::vector<int> SlotsNeeded;
  std::map<std::string, cmCTestResourceAllocator::Resource> Resources;
  bool ExpectedReturnValue;
  std::vector<cmCTestBinPackerAllocation> ExpectedRoundRobinAllocations;
  std::vector<cmCTestBinPackerAllocation> ExpectedBlockAllocations;
};

static const std::vector<ExpectedPackResult> expectedResults
{
  /* clang-format off */
  {
    { 2, 2, 2, 2 },
    { { "0", { 4, 0 } }, { "1", { 4, 0 } }, { "2", { 4, 0 } },
      { "3", { 4, 0 } } },
    true,
    {
      { 0, 2, "0" },
      { 1, 2, "1" },
      { 2, 2, "2" },
      { 3, 2, "3" },
    },
    {
      { 0, 2, "0" },
      { 1, 2, "0" },
      { 2, 2, "1" },
      { 3, 2, "1" },
    },
  },
  {
    { 2, 3, 2 },
    { { "0", { 5, 0 } }, { "1", { 2, 0 } } },
    true,
    {
      { 0, 2, "0" },
      { 1, 3, "0" },
      { 2, 2, "1" },
    },
    {
      { 0, 2, "0" },
      { 1, 3, "0" },
      { 2, 2, "1" },
    },
  },
  {
    { 1, 2, 3 },
    { { "0", { 1, 0 } }, { "1", { 2, 0 } }, { "2", { 2, 0 } } },
    false,
    { },
    { },
  },
  {
    { 48, 21, 31, 10, 40 },
    { { "0", { 81, 0 } }, { "1", { 68, 0 } }, { "2", { 20, 0 } },
      { "3", { 13, 0 } } },
    true,
    {
      { 0, 48, "0" },
      { 1, 21, "1" },
      { 2, 31, "0" },
      { 3, 10, "2" },
      { 4, 40, "1" },
    },
    {
      { 0, 48, "0" },
      { 1, 21, "1" },
      { 2, 31, "0" },
      { 3, 10, "2" },
      { 4, 40, "1" },
    },
  },
  {
    { 30, 31, 39, 67 },
    { { "0", { 16, 0 } }, { "1", { 81, 0 } }, { "2", { 97, 0 } } },
    true,
    {
      { 0, 30, "2" },
      { 1, 31, "1" },
      { 2, 39, "1" },
      { 3, 67, "2" },
    },
    {
      { 0, 30, "2" },
      { 1, 31, "1" },
      { 2, 39, "1" },
      { 3, 67, "2" },
    },
  },
  {
    { 63, 47, 1, 9 },
    { { "0", { 18, 0 } }, { "1", { 29, 0 } }, { "2", { 9, 0 } },
      { "3", { 52, 0 } } },
    false,
    { },
    { },
  },
  {
    { 22, 29, 46, 85 },
    { { "0", { 65, 0 } }, { "1", { 85, 0 } }, { "2", { 65, 0 } },
      { "3", { 78, 0 } } },
    true,
    {
      { 0, 22, "2" },
      { 1, 29, "0" },
      { 2, 46, "3" },
      { 3, 85, "1" },
    },
    {
      { 0, 22, "0" },
      { 1, 29, "3" },
      { 2, 46, "3" },
      { 3, 85, "1" },
    },
  },
  {
    { 66, 11, 34, 21 },
    { { "0", { 24, 0 } }, { "1", { 57, 0 } }, { "2", { 61, 0 } },
      { "3", { 51, 0 } } },
    false,
    { },
    { },
  },
  {
    { 72, 65, 67, 45 },
    { { "0", { 29, 0 } }, { "1", { 77, 0 } }, { "2", { 98, 0 } },
      { "3", { 58, 0 } } },
    false,
    { },
    { },
  },
  /*
   * The following is a contrived attack on the bin-packing algorithm that
   * causes it to execute with n! complexity, where n is the number of
   * resources. This case is very unrepresentative of real-world usage, and
   * has been documented but disabled. The bin-packing problem is NP-hard, and
   * we may not be able to fix this case at all.
   */
#if 0
  {
    { 1000, 999, 998, 997, 996, 995, 994, 993, 992, 991, 19 },
    { { "0", { 1000, 0 } }, { "1", { 1001, 0 } }, { "2", { 1002, 0 } },
      { "3", { 1003, 0 } }, { "4", { 1004, 0 } }, { "5", { 1005, 0 } },
      { "6", { 1006, 0 } }, { "7", { 1007, 0 } }, { "8", { 1008, 0 } },
      { "9", { 1009, 0 } } },
    false,
    { },
    { },
  },
#endif
  /*
   * These cases are more representative of real-world usage (the resource
   * sizes are all the same.)
   */
  {
    { 1000, 999, 998, 997, 996, 995, 994, 993, 992, 991, 10 },
    { { "0", { 1000, 0 } }, { "1", { 1000, 0 } }, { "2", { 1000, 0 } },
      { "3", { 1000, 0 } }, { "4", { 1000, 0 } }, { "5", { 1000, 0 } },
      { "6", { 1000, 0 } }, { "7", { 1000, 0 } }, { "8", { 1000, 0 } },
      { "9", { 1000, 0 } } },
    false,
    { },
    { },
  },
  {
    { 1000, 999, 998, 997, 996, 995, 994, 993, 992, 991, 9 },
    { { "0", { 1000, 0 } }, { "1", { 1000, 0 } }, { "2", { 1000, 0 } },
      { "3", { 1000, 0 } }, { "4", { 1000, 0 } }, { "5", { 1000, 0 } },
      { "6", { 1000, 0 } }, { "7", { 1000, 0 } }, { "8", { 1000, 0 } },
      { "9", { 1000, 0 } } },
    true,
    {
      { 0, 1000, "0" },
      { 1, 999, "1" },
      { 2, 998, "2" },
      { 3, 997, "3" },
      { 4, 996, "4" },
      { 5, 995, "5" },
      { 6, 994, "6" },
      { 7, 993, "7" },
      { 8, 992, "8" },
      { 9, 991, "9" },
      { 10, 9, "9" },
    },
    {
      { 0, 1000, "0" },
      { 1, 999, "1" },
      { 2, 998, "2" },
      { 3, 997, "3" },
      { 4, 996, "4" },
      { 5, 995, "5" },
      { 6, 994, "6" },
      { 7, 993, "7" },
      { 8, 992, "8" },
      { 9, 991, "9" },
      { 10, 9, "9" },
    },
  },
  /* clang-format on */
};

struct AllocationComparison
{
  cmCTestBinPackerAllocation First;
  cmCTestBinPackerAllocation Second;
  bool Equal;
};

static const std::vector<AllocationComparison> comparisons{
  /* clang-format off */
  { { 0, 1, "0" }, { 0, 1, "0" }, true },
  { { 0, 1, "0" }, { 1, 1, "0" }, false },
  { { 0, 1, "0" }, { 0, 2, "0" }, false },
  { { 0, 1, "0" }, { 0, 1, "1" }, false },
  /* clang-format on */
};

static bool TestExpectedPackResult(const ExpectedPackResult& expected)
{
  std::vector<cmCTestBinPackerAllocation> roundRobinAllocations;
  roundRobinAllocations.reserve(expected.SlotsNeeded.size());
  std::size_t index = 0;
  for (auto const& n : expected.SlotsNeeded) {
    roundRobinAllocations.push_back({ index++, n, "" });
  }

  bool roundRobinResult = cmAllocateCTestResourcesRoundRobin(
    expected.Resources, roundRobinAllocations);
  if (roundRobinResult != expected.ExpectedReturnValue) {
    std::cout
      << "cmAllocateCTestResourcesRoundRobin did not return expected value"
      << std::endl;
    return false;
  }

  if (roundRobinResult &&
      roundRobinAllocations != expected.ExpectedRoundRobinAllocations) {
    std::cout << "cmAllocateCTestResourcesRoundRobin did not return expected "
                 "allocations"
              << std::endl;
    return false;
  }

  std::vector<cmCTestBinPackerAllocation> blockAllocations;
  blockAllocations.reserve(expected.SlotsNeeded.size());
  index = 0;
  for (auto const& n : expected.SlotsNeeded) {
    blockAllocations.push_back({ index++, n, "" });
  }

  bool blockResult =
    cmAllocateCTestResourcesBlock(expected.Resources, blockAllocations);
  if (blockResult != expected.ExpectedReturnValue) {
    std::cout << "cmAllocateCTestResourcesBlock did not return expected value"
              << std::endl;
    return false;
  }

  if (blockResult && blockAllocations != expected.ExpectedBlockAllocations) {
    std::cout << "cmAllocateCTestResourcesBlock did not return expected"
                 " allocations"
              << std::endl;
    return false;
  }

  return true;
}

int testCTestBinPacker(int /*unused*/, char* /*unused*/ [])
{
  int retval = 0;

  for (auto const& comparison : comparisons) {
    if ((comparison.First == comparison.Second) != comparison.Equal) {
      std::cout << "Comparison did not match expected" << std::endl;
      retval = 1;
    }
    if ((comparison.First != comparison.Second) == comparison.Equal) {
      std::cout << "Comparison did not match expected" << std::endl;
      retval = 1;
    }
  }

  for (auto const& expected : expectedResults) {
    if (!TestExpectedPackResult(expected)) {
      retval = 1;
    }
  }

  return retval;
}
