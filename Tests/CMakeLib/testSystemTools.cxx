/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cmConfigure.h> // IWYU pragma: keep

#include <string>
#include <vector>

#include <stddef.h>

#include "cmSystemTools.h"

#include "testCommon.h"

static bool testUpperCase()
{
  std::cout << "testUpperCase()\n";
  ASSERT_EQUAL(cmSystemTools::UpperCase("abc"), "ABC");
  return true;
}

static bool testVersionCompare()
{
  std::cout << "testVersionCompare()\n";
  ASSERT_TRUE(cmSystemTools::VersionCompareEqual("", ""));
  ASSERT_TRUE(!cmSystemTools::VersionCompareGreater("", ""));
  ASSERT_TRUE(cmSystemTools::VersionCompareEqual("1", "1a"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareGreater("1", "1a"));
  ASSERT_TRUE(cmSystemTools::VersionCompareEqual("001", "1"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareGreater("001", "1"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareEqual("002", "1"));
  ASSERT_TRUE(cmSystemTools::VersionCompareGreater("002", "1"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareEqual("6.2.1", "6.3.1"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareGreater("6.2.1", "6.3.1"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareEqual("6.2.1", "6.2"));
  ASSERT_TRUE(cmSystemTools::VersionCompareGreater("6.2.1", "6.2"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareEqual(
    "3.14159265358979323846264338327950288419716939937510582097494459230",
    "3.14159265358979323846264338327950288419716939937510582097494459231"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareGreater(
    "3.14159265358979323846264338327950288419716939937510582097494459230",
    "3.14159265358979323846264338327950288419716939937510582097494459231"));
  ASSERT_TRUE(!cmSystemTools::VersionCompareEqual(
    "3.141592653589793238462643383279502884197169399375105820974944592307",
    "3.14159265358979323846264338327950288419716939937510582097494459231"));
  ASSERT_TRUE(cmSystemTools::VersionCompareGreater(
    "3.141592653589793238462643383279502884197169399375105820974944592307",
    "3.14159265358979323846264338327950288419716939937510582097494459231"));
  return true;
}

static bool testStrVersCmp()
{
  std::cout << "testStrVersCmp()\n";
  ASSERT_TRUE(cmSystemTools::strverscmp("", "") == 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("abc", "") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("abc", "abc") == 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("abd", "abc") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("abc", "abd") < 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("12345", "12344") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("100", "99") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("12345", "00345") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("99999999999999", "99999999999991") >
              0);
  ASSERT_TRUE(cmSystemTools::strverscmp("00000000000009", "00000000000001") >
              0);
  ASSERT_TRUE(cmSystemTools::strverscmp("a.b.c.0", "a.b.c.000") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("lib_1.2_10", "lib_1.2_2") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("12lib", "2lib") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("02lib", "002lib") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("10", "9a") > 0);
  ASSERT_TRUE(cmSystemTools::strverscmp("000", "0001") > 0);

  // test sorting using standard strvercmp input
  std::vector<std::string> testString;
  testString.push_back("000");
  testString.push_back("00");
  testString.push_back("01");
  testString.push_back("010");
  testString.push_back("09");
  testString.push_back("0");
  testString.push_back("1");
  testString.push_back("9");
  testString.push_back("10");

  // test global ordering of input strings
  for (size_t i = 0; i < testString.size() - 1; i++) {
    for (size_t j = i + 1; j < testString.size(); j++) {
      if (cmSystemTools::strverscmp(testString[i], testString[j]) >= 0) {
        std::cout << "cmSystemTools::strverscmp error in comparing strings "
                  << testString[i] << ' ' << testString[j] << '\n';
        return false;
      }
    }
  }
  return true;
}

static bool testMakeTempDirectory()
{
  std::cout << "testMakeTempDirectory()\n";

  static std::string const kTemplate = "testMakeTempDirectory-XXXXXX";
  std::string tempDir = kTemplate;
  cmsys::Status status = cmSystemTools::MakeTempDirectory(tempDir);
  if (!status) {
    std::cout << "cmSystemTools::MakeTempDirectory failed on \"" << tempDir
              << "\": " << status.GetString() << '\n';
    return false;
  }
  if (!cmSystemTools::FileIsDirectory(tempDir)) {
    std::cout << "cmSystemTools::MakeTempDirectory did not create \""
              << tempDir << '\n';
    return false;
  }
  cmSystemTools::RemoveADirectory(tempDir);
  ASSERT_TRUE(tempDir != kTemplate);
  return true;
}

int testSystemTools(int /*unused*/, char* /*unused*/[])
{
  return runTests({
    testUpperCase,
    testVersionCompare,
    testStrVersCmp,
    testMakeTempDirectory,
  });
}
