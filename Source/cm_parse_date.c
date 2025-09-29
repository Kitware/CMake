/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cm_parse_date.h"

// FIXME: This suppresses use of localtime_r because archive_parse_date.c
// depends the rest of libarchive's checks for that.
#define CM_PARSE_DATE

#define archive_parse_date cm_parse_date

#include "../Utilities/cmlibarchive/libarchive/archive_parse_date.c"
