
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>

#include <cmext/enum_set>

namespace {

int failed = 0;

void testDeclaration()
{
  std::cout << "testDeclaration()" << std::endl;

  {
    enum class Test : std::uint8_t
    {
      A,
      B,
      C,
      D
    };
    cm::enum_set<Test> testSet1;
    cm::enum_set<Test> testSet2 = Test::A;
    cm::enum_set<Test> testSet3 = Test::A | Test::C;
    cm::enum_set<Test> testSet4 = Test::A + Test::C;
    cm::enum_set<Test> testSet5{ Test::A, Test::C };
    cm::enum_set<Test> testSet6 = testSet3;

    if (!testSet1.empty()) {
      ++failed;
    }
    if (testSet2.size() != 1) {
      ++failed;
    }
    if (testSet3.size() != 2 || testSet4.size() != 2 || testSet5.size() != 2 ||
        testSet6.size() != 2) {
      ++failed;
    }
    if (testSet3 != testSet4 || testSet4 != testSet5 || testSet5 != testSet6) {
      ++failed;
    }
  }
  {
    enum class Test : std::uint8_t
    {
      A,
      B,
      C,
      D
    };
    cm::enum_set<Test> testSet1;
    cm::enum_set<Test, 4> testSet2;

    if (testSet1.size() != 0 ||
        testSet1.max_size() !=
          std::numeric_limits<
            typename std::underlying_type<Test>::type>::digits) {
      ++failed;
    }
    if (testSet2.size() != 0 || testSet2.max_size() != 4) {
      ++failed;
    }
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
  cm::enum_set<Test, 4> testSet{ Test::A, Test::C, Test::B };

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
  {
    cm::enum_set<Test> testSet1;
    auto testSet2 = Test::A + Test::C + Test::B;

    testSet1.set({ Test::A, Test::C, Test::B });
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.reset();
    testSet1.set(Test::A | Test::C | Test::B);
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.reset();
    testSet1.set(Test::A + Test::C + Test::B);
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.reset();
    testSet1 = { Test::A, Test::C, Test::B };
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.reset();
    testSet1 = Test::A | Test::C | Test::B;
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.clear();
    testSet1 = Test::A + Test::C + Test::B;
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.clear();
    testSet1 |= Test::A;
    testSet1 |= Test::C | Test::B;
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
  }
  {
    using ESet = cm::enum_set<Test, 5>;
    ESet testSet1;
    ESet testSet2{ Test::A, Test::C, Test::B };

    testSet1.set();
    if (testSet1.size() != 5 || testSet1.size() != testSet1.max_size()) {
      ++failed;
    }
    testSet1.flip({ Test::D, Test::E });
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.flip(Test::D | Test::E);
    testSet2 += Test::D + Test::E;
    if (testSet1.size() != 5 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1.flip(Test::E);
    testSet2 -= Test::E;
    if (testSet1.size() != 4 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1 ^= { Test::A, Test::B, Test::E, Test::D };
    testSet2 = { Test::C, Test::E };
    if (testSet1.size() != 2 || testSet1 != testSet2) {
      ++failed;
    }
    testSet1 ^= { Test::A, Test::B, Test::E };
    testSet2 = { Test::A, Test::B, Test::C };
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
    testSet2 = Test::A | Test::B | Test::C;
    if (testSet1.size() != 3 || testSet1 != testSet2) {
      ++failed;
    }
  }
}

void testChecks()
{
  std::cout << "testChecks()" << std::endl;

  {
    enum class Test : std::uint8_t
    {
      A,
      B,
      C,
      D
    };

    cm::enum_set<Test> testSet;

    if (!testSet.empty()) {
      ++failed;
    }

    testSet = Test::A;
    if (testSet.empty()) {
      ++failed;
    }
    if (!testSet) {
      ++failed;
    }
    if (!testSet.contains(Test::A)) {
      ++failed;
    }
    if (testSet.find(Test::A) == testSet.end()) {
      ++failed;
    }
    if (testSet.find(Test::C) != testSet.end()) {
      ++failed;
    }
  }
  {
    enum class Test : std::uint8_t
    {
      A,
      B,
      C,
      D
    };

    cm::enum_set<Test, 4> testSet;

    if (!testSet.none()) {
      ++failed;
    }
    if (testSet.any() || testSet.all()) {
      ++failed;
    }

    testSet = Test::A;
    if (!testSet.any() || testSet.none() || testSet.all()) {
      ++failed;
    }

    testSet.set();
    if (!testSet.all() || !testSet.any() || testSet.none()) {
      ++failed;
    }
  }
  {
    enum class Test : std::uint8_t
    {
      A,
      B,
      C,
      D
    };

    cm::enum_set<Test> testSet1;
    cm::enum_set<Test> testSet2{ Test::A, Test::C };

    if (!testSet1.none_of(testSet2) || testSet1.any_of(testSet2) ||
        testSet1.all_of(testSet2)) {
      ++failed;
    }

    testSet1 = Test::A | Test::D;
    if (testSet1.none_of(testSet2) || !testSet1.any_of(testSet2) ||
        testSet1.all_of(testSet2)) {
      ++failed;
    }

    testSet1 |= Test::C;
    if (testSet1.none_of(testSet2) || !testSet1.any_of(testSet2) ||
        !testSet1.all_of(testSet2)) {
      ++failed;
    }
  }
}
}

int testCMExtEnumSet(int /*unused*/, char* /*unused*/[])
{
  testDeclaration();
  testIteration();
  testEdition();
  testChecks();

  return failed;
}
