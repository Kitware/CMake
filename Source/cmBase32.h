/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2016 Sebastian Holtermann <sebholt@xwmw.org>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmBase32_h
#define cmBase32_h

#include <cmConfigure.h> // IWYU pragma: keep

#include <stddef.h>
#include <string>

/** \class cmBase32Encoder
 * \brief Encodes a byte sequence to a Base32 byte sequence according to
 * RFC4648
 *
 */
class cmBase32Encoder
{
public:
  static const char paddingChar = '=';

public:
  cmBase32Encoder();
  ~cmBase32Encoder();

  // Encodes the given input byte sequence into a string
  // @arg input Input data pointer
  // @arg len Input data size
  // @arg padding Flag to append "=" on demand
  std::string encodeString(const unsigned char* input, size_t len,
                           bool padding = true);
};

#endif
