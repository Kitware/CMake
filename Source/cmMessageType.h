/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

enum class MessageType
{
  AUTHOR_WARNING,
  AUTHOR_ERROR,
  FATAL_ERROR,
  INTERNAL_ERROR,
  MESSAGE,
  WARNING,
  LOG,
  DEPRECATION_ERROR,
  DEPRECATION_WARNING
};
