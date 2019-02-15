/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmRange_h
#define cmRange_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cm_kwiml.h"
#include <algorithm>
#include <iterator>

template <typename const_iterator_>
struct cmRange
{
  typedef const_iterator_ const_iterator;
  typedef typename std::iterator_traits<const_iterator>::value_type value_type;
  typedef typename std::iterator_traits<const_iterator>::difference_type
    difference_type;
  cmRange(const_iterator begin_, const_iterator end_)
    : Begin(begin_)
    , End(end_)
  {
  }
  const_iterator begin() const { return Begin; }
  const_iterator end() const { return End; }
  bool empty() const { return std::distance(Begin, End) == 0; }
  difference_type size() const { return std::distance(Begin, End); }

  cmRange& advance(KWIML_INT_intptr_t amount) &
  {
    std::advance(this->Begin, amount);
    return *this;
  }
  cmRange advance(KWIML_INT_intptr_t amount) &&
  {
    std::advance(this->Begin, amount);
    return std::move(*this);
  }

  cmRange& retreat(KWIML_INT_intptr_t amount) &
  {
    std::advance(End, -amount);
    return *this;
  }

  cmRange retreat(KWIML_INT_intptr_t amount) &&
  {
    std::advance(End, -amount);
    return std::move(*this);
  }

private:
  const_iterator Begin;
  const_iterator End;
};

template <typename Iter1, typename Iter2>
cmRange<Iter1> cmMakeRange(Iter1 begin, Iter2 end)
{
  return cmRange<Iter1>(begin, end);
}

template <typename Range>
cmRange<typename Range::const_iterator> cmMakeRange(Range const& range)
{
  return cmRange<typename Range::const_iterator>(range.begin(), range.end());
}

template <typename Range>
cmRange<typename Range::const_reverse_iterator> cmReverseRange(
  Range const& range)
{
  return cmRange<typename Range::const_reverse_iterator>(range.rbegin(),
                                                         range.rend());
}

#endif
