/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmFileAPICodemodel_h
#define cmFileAPICodemodel_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cm_jsoncpp_value.h"

class cmFileAPI;

extern Json::Value cmFileAPICodemodelDump(cmFileAPI& fileAPI,
                                          unsigned long version);

#endif
