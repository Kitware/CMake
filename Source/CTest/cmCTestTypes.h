/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

namespace cmCTestTypes {

enum class TruncationMode
{ // Test output truncation mode
  Tail,
  Middle,
  Head
};
}
