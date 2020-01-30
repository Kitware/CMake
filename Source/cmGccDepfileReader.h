/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGccDepfileReader_h
#define cmGccDepfileReader_h

#include "cmGccDepfileReaderTypes.h"

cmGccDepfileContent cmReadGccDepfile(const char* filePath);

#endif
