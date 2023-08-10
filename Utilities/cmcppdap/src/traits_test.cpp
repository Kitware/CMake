// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dap/traits.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {
namespace traits {

namespace {
struct S {};
struct E : S {};
void F1(S) {}
void F3(int, S, float) {}
void E1(E) {}
void E3(int, E, float) {}
}  // namespace

TEST(ParameterType, Function) {
  F1({});        // Avoid unused method warning
  F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParameterType<decltype(&F1), 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&F3), 0>, int>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&F3), 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&F3), 2>, float>::value,
                "");
}

TEST(ParameterType, Method) {
  class C {
   public:
    void F1(S) {}
    void F3(int, S, float) {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParameterType<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 0>, int>::value,
                "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParameterType, ConstMethod) {
  class C {
   public:
    void F1(S) const {}
    void F3(int, S, float) const {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParameterType<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 0>, int>::value,
                "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParameterType, StaticMethod) {
  class C {
   public:
    static void F1(S) {}
    static void F3(int, S, float) {}
  };
  C::F1({});        // Avoid unused method warning
  C::F3(0, {}, 0);  // Avoid unused method warning
  static_assert(std::is_same<ParameterType<decltype(&C::F1), 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 0>, int>::value,
                "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(&C::F3), 2>, float>::value,
                "");
}

TEST(ParameterType, FunctionLike) {
  using F1 = std::function<void(S)>;
  using F3 = std::function<void(int, S, float)>;
  static_assert(std::is_same<ParameterType<F1, 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<F3, 0>, int>::value, "");
  static_assert(std::is_same<ParameterType<F3, 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<F3, 2>, float>::value, "");
}

TEST(ParameterType, Lambda) {
  auto l1 = [](S) {};
  auto l3 = [](int, S, float) {};
  static_assert(std::is_same<ParameterType<decltype(l1), 0>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(l3), 0>, int>::value, "");
  static_assert(std::is_same<ParameterType<decltype(l3), 1>, S>::value, "");
  static_assert(std::is_same<ParameterType<decltype(l3), 2>, float>::value, "");
}

TEST(HasSignature, Function) {
  F1({});        // Avoid unused method warning
  F3(0, {}, 0);  // Avoid unused method warning
  static_assert(HasSignature<decltype(&F1), decltype(&F1)>::value, "");
  static_assert(HasSignature<decltype(&F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&F3), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&F1), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&F3), decltype(&F1)>::value, "");
}

TEST(HasSignature, Method) {
  class C {
   public:
    void F1(S) {}
    void F3(int, S, float) {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning

  static_assert(HasSignature<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
}

TEST(HasSignature, ConstMethod) {
  class C {
   public:
    void F1(S) const {}
    void F3(int, S, float) const {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning

  static_assert(HasSignature<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
}

TEST(HasSignature, StaticMethod) {
  class C {
   public:
    static void F1(S) {}
    static void F3(int, S, float) {}
  };
  C::F1({});        // Avoid unused method warning
  C::F3(0, {}, 0);  // Avoid unused method warning

  static_assert(HasSignature<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(&C::F3), decltype(&F1)>::value, "");
}

TEST(HasSignature, FunctionLike) {
  using f1 = std::function<void(S)>;
  using f3 = std::function<void(int, S, float)>;
  static_assert(HasSignature<f1, decltype(&F1)>::value, "");
  static_assert(HasSignature<f3, decltype(&F3)>::value, "");
  static_assert(HasSignature<f3, decltype(&F3)>::value, "");
  static_assert(HasSignature<f3, decltype(&F3)>::value, "");
  static_assert(!HasSignature<f1, decltype(&F3)>::value, "");
  static_assert(!HasSignature<f3, decltype(&F1)>::value, "");
  static_assert(!HasSignature<f3, decltype(&F1)>::value, "");
  static_assert(!HasSignature<f3, decltype(&F1)>::value, "");
}

TEST(HasSignature, Lambda) {
  auto l1 = [](S) {};
  auto l3 = [](int, S, float) {};
  static_assert(HasSignature<decltype(l1), decltype(&F1)>::value, "");
  static_assert(HasSignature<decltype(l3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(l3), decltype(&F3)>::value, "");
  static_assert(HasSignature<decltype(l3), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(l1), decltype(&F3)>::value, "");
  static_assert(!HasSignature<decltype(l3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(l3), decltype(&F1)>::value, "");
  static_assert(!HasSignature<decltype(l3), decltype(&F1)>::value, "");
}

////

TEST(CompatibleWith, Function) {
  F1({});        // Avoid unused method warning
  F3(0, {}, 0);  // Avoid unused method warning
  E1({});        // Avoid unused method warning
  E3(0, {}, 0);  // Avoid unused method warning
  static_assert(CompatibleWith<decltype(&F1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(&F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&F3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&F1), decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&F1)>::value, "");

  static_assert(CompatibleWith<decltype(&E1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(&E3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&E3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&E3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&F1), decltype(&E1)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&F3), decltype(&E3)>::value, "");
}

TEST(CompatibleWith, Method) {
  class C {
   public:
    void F1(S) {}
    void F3(int, S, float) {}
    void E1(E) {}
    void E3(int, E, float) {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  C().E1({});        // Avoid unused method warning
  C().E3(0, {}, 0);  // Avoid unused method warning

  static_assert(CompatibleWith<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");

  static_assert(CompatibleWith<decltype(&C::E1), decltype(&C::F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&C::E1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
}

TEST(CompatibleWith, ConstMethod) {
  class C {
   public:
    void F1(S) const {}
    void F3(int, S, float) const {}
    void E1(E) const {}
    void E3(int, E, float) const {}
  };
  C().F1({});        // Avoid unused method warning
  C().F3(0, {}, 0);  // Avoid unused method warning
  C().E1({});        // Avoid unused method warning
  C().E3(0, {}, 0);  // Avoid unused method warning

  static_assert(CompatibleWith<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");

  static_assert(CompatibleWith<decltype(&C::E1), decltype(&C::F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&C::E1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
}

TEST(CompatibleWith, StaticMethod) {
  class C {
   public:
    static void F1(S) {}
    static void F3(int, S, float) {}
    static void E1(E) {}
    static void E3(int, E, float) {}
  };
  C::F1({});        // Avoid unused method warning
  C::F3(0, {}, 0);  // Avoid unused method warning
  C::E1({});        // Avoid unused method warning
  C::E3(0, {}, 0);  // Avoid unused method warning

  static_assert(CompatibleWith<decltype(&C::F1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::F3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&F1)>::value, "");

  static_assert(CompatibleWith<decltype(&C::E1), decltype(&C::F1)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");
  static_assert(CompatibleWith<decltype(&C::E3), decltype(&C::F3)>::value, "");

  static_assert(!CompatibleWith<decltype(&C::F1), decltype(&C::E1)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
  static_assert(!CompatibleWith<decltype(&C::F3), decltype(&C::E3)>::value, "");
}

TEST(CompatibleWith, FunctionLike) {
  using f1 = std::function<void(S)>;
  using f3 = std::function<void(int, S, float)>;
  using e1 = std::function<void(E)>;
  using e3 = std::function<void(int, E, float)>;
  static_assert(CompatibleWith<f1, decltype(&F1)>::value, "");
  static_assert(CompatibleWith<f3, decltype(&F3)>::value, "");
  static_assert(CompatibleWith<f3, decltype(&F3)>::value, "");
  static_assert(CompatibleWith<f3, decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<f1, decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<f3, decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<f3, decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<f3, decltype(&F1)>::value, "");

  static_assert(CompatibleWith<e1, f1>::value, "");
  static_assert(CompatibleWith<e3, f3>::value, "");
  static_assert(CompatibleWith<e3, f3>::value, "");
  static_assert(CompatibleWith<e3, f3>::value, "");

  static_assert(!CompatibleWith<f1, e1>::value, "");
  static_assert(!CompatibleWith<f3, e3>::value, "");
  static_assert(!CompatibleWith<f3, e3>::value, "");
  static_assert(!CompatibleWith<f3, e3>::value, "");
}

TEST(CompatibleWith, Lambda) {
  auto f1 = [](S) {};
  auto f3 = [](int, S, float) {};
  auto e1 = [](E) {};
  auto e3 = [](int, E, float) {};
  static_assert(CompatibleWith<decltype(f1), decltype(&F1)>::value, "");
  static_assert(CompatibleWith<decltype(f3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(f3), decltype(&F3)>::value, "");
  static_assert(CompatibleWith<decltype(f3), decltype(&F3)>::value, "");

  static_assert(!CompatibleWith<decltype(f1), decltype(&F3)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(&F1)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(&F1)>::value, "");

  static_assert(CompatibleWith<decltype(e1), decltype(f1)>::value, "");
  static_assert(CompatibleWith<decltype(e3), decltype(f3)>::value, "");
  static_assert(CompatibleWith<decltype(e3), decltype(f3)>::value, "");
  static_assert(CompatibleWith<decltype(e3), decltype(f3)>::value, "");

  static_assert(!CompatibleWith<decltype(f1), decltype(e1)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(e3)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(e3)>::value, "");
  static_assert(!CompatibleWith<decltype(f3), decltype(e3)>::value, "");
}

}  // namespace traits
}  // namespace dap
