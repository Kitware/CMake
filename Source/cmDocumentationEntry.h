/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** Standard documentation entry for cmDocumentation's formatting.  */
struct cmDocumentationEntry
{
  std::string Name;
  std::string Brief;
  char CustomNamePrefix = ' ';
  cmDocumentationEntry() = default;
  cmDocumentationEntry(const char* const n, const char* const b)
  {
    if (n) {
      this->Name = n;
    }
    if (b) {
      this->Brief = b;
    }
  }
};
