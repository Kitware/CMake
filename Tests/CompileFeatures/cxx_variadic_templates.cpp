template<int... Is>
struct Interface;

template<int I>
struct Interface<I>
{
  static int accumulate()
  {
    return I;
  }
};

template<int I, int... Is>
struct Interface<I, Is...>
{
  static int accumulate()
  {
    return I + Interface<Is...>::accumulate();
  }
};

// Note: split this into a separate test if a
// cxx_variadic_template_template_parameters feature is added.

template<typename T>
struct eval {
  enum {
    Matched = 0
  };
};

template<template<typename...> class T, typename... U>
struct eval<T<U...> > {
  enum {
    Matched = 1
  };
};

template<typename...>
struct A {

};
template<typename T>
struct B {

};
template<typename T, typename U>
struct C {

};
template<typename T, typename U, typename...>
struct D {

};

// Note: This test assumes that a compiler supporting this feature
// supports static_assert. Add a workaround if that does not hold.
static_assert(eval<A<> >::Matched, "A Matches");
static_assert(eval<A<int> >::Matched, "A Matches");
static_assert(eval<A<int, char> >::Matched, "A Matches");
static_assert(eval<B<int> >::Matched, "B Matches");
static_assert(eval<C<int, char> >::Matched, "C Matches");
static_assert(eval<D<int, char> >::Matched, "D Matches");
static_assert(eval<D<int, char, bool> >::Matched, "D Matches");
static_assert(eval<D<int, char, bool, double> >::Matched, "D Matches");
