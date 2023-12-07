/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmString.hxx"

#define ASSERT_TRUE(x)                                                        \
  do {                                                                        \
    if (!(x)) {                                                               \
      std::cout << "ASSERT_TRUE(" #x ") failed on line " << __LINE__ << "\n"; \
      return false;                                                           \
    }                                                                         \
  } while (false)

static bool testConstructDefault()
{
  std::cout << "testConstructDefault()\n";
  cm::String str;
  cm::String const& str_const = str;
  ASSERT_TRUE(bool(str_const) == false);
  ASSERT_TRUE(str_const.data() == nullptr);
  ASSERT_TRUE(str_const.size() == 0);
  ASSERT_TRUE(str_const.empty());
  ASSERT_TRUE(str_const.is_stable());
  ASSERT_TRUE(str.c_str() == nullptr);
  ASSERT_TRUE(str.str().empty());
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testFromNullPtr(cm::String str)
{
  cm::String const& str_const = str;
  ASSERT_TRUE(bool(str_const) == false);
  ASSERT_TRUE(str_const.data() == nullptr);
  ASSERT_TRUE(str_const.size() == 0);
  ASSERT_TRUE(str_const.empty());
  ASSERT_TRUE(str_const.is_stable());
  ASSERT_TRUE(str.c_str() == nullptr);
  ASSERT_TRUE(str.str().empty());
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromNullPtr()
{
  std::cout << "testConstructFromNullPtr()\n";
  return testFromNullPtr(nullptr);
}

static bool testAssignFromNullPtr()
{
  std::cout << "testAssignFromNullPtr()\n";
  cm::String str;
  str = nullptr;
  return testFromNullPtr(str);
}

static bool testFromCStrNull(cm::String str)
{
  cm::String const& str_const = str;
  ASSERT_TRUE(bool(str_const) == false);
  ASSERT_TRUE(str_const.data() == nullptr);
  ASSERT_TRUE(str_const.size() == 0);
  ASSERT_TRUE(str_const.empty());
  ASSERT_TRUE(str_const.is_stable());
  ASSERT_TRUE(str.c_str() == nullptr);
  ASSERT_TRUE(str.str().empty());
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromCStrNull()
{
  std::cout << "testConstructFromCStrNull()\n";
  const char* null = nullptr;
  return testFromCStrNull(null);
}

static bool testAssignFromCStrNull()
{
  std::cout << "testAssignFromCStrNull()\n";
  const char* null = nullptr;
  cm::String str;
  str = null;
  return testFromCStrNull(str);
}

static char const charArray[] = "abc";

static bool testFromCharArray(cm::String str)
{
  cm::String const& str_const = str;
  ASSERT_TRUE(str_const.data() != charArray);
  ASSERT_TRUE(str_const.size() == sizeof(charArray) - 1);
  ASSERT_TRUE(str_const.is_stable());
  ASSERT_TRUE(str.c_str() != charArray);
  ASSERT_TRUE(str.is_stable());
  cm::String substr = str.substr(1);
  cm::String const& substr_const = substr;
  ASSERT_TRUE(substr_const.data() != &charArray[1]);
  ASSERT_TRUE(substr_const.size() == 2);
  ASSERT_TRUE(!substr_const.is_stable());
  ASSERT_TRUE(substr.c_str() != &charArray[1]);
  ASSERT_TRUE(!substr.is_stable());
  return true;
}

static bool testConstructFromCharArray()
{
  std::cout << "testConstructFromCharArray()\n";
  return testFromCharArray(charArray);
}

static bool testAssignFromCharArray()
{
  std::cout << "testAssignFromCharArray()\n";
  cm::String str;
  str = charArray;
  return testFromCharArray(str);
}

static const char* cstr = "abc";

static bool testFromCStr(cm::String const& str)
{
  ASSERT_TRUE(str.data() != cstr);
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), cstr, 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromCStr()
{
  std::cout << "testConstructFromCStr()\n";
  return testFromCStr(cstr);
}

static bool testAssignFromCStr()
{
  std::cout << "testAssignFromCStr()\n";
  cm::String str;
  str = cstr;
  return testFromCStr(str);
}

static const std::string stdstr = "abc";

static bool testFromStdString(cm::String const& str)
{
#if defined(_GLIBCXX_USE_CXX11_ABI) && _GLIBCXX_USE_CXX11_ABI
  // It would be nice to check this everywhere, but several platforms
  // still use a CoW implementation even in C++11.
  ASSERT_TRUE(str.data() != stdstr.data());
#endif
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), stdstr.data(), 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromStdString()
{
  std::cout << "testConstructFromStdString()\n";
  return testFromStdString(stdstr);
}

static bool testAssignFromStdString()
{
  std::cout << "testAssignFromStdString()\n";
  cm::String str;
  str = stdstr;
  return testFromStdString(str);
}

static bool testConstructFromView()
{
  std::cout << "testConstructFromView()\n";
  cm::string_view view = cstr;
  return testFromCStr(cm::String(view));
}

static bool testAssignFromView()
{
  std::cout << "testAssignFromView()\n";
  cm::string_view view = cstr;
  cm::String str;
  str = view;
  return testFromCStr(str);
}

static bool testFromChar(cm::String const& str)
{
  ASSERT_TRUE(str.size() == 1);
  ASSERT_TRUE(std::strncmp(str.data(), "a", 1) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromChar()
{
  std::cout << "testConstructFromChar()\n";
  return testFromChar('a');
}

static bool testAssignFromChar()
{
  std::cout << "testAssignFromChar()\n";
  cm::String str;
  str = 'a';
  return testFromChar(str);
}

static bool testConstructFromInitList()
{
  std::cout << "testConstructFromInitList()\n";
  cm::String const str{ 'a', 'b', 'c' };
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), "abc", 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testAssignFromInitList()
{
  std::cout << "testAssignFromInitList()\n";
  cm::String str;
  str = { 'a', 'b', 'c' };
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), "abc", 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromBuffer()
{
  std::cout << "testConstructFromBuffer()\n";
  cm::String const str(cstr, 3);
  return testFromCStr(str);
}

static bool testConstructFromInputIterator()
{
  std::cout << "testConstructFromInputIterator()\n";
  cm::String const str(cstr, cstr + 3);
  ASSERT_TRUE(str.data() != cstr);
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), cstr, 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testConstructFromN()
{
  std::cout << "testConstructFromN()\n";
  cm::String const str(3, 'a');
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), "aaa", 3) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static const auto staticStringView = "abc"_s;

static bool testFromStaticStringView(cm::String str)
{
  cm::String const& str_const = str;
  ASSERT_TRUE(str_const.data() == staticStringView.data());
  ASSERT_TRUE(str_const.size() == staticStringView.size());
  ASSERT_TRUE(!str_const.is_stable());
  ASSERT_TRUE(str.c_str() == staticStringView);
  ASSERT_TRUE(!str.is_stable());
  cm::String substr = str.substr(1);
  cm::String const& substr_const = substr;
  ASSERT_TRUE(substr_const.data() == &staticStringView[1]);
  ASSERT_TRUE(substr_const.size() == 2);
  ASSERT_TRUE(!substr_const.is_stable());
  ASSERT_TRUE(substr.c_str() == &staticStringView[1]);
  ASSERT_TRUE(!substr.is_stable());
  return true;
}

static bool testConstructFromStaticStringView()
{
  std::cout << "testConstructFromStaticStringView()\n";
  return testFromStaticStringView(cm::String(staticStringView));
}

static bool testAssignFromStaticStringView()
{
  std::cout << "testAssignFromStaticStringView()\n";
  cm::String str;
  str = staticStringView;
  return testFromStaticStringView(str);
}

static bool testConstructCopy()
{
  std::cout << "testConstructCopy()\n";
  cm::String s1 = std::string("abc");
  cm::String s2 = s1;
  ASSERT_TRUE(s1.data() == s2.data());
  ASSERT_TRUE(s1.size() == 3);
  ASSERT_TRUE(s2.size() == 3);
  ASSERT_TRUE(std::strncmp(s2.data(), "abc", 3) == 0);
  ASSERT_TRUE(s1.is_stable());
  ASSERT_TRUE(s2.is_stable());
  return true;
}

static bool testConstructMove()
{
  std::cout << "testConstructMove()\n";
  cm::String s1 = std::string("abc");
  cm::String s2 = std::move(s1);
#ifndef __clang_analyzer__ /* cplusplus.Move */
  ASSERT_TRUE(s1.data() == nullptr);
  ASSERT_TRUE(s1.size() == 0);
  ASSERT_TRUE(s2.size() == 3);
  ASSERT_TRUE(std::strncmp(s2.data(), "abc", 3) == 0);
  ASSERT_TRUE(s1.is_stable());
  ASSERT_TRUE(s2.is_stable());
#endif
  return true;
}

static bool testAssignCopy()
{
  std::cout << "testAssignCopy()\n";
  cm::String s1 = std::string("abc");
  cm::String s2;
  s2 = s1;
  ASSERT_TRUE(s1.data() == s2.data());
  ASSERT_TRUE(s1.size() == 3);
  ASSERT_TRUE(s2.size() == 3);
  ASSERT_TRUE(std::strncmp(s2.data(), "abc", 3) == 0);
  ASSERT_TRUE(s1.is_stable());
  ASSERT_TRUE(s2.is_stable());
  return true;
}

static bool testAssignMove()
{
  std::cout << "testAssignMove()\n";
  cm::String s1 = std::string("abc");
  cm::String s2;
  s2 = std::move(s1);
#ifndef __clang_analyzer__ /* cplusplus.Move */
  ASSERT_TRUE(s1.data() == nullptr);
  ASSERT_TRUE(s1.size() == 0);
  ASSERT_TRUE(s2.size() == 3);
  ASSERT_TRUE(std::strncmp(s2.data(), "abc", 3) == 0);
  ASSERT_TRUE(s1.is_stable());
  ASSERT_TRUE(s2.is_stable());
#endif
  return true;
}

static bool testOperatorBool()
{
  std::cout << "testOperatorBool()\n";
  cm::String str;
  ASSERT_TRUE(!str);
  str = "";
  ASSERT_TRUE(str);
  str = static_cast<const char*>(nullptr);
  ASSERT_TRUE(!str);
  str = std::string();
  ASSERT_TRUE(str);
  str = nullptr;
  ASSERT_TRUE(!str);
  return true;
}

static bool testOperatorIndex()
{
  std::cout << "testOperatorIndex()\n";
  cm::String str = "abc";
  ASSERT_TRUE(str[0] == 'a');
  ASSERT_TRUE(str[1] == 'b');
  ASSERT_TRUE(str[2] == 'c');
  return true;
}

static bool testOperatorPlusEqual()
{
  std::cout << "testOperatorPlusEqual()\n";
  cm::String str = "a";
  str += "b";
  {
    const char* c = "c";
    str += c;
  }
  str += 'd';
  str += std::string("e");
  str += cm::string_view("f", 1);
  str += cm::String("g");
  ASSERT_TRUE(str.size() == 7);
  ASSERT_TRUE(std::strncmp(str.data(), "abcdefg", 7) == 0);
  ASSERT_TRUE(str.is_stable());
  return true;
}

static bool testOperatorCompare()
{
  std::cout << "testOperatorCompare()\n";
  cm::String str = "b";
  {
    ASSERT_TRUE(str == "b");
    ASSERT_TRUE("b" == str);
    ASSERT_TRUE(str != "a");
    ASSERT_TRUE("a" != str);
    ASSERT_TRUE(str < "c");
    ASSERT_TRUE("a" < str);
    ASSERT_TRUE(str > "a");
    ASSERT_TRUE("c" > str);
    ASSERT_TRUE(str <= "b");
    ASSERT_TRUE("b" <= str);
    ASSERT_TRUE(str >= "b");
    ASSERT_TRUE("b" >= str);
  }
  {
    const char* a = "a";
    const char* b = "b";
    const char* c = "c";
    ASSERT_TRUE(str == b);
    ASSERT_TRUE(b == str);
    ASSERT_TRUE(str != a);
    ASSERT_TRUE(a != str);
    ASSERT_TRUE(str < c);
    ASSERT_TRUE(a < str);
    ASSERT_TRUE(str > a);
    ASSERT_TRUE(c > str);
    ASSERT_TRUE(str <= b);
    ASSERT_TRUE(b <= str);
    ASSERT_TRUE(str >= b);
    ASSERT_TRUE(b >= str);
  }
  {
    ASSERT_TRUE(str == 'b');
    ASSERT_TRUE('b' == str);
    ASSERT_TRUE(str != 'a');
    ASSERT_TRUE('a' != str);
    ASSERT_TRUE(str < 'c');
    ASSERT_TRUE('a' < str);
    ASSERT_TRUE(str > 'a');
    ASSERT_TRUE('c' > str);
    ASSERT_TRUE(str <= 'b');
    ASSERT_TRUE('b' <= str);
    ASSERT_TRUE(str >= 'b');
    ASSERT_TRUE('b' >= str);
  }
  {
    std::string const a = "a";
    std::string const b = "b";
    std::string const c = "c";
    ASSERT_TRUE(str == b);
    ASSERT_TRUE(b == str);
    ASSERT_TRUE(str != a);
    ASSERT_TRUE(a != str);
    ASSERT_TRUE(str < c);
    ASSERT_TRUE(a < str);
    ASSERT_TRUE(str > a);
    ASSERT_TRUE(c > str);
    ASSERT_TRUE(str <= b);
    ASSERT_TRUE(b <= str);
    ASSERT_TRUE(str >= b);
    ASSERT_TRUE(b >= str);
  }
  {
    cm::string_view const a("a", 1);
    cm::string_view const b("b", 1);
    cm::string_view const c("c", 1);
    ASSERT_TRUE(str == b);
    ASSERT_TRUE(b == str);
    ASSERT_TRUE(str != a);
    ASSERT_TRUE(a != str);
    ASSERT_TRUE(str < c);
    ASSERT_TRUE(a < str);
    ASSERT_TRUE(str > a);
    ASSERT_TRUE(c > str);
    ASSERT_TRUE(str <= b);
    ASSERT_TRUE(b <= str);
    ASSERT_TRUE(str >= b);
    ASSERT_TRUE(b >= str);
  }
  {
    cm::String const a("a");
    cm::String const b("b");
    cm::String const c("c");
    ASSERT_TRUE(str == b);
    ASSERT_TRUE(b == str);
    ASSERT_TRUE(str != a);
    ASSERT_TRUE(a != str);
    ASSERT_TRUE(str < c);
    ASSERT_TRUE(a < str);
    ASSERT_TRUE(str > a);
    ASSERT_TRUE(c > str);
    ASSERT_TRUE(str <= b);
    ASSERT_TRUE(b <= str);
    ASSERT_TRUE(str >= b);
    ASSERT_TRUE(b >= str);
  }
  return true;
}

static bool testOperatorStream()
{
  std::cout << "testOperatorStream()\n";
  std::ostringstream ss;
  ss << "a" << cm::String("b") << 'c';
  ASSERT_TRUE(ss.str() == "abc");
  return true;
}

static bool testOperatorStdStringPlusEqual()
{
  std::cout << "testOperatorStdStringPlusEqual()\n";
  std::string s = "a";
  s += cm::String("b");
  ASSERT_TRUE(s == "ab");
  return true;
}

static bool testMethod_borrow()
{
  std::cout << "testMethod_borrow()\n";
  std::string s = "abc";
  cm::String str = cm::String::borrow(s);
  ASSERT_TRUE(str.data() == s.data());
  ASSERT_TRUE(str.size() == s.size());
  ASSERT_TRUE(str == s);
  return true;
}

static bool testMethod_view()
{
  std::cout << "testMethod_view()\n";
  cm::String str;
  ASSERT_TRUE(str.view().data() == nullptr);
  ASSERT_TRUE(str.view().size() == 0);
  str = charArray;
  ASSERT_TRUE(str.view().data() != charArray);
  ASSERT_TRUE(str.view().size() == sizeof(charArray) - 1);
  str = std::string("abc");
  ASSERT_TRUE(str.view().size() == 3);
  ASSERT_TRUE(strncmp(str.view().data(), "abc", 3) == 0);
  return true;
}

static bool testMethod_empty()
{
  std::cout << "testMethod_empty()\n";
  cm::String str;
  ASSERT_TRUE(str.empty());
  str = "";
  ASSERT_TRUE(str.empty());
  str = "abc";
  ASSERT_TRUE(!str.empty());
  str = std::string();
  ASSERT_TRUE(str.empty());
  str = std::string("abc");
  ASSERT_TRUE(!str.empty());
  return true;
}

static bool testMethod_length()
{
  std::cout << "testMethod_length()\n";
  cm::String str;
  ASSERT_TRUE(str.length() == 0);
  str = "";
  ASSERT_TRUE(str.length() == 0);
  str = "abc";
  ASSERT_TRUE(str.length() == 3);
  str = std::string();
  ASSERT_TRUE(str.length() == 0);
  str = std::string("abc");
  ASSERT_TRUE(str.length() == 3);
  return true;
}

static bool testMethod_at()
{
  std::cout << "testMethod_at()\n";
  cm::String str = "abc";
  ASSERT_TRUE(str.at(0) == 'a');
  ASSERT_TRUE(str.at(1) == 'b');
  ASSERT_TRUE(str.at(2) == 'c');
  bool except_out_of_range = false;
  try {
    str.at(3);
  } catch (std::out_of_range&) {
    except_out_of_range = true;
  }
  ASSERT_TRUE(except_out_of_range);
  return true;
}

static bool testMethod_front_back()
{
  std::cout << "testMethod_front_back()\n";
  cm::String str = "abc";
  ASSERT_TRUE(str.front() == 'a');
  ASSERT_TRUE(str.back() == 'c');
  return true;
}

static bool testMethodIterators()
{
  std::cout << "testMethodIterators()\n";
  cm::String str = "abc";
  ASSERT_TRUE(*str.begin() == 'a');
  ASSERT_TRUE(*(str.end() - 1) == 'c');
  ASSERT_TRUE(str.end() - str.begin() == 3);
  ASSERT_TRUE(*str.cbegin() == 'a');
  ASSERT_TRUE(*(str.cend() - 1) == 'c');
  ASSERT_TRUE(str.cend() - str.cbegin() == 3);
  ASSERT_TRUE(*str.rbegin() == 'c');
  ASSERT_TRUE(*(str.rend() - 1) == 'a');
  ASSERT_TRUE(str.rend() - str.rbegin() == 3);
  ASSERT_TRUE(*str.crbegin() == 'c');
  ASSERT_TRUE(*(str.crend() - 1) == 'a');
  ASSERT_TRUE(str.crend() - str.crbegin() == 3);
  return true;
}

static bool testMethod_clear()
{
  std::cout << "testMethod_clear()\n";
  cm::String str = "abc";
  ASSERT_TRUE(!str.empty());
  str.clear();
  ASSERT_TRUE(str.empty());
  return true;
}

static bool testMethod_insert()
{
  std::cout << "testMethod_insert()\n";
  cm::String str = "abc";
  str.insert(1, 2, 'd').insert(0, 1, '_');
  ASSERT_TRUE(str.size() == 6);
  ASSERT_TRUE(std::strncmp(str.data(), "_addbc", 6) == 0);
  return true;
}

static bool testMethod_erase()
{
  std::cout << "testMethod_erase()\n";
  cm::String str = "abcdefg";
  str.erase(5).erase(1, 2);
  ASSERT_TRUE(str.size() == 3);
  ASSERT_TRUE(std::strncmp(str.data(), "ade", 3) == 0);
  return true;
}

static bool testMethod_push_back()
{
  std::cout << "testMethod_push_back()\n";
  cm::String str = "abc";
  str.push_back('d');
  ASSERT_TRUE(str == "abcd");
  return true;
}

static bool testMethod_pop_back()
{
  std::cout << "testMethod_pop_back()\n";
  cm::String str = "abc";
  str.pop_back();
  ASSERT_TRUE(str == "ab");
  return true;
}

static bool testMethod_replace()
{
  std::cout << "testMethod_replace()\n";
  {
    cm::String str = "abcd";
    const char* bc = "bc";
    ASSERT_TRUE(str.replace(1, 2, "BC") == "aBCd");
    ASSERT_TRUE(str.replace(1, 2, bc) == "abcd");
    ASSERT_TRUE(str.replace(1, 2, 'x') == "axd");
    ASSERT_TRUE(str.replace(1, 1, std::string("bc")) == "abcd");
    ASSERT_TRUE(str.replace(1, 2, cm::string_view("BC", 2)) == "aBCd");
    ASSERT_TRUE(str.replace(1, 2, cm::String("bc")) == "abcd");
  }
  {
    cm::String str = "abcd";
    const char* bc = "bc";
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3, "BC") == "aBCd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3, bc) == "abcd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3, 'x') == "axd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 2,
                            std::string("bc")) == "abcd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3,
                            cm::string_view("BC", 2)) == "aBCd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3,
                            cm::String("bc")) == "abcd");
  }
  {
    cm::String str = "abcd";
    const char* bc = "_bc";
    ASSERT_TRUE(str.replace(1, 2, "_BC_", 1, 2) == "aBCd");
    ASSERT_TRUE(str.replace(1, 2, bc, 1) == "abcd");
    ASSERT_TRUE(str.replace(1, 2, 'x', 0) == "axd");
    ASSERT_TRUE(str.replace(1, 1, std::string("_bc_"), 1, 2) == "abcd");
    ASSERT_TRUE(str.replace(1, 2, cm::string_view("_BC", 3), 1) == "aBCd");
    ASSERT_TRUE(str.replace(1, 2, cm::String("_bc_"), 1, 2) == "abcd");
  }
  {
    cm::String str = "abcd";
    const char* bc = "_bc_";
    ASSERT_TRUE(str.replace(1, 2, 2, 'x') == "axxd");
    ASSERT_TRUE(str.replace(str.begin() + 1, str.begin() + 3, 2, 'y') ==
                "ayyd");
    ASSERT_TRUE(
      str.replace(str.begin() + 1, str.begin() + 3, bc + 1, bc + 3) == "abcd");
  }
  return true;
}

