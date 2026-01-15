/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's path manipulation (cmCMakePath)
 *
 * Tests path parsing, normalization, and manipulation operations.
 */

#include <cstddef>
#include <cstdint>
#include <string>

#include "cmCMakePath.h"

static constexpr size_t kMaxInputSize = 4096;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  // Test various path operations
  cmCMakePath path(input);

  // Basic queries
  (void)path.IsEmpty();
  (void)path.HasRootPath();
  (void)path.HasRootName();
  (void)path.HasRootDirectory();
  (void)path.HasRelativePath();
  (void)path.HasParentPath();
  (void)path.HasFileName();
  (void)path.HasStem();
  (void)path.HasExtension();
  (void)path.IsAbsolute();
  (void)path.IsRelative();

  // Component extraction
  (void)path.GetRootName();
  (void)path.GetRootDirectory();
  (void)path.GetRootPath();
  (void)path.GetRelativePath();
  (void)path.GetParentPath();
  (void)path.GetFileName();
  (void)path.GetExtension();
  (void)path.GetStem();
  (void)path.String();
  (void)path.GenericString();

  // Manipulation
  cmCMakePath normalized = path.Normal();
  (void)normalized.String();

  // If we have two paths in input, test operations between them
  size_t midpoint = size / 2;
  if (midpoint > 0) {
    std::string input1(reinterpret_cast<char const*>(data), midpoint);
    std::string input2(reinterpret_cast<char const*>(data + midpoint),
                       size - midpoint);

    cmCMakePath path1(input1);
    cmCMakePath path2(input2);

    // Append operations
    cmCMakePath appended = path1 / path2;
    (void)appended.String();

    // Comparison
    (void)(path1 == path2);
  }

  return 0;
}
