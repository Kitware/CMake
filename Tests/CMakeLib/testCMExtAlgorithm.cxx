#include <iostream>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <cmext/algorithm>

namespace {

int failed = 0;

void testAppend()
{
  std::cout << "testAppend()" << std::endl;

  // ----------------------------------------------------
  // cm::append(Vector, Iterator, Iterator)
  {
    std::vector<int> v1{ 1, 2, 3 };
    std::vector<int> v1_ref{ 1, 2, 3, 4, 5, 6 };
    std::vector<int> v2{ 4, 5, 6 };
    std::vector<int> v2_ref{ 4, 5, 6 };

    cm::append(v1, v2.begin(), v2.end());

    if (v1 != v1_ref || v2 != v2_ref) {
      ++failed;
    }
  }

  // ----------------------------------------------------
  // cm::append(Vector, Range)
  {
    std::vector<int> v1{ 1, 2, 3 };
    std::vector<int> v1_ref{ 1, 2, 3, 4, 5, 6 };
    std::vector<int> v2{ 4, 5, 6 };
    std::vector<int> v2_ref{ 4, 5, 6 };

    cm::append(v1, v2);

    if (v1 != v1_ref || v2 != v2_ref) {
      ++failed;
    }
  }

  // ----------------------------------------------------
  // cm::append(Vector<*>, Vector<unique_ptr>)
  {
    std::vector<int*> v1{ new int(1), new int(2), new int(3) };
    std::vector<int*> v1_ref = v1;
    std::vector<std::unique_ptr<int>> v2;

    v2.emplace_back(new int(4));
    v2.emplace_back(new int(5));
    v2.emplace_back(new int(6));

    cm::append(v1, v2);

    if (v1.size() == 6 && v2.size() == 3) {
      for (int i = 0; i < 3; i++) {
        if (v1[i] != v1_ref[i]) {
          ++failed;
          break;
        }
      }
      for (int i = 0; i < 3; i++) {
        if (v1[i + 3] != v2[i].get()) {
          ++failed;
          break;
        }
      }
    } else {
      ++failed;
    }

    // free memory to please memory sanitizer
    delete v1[0];
    delete v1[1];
    delete v1[2];
  }

  // ----------------------------------------------------
  // cm::append(Vector<unique_ptr>, Vector<unique_ptr>)
  {
    std::vector<std::unique_ptr<int>> v1;
    std::vector<std::unique_ptr<int>> v2;

    v1.emplace_back(new int(1));
    v1.emplace_back(new int(2));
    v1.emplace_back(new int(3));

    v2.emplace_back(new int(4));
    v2.emplace_back(new int(5));
    v2.emplace_back(new int(6));

    cm::append(v1, std::move(v2));

    if (v1.size() == 6 && v2.empty()) {
      for (int i = 0; i < 6; i++) {
        if (*v1[i] != i + 1) {
          ++failed;
          break;
        }
      }
    } else {
      ++failed;
    }
  }
}
}

int testCMExtAlgorithm(int /*unused*/, char* /*unused*/ [])
{
  testAppend();

  return failed;
}
