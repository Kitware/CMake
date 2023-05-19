/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _SCL_SECURE_NO_WARNINGS

#include "cmString.hxx"

#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>

namespace cm {

static std::string const empty_string_;

void String::internally_mutate_to_stable_string()
{
  // We assume that only one thread mutates this instance at
  // a time even if we point to a shared string buffer referenced
  // by other threads.
  *this = String(this->data(), this->size());
}

bool String::is_stable() const
{
  return this->str_if_stable() != nullptr;
}

void String::stabilize()
{
  if (this->is_stable()) {
    return;
  }
  this->internally_mutate_to_stable_string();
}

std::string const* String::str_if_stable() const
{
  if (!this->data()) {
    // We view no string.
    // This is stable for the lifetime of our current value.
    return &empty_string_;
  }

  if (this->string_ && this->data() == this->string_->data() &&
      this->size() == this->string_->size()) {
    // We view an entire string.
    // This is stable for the lifetime of our current value.
    return this->string_.get();
  }

  return nullptr;
}

std::string const& String::str()
{
  if (std::string const* s = this->str_if_stable()) {
    return *s;
  }
  // Mutate to hold a std::string that is stable for the lifetime
  // of our current value.
  this->internally_mutate_to_stable_string();
  return *this->string_;
}

const char* String::c_str()
{
  const char* c = this->data();
  if (c == nullptr) {
    return c;
  }

  // We always point into a null-terminated string so it is safe to
  // access one past the end.  If it is a null byte then we can use
  // the pointer directly.
  if (c[this->size()] == '\0') {
    return c;
  }

  // Mutate to hold a std::string so we can get a null terminator.
  this->internally_mutate_to_stable_string();
  c = this->string_->c_str();
  return c;
}

String& String::insert(size_type index, size_type count, char ch)
{
  std::string s;
  s.reserve(this->size() + count);
  s.assign(this->data(), this->size());
  s.insert(index, count, ch);
  return *this = std::move(s);
}

String& String::erase(size_type index, size_type count)
{
  if (index > this->size()) {
    throw std::out_of_range("Index out of range in String::erase");
  }
  size_type const rcount = std::min(count, this->size() - index);
  size_type const rindex = index + rcount;
  std::string s;
  s.reserve(this->size() - rcount);
  s.assign(this->data(), index);
  s.append(this->data() + rindex, this->size() - rindex);
  return *this = std::move(s);
}

String String::substr(size_type pos, size_type count) const
{
  if (pos > this->size()) {
    throw std::out_of_range("Index out of range in String::substr");
  }
  return String(*this, pos, count);
}

String::String(std::string&& s, Private)
  : string_(std::make_shared<std::string>(std::move(s)))
  , view_(this->string_->data(), this->string_->size())
{
}

String::size_type String::copy(char* dest, size_type count,
                               size_type pos) const
{
  return this->view_.copy(dest, count, pos);
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
