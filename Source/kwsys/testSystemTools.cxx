/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"

#if defined(_MSC_VER)
#  pragma warning(disable : 4786)
#endif

#include KWSYS_HEADER(FStream.hxx)
#include KWSYS_HEADER(SystemTools.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#  include "FStream.hxx.in"
#  include "SystemTools.hxx.in"
#endif

// Include with <> instead of "" to avoid getting any in-source copy
// left on disk.
#include <testSystemTools.h>

#include <cstdlib> /* free */
#include <cstring> /* strcmp */
#include <iostream>
#include <sstream>
#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <io.h> /* _umask (MSVC) */
#  ifdef _MSC_VER
#    define umask _umask
#  endif
#  include <windows.h>
#endif
#include <sys/stat.h> /* umask (POSIX), _S_I* constants (Windows) */
// Visual C++ does not define mode_t.
#if defined(_MSC_VER)
typedef unsigned short mode_t;
#endif

static const char* toUnixPaths[][2] = {
  { "/usr/local/bin/passwd", "/usr/local/bin/passwd" },
  { "/usr/lo cal/bin/pa sswd", "/usr/lo cal/bin/pa sswd" },
  { "/usr/lo\\ cal/bin/pa\\ sswd", "/usr/lo/ cal/bin/pa/ sswd" },
  { "c:/usr/local/bin/passwd", "c:/usr/local/bin/passwd" },
  { "c:/usr/lo cal/bin/pa sswd", "c:/usr/lo cal/bin/pa sswd" },
  { "c:/usr/lo\\ cal/bin/pa\\ sswd", "c:/usr/lo/ cal/bin/pa/ sswd" },
  { "\\usr\\local\\bin\\passwd", "/usr/local/bin/passwd" },
  { "\\usr\\lo cal\\bin\\pa sswd", "/usr/lo cal/bin/pa sswd" },
  { "\\usr\\lo\\ cal\\bin\\pa\\ sswd", "/usr/lo/ cal/bin/pa/ sswd" },
  { "c:\\usr\\local\\bin\\passwd", "c:/usr/local/bin/passwd" },
  { "c:\\usr\\lo cal\\bin\\pa sswd", "c:/usr/lo cal/bin/pa sswd" },
  { "c:\\usr\\lo\\ cal\\bin\\pa\\ sswd", "c:/usr/lo/ cal/bin/pa/ sswd" },
  { "\\\\usr\\local\\bin\\passwd", "//usr/local/bin/passwd" },
  { "\\\\usr\\lo cal\\bin\\pa sswd", "//usr/lo cal/bin/pa sswd" },
  { "\\\\usr\\lo\\ cal\\bin\\pa\\ sswd", "//usr/lo/ cal/bin/pa/ sswd" },
  { nullptr, nullptr }
};

static bool CheckConvertToUnixSlashes(std::string const& input,
                                      std::string const& output)
{
  std::string result = input;
  kwsys::SystemTools::ConvertToUnixSlashes(result);
  if (result != output) {
    std::cerr << "Problem with ConvertToUnixSlashes - input: " << input
              << " output: " << result << " expected: " << output << std::endl;
    return false;
  }
  return true;
}

static const char* checkEscapeChars[][4] = {
  { "1 foo 2 bar 2", "12", "\\", "\\1 foo \\2 bar \\2" },
  { " {} ", "{}", "#", " #{#} " },
  { nullptr, nullptr, nullptr, nullptr }
};

static bool CheckEscapeChars(std::string const& input,
                             const char* chars_to_escape, char escape_char,
                             std::string const& output)
{
  std::string result = kwsys::SystemTools::EscapeChars(
    input.c_str(), chars_to_escape, escape_char);
  if (result != output) {
    std::cerr << "Problem with CheckEscapeChars - input: " << input
              << " output: " << result << " expected: " << output << std::endl;
    return false;
  }
  return true;
}