static bool testMethod_copy()
{
  std::cout << "testMethod_copy()\n";
  cm::String str = "abc";
  char dest[2];
  cm::String::size_type n = str.copy(dest, 2, 1);
  ASSERT_TRUE(n == 2);
  ASSERT_TRUE(std::strncmp(dest, "bc", 2) == 0);
  n = str.copy(dest, 2);
  ASSERT_TRUE(n == 2);
  ASSERT_TRUE(std::strncmp(dest, "ab", 2) == 0);
  return true;
}

static bool testMethod_resize()
{
  std::cout << "testMethod_resize()\n";
  cm::String str = "abc";
  str.resize(3);
  ASSERT_TRUE(str == "abc");
  str.resize(2);
  ASSERT_TRUE(str == "ab");
  str.resize(3, 'c');
  ASSERT_TRUE(str == "abc");
  return true;
}

static bool testMethod_swap()
{
  std::cout << "testMethod_swap()\n";
  cm::String str1 = std::string("1");
  cm::String str2 = std::string("2");
  str1.swap(str2);
  ASSERT_TRUE(str1 == "2");
  ASSERT_TRUE(str2 == "1");
  return true;
}

static bool testMethod_substr_AtEnd(cm::String str)
{
  cm::String substr = str.substr(1);
  ASSERT_TRUE(substr.data() == str.data() + 1);
  ASSERT_TRUE(substr.size() == 2);
  ASSERT_TRUE(!substr.is_stable());

  // c_str() at the end of the buffer does not internally mutate.
  ASSERT_TRUE(std::strcmp(substr.c_str(), "bc") == 0);
  ASSERT_TRUE(substr.c_str() == str.data() + 1);
  ASSERT_TRUE(substr.data() == str.data() + 1);
  ASSERT_TRUE(substr.size() == 2);
  ASSERT_TRUE(!substr.is_stable());

  // str() internally mutates.
  ASSERT_TRUE(substr.str() == "bc");
  ASSERT_TRUE(substr.is_stable());
  ASSERT_TRUE(substr.data() != str.data() + 1);
  ASSERT_TRUE(substr.size() == 2);
  ASSERT_TRUE(substr.c_str() != str.data() + 1);
  ASSERT_TRUE(std::strcmp(substr.c_str(), "bc") == 0);
  return true;
}

