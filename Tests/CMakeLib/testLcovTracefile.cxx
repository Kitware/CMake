/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <array>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/FStream.hxx"

#include "cmLcovTracefile.h"
#include "cmSystemTools.h"

#include "testCommon.h"

namespace {

cmLcovTracefile makeSample()
{
  cmLcovTracefile t;
  t.TestName = "t1";
  cmLcovSourceFile& f = t.Files["/s/a.cmake"];
  f.Lines = { { 4, 1 }, { 2, 0 } };
  f.Functions[11] = { "dup", 1 };
  f.Functions[1] = { "f1", 2 };
  f.Functions[14] = { "dup", 0 };
  return t;
}

std::string writeText(cmLcovTracefile const& t)
{
  std::ostringstream os;
  t.Write(os);
  return os.str();
}

bool readText(std::string const& text, cmLcovTracefile& t,
              std::string* error = nullptr)
{
  std::string localError;
  std::istringstream is(text);
  bool const ok = t.Read(is, error ? *error : localError);
  if (!ok && !error) {
    std::cout << "Read failed: " << localError << '\n';
  }
  return ok;
}

bool testWriteOrderAndDuplicateNames()
{
  ASSERT_EQUAL(writeText(makeSample()),
               "TN:t1\nSF:/s/a.cmake\n"
               "FN:1,f1\nFN:11,dup@11\nFN:14,dup@14\n"
               "FNDA:2,f1\nFNDA:1,dup@11\nFNDA:0,dup@14\n"
               "FNF:3\nFNH:2\nDA:2,0\nDA:4,1\nLF:2\nLH:1\nend_of_record\n");
  return true;
}

bool testReadRoundTrip()
{
  std::string const text = writeText(makeSample());
  cmLcovTracefile t;
  ASSERT_TRUE(readText(text, t));
  ASSERT_EQUAL(t.TestName, "t1");
  ASSERT_EQUAL(t.Files["/s/a.cmake"].Functions[11].Name, "dup");
  ASSERT_EQUAL(writeText(t), text);
  return true;
}

bool testReadIgnoresUnknown()
{
  cmLcovTracefile t;
  ASSERT_TRUE(readText("VER:1\nSF:/x\nBRDA:1,0,0,1\nBRF:1\nBRH:1\nDA:1,3,abc\n"
                       "end_of_record\n",
                       t));
  ASSERT_EQUAL(writeText(t),
               "SF:/x\nFNF:0\nFNH:0\nDA:1,3\nLF:1\nLH:1\nend_of_record\n");
  return true;
}

bool testReadCrlfAndDrive()
{
  cmLcovTracefile t;
  ASSERT_TRUE(readText("SF:C:/a b/x.cmake\r\nDA:1,1\r\nend_of_record\r\n", t));
  ASSERT_EQUAL(t.Files.size(), 1u);
  ASSERT_EQUAL(writeText(t),
               "SF:C:/a b/x.cmake\nFNF:0\nFNH:0\nDA:1,1\nLF:1\nLH:1\n"
               "end_of_record\n");
  return true;
}

bool testReadSameFileTwice()
{
  cmLcovTracefile t;
  ASSERT_TRUE(readText("SF:/x\nDA:1,1\nend_of_record\n"
                       "SF:/x\nDA:1,2\nend_of_record\n",
                       t));
  ASSERT_EQUAL(writeText(t),
               "SF:/x\nFNF:0\nFNH:0\nDA:1,3\nLF:1\nLH:1\nend_of_record\n");
  return true;
}

bool testReadStripsSuffix()
{
  cmLcovTracefile t;
  ASSERT_TRUE(
    readText("SF:/x\nFN:12,foo@12\nFNDA:1,foo@12\nend_of_record\n", t));
  std::map<long, cmLcovFunction> const& fns = t.Files["/x"].Functions;
  ASSERT_EQUAL(fns.size(), 1u);
  ASSERT_EQUAL(fns.begin()->first, 12);
  ASSERT_EQUAL(fns.begin()->second.Name, "foo");
  ASSERT_EQUAL(fns.begin()->second.Hits, 1u);
  return true;
}

bool testReadErrors()
{
  std::array<char const*, 5> const inputs{ {
    "",
    "DA:1,1\n",
    "SF:/x\nDA:a,1\nend_of_record\n",
    "SF:/x\nFNDA:1,f\nFN:1,f\nend_of_record\n",
    "SF:/x\nDA:1,1\n",
  } };
  for (char const* input : inputs) {
    cmLcovTracefile t;
    std::string error;
    if (readText(input, t, &error)) {
      std::cout << "Read unexpectedly succeeded for '" << input << "'\n";
      return false;
    }
    if (error.empty()) {
      std::cout << "Read set no error for '" << input << "'\n";
      return false;
    }
  }
  return true;
}

bool testNameEncoding()
{
  // Names are percent-encoded as in URLs, so that every name reads back.
  cmLcovTracefile t;
  std::map<long, cmLcovFunction>& fns = t.Files["/x"].Functions;
  fns[3] = { "a,b", 0 };
  fns[5] = { "50%", 0 };
  fns[7] = { "x@y", 0 };
  fns[9] = { "dup", 0 };
  fns[11] = { "dup", 0 };
  fns[13] = { "foo@13", 0 };
  fns[15] = { "two\nlines", 0 };
  fns[17] = { "d\re", 0 };
  fns[19] = { "__check_flag", 0 };
  std::string const text = writeText(t);
  ASSERT_EQUAL(text,
               "SF:/x\n"
               "FN:3,a%2Cb\nFN:5,50%25\nFN:7,x%40y\nFN:9,dup@9\n"
               "FN:11,dup@11\nFN:13,foo%4013\nFN:15,two%0Alines\n"
               "FN:17,d%0De\nFN:19,__check_flag\n"
               "FNDA:0,a%2Cb\nFNDA:0,50%25\nFNDA:0,x%40y\nFNDA:0,dup@9\n"
               "FNDA:0,dup@11\nFNDA:0,foo%4013\nFNDA:0,two%0Alines\n"
               "FNDA:0,d%0De\nFNDA:0,__check_flag\n"
               "FNF:9\nFNH:0\nLF:0\nLH:0\nend_of_record\n");

  cmLcovTracefile back;
  ASSERT_TRUE(readText(text, back));
  std::map<long, cmLcovFunction> const& backFns = back.Files["/x"].Functions;
  ASSERT_EQUAL(backFns.size(), fns.size());
  for (auto const& fn : fns) {
    auto it = backFns.find(fn.first);
    ASSERT_TRUE(it != backFns.end());
    ASSERT_EQUAL(it->second.Name, fn.second.Name);
  }

  // Lowercase hexadecimal digits decode too.  A "%" that does not start an
  // escape stays as written.
  cmLcovTracefile other;
  ASSERT_TRUE(readText("SF:/x\nFN:1,a%2cb\nFN:2,p%zz\nFN:3,q%\n"
                       "end_of_record\n",
                       other));
  std::map<long, cmLcovFunction> const& otherFns = other.Files["/x"].Functions;
  ASSERT_EQUAL(otherFns.at(1).Name, "a,b");
  ASSERT_EQUAL(otherFns.at(2).Name, "p%zz");
  ASSERT_EQUAL(otherFns.at(3).Name, "q%");
  return true;
}

bool testWriteSkipsEmptyFiles()
{
  cmLcovTracefile t;
  t.Files["/empty"];
  t.Files["/lines"].Lines[1] = 0;
  t.Files["/functions"].Functions[1] = { "f", 0 };
  ASSERT_EQUAL(writeText(t),
               "SF:/functions\nFN:1,f\nFNDA:0,f\nFNF:1\nFNH:0\nLF:0\nLH:0\n"
               "end_of_record\n"
               "SF:/lines\nFNF:0\nFNH:0\nDA:1,0\nLF:1\nLH:0\nend_of_record\n");
  return true;
}

bool testHasData()
{
  cmLcovTracefile t;
  ASSERT_TRUE(!t.HasData());
  t.Files["/empty"];
  ASSERT_TRUE(!t.HasData());
  t.Files["/lines"].Lines[1] = 0;
  ASSERT_TRUE(t.HasData());
  t.Files.erase("/lines");
  t.Files["/functions"].Functions[1] = { "f", 0 };
  ASSERT_TRUE(t.HasData());
  return true;
}

bool testMerge()
{
  cmLcovTracefile a;
  a.TestName = "a";
  a.Files["/x"].Lines = { { 1, 1 }, { 2, 0 } };
  a.Files["/x"].Functions[1] = { "f", 1 };
  cmLcovTracefile b;
  b.TestName = "b";
  b.Files["/x"].Lines = { { 2, 3 }, { 5, 1 } };
  b.Files["/x"].Functions[1] = { "g", 2 };
  b.Files["/y"].Lines = { { 1, 1 } };
  a.Merge(b);

  cmLcovTracefile expected;
  expected.TestName = "a";
  expected.Files["/x"].Lines = { { 1, 1 }, { 2, 3 }, { 5, 1 } };
  expected.Files["/x"].Functions[1] = { "f", 3 };
  expected.Files["/y"].Lines = { { 1, 1 } };
  ASSERT_EQUAL(writeText(a), writeText(expected));
  return true;
}

bool testMergeFiles()
{
  std::string const good = "testLcovTracefile-good.info";
  std::string const bad = "testLcovTracefile-bad.info";
  {
    cmsys::ofstream fout(good.c_str());
    fout << "SF:/x\nDA:1,2\nend_of_record\n";
  }
  {
    cmsys::ofstream fout(bad.c_str());
    fout << "SF:/y\nDA:1,1\nend_of_record\nSF:/z\nDA:a,1\nend_of_record\n";
  }

  cmLcovTracefile t;
  t.Files["/x"].Lines[1] = 1;
  std::vector<std::string> warnings;
  auto warn = [&warnings](std::string const& w) { warnings.push_back(w); };
  ASSERT_TRUE(t.MergeFiles({ good }, warn));
  ASSERT_TRUE(warnings.empty());
  ASSERT_TRUE(!t.MergeFiles({ bad, "testLcovTracefile-missing.info" }, warn));
  cmSystemTools::RemoveFile(good);
  cmSystemTools::RemoveFile(bad);
  ASSERT_EQUAL(warnings.size(), 2u);
  ASSERT_EQUAL(warnings[0],
               "Skipping unreadable CMake listfile coverage file: "
               "testLcovTracefile-bad.info\n  line 5: malformed DA record "
               "\"a,1\"");
  ASSERT_EQUAL(warnings[1],
               "Skipping unreadable CMake listfile coverage file: "
               "testLcovTracefile-missing.info");

  // The bad file adds nothing, not even its good first record.
  ASSERT_EQUAL(writeText(t),
               "SF:/x\nFNF:0\nFNH:0\nDA:1,3\nLF:1\nLH:1\nend_of_record\n");
  return true;
}

bool testRemoveBuildTreeFiles()
{
  auto sample = []() {
    cmLcovTracefile t;
    for (char const* path :
         { "/s/a.cmake", "/b/gen.cmake", "/b/sub/x.cmake", "/bb/y.cmake" }) {
      t.Files[path].Lines[1] = 1;
    }
    return t;
  };

  std::vector<std::string> removed;
  auto collect = [&removed](std::string const& path) {
    removed.push_back(path);
  };

  cmLcovTracefile t = sample();
  t.RemoveBuildTreeFiles("/s", "/b", collect);
  ASSERT_EQUAL(removed.size(), 2u);
  ASSERT_EQUAL(t.Files.size(), 2u);
  ASSERT_EQUAL(t.Files.count("/s/a.cmake"), 1u);
  ASSERT_EQUAL(t.Files.count("/bb/y.cmake"), 1u);

  // An in-source build, or no build tree, keeps every file.
  removed.clear();
  t = sample();
  t.RemoveBuildTreeFiles("/b", "/b/", collect);
  t.RemoveBuildTreeFiles("/s", "", collect);
  ASSERT_TRUE(removed.empty());
  ASSERT_EQUAL(t.Files.size(), 4u);
  return true;
}
}

int testLcovTracefile(int /*unused*/, char* /*unused*/[])
{
  return runTests({ testWriteOrderAndDuplicateNames, testReadRoundTrip,
                    testReadIgnoresUnknown, testReadCrlfAndDrive,
                    testReadSameFileTwice, testReadStripsSuffix,
                    testReadErrors, testNameEncoding, testWriteSkipsEmptyFiles,
                    testHasData, testMerge, testMergeFiles,
                    testRemoveBuildTreeFiles },
                  false);
}
