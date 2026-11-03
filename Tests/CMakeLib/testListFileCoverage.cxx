/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <sstream>
#include <string>
#include <vector>

#include "cmsys/Directory.hxx"

#include "cmLcovTracefile.h"
#include "cmListFileCache.h"
#include "cmListFileCoverage.h"
#include "cmSystemTools.h"

#include "testCommon.h"
#include "testConfig.h"

namespace {

std::vector<cmListFileFunction> parse(std::string const& text)
{
  cmListFile lf;
  if (!lf.ParseString(text, "test.cmake", nullptr, cmListFileBacktrace())) {
    std::cout << "ParseString failed\n";
  }
  return lf.Functions;
}

std::string writeText(cmLcovTracefile const& t)
{
  std::ostringstream os;
  t.Write(os);
  return os.str();
}

bool contains(std::string const& text, std::string const& part)
{
  if (text.find(part) == std::string::npos) {
    std::cout << "Missing '" << part << "' in:\n" << text;
    return false;
  }
  return true;
}

// Turns coverage off on scope exit, so a failed ASSERT never leaves it on
// for the next test.
struct FinishGuard
{
  FinishGuard() = default;
  FinishGuard(FinishGuard const&) = delete;
  FinishGuard& operator=(FinishGuard const&) = delete;
  ~FinishGuard() { cmListFileCoverage::Finish(); }
};

// A fresh directory, removed on scope exit.  Declare it before the
// FinishGuard, so that the tracefile is written before the removal.
struct TempDir
{
  TempDir()
    : Path(BUILD_DIR "/testListFileCoverage-XXXXXX")
  {
    this->Ok = static_cast<bool>(cmSystemTools::MakeTempDirectory(this->Path));
    if (!this->Ok) {
      std::cout << "Cannot create temporary directory " << this->Path << '\n';
    }
  }
  TempDir(TempDir const&) = delete;
  TempDir& operator=(TempDir const&) = delete;
  ~TempDir()
  {
    if (this->Ok) {
      cmSystemTools::RemoveADirectory(this->Path);
    }
  }