static bool testMethod_substr_AtEndBorrowed()
{
  std::cout << "testMethod_substr_AtEndBorrowed()\n";
  return testMethod_substr_AtEnd(cm::String("abc"_s));
}

static bool testMethod_substr_AtEndOwned()
{
  std::cout << "testMethod_substr_AtEndOwned()\n";
  return testMethod_substr_AtEnd(std::string("abc"));
}

static bool testMethod_substr_AtStart(cm::String str)
{
  {
    cm::String substr = str.substr(0, 2);
    ASSERT_TRUE(substr.data() == str.data());
    ASSERT_TRUE(substr.size() == 2);

    // c_str() not at the end of the buffer internally mutates.
    const char* substr_c = substr.c_str();
    ASSERT_TRUE(std::strcmp(substr_c, "ab") == 0);
    ASSERT_TRUE(substr_c != str.data());
    ASSERT_TRUE(substr.data() != str.data());
    ASSERT_TRUE(substr.size() == 2);
    ASSERT_TRUE(substr.is_stable());

    // str() does not need to internally mutate after c_str() did so
    ASSERT_TRUE(substr.str() == "ab");
    ASSERT_TRUE(substr.is_stable());
    ASSERT_TRUE(substr.data() == substr_c);
    ASSERT_TRUE(substr.size() == 2);
    ASSERT_TRUE(substr.c_str() == substr_c);
  }

  {
    cm::String substr = str.substr(0, 2);
    ASSERT_TRUE(substr.data() == str.data());
    ASSERT_TRUE(substr.size() == 2);
    ASSERT_TRUE(!substr.is_stable());

    // str() internally mutates.
    ASSERT_TRUE(substr.str() == "ab");
    ASSERT_TRUE(substr.is_stable());
    ASSERT_TRUE(substr.data() != str.data());
    ASSERT_TRUE(substr.size() == 2);
    ASSERT_TRUE(substr.c_str() != str.data());

    // c_str() does not internally after str() did so
    const char* substr_c = substr.c_str();
    ASSERT_TRUE(std::strcmp(substr_c, "ab") == 0);
    ASSERT_TRUE(substr_c == substr.data());
    ASSERT_TRUE(substr.size() == 2);
    ASSERT_TRUE(substr.is_stable());
  }

  return true;
}

