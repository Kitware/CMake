#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cmCTestResourceAllocator.h"
#include "cmCTestResourceSpec.h"
#include "cmJSONState.h"

static const cmCTestResourceSpec spec{
  { {
    /* clang-format off */
  { "gpus", { { "0", 4 }, { "1", 8 }, { "2", 0 }, { "3", 8 } }, },
    /* clang-format on */
  } },
  cmJSONState()
};

static bool testInitializeFromResourceSpec()
{
  bool retval = true;

  cmCTestResourceAllocator allocator;
  allocator.InitializeFromResourceSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 0 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.GetResources() != expected) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  return retval;
}

static bool testAllocateResource()
{
  bool retval = true;

  cmCTestResourceAllocator allocator;
  allocator.InitializeFromResourceSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected1{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 2 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (!allocator.AllocateResource("gpus", "0", 2)) {
    std::cout
      << "AllocateResource(\"gpus\", \"0\", 2) returned false, should be "
         "true\n";
    retval = false;
  }
  if (allocator.GetResources() != expected1) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected2{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (!allocator.AllocateResource("gpus", "0", 2)) {
    std::cout
      << "AllocateResource(\"gpus\", \"0\", 2) returned false, should be "
         "true\n";
    retval = false;
  }
  if (allocator.GetResources() != expected2) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected3{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.AllocateResource("gpus", "0", 1)) {
    std::cout
      << "AllocateResource(\"gpus\", \"0\", 1) returned true, should be "
         "false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected3) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected4{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 7 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (!allocator.AllocateResource("gpus", "1", 7)) {
    std::cout
      << "AllocateResource(\"gpus\", \"1\", 7) returned false, should be "
         "true\n";
    retval = false;
  }
  if (allocator.AllocateResource("gpus", "1", 2)) {
    std::cout
      << "AllocateResource(\"gpus\", \"1\", 2) returned true, should be "
         "false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected4) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected5{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 7 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.AllocateResource("gpus", "2", 1)) {
    std::cout
      << "AllocateResource(\"gpus\", \"2\", 1) returned true, should be "
         "false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected5) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected6{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 7 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.AllocateResource("gpus", "4", 1)) {
    std::cout
      << "AllocateResource(\"gpus\", \"4\", 1) returned true, should be "
         "false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected6) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected7{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 4 } },
        { "1", { 8, 7 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.AllocateResource("threads", "0", 1)) {
    std::cout
      << "AllocateResource(\"threads\", \"0\", 1) returned true, should be"
         " false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected7) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  return retval;
}

static bool testDeallocateResource()
{
  bool retval = true;

  cmCTestResourceAllocator allocator;
  allocator.InitializeFromResourceSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected1{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 1 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (!allocator.AllocateResource("gpus", "0", 2)) {
    std::cout
      << "AllocateResource(\"gpus\", \"0\", 2) returned false, should be "
         "true\n";
    retval = false;
  }
  if (!allocator.DeallocateResource("gpus", "0", 1)) {
    std::cout
      << "DeallocateResource(\"gpus\", \"0\", 1) returned false, should be"
         " true\n";
    retval = false;
  }
  if (allocator.GetResources() != expected1) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected2{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 1 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.DeallocateResource("gpus", "0", 2)) {
    std::cout
      << "DeallocateResource(\"gpus\", \"0\", 2) returned true, should be"
         " false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected2) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected3{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 0 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (!allocator.DeallocateResource("gpus", "0", 1)) {
    std::cout
      << "DeallocateResource(\"gpus\", \"0\", 1) returned false, should be"
         " true\n";
    retval = false;
  }
  if (allocator.GetResources() != expected3) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected4{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 0 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.DeallocateResource("gpus", "0", 1)) {
    std::cout
      << "DeallocateResource(\"gpus\", \"0\", 1) returned true, should be"
         " false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected4) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected5{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 0 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.DeallocateResource("gpus", "4", 1)) {
    std::cout
      << "DeallocateResource(\"gpus\", \"4\", 1) returned true, should be"
         " false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected5) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  static const std::map<
    std::string, std::map<std::string, cmCTestResourceAllocator::Resource>>
    expected6{
      /* clang-format off */
      { "gpus", {
        { "0", { 4, 0 } },
        { "1", { 8, 0 } },
        { "2", { 0, 0 } },
        { "3", { 8, 0 } },
      } },
      /* clang-format on */
    };
  if (allocator.DeallocateResource("threads", "0", 1)) {
    std::cout
      << "DeallocateResource(\"threads\", \"0\", 1) returned true, should be"
         " false\n";
    retval = false;
  }
  if (allocator.GetResources() != expected6) {
    std::cout << "GetResources() did not return expected value\n";
    retval = false;
  }

  return retval;
}

static bool testResourceFree()
{
  bool retval = true;

  const cmCTestResourceAllocator::Resource r1{ 5, 0 };
  if (r1.Free() != 5) {
    std::cout << "cmCTestResourceAllocator::Resource::Free() did not return "
                 "expected value for { 5, 0 }\n";
    retval = false;
  }

  const cmCTestResourceAllocator::Resource r2{ 3, 2 };
  if (r2.Free() != 1) {
    std::cout << "cmCTestResourceAllocator::Resource::Free() did not return "
                 "expected value for { 3, 2 }\n";
    retval = false;
  }

  const cmCTestResourceAllocator::Resource r3{ 4, 4 };
  if (r3.Free() != 0) {
    std::cout << "cmCTestResourceAllocator::Resource::Free() did not return "
                 "expected value for { 4, 4 }\n";
    retval = false;
  }

  return retval;
}

int testCTestResourceAllocator(int, char** const)
{
  int retval = 0;

  if (!testInitializeFromResourceSpec()) {
    std::cout << "in testInitializeFromResourceSpec()\n";
    retval = -1;
  }

  if (!testAllocateResource()) {
    std::cout << "in testAllocateResource()\n";
    retval = -1;
  }

  if (!testDeallocateResource()) {
    std::cout << "in testDeallocateResource()\n";
    retval = -1;
  }

  if (!testResourceFree()) {
    std::cout << "in testResourceFree()\n";
    retval = -1;
  }

  return retval;
}
