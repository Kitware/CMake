/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

namespace cm {
namespace StdIo {

/**
 * Initialize process-wide `stdin`, `stdout`, and `stderr` streams.
 * After construction, standard in/out/err descriptors/handles are open,
 * and standard `FILE*` streams from `<cstdio>` are associated with them.
 */
class Init
{
public:
  Init();
  ~Init() = default;
  Init(Init&&) noexcept = default;
  Init(Init const&) = delete;
  Init& operator=(Init&&) noexcept = default;
  Init& operator=(Init const&) = delete;
};

}
}
