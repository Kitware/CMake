/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <cstdint>

#include <cm/string_view>
#include <cmext/enum_set>

namespace cm {
namespace StdIo {

class OStream;

/**
 * Represent a text attribute.
 */
enum class TermAttr : std::uint8_t
{
  Normal,
  ForegroundBold,
  ForegroundBlack,
  ForegroundBlue,
  ForegroundCyan,
  ForegroundGreen,
  ForegroundMagenta,
  ForegroundRed,
  ForegroundWhite,
  ForegroundYellow,
  BackgroundBold,
  BackgroundBlack,
  BackgroundBlue,
  BackgroundCyan,
  BackgroundGreen,
  BackgroundMagenta,
  BackgroundRed,
  BackgroundWhite,
  BackgroundYellow,
};
static constexpr std::size_t kTermAttrCount = 19;

/**
 * Represent a set of text attributes.
 */
using TermAttrSet = cm::enum_set<TermAttr, kTermAttrCount>;

/**
 * Print text to an output stream using a given set of color attributes.
 */
void Print(OStream& os, TermAttrSet const& attrs, cm::string_view text);

}
}