static bool testMethod_substr_AtStartBorrowed()
{
  std::cout << "testMethod_substr_AtStartBorrowed()\n";
  return testMethod_substr_AtStart(cm::String("abc"_s));
}

static bool testMethod_substr_AtStartOwned()
{
  std::cout << "testMethod_substr_AtStartOwned()\n";
  return testMethod_substr_AtStart(std::string("abc"));
}

static bool testMethod_compare()
{
  std::cout << "testMethod_compare()\n";
  cm::String str = "b";
  ASSERT_TRUE(str.compare("a") > 0);
  ASSERT_TRUE(str.compare("b") == 0);
  ASSERT_TRUE(str.compare("c") < 0);
  {
    const char* a = "a";
    const char* b = "b";
    const char* c = "c";
    ASSERT_TRUE(str.compare(a) > 0);
    ASSERT_TRUE(str.compare(b) == 0);
    ASSERT_TRUE(str.compare(c) < 0);
  }
  ASSERT_TRUE(str.compare('a') > 0);
  ASSERT_TRUE(str.compare('b') == 0);
  ASSERT_TRUE(str.compare('c') < 0);
  ASSERT_TRUE(str.compare(std::string("a")) > 0);
  ASSERT_TRUE(str.compare(std::string("b")) == 0);
  ASSERT_TRUE(str.compare(std::string("c")) < 0);
  ASSERT_TRUE(str.compare(cm::string_view("a_", 1)) > 0);
  ASSERT_TRUE(str.compare(cm::string_view("b_", 1)) == 0);
  ASSERT_TRUE(str.compare(cm::string_view("c_", 1)) < 0);
  ASSERT_TRUE(str.compare(cm::String("a")) > 0);
  ASSERT_TRUE(str.compare(cm::String("b")) == 0);
  ASSERT_TRUE(str.compare(cm::String("c")) < 0);
  ASSERT_TRUE(str.compare(0, 1, cm::string_view("a", 1)) > 0);
  ASSERT_TRUE(str.compare(1, 0, cm::string_view("", 0)) == 0);
  ASSERT_TRUE(str.compare(0, 1, cm::string_view("ac", 2), 1, 1) < 0);
  ASSERT_TRUE(str.compare(1, 0, "") == 0);
  ASSERT_TRUE(str.compare(1, 0, "_", 0) == 0);
  return true;
}

