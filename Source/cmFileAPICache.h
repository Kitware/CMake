/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cm3p/json/value.h>

class cmFileAPI;

extern Json::Value cmFileAPICacheDump(cmFileAPI& fileAPI,
                                      unsigned long version);
