/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>

class GhsMultiGpj
{
public:
  enum Types
  {
    INTEGRITY_APPLICATION,
    LIBRARY,
    PROJECT,
    PROGRAM,
    REFERENCE,
    SUBPROJECT,
    CUSTOM_TARGET
  };

  static void WriteGpjTag(Types gpjType, std::ostream& fout);

  static char const* GetGpjTag(Types gpjType);
};
