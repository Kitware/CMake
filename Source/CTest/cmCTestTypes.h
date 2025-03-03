/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cm/string_view>

namespace cmCTestTypes {

// Test output truncation mode
enum class TruncationMode
{
  Tail,
  Middle,
  Head
};

bool SetTruncationMode(TruncationMode& mode, cm::string_view str);

} // namespace cmCTestTypes
