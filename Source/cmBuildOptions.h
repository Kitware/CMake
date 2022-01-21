/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

struct cmBuildOptions
{
public:
  cmBuildOptions() noexcept = default;
  explicit cmBuildOptions(bool clean, bool fast) noexcept
    : Clean(clean)
    , Fast(fast)
  {
  }
  explicit cmBuildOptions(const cmBuildOptions&) noexcept = default;
  cmBuildOptions& operator=(const cmBuildOptions&) noexcept = default;

  bool Clean = false;
  bool Fast = false;
};