static bool testMethod_find()
{
  std::cout << "testMethod_find()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.find("a") == 0);
  ASSERT_TRUE(str.find("a", 1) == 3);
  {
    const char* a = "a";
    ASSERT_TRUE(str.find(a) == 0);
    ASSERT_TRUE(str.find(a, 1) == 3);
  }
  ASSERT_TRUE(str.find('a') == 0);
  ASSERT_TRUE(str.find('a', 1) == 3);
  ASSERT_TRUE(str.find(std::string("a")) == 0);
  ASSERT_TRUE(str.find(std::string("a"), 1) == 3);
  ASSERT_TRUE(str.find(cm::string_view("a_", 1)) == 0);
  ASSERT_TRUE(str.find(cm::string_view("a_", 1), 1) == 3);
  ASSERT_TRUE(str.find(cm::String("a")) == 0);
  ASSERT_TRUE(str.find(cm::String("a"), 1) == 3);
  ASSERT_TRUE(str.find("ab_", 1, 2) == 3);
  return true;
}

static bool testMethod_rfind()
{
  std::cout << "testMethod_rfind()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.rfind("a") == 3);
  ASSERT_TRUE(str.rfind("a", 1) == 0);
  {
    const char* a = "a";
    ASSERT_TRUE(str.rfind(a) == 3);
    ASSERT_TRUE(str.rfind(a, 1) == 0);
  }
  ASSERT_TRUE(str.rfind('a') == 3);
  ASSERT_TRUE(str.rfind('a', 1) == 0);
  ASSERT_TRUE(str.rfind(std::string("a")) == 3);
  ASSERT_TRUE(str.rfind(std::string("a"), 1) == 0);
  ASSERT_TRUE(str.rfind(cm::string_view("a_", 1)) == 3);
  ASSERT_TRUE(str.rfind(cm::string_view("a_", 1), 1) == 0);
  ASSERT_TRUE(str.rfind(cm::String("a")) == 3);
  ASSERT_TRUE(str.rfind(cm::String("a"), 1) == 0);
  ASSERT_TRUE(str.rfind("ab_", 1, 2) == 0);
  return true;
}

