/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmRange_h
#define cmRange_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <iterator>

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

private:
  Iter Begin;
  Iter End;
};

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

#endif
