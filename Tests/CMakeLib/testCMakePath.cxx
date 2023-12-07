/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include <iostream>
#include <string>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmCMakePath.h"

namespace {

void checkResult(bool success)
{
  if (!success) {
    std::cout << " => failed";
  }
  std::cout << std::endl;
}

bool testConstructors()
{
  std::cout << "testConstructors()";

  bool result = true;

  {
    cmCMakePath path;
    if (!path.String().empty() || path != cmCMakePath{}) {
      result = false;
    }
  }
  {
    cmCMakePath path{ "aa/bb" };
    if (path.String() != "aa/bb") {
      result = false;
    }
  }
  {
    std::string s{ "aa/bb" };
    cmCMakePath path{ s };
    if (path.String() != "aa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path{ "aa/bb"_s };
    if (path.String() != "aa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2("aa/bb"_s);

    if (path1 != path2) {
      result = false;
    }
    if (path1.String() != "aa/bb") {
      result = false;
    }
    if (path1.String() != path2.String()) {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ path1 };

    if (path1 != path2) {
      result = false;
    }
    if (path1.String() != "aa/bb") {
      result = false;
    }
    if (path1.String() != path2.String()) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testAssign()
{
  std::cout << "testAssign()";

  bool result = true;

  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 = path1;
    if (path1 != path2) {
      result = false;
    }
    if (path1.String() != "aa/bb") {
      result = false;
    }
    if (path1.String() != path2.String()) {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 = std::move(path1);
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 = path1;
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 = std::move(path1);
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 = path1;
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2 = path1;
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Assign(path1);
    if (path2.String() != path1) {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Assign(std::move(path1));
    if (path2.String() != "aa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Assign(path1);
    if (path2.String() != path1) {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2.Assign(path1);
    if (path2.String() != path1) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testConcat()
{
  std::cout << "testConcat()";

  bool result = true;

  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 += path1;

    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 += std::move(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 += path1;
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 += std::move(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 += path1;
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2 += path1;
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Concat(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Concat(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Concat(std::move(path1));
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Concat(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2.Concat(path1);
    if (path2.String() != "cc/ddaa/bb") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testAppend()
{
  std::cout << "testAppend()";

  bool result = true;

  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 /= path1;

    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 /= std::move(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 /= path1;
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 /= std::move(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2 /= path1;
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2 /= path1;
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    cmCMakePath path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Append(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Append(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    std::string path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Append(std::move(path1));
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    cm::string_view path1{ "aa/bb" };
    cmCMakePath path2{ "cc/dd" };

    path2.Append(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }
  {
    char path1[] = "aa/bb";
    cmCMakePath path2{ "cc/dd" };

    path2.Append(path1);
    if (path2.String() != "cc/dd/aa/bb") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}
}

int testCMakePath(int /*unused*/, char* /*unused*/[])
{
  int result = 0;

  if (!testConstructors()) {
    result = 1;
  }
  if (!testAssign()) {
    result = 1;
  }
  if (!testConcat()) {
    result = 1;
  }
  if (!testAppend()) {
    result = 1;
  }

  return result;
}
