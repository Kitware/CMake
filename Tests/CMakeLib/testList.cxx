/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <cmext/string_view>

#include "cmList.h"

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
    cmList list;
    if (!list.empty() || list != cmList{}) {
      result = false;
    }
  }
  {
    cmList list{ "aa;bb" };
    if (list.size() != 2 || list.to_string() != "aa;bb") {
      result = false;
    }
  }
  {
    cmList list1{ "aa", "bb" };
    cmList list2("aa;bb"_s);

    if (list1.size() != 2 || list2.size() != 2 || list1 != list2) {
      result = false;
    }
    if (list1.to_string() != "aa;bb") {
      result = false;
    }
    if (list1.to_string() != list2.to_string()) {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "aa", "bb", "cc" };
    cmList list(v.begin(), v.end());
    if (list.size() != 3 || list.to_string() != "aa;bb;cc") {
      result = false;
    }
  }
  {
    std::vector<std::string> values{ "aa;bb", "cc", "dd;ee" };
    cmList list1(values.begin(), values.end());
    cmList list2(values.begin(), values.end(), cmList::ExpandElements::No);

    if (list1.size() != 5 || list1.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
    if (list2.size() != 3 || list2.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
  }
  {
    std::vector<std::string> values{ "aa;bb;;cc", "", "dd;ee" };
    cmList list1(values.begin(), values.end(), cmList::ExpandElements::No,
                 cmList::EmptyElements::No);
    cmList list2(values.begin(), values.end(), cmList::ExpandElements::No,
                 cmList::EmptyElements::Yes);
    cmList list3(values.begin(), values.end(), cmList::ExpandElements::Yes,
                 cmList::EmptyElements::No);
    cmList list4(values.begin(), values.end(), cmList::ExpandElements::Yes,
                 cmList::EmptyElements::Yes);

    if (list1.size() != 2 || list1.to_string() != "aa;bb;;cc;dd;ee") {
      result = false;
    }
    if (list2.size() != 3 || list2.to_string() != "aa;bb;;cc;;dd;ee") {
      result = false;
    }
    if (list3.size() != 5 || list3.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
    if (list4.size() != 7 || list4.to_string() != "aa;bb;;cc;;dd;ee") {
      result = false;
    }
  }
  {
    std::vector<std::string> values{ "aa;bb", "cc", "dd;ee" };
    cmList list1(values);
    cmList list2(values, cmList::ExpandElements::No);

    if (list1.size() != 5 || list1.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
    if (list2.size() != 3 || list2.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
  }
  {
    std::vector<std::string> values{ "aa", "bb", "cc", "dd", "ee" };
    cmList list(std::move(values));

    if (list.size() != 5 || list.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
    if (!values.empty()) {
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
    cmList list1{ "aa", "bb" };
    cmList list2{ "cc", "dd" };

    list2 = list1;
    if (list1.size() != 2 || list2.size() != 2 || list1 != list2) {
      result = false;
    }
    if (list1.to_string() != "aa;bb") {
      result = false;
    }
    if (list1.to_string() != list2.to_string()) {
      result = false;
    }
  }
  {
    cmList list1{ "aa", "bb" };
    cmList list2{ "cc", "dd" };

    list2 = std::move(list1);
    if (!list1.empty() || list2.size() != 2) {
      result = false;
    }
    if (list2.to_string() != "aa;bb") {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "aa", "bb" };
    cmList list{ "cc", "dd" };

    list = std::move(v);
    if (!v.empty() || list.size() != 2) {
      result = false;
    }
    if (list.to_string() != "aa;bb") {
      result = false;
    }
  }
  {
    cmList list{ "cc", "dd" };

    list = "aa;bb"_s;
    if (list.size() != 2) {
      result = false;
    }
    if (list.to_string() != "aa;bb") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testConversions()
{
  std::cout << "testConversions()";

  bool result = true;

  {
    cmList list("a;b;c"_s);
    std::string s = list.to_string();

    if (s != "a;b;c") {
      result = false;
    }
  }
  {
    cmList list("a;b;c"_s);
    std::vector<std::string> v = list;

    if (list.size() != 3 || v.size() != 3) {
      result = false;
    }
  }
  {
    cmList list("a;b;c"_s);
    std::vector<std::string> v = std::move(list);

    // Microsoft compiler is not able to handle correctly the move semantics
    // so the initial list is not moved, so do not check its size...
    if (v.size() != 3) {
      result = false;
    }
  }
  {
    cmList list("a;b;c"_s);
    std::vector<std::string> v;

    // compiler is not able to select the cmList conversion operator
    // and the std::vector assignment operator using the move semantics
    // v = std::move(list);
    v = std::move(list.data());

    if (!list.empty() || v.size() != 3) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testAccess()
{
  std::cout << "testAccess()";

  bool result = true;

  {
    cmList list{ "a", "b", "c" };
    if (list.get_item(1) != "b") {
      result = false;
    }
  }
  {
    cmList list{ "a", "b", "c" };
    if (list.get_item(-3) != "a") {
      result = false;
    }
  }
  {
    try {
      cmList list{ "a", "b", "c" };
      if (list.get_item(4) != "a") {
        result = false;
      }
    } catch (std::out_of_range&) {
    }
  }
  {
    try {
      cmList list{ "a", "b", "c" };
      if (list.get_item(-4) != "a") {
        result = false;
      }
    } catch (std::out_of_range&) {
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e" };
    auto sublist = list.sublist(list.begin() + 1, list.begin() + 3);
    if (sublist.size() != 2 || sublist != cmList{ "b", "c" }) {
      result = false;
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e" };
    auto sublist = list.sublist(1, 2);
    if (sublist.size() != 2 || sublist != cmList{ "b", "c" }) {
      result = false;
    }

    sublist = list.sublist(1, cmList::npos);
    if (sublist.size() != 4 || sublist != cmList{ "b", "c", "d", "e" }) {
      result = false;
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e", "f" };
    auto sublist = list.get_items({ 1, 3, 5 });
    if (sublist.size() != 3 || sublist != cmList{ "b", "d", "f" }) {
      result = false;
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e", "f" };
    auto sublist = list.get_items({ 1, -3, 5, -3 });
    if (sublist.size() != 4 || sublist != cmList{ "b", "d", "f", "d" }) {
      result = false;
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e", "f" };
    try {
      if (list.get_items({ 1, -3, 5, -3, 10 }).size() != 5) {
        result = false;
      }
    } catch (std::out_of_range&) {
    }
  }
  {
    cmList list{ "a", "b", "c", "d", "e", "f" };

    if (list.find("b") != 1) {
      result = false;
    }
    if (list.find("x") != cmList::npos) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testModifiers()
{
  std::cout << "testModifiers()";

  bool result = true;

  {
    cmList list{ "1;2;3;4;5" };

    auto it = list.insert(list.begin() + 2, "6;7;8"_s);
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };

    auto it =
      list.insert(list.begin() + 2, "6;7;8"_s, cmList::ExpandElements::No);
    if (list.size() != 6 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6;7;8") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    cmList v{ "6", "7", "8" };

    auto it = list.insert(list.begin() + 2, v);
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    cmList v{ "6", "7", "8" };

    auto it = list.insert(list.begin() + 2, std::move(v));
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }

    if (!v.empty()) {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6", "7", "8" };

    auto it = list.insert(list.begin() + 2, v);
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6;7", "8" };

    auto it = list.insert(list.begin() + 2, v);
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6;7", "8" };

    auto it = list.insert(list.begin() + 2, v, cmList::ExpandElements::No);
    if (list.size() != 7 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6;7") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6;;7", "8" };

    auto it = list.insert(list.begin() + 2, v);
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6;;7", "8" };

    auto it = list.insert(list.begin() + 2, v, cmList::EmptyElements::Yes);
    if (list.size() != 9 || list.to_string() != "1;2;6;;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }
  }
  {
    cmList list{ "1;2;3;4;5" };
    std::vector<std::string> v{ "6", "7", "8" };

    auto it = list.insert(list.begin() + 2, std::move(v));
    if (list.size() != 8 || list.to_string() != "1;2;6;7;8;3;4;5") {
      result = false;
    }
    if (*it != "6") {
      result = false;
    }

    if (!v.empty()) {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testRemoveItems()
{
  std::cout << "testRemoveItems()";

  bool result = true;

  {
    cmList list("a;b;c;d;e;f;g;h"_s);

    list.remove_items({ 1, 3, 5 });

    if (list.size() != 5 || list.to_string() != "a;c;e;g;h") {
      result = false;
    }
  }
  {
    cmList list("a;b;c;b;a;d;e;f"_s);

    list.remove_items({ "a", "b", "h" });

    if (list.size() != 4 || list.to_string() != "c;d;e;f") {
      result = false;
    }
  }
  {
    cmList list("a;b;c;d;e;f;g;h"_s);
    std::vector<cmList::index_type> remove{ 1, 3, 5 };

    list.remove_items(remove.begin(), remove.end());

    if (list.size() != 5 || list.to_string() != "a;c;e;g;h") {
      result = false;
    }
  }
  {
    cmList list("a;b;c;b;a;d;e;f"_s);
    std::vector<std::string> remove{ "b", "a", "h" };

    list.remove_items(remove.begin(), remove.end());

    if (list.size() != 4 || list.to_string() != "c;d;e;f") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testRemoveDuplicates()
{
  std::cout << "testRemoveDuplicates()";

  bool result = true;

  {
    cmList list("b;c;b;a;a;c;b;a;c;b"_s);

    list.remove_duplicates();

    if (list.size() != 3 || list.to_string() != "b;c;a") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testFilter()
{
  std::cout << "testFilter()";

  bool result = true;

  {
    cmList list{ "AA", "Aa", "aA" };

    list.filter("^A", cmList::FilterMode::INCLUDE);
    if (list.size() != 2 || list.to_string() != "AA;Aa") {
      result = false;
    }
  }
  {
    cmList list{ "AA", "Aa", "aA" };

    list.filter("^A", cmList::FilterMode::EXCLUDE);
    if (list.size() != 1 || list.to_string() != "aA") {
      result = false;
    }
  }
  {
    cmList list{ "AA", "Aa", "aA" };

    try {
      list.filter("^(A", cmList::FilterMode::EXCLUDE);
      if (list.size() != 1) {
        result = false;
      }
    } catch (const std::invalid_argument&) {
    }
  }

  checkResult(result);

  return result;
}

bool testReverse()
{
  std::cout << "testReverse()";

  bool result = true;

  {
    cmList list{ "a", "b", "c" };
    if (list.reverse().to_string() != "c;b;a") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testSort()
{
  std::cout << "testSort()";

  bool result = true;

  using SortConfiguration = cmList::SortConfiguration;

  {
    cmList list{ "A", "D", "C", "B", "A" };

    list.sort();
    if (list.to_string() != "A;A;B;C;D") {
      result = false;
    }

    list.sort({ SortConfiguration::OrderMode::DESCENDING,
                SortConfiguration::CompareMethod::DEFAULT,
                SortConfiguration::CaseSensitivity::DEFAULT });
    if (list.to_string() != "D;C;B;A;A") {
      result = false;
    }
  }
  {
    SortConfiguration sortCfg;
    cmList list{ "1.0", "1.1", "2.5", "10.2" };

    list.sort(sortCfg);
    if (list.to_string() != "1.0;1.1;10.2;2.5") {
      result = false;
    }

    sortCfg.Compare = SortConfiguration::CompareMethod::NATURAL;
    list.sort(sortCfg);
    if (list.to_string() != "1.0;1.1;2.5;10.2") {
      result = false;
    }

    sortCfg.Order = SortConfiguration::OrderMode::DESCENDING;
    list.sort(sortCfg);
    if (list.to_string() != "10.2;2.5;1.1;1.0") {
      result = false;
    }
  }
  {
    SortConfiguration sortCfg;
    cmList list{ "/zz/bb.cc", "/xx/yy/dd.cc", "/aa/cc.aa" };

    list.sort(sortCfg);
    if (list.to_string() != "/aa/cc.aa;/xx/yy/dd.cc;/zz/bb.cc") {
      result = false;
    }

    sortCfg.Compare = SortConfiguration::CompareMethod::FILE_BASENAME;
    if (list.sort(sortCfg).to_string() != "/zz/bb.cc;/aa/cc.aa;/xx/yy/dd.cc") {
      result = false;
    }
  }
  {
    SortConfiguration sortCfg;
    cmList list{ "c/B", "a/c", "B/a" };

    if (list.sort().to_string() != "B/a;a/c;c/B") {
      result = false;
    }

    sortCfg.Case = SortConfiguration::CaseSensitivity::INSENSITIVE;
    if (list.sort(sortCfg).to_string() != "a/c;B/a;c/B") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testTransform()
{
  std::cout << "testTransform()";

  bool result = true;

  using AT = cmList::TransformSelector::AT;
  using FOR = cmList::TransformSelector::FOR;
  using REGEX = cmList::TransformSelector::REGEX;

  {
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::APPEND, "-X");
    if (list.to_string() != "AA-X;BB-X;CC-X;DD-X;EE-X") {
      result = false;
    }
  }
  {
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::PREPEND, "X-");
    if (list.to_string() != "X-AA;X-BB;X-CC;X-DD;X-EE") {
      result = false;
    }
  }
  {
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER);
    if (list.to_string() != "aa;bb;cc;dd;ee") {
      result = false;
    }
  }
  {
    cmList list({ "aa", "bb", "cc", "dd", "ee" });

    list.transform(cmList::TransformAction::TOUPPER);
    if (list.to_string() != "AA;BB;CC;DD;EE") {
      result = false;
    }
  }
  {
    cmList list({ "  AA", "BB  ", "  CC  ", "DD", "EE" });

    list.transform(cmList::TransformAction::STRIP);
    if (list.to_string() != "AA;BB;CC;DD;EE") {
      result = false;
    }
  }
  {
    cmList list({ "$<CONFIG>AA", "BB$<OR>", "C$<AND>C", "$<OR>DD$<AND>",
                  "$<>E$<>E$<>" });

    list.transform(cmList::TransformAction::GENEX_STRIP);
    if (list.to_string() != "AA;BB;CC;DD;EE") {
      result = false;
    }
  }
  {
    cmList list({ "ABC", "BBCB", "BCCCBC", "BCBCDD", "EBCBCEBC" });

    list.transform(cmList::TransformAction::REPLACE, "^BC|BC$", "X");
    if (list.to_string() != "AX;BBCB;XCCX;XXDD;EBCBCEX") {
      result = false;
    }
  }
  {
    auto atSelector = cmList::TransformSelector::New<AT>({ 1, 2, 4 });
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER, std::move(atSelector));
    if (list.to_string() != "AA;bb;cc;DD;ee") {
      result = false;
    }
  }
  {
    auto atSelector = cmList::TransformSelector::New<AT>({ 1, 2, -1 });
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER, std::move(atSelector));
    if (list.to_string() != "AA;bb;cc;DD;ee") {
      result = false;
    }
  }
  {
    auto forSelector = cmList::TransformSelector::New<FOR>({ 1, 3 });
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER, std::move(forSelector));
    if (list.to_string() != "AA;bb;cc;dd;EE") {
      result = false;
    }
  }
  {
    auto forSelector = cmList::TransformSelector::New<FOR>({ 0, 4, 2 });
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER, std::move(forSelector));
    if (list.to_string() != "aa;BB;cc;DD;ee") {
      result = false;
    }
  }
  {
    auto regexSelector = cmList::TransformSelector::New<REGEX>("^(A|D|E)");
    cmList list({ "AA", "BB", "CC", "DD", "EE" });

    list.transform(cmList::TransformAction::TOLOWER, std::move(regexSelector));
    if (list.to_string() != "aa;BB;CC;dd;ee") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}

bool testStaticModifiers()
{
  std::cout << "testStaticModifiers()";

  bool result = true;

  {
    std::vector<std::string> v{ "a", "b", "c" };
    cmList::assign(v, "d;e"_s);

    if (v.size() != 2 || v[0] != "d" || v[1] != "e") {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "a", "b", "c" };
    cmList::append(v, "d;;e"_s);

    if (v.size() != 5 || v[3] != "d" || v[4] != "e") {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "a", "b", "c" };
    cmList::append(v, "d;;e"_s, cmList::EmptyElements::Yes);

    if (v.size() != 6 || v[3] != "d" || !v[4].empty() || v[5] != "e") {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "a", "b", "c" };
    cmList::prepend(v, "d;e"_s);

    if (v.size() != 5 || v[0] != "d" || v[1] != "e") {
      result = false;
    }
  }
  {
    std::vector<std::string> v{ "a", "b", "c" };
    cmList::prepend(v, "d;;e"_s, cmList::EmptyElements::Yes);

    if (v.size() != 6 || v[0] != "d" || !v[1].empty() || v[2] != "e") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    cmList::append(list, "d;e"_s);

    if (list != "a;b;c;d;e") {
      result = false;
    }
  }
  {
    std::string list;
    cmList::append(list, "d;e"_s);

    if (list != "d;e") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    cmList::append(list, ""_s);

    if (list != "a;b;c;") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    std::vector<std::string> v{ "d", "e" };
    cmList::append(list, v.begin(), v.end());

    if (list != "a;b;c;d;e") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    std::vector<std::string> v;
    cmList::append(list, v.begin(), v.end());

    if (list != "a;b;c") {
      result = false;
    }
  }
  {
    std::string list;
    std::vector<std::string> v{ "d", "e" };
    cmList::append(list, v.begin(), v.end());

    if (list != "d;e") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    cmList::prepend(list, "d;e"_s);

    if (list != "d;e;a;b;c") {
      result = false;
    }
  }
  {
    std::string list;
    cmList::prepend(list, "d;e"_s);

    if (list != "d;e") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    cmList::prepend(list, ""_s);

    if (list != ";a;b;c") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    std::vector<std::string> v{ "d", "e" };
    cmList::prepend(list, v.begin(), v.end());

    if (list != "d;e;a;b;c") {
      result = false;
    }
  }
  {
    std::string list{ "a;b;c" };
    std::vector<std::string> v;
    cmList::prepend(list, v.begin(), v.end());

    if (list != "a;b;c") {
      result = false;
    }
  }
  {
    std::string list;
    std::vector<std::string> v{ "d", "e" };
    cmList::prepend(list, v.begin(), v.end());

    if (list != "d;e") {
      result = false;
    }
  }

  checkResult(result);

  return result;
}
}

int testList(int /*unused*/, char* /*unused*/[])
{
  int result = 0;

  if (!testConstructors()) {
    result = 1;
  }
  if (!testAssign()) {
    result = 1;
  }
  if (!testConversions()) {
    result = 1;
  }
  if (!testAccess()) {
    result = 1;
  }
  if (!testModifiers()) {
    result = 1;
  }
  if (!testRemoveItems()) {
    result = 1;
  }
  if (!testRemoveDuplicates()) {
    result = 1;
  }
  if (!testFilter()) {
    result = 1;
  }
  if (!testReverse()) {
    result = 1;
  }
  if (!testSort()) {
    result = 1;
  }
  if (!testTransform()) {
    result = 1;
  }
  if (!testStaticModifiers()) {
    result = 1;
  }

  return result;
}
