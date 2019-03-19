/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#define _SCL_SECURE_NO_WARNINGS

#include "cmString.hxx"

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace cm {

static std::string const empty_string_;

void String::internally_mutate_to_stable_string()
{
  // We assume that only one thread mutates this instance at
  // a time even if we point to a shared string buffer refernced
  // by other threads.
  *this = String(data(), size());
}

bool String::is_stable() const
{
  return str_if_stable() != nullptr;
}

void String::stabilize()
{
  if (is_stable()) {
    return;
  }
  this->internally_mutate_to_stable_string();
}

std::string const* String::str_if_stable() const
{
  if (!data()) {
    // We view no string.
    // This is stable for the lifetime of our current value.
    return &empty_string_;
  }

  if (string_ && data() == string_->data() && size() == string_->size()) {
    // We view an entire string.
    // This is stable for the lifetime of our current value.
    return string_.get();
  }

  return nullptr;
}

std::string const& String::str()
{
  if (std::string const* s = str_if_stable()) {
    return *s;
  }
  // Mutate to hold a std::string that is stable for the lifetime
  // of our current value.
  this->internally_mutate_to_stable_string();
  return *string_;
}

const char* String::c_str()
{
  const char* c = data();
  if (c == nullptr) {
    return c;
  }

  // We always point into a null-terminated string so it is safe to
  // access one past the end.  If it is a null byte then we can use
  // the pointer directly.
  if (c[size()] == '\0') {
    return c;
  }

  // Mutate to hold a std::string so we can get a null terminator.
  this->internally_mutate_to_stable_string();
  c = string_->c_str();
  return c;
}

String& String::insert(size_type index, size_type count, char ch)
{
  std::string s;
  s.reserve(size() + count);
  s.assign(data(), size());
  s.insert(index, count, ch);
  return *this = std::move(s);
}

String& String::erase(size_type index, size_type count)
{
  if (index > size()) {
    throw std::out_of_range("Index out of range in String::erase");
  }
  size_type const rcount = std::min(count, size() - index);
  size_type const rindex = index + rcount;
  std::string s;
  s.reserve(size() - rcount);
  s.assign(data(), index);
  s.append(data() + rindex, size() - rindex);
  return *this = std::move(s);
}

String String::substr(size_type pos, size_type count) const
{
  if (pos > size()) {
    throw std::out_of_range("Index out of range in String::substr");
  }
  return String(*this, pos, count);
}

String::String(std::string&& s, Private)
  : string_(std::make_shared<std::string>(std::move(s)))
  , view_(string_->data(), string_->size())
{
}

String::size_type String::copy(char* dest, size_type count,
                               size_type pos) const
{
  return view_.copy(dest, count, pos);
}

std::ostream& operator<<(std::ostream& os, String const& s)
{
  return os.write(s.data(), s.size());
}

std::string& operator+=(std::string& self, String const& s)
{
  return self += s.view();
}

String IntoString<char*>::into_string(const char* s)
{
  if (!s) {
    return String();
  }
  return std::string(s);
}

string_view AsStringView<String>::view(String const& s)
{
  return s.view();
}

} // namespace cm
