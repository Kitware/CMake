/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>

/** \class cmBase32Encoder
 * \brief Encodes a byte sequence to a Base32 byte sequence according to
 * RFC4648
 *
 */
class cmBase32Encoder
{
public:
  static char const paddingChar = '=';

  cmBase32Encoder();
  ~cmBase32Encoder() = default;

  // Encodes the given input byte sequence into a string
  // @arg input Input data pointer
  // @arg len Input data size
  // @arg padding Flag to append "=" on demand
  std::string encodeString(unsigned char const* input, size_t len,
                           bool padding = true);
};
