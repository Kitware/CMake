/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cassert>
#include <memory>
#include <utility>

template <typename T>
struct cmStackEntry<T, cmStackType::Mutable>
{
  cmStackEntry(std::shared_ptr<cmStackEntry const> parent, T value)
    : Value(std::move(value))
    , Parent(std::move(parent))
  {
  }

  T mutable Value;
  std::shared_ptr<cmStackEntry const> Parent;
};

template <typename T>
struct cmStackEntry<T, cmStackType::Const>
{
  cmStackEntry(std::shared_ptr<cmStackEntry const> parent, T value)
    : Value(std::move(value))
    , Parent(std::move(parent))
  {
  }

  T Value;
  std::shared_ptr<cmStackEntry const> Parent;
};

template <typename T, typename Stack, cmStackType Mutable>
cmStack<T, Stack, Mutable>::cmStack() = default;

template <typename T, typename Stack, cmStackType Mutable>
Stack cmStack<T, Stack, Mutable>::Push(T value) const
{
  return Stack(this->TopEntry, std::move(value));
}

template <typename T, typename Stack, cmStackType Mutable>
Stack cmStack<T, Stack, Mutable>::Pop() const
{
  assert(this->TopEntry);
  return Stack(this->TopEntry->Parent);
}

template <typename T, typename Stack, cmStackType Mutable>
T const& cmStack<T, Stack, Mutable>::Top() const
{
  assert(this->TopEntry);
  return this->TopEntry->Value;
}

template <typename T, typename Stack, cmStackType Mutable>
template <bool E>
typename std::enable_if<E, T>::type& cmStack<T, Stack, Mutable>::Top()
{
  static_assert(Mutable == cmStackType::Mutable,
                "T& cmStack::Top should only exist for mutable cmStack");
  assert(this->TopEntry);
  return this->TopEntry->Value;
}

template <typename T, typename Stack, cmStackType Mutable>
bool cmStack<T, Stack, Mutable>::Empty() const
{
  return !this->TopEntry;
}

template <typename T, typename Stack, cmStackType Mutable>
cmStack<T, Stack, Mutable>::cmStack(std::shared_ptr<Entry const> parent,
                                    T value)
  : TopEntry(
      std::make_shared<Entry const>(std::move(parent), std::move(value)))
{
}

template <typename T, typename Stack, cmStackType Mutable>
cmStack<T, Stack, Mutable>::cmStack(std::shared_ptr<Entry const> top)
  : TopEntry(std::move(top))
{
}
