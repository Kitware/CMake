/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <type_traits>

enum class cmStackType
{
  Const,
  Mutable,
};

template <typename T, cmStackType Mutable>
struct cmStackEntry;

/** Base class template for CRTP to represent a stack of values.
    Copies of the stack <i>share data</i>; mutating data on one copy will
    change the data on <i>all</i> copies.  */
template <typename T, typename Stack,
          cmStackType Mutable = cmStackType::Mutable>
class cmStack
{
  using Entry = cmStackEntry<T, Mutable>;

  std::shared_ptr<Entry const> TopEntry;

public:
  /** Default-construct an empty stack.  */
  cmStack();

  /** Get a stack with the given call context added to the top.  */
  Stack Push(T value) const;

  /** Get a stack with the top level removed.
      May not be called until after a matching Push.  */
  Stack Pop() const;

  /** Get the value at the top of the stack.
      This may be called only if Empty() would return false.  */
  T const& Top() const;
  template <bool E = (Mutable == cmStackType::Mutable)>
  typename std::enable_if<E, T>::type& Top();

  /** Return true if this stack is empty.  */
  bool Empty() const;

protected:
  using Base = cmStack<T, Stack, Mutable>;

  cmStack(std::shared_ptr<Entry const> parent, T value);
  cmStack(std::shared_ptr<Entry const> top);
};

/** Specialization of cmStack for CRTP to represent a stack of constant values.
    Provide value semantics, but use efficient reference-counting underneath
    to avoid copies.  */
template <typename T, typename Stack>
using cmConstStack = cmStack<T const, Stack, cmStackType::Const>;
