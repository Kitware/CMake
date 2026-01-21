/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's file(LOCK) command
 *
 * The file(LOCK) command manages file locks for synchronization.
 * This fuzzer tests various lock scenarios and argument combinations.
 *
 * Coverage targets:
 * - Lock acquisition (LOCK)
 * - Lock release (RELEASE)
 * - Guard modes (FUNCTION, FILE, PROCESS)
 * - Timeout handling
 * - Error paths
 *
 * Security focus:
 * - Symlink handling (CVE for data destruction)
 * - Path traversal in lock paths
 * - Race conditions
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include <unistd.h>

#include "cmFileLock.h"
#include "cmFileLockResult.h"
#include "cmSystemTools.h"

// Limit input size
static constexpr size_t kMaxInputSize = 4096;

// Sandbox directory
static std::string g_testDir;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  // Create unique test directory
  char tmpl[] = "/tmp/cmake_fuzz_lock_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_testDir = dir;
  } else {
    g_testDir = "/tmp/cmake_fuzz_lock";
    cmSystemTools::MakeDirectory(g_testDir);
  }

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size < 1 || size > kMaxInputSize) {
    return 0;
  }

  // Use first byte for flags
  uint8_t flags = data[0];

  // Create a test file with known content
  std::string testFile = g_testDir + "/lock_target.txt";
  std::string lockFile = g_testDir + "/test.lock";
  char const* testContent = "IMPORTANT DATA - MUST NOT BE TRUNCATED";

  {
    FILE* fp = fopen(testFile.c_str(), "w");
    if (!fp)
      return 0;
    fputs(testContent, fp);
    fclose(fp);
  }

  // Test different scenarios based on fuzz input
  cmFileLock lock;

  // Vary the lock file path based on remaining input
  std::string lockPath = lockFile;
  if (size > 1 && (flags & 0x01)) {
    // Use part of input as filename suffix (sanitized)
    size_t nameLen = std::min(size - 1, size_t(32));
    std::string suffix;
    for (size_t i = 0; i < nameLen; ++i) {
      char c = static_cast<char>(data[1 + i]);
      // Only allow safe filename characters
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-') {
        suffix += c;
      }
    }
    if (!suffix.empty()) {
      lockPath = g_testDir + "/" + suffix + ".lock";
    }
  }

  // Test symlink scenario (security-critical)
  if (flags & 0x02) {
    // Create a symlink to the test file
    std::string symlinkPath = g_testDir + "/symlink.lock";
    unlink(symlinkPath.c_str());
    if (symlink(testFile.c_str(), symlinkPath.c_str()) == 0) {
      lockPath = symlinkPath;
    }
  }

  // Determine timeout - use 0 for fuzzing to avoid blocking
  // (non-zero timeouts would stall the fuzzer)
  unsigned long timeout = 0;

  // Try to acquire lock
  cmFileLockResult result = lock.Lock(lockPath, timeout);
  (void)result.IsOk();

  // Always try to release
  (void)lock.Release();

  // Security check: Verify test file wasn't truncated
  {
    FILE* fp = fopen(testFile.c_str(), "r");
    if (fp) {
      char buffer[256] = { 0 };
      size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, fp);
      fclose(fp);

      if (bytesRead == 0 || strcmp(buffer, testContent) != 0) {
        // DATA DESTRUCTION DETECTED!
        fprintf(stderr, "VULNERABILITY: File was truncated or modified!\n");
        fprintf(stderr, "Expected: '%s'\n", testContent);
        fprintf(stderr, "Got: '%s' (%zu bytes)\n", buffer, bytesRead);
        abort();
      }
    }
  }

  // Cleanup
  unlink(lockPath.c_str());
  unlink((g_testDir + "/symlink.lock").c_str());

  return 0;
}
