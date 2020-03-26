/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmAlgorithms_h
#define cmAlgorithms_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cm_kwiml.h"

#include "cmRange.h"

template <std::size_t N>
struct cmOverloadPriority : cmOverloadPriority<N - 1>
{
};

template <>
struct cmOverloadPriority<0>
{
};

template <typename FwdIt>
FwdIt cmRotate(FwdIt first, FwdIt middle, FwdIt last)
{
  const typename std::iterator_traits<FwdIt>::difference_type dist =
    std::distance(middle, last);
  std::rotate(first, middle, last);
  std::advance(first, dist);
  return first;
}

template <typename Range, typename Key>
auto cmContainsImpl(Range const& range, Key const& key, cmOverloadPriority<2>)
  -> decltype(range.exists(key))
{
  return range.exists(key);
}

template <typename Range, typename Key>
auto cmContainsImpl(Range const& range, Key const& key, cmOverloadPriority<1>)
  -> decltype(range.find(key) != range.end())
{
  return range.find(key) != range.end();
}

template <typename Range, typename Key>
bool cmContainsImpl(Range const& range, Key const& key, cmOverloadPriority<0>)
{
  using std::begin;
  using std::end;
  return std::find(begin(range), end(range), key) != end(range);
}

template <typename Range, typename Key>
bool cmContains(Range const& range, Key const& key)
{
  return cmContainsImpl(range, key, cmOverloadPriority<2>{});
}

namespace ContainerAlgorithms {

template <typename FwdIt>
FwdIt RemoveN(FwdIt i1, FwdIt i2, size_t n)
{
  FwdIt m = i1;
  std::advance(m, n);
  return cmRotate(i1, m, i2);
}

template <typename Range>
struct BinarySearcher
{
  using argument_type = typename Range::value_type;
  BinarySearcher(Range const& r)
    : m_range(r)
  {
  }

  bool operator()(argument_type const& item) const
  {
    return std::binary_search(m_range.begin(), m_range.end(), item);
  }

private:
  Range const& m_range;
};
}

class cmListFileBacktrace;
using cmBacktraceRange =
  cmRange<std::vector<cmListFileBacktrace>::const_iterator>;

template <typename Range>
typename Range::const_iterator cmRemoveN(Range& r, size_t n)
{
  return ContainerAlgorithms::RemoveN(r.begin(), r.end(), n);
}

template <typename Range, typename InputRange>
typename Range::const_iterator cmRemoveIndices(Range& r, InputRange const& rem)
{
  typename InputRange::const_iterator remIt = rem.begin();
  typename InputRange::const_iterator remEnd = rem.end();
  const auto rangeEnd = r.end();
  if (remIt == remEnd) {
    return rangeEnd;
  }

  auto writer = r.begin();
  std::advance(writer, *remIt);
  auto pivot = writer;
  typename InputRange::value_type prevRem = *remIt;
  ++remIt;
  size_t count = 1;
  for (; writer != rangeEnd && remIt != remEnd; ++count, ++remIt) {
    std::advance(pivot, *remIt - prevRem);
    prevRem = *remIt;
    writer = ContainerAlgorithms::RemoveN(writer, pivot, count);
  }
  return ContainerAlgorithms::RemoveN(writer, rangeEnd, count);
}

template <typename Range, typename MatchRange>
typename Range::const_iterator cmRemoveMatching(Range& r, MatchRange const& m)
{
  return std::remove_if(r.begin(), r.end(),
                        ContainerAlgorithms::BinarySearcher<MatchRange>(m));
}

template <typename ForwardIterator>
ForwardIterator cmRemoveDuplicates(ForwardIterator first, ForwardIterator last)
{
  using Value = typename std::iterator_traits<ForwardIterator>::value_type;
  struct Hash
  {
    std::size_t operator()(ForwardIterator it) const
    {
      return std::hash<Value>{}(*it);
    }
  };

  struct Equal
  {
    bool operator()(ForwardIterator it1, ForwardIterator it2) const
    {
      return *it1 == *it2;
    }
  };
  std::unordered_set<ForwardIterator, Hash, Equal> uniq;

  ForwardIterator result = first;
  while (first != last) {
    if (!cmContains(uniq, first)) {
      if (result != first) {
        *result = std::move(*first);
      }
      uniq.insert(result);
      ++result;
    }
    ++first;
  }
  return result;
}

template <typename Range>
typename Range::const_iterator cmRemoveDuplicates(Range& r)
{
  return cmRemoveDuplicates(r.begin(), r.end());
}

template <typename Range, typename T>
typename Range::const_iterator cmFindNot(Range const& r, T const& t)
{
  return std::find_if(r.begin(), r.end(), [&t](T const& i) { return i != t; });
}

#endif
