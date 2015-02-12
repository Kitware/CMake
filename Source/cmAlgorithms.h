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
  void operator()(typename Container::value_type value) {
    delete value;
  }
};

template<typename Container>
struct DefaultDeleter<Container, /* valueTypeIsPair = */ true>
{
  void operator()(typename Container::value_type value) {
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

#endif
