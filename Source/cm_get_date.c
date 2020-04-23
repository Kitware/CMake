/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cm_get_date.h"

// FIXME: This suppresses use of localtime_r because archive_getdate.c
// depends the rest of libarchive's checks for that.
#define CM_GET_DATE

#define __archive_get_date cm_get_date

#include "../Utilities/cmlibarchive/libarchive/archive_getdate.c"
