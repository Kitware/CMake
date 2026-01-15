/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's list file parser (CMakeLists.txt full parsing)
 *
 * This fuzzer tests the complete parsing of CMake files including:
 * - Command parsing
 * - Argument handling
 * - Bracket arguments
 * - Comments
 * - Line continuations
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

#include <unistd.h>

#include "cmListFileCache.h"
#include "cmMessenger.h"
#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 256 * 1024;
static std::string g_testDir;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  char tmpl[] = "/tmp/cmake_fuzz_listfile_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_testDir = dir;
  } else {
    g_testDir = "/tmp/cmake_fuzz_listfile";
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
  std::string testFile = g_testDir + "/CMakeLists.txt";
  {
    FILE* fp = fopen(testFile.c_str(), "wb");
    if (!fp)
      return 0;
    fwrite(data, 1, size, fp);
    fclose(fp);
  }

  // Create a messenger for error handling
  cmMessenger messenger;

  // Parse the file
  cmListFile listFile;
  cmListFileBacktrace backtrace;
  if (listFile.ParseFile(testFile.c_str(), &messenger, backtrace)) {
    // Successfully parsed - examine results
    for (auto const& func : listFile.Functions) {
      (void)func.LowerCaseName();
      (void)func.OriginalName();
      for (auto const& arg : func.Arguments()) {
        (void)arg.Value;
        (void)arg.Delim;
      }
    }
  }

  // Clean up
  unlink(testFile.c_str());

  return 0;
}
