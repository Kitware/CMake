/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstdint>

namespace cm {
namespace VS {
/** Known versions of Visual Studio.  */
enum class Version : std::uint16_t
{
  VS14 = 140,
  VS15 = 150,
  VS16 = 160,
  VS17 = 170,
  VS18 = 180,
};

enum class VersionExpress
{
  No,
  Yes,
};
}
}
