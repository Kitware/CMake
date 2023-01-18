/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cm/filesystem>

namespace {

namespace fs = cm::filesystem;

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
    fs::path p;
    if (p != fs::path()) {
      result = false;
    }
  }
  {
    fs::path p1("/a/b/c");
    fs::path p2("/a/b/c");
    if (p1 != p2) {
      result = false;
    }
    if (p1.string() != p2.string()) {
      result = false;
    }
    if (p1.string() != "/a/b/c") {
      result = false;
    }
  }
  {
    std::string s("/a/b/c");
    fs::path p1(s);
    fs::path p2(s.begin(), s.end());
    if (p1 != p2) {
      result = false;
    }
    if (p1.string() != s || p2.string() != s) {
      result = false;
    }
#if CM_FILESYSTEM_SOURCE_TRAITS_ITERATOR
    std::string s2(s);
    s2 += '\0';
    fs::path p3(s2.begin());
    if (p1 != p3 || p3.string() != s) {
      result = false;
    }
#endif
  }
  {
    std::wstring s(L"/a/b/c");
    fs::path p1(s);
    fs::path p2(s.begin(), s.end());
    if (p1 != p2) {
      result = false;
    }
    if (p1.wstring() != s || p2.wstring() != s) {
      result = false;
    }
#if CM_FILESYSTEM_SOURCE_TRAITS_ITERATOR
    std::wstring s2(s);
    s2 += L'\0';
    fs::path p3(s2.begin());
    if (p1 != p3 || p3.wstring() != s) {
      result = false;
    }
#endif
  }
  {
    std::string s("/a/b/c");
    fs::path::string_type ws;
    for (auto c : s) {
      ws += fs::path::value_type(c);
    }
    fs::path p1(ws);
    fs::path p2(ws.begin(), ws.end());
    if (p1 != p2) {
      result = false;
    }
    if (p1.native() != ws || p2.native() != ws) {
      result = false;
    }
#if CM_FILESYSTEM_SOURCE_TRAITS_ITERATOR
    fs::path::string_type ws2(ws);
    ws2 += fs::path::value_type('\0');
    fs::path p3(ws2.begin());
    if (p1 != p3 || p3.native() != ws) {
      result = false;
    }
#endif
  }

  checkResult(result);

  return result;
}

bool testConcatenation()
{
  std::cout << "testConcatenation()";

  bool result = true;

  {
    fs::path p("/a/b");
    p /= "c";
    if (!(p.string() == "/a/b/c" || p.string() == "/a/b\\c")) {
      result = false;
    }
    p += "d";
    if (!(p.string() == "/a/b/cd" || p.string() == "/a/b\\cd")) {
      result = false;
    }
    fs::path p2("x/y");
    p /= p2;
    if (!(p.string() == "/a/b/cd/x/y" || p.string() == "/a/b\\cd\\x/y")) {
      result = false;
    }
    p = p / p2;
    if (!(p.string() == "/a/b/cd/x/y/x/y" ||
          p.string() == "/a/b\\cd\\x/y\\x/y")) {
      result = false;
    }
  }
  {
    fs::path p("a");
    p /= "";
    if (!(p.string() == "a/" || p.string() == "a\\")) {
      result = false;
    }
    p /= "/b";
    if (p.string() != "/b") {
      result = false;
    }
  }
#if defined(_WIN32)
  {
    fs::path p("a");
    p /= "c:/b";
    if (p.string() != "c:/b") {
      result = false;
    }
    p = fs::path("a") / "c:";
    if (p.string() != "c:") {
      result = false;
    }
    p = fs::path("c:") / "";
    if (p.string() != "c:") {
      result = false;
    }
    p = fs::path("c:a") / "/b";
    if (p.string() != "c:/b") {
      result = false;
    }
    p = fs::path("c:a") / "c:b";
    if (p.string() != "c:a\\b") {
      result = false;
    }
    p = fs::path("//host") / "b";
    if (p.string() != "//host\\b") {
      result = false;
    }
    p = fs::path("//host/") / "b";
    if (p.string() != "//host/b") {
      result = false;
    }
  }
#endif

  checkResult(result);

  return result;
}

