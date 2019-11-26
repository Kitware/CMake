/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <cm/string_view> // IWYU pragma: associated

#ifndef CMake_HAVE_CXX_STRING_VIEW

#  include <algorithm>
#  include <ostream>
#  include <stdexcept>

#  include "cm_kwiml.h"

namespace cm {

string_view::const_reference string_view::at(size_type pos) const
{
  if (pos >= size_) {
    throw std::out_of_range("Index out of range in string_view::at");
  }
  return data_[pos];
}

string_view::size_type string_view::copy(char* dest, size_type count,
                                         size_type pos) const
{
  if (pos > size_) {
    throw std::out_of_range("Index out of range in string_view::copy");
  }
  size_type const rcount = std::min(count, size_ - pos);
  traits_type::copy(dest, data_ + pos, rcount);
  return rcount;
}

string_view string_view::substr(size_type pos, size_type count) const
{
  if (pos > size_) {
    throw std::out_of_range("Index out of range in string_view::substr");
  }
  size_type const rcount = std::min(count, size_ - pos);
  return string_view(data_ + pos, rcount);
}

int string_view::compare(string_view v) const noexcept
{
  size_type const rlen = std::min(size_, v.size_);
  int c = traits_type::compare(data_, v.data_, rlen);
  if (c == 0) {
    if (size_ < v.size_) {
      c = -1;
    } else if (size_ > v.size_) {
      c = 1;
    }
  }
  return c;
}

int string_view::compare(size_type pos1, size_type count1, string_view v) const
{
  return substr(pos1, count1).compare(v);
}

int string_view::compare(size_type pos1, size_type count1, string_view v,
                         size_type pos2, size_type count2) const
{
  return substr(pos1, count1).compare(v.substr(pos2, count2));
}

int string_view::compare(const char* s) const
{
  return compare(string_view(s));
}

int string_view::compare(size_type pos1, size_type count1, const char* s) const
{
  return substr(pos1, count1).compare(string_view(s));
}

int string_view::compare(size_type pos1, size_type count1, const char* s,
                         size_type count2) const
{
  return substr(pos1, count1).compare(string_view(s, count2));
}

string_view::size_type string_view::find(string_view v, size_type pos) const
  noexcept
{
  for (; pos + v.size_ <= size_; ++pos) {
    if (std::char_traits<char>::compare(data_ + pos, v.data_, v.size_) == 0) {
      return pos;
    }
  }
  return npos;
}

string_view::size_type string_view::find(char c, size_type pos) const noexcept
{
  return find(string_view(&c, 1), pos);
}

string_view::size_type string_view::find(const char* s, size_type pos,
                                         size_type count) const
{
  return find(string_view(s, count), pos);
}

string_view::size_type string_view::find(const char* s, size_type pos) const
{
  return find(string_view(s), pos);
}

string_view::size_type string_view::rfind(string_view v, size_type pos) const
  noexcept
{
  if (size_ >= v.size_) {
    for (pos = std::min(pos, size_ - v.size_) + 1; pos > 0;) {
      --pos;
      if (std::char_traits<char>::compare(data_ + pos, v.data_, v.size_) ==
          0) {
        return pos;
      }
    }
  }
  return npos;
}

string_view::size_type string_view::rfind(char c, size_type pos) const noexcept
{
  return rfind(string_view(&c, 1), pos);
}

string_view::size_type string_view::rfind(const char* s, size_type pos,
                                          size_type count) const
{
  return rfind(string_view(s, count), pos);
}

string_view::size_type string_view::rfind(const char* s, size_type pos) const
{
  return rfind(string_view(s), pos);
}

string_view::size_type string_view::find_first_of(string_view v,
                                                  size_type pos) const noexcept
{
  for (; pos < size_; ++pos) {
    if (traits_type::find(v.data_, v.size_, data_[pos])) {
      return pos;
    }
  }
  return npos;
}

string_view::size_type string_view::find_first_of(char c, size_type pos) const
  noexcept
{
  return find_first_of(string_view(&c, 1), pos);
}

string_view::size_type string_view::find_first_of(const char* s, size_type pos,
                                                  size_type count) const
{
  return find_first_of(string_view(s, count), pos);
}

string_view::size_type string_view::find_first_of(const char* s,
                                                  size_type pos) const
{
  return find_first_of(string_view(s), pos);
}

string_view::size_type string_view::find_last_of(string_view v,
                                                 size_type pos) const noexcept
{
  if (size_ > 0) {
    for (pos = std::min(pos, size_ - 1) + 1; pos > 0;) {
      --pos;
      if (traits_type::find(v.data_, v.size_, data_[pos])) {
        return pos;
      }
    }
  }
  return npos;
}

string_view::size_type string_view::find_last_of(char c, size_type pos) const
  noexcept
{
  return find_last_of(string_view(&c, 1), pos);
}

string_view::size_type string_view::find_last_of(const char* s, size_type pos,
                                                 size_type count) const
{
  return find_last_of(string_view(s, count), pos);
}

string_view::size_type string_view::find_last_of(const char* s,
                                                 size_type pos) const
{
  return find_last_of(string_view(s), pos);
}

string_view::size_type string_view::find_first_not_of(string_view v,
                                                      size_type pos) const
  noexcept
{
  for (; pos < size_; ++pos) {
    if (!traits_type::find(v.data_, v.size_, data_[pos])) {
      return pos;
    }
  }
  return npos;
}

string_view::size_type string_view::find_first_not_of(char c,
                                                      size_type pos) const
  noexcept
{
  return find_first_not_of(string_view(&c, 1), pos);
}

string_view::size_type string_view::find_first_not_of(const char* s,
                                                      size_type pos,
                                                      size_type count) const
{
  return find_first_not_of(string_view(s, count), pos);
}

string_view::size_type string_view::find_first_not_of(const char* s,
                                                      size_type pos) const
{
  return find_first_not_of(string_view(s), pos);
}

string_view::size_type string_view::find_last_not_of(string_view v,
                                                     size_type pos) const
  noexcept
{
  if (size_ > 0) {
    for (pos = std::min(pos, size_ - 1) + 1; pos > 0;) {
      --pos;
      if (!traits_type::find(v.data_, v.size_, data_[pos])) {
        return pos;
      }
    }
  }
  return npos;
}

string_view::size_type string_view::find_last_not_of(char c,
                                                     size_type pos) const
  noexcept
{
  return find_last_not_of(string_view(&c, 1), pos);
}

string_view::size_type string_view::find_last_not_of(const char* s,
                                                     size_type pos,
                                                     size_type count) const
{
  return find_last_not_of(string_view(s, count), pos);
}

string_view::size_type string_view::find_last_not_of(const char* s,
                                                     size_type pos) const
{
  return find_last_not_of(string_view(s), pos);
}

std::ostream& operator<<(std::ostream& o, string_view v)
{
  return o.write(v.data(), v.size());
}

std::string& operator+=(std::string& s, string_view v)
{
  s.append(v.data(), v.size());
  return s;
}
}

std::hash<cm::string_view>::result_type std::hash<cm::string_view>::operator()(
  argument_type const& s) const noexcept
{
  // FNV-1a hash.
  static KWIML_INT_uint64_t const fnv_offset_basis = 0xcbf29ce484222325;
  static KWIML_INT_uint64_t const fnv_prime = 0x100000001b3;
  KWIML_INT_uint64_t h = fnv_offset_basis;
  for (char const& c : s) {
    h = h ^ KWIML_INT_uint64_t(KWIML_INT_uint8_t(c));
    h = h * fnv_prime;
  }
  return result_type(h);
}
#else
// Avoid empty translation unit.
void cm_string_view_cxx()
{
}
#endif
