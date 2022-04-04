/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>

/** Base class template for CRTP to represent a stack of constant values.
    Provide value semantics, but use efficient reference-counting underneath
    to avoid copies.  */
template <typename T, typename Stack>
class cmConstStack
{
  struct Entry;
  std::shared_ptr<Entry const> TopEntry;

public:
  /** Default-construct an empty stack.  */
  cmConstStack();

  /** Get a stack with the given call context added to the top.  */
  Stack Push(T value) const;

  /** Get a stack with the top level removed.
      May not be called until after a matching Push.  */
  Stack Pop() const;

  /** Get the value at the top of the stack.
      This may be called only if Empty() would return false.  */
  T const& Top() const;

  /** Return true if this stack is empty.  */
  bool Empty() const;

protected:
  cmConstStack(std::shared_ptr<Entry const> parent, T value);
  cmConstStack(std::shared_ptr<Entry const> top);
};
