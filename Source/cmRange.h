/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <functional>
#include <iterator>

namespace RangeIterators {

template <typename Iter, typename UnaryPredicate>
class FilterIterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using reference = typename std::iterator_traits<Iter>::reference;

  FilterIterator(Iter b, Iter e, UnaryPredicate p)
    : Cur(std::move(b))
    , End(std::move(e))
    , Pred(std::move(p))
  {
    this->SatisfyPredicate();
  }

  FilterIterator& operator++()
  {
    ++this->Cur;
    this->SatisfyPredicate();
    return *this;
  }

  FilterIterator& operator--()
  {
    do {
      --this->Cur;
    } while (!this->Pred(*this->Cur));
    return *this;
  }

  bool operator==(FilterIterator const& other) const
  {
    return this->Cur == other.Cur;
  }

  bool operator!=(FilterIterator const& other) const
  {
    return !this->operator==(other);
  }

  auto operator*() const -> decltype(*std::declval<Iter>())
  {
    return *this->Cur;
  }

private:
  void SatisfyPredicate()
  {
    while (this->Cur != this->End && !this->Pred(*this->Cur)) {
      ++this->Cur;
    }
  }

  Iter Cur;
  Iter End;
  UnaryPredicate Pred;
};

template <typename Iter, typename UnaryFunction>
class TransformIterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type =
    typename std::remove_cv<typename std::remove_reference<decltype(
      std::declval<UnaryFunction>()(*std::declval<Iter>()))>::type>::type;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using pointer = value_type const*;
  using reference = value_type const&;

  TransformIterator(Iter i, UnaryFunction f)
    : Base(std::move(i))
    , Func(std::move(f))
  {
  }

  TransformIterator& operator++()
  {
    ++this->Base;
    return *this;
  }

  TransformIterator& operator--()
  {
    --this->Base;
    return *this;
  }

  bool operator==(TransformIterator const& other) const
  {
    return this->Base == other.Base;
  }

  bool operator!=(TransformIterator const& other) const
  {
    return !this->operator==(other);
  }

  auto operator*() const
    -> decltype(std::declval<UnaryFunction>()(*std::declval<Iter>()))
  {
    return this->Func(*this->Base);
  }

private:
  Iter Base;
  UnaryFunction Func;
};

} // namespace RangeIterators

template <typename Iter>
class cmRange
{
public:
  using const_iterator = Iter;
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;

  cmRange(Iter b, Iter e)
    : Begin(std::move(b))
    , End(std::move(e))
  {
  }

  Iter begin() const { return this->Begin; }
  Iter end() const { return this->End; }
  bool empty() const { return this->Begin == this->End; }

  difference_type size() const
  {
    return std::distance(this->Begin, this->End);
  }

  cmRange& advance(difference_type amount) &
  {
    std::advance(this->Begin, amount);
    return *this;
  }

  cmRange advance(difference_type amount) &&
  {
    std::advance(this->Begin, amount);
    return std::move(*this);
  }

  cmRange& retreat(difference_type amount) &
  {
    std::advance(this->End, -amount);
    return *this;
  }

  cmRange retreat(difference_type amount) &&
  {
    std::advance(this->End, -amount);
    return std::move(*this);
  }

  template <typename UnaryPredicate>
  bool all_of(UnaryPredicate p) const
  {
    return std::all_of(this->Begin, this->End, std::ref(p));
  }

  template <typename UnaryPredicate>
  bool any_of(UnaryPredicate p) const
  {
    return std::any_of(this->Begin, this->End, std::ref(p));
  }

  template <typename UnaryPredicate>
  bool none_of(UnaryPredicate p) const
  {
    return std::none_of(this->Begin, this->End, std::ref(p));
  }

  template <typename UnaryPredicate>
  auto filter(UnaryPredicate p) const
    -> cmRange<RangeIterators::FilterIterator<Iter, UnaryPredicate>>
  {
    using It = RangeIterators::FilterIterator<Iter, UnaryPredicate>;
    return { It(this->Begin, this->End, p), It(this->End, this->End, p) };
  }

  template <typename UnaryFunction>
  auto transform(UnaryFunction f) const
    -> cmRange<RangeIterators::TransformIterator<Iter, UnaryFunction>>
  {
    using It = RangeIterators::TransformIterator<Iter, UnaryFunction>;
    return { It(this->Begin, f), It(this->End, f) };
  }

private:
  Iter Begin;
  Iter End;
};

template <typename Iter1, typename Iter2>
bool operator==(cmRange<Iter1> const& left, cmRange<Iter2> const& right)
{
  return left.size() == right.size() &&
    std::equal(left.begin(), left.end(), right.begin());
}

template <typename Iter1, typename Iter2>
auto cmMakeRange(Iter1 begin, Iter2 end) -> cmRange<Iter1>
{
  return { begin, end };
}

template <typename Range>
auto cmMakeRange(Range const& range) -> cmRange<decltype(range.begin())>
{
  return { range.begin(), range.end() };
}

template <typename Range>
auto cmReverseRange(Range const& range) -> cmRange<decltype(range.rbegin())>
{
  return { range.rbegin(), range.rend() };
}
