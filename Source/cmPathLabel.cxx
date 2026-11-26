/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmPathLabel.h"

#include <utility>

#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(clang::no_sanitize)
#    define CM_NO_SANITIZE(check) [[clang::no_sanitize(check)]]
#  endif
#endif
#ifndef CM_NO_SANITIZE
#  define CM_NO_SANITIZE(check)
#endif

// The hash relies on unsigned wraparound.
CM_NO_SANITIZE("unsigned-integer-overflow")
cmPathLabel::cmPathLabel(std::string label)
  : Label(std::move(label))
  , Hash(0)
{
  // Use a Jenkins one-at-a-time hash with under/over-flow protection
  for (char i : this->Label) {
    this->Hash += i;
    this->Hash += ((this->Hash & 0x003FFFFF) << 10);
    this->Hash ^= ((this->Hash & 0xFFFFFFC0) >> 6);
  }
  this->Hash += ((this->Hash & 0x1FFFFFFF) << 3);
  this->Hash ^= ((this->Hash & 0xFFFFF800) >> 11);
  this->Hash += ((this->Hash & 0x0001FFFF) << 15);
}

bool cmPathLabel::operator<(cmPathLabel const& l) const
{
  return this->Hash < l.Hash;
}

bool cmPathLabel::operator==(cmPathLabel const& l) const
{
  return this->Hash == l.Hash;
}
