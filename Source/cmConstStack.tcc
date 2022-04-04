/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <cassert>
#include <memory>
#include <utility>

template <typename T, typename Stack>
struct cmConstStack<T, Stack>::Entry
{
  Entry(std::shared_ptr<Entry const> parent, T value)
    : Value(std::move(value))
    , Parent(std::move(parent))
  {
  }

  T Value;
  std::shared_ptr<Entry const> Parent;
};

template <typename T, typename Stack>
cmConstStack<T, Stack>::cmConstStack() = default;

template <typename T, typename Stack>
Stack cmConstStack<T, Stack>::Push(T value) const
{
  return Stack(this->TopEntry, std::move(value));
}

template <typename T, typename Stack>
Stack cmConstStack<T, Stack>::Pop() const
{
  assert(this->TopEntry);
  return Stack(this->TopEntry->Parent);
}

template <typename T, typename Stack>
T const& cmConstStack<T, Stack>::Top() const
{
  assert(this->TopEntry);
  return this->TopEntry->Value;
}

template <typename T, typename Stack>
bool cmConstStack<T, Stack>::Empty() const
{
  return !this->TopEntry;
}

template <typename T, typename Stack>
cmConstStack<T, Stack>::cmConstStack(std::shared_ptr<Entry const> parent,
                                     T value)
  : TopEntry(
      std::make_shared<Entry const>(std::move(parent), std::move(value)))
{
}

template <typename T, typename Stack>
cmConstStack<T, Stack>::cmConstStack(std::shared_ptr<Entry const> top)
  : TopEntry(std::move(top))
{
}