  std::string Path;
  bool Ok = false;
};

bool startCoverageIn(std::string const& dir)
{
  cmSystemTools::PutEnv("CMAKE_LISTFILE_COVERAGE_DIR=" + dir);
  std::string error;
  bool const ok = cmListFileCoverage::Start(error);
  cmSystemTools::UnPutEnv("CMAKE_LISTFILE_COVERAGE_DIR");
  if (!ok) {
    std::cout << "Start failed: " << error << '\n';
  }
  return ok;
}

unsigned long countEntries(std::string const& dir)
{
  cmsys::Directory d;
  if (!d.Load(dir)) {
    return 0;
  }
  unsigned long n = 0;
  for (unsigned long i = 0; i < d.GetNumberOfFiles(); ++i) {
    std::string const name = d.GetFileName(i);
    if (name != "." && name != "..") {
      ++n;
    }
  }
  return n;
}

bool testExecutableLines()
{
  std::vector<long> const lines =
    cmListFileCoverage::ExecutableLines(parse("set(a 1)\n"
                                              "if(a)\n"
                                              "else()\n"
                                              "endif()\n"
                                              "function(f)\n"
                                              "endfunction()\n"
                                              "set(b\n"
                                              "  2)\n"));
  std::vector<long> const expected = { 1, 2, 5, 7 };
  ASSERT_TRUE(lines == expected);
  return true;
}

bool testRecording()
{
  TempDir dir;
  ASSERT_TRUE(dir.Ok);
  FinishGuard guard;
  ASSERT_TRUE(startCoverageIn(dir.Path));
  cmListFileCoverage* cov = cmListFileCoverage::Get();
  ASSERT_TRUE(cov != nullptr);

  std::vector<cmListFileFunction> const functions = parse("function(f)\n"
                                                          "  set(x 1)\n"
                                                          "endfunction()\n"
                                                          "f()\n");
  cov->FileParsed("/s/a.cmake", functions);
  cov->CommandRan(functions[0], "/s/a.cmake", 1);
  cov->CommandRan(functions[3], "/s/a.cmake", 4);
  cov->CommandRan(functions[1], "/s/a.cmake", 2);
  cov->FunctionCalled("/s/a.cmake", 1);

  // Ignored: the file is not registered.
  cov->CommandRan(functions[0], "/s/other.cmake", 1);
  // Ignored: endfunction is not an executable line.
  cov->CommandRan(functions[2], "/s/a.cmake", 3);
  // Ignored: no function is defined on this line.
  cov->FunctionCalled("/s/a.cmake", 99);

  std::string const text = writeText(cov->ToTracefile());
  ASSERT_TRUE(contains(text, "SF:/s/a.cmake\n"));
  ASSERT_TRUE(contains(text, "FN:1,f\n"));
  ASSERT_TRUE(contains(text, "FNDA:1,f\n"));
  ASSERT_TRUE(contains(text, "DA:1,1\nDA:2,1\nDA:4,1\nLF:3\nLH:3\n"));
  ASSERT_TRUE(text.find("other.cmake") == std::string::npos);
  ASSERT_TRUE(text.find("DA:3,") == std::string::npos);
  ASSERT_TRUE(text.find("FN:99") == std::string::npos);
  return true;
}

bool testFileParsedTwice()
{
  TempDir dir;
  ASSERT_TRUE(dir.Ok);
  FinishGuard guard;
  ASSERT_TRUE(startCoverageIn(dir.Path));
  cmListFileCoverage* cov = cmListFileCoverage::Get();
  ASSERT_TRUE(cov != nullptr);

  std::vector<cmListFileFunction> const first = parse("set(a 1)\n");
  cov->FileParsed("/s/a.cmake", first);
  cov->CommandRan(first[0], "/s/a.cmake", 1);
  cov->FileParsed("/s/a.cmake", parse("set(a 1)\nset(b 2)\n"));

  std::string const text = writeText(cov->ToTracefile());
  ASSERT_TRUE(contains(text, "DA:1,1\nDA:2,0\nLF:2\nLH:1\n"));
  return true;
}

bool testSyntheticCommand()
{
  TempDir dir;
  ASSERT_TRUE(dir.Ok);
  FinishGuard guard;
  ASSERT_TRUE(startCoverageIn(dir.Path));
  cmListFileCoverage* cov = cmListFileCoverage::Get();
  ASSERT_TRUE(cov != nullptr);

  cov->FileParsed("/s/a.cmake", parse("set(a 1)\n"));
  cmListFileFunction const synthetic("set", 1, 1, {});
  cmListFileFunction const real("set", 1, 1, {});
  {
    cmListFileCoverage::SyntheticCommandScope scope(synthetic);
    cov->CommandRan(synthetic, "/s/a.cmake", 1);
    cov->CommandRan(real, "/s/a.cmake", 1);
  }
  // The scope has ended, so the object counts again.
  cov->CommandRan(synthetic, "/s/a.cmake", 1);

  std::string const text = writeText(cov->ToTracefile());
  ASSERT_TRUE(contains(text, "DA:1,2\n"));
  return true;
}

bool testStartOffWhenUnset()
{
  FinishGuard guard;
  cmSystemTools::UnPutEnv("CMAKE_LISTFILE_COVERAGE_DIR");
  std::string error;
  ASSERT_TRUE(cmListFileCoverage::Start(error));
  ASSERT_TRUE(cmListFileCoverage::Get() == nullptr);
  return true;
}

bool testFinishWithNothingRegistered()
{
  TempDir dir;
  ASSERT_TRUE(dir.Ok);
  FinishGuard guard;
  ASSERT_TRUE(startCoverageIn(dir.Path));
  cmListFileCoverage::Finish();
  ASSERT_TRUE(cmListFileCoverage::Get() == nullptr);
  ASSERT_EQUAL(countEntries(dir.Path), 0);
  return true;
}

bool testFinishWithNoExecutableLines()
{
  TempDir dir;
  ASSERT_TRUE(dir.Ok);
  {
    FinishGuard guard;
    ASSERT_TRUE(startCoverageIn(dir.Path));
    cmListFileCoverage* cov = cmListFileCoverage::Get();
    ASSERT_TRUE(cov != nullptr);
    cov->FileParsed("/s/comments.cmake", parse("# Only a comment.\n"));
  }
  ASSERT_EQUAL(countEntries(dir.Path), 0);
  return true;
}
}

int testListFileCoverage(int /*unused*/, char* /*unused*/[])
{
  return runTests({ testExecutableLines, testRecording, testFileParsedTwice,
                    testSyntheticCommand, testStartOffWhenUnset,
                    testFinishWithNothingRegistered,
                    testFinishWithNoExecutableLines },
                  false);
}
