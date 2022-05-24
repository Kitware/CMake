/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

/**
 * Enumerate types known to file(INSTALL).
 */
enum class cmInstallMode
{
  COPY,
  ABS_SYMLINK,
  ABS_SYMLINK_OR_COPY,
  REL_SYMLINK,
  REL_SYMLINK_OR_COPY,
  SYMLINK,
  SYMLINK_OR_COPY
};
