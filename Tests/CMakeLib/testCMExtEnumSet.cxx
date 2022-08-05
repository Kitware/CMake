
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <set>
#include <utility>

#include <cmext/enum_set>

namespace {

int failed = 0;

void testDeclaration()
{
  std::cout << "testDeclaration()" << std::endl;

  enum class Test : std::uint8_t
  {
    A,
    B,
    C,
    D
  };
  cm::enum_set<Test> testSet1;
  cm::enum_set<Test> testSet2{ Test::A, Test::C };
  cm::enum_set<Test> testSet3 = testSet2;

  if (!testSet1.empty()) {
    ++failed;
  }
  if (testSet2.size() != 2) {
    ++failed;
  }
  if (testSet3.size() != 2) {
    ++failed;
  }
}

void testIteration()
{
  std::cout << "testIteration()" << std::endl;

  enum class Test : std::uint8_t
  {
    A,
    B,
    C,
    D
  };
  cm::enum_set<Test> testSet{ Test::A, Test::C, Test::B };

  if (testSet.size() != 3) {
    ++failed;
  }

  std::set<std::uint8_t> reference{ static_cast<std::uint8_t>(Test::A),
                                    static_cast<std::uint8_t>(Test::B),
                                    static_cast<std::uint8_t>(Test::C) };
  std::set<std::uint8_t> s;

  for (auto e : testSet) {
    s.insert(static_cast<std::uint8_t>(e));
  }
  if (s != reference) {
    ++failed;
  }

  s.clear();
  for (auto rit = testSet.rbegin(); rit != testSet.rend(); rit++) {
    s.insert(static_cast<std::uint8_t>(*rit));
  }
  if (s != reference) {
    ++failed;
  }
}

void testEdition()
{
  std::cout << "testEdition()" << std::endl;

  enum class Test : std::uint8_t
  {
    A,
    B,
    C,
    D,
    E
  };

  {
    cm::enum_set<Test> testSet{ Test::A, Test::C, Test::B };

    auto pos = testSet.insert(Test::E);
    if (!pos.second || testSet.size() != 4 || *(pos.first) != Test::E ||
        testSet.find(Test::E) == testSet.end()) {
      ++failed;
    }
    testSet.insert(Test::E);
    if (testSet.size() != 4 || testSet.find(Test::E) == testSet.end()) {
      ++failed;
    }

    testSet.erase(Test::A);
    if (testSet.size() != 3 || testSet.find(Test::A) != testSet.end()) {
      ++failed;
    }
    testSet.erase(Test::A);
    if (testSet.size() != 3 || testSet.find(Test::A) != testSet.end()) {
      ++failed;
    }
  }
  {
    cm::enum_set<Test> testSet{ Test::A, Test::C, Test::B };

    testSet += { Test::D, Test::E };

    std::set<std::uint8_t> reference{ static_cast<std::uint8_t>(Test::A),
                                      static_cast<std::uint8_t>(Test::B),
                                      static_cast<std::uint8_t>(Test::C),
                                      static_cast<std::uint8_t>(Test::D),
                                      static_cast<std::uint8_t>(Test::E) };
    std::set<std::uint8_t> s;
    for (auto e : testSet) {
      s.insert(static_cast<std::uint8_t>(e));
    }
    if (s != reference) {
      ++failed;
    }

    testSet -= { Test::D, Test::B };
    reference.erase(static_cast<std::uint8_t>(Test::D));
    reference.erase(static_cast<std::uint8_t>(Test::B));
    s.clear();
    for (auto e : testSet) {
      s.insert(static_cast<std::uint8_t>(e));
    }
    if (s != reference) {
      ++failed;
    }
  }
  {
    cm::enum_set<Test> testSet1{ Test::A, Test::C, Test::B };
    cm::enum_set<Test> testSet2{ Test::A, Test::D, Test::E };
    testSet1.insert(testSet2.cbegin(), testSet2.cend());

    std::set<std::uint8_t> reference{ static_cast<std::uint8_t>(Test::A),
                                      static_cast<std::uint8_t>(Test::B),
                                      static_cast<std::uint8_t>(Test::C),
                                      static_cast<std::uint8_t>(Test::D),
                                      static_cast<std::uint8_t>(Test::E) };
    std::set<std::uint8_t> s;
    for (auto e : testSet1) {
      s.insert(static_cast<std::uint8_t>(e));
    }
    if (s != reference) {
      ++failed;
    }

    testSet1.erase(testSet2);

    reference.erase(static_cast<std::uint8_t>(Test::A));
    reference.erase(static_cast<std::uint8_t>(Test::D));
    reference.erase(static_cast<std::uint8_t>(Test::E));
    s.clear();
    for (auto e : testSet1) {
      s.insert(static_cast<std::uint8_t>(e));
    }
    if (s != reference) {
      ++failed;
    }
  }
  {
    cm::enum_set<Test> testSet1{ Test::A, Test::C, Test::B };
    cm::enum_set<Test> testSet2{ Test::C, Test::E };

    testSet1.flip(Test::A);
    if (testSet1.size() != 2 || testSet1.contains(Test::A)) {
      ++failed;
    }

    testSet1.flip(testSet2);
    std::set<std::uint8_t> reference{ static_cast<std::uint8_t>(Test::B),
                                      static_cast<std::uint8_t>(Test::E) };
    std::set<std::uint8_t> s;
    for (auto e : testSet1) {
      s.insert(static_cast<std::uint8_t>(e));
    }
    if (s != reference) {
      ++failed;
    }
  }
}
}

int testCMExtEnumSet(int /*unused*/, char* /*unused*/ [])
{
  testDeclaration();
  testIteration();
  testEdition();

  return failed;
}
