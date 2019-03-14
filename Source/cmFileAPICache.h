/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileAPICache_h
#define cmFileAPICache_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cm_jsoncpp_value.h"

class cmFileAPI;

extern Json::Value cmFileAPICacheDump(cmFileAPI& fileAPI,
                                      unsigned long version);

#endif