static bool CheckFileOperations()
{
  bool res = true;
  const std::string testNonExistingFile(TEST_SYSTEMTOOLS_SOURCE_DIR
                                        "/testSystemToolsNonExistingFile");
  const std::string testDotFile(TEST_SYSTEMTOOLS_SOURCE_DIR "/.");
  const std::string testBinFile(TEST_SYSTEMTOOLS_SOURCE_DIR
                                "/testSystemTools.bin");
  const std::string testTxtFile(TEST_SYSTEMTOOLS_SOURCE_DIR
                                "/testSystemTools.cxx");
  const std::string testNewDir(TEST_SYSTEMTOOLS_BINARY_DIR
                               "/testSystemToolsNewDir");
  const std::string testNewFile(testNewDir + "/testNewFile.txt");

  if (kwsys::SystemTools::DetectFileType(testNonExistingFile.c_str()) !=
      kwsys::SystemTools::FileTypeUnknown) {
    std::cerr << "Problem with DetectFileType - failed to detect type of: "
              << testNonExistingFile << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::DetectFileType(testDotFile.c_str()) !=
      kwsys::SystemTools::FileTypeUnknown) {
    std::cerr << "Problem with DetectFileType - failed to detect type of: "
              << testDotFile << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::DetectFileType(testBinFile.c_str()) !=
      kwsys::SystemTools::FileTypeBinary) {
    std::cerr << "Problem with DetectFileType - failed to detect type of: "
              << testBinFile << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::DetectFileType(testTxtFile.c_str()) !=
      kwsys::SystemTools::FileTypeText) {
    std::cerr << "Problem with DetectFileType - failed to detect type of: "
              << testTxtFile << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::FileLength(testBinFile) != 766) {
    std::cerr << "Problem with FileLength - incorrect length for: "
              << testBinFile << std::endl;
    res = false;
  }

  kwsys::SystemTools::Stat_t buf;
  if (kwsys::SystemTools::Stat(testTxtFile.c_str(), &buf) != 0) {
    std::cerr << "Problem with Stat - unable to stat text file: "
              << testTxtFile << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::Stat(testBinFile, &buf) != 0) {
    std::cerr << "Problem with Stat - unable to stat bin file: " << testBinFile
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::MakeDirectory(testNewDir)) {
    std::cerr << "Problem with MakeDirectory for: " << testNewDir << std::endl;
    res = false;
  }
  // calling it again should just return true
  if (!kwsys::SystemTools::MakeDirectory(testNewDir)) {
    std::cerr << "Problem with second call to MakeDirectory for: "
              << testNewDir << std::endl;
    res = false;
  }
  // calling with 0 pointer should return false
  if (kwsys::SystemTools::MakeDirectory(nullptr)) {
    std::cerr << "Problem with MakeDirectory(0)" << std::endl;
    res = false;
  }
  // calling with an empty string should return false
  if (kwsys::SystemTools::MakeDirectory(std::string())) {
    std::cerr << "Problem with MakeDirectory(std::string())" << std::endl;
    res = false;
  }
  // check existence
  if (!kwsys::SystemTools::FileExists(testNewDir.c_str(), false)) {
    std::cerr << "Problem with FileExists as C string and not file for: "
              << testNewDir << std::endl;
    res = false;
  }
  // check existence
  if (!kwsys::SystemTools::PathExists(testNewDir)) {
    std::cerr << "Problem with PathExists for: " << testNewDir << std::endl;
    res = false;
  }
  // remove it
  if (!kwsys::SystemTools::RemoveADirectory(testNewDir)) {
    std::cerr << "Problem with RemoveADirectory for: " << testNewDir
              << std::endl;
    res = false;
  }
  // check existence
  if (kwsys::SystemTools::FileExists(testNewDir.c_str(), false)) {
    std::cerr << "After RemoveADirectory: "
              << "Problem with FileExists as C string and not file for: "
              << testNewDir << std::endl;
    res = false;
  }
  // check existence
  if (kwsys::SystemTools::PathExists(testNewDir)) {
    std::cerr << "After RemoveADirectory: "
              << "Problem with PathExists for: " << testNewDir << std::endl;
    res = false;
  }
  // create it using the char* version
  if (!kwsys::SystemTools::MakeDirectory(testNewDir.c_str())) {
    std::cerr << "Problem with second call to MakeDirectory as C string for: "
              << testNewDir << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::Touch(testNewFile, true)) {
    std::cerr << "Problem with Touch for: " << testNewFile << std::endl;
    res = false;
  }
  // calling MakeDirectory with something that is no file should fail
  if (kwsys::SystemTools::MakeDirectory(testNewFile)) {
    std::cerr << "Problem with to MakeDirectory for: " << testNewFile
              << std::endl;
    res = false;
  }

  // calling with 0 pointer should return false
  if (kwsys::SystemTools::FileExists(nullptr)) {
    std::cerr << "Problem with FileExists(0)" << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::FileExists(nullptr, true)) {
    std::cerr << "Problem with FileExists(0) as file" << std::endl;
    res = false;
  }
  // calling with an empty string should return false
  if (kwsys::SystemTools::FileExists(std::string())) {
    std::cerr << "Problem with FileExists(std::string())" << std::endl;
    res = false;
  }
  // FileExists(x, true) should return false on a directory
  if (kwsys::SystemTools::FileExists(testNewDir, true)) {
    std::cerr << "Problem with FileExists as file for: " << testNewDir
              << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::FileExists(testNewDir.c_str(), true)) {
    std::cerr << "Problem with FileExists as C string and file for: "
              << testNewDir << std::endl;
    res = false;
  }
  // FileExists(x, false) should return true even on a directory
  if (!kwsys::SystemTools::FileExists(testNewDir, false)) {
    std::cerr << "Problem with FileExists as not file for: " << testNewDir
              << std::endl;
    res = false;
  }
  if (!kwsys::SystemTools::FileExists(testNewDir.c_str(), false)) {
    std::cerr << "Problem with FileExists as C string and not file for: "
              << testNewDir << std::endl;
    res = false;
  }
  // should work, was created as new file before
  if (!kwsys::SystemTools::FileExists(testNewFile)) {
    std::cerr << "Problem with FileExists for: " << testNewFile << std::endl;
    res = false;
  }
  if (!kwsys::SystemTools::FileExists(testNewFile.c_str())) {
    std::cerr << "Problem with FileExists as C string for: " << testNewFile
              << std::endl;
    res = false;
  }
  if (!kwsys::SystemTools::FileExists(testNewFile, true)) {
    std::cerr << "Problem with FileExists as file for: " << testNewFile
              << std::endl;
    res = false;
  }
  if (!kwsys::SystemTools::FileExists(testNewFile.c_str(), true)) {
    std::cerr << "Problem with FileExists as C string and file for: "
              << testNewFile << std::endl;
    res = false;
  }

  // calling with an empty string should return false
  if (kwsys::SystemTools::PathExists(std::string())) {
    std::cerr << "Problem with PathExists(std::string())" << std::endl;
    res = false;
  }
  // PathExists(x) should return true on a directory
  if (!kwsys::SystemTools::PathExists(testNewDir)) {
    std::cerr << "Problem with PathExists for: " << testNewDir << std::endl;
    res = false;
  }
  // should work, was created as new file before
  if (!kwsys::SystemTools::PathExists(testNewFile)) {
    std::cerr << "Problem with PathExists for: " << testNewFile << std::endl;
    res = false;
  }

  std::cerr << std::oct;
// Reset umask
#ifdef __MSYS__
  mode_t fullMask = S_IWRITE;
  mode_t testPerm = S_IREAD;
#elif defined(_WIN32) && !defined(__CYGWIN__)
  // NOTE:  Windows doesn't support toggling _S_IREAD.
  mode_t fullMask = _S_IWRITE;
  mode_t testPerm = 0;
#else
  // On a normal POSIX platform, we can toggle all permissions.
  mode_t fullMask = S_IRWXU | S_IRWXG | S_IRWXO;
  mode_t testPerm = S_IRUSR;
#endif

  // Test file permissions without umask
  mode_t origPerm, thisPerm;
  if (!kwsys::SystemTools::GetPermissions(testNewFile, origPerm)) {
    std::cerr << "Problem with GetPermissions (1) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::SetPermissions(testNewFile, testPerm)) {
    std::cerr << "Problem with SetPermissions (1) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm)) {
    std::cerr << "Problem with GetPermissions (2) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if ((thisPerm & fullMask) != testPerm) {
    std::cerr << "SetPermissions failed to set permissions (1) for: "
              << testNewFile << ": actual = " << thisPerm
              << "; expected = " << testPerm << std::endl;
    res = false;
  }

  // While we're at it, check proper TestFileAccess functionality.
  bool do_write_test = true;
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||     \
  defined(__NetBSD__) || defined(__DragonFly__) || defined(__HOS_AIX__)
  // If we are running as root on POSIX-ish systems (Linux and the BSDs,
  // at least), ignore this check, as root can always write to files.
  do_write_test = (getuid() != 0);
#endif
  if (do_write_test &&
      kwsys::SystemTools::TestFileAccess(testNewFile,
                                         kwsys::TEST_FILE_WRITE)) {
    std::cerr
      << "TestFileAccess incorrectly indicated that this is a writable file:"
      << testNewFile << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::TestFileAccess(testNewFile, kwsys::TEST_FILE_OK)) {
    std::cerr
      << "TestFileAccess incorrectly indicated that this file does not exist:"
      << testNewFile << std::endl;
    res = false;
  }

  // Test restoring/setting full permissions.
  if (!kwsys::SystemTools::SetPermissions(testNewFile, fullMask)) {
    std::cerr << "Problem with SetPermissions (2) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm)) {
    std::cerr << "Problem with GetPermissions (3) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if ((thisPerm & fullMask) != fullMask) {
    std::cerr << "SetPermissions failed to set permissions (2) for: "
              << testNewFile << ": actual = " << thisPerm
              << "; expected = " << fullMask << std::endl;
    res = false;
  }

  mode_t orig_umask = umask(fullMask);
  // Test setting file permissions while honoring umask
  if (!kwsys::SystemTools::SetPermissions(testNewFile, fullMask, true)) {
    std::cerr << "Problem with SetPermissions (3) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::GetPermissions(testNewFile, thisPerm)) {
    std::cerr << "Problem with GetPermissions (4) for: " << testNewFile
              << std::endl;
    res = false;
  }

  if ((thisPerm & fullMask) != 0) {
    std::cerr << "SetPermissions failed to honor umask for: " << testNewFile
              << ": actual = " << thisPerm << "; expected = " << 0
              << std::endl;
    res = false;
  }

  // Restore umask
  umask(orig_umask);

  // Restore file permissions
  if (!kwsys::SystemTools::SetPermissions(testNewFile, origPerm)) {
    std::cerr << "Problem with SetPermissions (4) for: " << testNewFile
              << std::endl;
    res = false;
  }

  // Remove the test file
  if (!kwsys::SystemTools::RemoveFile(testNewFile)) {
    std::cerr << "Problem with RemoveFile: " << testNewFile << std::endl;
    res = false;
  }

  std::string const testFileMissing(testNewDir + "/testMissingFile.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissing)) {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    std::cerr << "RemoveFile(\"" << testFileMissing << "\") failed: " << msg
              << "\n";
    res = false;
  }

  std::string const testFileMissingDir(testNewDir + "/missing/file.txt");
  if (!kwsys::SystemTools::RemoveFile(testFileMissingDir)) {
    std::string const& msg = kwsys::SystemTools::GetLastSystemError();
    std::cerr << "RemoveFile(\"" << testFileMissingDir << "\") failed: " << msg
              << "\n";
    res = false;
  }

  std::string const testBadSymlink(testNewDir + "/badSymlink.txt");
  std::string const testBadSymlinkTgt(testNewDir + "/missing/symlinkTgt.txt");
  kwsys::Status const symlinkStatus =
    kwsys::SystemTools::CreateSymlink(testBadSymlinkTgt, testBadSymlink);
#if defined(_WIN32)
  // Under Windows, the user may not have enough privileges to create symlinks
  if (symlinkStatus.GetWindows() != ERROR_PRIVILEGE_NOT_HELD)
#endif
  {
    if (!symlinkStatus.IsSuccess()) {
      std::cerr << "CreateSymlink for: " << testBadSymlink << " -> "
                << testBadSymlinkTgt
                << " failed: " << symlinkStatus.GetString() << std::endl;
      res = false;
    }

    if (!kwsys::SystemTools::Touch(testBadSymlink, false)) {
      std::cerr << "Problem with Touch (no create) for: " << testBadSymlink
                << std::endl;
      res = false;
    }
  }

  if (!kwsys::SystemTools::Touch(testNewDir, false)) {
    std::cerr << "Problem with Touch (no create) for: " << testNewDir
              << std::endl;
    res = false;
  }

  kwsys::SystemTools::Touch(testNewFile, true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewDir)) {
    std::cerr << "Problem with RemoveADirectory for: " << testNewDir
              << std::endl;
    res = false;
  }

#ifdef KWSYS_TEST_SYSTEMTOOLS_LONG_PATHS
  // Perform the same file and directory creation and deletion tests but
  // with paths > 256 characters in length.

  const std::string testNewLongDir(
    TEST_SYSTEMTOOLS_BINARY_DIR
    "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "01234567890123");
  const std::string testNewLongFile(
    testNewLongDir +
    "/"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "012345678901234567890123456789012345678901234567890123456789"
    "0123456789.txt");

  if (!kwsys::SystemTools::MakeDirectory(testNewLongDir)) {
    std::cerr << "Problem with MakeDirectory for: " << testNewLongDir
              << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::Touch(testNewLongFile.c_str(), true)) {
    std::cerr << "Problem with Touch for: " << testNewLongFile << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::RemoveFile(testNewLongFile)) {
    std::cerr << "Problem with RemoveFile: " << testNewLongFile << std::endl;
    res = false;
  }

  kwsys::SystemTools::Touch(testNewLongFile.c_str(), true);
  if (!kwsys::SystemTools::RemoveADirectory(testNewLongDir)) {
    std::cerr << "Problem with RemoveADirectory for: " << testNewLongDir
              << std::endl;
    res = false;
  }
#endif

  std::cerr << std::dec;
  return res;
}

static bool CheckStringOperations()
{
  bool res = true;

  std::string test = "mary had a little lamb.";
  if (kwsys::SystemTools::CapitalizedWords(test) !=
      "Mary Had A Little Lamb.") {
    std::cerr << "Problem with CapitalizedWords " << '"' << test << '"'
              << std::endl;
    res = false;
  }

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::UnCapitalizedWords(test) !=
      "mary had a little lamb.") {
    std::cerr << "Problem with UnCapitalizedWords " << '"' << test << '"'
              << std::endl;
    res = false;
  }

  test = "MaryHadTheLittleLamb.";
  if (kwsys::SystemTools::AddSpaceBetweenCapitalizedWords(test) !=
      "Mary Had The Little Lamb.") {
    std::cerr << "Problem with AddSpaceBetweenCapitalizedWords " << '"' << test
              << '"' << std::endl;
    res = false;
  }

  char* cres =
    kwsys::SystemTools::AppendStrings("Mary Had A", " Little Lamb.");
  if (strcmp(cres, "Mary Had A Little Lamb.") != 0) {
    std::cerr << "Problem with AppendStrings "
              << "\"Mary Had A\" \" Little Lamb.\"" << std::endl;
    res = false;
  }
  delete[] cres;

  cres = kwsys::SystemTools::AppendStrings("Mary Had", " A ", "Little Lamb.");
  if (strcmp(cres, "Mary Had A Little Lamb.") != 0) {
    std::cerr << "Problem with AppendStrings "
              << "\"Mary Had\" \" A \" \"Little Lamb.\"" << std::endl;
    res = false;
  }
  delete[] cres;

  if (kwsys::SystemTools::CountChar("Mary Had A Little Lamb.", 'a') != 3) {
    std::cerr << "Problem with CountChar "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }

  cres = kwsys::SystemTools::RemoveChars("Mary Had A Little Lamb.", "aeiou");
  if (strcmp(cres, "Mry Hd A Lttl Lmb.") != 0) {
    std::cerr << "Problem with RemoveChars "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }
  delete[] cres;

  cres = kwsys::SystemTools::RemoveCharsButUpperHex("Mary Had A Little Lamb.");
  if (strcmp(cres, "A") != 0) {
    std::cerr << "Problem with RemoveCharsButUpperHex "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }
  delete[] cres;

  char* cres2 = strdup("Mary Had A Little Lamb.");
  kwsys::SystemTools::ReplaceChars(cres2, "aeiou", 'X');
  if (strcmp(cres2, "MXry HXd A LXttlX LXmb.") != 0) {
    std::cerr << "Problem with ReplaceChars "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }
  free(cres2);

  if (!kwsys::SystemTools::StringStartsWith("Mary Had A Little Lamb.",
                                            "Mary ")) {
    std::cerr << "Problem with StringStartsWith "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }

  if (!kwsys::SystemTools::StringEndsWith("Mary Had A Little Lamb.",
                                          " Lamb.")) {
    std::cerr << "Problem with StringEndsWith "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }

  cres = kwsys::SystemTools::DuplicateString("Mary Had A Little Lamb.");
  if (strcmp(cres, "Mary Had A Little Lamb.") != 0) {
    std::cerr << "Problem with DuplicateString "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }
  delete[] cres;

  test = "Mary Had A Little Lamb.";
  if (kwsys::SystemTools::CropString(test, 13) != "Mary ...Lamb.") {
    std::cerr << "Problem with CropString "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }

  std::vector<std::string> lines;
  kwsys::SystemTools::Split("Mary Had A Little Lamb.", lines, ' ');
  if (lines[0] != "Mary" || lines[1] != "Had" || lines[2] != "A" ||
      lines[3] != "Little" || lines[4] != "Lamb.") {
    std::cerr << "Problem with Split "
              << "\"Mary Had A Little Lamb.\"" << std::endl;
    res = false;
  }

  std::vector<std::string> linesToJoin = { "Mary", "Had", "A", "Little",
                                           "Lamb." };
  std::string joinResult = kwsys::SystemTools::Join(linesToJoin, " ");
  if (joinResult != "Mary Had A Little Lamb.") {
    std::cerr << "Problem with Join "
                 "\"Mary Had A Little Lamb.\""
              << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::ConvertToWindowsOutputPath(
        "L://Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "\"L:\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"") {
    std::cerr << "Problem with ConvertToWindowsOutputPath "
              << "\"L://Local Mojo/Hex Power Pack/Iffy Voodoo\"" << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::ConvertToWindowsOutputPath(
        "//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "\"\\\\grayson\\Local Mojo\\Hex Power Pack\\Iffy Voodoo\"") {
    std::cerr << "Problem with ConvertToWindowsOutputPath "
              << "\"//grayson/Local Mojo/Hex Power Pack/Iffy Voodoo\""
              << std::endl;
    res = false;
  }

  if (kwsys::SystemTools::ConvertToUnixOutputPath(
        "//Local Mojo/Hex Power Pack/Iffy Voodoo") !=
      "//Local\\ Mojo/Hex\\ Power\\ Pack/Iffy\\ Voodoo") {
    std::cerr << "Problem with ConvertToUnixOutputPath "
              << "\"//Local Mojo/Hex Power Pack/Iffy Voodoo\"" << std::endl;
    res = false;
  }

  return res;
}

static bool CheckPutEnv(const std::string& env, const char* name,
                        const char* value)
{
  if (!kwsys::SystemTools::PutEnv(env)) {
    std::cerr << "PutEnv(\"" << env << "\") failed!" << std::endl;
    return false;
  }
  std::string v = "(null)";
  kwsys::SystemTools::GetEnv(name, v);
  if (v != value) {
    std::cerr << "GetEnv(\"" << name << "\") returned \"" << v << "\", not \""
              << value << "\"!" << std::endl;
    return false;
  }
  return true;
}

static bool CheckUnPutEnv(const char* env, const char* name)
{
  if (!kwsys::SystemTools::UnPutEnv(env)) {
    std::cerr << "UnPutEnv(\"" << env << "\") failed!" << std::endl;
    return false;
  }
  std::string v;
  if (kwsys::SystemTools::GetEnv(name, v)) {
    std::cerr << "GetEnv(\"" << name << "\") returned \"" << v
              << "\", not (null)!" << std::endl;
    return false;
  }
  return true;
}

static bool CheckEnvironmentOperations()
{
  bool res = true;
  res &= CheckPutEnv("A=B", "A", "B");
  res &= CheckPutEnv("B=C", "B", "C");
  res &= CheckPutEnv("C=D", "C", "D");
  res &= CheckPutEnv("D=E", "D", "E");
  res &= CheckUnPutEnv("A", "A");
  res &= CheckUnPutEnv("B=", "B");
  res &= CheckUnPutEnv("C=D", "C");
  /* Leave "D=E" in environment so a memory checker can test for leaks.  */
  return res;
}

static bool CheckRelativePath(const std::string& local,
                              const std::string& remote,
                              const std::string& expected)
{
  std::string result = kwsys::SystemTools::RelativePath(local, remote);
  if (!kwsys::SystemTools::ComparePath(expected, result)) {
    std::cerr << "RelativePath(" << local << ", " << remote << ")  yielded "
              << result << " instead of " << expected << std::endl;
    return false;
  }
  return true;
}

static bool CheckRelativePaths()
{
  bool res = true;
  res &= CheckRelativePath("/usr/share", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/./share/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr//share/", "/bin/bash", "../../bin/bash");
  res &=
    CheckRelativePath("/usr/share/../bin/", "/bin/bash", "../../bin/bash");
  res &= CheckRelativePath("/usr/share", "/usr/share//bin", "bin");
  return res;
}

static bool CheckCollapsePath(const std::string& path,
                              const std::string& expected,
                              const char* base = nullptr)
{
  std::string result = kwsys::SystemTools::CollapseFullPath(path, base);
  if (!kwsys::SystemTools::ComparePath(expected, result)) {
    std::cerr << "CollapseFullPath(" << path << ")  yielded " << result
              << " instead of " << expected << std::endl;
    return false;
  }
  return true;
}

static bool CheckCollapsePath()
{
  bool res = true;
  res &= CheckCollapsePath("/usr/share/*", "/usr/share/*");
  res &= CheckCollapsePath("C:/Windows/*", "C:/Windows/*");
  res &= CheckCollapsePath("/usr/share/../lib", "/usr/lib");
  res &= CheckCollapsePath("/usr/share/./lib", "/usr/share/lib");
  res &= CheckCollapsePath("/usr/share/../../lib", "/lib");
  res &= CheckCollapsePath("/usr/share/.././../lib", "/lib");
  res &= CheckCollapsePath("/../lib", "/lib");
  res &= CheckCollapsePath("/../lib/", "/lib");
  res &= CheckCollapsePath("/", "/");
  res &= CheckCollapsePath("C:/", "C:/");
  res &= CheckCollapsePath("C:/../", "C:/");
  res &= CheckCollapsePath("C:/../../", "C:/");
  res &= CheckCollapsePath("../b", "../../b", "../");
  res &= CheckCollapsePath("../a/../b", "../b", "../rel");
  res &= CheckCollapsePath("a/../b", "../rel/b", "../rel");
  return res;
}

static std::string StringVectorToString(const std::vector<std::string>& vec)
{
  std::stringstream ss;
  ss << "vector(";
  for (auto i = vec.begin(); i != vec.end(); ++i) {
    if (i != vec.begin()) {
      ss << ", ";
    }
    ss << *i;
  }
  ss << ")";
  return ss.str();
}

static bool CheckGetPath()
{
  const char* envName = "S";
#ifdef _WIN32
  const char* envValue = "C:\\Somewhere\\something;D:\\Temp";
#else
  const char* envValue = "/Somewhere/something:/tmp";
#endif
  const char* registryPath = "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MyApp; MyKey]";

  std::vector<std::string> originalPaths;
  originalPaths.emplace_back(registryPath);

  std::vector<std::string> expectedPaths;
  expectedPaths.emplace_back(registryPath);
#ifdef _WIN32
  expectedPaths.push_back("C:/Somewhere/something");
  expectedPaths.push_back("D:/Temp");
#else
  expectedPaths.emplace_back("/Somewhere/something");
  expectedPaths.emplace_back("/tmp");
#endif

  bool res = true;
  res &= CheckPutEnv(std::string(envName) + "=" + envValue, envName, envValue);

  std::vector<std::string> paths = originalPaths;
  kwsys::SystemTools::GetPath(paths, envName);

  if (paths != expectedPaths) {
    std::cerr << "GetPath(" << StringVectorToString(originalPaths) << ", "
              << envName << ")  yielded " << StringVectorToString(paths)
              << " instead of " << StringVectorToString(expectedPaths)
              << std::endl;
    res = false;
  }

  res &= CheckUnPutEnv(envName, envName);
  return res;
}

static bool CheckGetFilenameName()
{
  const char* windowsFilepath = "C:\\somewhere\\something";
  const char* unixFilepath = "/somewhere/something";

#if defined(_WIN32) || defined(KWSYS_SYSTEMTOOLS_SUPPORT_WINDOWS_SLASHES)
  std::string expectedWindowsFilename = "something";
#else
  std::string expectedWindowsFilename = "C:\\somewhere\\something";
#endif
  std::string expectedUnixFilename = "something";

  bool res = true;
  std::string filename = kwsys::SystemTools::GetFilenameName(windowsFilepath);
  if (filename != expectedWindowsFilename) {
    std::cerr << "GetFilenameName(" << windowsFilepath << ") yielded "
              << filename << " instead of " << expectedWindowsFilename
              << std::endl;
    res = false;
  }

  filename = kwsys::SystemTools::GetFilenameName(unixFilepath);
  if (filename != expectedUnixFilename) {
    std::cerr << "GetFilenameName(" << unixFilepath << ") yielded " << filename
              << " instead of " << expectedUnixFilename << std::endl;
    res = false;
  }
  return res;
}

static bool CheckFind()
{
  bool res = true;
  const std::string testFindFileName("testFindFile.txt");
  const std::string testFindFile(TEST_SYSTEMTOOLS_BINARY_DIR "/" +
                                 testFindFileName);

  if (!kwsys::SystemTools::Touch(testFindFile, true)) {
    std::cerr << "Problem with Touch for: " << testFindFile << std::endl;
    // abort here as the existence of the file only makes the test meaningful
    return false;
  }

  std::vector<std::string> searchPaths;
  searchPaths.emplace_back(TEST_SYSTEMTOOLS_BINARY_DIR);
  if (kwsys::SystemTools::FindFile(testFindFileName, searchPaths, true)
        .empty()) {
    std::cerr << "Problem with FindFile without system paths for: "
              << testFindFileName << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::FindFile(testFindFileName, searchPaths, false)
        .empty()) {
    std::cerr << "Problem with FindFile with system paths for: "
              << testFindFileName << std::endl;
    res = false;
  }

  return res;
}

static bool CheckIsSubDirectory()
{
  bool res = true;

  if (kwsys::SystemTools::IsSubDirectory("/foo", "/") == false) {
    std::cerr << "Problem with IsSubDirectory (root - unix): " << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::IsSubDirectory("c:/foo", "c:/") == false) {
    std::cerr << "Problem with IsSubDirectory (root - dos): " << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::IsSubDirectory("/foo/bar", "/foo") == false) {
    std::cerr << "Problem with IsSubDirectory (deep): " << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::IsSubDirectory("/foo", "/foo") == true) {
    std::cerr << "Problem with IsSubDirectory (identity): " << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::IsSubDirectory("/fooo", "/foo") == true) {
    std::cerr << "Problem with IsSubDirectory (substring): " << std::endl;
    res = false;
  }
  if (kwsys::SystemTools::IsSubDirectory("/foo/", "/foo") == true) {
    std::cerr << "Problem with IsSubDirectory (prepended slash): "
              << std::endl;
    res = false;
  }

  return res;
}

static bool CheckGetLineFromStream()
{
  const std::string fileWithFiveCharsOnFirstLine(TEST_SYSTEMTOOLS_SOURCE_DIR
                                                 "/README.rst");

  kwsys::ifstream file(fileWithFiveCharsOnFirstLine.c_str(), std::ios::in);

  if (!file) {
    std::cerr << "Problem opening: " << fileWithFiveCharsOnFirstLine
              << std::endl;
    return false;
  }

  std::string line;
  bool has_newline = false;
  bool result;

  file.seekg(0, std::ios::beg);
  result = kwsys::SystemTools::GetLineFromStream(file, line, &has_newline,
                                                 std::string::npos);
  if (!result || line.size() != 5) {
    std::cerr << "First line does not have five characters: " << line.size()
              << std::endl;
    return false;
  }

  file.seekg(0, std::ios::beg);
  result = kwsys::SystemTools::GetLineFromStream(file, line, &has_newline,
                                                 std::string::npos);
  if (!result || line.size() != 5) {
    std::cerr << "First line does not have five characters after rewind: "
              << line.size() << std::endl;
    return false;
  }

  bool ret = true;

  for (std::string::size_type size = 1; size <= 5; ++size) {
    file.seekg(0, std::ios::beg);
    result =
      kwsys::SystemTools::GetLineFromStream(file, line, &has_newline, size);
    if (!result || line.size() != size) {
      std::cerr << "Should have read " << size << " characters but got "
                << line.size() << std::endl;
      ret = false;
    }
  }

  return ret;
}

static bool CheckGetLineFromStreamLongLine()
{
  const std::string fileWithLongLine("longlines.txt");
  std::string firstLine, secondLine;
  // First line: large buffer, containing a carriage return for some reason.
  firstLine.assign(2050, ' ');
  firstLine += "\rfirst";
  secondLine.assign(2050, 'y');
  secondLine += "second";

  // Create file with long lines.
  {
    kwsys::ofstream out(fileWithLongLine.c_str(), std::ios::binary);
    if (!out) {
      std::cerr << "Problem opening for write: " << fileWithLongLine
                << std::endl;
      return false;
    }
    out << firstLine << "\r\n\n" << secondLine << "\n";
  }

  kwsys::ifstream file(fileWithLongLine.c_str(), std::ios::binary);
  if (!file) {
    std::cerr << "Problem opening: " << fileWithLongLine << std::endl;
    return false;
  }

  std::string line;
  bool has_newline = false;
  bool result;

  // Read first line.
  result = kwsys::SystemTools::GetLineFromStream(file, line, &has_newline,
                                                 std::string::npos);
  if (!result || line != firstLine) {
    std::cerr << "First line does not match, expected " << firstLine.size()
              << " characters, got " << line.size() << std::endl;
    return false;
  }
  if (!has_newline) {
    std::cerr << "Expected new line to be read from first line" << std::endl;
    return false;
  }

  // Read empty line.
  has_newline = false;
  result = kwsys::SystemTools::GetLineFromStream(file, line, &has_newline,
                                                 std::string::npos);
  if (!result || !line.empty()) {
    std::cerr << "Expected successful read with an empty line, got "
              << line.size() << " characters" << std::endl;
    return false;
  }
  if (!has_newline) {
    std::cerr << "Expected new line to be read for an empty line" << std::endl;
    return false;
  }

  // Read second line.
  has_newline = false;
  result = kwsys::SystemTools::GetLineFromStream(file, line, &has_newline,
                                                 std::string::npos);
  if (!result || line != secondLine) {
    std::cerr << "Second line does not match, expected " << secondLine.size()
              << " characters, got " << line.size() << std::endl;
    return false;
  }
  if (!has_newline) {
    std::cerr << "Expected new line to be read from second line" << std::endl;
    return false;
  }

  return true;
}

static bool writeFile(const char* fileName, const char* data)
{
  kwsys::ofstream out(fileName, std::ios::binary);
  out << data;
  if (!out) {
    std::cerr << "Failed to write file: " << fileName << std::endl;
    return false;
  }
  return true;
}

static std::string readFile(const char* fileName)
{
  kwsys::ifstream in(fileName, std::ios::binary);
  std::stringstream sstr;
  sstr << in.rdbuf();
  std::string data = sstr.str();
  if (!in) {
    std::cerr << "Failed to read file: " << fileName << std::endl;
    return std::string();
  }
  return data;
}

struct
{
  const char* a;
  const char* b;
  bool differ;
} diff_test_cases[] = { { "one", "one", false },
                        { "one", "two", true },
                        { "", "", false },
                        { "\n", "\r\n", false },
                        { "one\n", "one\n", false },
                        { "one\r\n", "one\n", false },
                        { "one\n", "one", false },
                        { "one\ntwo", "one\ntwo", false },
                        { "one\ntwo", "one\r\ntwo", false } };

static bool CheckTextFilesDiffer()
{
  const int num_test_cases =
    sizeof(diff_test_cases) / sizeof(diff_test_cases[0]);
  for (int i = 0; i < num_test_cases; ++i) {
    if (!writeFile("file_a", diff_test_cases[i].a) ||
        !writeFile("file_b", diff_test_cases[i].b)) {
      return false;
    }
    if (kwsys::SystemTools::TextFilesDiffer("file_a", "file_b") !=
        diff_test_cases[i].differ) {
      std::cerr << "Incorrect TextFilesDiffer result for test case " << i + 1
                << "." << std::endl;
      return false;
    }
  }

  return true;
}

static bool CheckCopyFileIfDifferent()
{
  bool ret = true;
  const int num_test_cases =
    sizeof(diff_test_cases) / sizeof(diff_test_cases[0]);
  for (int i = 0; i < num_test_cases; ++i) {
    if (!writeFile("file_a", diff_test_cases[i].a) ||
        !writeFile("file_b", diff_test_cases[i].b)) {
      return false;
    }
    const char* cptarget =
      i < 4 ? TEST_SYSTEMTOOLS_BINARY_DIR "/file_b" : "file_b";
    if (!kwsys::SystemTools::CopyFileIfDifferent("file_a", cptarget)) {
      std::cerr << "CopyFileIfDifferent() returned false for test case "
                << i + 1 << "." << std::endl;
      ret = false;
      continue;
    }
    std::string bdata = readFile(cptarget);
    if (diff_test_cases[i].a != bdata) {
      std::cerr << "Incorrect CopyFileIfDifferent file contents in test case "
                << i + 1 << "." << std::endl;
      ret = false;
      continue;
    }
  }

  if (!kwsys::SystemTools::MakeDirectory("dir_a") ||
      !kwsys::SystemTools::MakeDirectory("dir_b")) {
    return false;
  }

  if (!kwsys::SystemTools::CopyFileIfDifferent("dir_a/", "dir_b")) {
    ret = false;
  }

  return ret;
}

static bool CheckURLParsing()
{
  bool ret = true;
  std::string url = "https://user:pw@hostname:42/full/url.com";

  std::string protocol, username, password, hostname, dataport, database;
  kwsys::SystemTools::ParseURL(url, protocol, username, password, hostname,
                               dataport, database);
  if (protocol != "https" || username != "user" || password != "pw" ||
      hostname != "hostname" || dataport != "42" ||
      database != "full/url.com") {
    std::cerr << "Incorrect URL parsing" << std::endl;
    ret = false;
  }

  std::string uri =
    "file://hostname/path/to/"
    "a%20file%20with%20str%C3%A0ng%C3%A8%20ch%40r%20and%20s%C2%B5aces";
  kwsys::SystemTools::ParseURL(uri, protocol, username, password, hostname,
                               dataport, database, true);
  if (protocol != "file" || hostname != "hostname" ||
      database != "path/to/a file with stràngè ch@r and sµaces") {
    std::cerr << "Incorrect URL parsing or decoding" << std::endl;
    ret = false;
  }
  return ret;
}

static bool CheckSplitString()
{
  bool ret = true;

  auto check_split = [](std::string const& input,
                        std::initializer_list<const char*> expected) -> bool {
    auto const components = kwsys::SystemTools::SplitString(input, '/');
    if (components.size() != expected.size()) {
      std::cerr << "Incorrect split count for " << input << ": "
                << components.size() << std::endl;
      return false;
    }
    size_t i = 0;
    for (auto& part : expected) {
      if (components[i] != part) {
        std::cerr << "Incorrect split component " << i << " for " << input
                  << ": " << components[i] << std::endl;
        return false;
      }
      ++i;
    }
    return true;
  };

  // No separators
  ret &= check_split("nosep", { "nosep" });
  // Simple
  ret &= check_split("first/second", { "first", "second" });
  // Separator at beginning
  ret &= check_split("/starts/sep", { "", "starts", "sep" });
  // Separator at end
  ret &= check_split("ends/sep/", { "ends", "sep", "" });

  return ret;
}

int testSystemTools(int, char*[])
{
  bool res = true;

  int cc;
  for (cc = 0; toUnixPaths[cc][0]; cc++) {
    res &= CheckConvertToUnixSlashes(toUnixPaths[cc][0], toUnixPaths[cc][1]);
  }

  // Special check for ~
  std::string output;
  if (kwsys::SystemTools::GetEnv("HOME", output)) {
    output += "/foo bar/lala";
    res &= CheckConvertToUnixSlashes("~/foo bar/lala", output);
  }

  for (cc = 0; checkEscapeChars[cc][0]; cc++) {
    res &= CheckEscapeChars(checkEscapeChars[cc][0], checkEscapeChars[cc][1],
                            *checkEscapeChars[cc][2], checkEscapeChars[cc][3]);
  }

  res &= CheckFileOperations();

  res &= CheckStringOperations();

  res &= CheckEnvironmentOperations();

  res &= CheckRelativePaths();

  res &= CheckCollapsePath();

  res &= CheckGetPath();

  res &= CheckFind();

  res &= CheckIsSubDirectory();

  res &= CheckGetLineFromStream();

  res &= CheckGetLineFromStreamLongLine();

  res &= CheckGetFilenameName();

  res &= CheckTextFilesDiffer();

  res &= CheckCopyFileIfDifferent();

  res &= CheckURLParsing();

  res &= CheckSplitString();

  return res ? 0 : 1;
}
