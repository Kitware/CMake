/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's GCC dependency file parser
 *
 * GCC generates .d files with make-style dependencies.
 * This fuzzer tests parsing of these files.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

#include <unistd.h>

#include "cmGccDepfileReader.h"
#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 256 * 1024;
static std::string g_testDir;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  char tmpl[] = "/tmp/cmake_fuzz_depfile_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_testDir = dir;
  } else {
    g_testDir = "/tmp/cmake_fuzz_depfile";
    cmSystemTools::MakeDirectory(g_testDir);
  }

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  // Write input to temp file
  std::string depFile = g_testDir + "/test.d";
  {
    FILE* fp = fopen(depFile.c_str(), "wb");
    if (!fp)
      return 0;
    fwrite(data, 1, size, fp);
    fclose(fp);
  }

  // Parse the depfile
  auto result = cmReadGccDepfile(depFile.c_str());

  // Examine results if parsing succeeded
  if (result) {
    for (auto const& entry : *result) {
      for (auto const& rule : entry.rules) {
        (void)rule;
      }
      for (auto const& path : entry.paths) {
        (void)path;
      }
    }
  }

  // Clean up
  unlink(depFile.c_str());

  return 0;
}