static bool testMethod_find_first_of()
{
  std::cout << "testMethod_find_first_of()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.find_first_of("_a") == 0);
  ASSERT_TRUE(str.find_first_of("_a", 1) == 3);
  {
    const char* a = "_a";
    ASSERT_TRUE(str.find_first_of(a) == 0);
    ASSERT_TRUE(str.find_first_of(a, 1) == 3);
  }
  ASSERT_TRUE(str.find_first_of('a') == 0);
  ASSERT_TRUE(str.find_first_of('a', 1) == 3);
  ASSERT_TRUE(str.find_first_of(std::string("_a")) == 0);
  ASSERT_TRUE(str.find_first_of(std::string("_a"), 1) == 3);
  ASSERT_TRUE(str.find_first_of(cm::string_view("ba_", 1)) == 1);
  ASSERT_TRUE(str.find_first_of(cm::string_view("ba_", 1), 2) == 4);
  ASSERT_TRUE(str.find_first_of(cm::String("ab")) == 0);
  ASSERT_TRUE(str.find_first_of(cm::String("ab"), 2) == 3);
  ASSERT_TRUE(str.find_first_of("_ab", 1, 2) == 3);
  return true;
}

static bool testMethod_find_first_not_of()
{
  std::cout << "testMethod_find_first_not_of()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.find_first_not_of("_a") == 1);
  ASSERT_TRUE(str.find_first_not_of("_a", 2) == 2);
  {
    const char* a = "_a";
    ASSERT_TRUE(str.find_first_not_of(a) == 1);
    ASSERT_TRUE(str.find_first_not_of(a, 2) == 2);
  }
  ASSERT_TRUE(str.find_first_not_of('a') == 1);
  ASSERT_TRUE(str.find_first_not_of('a', 2) == 2);
  ASSERT_TRUE(str.find_first_not_of(std::string("_a")) == 1);
  ASSERT_TRUE(str.find_first_not_of(std::string("_a"), 2) == 2);
  ASSERT_TRUE(str.find_first_not_of(cm::string_view("ba_", 1)) == 0);
  ASSERT_TRUE(str.find_first_not_of(cm::string_view("ba_", 1), 1) == 2);
  ASSERT_TRUE(str.find_first_not_of(cm::String("_a")) == 1);
  ASSERT_TRUE(str.find_first_not_of(cm::String("_a"), 2) == 2);
  ASSERT_TRUE(str.find_first_not_of("_bca", 1, 3) == 3);
  return true;
}