bool testModifiers()
{
  std::cout << "testModifiers()";

  bool result = true;

  {
    std::string s("a///b/");
    fs::path p(s);
    std::replace(
      s.begin(), s.end(), '/',
      static_cast<std::string::value_type>(fs::path::preferred_separator));
    p.make_preferred();
    if (p.string() != s) {
      result = false;
    }
  }
  {
    fs::path p("a/b/c.e.f");
    p.remove_filename();
    if (p.string() != "a/b/") {
      result = false;
    }
    p.remove_filename();
    if (p.string() != "a/b/") {
      result = false;
    }
  }
  {
    fs::path p("a/b/c.e.f");
    p.replace_filename("x.y");
    if (p.string() != "a/b/x.y") {
      result = false;
    }
  }
  {
    fs::path p("a/b/c.e.f");
    p.replace_extension(".x");
    if (p.string() != "a/b/c.e.x") {
      result = false;
    }
    p.replace_extension(".y");
    if (p.string() != "a/b/c.e.y") {
      result = false;
    }
    p.replace_extension();
    if (p.string() != "a/b/c.e") {
      result = false;
    }
    p = "/a/b";
    p.replace_extension(".x");
    if (p.string() != "/a/b.x") {
      result = false;
    }
    p = "/a/b/";
    p.replace_extension(".x");
    if (p.string() != "/a/b/.x") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testObservers()
{
  std::cout << "testObservers()";

  bool result = true;

  {
    std::string s("a/b/c");
    fs::path p(s);
    fs::path::string_type st;
    for (auto c : s) {
      st += static_cast<fs::path::value_type>(c);
    }
    if (p.native() != st || static_cast<fs::path::string_type>(p) != st ||
        p.c_str() != st) {
      result = false;
    }
  }
  {
    std::string s("a//b//c");
    std::wstring ws(L"a//b//c");
    fs::path p(s);
    if (p.string() != s || p.wstring() != ws) {
      result = false;
    }
  }
  {
    std::string s("a/b/c");
    std::wstring ws;
    for (auto c : s) {
      ws += static_cast<std::wstring::value_type>(c);
    }
    std::string ns(s);
    std::replace(
      ns.begin(), ns.end(), '/',
      static_cast<std::string::value_type>(fs::path::preferred_separator));
    fs::path p(ns);
    if (p.generic_string() != s || p.generic_wstring() != ws) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testCompare()
{
  std::cout << "testCompare()";

  bool result = true;

  {
    std::string s("a/b/c");
    fs::path p1(s);
    fs::path p2(s);
    if (p1.compare(p2) != 0) {
      result = false;
    }
    p2 = "a/b";
    if (p1.compare(p2) <= 0) {
      result = false;
    }
    p2 = "a/d";
    if (p1.compare(p2) >= 0) {
      result = false;
    }
    p2 = "a/b/d";
    if (p1.compare(p2) >= 0) {
      result = false;
    }
    p2 = "a/b/a";
    if (p1.compare(p2) <= 0) {
      result = false;
    }
    p2 = "a/b/c/d";
    if (p1.compare(p2) >= 0) {
      result = false;
    }
    p1 = "a";
    p2 = "b";
    if (p1.compare(p2) == 0) {
      result = false;
    }
  }
  {
    // LWG 3096 (https://cplusplus.github.io/LWG/issue3096)
    // fs::path p1("/a/");
    // fs::path p2("/a/.");
    // if (p1.compare(p2) != 0) {
    //   result = false;
    // }
  }

  checkResult(result);

  return result;
}

bool testGeneration()
{
  std::cout << "testGeneration()";

  bool result = true;

  {
    fs::path p("a/./b/..");
    if (p.lexically_normal().generic_string() != "a/") {
      result = false;
    }
    p = "a/.///b/../";
    if (p.lexically_normal().generic_string() != "a/") {
      result = false;
    }
  }
#if defined(_WIN32)
  {
    fs::path p("//host/./b/..");
    if (p.lexically_normal().string() != "\\\\host\\") {
      result = false;
    }
    p = "//host/.///b/../";
    if (p.lexically_normal().string() != "\\\\host\\") {
      result = false;
    }
    p = "c://a/.///b/../";
    if (p.lexically_normal().string() != "c:\\a\\") {
      result = false;
    }
  }
#endif

  {
    if (fs::path("/a//d").lexically_relative("/a/b/c") != "../../d") {
      result = false;
    }
    if (fs::path("/a//b///c").lexically_relative("/a/d") != "../b/c") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a") != "b/c") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a/b/c/x/y") != "../..") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a/b/c") != ".") {
      result = false;
    }
    if (fs::path("a/b").lexically_relative("c/d") != "../../a/b") {
      result = false;
    }
  }
  {
#if defined(_WIN32)
    if (fs::path("/a/d").lexically_relative("e/d/c") != "/a/d") {
      result = false;
    }
    if (!fs::path("c:/a/d").lexically_relative("e/d/c").empty()) {
      result = false;
    }
#else
    if (!fs::path("/a/d").lexically_relative("e/d/c").empty()) {
      result = false;
    }
#endif
  }
  {
#if defined(_WIN32)
    if (fs::path("c:/a/d").lexically_proximate("e/d/c") != "c:/a/d") {
      result = false;
    }
#else
    if (fs::path("/a/d").lexically_proximate("e/d/c") != "/a/d") {
      result = false;
    }
#endif
    if (fs::path("/a/d").lexically_proximate("/a/b/c") != "../../d") {
      result = false;
    }
  }
  // LWG 3070
  {
#if defined(_WIN32)
    if (!fs::path("/a:/b:").lexically_relative("/a:/c:").empty()) {
      result = false;
    }
    if (fs::path("c:/a/b").lexically_relative("c:/a/d") != "../b") {
      result = false;
    }
    if (!fs::path("c:/a/b:").lexically_relative("c:/a/d").empty()) {
      result = false;
    }
    if (!fs::path("c:/a/b").lexically_relative("c:/a/d:").empty()) {
      result = false;
    }
#else
    if (fs::path("/a:/b:").lexically_relative("/a:/c:") != "../b:") {
      result = false;
    }
#endif
  }
  // LWG 3096
  {
    if (fs::path("/a").lexically_relative("/a/.") != ".") {
      result = false;
    }
    if (fs::path("/a").lexically_relative("/a/") != ".") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a/b/c") != ".") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a/b/c/") != ".") {
      result = false;
    }
    if (fs::path("a/b/c").lexically_relative("a/b/c/.") != ".") {
      result = false;
    }
    if (fs::path("a/b/c/").lexically_relative("a/b/c") != ".") {
      result = false;
    }
    if (fs::path("a/b/c/.").lexically_relative("a/b/c") != ".") {
      result = false;
    }
    if (fs::path("a/b/c/.").lexically_relative("a/b/c/") != ".") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testDecomposition()
{
  std::cout << "testDecomposition()";

  bool result = true;

  {
    if (!fs::path("/a/b").root_name().empty()) {
      result = false;
    }
#if defined(_WIN32)
    if (fs::path("c:/a/b").root_name() != "c:") {
      result = false;
    }
    if (fs::path("c:a/b").root_name() != "c:") {
      result = false;
    }
    if (fs::path("c:").root_name() != "c:") {
      result = false;
    }
    if (fs::path("//host/b").root_name() != "//host") {
      result = false;
    }
    if (fs::path("//host").root_name() != "//host") {
      result = false;
    }
#endif
  }
  {
    if (!fs::path("a/b").root_directory().empty()) {
      result = false;
    }
    if (fs::path("/a/b").root_directory() != "/") {
      result = false;
    }
#if defined(_WIN32)
    if (!fs::path("c:a/b").root_directory().empty()) {
      result = false;
    }
    if (fs::path("/a/b").root_directory() != "/") {
      result = false;
    }
    if (fs::path("c:/a/b").root_directory() != "/") {
      result = false;
    }
    if (fs::path("//host/b").root_directory() != "/") {
      result = false;
    }
#endif
  }
  {
    if (!fs::path("a/b").root_path().empty()) {
      result = false;
    }
    if (fs::path("/a/b").root_path() != "/") {
      result = false;
    }
#if defined(_WIN32)
    if (fs::path("c:a/b").root_path() != "c:") {
      result = false;
    }
    if (fs::path("/a/b").root_path() != "/") {
      result = false;
    }
    if (fs::path("c:/a/b").root_path() != "c:/") {
      result = false;
    }
    if (fs::path("//host/b").root_path() != "//host/") {
      result = false;
    }
#endif
  }
  {
    if (!fs::path("/").relative_path().empty()) {
      result = false;
    }
    if (fs::path("a/b").relative_path() != "a/b") {
      result = false;
    }
    if (fs::path("/a/b").relative_path() != "a/b") {
      result = false;
    }
#if defined(_WIN32)
    if (fs::path("c:a/b").relative_path() != "a/b") {
      result = false;
    }
    if (fs::path("/a/b").relative_path() != "a/b") {
      result = false;
    }
    if (fs::path("c:/a/b").relative_path() != "a/b") {
      result = false;
    }
    if (fs::path("//host/b").relative_path() != "b") {
      result = false;
    }
#endif
  }
  {
    if (fs::path("/a/b").parent_path() != "/a") {
      result = false;
    }
    if (fs::path("/a/b/").parent_path() != "/a/b") {
      result = false;
    }
    if (fs::path("/a/b/.").parent_path() != "/a/b") {
      result = false;
    }
    if (fs::path("/").parent_path() != "/") {
      result = false;
    }
#if defined(_WIN32)
    if (fs::path("c:/a/b").parent_path() != "c:/a") {
      result = false;
    }
    if (fs::path("c:a").parent_path() != "c:") {
      result = false;
    }
    if (fs::path("c:/").parent_path() != "c:/") {
      result = false;
    }
    if (fs::path("c:").parent_path() != "c:") {
      result = false;
    }
    if (fs::path("//host/").parent_path() != "//host/") {
      result = false;
    }
    if (fs::path("//host").parent_path() != "//host") {
      result = false;
    }
#endif
  }
  {
    if (fs::path("/a/b.txt").filename() != "b.txt") {
      result = false;
    }
    if (fs::path("/a/.b").filename() != ".b") {
      result = false;
    }
    if (fs::path("/foo/bar/").filename() != "") {
      result = false;
    }
    if (fs::path("/foo/.").filename() != ".") {
      result = false;
    }
    if (fs::path("/foo/..").filename() != "..") {
      result = false;
    }
    if (fs::path(".").filename() != ".") {
      result = false;
    }
    if (fs::path("..").filename() != "..") {
      result = false;
    }
    if (!fs::path("/").filename().empty()) {
      result = false;
    }
#if defined(_WIN32)
    if (fs::path("c:a").filename() != "a") {
      result = false;
    }
    if (fs::path("c:/a").filename() != "a") {
      result = false;
    }
    if (!fs::path("c:").filename().empty()) {
      result = false;
    }
    if (!fs::path("c:/").filename().empty()) {
      result = false;
    }
    if (!fs::path("//host").filename().empty()) {
      result = false;
    }
#endif
  }
  {
    if (fs::path("/a/b.txt").stem() != "b") {
      result = false;
    }
    if (fs::path("/a/b.c.txt").stem() != "b.c") {
      result = false;
    }
    if (fs::path("/a/.b").stem() != ".b") {
      result = false;
    }
    if (fs::path("/a/b").stem() != "b") {
      result = false;
    }
    if (fs::path("/a/b/.").stem() != ".") {
      result = false;
    }
    if (fs::path("/a/b/..").stem() != "..") {
      result = false;
    }
    if (!fs::path("/a/b/").stem().empty()) {
      result = false;
    }
#if defined(_WIN32)
    if (!fs::path("c:/a/b/").stem().empty()) {
      result = false;
    }
    if (!fs::path("c:/").stem().empty()) {
      result = false;
    }
    if (!fs::path("c:").stem().empty()) {
      result = false;
    }
    if (!fs::path("//host/").stem().empty()) {
      result = false;
    }
    if (!fs::path("//host").stem().empty()) {
      result = false;
    }
#endif
  }
  {
    if (fs::path("/a/b.txt").extension() != ".txt") {
      result = false;
    }
    if (fs::path("/a/b.").extension() != ".") {
      result = false;
    }
    if (!fs::path("/a/b").extension().empty()) {
      result = false;
    }
    if (fs::path("/a/b.txt/b.cc").extension() != ".cc") {
      result = false;
    }
    if (fs::path("/a/b.txt/b.").extension() != ".") {
      result = false;
    }
    if (!fs::path("/a/b.txt/b").extension().empty()) {
      result = false;
    }
    if (!fs::path("/a/.").extension().empty()) {
      result = false;
    }
    if (!fs::path("/a/..").extension().empty()) {
      result = false;
    }
    if (!fs::path("/a/.hidden").extension().empty()) {
      result = false;
    }
    if (fs::path("/a/..b").extension() != ".b") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testQueries()
{
  std::cout << "testQueries()";

  bool result = true;

  {
    if (fs::path("/a/b").has_root_name()) {
      result = false;
    }
    fs::path p("/a/b");
    if (!p.has_root_directory() || !p.has_root_path()) {
      result = false;
    }
    if (!fs::path("/a/b").has_root_path() || fs::path("a/b").has_root_path()) {
      result = false;
    }
    if (!fs::path("/a/b").has_relative_path() ||
        fs::path("/").has_relative_path()) {
      result = false;
    }
    if (!fs::path("/a/b").has_parent_path() ||
        !fs::path("/").has_parent_path() || fs::path("a").has_parent_path()) {
      result = false;
    }
    if (!fs::path("/a/b").has_filename() || !fs::path("a.b").has_filename() ||
        fs::path("/a/").has_filename() || fs::path("/").has_filename()) {
      result = false;
    }
    if (!fs::path("/a/b").has_stem() || !fs::path("a.b").has_stem() ||
        !fs::path("/.a").has_stem() || fs::path("/a/").has_stem() ||
        fs::path("/").has_stem()) {
      result = false;
    }
    if (!fs::path("/a/b.c").has_extension() ||
        !fs::path("a.b").has_extension() || fs::path("/.a").has_extension() ||
        fs::path("/a/").has_extension() || fs::path("/").has_extension()) {
      result = false;
    }
#if defined(_WIN32)
    p = "c:/a/b";
    if (!fs::path("c:/a/b").has_root_name() || !p.has_root_directory() ||
        !p.has_root_path()) {
      result = false;
    }
    p = "c:a/b";
    if (!p.has_root_name() || p.has_root_directory() || !p.has_root_path()) {
      result = false;
    }
    p = "//host/b";
    if (!p.has_root_name() || !p.has_root_directory() || !p.has_root_path()) {
      result = false;
    }
    p = "//host";
    if (!p.has_root_name() || p.has_root_directory() || !p.has_root_path()) {
      result = false;
    }
    if (!fs::path("c:/a/b").has_relative_path() ||
        !fs::path("c:a/b").has_relative_path() ||
        !fs::path("//host/b").has_relative_path()) {
      result = false;
    }
    if (!fs::path("c:/a/b").has_parent_path() ||
        !fs::path("c:/").has_parent_path() ||
        !fs::path("c:").has_parent_path() ||
        !fs::path("//host/").has_parent_path() ||
        !fs::path("//host").has_parent_path()) {
      result = false;
    }
#endif
  }
  {
#if defined(_WIN32)
    fs::path p("c:/a");
#else
    fs::path p("/a");
#endif
    if (!p.is_absolute() || p.is_relative()) {
      result = false;
    }
    p = "a/b";
    if (p.is_absolute() || !p.is_relative()) {
      result = false;
    }
#if defined(_WIN32)
    p = "c:/a/b";
    if (!p.is_absolute() || p.is_relative()) {
      result = false;
    }
    p = "//host/b";
    if (!p.is_absolute() || p.is_relative()) {
      result = false;
    }
    p = "/a";
    if (p.is_absolute() || !p.is_relative()) {
      result = false;
    }
    p = "c:a";
    if (p.is_absolute() || !p.is_relative()) {
      result = false;
    }
#endif
  }

  checkResult(result);

  return result;
}

bool testIterators()
{
  std::cout << "testIterators()";

  bool result = true;

  {
    fs::path p("/a/b/");
#if defined(_WIN32)
    std::vector<fs::path::string_type> ref{ L"/", L"a", L"b", L"" };
#else
    std::vector<fs::path::string_type> ref{ "/", "a", "b", "" };
#endif
    std::vector<fs::path::string_type> res;
    for (auto i = p.begin(), e = p.end(); i != e; ++i) {
      res.push_back(*i);
    }
    if (res != ref) {
      result = false;
    }
    res.clear();
    for (const auto& e : p) {
      res.push_back(e);
    }
    if (res != ref) {
      result = false;
    }
  }
  {
    fs::path p("/a/b/");
#if defined(_WIN32)
    std::vector<fs::path::string_type> ref{ L"", L"b", L"a", L"/" };
#else
    std::vector<fs::path::string_type> ref{ "", "b", "a", "/" };
#endif
    std::vector<fs::path::string_type> res;
    auto i = p.end(), b = p.begin();
    do {
      res.push_back(*--i);
    } while (i != b);
    if (res != ref) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testNonMemberFunctions()
{
  std::cout << "testNonMemberFunctions()";

  bool result = true;

  {
    fs::path p1("/a/b/");
    fs::path p2("/c/d");
    fs::swap(p1, p2);
    if (p1.string() != "/c/d" || p2.string() != "/a/b/")
      result = false;
  }
  {
    auto h1 = fs::hash_value(fs::path("/a//b//"));
    auto h2 = fs::hash_value(fs::path("/a/b/"));
    if (h1 != h2)
      result = false;
  }
  {
    fs::path p1("/a/b/");
    fs::path p2("/c/d");
    if (p1 == p2)
      result = false;
    p1 = "/a//b//";
    p2 = "/a/b/";
    if (p1 != p2)
      result = false;
  }
  {
    fs::path p = "/a";
    p = p / "b" / "c";
    if (p.generic_string() != "/a/b/c") {
      result = false;
    }
    fs::path::string_type ref;
    ref += fs::path::value_type('/');
    ref += fs::path::value_type('a');
    ref += fs::path::preferred_separator;
    ref += fs::path::value_type('b');
    ref += fs::path::preferred_separator;
    ref += fs::path::value_type('c');
    if (p.native() != ref) {
      result = false;
    }
  }
  {
    fs::path p("/a b\\c/");
    std::ostringstream oss;
    oss << p;
    if (oss.str() != "\"/a b\\\\c/\"") {
      result = false;
    }
    std::istringstream iss(oss.str());
    fs::path p2;
    iss >> p2;
    if (p2 != p) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}
}

int testCMFilesystemPath(int /*unused*/, char* /*unused*/[])
{
  int result = 0;

  if (!testConstructors()) {
    result = 1;
  }
  if (!testConcatenation()) {
    result = 1;
  }
  if (!testModifiers()) {
    result = 1;
  }
  if (!testObservers()) {
    result = 1;
  }
  if (!testCompare()) {
    result = 1;
  }
  if (!testGeneration()) {
    result = 1;
  }
  if (!testDecomposition()) {
    result = 1;
  }
  if (!testQueries()) {
    result = 1;
  }
  if (!testIterators()) {
    result = 1;
  }
  if (!testNonMemberFunctions()) {
    result = 1;
  }

  return result;
}
