/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmAlgorithms_h
#define cmAlgorithms_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmRange.h"

#include "cm_kwiml.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

inline bool cmHasLiteralPrefixImpl(const std::string& str1, const char* str2,
                                   size_t N)
{
  return strncmp(str1.c_str(), str2, N) == 0;
}

inline bool cmHasLiteralPrefixImpl(const char* str1, const char* str2,
                                   size_t N)
{
  return strncmp(str1, str2, N) == 0;
}

inline bool cmHasLiteralSuffixImpl(const std::string& str1, const char* str2,
                                   size_t N)
{
  size_t len = str1.size();
  return len >= N && strcmp(str1.c_str() + len - N, str2) == 0;
}

inline bool cmHasLiteralSuffixImpl(const char* str1, const char* str2,
                                   size_t N)
{
  size_t len = strlen(str1);
  return len >= N && strcmp(str1 + len - N, str2) == 0;
}

template <typename T, size_t N>
bool cmHasLiteralPrefix(const T& str1, const char (&str2)[N])
{
  return cmHasLiteralPrefixImpl(str1, str2, N - 1);
}

template <typename T, size_t N>
bool cmHasLiteralSuffix(const T& str1, const char (&str2)[N])
{
  return cmHasLiteralSuffixImpl(str1, str2, N - 1);
}

struct cmStrCmp
{
  cmStrCmp(const char* test)
    : m_test(test)
  {
  }
  cmStrCmp(std::string test)
    : m_test(std::move(test))
  {
  }

  bool operator()(const std::string& input) const { return m_test == input; }

  bool operator()(const char* input) const
  {
    return strcmp(input, m_test.c_str()) == 0;
  }

private:
  const std::string m_test;
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

template <typename Container, typename Predicate>
void cmEraseIf(Container& cont, Predicate pred)
{
  cont.erase(std::remove_if(cont.begin(), cont.end(), pred), cont.end());
}

namespace ContainerAlgorithms {

template <typename T>
struct cmIsPair
{
  enum
  {
    value = false
  };
};

template <typename K, typename V>
struct cmIsPair<std::pair<K, V>>
{
  enum
  {
    value = true
  };
};

template <typename Range,
          bool valueTypeIsPair = cmIsPair<typename Range::value_type>::value>
struct DefaultDeleter
{
  void operator()(typename Range::value_type value) const { delete value; }
};

template <typename Range>
struct DefaultDeleter<Range, /* valueTypeIsPair = */ true>
{
  void operator()(typename Range::value_type value) const
  {
    delete value.second;
  }
};

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

typedef cmRange<std::vector<std::string>::const_iterator> cmStringRange;

class cmListFileBacktrace;
typedef cmRange<std::vector<cmListFileBacktrace>::const_iterator>
  cmBacktraceRange;

template <typename Range>
void cmDeleteAll(Range const& r)
{
  std::for_each(r.begin(), r.end(),
                ContainerAlgorithms::DefaultDeleter<Range>());
}

template <typename T, typename Range>
void cmAppend(std::vector<T>& v, Range const& r)
{
  v.insert(v.end(), r.begin(), r.end());
}

template <typename T, typename InputIt>
void cmAppend(std::vector<T>& v, InputIt first, InputIt last)
{
  v.insert(v.end(), first, last);
}

template <typename Range>
std::string cmJoin(Range const& r, const char* delimiter)
{
  if (r.empty()) {
    return std::string();
  }
  std::ostringstream os;
  typedef typename Range::value_type ValueType;
  typedef typename Range::const_iterator InputIt;
  const InputIt first = r.begin();
  InputIt last = r.end();
  --last;
  std::copy(first, last, std::ostream_iterator<ValueType>(os, delimiter));

  os << *last;

  return os.str();
}

template <typename Range>
std::string cmJoin(Range const& r, std::string const& delimiter)
{
  return cmJoin(r, delimiter.c_str());
}

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
  const typename Range::iterator rangeEnd = r.end();
  if (remIt == remEnd) {
    return rangeEnd;
  }

  typename Range::iterator writer = r.begin();
  std::advance(writer, *remIt);
  typename Range::iterator pivot = writer;
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
  using Hash = struct
  {
    std::size_t operator()(ForwardIterator it) const
    {
      return std::hash<Value>{}(*it);
    }
  };

  using Equal = struct
  {
    bool operator()(ForwardIterator it1, ForwardIterator it2) const
    {
      return *it1 == *it2;
    }
  };
  std::unordered_set<ForwardIterator, Hash, Equal> uniq;

  ForwardIterator result = first;
  while (first != last) {
    if (uniq.find(first) == uniq.end()) {
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

template <typename Range>
std::string cmWrap(std::string const& prefix, Range const& r,
                   std::string const& suffix, std::string const& sep)
{
  if (r.empty()) {
    return std::string();
  }
  return prefix + cmJoin(r, suffix + sep + prefix) + suffix;
}

template <typename Range>
std::string cmWrap(char prefix, Range const& r, char suffix,
                   std::string const& sep)
{
  return cmWrap(std::string(1, prefix), r, std::string(1, suffix), sep);
}

template <typename Range, typename T>
typename Range::const_iterator cmFindNot(Range const& r, T const& t)
{
  return std::find_if(r.begin(), r.end(), [&t](T const& i) { return i != t; });
}

template <class Iter>
std::reverse_iterator<Iter> cmMakeReverseIterator(Iter it)
{
  return std::reverse_iterator<Iter>(it);
}

inline bool cmHasPrefix(std::string const& str, std::string const& prefix)
{
  if (str.size() < prefix.size()) {
    return false;
  }
  return str.compare(0, prefix.size(), prefix) == 0;
}

inline bool cmHasSuffix(const std::string& str, const std::string& suffix)
{
  if (str.size() < suffix.size()) {
    return false;
  }
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline void cmStripSuffixIfExists(std::string& str, const std::string& suffix)
{
  if (cmHasSuffix(str, suffix)) {
    str.resize(str.size() - suffix.size());
  }
}

namespace cm {

#if defined(CMake_HAVE_CXX_MAKE_UNIQUE)

using std::make_unique;

#else

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif

#if __cplusplus >= 201703L || defined(_MSVC_LANG) && _MSVC_LANG >= 201703L

using std::size;

#else

// std::size backport from C++17.
template <class C>
#  if !defined(_MSC_VER) || _MSC_VER >= 1900
constexpr
#  endif
  auto
  size(C const& c) -> decltype(c.size())
{
  return c.size();
}

template <typename T, size_t N>
#  if !defined(_MSC_VER) || _MSC_VER >= 1900
constexpr
#  endif
  std::size_t
  size(const T (&)[N]) throw()
{
  return N;
}

#endif

template <typename T>
int isize(const T& t)
{
  return static_cast<int>(cm::size(t));
}

#if __cplusplus >= 201402L || defined(_MSVC_LANG) && _MSVC_LANG >= 201402L

using std::cbegin;
using std::cend;

#else

// std::c{begin,end} backport from C++14
template <class C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
auto cbegin(C const& c)
#  else
constexpr auto cbegin(C const& c) noexcept(noexcept(std::begin(c)))
#  endif
  -> decltype(std::begin(c))
{
  return std::begin(c);
}

template <class C>
#  if defined(_MSC_VER) && _MSC_VER < 1900
auto cend(C const& c)
#  else
constexpr auto cend(C const& c) noexcept(noexcept(std::end(c)))
#  endif
  -> decltype(std::end(c))
{
  return std::end(c);
}

#endif

} // namespace cm

#endif
