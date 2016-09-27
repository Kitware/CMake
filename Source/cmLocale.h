/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmLocale_h
#define cmLocale_h

#include <cmConfigure.h>

#include "cmStandardIncludes.h"

#include <locale.h>

class cmLocaleRAII
{
  const char* OldLocale;

public:
  cmLocaleRAII()
    : OldLocale(setlocale(LC_CTYPE, CM_NULLPTR))
  {
    setlocale(LC_CTYPE, "");
  }
  ~cmLocaleRAII() { setlocale(LC_CTYPE, this->OldLocale); }
};

#endif
