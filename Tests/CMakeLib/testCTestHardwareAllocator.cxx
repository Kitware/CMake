#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "cmCTestHardwareAllocator.h"
#include "cmCTestHardwareSpec.h"

static const cmCTestHardwareSpec spec{ { {
  /* clang-format off */
  { "gpus", { { "0", 4 }, { "1", 8 }, { "2", 0 }, { "3", 8 } } },
  /* clang-format on */
} } };

bool testInitializeFromHardwareSpec()
{
  bool retval = true;

  cmCTestHardwareAllocator allocator;
  allocator.InitializeFromHardwareSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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

bool testAllocateResource()
{
  bool retval = true;

  cmCTestHardwareAllocator allocator;
  allocator.InitializeFromHardwareSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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

bool testDeallocateResource()
{
  bool retval = true;

  cmCTestHardwareAllocator allocator;
  allocator.InitializeFromHardwareSpec(spec);

  static const std::map<
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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
    std::string, std::map<std::string, cmCTestHardwareAllocator::Resource>>
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

bool testResourceFree()
{
  bool retval = true;

  const cmCTestHardwareAllocator::Resource r1{ 5, 0 };
  if (r1.Free() != 5) {
    std::cout << "cmCTestHardwareAllocator::Resource::Free() did not return "
                 "expected value for { 5, 0 }\n";
    retval = false;
  }

  const cmCTestHardwareAllocator::Resource r2{ 3, 2 };
  if (r2.Free() != 1) {
    std::cout << "cmCTestHardwareAllocator::Resource::Free() did not return "
                 "expected value for { 3, 2 }\n";
    retval = false;
  }

  const cmCTestHardwareAllocator::Resource r3{ 4, 4 };
  if (r3.Free() != 0) {
    std::cout << "cmCTestHardwareAllocator::Resource::Free() did not return "
                 "expected value for { 4, 4 }\n";
    retval = false;
  }

  return retval;
}

int testCTestHardwareAllocator(int, char** const)
{
  int retval = 0;

  if (!testInitializeFromHardwareSpec()) {
    std::cout << "in testInitializeFromHardwareSpec()\n";
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
