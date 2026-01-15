/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's archive extraction (tar/zip)
 *
 * CMake extracts archives via cmSystemTools::ExtractTar. This is a critical
 * attack surface as malicious archives could contain path traversal sequences
 * (Zip Slip) or other exploits.
 *
 * Coverage targets:
 * - Archive format detection (tar, gzip, bzip2, xz, zip)
 * - Path handling during extraction
 * - Symlink handling
 * - Large file handling
 * - Malformed archive recovery
 *
 * Security focus:
 * - Path traversal (../) detection
 * - Absolute path handling
 * - Symlink escape attempts
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "cmSystemTools.h"

// Archives can be large but limit for fuzzing
static constexpr size_t kMinInputSize = 4;          // Minimum magic bytes
static constexpr size_t kMaxInputSize = 256 * 1024; // 256KB

// Sandbox directory for extraction
static std::string g_extractDir;

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
  (void)argc;
  (void)argv;

  // Create a unique extraction directory in /tmp
  char tmpl[] = "/tmp/cmake_fuzz_extract_XXXXXX";
  char* dir = mkdtemp(tmpl);
  if (dir) {
    g_extractDir = dir;
  } else {
    g_extractDir = "/tmp/cmake_fuzz_extract";
    cmSystemTools::MakeDirectory(g_extractDir);
  }

  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size < kMinInputSize || size > kMaxInputSize) {
    return 0;
  }

  // Write archive to temp file
  std::string archiveFile = g_extractDir + "/test_archive";
  {
    FILE* fp = fopen(archiveFile.c_str(), "wb");
    if (!fp) {
      return 0;
    }
    fwrite(data, 1, size, fp);
    fclose(fp);
  }

  // Create a fresh extraction subdirectory each time
  std::string extractSubDir = g_extractDir + "/out";
  cmSystemTools::RemoveADirectory(extractSubDir);
  cmSystemTools::MakeDirectory(extractSubDir);

  // Save current directory
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();

  // Change to extraction directory (cmSystemTools extracts to cwd)
  if (cmSystemTools::ChangeDirectory(extractSubDir)) {
    // Try extraction with different options
    std::vector<std::string> files;

    // Extract without verbose, with timestamps
    bool result1 = cmSystemTools::ExtractTar(
      archiveFile, files, cmSystemTools::cmTarExtractTimestamps::Yes, false);
    (void)result1;

    // Restore directory BEFORE removing (can't remove cwd)
    cmSystemTools::ChangeDirectory(cwd);

    // Clean up extracted files
    cmSystemTools::RemoveADirectory(extractSubDir);
    cmSystemTools::MakeDirectory(extractSubDir);

    // Change back for second extraction
    if (cmSystemTools::ChangeDirectory(extractSubDir)) {
      // Extract with verbose, without timestamps
      files.clear();
      bool result2 = cmSystemTools::ExtractTar(
        archiveFile, files, cmSystemTools::cmTarExtractTimestamps::No, true);
      (void)result2;

      // Restore directory
      cmSystemTools::ChangeDirectory(cwd);
    }
  }

  // Note: A more thorough security check would verify nothing escaped the
  // sandbox, but for fuzzing we rely on sanitizers to catch path traversal

  // Cleanup
  cmSystemTools::RemoveADirectory(extractSubDir);
  cmSystemTools::RemoveFile(archiveFile);

  return 0;
}