static bool testMethod_find_last_of()
{
  std::cout << "testMethod_find_last_of()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.find_last_of("_a") == 3);
  ASSERT_TRUE(str.find_last_of("_a", 1) == 0);
  {
    const char* a = "_a";
    ASSERT_TRUE(str.find_last_of(a) == 3);
    ASSERT_TRUE(str.find_last_of(a, 1) == 0);
  }
  ASSERT_TRUE(str.find_last_of('a') == 3);
  ASSERT_TRUE(str.find_last_of('a', 1) == 0);
  ASSERT_TRUE(str.find_last_of(std::string("_a")) == 3);
  ASSERT_TRUE(str.find_last_of(std::string("_a"), 1) == 0);
  ASSERT_TRUE(str.find_last_of(cm::string_view("ba_", 1)) == 4);
  ASSERT_TRUE(str.find_last_of(cm::string_view("ba_", 1), 2) == 1);
  ASSERT_TRUE(str.find_last_of(cm::String("ab")) == 4);
  ASSERT_TRUE(str.find_last_of(cm::String("ab"), 2) == 1);
  ASSERT_TRUE(str.find_last_of("_ab", 1, 2) == 0);
  return true;
}

static bool testMethod_find_last_not_of()
{
  std::cout << "testMethod_find_last_not_of()\n";
  cm::String str = "abcabc";
  ASSERT_TRUE(str.find_last_not_of("_a") == 5);
  ASSERT_TRUE(str.find_last_not_of("_a", 1) == 1);
  {
    const char* a = "_a";
    ASSERT_TRUE(str.find_last_not_of(a) == 5);
    ASSERT_TRUE(str.find_last_not_of(a, 1) == 1);
  }
  ASSERT_TRUE(str.find_last_not_of('a') == 5);
  ASSERT_TRUE(str.find_last_not_of('a', 1) == 1);
  ASSERT_TRUE(str.find_last_not_of(std::string("_a")) == 5);
  ASSERT_TRUE(str.find_last_not_of(std::string("_a"), 1) == 1);
  ASSERT_TRUE(str.find_last_not_of(cm::string_view("cb_", 1)) == 4);
  ASSERT_TRUE(str.find_last_not_of(cm::string_view("cb_", 1), 2) == 1);
  ASSERT_TRUE(str.find_last_not_of(cm::String("_a")) == 5);
  ASSERT_TRUE(str.find_last_not_of(cm::String("_a"), 1) == 1);
  ASSERT_TRUE(str.find_last_not_of("cb_", 2, 2) == 0);
  return true;
}

static bool testAddition()
{
  std::cout << "testAddition()\n";
  {
    ASSERT_TRUE(cm::String("a") + "b" == "ab");
    ASSERT_TRUE("ab" == "a" + cm::String("b"));
    ASSERT_TRUE("a" + cm::String("b") + "c" == "abc");
    ASSERT_TRUE("abc" == "a" + cm::String("b") + "c");
    ASSERT_TRUE("a" + (cm::String("b") + "c") + "d" == "abcd");
    ASSERT_TRUE("abcd" == "a" + (cm::String("b") + "c") + "d");
  }
  {
    ASSERT_TRUE(cm::String("a"_s) + "b"_s == "ab"_s);
    ASSERT_TRUE("ab"_s == "a"_s + cm::String("b"_s));
    ASSERT_TRUE("a"_s + cm::String("b"_s) + "c"_s == "abc"_s);
    ASSERT_TRUE("abc"_s == "a"_s + cm::String("b"_s) + "c"_s);
    ASSERT_TRUE("a"_s + (cm::String("b"_s) + "c"_s) + "d"_s == "abcd"_s);
    ASSERT_TRUE("abcd"_s == "a"_s + (cm::String("b"_s) + "c"_s) + "d"_s);
  }
  {
    const char* a = "a";
    const char* b = "b";
    const char* ab = "ab";
    ASSERT_TRUE(cm::String(a) + b == ab);
    ASSERT_TRUE(ab == a + cm::String(b));
    const char* c = "c";
    const char* abc = "abc";
    ASSERT_TRUE(a + cm::String(b) + c == abc);
    ASSERT_TRUE(abc == a + cm::String(b) + c);
    const char* d = "d";
    const char* abcd = "abcd";
    ASSERT_TRUE(a + (cm::String(b) + c) + d == abcd);
    ASSERT_TRUE(abcd == a + (cm::String(b) + c) + d);
  }
  {
    ASSERT_TRUE(cm::String('a') + 'b' == "ab");
    ASSERT_TRUE("ab" == 'a' + cm::String('b'));
    ASSERT_TRUE('a' + cm::String('b') + 'c' == "abc");
    ASSERT_TRUE("abc" == 'a' + cm::String('b') + 'c');
    ASSERT_TRUE('a' + (cm::String('b') + 'c') + 'd' == "abcd");
    ASSERT_TRUE("abcd" == 'a' + (cm::String('b') + 'c') + 'd');
  }
  {
    std::string a = "a";
    std::string b = "b";
    std::string ab = "ab";
    ASSERT_TRUE(cm::String(a) + b == ab);
    ASSERT_TRUE(ab == a + cm::String(b));
    std::string c = "c";
    std::string abc = "abc";
    ASSERT_TRUE(a + cm::String(b) + c == abc);
    ASSERT_TRUE(abc == a + cm::String(b) + c);
    std::string d = "d";
    std::string abcd = "abcd";
    ASSERT_TRUE(a + (cm::String(b) + c) + d == abcd);
    ASSERT_TRUE(abcd == a + (cm::String(b) + c) + d);
  }
  {
    cm::string_view a("a", 1);
    cm::string_view b("b", 1);
    cm::string_view ab("ab", 2);
    ASSERT_TRUE(cm::String(a) + b == ab);
    ASSERT_TRUE(ab == a + cm::String(b));
    cm::string_view c("c", 1);
    cm::string_view abc("abc", 3);
    ASSERT_TRUE(a + cm::String(b) + c == abc);
    ASSERT_TRUE(abc == a + cm::String(b) + c);
    cm::string_view d("d", 1);
    cm::string_view abcd("abcd", 4);
    ASSERT_TRUE(a + (cm::String(b) + c) + d == abcd);
    ASSERT_TRUE(abcd == a + (cm::String(b) + c) + d);
  }
  {
    cm::String a = "a";
    cm::String b = "b";
    cm::String ab = "ab";
    ASSERT_TRUE(a + b == ab);
    ASSERT_TRUE(ab == a + b);
    cm::String c = "c";
    cm::String abc = "abc";
    ASSERT_TRUE(a + cm::String(b) + c == abc);
    ASSERT_TRUE(abc == a + cm::String(b) + c);
    cm::String d = "d";
    cm::String abcd = "abcd";
    ASSERT_TRUE(a + (cm::String(b) + c) + d == abcd);
    ASSERT_TRUE(abcd == a + (cm::String(b) + c) + d);
  }
  {
    cm::String str;
    str += "a" + cm::String("b") + 'c';
    ASSERT_TRUE(str == "abc");
    ASSERT_TRUE(str.is_stable());
  }
  {
    std::string s;
    s += "a" + cm::String("b") + 'c';
    ASSERT_TRUE(s == "abc");
  }
  {
    std::ostringstream ss;
    ss << ("a" + cm::String("b") + 'c');
    ASSERT_TRUE(ss.str() == "abc");
  }
  return true;
}

