/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmAlgorithms_h
#define cmAlgorithms_h

#include "cmStandardIncludes.h"

inline bool cmHasLiteralPrefixImpl(const std::string &str1,
                                 const char *str2,
                                 size_t N)
{
  return strncmp(str1.c_str(), str2, N) == 0;
}

inline bool cmHasLiteralPrefixImpl(const char* str1,
                                 const char *str2,
                                 size_t N)
{
  return strncmp(str1, str2, N) == 0;
}

inline bool cmHasLiteralSuffixImpl(const std::string &str1,
                                   const char *str2,
                                   size_t N)
{
  size_t len = str1.size();
  return len >= N && strcmp(str1.c_str() + len - N, str2) == 0;
}

inline bool cmHasLiteralSuffixImpl(const char* str1,
                                   const char* str2,
                                   size_t N)
{
  size_t len = strlen(str1);
  return len >= N && strcmp(str1 + len - N, str2) == 0;
}

template<typename T, size_t N>
const T* cmArrayBegin(const T (&a)[N]) { return a; }
template<typename T, size_t N>
const T* cmArrayEnd(const T (&a)[N]) { return a + N; }
template<typename T, size_t N>
size_t cmArraySize(const T (&)[N]) { return N; }

template<typename T, size_t N>
bool cmHasLiteralPrefix(T str1, const char (&str2)[N])
{
  return cmHasLiteralPrefixImpl(str1, str2, N - 1);
}

template<typename T, size_t N>
bool cmHasLiteralSuffix(T str1, const char (&str2)[N])
{
  return cmHasLiteralSuffixImpl(str1, str2, N - 1);
}

struct cmStrCmp {
  cmStrCmp(const char *test) : m_test(test) {}
  cmStrCmp(const std::string &test) : m_test(test) {}

  bool operator()(const std::string& input) const
  {
    return m_test == input;
  }

  bool operator()(const char * input) const
  {
    return strcmp(input, m_test.c_str()) == 0;
  }

private:
  const std::string m_test;
};

template<typename FwdIt>
FwdIt cmRotate(FwdIt first, FwdIt middle, FwdIt last)
{
  typename std::iterator_traits<FwdIt>::difference_type dist =
      std::distance(middle, last);
  std::rotate(first, middle, last);
  std::advance(first, dist);
  return first;
}

namespace ContainerAlgorithms {

template<typename T>
struct cmIsPair
{
  enum { value = false };
};

template<typename K, typename V>
struct cmIsPair<std::pair<K, V> >
{
  enum { value = true };
};

template<typename Container,
    bool valueTypeIsPair = cmIsPair<typename Container::value_type>::value>
struct DefaultDeleter
{
  void operator()(typename Container::value_type value) const {
    delete value;
  }
};

template<typename Container>
struct DefaultDeleter<Container, /* valueTypeIsPair = */ true>
{
  void operator()(typename Container::value_type value) const {
    delete value.second;
  }
};

template<typename const_iterator_>
struct Range
{
  typedef const_iterator_ const_iterator;
  typedef typename std::iterator_traits<const_iterator>::value_type value_type;
  Range(const_iterator begin_, const_iterator end_)
    : Begin(begin_), End(end_) {}
  const_iterator begin() const { return Begin; }
  const_iterator end() const { return End; }
  bool empty() const { return std::distance(Begin, End) == 0; }
  Range& advance(cmIML_INT_intptr_t amount)
  {
    std::advance(Begin, amount);
    return *this;
  }

  Range& retreat(cmIML_INT_intptr_t amount)
  {
    std::advance(End, -amount);
    return *this;
  }
private:
  const_iterator Begin;
  const_iterator End;
};

template<typename Iter>
Iter RemoveN(Iter i1, Iter i2, size_t n)
{
  return cmRotate(i1, i1 + n, i2);
}

template<typename Range>
struct BinarySearcher
{
  typedef typename Range::value_type argument_type;
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

template<typename Iter1, typename Iter2>
ContainerAlgorithms::Range<Iter1> cmRange(Iter1 begin, Iter2 end)
{
  return ContainerAlgorithms::Range<Iter1>(begin, end);
}

template<typename Range>
ContainerAlgorithms::Range<typename Range::const_iterator>
cmRange(Range const& range)
{
  return ContainerAlgorithms::Range<typename Range::const_iterator>(
      range.begin(), range.end());
}

template<typename Container>
void cmDeleteAll(Container const& c)
{
  std::for_each(c.begin(), c.end(),
                ContainerAlgorithms::DefaultDeleter<Container>());
}

template<typename Range>
std::string cmJoin(Range const& r, const char* delimiter)
{
  if (r.empty())
    {
    return std::string();
    }
  std::ostringstream os;
  typedef typename Range::value_type ValueType;
  typedef typename Range::const_iterator InputIt;
  InputIt first = r.begin();
  InputIt last = r.end();
  --last;
  std::copy(first, last,
      std::ostream_iterator<ValueType>(os, delimiter));

  os << *last;

  return os.str();
}

template<typename Range>
std::string cmJoin(Range const& r, std::string delimiter)
{
  return cmJoin(r, delimiter.c_str());
};

template<typename Range>
typename Range::const_iterator cmRemoveN(Range& r, size_t n)
{
  return ContainerAlgorithms::RemoveN(r.begin(), r.end(), n);
}

template<typename Range, typename InputRange>
typename Range::const_iterator cmRemoveIndices(Range& r, InputRange const& rem)
{
  typename InputRange::const_iterator remIt = rem.begin();

  typename Range::iterator writer = r.begin() + *remIt;
  ++remIt;
  size_t count = 1;
  for ( ; writer != r.end() && remIt != rem.end(); ++count, ++remIt)
    {
    writer = ContainerAlgorithms::RemoveN(writer, r.begin() + *remIt, count);
    }
  writer = ContainerAlgorithms::RemoveN(writer, r.end(), count);
  return writer;
}

template<typename Range, typename MatchRange>
typename Range::const_iterator cmRemoveMatching(Range &r, MatchRange const& m)
{
  return std::remove_if(r.begin(), r.end(),
                        ContainerAlgorithms::BinarySearcher<MatchRange>(m));
}

template<typename Range>
typename Range::const_iterator cmRemoveDuplicates(Range& r)
{
  std::vector<typename Range::value_type> unique;
  unique.reserve(r.size());
  std::vector<size_t> indices;
  size_t count = 0;
  for(typename Range::const_iterator it = r.begin();
      it != r.end(); ++it, ++count)
    {
    typename Range::iterator low =
        std::lower_bound(unique.begin(), unique.end(), *it);
    if (low == unique.end() || *low != *it)
      {
      unique.insert(low, *it);
      }
    else
      {
      indices.push_back(count);
      }
    }
  if (indices.empty())
    {
    return r.end();
    }
  return cmRemoveIndices(r, indices);
}

template<typename Range>
std::string cmWrap(std::string prefix, Range const& r, std::string suffix,
                   std::string sep)
{
  if (r.empty())
    {
    return std::string();
    }
  return prefix + cmJoin(r, (suffix + sep + prefix).c_str()) + suffix;
}

template<typename Range>
std::string cmWrap(char prefix, Range const& r, char suffix, std::string sep)
{
  return cmWrap(std::string(1, prefix), r, std::string(1, suffix), sep);
}

template<typename Range, typename T>
typename Range::const_iterator cmFindNot(Range const& r, T const& t)
{
  return std::find_if(r.begin(), r.end(),
                      std::bind1st(std::not_equal_to<T>(), t));
}

#endif
