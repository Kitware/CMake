/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** \class cmHexFileConverter
 * \brief Can detects Intel Hex and Motorola S-record files and convert them
 *        to binary files.
 *
 */
class cmHexFileConverter
{
public:
  enum FileType
  {
    Binary,
    IntelHex,
    MotorolaSrec
  };
  static FileType DetermineFileType(std::string const& inFileName);
  static bool TryConvert(std::string const& inFileName,
                         std::string const& outFileName);
};