static bool testStability()
{
  std::cout << "testStability()\n";
  cm::String str("abc"_s);
  ASSERT_TRUE(!str.is_stable());
  ASSERT_TRUE(str.str_if_stable() == nullptr);
  str.stabilize();
  ASSERT_TRUE(str.is_stable());
  std::string const* str_if_stable = str.str_if_stable();
  ASSERT_TRUE(str_if_stable != nullptr);
  ASSERT_TRUE(*str_if_stable == "abc");
  str.stabilize();
  ASSERT_TRUE(str.is_stable());
  ASSERT_TRUE(str.str_if_stable() == str_if_stable);
  return true;
}

int testString(int /*unused*/, char* /*unused*/[])
{
  if (!testConstructDefault()) {
    return 1;
  }
  if (!testConstructFromNullPtr()) {
    return 1;
  }
  if (!testConstructFromCStrNull()) {
    return 1;
  }
  if (!testConstructFromCharArray()) {
    return 1;
  }
  if (!testConstructFromCStr()) {
    return 1;
  }
  if (!testConstructFromStdString()) {
    return 1;
  }
  if (!testConstructFromView()) {
    return 1;
  }
  if (!testConstructFromChar()) {
    return 1;
  }
  if (!testConstructFromInitList()) {
    return 1;
  }
  if (!testConstructFromBuffer()) {
    return 1;
  }
  if (!testConstructFromInputIterator()) {
    return 1;
  }
  if (!testConstructFromN()) {
    return 1;
  }
  if (!testConstructFromStaticStringView()) {
    return 1;
  }
  if (!testConstructCopy()) {
    return 1;
  }
  if (!testConstructMove()) {
    return 1;
  }
  if (!testAssignCopy()) {
    return 1;
  }
  if (!testAssignMove()) {
    return 1;
  }
  if (!testAssignFromChar()) {
    return 1;
  }
  if (!testAssignFromView()) {
    return 1;
  }
  if (!testAssignFromStdString()) {
    return 1;
  }
  if (!testAssignFromCStr()) {
    return 1;
  }
  if (!testAssignFromCharArray()) {
    return 1;
  }
  if (!testAssignFromCStrNull()) {
    return 1;
  }
  if (!testAssignFromNullPtr()) {
    return 1;
  }
  if (!testAssignFromInitList()) {
    return 1;
  }
  if (!testAssignFromStaticStringView()) {
    return 1;
  }
  if (!testOperatorBool()) {
    return 1;
  }
  if (!testOperatorIndex()) {
    return 1;
  }
  if (!testOperatorPlusEqual()) {
    return 1;
  }
  if (!testOperatorCompare()) {
    return 1;
  }
  if (!testOperatorStream()) {
    return 1;
  }
  if (!testOperatorStdStringPlusEqual()) {
    return 1;
  }
  if (!testMethod_borrow()) {
    return 1;
  }
  if (!testMethod_view()) {
    return 1;
  }
  if (!testMethod_empty()) {
    return 1;
  }
  if (!testMethod_length()) {
    return 1;
  }
  if (!testMethod_at()) {
    return 1;
  }
  if (!testMethod_front_back()) {
    return 1;
  }
  if (!testMethod_clear()) {
    return 1;
  }
  if (!testMethod_insert()) {
    return 1;
  }
  if (!testMethod_erase()) {
    return 1;
  }
  if (!testMethod_push_back()) {
    return 1;
  }
  if (!testMethod_pop_back()) {
    return 1;
  }
  if (!testMethod_replace()) {
    return 1;
  }
  if (!testMethod_copy()) {
    return 1;
  }
  if (!testMethod_resize()) {
    return 1;
  }
  if (!testMethod_swap()) {
    return 1;
  }
  if (!testMethodIterators()) {
    return 1;
  }
  if (!testMethod_substr_AtEndBorrowed()) {
    return 1;
  }
  if (!testMethod_substr_AtEndOwned()) {
    return 1;
  }
  if (!testMethod_substr_AtStartBorrowed()) {
    return 1;
  }
  if (!testMethod_substr_AtStartOwned()) {
    return 1;
  }
  if (!testMethod_compare()) {
    return 1;
  }
  if (!testMethod_find()) {
    return 1;
  }
  if (!testMethod_rfind()) {
    return 1;
  }
  if (!testMethod_find_first_of()) {
    return 1;
  }
  if (!testMethod_find_first_not_of()) {
    return 1;
  }
  if (!testMethod_find_last_of()) {
    return 1;
  }
  if (!testMethod_find_last_not_of()) {
    return 1;
  }
  if (!testAddition()) {
    return 1;
  }
  if (!testStability()) {
    return 1;
  }
  return 0;
}
