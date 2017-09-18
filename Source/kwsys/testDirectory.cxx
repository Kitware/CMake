/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Directory.hxx)
#include KWSYS_HEADER(Encoding.hxx)
#include KWSYS_HEADER(SystemTools.hxx)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
#include "Directory.hxx.in"
#include "Encoding.hxx.in"
#include "SystemTools.hxx.in"
#endif

#include <fstream>
#include <iostream>
#include <sstream>

#include <testSystemTools.h>

int _doLongPathTest()
{
  using namespace kwsys;
  static const int LONG_PATH_THRESHOLD = 512;
  int res = 0;
  std::string topdir(TEST_SYSTEMTOOLS_BINARY_DIR "/directory_testing/");
  std::stringstream testpathstrm;
  std::string testdirpath;
  std::string extendedtestdirpath;

  testpathstrm << topdir;
  size_t pathlen = testpathstrm.str().length();
  testpathstrm.seekp(0, std::ios_base::end);
  while (pathlen < LONG_PATH_THRESHOLD) {
    testpathstrm << "0123456789/";
    pathlen = testpathstrm.str().length();
  }

  testdirpath = testpathstrm.str();
#ifdef _WIN32
  extendedtestdirpath =
    Encoding::ToNarrow(SystemTools::ConvertToWindowsExtendedPath(testdirpath));
#else
  extendedtestdirpath = testdirpath;
#endif

  if (SystemTools::MakeDirectory(extendedtestdirpath)) {
    std::ofstream testfile1(
      (extendedtestdirpath + "longfilepathtest1.txt").c_str());
    std::ofstream testfile2(
      (extendedtestdirpath + "longfilepathtest2.txt").c_str());
    testfile1 << "foo";
    testfile2 << "bar";
    testfile1.close();
    testfile2.close();

    Directory testdir;
    // Set res to failure if the directory doesn't load
    res += !testdir.Load(testdirpath);
    // Increment res failure if the directory appears empty
    res += testdir.GetNumberOfFiles() == 0;
    // Increment res failures if the path has changed from
    // what was provided.
    res += testdirpath != testdir.GetPath();

    SystemTools::RemoveADirectory(topdir);
  } else {
    std::cerr << "Failed to create directory with long path: "
              << extendedtestdirpath << std::endl;
    res += 1;
  }
  return res;
}

int testDirectory(int, char* [])
{
  return _doLongPathTest();
}
