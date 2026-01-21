/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's ELF parser
 *
 * CMake parses ELF files to extract RPATH, RUNPATH, and SONAME information.
 * Malformed ELF files from untrusted sources could trigger vulnerabilities.
 *
 * Coverage targets:
 * - ELF header parsing (32-bit and 64-bit)
 * - Section header parsing
 * - Dynamic section parsing
 * - String table extraction
 * - RPATH/RUNPATH/SONAME extraction
 *
 * Performance notes:
 * - Uses memfd_create on Linux for memory-backed file I/O
 * - Falls back to temp files on other platforms
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#include <unistd.h>

#include "cmELF.h"

#ifdef __linux__
#  include <sys/mman.h>
#  ifndef MFD_CLOEXEC
#    define MFD_CLOEXEC 0x0001U
#  endif
#endif

// ELF files can be large, but we limit to avoid timeouts
static constexpr size_t kMinInputSize = 16;         // Minimum ELF header
static constexpr size_t kMaxInputSize = 512 * 1024; // 512KB

static int g_memfd = -1;
static std::string g_memfdPath;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

#ifdef __linux__
  g_memfd = memfd_create("cmake_fuzz_elf", MFD_CLOEXEC);
  if (g_memfd >= 0) {
    g_memfdPath = "/proc/self/fd/" + std::to_string(g_memfd);
  }
#endif
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  // ELF files need minimum header size
  if (size < kMinInputSize || size > kMaxInputSize) {
    return 0;
  }

  std::string testFile;

#ifdef __linux__
  if (g_memfd >= 0) {
    // Use memfd for better performance
    ftruncate(g_memfd, 0);
    lseek(g_memfd, 0, SEEK_SET);
    if (write(g_memfd, data, size) != static_cast<ssize_t>(size)) {
      return 0;
    }
    testFile = g_memfdPath;
  } else
#endif
  {
    // Fallback to temp file
    char tmpFile[] = "/tmp/fuzz_elf_XXXXXX";
    int fd = mkstemp(tmpFile);
    if (fd < 0) {
      return 0;
    }
    if (write(fd, data, size) != static_cast<ssize_t>(size)) {
      close(fd);
      unlink(tmpFile);
      return 0;
    }
    close(fd);
    testFile = tmpFile;
  }

  // Parse the ELF file
  {
    cmELF elf(testFile.c_str());

    // Check validity
    if (elf) {
      // Exercise all the parsing functions
      (void)elf.GetFileType();
      (void)elf.GetMachine();
      (void)elf.GetNumberOfSections();
      (void)elf.HasDynamicSection();
      (void)elf.IsMIPS();

      // Try to get string entries
      std::string soname;
      elf.GetSOName(soname);
      (void)elf.GetSOName();
      (void)elf.GetRPath();
      (void)elf.GetRunPath();

      // Get dynamic entries
      auto entries = elf.GetDynamicEntries();
      if (!entries.empty()) {
        auto encoded = elf.EncodeDynamicEntries(entries);
        (void)encoded;

        // Get positions (limit iterations to avoid timeout)
        for (size_t i = 0; i < entries.size() && i < 100; ++i) {
          (void)elf.GetDynamicEntryPosition(static_cast<int>(i));
        }
      }

      // Print info to exercise that code path
      std::ostringstream oss;
      elf.PrintInfo(oss);
    }

    // Always check error message path
    (void)elf.GetErrorMessage();
  }

  // Cleanup temp file (memfd doesn't need cleanup)
#ifdef __linux__
  if (g_memfd < 0)
#endif
  {
    unlink(testFile.c_str());
  }

  return 0;
}
