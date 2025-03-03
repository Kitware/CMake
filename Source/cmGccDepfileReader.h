/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

#include "cmGccDepfileReaderTypes.h"

enum class GccDepfilePrependPaths
{
  All,
  Deps,
};

/*
 * Read dependencies file and prepend prefix to all relative paths
 */
cm::optional<cmGccDepfileContent> cmReadGccDepfile(
  char const* filePath, std::string const& prefix = {},
  GccDepfilePrependPaths prependPaths = GccDepfilePrependPaths::All);
