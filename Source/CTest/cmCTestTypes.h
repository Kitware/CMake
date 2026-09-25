/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/optional>
#include <cm/string_view>

namespace cmCTestTypes {

// Test output truncation mode
enum class TruncationMode
{
  Tail,
  Middle,
  Head
};

// Resource error action
enum class ResourceErrorAction
{
  Fail,
  Skip
};

bool SetTruncationMode(TruncationMode& mode, cm::string_view str);

cm::optional<ResourceErrorAction> GetResourceErrorAction(cm::string_view str);
std::string ResourceErrorActionToString(ResourceErrorAction action);

} // namespace cmCTestTypes
